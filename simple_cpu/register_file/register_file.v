//register file, 32 registers of 32bits

//allows to have unused signals
/* verilator lint_off UNUSED */

module register_file(
reset, read_reg_1, read_reg_2, write_reg, write_data, write_en, data_out_1, data_out_2
);

input reset;
input [4:0] read_reg_1;
input [4:0] read_reg_2;
input [4:0] write_reg;
input [31:0] write_data;
input write_en;
output [31:0] data_out_1;
output [31:0] data_out_2;

//32 registers of 32bits
reg [31:0] registers [0:31];

initial begin
  for(int i = 0; i < 32; i++) begin
    registers[i] = 32'h0;    
  end
  data_out_1 = 32'h0;
  data_out_2 = 32'h0;
end

always @(*)
begin
  if (reset == 1'b1) begin
    for(int i = 0; i < 32; i++) begin
      registers[i] = 32'h0;    
    end
    data_out_1 = 32'h0;
    data_out_2 = 32'h0;
  end else begin
    data_out_1 = registers[read_reg_1];
    data_out_2 = registers[read_reg_2];
    if(write_en && write_reg != 0) begin
      registers[write_reg] = write_data;
    end
  end
end

endmodule
