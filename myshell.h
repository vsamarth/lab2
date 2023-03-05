// DO NOT EDIT THIS FILE

#include <stddef.h>
#include <sys/types.h>

#define MAX_PROCESSES 50

struct PCBTable {
    pid_t pid;
    int status;   // 4: Stopped, 3: Terminating, 2: Running, 1: exited
    int exitCode; // -1 not exit, else exit code status
};

struct PCBContainer {
    struct PCBTable table[MAX_PROCESSES];
    size_t size;
};

void pcb_init(struct PCBContainer *pcb);
void pcb_add(struct PCBContainer *pcb, pid_t pid);

void my_init(void);
void my_process_command(size_t num_tokens, char **tokens);
void my_quit(void);