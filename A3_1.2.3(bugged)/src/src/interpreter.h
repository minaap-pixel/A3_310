#include <stddef.h>
#include "queue.h"
int interpreter(char *command_args[], int args_size);
int help();

extern const struct schedule_policy *policy;
extern struct queue *q;

struct PCB *run_pcb_to_completion(struct PCB *pcb);
struct PCB *run_pcb_for_n_steps(struct PCB *pcb, size_t n);