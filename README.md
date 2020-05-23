# README

## Introduction and Features
Our program implements an algorithm that translates virtual address to physical addresses using two-level page tables and a TLB that implements the FIFO policy for page replacement. As well as the LRU for TLB replacement.

Virtual addresses which can be used for testing are provided in virtual.txt in bin folder.

## Setup
To execute our program, open a terminal to grant accessibility. To compile and starts the program, enter the following commands in your terminal:
```
$ make
$ ./vmanager
```
or
```
$ ./vmanager -lru
```
Press enter and you should be able to see the result.

## Bugs
None
