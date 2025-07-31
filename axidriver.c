/*  axidriver.c - The simplest kernel module.

* Copyright (C) 2013 - 2016 Xilinx, Inc
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.

*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along
*   with this program. If not, see <http://www.gnu.org/licenses/>.

*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/sysctl.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>

#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>



#include "axidriver.h"
/* Standard module information, edit as appropriate */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hussam Abdellatif");
MODULE_DESCRIPTION("AXI - DMA - Loadable module for xilinx axi dma allow just AXI DMA WRITe channel {AXI DMA writing to DRAM}");




struct axidriver_local {
  int irq_s2mm; //stream to Memory Mapped Interrupt
  unsigned long mem_start;
  unsigned long mem_end;
  void __iomem *base_addr;

  dma_addr_t dma_buffer_physical_address_s2mm[DRIVER_DMA_NUMBER_OF_BUFFERS_S2MM]; //used from dma_mapping.h, it hold dma memory ... cpu cant directly use it.
  void * dma_buffer_virtual_address_s2mm[DRIVER_DMA_NUMBER_OF_BUFFERS_S2MM];

  struct device *dev;
  struct cdev cdev;
  dev_t devt;
  struct class *driver_class;

  unsigned int s2mm_pointer; //buffer pointer for s2mm transfers.
  unsigned int total_blocks_to_s2mm; //total number of blocks to transfer in each s2mm transfer task
  unsigned int driver_dma_size_of_one_buffer; //size of one transfer. max value is 4Mbytes.
  unsigned int set_s2mm_mmap;
  unsigned int set_buffer_no_mmap; //which buffer number mmap should use for mapping;

  unsigned int dma_device_number; //device number for dma
  unsigned int print_isr_message; // should we do debug prints if interrupt happen
  unsigned int number_of_buffers_s2mm; //total # of buffers that we allocate from dram mem to s2mm channel
  unsigned int int_status;
};

static int probe_counter = 0;
static int irq_occured = 0;
static int stop_read =0;
static char ker_buf[100];

static ssize_t axidriver_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)  {
	return count;
}

static ssize_t axidriver_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)  {
	int n;
	if (stop_read == 0){
	    if (irq_occured){
		    int n = sprintf(ker_buf, "%d\n", irq_occured);
		    copy_to_user(buf, ker_buf, n);
		    stop_read = 1;
		}
     }




	return count;
}

static int axidriver_open(struct inode *inode, struct file *file){
  struct axidriver_local *lp;
  lp = container_of(inode->i_cdev, struct axidriver_local, cdev);
  file->private_data = lp;
  return 0;
}

static int axidriver_release(struct inode *inode, struct file *file)  {
	//
	return 0;
}



