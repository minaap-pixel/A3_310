#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset
#include "shell.h" // MAX_USER_INPUT
#include "shellmemory.h"
#include "pcb.h"

int pcb_has_next_instruction(struct PCB *pcb) {
    return pcb->pc < pcb->line_count;
}

// returns memory line string (not index)
const char *pcb_next_instruction(struct PCB *pcb) {
    int page = pcb->pc / PAGE_SIZE;
    int offset = pcb->pc % PAGE_SIZE; //use modulus for cyclical arithmetic
    pcb->pc++;

    int frame = pcb->page_table[page];
    if (frame == -1) return NULL; // page fault 
    return get_frame_line (frame, offset);
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
    pcb->pc = 0;
    pcb->line_count = 0;
    pcb->num_pages = 0; //changed from line_base to num_pages

    // init page table to invalid
    for (int i = 0; i < MAX_PAGES; i++)
        pcb->page_table[i] = -1;

    char linebuf[MAX_USER_INPUT];
    int current_frame = -1;
    int offset_in_frame = 0;

    while (fgets(linebuf, MAX_USER_INPUT, script)) {
        // start new frame every PAGE_SIZE lines
        if (offset_in_frame == 0) {
            current_frame = allocate_frame();
            if (current_frame == -1) {
                free_pcb(pcb);
                return NULL; // out of frame memory (when = -1)
            }
            pcb->page_table[pcb->num_pages] = current_frame;
            pcb->num_pages++;
        }

        set_frame_line(current_frame, offset_in_frame, linebuf);
        offset_in_frame = (offset_in_frame + 1) % PAGE_SIZE;
        pcb->line_count++;
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
