//Implement the API modeling the translation of virtual page address to a
//physical frame address. We assume a 32 bit virtual memory and physical memory.
//Access to the page table is only via the CR3 register.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "vmemory.h"

#define OUT_TLB "../bin/tlb_out.txt"
#define VIR_ADDR 32

bool FIFO_policy = true;
int **cr3;


//create TLB
int hit_count = 0;
int total_tlb_lookup = 0;
char tlb[8][VIR_ADDR];
int tlb_index;

void tlb_init(){
	tlb_index = 0;
	for(int i = 0; i < 8; i++){
		strcpy(tlb[i], "-1 -1");
	}
}
//
// More static functions can be implemented here
//
void sub_str(char vir_addr[], char subset[], int start, int end){
	int i;
	for(i = 0; i < (end-start); i++){
		subset[i] = vir_addr[start + i];
	}
	subset[i] = '\0';
}

//void bi_to_hex(char hex[VIR_ADDR], char bi[VIR_ADDR]){

//}

void int_to_bi(char bi[VIR_ADDR], long vir_addr){
	long copy = vir_addr;
	int binary[VIR_ADDR];

	int i = 0;
	while (copy != 0) {
			binary[i] = copy % 2;
			copy = copy / 2;
			i++;
	}

	int k = 0;
	// store reversed binary into char bi
	for (int j = i - 1; j >= 0; j--){
			bi[k] = '0' + binary[j];
			//printf("%d", binaryNum[j]);
			k++;
	}
	//end with null terminator
	bi[k] = '\0';
}


// The implementation of get_vpage_cr3 is provided in
// an object file, so no need to re-implement it
void initialize_vmanager(int policy)
{
	// Set LRU policy when passsed as a parameter
	if (policy)
		FIFO_policy = false;
	cr3 = get_vpage_cr3();
	//printf("cr3: %p\n", cr3);
}

//
// The implementation of following functions is required
//
int translate_virtual_address(unsigned long v_addr, int lru)
{	//convert from decimal to binary
	char bi_vir_addr[VIR_ADDR];
	int_to_bi(bi_vir_addr, v_addr);

  //split each addr into three parts
	int length = strlen(bi_vir_addr);

	char pages[20];
	char offset_[12];
	//following three variables stores the splitted page index and offset
	char first_level[12];
	char second_level[12];
	char offset[12];

	//subset index and offset from binary virtual address respectively
	sub_str(bi_vir_addr, offset_, (length-12), length);
	sub_str(bi_vir_addr, pages, 0, (length-12));
	int unsigned page_num = strtol(pages, NULL, 2);

	sub_str(pages, second_level, (strlen(pages)-10), (strlen(pages)));
	sub_str(pages, first_level, 0, (strlen(pages)-10));
	sub_str(offset_, offset, 0, 12);

	//convert first and second page index into decimal
	int unsigned first_index = strtol(first_level, NULL, 2);
	int unsigned second_index = strtol(second_level, NULL, 2);
	int unsigned _offset = strtol(offset, NULL, 2);

	int check_tlb = get_tlb_entry(page_num);
  	total_tlb_lookup++;
	//if TLB hit
	if(check_tlb != -1){
		//skip the page table look up
		hit_count++;
		if (lru > 0){
			populate_tlb_LRU(page_num, check_tlb);
		}
		print_physical_address(check_tlb, _offset);
		return 0;
	}
	//page lookup if address not in tlb
	//check if the address if valid
	if(first_index != 0){
		if(*(cr3 + first_index) != NULL){
			int physical_frame_num = *(*(cr3 + first_index) + second_index);
			if(physical_frame_num != -1){
				//if frame base address is valid
				populate_tlb(page_num, physical_frame_num);
				//get the converted physical address and print it in hexdecimal format
				print_physical_address(physical_frame_num, _offset);
				return 0;
			}else{
				//a page fault is encountered
				printf("INVALID ADDRESS\n");
				return -1;
			}
		}else{
			//a page fault is encountered
			printf("INVALID ADDRESS\n");
			return -1;
		}
	}
}