static long axidriver_ioctl(struct file *filep, unsigned int cmd, unsigned long arg){
  struct axidriver_local *lp = filep->private_data;
  int buffer_counter = 0 ;
  unsigned int tmpVal;

  switch (cmd){
    case IOCTL_MY_DMA_SET_TRANSFER_BLOCK_SIZE:
      lp->driver_dma_size_of_one_buffer = arg;
      break;

    case IOCTL_MY_DMA_SET_NUMBER_OF_BUFFERS_S2MM:
      if (arg > DRIVER_DMA_NUMBER_OF_BUFFERS_S2MM){
        printk("my_dma_driver_ioctl: Set Number of S2MM buffers. Value is Bigger than max possible. Setting number to %d\n", DRIVER_DMA_NUMBER_OF_BUFFERS_S2MM);
        tmpVal = DRIVER_DMA_NUMBER_OF_BUFFERS_S2MM;
      }else{
        tmpVal = arg;
      }
      lp->number_of_buffers_s2mm = tmpVal;
      break;

    case IOCTL_MY_DMA_ALLOCATE_COHERENT_MEMORY:
      for (buffer_counter =0; buffer_counter < lp->number_of_buffers_s2mm; buffer_counter++){
        lp->dma_buffer_virtual_address_s2mm[buffer_counter] = dma_alloc_coherent(lp->dev, lp->driver_dma_size_of_one_buffer, &lp->dma_buffer_physical_address_s2mm[buffer_counter], GFP_KERNEL);
          printk ("my_dma_driver_ioctl: my-axi-dma s2mm buffer %d physical address: %x, virtual address: %x\n",
					buffer_counter,
					lp->dma_buffer_physical_address_s2mm[buffer_counter],
					lp->dma_buffer_virtual_address_s2mm[buffer_counter]
				);
      }
      break;
    case IOCTL_MY_DMA_RELEASE_COHERENT_MEMORY:
      printk ("my_dma_driver_ioctl: running mem free...\n");
      for ( buffer_counter = 0; buffer_counter < lp->number_of_buffers_s2mm; buffer_counter++ )  {
        dma_free_coherent ( lp->dev, lp->driver_dma_size_of_one_buffer , lp->dma_buffer_virtual_address_s2mm[buffer_counter] , lp->dma_buffer_physical_address_s2mm[buffer_counter] );
      }

      printk ("my_dma_driver_ioctl: mem free done!\n");
      break;
    case IOCTL_MY_DMA_SET_NUMBER_OF_BLOCKS_TO_S2MM: //set how many blocks to transfer in each dma tast (how many times to serve the interrupt)
      lp->total_blocks_to_s2mm = arg;
      break;
    case IOCTL_MY_DMA_START_S2MM:
      lp->s2mm_pointer = 0;
      lp->int_status =0;
      iowrite32(0, lp->base_addr + S2MM_DEST_ADDRESS_MSB);
      iowrite32(lp->dma_buffer_physical_address_s2mm[lp->s2mm_pointer], lp->base_addr+S2MM_DEST_ADDRESS_LSB);
      iowrite32(lp->driver_dma_size_of_one_buffer, lp->base_addr+S2MM_LENGTH);
      break;
    case IOCTL_MY_DMA_GET_S2MM_POINTER:
      tmpVal = lp->s2mm_pointer;
      __put_user(tmpVal, (unsigned int __user *)arg);
      printk ( "my_dma_driver_ioctl: s2mm_pointer is pointing to buffer no. %d \n", lp->s2mm_pointer );
      break;
    case IOCTL_MY_DMA_SET_S2MM_MMAP:
      lp->set_s2mm_mmap = arg;
      break;
    case IOCLT_MY_DMA_SET_BUFFER_NO_MMAP:
      lp->set_buffer_no_mmap = arg;
      break;
    case IOCTL_MY_DMA_DISABLE_ISR_MESSAGES:
      lp->print_isr_message = 0;
      break;
    case IOCTL_INTR_HELPER:
      printk("\n INT STATUS %d \n" ,lp->int_status );
      lp->int_status = 0;
      printk("\n INT STATUS Changed %d \n" ,lp->int_status );
      if ( lp->total_blocks_to_s2mm > 0 )  {
        iowrite32 ( lp->dma_buffer_physical_address_s2mm[lp->s2mm_pointer] , lp->base_addr + S2MM_DEST_ADDRESS_LSB );
        iowrite32 ( lp->driver_dma_size_of_one_buffer, lp->base_addr + S2MM_LENGTH );
      }
      break;
    case IOCTL_CHECK_STATUS:
      __put_user(lp->int_status, (unsigned int __user *)arg);
      break;
    case IOCTL_MY_DMA_DUMP_REGS:
      printk ( "my_dma_driver_ioctl: s2mm cr: %x\n", ioread32(lp->base_addr + S2MM_DMA_CR) );
      printk ( "my_dma_driver_ioctl: s2mm status: %x\n", ioread32(lp->base_addr + S2MM_DMA_SR) );
      printk ( "my_dma_driver_ioctl: s2mm addr lsb: %x\n", ioread32(lp->base_addr + S2MM_DEST_ADDRESS_LSB) );
      printk ( "my_dma_driver_ioctl: s2mm addr msb: %x\n", ioread32(lp->base_addr + S2MM_DEST_ADDRESS_MSB) );
      printk ( "my_dma_driver_ioctl: s2mm length: %x\n", ioread32(lp->base_addr + S2MM_LENGTH ));
      break;
    case IOCTL_MY_DMA_RESET:
      iowrite32 ( 0x4 , lp->base_addr  + S2MM_DMA_CR );

      printk ("my_dma_driver_ioctl: dma reset is done! \n");
      printk ( "my_dma_driver_ioctl: s2mm cr: %x\n", ioread32(lp->base_addr + S2MM_DMA_CR) );

      /* setup s2mm channel */
      tmpVal = ioread32 ( lp->base_addr + S2MM_DMA_CR );
      tmpVal = tmpVal | 0x1001;
      iowrite32  ( tmpVal, lp->base_addr  + S2MM_DMA_CR );

      /* double check if the value is set */
      tmpVal = ioread32 ( lp->base_addr  + S2MM_DMA_CR );
      printk ( "my_dma_driver_ioctl: value for dma config reg for s2mm is %x\n", tmpVal );
      break;
    default:
      printk("Default Case Should never happen :-)");
      break;
  }
}




