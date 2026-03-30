#pragma once
#include <stdio.h>
#include "pcb.h"
#include "queue.h"
#define MEM_SIZE 1000


#define PAGE_SIZE 3 //lines/page
#define MAX_PAGES 10

#define FRAME_STORE_START 0
#define VAR_STORE_START FRAME_STORE_SIZE

// 1.2.3 lru
extern long used_time; // time counter for frames
extern long least_recently_used[FRAME_STORE_SIZE / PAGE_SIZE]; // last used time for each frame


void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);


void assert_linememory_is_empty(void);
//size_t allocate_line(const char *line);
//void free_line(size_t index);
//const char *get_line(size_t index);
void reset_linememory_allocator(void);

int allocate_frame(void);
void set_frame_line(int frame, int offset, const char *line);
const char *get_frame_line(int frame, int offset);
void load_page(FILE *file, int page, int frame);
int victim_frame(void);
void update_table_evicted_frame(struct queue *q, int evicted_frame, struct PCB *current_process);
void page_faults_handler(struct PCB *pcb, struct queue *ready_queue);
int frame_store_is_full(void);
