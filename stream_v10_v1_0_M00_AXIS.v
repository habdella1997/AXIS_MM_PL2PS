
`timescale 1 ns / 1 ps

	module stream_v10_v1_0_M00_AXIS #
	(
		// Users to add parameters here
        parameter integer buffer_width_default = 1024,
        parameter integer data_width = 64,
        parameter integer NumberOfPacketsToSend = 1024,
		// User parameters ends
		// Do not modify the parameters beyond this line

		// Width of S_AXIS address bus. The slave accepts the read and write addresses of width C_M_AXIS_TDATA_WIDTH.
		parameter integer C_M_AXIS_TDATA_WIDTH	= 64,
		// Start count is the number of clock cycles the master will wait before initiating/issuing any transaction.
		parameter integer C_M_START_COUNT	= 32
	)
	(
	    input wire [31:0] buffer_width,
		// Users to add ports here 
        input wire enable_generator,
        input wire reset_buffer, 
        input wire  [40:0] eqed_re_0 ,   
        input wire  [40:0] eqed_re_1 ,   
        input wire  [40:0] eqed_re_2 ,   
        input wire  [40:0] eqed_re_3 ,   
        input wire  [40:0] eqed_re_4 ,   
        input wire  [40:0] eqed_re_5 ,   
        input wire  [40:0] eqed_re_6 ,   
        input wire  [40:0] eqed_re_7 ,   
        input wire  [40:0] eqed_re_8 ,   
        input wire  [40:0] eqed_re_9 ,   
        input wire  [40:0] eqed_re_10 ,  
        input wire  [40:0] eqed_re_11 ,  
        input wire  [40:0] eqed_re_12 ,  
        input wire  [40:0] eqed_re_13 ,  
        input wire  [40:0] eqed_re_14 ,  
        input wire  [40:0] eqed_re_15 ,  
                                         
        input wire  [40:0] eqed_im_0 ,   
        input wire  [40:0] eqed_im_1 ,   
        input wire  [40:0] eqed_im_2 ,   
        input wire  [40:0] eqed_im_3 ,   
        input wire  [40:0] eqed_im_4 ,   
        input wire  [40:0] eqed_im_5 ,   
        input wire  [40:0] eqed_im_6 ,   
        input wire  [40:0] eqed_im_7 ,   
        input wire  [40:0] eqed_im_8 ,   
        input wire  [40:0] eqed_im_9 ,   
        input wire  [40:0] eqed_im_10 ,  
        input wire  [40:0] eqed_im_11 ,  
        input wire  [40:0] eqed_im_12 ,  
        input wire  [40:0] eqed_im_13 ,  
        input wire  [40:0] eqed_im_14 ,  
        input wire  [40:0] eqed_im_15 ,   
           
        input wire [15:0] eqed_valid, 
         
 
		// User ports ends
		// Do not modify the ports beyond this line

		// Global ports
		input wire  M_AXIS_ACLK,
		// 
		input wire  M_AXIS_ARESETN,
		// Master Stream Ports. TVALID indicates that the master is driving a valid transfer, A transfer takes place when both TVALID and TREADY are asserted. 
		output wire  M_AXIS_TVALID,
		// TDATA is the primary payload that is used to provide the data that is passing across the interface from the master.
		output wire [C_M_AXIS_TDATA_WIDTH-1 : 0] M_AXIS_TDATA,
		// TSTRB is the byte qualifier that indicates whether the content of the associated byte of TDATA is processed as a data byte or a position byte.
		output wire [(C_M_AXIS_TDATA_WIDTH/8)-1 : 0] M_AXIS_TSTRB,
		// TLAST indicates the boundary of a packet.
		output wire  M_AXIS_TLAST,
		// TREADY indicates that the slave can accept a transfer in the current cycle.
		input wire  M_AXIS_TREADY
	);
	
	/*
	   SET THE BUFFER WIDTH FOR THE AXI STREAM. LETS HAVE 1024 BE DEFAULT VAL.
	   Set the reset as well
	*/
	reg [31:0] buffer_max_counter;
	
	always @ (posedge M_AXIS_ACLK) begin 
	   if(!M_AXIS_ARESETN) begin
	       buffer_max_counter <= 32'b0;
	   end
	   buffer_max_counter <= ( buffer_width > 4096 || buffer_width < 1024) ? buffer_width_default: buffer_width;
	end
	
	
	/*
	
	   SET THE ENABLE SIGNAL HERE
	
	*/
	
	
	reg [ data_width-1:0] buffer_data[0:buffer_width_default-1]; // 4096 elements of 40bits wide buffer. If doesnt work replace with BRAM.
	reg [12:0] buffer_counter; //buffer counter to start sending data when it fills up. 
	reg [6:0] sleep_counter = 0;
	
	integer i;
    integer valid_counter;
    integer eq_counter;
    integer j;
	reg stop_signal ; 
	
	reg [10:0] counter_to_force_enable;
	reg force_enable; 

	always @ (posedge M_AXIS_ACLK) begin
	   if(!M_AXIS_ARESETN) begin
	       counter_to_force_enable <= 0;
	       stop_signal <= 0;
	       force_enable <= 0; 
	       valid_counter <= 0;
	       eq_counter <= 0;
	   end
	   else begin
	      if (M_AXIS_TLAST)begin
	           stop_signal <= 0;
	       end
           if (enable_generator && !force_enable && !stop_signal) // staart the counter delay
               counter_to_force_enable <= counter_to_force_enable + 1;
           if (counter_to_force_enable >= 10 && (buffer_counter+32) < buffer_width_default )begin //begin filling buffer
               force_enable <=1;
               stop_signal  <=0;
           end else if (counter_to_force_enable >= 10 && (buffer_counter+32) >= buffer_width_default )  begin
                force_enable <= 0;
                stop_signal  <= 1;
                counter_to_force_enable <= 0 ; 
           end else begin 
                //do nothing
                force_enable <= force_enable;
           end
       end
	end

    /*
    
     FILL BUFFER HERE
    
    */
    
    wire [40:0] eqed_vals [0:31]; // [i,q,i,q,i,q .....]

    assign eqed_vals[0] =eqed_re_0;
    assign eqed_vals[2] =eqed_re_1 ;
    assign eqed_vals[4] =eqed_re_2 ;
    assign eqed_vals[6] =eqed_re_3 ;
    assign eqed_vals[8] =eqed_re_4 ;
    assign eqed_vals[10] =eqed_re_5 ;
    assign eqed_vals[12] =eqed_re_6 ;
    assign eqed_vals[14] =eqed_re_7 ;
    assign eqed_vals[16] =eqed_re_8 ;
    assign eqed_vals[18] =eqed_re_9 ;
    assign eqed_vals[20] =eqed_re_10;
    assign eqed_vals[22] =eqed_re_11;
    assign eqed_vals[24] =eqed_re_12;
    assign eqed_vals[26] =eqed_re_13;
    assign eqed_vals[28] =eqed_re_14;
    assign eqed_vals[30] =eqed_re_15;
    assign eqed_vals[1] =eqed_im_0 ;
    assign eqed_vals[3] =eqed_im_1 ;
    assign eqed_vals[5] =eqed_im_2 ;
    assign eqed_vals[7] =eqed_im_3 ;
    assign eqed_vals[9] =eqed_im_4 ;
    assign eqed_vals[11] =eqed_im_5 ;
    assign eqed_vals[13] =eqed_im_6 ;
    assign eqed_vals[15] =eqed_im_7 ;
    assign eqed_vals[17] =eqed_im_8 ;
    assign eqed_vals[19] =eqed_im_9 ;
    assign eqed_vals[21] =eqed_im_10;
    assign eqed_vals[23] =eqed_im_11;
    assign eqed_vals[25] =eqed_im_12;
    assign eqed_vals[27] =eqed_im_13;
    assign eqed_vals[29] =eqed_im_14;
    assign eqed_vals[31] =eqed_im_15;




	always @ (posedge M_AXIS_ACLK) begin
	   if(!M_AXIS_ARESETN) begin
	       for (i=0; i < buffer_width_default; i = i+1)begin
	           buffer_data[i] <= 0 ;
	       end
	       buffer_counter <=0;
	   end else begin  
	       if (M_AXIS_TLAST)begin
	           buffer_counter <= 0;
	       end
           if(force_enable && !stop_signal) begin
               for (j=0; j <31; j = j+2)begin
                   buffer_data[j+buffer_counter]   <= (eqed_valid[j/2]) ? {23'b0 , eqed_vals[j]} : 64'b0 ;
                   buffer_data[j+buffer_counter+1] <= (eqed_valid[j/2]) ? {23'b0 , eqed_vals[j+1]} : 64'b0 ;
               end
               buffer_counter <= buffer_counter + 32;
           end
	   end
	end
	
	
	
   
   /*
    SET THE TVALID SIGNAL HERE
   */
    
    reg assert_tvalid;
    
    always @ (posedge M_AXIS_ACLK) begin 
        if(!M_AXIS_ARESETN) begin
            assert_tvalid <= 0;
        end
        else begin
            if (!force_enable && stop_signal)  // Buffer is filled, need to move data into DRAM now
                assert_tvalid <= 1;
            else 
                assert_tvalid <=0;
        end
    end
    assign M_AXIS_TVALID = assert_tvalid; 
 

    /*
    
    ASSIGN TDATA HERE, TDATA WILL BE TRANSFERED TO THE BUFFER
    
    */   
    
    
    reg  [31:0] data_transfer_counter; 
    reg  [63:0] data_hold;
    
    always @ (posedge M_AXIS_ACLK) begin 
        if(!M_AXIS_ARESETN) begin
            data_transfer_counter <= 32'b0;
            data_hold  <= 64'b0;
        end
        else begin
            if (M_AXIS_TVALID && M_AXIS_TREADY && data_transfer_counter < buffer_width_default) begin
                data_hold <= buffer_data[data_transfer_counter];
                data_transfer_counter <= data_transfer_counter + 1;
            end
            if (!M_AXIS_TVALID && data_transfer_counter >= buffer_width_default-1) 
                data_transfer_counter <= 0 ; //reset data-transfer-counter to 0 and completiong of transfer
        end
    end
    
    assign M_AXIS_TDATA = data_hold;

    /*
    
    Begin TLAST RTL: 
    
    */   
    assign M_AXIS_TLAST = (data_transfer_counter == buffer_width_default - 2) ? 1:0; // 4095 is last packet, 
    


    /*
    
    END TLAST RTL
    
    */
     //*******************************************************************************************
   //*******************************************************************************************
  //*******************************************************************************************
    wire data_transmission_ongoing; 
    assign data_transmission_ongoing = M_AXIS_TVALID && M_AXIS_TREADY;
    assign M_AXIS_TSTRB = (data_transmission_ongoing) ? 8'hff : 8'h00;

	endmodule
