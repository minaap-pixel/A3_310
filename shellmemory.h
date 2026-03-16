#pragma once
#include <stdio.h>
#define MEM_SIZE 1000

#define FRAME_STORE_SIZE 18 //multiple of 3
#define VAR_STORE_SIZE 10
#define PAGE_SIZE 3 //lines/page
#define MAX_PAGES 10

#define FRAME_STORE_START 0
#define VAR_STORE_START FRAME_STORE_SIZE

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
