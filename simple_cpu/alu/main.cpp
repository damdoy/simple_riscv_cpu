#include <stdlib.h>
#include "Valu.h"
#include "verilated.h"

void cycle(Valu *tb, uint32_t in1, uint32_t in2, uint8_t alu_op){
  tb->in1 = in1;
  tb->in2 = in2;
  tb->control = alu_op;
  tb->eval();
  
  uint32_t expected_out = 0;
  uint32_t out = tb->out;
  uint8_t zero = tb->zero;
  uint8_t neg = tb->neg;

  if(alu_op == 0){
    expected_out = in1+in2;
  }
  else if(alu_op == 1){
    expected_out = in1&in2;
  }
  else if(alu_op == 2){
    expected_out = in1|in2;
  }
  else if(alu_op == 3){
    expected_out = in1^in2;
  }
  else if(alu_op == 4){ //slt
    expected_out = ((int)in1 < (int)in2);
  }
  else if(alu_op == 5){ //sltu
    expected_out = (in1 < in2);
  }
  else if(alu_op == 6){ //sll
    expected_out = (in1 << in2);
  }
  else if(alu_op == 7){ //srl (logic)
    expected_out = (in1 >> in2);
  }
  else if(alu_op == 8){ //sra (arith)
    expected_out = ( (int)in1 >> in2);
  }
  else if(alu_op == 9){ //sub
    expected_out = ( in1-in2 );
  }
  
  printf("in1:%d, in2:%d, op:%d, out:%d, zero:%d, neg:%d, expect_out:%d\n", in1, in2, alu_op, out, zero, neg, expected_out);

  if( (out != expected_out) || (zero != (expected_out == 0)) || (neg != ((int)expected_out < 0)) ){
    printf("failed\n");
    exit(1);
  }

}

int main(int argc, char **argv) {
	// Initialize Verilators variables
	Verilated::commandArgs(argc, argv);

	// Create an instance of our module under test
	Valu *tb = new Valu;
  int function = 0;

  cycle(tb, 0, 0, 0);
  

  function = 0; //add
  cycle(tb, 23, 34, function);
  cycle(tb, -1, -1, function);
  cycle(tb, 1001, -1, function);
  cycle(tb, 1234, -1234, function);
  
  function = 1; //and
  cycle(tb, 23, 34, function);
  cycle(tb, -1, -1, function);
  cycle(tb, 1001, -1, function);
  cycle(tb, 1234, -1234, function);
  cycle(tb, 0x3, 0x4, function);

  function = 2; //or
  cycle(tb, 23, 34, function);
  cycle(tb, -1, -1, function);
  cycle(tb, 1001, -1, function);
  cycle(tb, 1234, -1234, function);
  
  function = 3; //xor
  cycle(tb, 23, 34, function);
  cycle(tb, -1, -1, function);
  cycle(tb, 1001, -1, function);
  cycle(tb, 1234, -1234, function);
  
  function = 4;//slt
  cycle(tb, 232, 3277, function);
  cycle(tb, 4324, 24, function);
  cycle(tb, -1, 1, function);
  cycle(tb, -34, -2, function);

  function = 5;//sltu
  cycle(tb, 232, 3277, function);
  cycle(tb, 4324, 24, function);
  cycle(tb, -1, 1, function);
  cycle(tb, -34, -2, function);

  function = 6;//sll
  cycle(tb, 1, 0, function);
  cycle(tb, 3, 4, function);
  cycle(tb, 423, 16, function);
  cycle(tb, 423, 31, function);
  cycle(tb, 2, 31, function);

  function = 7;//srl
  cycle(tb, 1, 0, function);
  cycle(tb, 3, 4, function);
  cycle(tb, 423, 16, function);
  cycle(tb, 423, 31, function);
  cycle(tb, 2, 31, function);

  function = 8;//sra
  cycle(tb, 1, 0, function);
  cycle(tb, -3, 4, function);
  cycle(tb, 423, 16, function);
  cycle(tb, -423, 31, function);
  cycle(tb, -2, 31, function);
  cycle(tb, -512, 2, function);

  function = 9;//sub
  cycle(tb, 1, 0, function);
  cycle(tb, 0, -1, function);
  cycle(tb, -1, -1, function);
  cycle(tb, 22553, -235522, function);
  cycle(tb, -23523, 23253, function);
  
  printf("test passed\n");
  exit(EXIT_SUCCESS);
}
