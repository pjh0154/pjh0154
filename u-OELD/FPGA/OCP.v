`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/02/01 13:49:05
// Design Name: 
// Module Name: OCP
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module OCP(
    input wire CLK_10MHz,
    input wire nRESET,
    input wire EN,
    input wire OCP_CLEAR,
    input wire OCP_IN,
    output wire OCP_RESULT
    );
    
    reg         [3:0]           REG_OCP_BUFFER;
    reg                         REG_OCP_RESULT;
    
    always @ (posedge CLK_10MHz) begin
        if(!nRESET) REG_OCP_BUFFER <= 4'b0000;
        else begin
            if(!REG_OCP_RESULT) begin
                if(EN) REG_OCP_BUFFER <= {REG_OCP_BUFFER[2:0], OCP_IN};
                else REG_OCP_BUFFER <= 4'b0000;
            end
        end
    end
    
    always @ (negedge CLK_10MHz) begin
        if(!nRESET) REG_OCP_RESULT <= 1'b0;
        else begin
            if(!OCP_CLEAR) begin
                if(REG_OCP_BUFFER == 4'b1111) REG_OCP_RESULT <= 1'b1;
            end
            else REG_OCP_RESULT <= 1'b0;
        end
    end
    
    assign OCP_RESULT = REG_OCP_RESULT;
    
endmodule
