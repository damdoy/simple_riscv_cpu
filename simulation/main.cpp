#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <string>
#include <fstream>
#include <vector>
#include <elf.h>
#include "Vsimple_cpu.h"
#include "verilated.h"

#define MAX_CYCLES 2048

void eval_and_update_cpu(Vsimple_cpu *tb);
void init_simple_add();
void read_elf(char *filename, uint8_t *data, uint32_t *size, uint32_t *start_addr);
void fill_memory(uint8_t *data, uint32_t size, uint32_t start_addr);
void init_load_elf(uint8_t *elf, uint32_t size, uint32_t addr_start);

uint8_t *memory;
uint32_t *memory32;

union Instruction{
   struct Bits_I_type{
      uint32_t opcode:7;
      uint32_t rd:5;
      uint32_t funct3:3;
      uint32_t rs1:5;
      uint32_t imm:12;
   };
   struct Bits_R_type{
      uint32_t opcode:7;
      uint32_t rd:5;
      uint32_t funct3:3;
      uint32_t rs1:5;
      uint32_t rs2:5;
      uint32_t funct7:7;
   };
   struct Bits_S_type{
      uint32_t opcode:7;
      uint32_t imm5:5;
      uint32_t funct3:3;
      uint32_t rs1:5;
      uint32_t rs2:5;
      uint32_t imm7:7;
   };
   Bits_I_type bits_i;
   Bits_R_type bits_r;
   Bits_S_type bits_s;
   uint32_t word;
};

