#!/bin/bash
set -e
#compile simple_cpu

if [ -d "obj_dir" ]; then
  rm -r obj_dir
fi

# to create the obj dir:
verilator -Wall --cc simple_cpu/simple_cpu.v simple_cpu/register_file/register_file.v simple_cpu/alu/alu.v --exe simulation/main.cpp

# to compile:
make -j -C obj_dir -f Vsimple_cpu.mk Vsimple_cpu

cp obj_dir/Vsimple_cpu riscv-compliance/.

# don't forget to add a riscv gcc compiler such as:
# PATH=$PATH:/opt/riscv/bin/

pushd riscv-compliance/
   make
popd
