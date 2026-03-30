#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset
#include "shell.h" // MAX_USER_INPUT
#include "shellmemory.h"
#include "pcb.h"

int pcb_has_next_instruction(struct PCB *pcb) {
    return pcb->pc_page * PAGE_SIZE + pcb-> pc_offset < pcb->line_count; // if pcb has no more lines, we are done
}

// returns memory line string (not index)
const char *pcb_next_instruction(struct PCB *pcb) {
    int page = pcb->pc_page;
    int offset = pcb->pc_offset;

    int frame = pcb->page_table[page];
    if (frame == -1) return NULL; // page fault

    // update time for LRU
    least_recently_used[frame] = ++used_time;

    // get instruction
    const char *inst = get_frame_line(frame, offset);

    pcb->pc_offset++;
    if (pcb->pc_offset >= PAGE_SIZE) {
        pcb->pc_page++;
        pcb->pc_offset = 0;
    }
    
    return inst;
}

struct PCB *create_process(const char *filename) {
    FILE *script = fopen(filename, "rt");
    if (!script) {
        perror("failed to open file for create_process");
        return NULL;
    }
    struct PCB *pcb = create_process_from_FILE(script);
    pcb->name = strdup(filename);
    return pcb;
}


struct PCB *create_process_from_FILE(FILE *script) {
    struct PCB *pcb = malloc(sizeof(struct PCB));
    static pid fresh_pid = 1;
    pcb->pid = fresh_pid++;
    pcb->name = "";
    pcb->next = NULL;
    pcb->line_count = 0;
    pcb->num_pages = 0; //changed from line_base to num_pages

    pcb->pc_page = 0; // start at first page
    pcb->pc_offset = 0; // start at first line of page

    // init page table to invalid
    for (int i = 0; i < MAX_PAGES; i++) pcb->page_table[i] = -1;

    char linebuf[MAX_USER_INPUT];

    // first count the lines in the program
    while (fgets(linebuf, MAX_USER_INPUT, script)) pcb->line_count++;

    // calculate how many pages we need
    pcb->num_pages = (pcb->line_count + PAGE_SIZE - 1) / PAGE_SIZE;

    // restart to actually read the program
    rewind(script);

    // load first 2 pages into memory
    for (int page = 0; page < pcb->num_pages && page < 2; page++) {
        int frame = allocate_frame();
        if (frame == -1) {
            fprintf(stderr, "failed to allocate frame for page %d\n", page);
            break;
        }
        pcb->page_table[page] = frame;
        load_page(script, page, frame);
    }
    

    fclose(script);
    pcb->duration = pcb->line_count;
    return pcb;
}
// for now we don't free frames 
void free_pcb(struct PCB *pcb) {
    //for (size_t ix = pcb->line_base; ix < pcb->line_base + pcb->line_count; ++ix) {
    //    free_line(ix);
    //} removed since we don't loop over free_line anymore
    if (strcmp("", pcb->name)) {
        free(pcb->name);
    }
    free(pcb);
}
