`timescale 1ns/1ps


import axi_vip_pkg::*;
import proc_system_axi_vip_0_0_pkg::*;

proc_system_axi_vip_0_0_mst_t master_agent;

logic resetn = 0;
logic clk125 = 0;
logic [3:0] leds;
logic [3:0] sw = 4'b1001; // MUTE = '1', Bypass = '0'

logic [31:0] RADIO_ADDR = 32'h43C0_0000;
logic [31:0] RADIO_ADC_OFFSET = 32'h0;
logic [31:0] RADIO_TUNE_OFFSET = 32'h4;
logic [31:0] RADIO_RST_OFFSET = 32'h8;
logic [31:0] RADIO_CLK_OFFSET = 32'hC;

logic [31:0] FIFO_ADDR = 32'h43C1_0000;
logic [31:0] FIFO_DATA_OFFSET = 32'h0;
logic [31:0] FIFO_DATA_CNT_OFFSET = 32'h4;

logic [31:0] INTC_ADDR = 32'h4180_0000;
logic [31:0] INTC_ISR_OFFSET = 32'h0;
logic [31:0] INTC_IER_OFFSET = 32'h8;
logic [31:0] INTC_IAR_OFFSET = 32'hC;
logic [31:0] INTC_MER_OFFSET = 32'h1C;


logic [31:0] addr = 32'h0;
logic [31:0] data = 32'h0;
logic [31:0] read_data;
xil_axi_resp_t resp;


module vip_full_sdr_tb();


    // Instantiate the VHDL module
    full_sdr_tb DUT (
        .resetn(resetn),
        .clk125(clk125),
        .leds(leds),
        .sw(sw)
    );
    
always #4 clk125 = ~clk125;

initial begin

resetn = 1;
#200
resetn = 0;
#300 
resetn = 1;
#500
resetn = 0;
#100
master_agent = new("master",DUT.i_lab7_top.i_proc_system_wrapper.proc_system_i.axi_vip_0.inst.IF);
master_agent.start_master();
// AXI4LITE_WRITE_BURST(addr,prot,data_in,resp) for a write transaction ()
// AXI4LITE_READ_BURST(addr,prot,data_out,resp) for a read transaction.
#500
// Set the tuning frequency to 1000000
master_agent.AXI4LITE_WRITE_BURST(RADIO_ADDR + RADIO_TUNE_OFFSET, 0, 1000000, resp);
// set the Surrogate ADC output to 1001000
master_agent.AXI4LITE_WRITE_BURST(RADIO_ADDR + RADIO_ADC_OFFSET, 0, 1001000, resp);
#500

master_agent.AXI4LITE_READ_BURST(RADIO_ADDR + RADIO_CLK_OFFSET, 0, read_data, resp);


// for (int i = 0; i < 1024; i++) begin
    // master_agent.AXI4LITE_WRITE_BURST(RADIO_ADDR + addr, 0, data, resp);
    // data = data + 1;
// end 

end
endmodule