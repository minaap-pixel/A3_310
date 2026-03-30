#include <string.h>
#include "interpreter.h"
#include "schedule_policy.h"
#include "stdbool.h"
#include "pcb.h"
#include "shellmemory.h"

#define MAKE_PREEMPTIVE_FN(n)                        \
  struct PCB * run_steps_##n (struct PCB *pcb) {     \
    return run_pcb_for_n_steps(pcb, n);              \
  }

MAKE_PREEMPTIVE_FN(1)
MAKE_PREEMPTIVE_FN(2)
MAKE_PREEMPTIVE_FN(30)

struct PCB *run_pcb_with_faults(struct PCB *pcb); 

const struct schedule_policy FCFS = {
    .run_pcb = run_pcb_to_completion,
    .enqueue = enqueue_fcfs,
    .dequeue = dequeue_typical,
    .enqueue_ignoring_priority = enqueue_ignoring_priority
};

const struct schedule_policy SJF = {
    .run_pcb = run_pcb_to_completion,
    .enqueue = enqueue_sjf,
    .dequeue = dequeue_typical,
    .enqueue_ignoring_priority = enqueue_ignoring_priority
};

const struct schedule_policy RR = {
    .run_pcb = run_pcb_with_faults,
    .enqueue = enqueue_fcfs,
    .dequeue = dequeue_typical,
    .enqueue_ignoring_priority = enqueue_ignoring_priority
};

const struct schedule_policy RR30 = {
    .run_pcb = run_steps_30,
    .enqueue = enqueue_fcfs,
    .dequeue = dequeue_typical,
    .enqueue_ignoring_priority = enqueue_ignoring_priority
};

const struct schedule_policy AGING = {
    .run_pcb = run_steps_1,
    .enqueue = enqueue_aging,
    .dequeue = dequeue_aging,
    .enqueue_ignoring_priority = enqueue_ignoring_priority
};

const struct schedule_policy *get_policy(const char *policy_name) {
    if (strcmp(policy_name, "FCFS")  == 0) return &FCFS;
    if (strcmp(policy_name, "SJF")   == 0) return &SJF;
    if (strcmp(policy_name, "RR")    == 0) return &RR;
    if (strcmp(policy_name, "RR30")  == 0) return &RR30;
    if (strcmp(policy_name, "AGING") == 0) return &AGING;

    return NULL;
}


struct PCB *run_pcb_with_faults(struct PCB *pcb) {
    // current process
    int current_page = pcb->pc_page;
    int current_offset = pcb->pc_offset;

    struct PCB *process = run_pcb_for_n_steps(pcb, 2);

    if (process == NULL) {
        return NULL; // process finished
    }

    // if page fault happens
    if (pcb->pc_page == current_page && pcb->pc_offset == current_offset) {
        page_faults_handler(pcb, q);
        return pcb; //requeue the same process after handling page fault
    }

    return process;


}