//Abid Bakhtiyar
//890459241
//Project 2: Virtual Memory
//CPSC 351
//McCarthy

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

int positive(int a, int b) //Makes sure the Cache size is positive
{
    if(a>b)
    return a;
    return b;
}

#define TBUFF_SIZE 16 //Size of the cache
#define P_FRAMES 128 //nuber of page frames, change this to 128 if you want to translate out of 128 bits
#define P_MASK 127 // change to 127
#define P_SIZE 256
#define O_BITS 8 // number of bits for the offset
#define O_MASK 255 

#define MEM_SIZE P_FRAMES * P_SIZE

#define BUFLEN 256 

#define ARGC_ERROR 1
#define FILE_ERROR 1

struct TLB{
    unsigned char virt_address;
    unsigned char phys_address;
};

struct TLB tlb[TBUFF_SIZE]; //struct array for the cache is the size of the cache 
int pgTable[P_FRAMES]; //physical page number for the virtual address
signed char memory [MEM_SIZE];

int tlb_num = 0;//Index for running through the Cache Buffer

//pointer for backing file
signed char *back_store;

//translate virtual page to physical page
int translate(unsigned char virt_page){
    for(int i = positive((tlb_num - TBUFF_SIZE), 0); i < tlb_num; i++)
    {
        struct TLB *entry = &tlb[i % TBUFF_SIZE];
        if (entry -> virt_address == virt_page)
        {
            return entry -> phys_address;
        }
    }
    return -1;
}
int getOffset(int x)
{
    return x & O_MASK;
}
int getPage(int x)
{
    return (x >> O_BITS) & P_MASK;
}

//Fifo replacement
void fifo_add(unsigned char virt, unsigned char phys){
    struct TLB *entry = &tlb[tlb_num % TBUFF_SIZE];
    tlb_num++;
    entry -> virt_address = virt;
    entry -> phys_address = phys;
}

//Prints all stats
void print(int a, int p, int t)
{
    printf("------------------\n");
    printf("    Statistics    \n");
    printf("------------------\n");
    printf("Page Faults Rate = %.3f\n", p/(1. * a));
    printf("TLB Hit Rate = %.3f\n", t/(1. * a));
}
int main(int argc, const char*argv[])
{

    if(argc != 2){
        fprintf(stderr, "Please run code with ./virt_mem addresses.txt");
        exit(ARGC_ERROR);
    }
    char buff[BUFLEN]; //Buffer array
    int addCounter = 0; //Counts the number of addresses
    int tlbHits = 0; //Counts the number fo TLB Hits
    int pgFaults = 0; //Counts the number of Page Faults
    unsigned char free_pg = 0; //next unallocated page in memory

    //Opens Address File
    const char *filename = argv[1];
    FILE *fp = fopen(filename, "r");
    if(fp == NULL)
    {
        fprintf(stderr, "Could not open file: \'%s \' \n", filename);
        exit(FILE_ERROR);
    }

    //Opens Backing Store File
    int backing = open("BACKING_STORE.bin", O_RDONLY);
    back_store = mmap(0, MEM_SIZE, PROT_READ, MAP_PRIVATE, backing, 0);

    //fill page table with -1 if the table is empty
    for(int i = 0; i < P_FRAMES; i++)
    {
        pgTable[i] = -1;
    }

    //RUns through each address and gets the page and offset and translates it
    while(fgets(buff, BUFLEN, fp) != NULL)
    {
        addCounter++;
        int virtAdd = atoi(buff);
        int offset = getOffset(virtAdd);//Find the offset
        int virtPage = getPage(virtAdd);//Finds the virtual page

        int physPage = translate(virtPage); //Translates the Virtual Page into a Physical Page

        //Check for TLB hit or miss

        if(physPage != -1)
        {
            tlbHits++; //Hit
        }
        else 
        {
            physPage = pgTable[virtPage]; //Miss
            //Check for page fault

            if(physPage == -1)
            {
                pgFaults++;
                physPage = free_pg;
                free_pg++;

                //copy the page from the backing file into the phys memory
                memcpy(memory + physPage * P_SIZE, back_store + virtPage * P_SIZE, P_SIZE);
                pgTable[virtPage] = physPage;
            }
            fifo_add(virtPage, physPage);
        }

        int physAdd = (physPage << O_BITS) | offset; //Physical Address
        signed char value = memory[physPage * P_SIZE + offset];//Value
        printf("Virtual address: %d Physical Address: %d Value: %d \n", virtAdd, physAdd, value);
        


    }
    print(addCounter, pgFaults, tlbHits);//Function will Print stats
    return 0;
}

















