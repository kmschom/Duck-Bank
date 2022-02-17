# Duck-Bank
CIS 415 Project 3: Multi-threaded Duck Bank

Build a multithreaded bank to handle hundreds of thousands of requests from clients. 
Part 1 implements the bank and it's transactions as a single threaded solution.
Part 2 implements the use of multiple threads with critical section protection.
Part 3 implements the ability to coordinate the multiples threads so that account balances get updated faster.

Project Details in Project3.pdf

The creator of each file is noted at the top of the file along with it's date of creation.

bank.c: Contains the main function that processes the input arguments and files, creates the Duck bank accounts, and handles the bank and worker threads. This file also contains the functions that checks the password for each account transaction, updates the balance of each account, and processes transactions. The main function has the following input parameters: argc and argv. 
argc contains the number of input parameters for the main function. argv contains the file name containing all transactions to be processed.

account.h: A header file that contains account struct and transact struct used in bank.c functions.

string_parser.h: A header file that contains user-defined command_line struct and functions headers related to them.

string_parser.c: This file contains all the functions for the string_parser header file.