int main(int argc, char **argv) {
   // Initialize Verilators variables
   Verilated::commandArgs(argc, argv);

   // Create an instance of our module under test
   Vsimple_cpu *tb = new Vsimple_cpu;

   memory = new uint8_t[1<<15]; //64k memory
   memory32 = (uint32_t*) memory;

   if(argc < 3)
   {
      printf("usage: Vsimple_cpu --isa=rv32i +signature=I-IO.signature.output file.elf\n");
      return 0;
   }


   std::string signature_path = argv[2];
   signature_path.erase(0, strlen("+signature="));

   std::ofstream signature_file;
   fprintf(stderr, "writing to %s\n", signature_path.c_str());
   signature_file.open(signature_path.c_str(), std::ofstream::out | std::ofstream::trunc);
   //signature_file << "hello.\n";

   std::string elf_path = argv[3];
   std::ifstream input_file( elf_path.c_str(), std::ios::binary );

   std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input_file), {});

   uint32_t elf_file_size = buffer.size();
   uint8_t *elf_array8 = &buffer[0];
   uint32_t *elf_array32 = (uint32_t *)elf_array8;

   Elf32_Ehdr *elf_header;

   elf_header = (Elf32_Ehdr*)((void*)elf_array8);

   std::cerr << "value of array8: " << std::hex << (int)elf_array8[0] << std::endl;
   std::cerr << "value of array32: " << std::hex << elf_array32[0] << std::endl;
   std::cerr << "elf_entry: " << std::hex << elf_header->e_entry << std::endl;
   std::cerr << "start of program header table: 0x" << std::hex << elf_header->e_phoff << std::endl;
   std::cerr << "start of section header table: 0x" << std::hex << elf_header->e_shoff << std::endl;
   std::cerr << "size of program header table: 0x" << std::hex << elf_header->e_phentsize << std::endl;
   std::cerr << "number of program header table: 0x" << std::hex << elf_header->e_phnum << std::endl;
   std::cerr << "number of section header table: 0x" << std::hex << elf_header->e_shnum << std::endl;
   std::cerr << "e_shstrndx: 0x" << std::hex << elf_header->e_shstrndx << std::endl;

   std::cerr << "Reading first program header" << std::endl;
   Elf32_Phdr *elf_pheader;
   elf_pheader = (Elf32_Phdr*)((void*)elf_array8+elf_header->e_phoff);

   std::cerr << "offset: 0x" << std::hex << elf_pheader->p_offset << std::endl;
   std::cerr << "vaddr: 0x" << std::hex << elf_pheader->p_vaddr << std::endl;
   std::cerr << "p_filesz: 0x" << std::hex << elf_pheader->p_filesz << std::endl;

   Elf32_Shdr *elf_sheaders;
   elf_sheaders = (Elf32_Shdr*)((void*)elf_array8+elf_header->e_shoff);

   uint32_t symtab_offset;
   uint32_t symtab_header_idx;
   uint32_t strtab_offset;
   uint32_t strtab_header_idx;
   Elf32_Sym *elf_symtable;

   //searching for the symbol table
   std::cerr << "******************\nsearching for symbol table\n****************" << std::endl;
   for (size_t i = 0; i < elf_header->e_shnum; i++) {
      std::cerr << "Reading section header nb " << i << std::endl;
      std::cerr << "type of section: " << elf_sheaders[i].sh_type << std::endl;
      //TODO: find name .strtab which should be in .shstrtab (given in elf_header->e_shstrndx of main header)
      uint32_t shstrtab_offset = elf_sheaders[elf_header->e_shstrndx].sh_offset;
      std::cerr << "offset of shstrtab: " << shstrtab_offset << std::endl; //This section holds section names.  This section is of type SHT_STRTAB.  No attribute types are used


      std::cerr << "name of the symbol: " << (char*)(elf_array8+shstrtab_offset+elf_sheaders[i].sh_name) << std::endl;
      if(strcmp( (char*)(elf_array8+shstrtab_offset+elf_sheaders[i].sh_name), ".symtab") == 0)
      {
         symtab_offset = elf_sheaders[i].sh_offset;
         symtab_header_idx = i;
         std::cerr << "found symtab at address: " << symtab_offset << std::endl;
         elf_symtable = (Elf32_Sym*)((void*)elf_array8+elf_sheaders[i].sh_offset);
         // std::cerr << "name at: " << elf_sheaders[j].sh_name << std::endl;
      }
      if(strcmp( (char*)(elf_array8+shstrtab_offset+elf_sheaders[i].sh_name), ".strtab") == 0)
      {
         strtab_offset = elf_sheaders[i].sh_offset;
         strtab_header_idx = i;
         std::cerr << "found strtab at address: " << strtab_offset << std::endl;
         // std::cerr << "name at: " << elf_sheaders[j].sh_name << std::endl;
      }


      // if(elf_sheaders[i].sh_type == SHT_SYMTAB ) //symbol table found
      // {
      //    std::cerr << "This section is a symtab!" << std::endl;
      //    std::cerr << "offset: " << elf_sheaders[i].sh_offset << std::endl;
      //    std::cerr << "size: " << elf_sheaders[i].sh_size << std::endl;
      //    std::cerr << "number of symbols in the table: " << elf_sheaders[i].sh_size/sizeof(Elf32_Sym) << std::endl;
      //
      //    uint32_t offset_strtab = 0;
      //    uint32_t nb_symbols = elf_sheaders[i].sh_size/sizeof(Elf32_Sym);
      //
      //    //searching the strtab
      //    for (size_t j = 0; j < nb_symbols; j++) {
      //       std::cerr << "name of the symbol: " << (char*)(elf_array8+shstrtab_offset+elf_sheaders[j].sh_name) << std::endl;
      //       if(strcmp( (char*)(elf_array8+shstrtab_offset+elf_sheaders[j].sh_name), ".strtab") == 0)
      //       {
      //          offset_strtab = elf_sheaders[j].sh_offset;
      //          std::cerr << "found strtab at address: " << offset_strtab << std::endl;
      //          // std::cerr << "name at: " << elf_sheaders[j].sh_name << std::endl;
      //       }
      //    }
      //
      //    Elf32_Sym *elf_symtable;
      //    elf_symtable = (Elf32_Sym*)((void*)elf_array8+elf_sheaders[i].sh_offset);
      //
      //    for (size_t j = 0; j < nb_symbols; j++) {
      //       std::cerr << "symbol number: " << j << std::endl;
      //       std::cerr << "symbol name addr: " << elf_symtable[j].st_name << std::endl;
      //       std::cerr << "symbol name: " << (char*)(&elf_array8[offset_strtab+elf_symtable[j].st_name]) << std::endl;
      //    }

         // Elf32_Sym
         // typedef struct {
         //     uint32_t      st_name;
         //     Elf32_Addr    st_value;
         //     uint32_t      st_size;
         //     unsigned char st_info;
         //     unsigned char st_other;
         //     uint16_t      st_shndx;
         // } Elf32_Sym;
   }

   uint32_t nb_symbols = elf_sheaders[symtab_header_idx].sh_size/sizeof(Elf32_Sym);
   std::cerr << "number of symbols in the table: " << nb_symbols << std::endl;

   uint32_t begin_signature_addr;
   uint32_t end_signature_addr;

   for (size_t i = 0; i < nb_symbols; i++) {
      std::cerr << "symbol id: " << i << std::endl;
      char *symbol_name = (char*)(elf_array8+strtab_offset+elf_symtable[i].st_name);
      std::cerr << "symbol name: " << symbol_name << std::endl;
      if(strcmp( symbol_name, "begin_signature") == 0)
      {
         begin_signature_addr = elf_symtable[i].st_value;
         std::cerr << "found begin_signature at " << elf_symtable[i].st_value << std::endl;
      }
      else if(strcmp( symbol_name, "end_signature") == 0)
      {
         end_signature_addr = elf_symtable[i].st_value;
         std::cerr << "found end_signature at " << elf_symtable[i].st_value << std::endl;
      }
   }



   //init_simple_add();
   // init_add_test();
   init_load_elf(elf_array8, elf_file_size, elf_header->e_entry);

   tb->clk = 0;
   tb->reset = 1;
   tb->eval();

   tb->clk = 1;
   tb->eval();
   tb->reset = 0;
   printf("reset done\n");


   for( int i = 0; i < MAX_CYCLES; i++)
   {
      tb->clk = 1;
      tb->eval();

      tb->clk = 0;
      tb->eval();

      fprintf(stderr, "Cycle %d, debug: 0x%x\n", i, tb->debug);

      tb->read_data_valid = 0;
      tb->read_data = 0;

      eval_and_update_cpu(tb);
   }
   //old signature write when it was 16B per lines
   // for (size_t addr = begin_signature_addr; addr < end_signature_addr; addr+=16) {
   //    //signature file is like hexdump, msb on the left
   //    for (int i = 15; i >= 0; i--) {
   //       char output[3];
   //       sprintf(output, "%02x", memory[addr+i]);
   //       signature_file << output;
   //    }
   //    signature_file << std::endl;
   // }
   for (size_t addr = begin_signature_addr/4; addr < end_signature_addr/4; addr++) {
      char output[3];
      sprintf(output, "%08x", memory32[addr]);
      signature_file << output << std::endl;
   }

   signature_file.close();
   input_file.close();

   printf("test passed\n");
   exit(EXIT_SUCCESS);
}

