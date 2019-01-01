#include <stdlib.h>
#include "Vregister_file.h"
#include "verilated.h"

uint32_t register_file[32];

void cycle(Vregister_file *tb, bool reset, uint8_t read_reg1, uint8_t read_reg2, uint8_t write_reg, bool write_en, uint32_t write_data){
  tb->reset = reset;
  tb->read_reg_1 = read_reg1;
  tb->read_reg_2 = read_reg2;
  tb->write_reg = write_reg;
  tb->write_en = write_en;
  tb->write_data = write_data;
  tb->eval();

  if(write_en && write_reg != 0){
    register_file[write_reg] = write_data;
  }
  tb->eval();
  
  if(reset){
    for(int i = 0; i < 32; i++){
      register_file[i] = 0;
    }
  }

  if( (tb->data_out_1 == register_file[read_reg1] &&
     tb->data_out_2 == register_file[read_reg2]) || reset){
  }
  else{
    printf("fail\n");
    printf("read reg %d with val %d, reg %d with val %d\n", read_reg1, tb->data_out_1, read_reg2, tb->data_out_2);
    printf("vals should be: %d and %d\n", register_file[read_reg1], register_file[read_reg2]);
    exit(1);
  }
  
}

int main(int argc, char **argv) {
	// Initialize Verilators variables
	Verilated::commandArgs(argc, argv);

	// Create an instance of our module under test
	Vregister_file *tb = new Vregister_file;

  cycle(tb, 1, 0, 0, 0, 0, 0);
  
  //write 123 to reg1
  cycle(tb, 0, 0, 0, 1, 1, 123);

  //write 345 to reg2
  cycle(tb, 0, 0, 0, 2, 1, 345);

  //read reg1
  cycle(tb, 0, 1, 0, 0, 0, 0);

  //read reg2
  cycle(tb, 0, 2, 0, 0, 0, 0);

  //read both
  cycle(tb, 0, 1, 2, 0, 0, 0);

  //edit reg1
  cycle(tb, 0, 0, 2, 1, 1, 888);

  //read reg1 &2 and try to edit reg0 (should be ignored)
  cycle(tb, 0, 1, 2, 0, 1, 111);

  //read reg0
  cycle(tb, 0, 0, 0, 0, 0, 0);
  
  printf("test passed\n");
  exit(EXIT_SUCCESS);
}
