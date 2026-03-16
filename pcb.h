#pragma once
#include <stddef.h>
#include <stdio.h> 

typedef size_t pid;

#define PAGE_SIZE 3
#define MAX_PAGES 10

struct PCB {
    pid pid;
    char *name;
    //size_t line_base; removed since page table replaces it
    size_t line_count;
    size_t duration;
    size_t pc;
    int page_table[MAX_PAGES]; 
    int num_pages;
    struct PCB *next;
};


int pcb_has_next_instruction(struct PCB *pcb);
const char *pcb_next_instruction(struct PCB *pcb); //changed from size_t to const char
struct PCB *create_process(const char *filename);
struct PCB *create_process_from_FILE(FILE *f);
void free_pcb(struct PCB *pcb);

