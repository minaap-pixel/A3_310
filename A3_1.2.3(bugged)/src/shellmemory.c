#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "shellmemory.h"
#include "pcb.h"
#include "queue.h"


#define true 1
#define false 0


// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i])
            matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else
        return 0;
}



// for exec memory

struct program_line {
    int allocated; // for sanity-checking
    char *line;
};

struct program_line linememory[FRAME_STORE_SIZE]; //frame store only
int frame_used[FRAME_STORE_SIZE / PAGE_SIZE]; //will be 1 is frame is occupied
size_t next_free_frame = 0; //next free frame

// 1.2.3 LRU variables
long used_time = 0; // time counter for frames
long least_recently_used[FRAME_STORE_SIZE / PAGE_SIZE]; // last used time for each frame


void reset_linememory_allocator() {
    next_free_frame = 0; // changed to next_free_frame
    assert_linememory_is_empty();
}

void assert_linememory_is_empty() {
    for (size_t i = 0; i < MEM_SIZE; ++i) {
        assert(!linememory[i].allocated);
        assert(linememory[i].line == NULL);
    }
}

void init_linemem() {
    for (size_t i = 0; i < FRAME_STORE_SIZE; ++i) { //changed to use FRAME_STORE_SIZE
        linememory[i].allocated = false;
        linememory[i].line = NULL;
    }
}

size_t allocate_line(const char *line) {
    if (next_free_frame >= FRAME_STORE_SIZE) { // changed to fraes
        // out of memory!
        return (size_t)(-1);
    }
    size_t index = next_free_frame++;
    assert(!linememory[index].allocated);

    linememory[index].allocated = true;
    linememory[index].line = strdup(line);
    return index;
}

void free_line(size_t index) {
    free(linememory[index].line);
    linememory[index].allocated = false;
    linememory[index].line = NULL;
}

const char *get_line(size_t index) {
    assert(linememory[index].allocated);
    return linememory[index].line;
}


// Shell memory functions

struct memory_struct { // block or line
    char *var;
    char *value;
};

struct memory_struct shellmemory[MEM_SIZE];



void mem_init() {
    int i;
    for (i = 0; i < MEM_SIZE; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    return;
}

//get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return NULL;
}

// return frame number or -1 if out of memory
int allocate_frame() {
    if (next_free_frame >= FRAME_STORE_SIZE / PAGE_SIZE) return -1;
    int frame = next_free_frame++;
    frame_used[frame] = 1;
    return frame;
}

// write a line in a specific frame slot
// frame: frame number, offset: 0, 1, or 2
void set_frame_line(int frame, int offset, const char *line) {
    int index = frame * PAGE_SIZE + offset;
    assert(index < FRAME_STORE_SIZE);
    if (linememory[index].line) free(linememory[index].line);
    linememory[index].allocated = true;
    linememory[index].line = strdup(line);
}

// read line from a frame
const char *get_frame_line(int frame, int offset) {
    int index = frame * PAGE_SIZE + offset;
    assert(linememory[index].allocated);
    return linememory[index].line;
}

void load_page(FILE *file, int page, int frame) {
    // assuming max line length is 100 since the test files are pretty small
    char buffer[100];

    /* changing start depending on page nb, so if input page
    is 2 and PAGE_SIZE is 3 then we start at line 6 for page 2 */
    int start = page * PAGE_SIZE;

    // return to pointer to start of file for when we take a different page
    rewind(file);

    // read and discard lines until we get to the start of the page
    for (int i = 0; i < start; i++) {
        fgets(buffer, sizeof(buffer), file);
    }

    // read lines for the page into the frame
    for (int i = 0; i < PAGE_SIZE; i++) {
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            set_frame_line(frame, i, buffer);
        } else {
            //fill up remaining slots to keep frame size consistent
            set_frame_line(frame, i, "");
        }

    }
}

/*int victim_frame() {
    // total frames in frame store
    int tot_frames = FRAME_STORE_SIZE / PAGE_SIZE;

    // count how many frames are currently used
    int used_frames = 0;
    for (int i = 0; i < tot_frames; i++) {
        if (frame_used[i]) used_frames++;
    }

    // picking a victim number among used frames (randomly for 1.2.2)
    int victim = rand() % used_frames;

    // pick the victim frame by counting through the used frames 
    // until we get to the victim number
    for (int i = 0; i < tot_frames; i++) {
        if (frame_used[i]) {
            if (victim == 0) return i;
            victim--;
        }
    }

    return -1;

} */

// victim_frame modified for LRU
int victim_frame() {

    // total frames in frame store
    int tot_frames = FRAME_STORE_SIZE / PAGE_SIZE;

    // no longer random, go by least recently used
    int victim = -1;
    long oldest_time = LONG_MAX; // no oldest time is set up yet, will do in loop

    for (int i = 0; i < tot_frames; i++) {
        if (!frame_used[i]) continue; // skip if frame isn't used
        
        // to set up the victim
        if (least_recently_used[i] < oldest_time) {
            victim = i;
            oldest_time = least_recently_used[i];
        }
    }
    return victim;
}

void update_table_evicted_frame(struct queue *q, int evicted_frame) {
    struct PCB *table_entry = queue_head(q);
    while (table_entry) {
        // go through pages of each process
        for (int page = 0; page < table_entry->num_pages; page++) {
            // update page using evicted frame
            if (table_entry->page_table[page] == evicted_frame) {
                table_entry->page_table[page] = -1;
            }
        }

        // next process
        table_entry = table_entry->next;
    }

}


void page_faults_handler(struct PCB *pcb, struct queue *ready_queue) {
    // loads the next page into the next free frame
    int next_page = pcb->pc_page;


    int frame = allocate_frame();
    if (frame == -1) {
        // if the frame store is full, evict frame
        int victim = victim_frame();
        printf("Page fault! Victim page contents:\n\n");
        // print the content of victim page line by line
        for (int i = 0; i < PAGE_SIZE; i++) {
            printf("%s", get_frame_line(victim, i));
        }
        printf("\nEnd of victim page contents.\n");

        // updating the page tables using evicted frame
        update_table_evicted_frame(ready_queue, victim);

        frame = victim;
    }
    // if frame store isn't full, only print this
    else printf("Page fault!\n");

    FILE *script = fopen(pcb->name, "rt");
    load_page(script, next_page, frame);
    fclose(script);

    pcb->page_table[next_page] = frame;

    least_recently_used[frame] = ++used_time; // update time for LRU
}