static int my_dma_driver_mmap(struct file *filep, struct vm_area_struct *vma){
  int result = 0;
  unsigned int addr = 0 ;
  unsigned int size = 0 ;

  struct axidriver_local *lp = filep->private_data;

  if ( ( vma->vm_end - vma->vm_start ) > (DRIVER_DMA_SIZE_OF_ONE_BUFFER) )  {
    printk ( KERN_INFO "%s:%i requested mmap region size is bigger dma buffer. requested=%d, max buffer=%x\n",
      __FUNCTION__,
      __LINE__ ,
      vma->vm_end - vma->vm_start ,
      DRIVER_DMA_SIZE_OF_ONE_BUFFER );
    return -1;
  }

  if ( lp->set_s2mm_mmap )
    addr = lp->dma_buffer_physical_address_s2mm[lp->set_buffer_no_mmap];
  else
    printk ("my-axi-dma-driver : mmap, either one of mm2s or s2mm should have been selected in user level app before running mmap. \n");

  size = vma->vm_end - vma->vm_start;

  addr = vma->vm_pgoff + (addr >> PAGE_SHIFT);

  vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP);
  vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

  result = io_remap_pfn_range( 	vma,
          vma->vm_start,
          addr,
          size, vma->vm_page_prot);

  if( result != 0 ){
    printk( KERN_INFO "%s:%i mmap failed.\n", __FUNCTION__, __LINE__);
    return -1;
  }

  printk( KERN_INFO "%s:%i mmap done!, virt %lx, phys %x\n", __FUNCTION__, __LINE__,
  vma->vm_start, addr << PAGE_SHIFT );

  return 0;

}


static const struct file_operations driver_fops  ={
  .owner = THIS_MODULE,
  .write = axidriver_write,
  .read  = axidriver_read,
  .open  = axidriver_open,
  .release = axidriver_release,
  .unlocked_ioctl = axidriver_ioctl,
  .mmap = my_dma_driver_mmap,
};


static irqreturn_t my_axi_dma_irq_s2mm(int irq, void *lp){
  struct axidriver_local *lp_in_function = lp;
  irq_occured = 1;
  iowrite32(0x1000, lp_in_function->base_addr + S2MM_DMA_SR); // clear interrupt
  if ( lp_in_function->print_isr_message )
		printk ( KERN_DEFAULT "%s:%i dma %d received s2mm interrupt. current s2mm pointer: %d\n", __FUNCTION__, __LINE__, lp_in_function->dma_device_number, lp_in_function->s2mm_pointer );

  if ( lp_in_function->s2mm_pointer == (lp_in_function->number_of_buffers_s2mm-1) )
		lp_in_function->s2mm_pointer = 0;
	else
		lp_in_function->s2mm_pointer = lp_in_function->s2mm_pointer + 1;

  lp_in_function->total_blocks_to_s2mm = lp_in_function->total_blocks_to_s2mm - 1;
  lp_in_function->int_status = 1;
  printk("\n INT STATUS Changed to %d \n" ,lp_in_function->int_status );
  return IRQ_HANDLED;

}




