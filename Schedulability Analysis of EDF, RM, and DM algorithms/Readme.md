                                               			README

						ABOUT:
						-----
->This program will check the different scheduling algorithms for real time task sets. (EDF,RM and DM)

->EDF stands for Earliest deadline first scheduling, RM is rate monotonic (priority according to the period) and DM stands for Deadline Monotonic (priority given to the deadline).

->Based upon the task-sets, the conclusion of whether the tasksets are schedulable or not is given.

->In the next part, synthetic task sets are generated and a comparative analysis is made for the various types of scheduling algorithms.




----------------------------------------------------------------------------------------------------------------------------------------

					 INCLUDED IN REPOSITORY:
					 ----------------------

->  C programs: analysis.c
->  Inputfile: Inputfile.txt
->  Report: Report.pdf
->  Readme file: Readme.txt

-----------------------------------------------------------------------------------------------------------------------------------------

				   SYSTEM REQUIREMENTS:
				   -------------------

-> Linux CPU for host. Compilation must be done in host and object code is copied and run in Galileo board.

-> LINUX KERNEL : Minimum version of 2.6.19 is expected.

-> SDK: iot-devkit-glibc-x86_64-image-full-i586-toolchain-1.7.2

-> GCC: Minimum version of 4.8 is required to run -pthread option while compiling.



-----------------------------------------------------------------------------------------------------------------------------------------

				     SYSTEM:
				     -------

-> If your SDK is installed in other location than default one, change the path accordingly in makefile.

-> Open 2 terminal windows, one will be used for host commands and other will be used to send command to Galileo board.

-> To compile this code, type make all. This will compile the given source code on the host Linux machine.
 
-> To run and execute this code on one CPU, type in sudo taskset 0x01 ./main

------------------------------------------------------------------------------------------------------------------------------------------------------

					    EXECUTION:
					    ----------

-> The input task-set is read from the inputfile.txt. First the user is prompted to enter the option of whether he wants to do a schedulability analysis for the tasks or do a comparative analysis.

->In case Wrong Option Selected is displayed, you need to recompile the code.

->It is necessary that the deadline be less than the period for the code to work. 

->For EDF analyis, the utilization analysis is the first step. If the summation of e/p is less than one, the task set is schedulable. If this condition is not satisfied, we still need to make sure that it is not schedulable. So we do a loading factor analysis which will determine if the task set is schedulable or not. (If not, then at what instant of deadline)

->For RM and DM, utilization bound test is the first step which will determine the schedulability. However, if it is inconclusive, we need to do time demand analysis or response time analysis.


---------------------------------------------------------------------------------------------------------------------------------------------------------
*******************************************************************************************************************************************************
