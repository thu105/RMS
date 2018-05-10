# RMS

## Created by:
  Hein Thu
## Description:
This project simulates a Rate Monotonic Scheduler (RMS) by using its own scheduler thread on top of SCHED_FIFO. Four threads with different time period (thus different priority) are running on one single core and are handled by the scheduler. The scheduler will dispatch a thread even if there is a deadline miss to not allow any recovery in case of a overrun. 3 cases are tested to see how the scheduler will behave in different scenarios.

## Note:
  * This file was built and tested on a Linux OS.
  * Processor affinity is set to core 3. Please change core number in source file if your have less than 4 cores.
  * Uses c++11 and pthread library so please compile with -lpthread and -std=c++11, or just use Makefile.

## Instruction: 
  * make all //compile the project
  * make run //to run all the test cases
  * make run2 //run all test cases with minimal output text
  * make clean // clean up the executable
