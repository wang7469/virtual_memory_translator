//main.c program to translate the virtual addresses from the virtual.txt
//into the physical frame address. Project 3 - CSci 4061

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmemory.h"

#define INPUT_FILE "../bin/virtual.txt"
#define VIR_ADDR 33



int main(int argc, char* argv[])
{
	if (argc > 2) {
		printf("Too many arguments, enter up to one argument\n");
		exit(-1);
	}

	int policy = (argc == 2) ? 1:0;
	initialize_vmanager(policy);

	//TODO: Fill the algorithm by reading from INPUT_FILE
	FILE *fp = fopen(INPUT_FILE, "r");
	if(fp == NULL){
		perror("Failed to open INPUT_FILE");
		exit(1);
	}

	//initialize tlb entries to -1
	tlb_init();

	char addr[256][12];
	int i;
	int j;
  //read in virtual.txt into a 2D array
	for(i = 0; i < 256; i++){
		for(j = 0; j < 11; j++){
			fscanf(fp, "%c", &addr[i][j]);
		}
	}

	for(int i = 0; i < 256; i++){
		//convert from hex to decimal
	  long unsigned int_addr = strtoul(addr[i], NULL, 16);
		//if the end of the virtual address is not reached yet
		if(int_addr != 0){
			// execute LRU
			if (argc > 1 && strcmp(argv[1],"-lru") == 0){
				int result = translate_virtual_address(int_addr, 1);
			}
			else{
				int result = translate_virtual_address(int_addr, 0);
			}
			
		}
	}

	//virtual address translate finished
	//print out tlb hit rates
	printf("Hit rate of the cache is ");
	printf("%.6f\n", get_hit_ratio());
 	printf("\n");
	print_tlb();



	fclose(fp);
	//Free the page table
	free_resources();
	return 0;
}