void print_physical_address(int frame, int offset)
{
	//convert physical frame number and offset to binary format
	char bi_phy_frame[VIR_ADDR];
	char bi_offset[VIR_ADDR];
	int_to_bi(bi_phy_frame, (long)frame);
	int_to_bi(bi_offset, (long)offset);

	//append offset to get physical address
	char physical_addr[VIR_ADDR];
	strcpy(physical_addr, bi_phy_frame);
	strcat(physical_addr, bi_offset); //converted physical address stored

	//convert physical address to hexdecimal format
	char to_print[VIR_ADDR];
	int unsigned temp = strtol(physical_addr, NULL, 2);
	sprintf(to_print, "%0x", temp);

	//format output
	int l = strlen(to_print);
	char zero[VIR_ADDR] = "0";
	//append 0 if address less than 8 digits
	if(l < 8){
		for(int i = 0; i < (8-l); i++){
			strcat(zero, to_print);
			strcpy(to_print, zero);
			strcpy(zero, "0");
		}
	}
	//add 0x at the front
	char formated[VIR_ADDR];
	strcpy(formated, "0x");
	strcat(formated, to_print);
	printf("%s\n",formated); //print formated physical address
	return;
}

int get_tlb_entry(int n)
{
	//convert to hexdecimal format
	char hex_[VIR_ADDR];
	sprintf(hex_, "%0x", n);

	//add 0x at the front
	char hex[VIR_ADDR];
	strcpy(hex, "0x");
	strcat(hex, hex_);

	//search in tlb
	for(int i = 0; i < 8; i++){
		//if virtual address already in tlb
		if(strncmp(hex, tlb[i], 7) == 0){
			char found_physical[VIR_ADDR];
			sub_str(tlb[i], found_physical, 8, 15);
			//return corresponding physical address in decimal format
			int unsigned phy_addr = strtoul(found_physical, NULL, 16);
			return phy_addr;
		}
	}

	//TLB miss
	return -1;
}

void populate_tlb(int v_addr, int p_addr)
{
	print_tlb();
	//convert to hexdecimal format
	char v_[VIR_ADDR];
	sprintf(v_, "%0x", v_addr);
	char v[VIR_ADDR];
	strcpy(v, "0x");
	strcat(v, v_);

	char p_[VIR_ADDR];
	sprintf(p_, "%0x", p_addr);
	char p[VIR_ADDR];
	strcpy(p, "0x");
	strcat(p, p_);

	strcat(v, " ");
	strcat(v, p);

	//if tlb is full
	if(tlb_index == 8){
		for(int i = 0; i < 7; i++){
			strcpy(tlb[i], tlb[i+1]);
		}tlb_index-=1;
	}

	strcpy(tlb[tlb_index], v);
	tlb_index++;

	/*
	for(int i = 0; i < 8; i++){
		printf("%s\n", tlb[i]);
	}*/
	return;
}

void populate_tlb_LRU(int v_addr, int p_addr)
{
	print_tlb();
	//convert to hexdecimal format
	char v_[VIR_ADDR];
	sprintf(v_, "%0x", v_addr);
	char v[VIR_ADDR];
	strcpy(v, "0x");
	strcat(v, v_);

	char p_[VIR_ADDR];
	sprintf(p_, "%0x", p_addr);
	char p[VIR_ADDR];
	strcpy(p, "0x");
	strcat(p, p_);

	strcat(v, " ");
	strcat(v, p);
	
	if(tlb_index == 8){
		char temp[32];
		int n;
		for(int i = 0; i < 8; i++){
			if (strcmp(tlb[i], v) == 0){
				strcpy(temp, v);
				n = i;
				break;
			}
		}

		while (n < 7){
			strcpy(tlb[n], tlb[n+1]);
			n++;
		}
		strcpy(tlb[7], temp);
	}
		
	/*
	for(int i = 0; i < 8; i++){
		printf("%s\n", tlb[i]);
	}*/
	return;
}

float get_hit_ratio()
{
	//TODO
	float r = (float)hit_count / (float)total_tlb_lookup;
	return r;
}

//Write to the file in OUT_TLB
void print_tlb()
{
	//TODO
	FILE *fp = fopen(OUT_TLB, "a");
	if(fp == NULL){
		printf("%s\n", "ERROR OPENING tlb_out.txt");
		exit(1);
	}

	for(int i = 0; i < 8; i++){
		//write tlb contents to tlb_out.txt file
		fprintf(fp, "%s\n", tlb[i]);
		// printf("%s\n", tlb[i]);
	}
	//print a new line to separate each call
	fprintf(fp, "\n");

	fclose(fp);
	return;
}