void eval_and_update_cpu(Vsimple_cpu *tb){
  if(tb->read_req)
  {
   //  tb->read_data = memory32[(tb->read_addr&0xffff)/4];
    uint8_t *word = &memory[tb->read_addr&0xffff];
    uint16_t *word16 = ((uint16_t*)word);
    uint32_t *word32 = ((uint32_t*)word);
    if (tb->memory_mask == 0x1)
    {
      tb->read_data = *word;
    }
    else if (tb->memory_mask == 0x3)
    {
      tb->read_data = *word16;
    }
    else
    {
      tb->read_data = *word32;
    }

    tb->read_data_valid = 1;
    fprintf(stderr, "read_data addr:0x%x, value:0x%x, memory_mask:0x%x, error_instr:%x\n", tb->read_addr, tb->read_data, tb->memory_mask, tb->error_instruction);
  }
  else if(tb->write_req)
  {
    //memory32[(tb->write_addr&0xffff)/4] = tb->write_data;
    uint8_t *word = &memory[tb->write_addr&0xffff];
    uint16_t *word16 = ((uint16_t*)word);
    uint32_t *word32 = ((uint32_t*)word);
    if (tb->memory_mask == 0x1)
    {
      *word = tb->write_data;
    }
    else if (tb->memory_mask == 0x3)
    {
      *word16 = tb->write_data;
    }
    else
    {
      *word32 = tb->write_data;
    }


    fprintf(stderr, "write_data addr:0x%x, value:0x%x, memory_mask:0x%x, error_instr:%x\n", tb->write_addr, tb->write_data, tb->memory_mask, tb->error_instruction);
  }
}

void init_simple_add(){
  //lw $1, 0x100($0) 100 223  imm[11:0] rs1[19:15] "010"[14:12] rd[11:7] 0000011[6:0]
  Instruction ld;
  ld.bits_i.opcode = 0x3;
  ld.bits_i.rd = 0x1;
  ld.bits_i.funct3 = 0x2;
  ld.bits_i.rs1 = 0x0;
  ld.bits_i.imm = 0x100;
  memory32[0] = ld.word;
  //lw $2, 0x104($0)
  ld.bits_i.opcode = 0x3;
  ld.bits_i.rd = 0x2;
  ld.bits_i.funct3 = 0x2;
  ld.bits_i.rs1 = 0x0;
  ld.bits_i.imm = 0x104;
  memory32[1] = ld.word;
  //add $3, $1, $2 0000000 rs2 rs1 "000" rd 0110011
  Instruction add;
  add.bits_r.opcode = 0x33;
  add.bits_r.rd = 0x3;
  add.bits_r.funct3 = 0x0;
  add.bits_r.rs1 = 0x1;
  add.bits_r.rs2 = 0x2;
  add.bits_r.funct7 = 0x0;
  memory32[2] = add.word;
  //sw $3, 0x108($0) imm[11:5] rs2 rs1 "010" imm[4:0] 0100011
  Instruction sw;
  sw.bits_s.opcode = 0x23;
  sw.bits_s.imm5 = 0x8;
  sw.bits_s.funct3 = 0x2;
  sw.bits_s.rs1 = 0x0; //base
  sw.bits_s.rs2 = 0x3;
  sw.bits_s.imm7 = 0x8; //0x108 = 000100001000
  memory32[3] = sw.word;
  memory32[0x100/4] = 0x1111;
  memory32[0x104/4] = 0x2222;
}

void init_load_elf(uint8_t *elf, uint32_t size, uint32_t addr_start)
{
   for (size_t i = 0; i < size; i++) {
      memory[i] = elf[i];
   }

   uint32_t instr = addr_start << 12;

   //as the cpu starts at address 0, tells it to go to 0x2000
   //memory32[0] = 0x00002097; //auipc	ra,0x2
   memory32[0] = addr_start | 0x97;
   memory32[1] = 0x00008067; //jr ra (jalr x0, ra, 0)
}
