#include <stdlib.h>
#include "Vregister_file.h"
#include "verilated.h"

uint32_t register_file[32];
uint32_t current_reg_1 = 0;
uint32_t current_reg_2 = 0;

uint32_t verification_reg_1 = 0;
uint32_t verification_reg_2 = 0;

void cycle(Vregister_file *tb, bool reset, bool rd_1_en, bool rd_2_en, uint8_t read_reg1, uint8_t read_reg2, uint8_t write_reg, bool write_en, uint32_t write_data){
  tb->clk = 0;
  tb->eval();
  tb->reset = reset;
  tb->rd_1_en = rd_1_en;
  tb->rd_2_en = rd_2_en;
  tb->read_reg_1 = read_reg1;
  tb->read_reg_2 = read_reg2;
  tb->write_reg = write_reg;
  tb->write_en = write_en;
  tb->write_data = write_data;
  tb->clk = 1;
  tb->eval();

  //debug output
  // printf("reset:%d, rd_1_en:%d, rd_2_en:%d, read_reg1:%d, read_reg2:%d, write_reg:%d, write_en:%d, write_data:%d, data_out_1:%d, data_out_2:%d\n",
  //  reset, rd_1_en, rd_2_en, read_reg1, read_reg2, write_reg, write_en, write_data, tb->data_out_1, tb->data_out_2);

  if(write_en && write_reg != 0){
    register_file[write_reg] = write_data;
  }
  tb->eval();

  if(reset){
    for(int i = 0; i < 32; i++){
      register_file[i] = 0;
    }
  }

  if(reset == 1)
  {

  }
  else if(rd_1_en == 1 && current_reg_1 != read_reg1)
  {
     current_reg_1 = read_reg1;
     verification_reg_1 = register_file[current_reg_1];
  }
  else if( tb->data_out_1 == verification_reg_1 ){
  }
  else{
    printf("fail 1\n");
    printf("read reg %d with val %d\n", current_reg_1, tb->data_out_1);
    printf("val should be: %d\n", verification_reg_1);
    exit(1);
  }

  if(reset == 1)
  {

  }
  else if(rd_2_en == 1 && current_reg_2 != read_reg2)
  {
     current_reg_2 = read_reg2;
     verification_reg_2 = register_file[current_reg_2];
  }
  else if( tb->data_out_2 == verification_reg_2 ){
  }
  else{
    printf("fail 2\n");
    printf("read reg %d with val %d\n", current_reg_2, tb->data_out_2);
    printf("val should be: %d\n", verification_reg_2);
    exit(1);
  }

}

int main(int argc, char **argv) {
	// Initialize Verilators variables
	Verilated::commandArgs(argc, argv);

	// Create an instance of our module under test
	Vregister_file *tb = new Vregister_file;

  cycle(tb, 1, 0, 0, 0, 0, 0, 0, 0);

  //write 123 to reg1
  cycle(tb, 0, 0, 0, 0, 0, 1, 1, 123);

  //write 345 to reg2
  cycle(tb, 0, 0, 0, 0, 0, 2, 1, 345);

  //read reg1
  cycle(tb, 0, 1, 0, 1, 0, 0, 0, 0);
  cycle(tb, 0, 0, 0, 0, 0, 0, 0, 0);
  //read reg2
  cycle(tb, 0, 1, 0, 2, 0, 0, 0, 0);
  cycle(tb, 0, 0, 0, 0, 0, 0, 0, 0);
  //read both
  cycle(tb, 0, 1, 1, 1, 2, 0, 0, 0);
  cycle(tb, 0, 0, 0, 0, 0, 0, 0, 0);

  //edit reg1
  cycle(tb, 0, 0, 0, 0, 2, 1, 1, 888);
  cycle(tb, 0, 0, 0, 0, 0, 0, 0, 0);

  //try to edit reg0 (should be ignored)
  cycle(tb, 0, 0, 0, 1, 2, 0, 1, 111);

  //read reg0
  cycle(tb, 0, 1, 1, 0, 0, 0, 0, 0);
  cycle(tb, 0, 0, 0, 0, 0, 0, 0, 0);

  printf("test passed\n");
  exit(EXIT_SUCCESS);
}