static int axidriver_probe(struct platform_device *pdev)
{
	struct resource *r_irq; /* Interrupt resources */
	struct resource *r_mem; /* IO mem resources */
	struct device *dev = &pdev->dev;
	struct axidriver_local *lp = NULL;
	printk("AXI_DMA_DRIVER_PROBE_FUCNTION");
	int rc = 0;
  unsigned int tmpVal;

  dev_t devt;
  int retval = 0;

  char *driver_name_str;
  char *driver_name_str_intr_s2mm;

  driver_name_str = kzalloc ( 32 , GFP_KERNEL );
	driver_name_str_intr_s2mm = kzalloc ( 32 , GFP_KERNEL );

  sprintf ( driver_name_str, "%s_%d", DRIVER_NAME, probe_counter );
  sprintf ( driver_name_str_intr_s2mm, "%s_%d_s2mm", DRIVER_NAME, probe_counter );
  printk ("******************* my_axi_dma_probe: driver name: %s probing ******************* \n", driver_name_str );


	/* Get iospace for the device */
	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_mem) {
		dev_err(dev, "invalid address\n");
		return -ENODEV;
	}
	lp = (struct axidriver_local *) kmalloc(sizeof(struct axidriver_local), GFP_KERNEL);
	if (!lp) {
		dev_err(dev, "Cound not allocate axidriver device\n");
		return -ENOMEM;
	}
	dev_set_drvdata(dev, lp);
	lp->mem_start = r_mem->start;
	lp->mem_end = r_mem->end;

	if (!request_mem_region(lp->mem_start,
				lp->mem_end - lp->mem_start + 1,
				DRIVER_NAME)) {
		dev_err(dev, "Couldn't lock memory region at %p\n",
			(void *)lp->mem_start);
		rc = -EBUSY;
		goto error1;
	}

	lp->base_addr = ioremap(lp->mem_start, lp->mem_end - lp->mem_start + 1);
	if (!lp->base_addr) {
		dev_err(dev, "axidriver: Could not allocate iomem\n");
		rc = -EIO;
		goto error2;
	}

	/* Get IRQ for the device */
  lp->irq_s2mm = 0;
	r_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!r_irq) {
		dev_info(dev, "no IRQ found\n");
		dev_info(dev, "axidriver at 0x%08x mapped to 0x%08x\n",
			(unsigned int __force)lp->mem_start,
			(unsigned int __force)lp->base_addr);
		return 0;
	}
	lp->irq_s2mm = r_irq->start;
	rc = request_irq(lp->irq_s2mm, my_axi_dma_irq_s2mm, 0, driver_name_str, lp);
	if (rc) {
		dev_err(dev, "testmodule: Could not allocate interrupt %d.\n",
			lp->irq_s2mm);
		goto error3;
	}


  // devt = MKDEV(DRIVER_MAJOR, DRIVER_MINOR + 0);
	alloc_chrdev_region ( &devt, 0, 0, driver_name_str );

	lp->devt = devt;

	cdev_init(&lp->cdev, &driver_fops);

	lp->cdev.owner = THIS_MODULE;

	retval = cdev_add(&lp->cdev, devt, 1); //  probe_counter + 1);
	if (retval) {
		printk ("my_axi_dma_probe: cdev_add() failed\n");
		goto error3;
	}

	lp->driver_class = class_create(THIS_MODULE, driver_name_str );
	device_create(lp->driver_class, dev, devt, NULL, "%s%d", driver_name_str, 0);
	printk ("my_axi_dma_probe: probe no %d , done creating character device: major : %d...\n", probe_counter, MAJOR(lp->devt) );

	// inside the main structure of the device, i want to keep the device number and use it for print messages for better debugging.
	lp->dma_device_number = probe_counter;

	probe_counter++;

	/***********************************************************************************************/
	/*
	/* setup pointers and init the dma
	/*
	/***********************************************************************************************/
	lp->dev = dev;
	lp->s2mm_pointer = 0;
	lp->driver_dma_size_of_one_buffer = DRIVER_DMA_SIZE_OF_ONE_BUFFER;
	lp->number_of_buffers_s2mm = 1;
	lp->total_blocks_to_s2mm = 1;
	lp->set_s2mm_mmap = 1;
	lp->set_buffer_no_mmap = 0;
	lp->print_isr_message = 1;



	dev_info(dev,"axidriver at 0x%08x mapped to 0x%08x, irq=%d\n",
		(unsigned int __force)lp->mem_start,
		(unsigned int __force)lp->base_addr,
		lp->irq_s2mm);
	return 0;
error3:
	free_irq(lp->irq_s2mm, lp);
error2:
	release_mem_region(lp->mem_start, lp->mem_end - lp->mem_start + 1);
error1:
	kfree(lp);
	dev_set_drvdata(dev, NULL);
	return rc;
}

static int axidriver_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct axidriver_local *lp = dev_get_drvdata(dev);
	free_irq(lp->irq_s2mm, lp);
	iounmap(lp->base_addr);
	release_mem_region(lp->mem_start, lp->mem_end - lp->mem_start + 1);
	kfree(lp);
	dev_set_drvdata(dev, NULL);
	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id axidriver_of_match[] = {
	{ .compatible = "xlnx,axidma-w32-1.0", },
	{ /* end of list */ },
};
MODULE_DEVICE_TABLE(of, axidriver_of_match);
#else
# define axidriver_of_match
#endif


static struct platform_driver axidriver_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table	= axidriver_of_match,
	},
	.probe		= axidriver_probe,
	.remove		= axidriver_remove,
};

static int __init axidriver_init(void)
{
		printk("my-axi-dma-driver: init.\n");
printk("AXI_DMA_DRIVER_PROBE_FUCNTION");

	return platform_driver_register(&axidriver_driver);
}


static void __exit axidriver_exit(void)
{
	platform_driver_unregister(&axidriver_driver);
	printk(KERN_ALERT "my-axi-dma-driver: exit.\n");
}

module_init(axidriver_init);
module_exit(axidriver_exit);
