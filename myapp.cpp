#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h> 		//inet_addr
#include <unistd.h>    		//write
#include <time.h>
#include <sys/ioctl.h>		/* ioctl */
#include <sys/sendfile.h>
#include <sys/mman.h>

#include "axidriver.h"

char read_buf[100];
char read_buf2[100];
char write_buf[100];

int main(int argc, char **argv)
{

/*******************************************************************************
 *
 * variables
 *
 *******************************************************************************/

	unsigned char my_axi_dma_driver_nodes[128] = "/dev/axidriver_00";
	unsigned char my_sample_generator_node[128] = "/dev/tutDevice";

	int interrupt_occur = 0;

	int my_axi_dma_driver_file_handles  = 0;
	int my_sample_generator_file_handle = 0;

  	unsigned int read_counter;
	unsigned int tmp_i = 0, tmp_j = 0, tmp_k = 0;
	unsigned int ret_val = 0;
	unsigned int* my_axi_dma_write_pointer;

	const unsigned int driver_dma_number_of_buffers_s2mm_local = 1;

	volatile unsigned long *dma_user_level_s2mm_buffers[driver_dma_number_of_buffers_s2mm_local];

	unsigned int number_of_blocks_to_transfer = 5;

	unsigned int counter_for_mem = 0;

 	my_axi_dma_driver_file_handles = open ( "/dev/axidriver_00", O_RDWR );
	if ( my_axi_dma_driver_file_handles  < 0 ) {
 		printf ("user level: %s can not be opened.\n", my_axi_dma_driver_nodes);
 		exit(-1);
 	}
	//printf ("user level: opening file node: %s\n", my_sample_generator_node );
	my_sample_generator_file_handle = open ( "/dev/tutDevice", O_RDWR );

	if ( my_sample_generator_file_handle < 0 ) {
		printf ("user level: %s can not be opened.\n", my_sample_generator_node );
		exit (-1);
	}

	ret_val = ioctl ( my_axi_dma_driver_file_handles, IOCTL_MY_DMA_SET_TRANSFER_BLOCK_SIZE, DRIVER_DMA_SIZE_OF_ONE_BUFFER );
	ret_val = ioctl ( my_axi_dma_driver_file_handles, IOCTL_MY_DMA_SET_NUMBER_OF_BUFFERS_S2MM, driver_dma_number_of_buffers_s2mm_local );
	ret_val = ioctl ( my_axi_dma_driver_file_handles, IOCTL_MY_DMA_ALLOCATE_COHERENT_MEMORY, 1 );
	ret_val = ioctl ( my_axi_dma_driver_file_handles, IOCTL_MY_DMA_SET_S2MM_MMAP, 1 );


	//write

	printf("Enter the string to write into the driver:\n");
        scanf(" %[^\t\n]s" , write_buf);
        write(my_sample_generator_file_handle, write_buf, strlen(write_buf)+1);
        printf("Done");


	ret_val = ioctl ( my_axi_dma_driver_file_handles, IOCTL_MY_DMA_RESET , 1 );
		for ( tmp_i = 0; tmp_i < driver_dma_number_of_buffers_s2mm_local; tmp_i++ )  {
		ret_val = ioctl ( my_axi_dma_driver_file_handles, IOCLT_MY_DMA_SET_BUFFER_NO_MMAP, tmp_i );
		dma_user_level_s2mm_buffers[tmp_i] = (volatile unsigned long*) mmap ( NULL, DRIVER_DMA_SIZE_OF_ONE_BUFFER , PROT_READ | PROT_WRITE , MAP_SHARED, my_axi_dma_driver_file_handles, 0);
	}




	ret_val = ioctl ( my_axi_dma_driver_file_handles, IOCTL_MY_DMA_SET_NUMBER_OF_BLOCKS_TO_S2MM, number_of_blocks_to_transfer ) ;
	ret_val = ioctl ( my_axi_dma_driver_file_handles, IOCTL_MY_DMA_START_S2MM, 1 );

	ret_val = ioctl ( my_axi_dma_driver_file_handles, IOCTL_MY_DMA_GET_S2MM_POINTER, my_axi_dma_write_pointer );


	read(my_sample_generator_file_handle, read_buf2, 100);
	read_counter = 0;

	unsigned int check_int_stauts;

	unsigned int blocks_read = 0;
	while (blocks_read<5){
		ret_val = ioctl(my_axi_dma_driver_file_handles, IOCTL_CHECK_STATUS, &check_int_stauts);
		if(check_int_stauts){
			printf("\n\n\n\n Reading Block # %d \n " ,blocks_read );
			for(tmp_i =0; tmp_i<driver_dma_number_of_buffers_s2mm_local;tmp_i++){
			printf("\n*********************************************\n");
			  	for(tmp_k=0; tmp_k<DRIVER_DMA_SIZE_OF_ONE_BUFFER;tmp_k++){
									if(tmp_k > 1250)
										break;
					  printf("Value %d = %lu\n",   tmp_k ,dma_user_level_s2mm_buffers[tmp_i][tmp_k]);
			  	}
			}
			ret_val = ioctl(my_axi_dma_driver_file_handles, IOCTL_INTR_HELPER, 0);
			blocks_read = blocks_read +1;
		}




	}







	ret_val = ioctl ( my_axi_dma_driver_file_handles, IOCTL_MY_DMA_RELEASE_COHERENT_MEMORY, 1 ); // relesae mem



	return 0;
}





/*

  read_counter = 0;
   for(tmp_i =0; tmp_i<driver_dma_number_of_buffers_s2mm_local;tmp_i++){
  	for(tmp_k=0; tmp_k<DRIVER_DMA_SIZE_OF_ONE_BUFFER;tmp_k++){
		  read_counter =   read_counter +1;
		if(read_counter > 20){
			break;
		}
		printf("Value %d = 0x%08x\n",   read_counter ,*(dma_user_level_s2mm_buffers[tmp_i]+read_counter));

	}
  }

*/
