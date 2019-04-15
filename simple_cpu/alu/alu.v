// simple verilog implementation of an alu

//allows to have unused signals
/* verilator lint_off UNUSED */

module alu(
in1, in2, control, out, zero, neg
);

input wire [31:0] in1;
input wire [31:0] in2;
input wire [3:0] control; //operation selection, linked with riscv doc
output wire [31:0] out;
output wire zero; //is result equals to 0?
output wire neg; //is result negative?

wire [31:0] acc;

always @*
begin
   if (control == 4'h0) begin //addition
      acc = in1+in2;
      out = acc;

   end else if (control == 4'b1000) begin //sub
      acc = in1-in2;

   end else if (control == 4'b1001) begin //subs (signed sub)
      acc = ($signed(in1)-$signed(in2));

   end else if (control == 4'h1) begin //sll
      acc = in1 << in2[4:0];

   end else if (control == 4'h2) begin //slt
      acc[0] = ($signed(in1) < $signed(in2));
      acc[31:1] = 31'h0;

   end else if (control == 4'h3) begin //sltu
      acc[0] = (in1 < in2);
      acc[31:1] = 31'h0;

   end else if (control == 4'b100) begin //xor
      acc = in1 ^ in2;

   end else if (control == 4'b101) begin //srl
      acc = in1 >> in2[4:0];

   end else if (control == 4'b1101) begin //sra (arith. shift)
      acc = $signed(in1) >>> in2[4:0];

   end else if (control == 4'b110) begin //or
      acc = in1 | in2;

   end else if (control == 4'b111) begin //and
      acc = in1 & in2;

   end else begin
      acc = 32'h0;
   end

   out = acc;
   neg = (acc[31] == 1'b1);
   zero = (acc == 32'h0);

end

endmodule
