Author: Aditya Thakkar
Student ID: 2021132636

Compilation and running instructions

Development Language: C++
Development Platform: Ubuntu 12.10

Compulation Dependencies:
- Need GCC Compiler

Files:
There are 4 files node.h, node.cc, controller.cc, and Makefile

** Use the scenarios.sh created for testing if necessary. 

Compile Commands:
1. make clean
2. make all

This will create 2 executable files node and controller

To run the controller use:
./controller

To run a node use:
./node <args>

**************************************************************
* Important Instructions for Running scenarios consecutive times
**************************************************************
After running 1 scenario do following:
1. Kill Controller, commands:
   ps
   kill -9 <controller process ID>
   
2. Remove output and input files
   rm output_*
   rm input_*
   
3. Remove Receive File
   rm ID_received_from_ID

***************************************************************
*Topology file must be named as topology without any extension*
***************************************************************


**************************************************************
* Scenarios Result on Ubuntu 12.10
**************************************************************
- All scenarios 1-4 run successfully 
- Data travels on the correct path
