/**
 * CS2106 AY22/23 Semester 2 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "myshell.h"

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct PCBContainer pcb_container;

char **pid_status = (char *[]){"Exited", "Running", "Terminating", "Stopped"};

void pcb_add(struct PCBContainer *pcb, pid_t pid) {
  size_t size = pcb->size;
  pcb->table[size].pid = pid;
  pcb->table[size].status = 2;
  pcb->table[size].exitCode = -1;
  pcb->size++;
}

bool pcb_exists(struct PCBContainer *pcb, pid_t pid) {
  for (size_t i = 0; i < pcb->size; i++) {
    if (pcb->table[i].pid == pid) {
      return true;
    }
  }
  return false;
}

void pcb_update_status(struct PCBContainer *pcb, pid_t pid, int status,
                       int exitCode) {
  for (size_t i = 0; i < pcb->size; i++) {
    if (pcb->table[i].pid == pid) {
      pcb->table[i].status = status;
      pcb->table[i].exitCode = exitCode;
      return;
    }
  }
}

/*******************************************************************************
 * Signal handler : ex4
 ******************************************************************************/

static void signal_handler(int signo) {

  // Use the signo to identy ctrl-Z or ctrl-C and print “[PID] stopped or print
  // “[PID] interrupted accordingly. Update the status of the process in the PCB
  // table
}

/*******************************************************************************
 * Built-in Commands
 ******************************************************************************/

static void command_info(int option) {
  switch (option) {
  case 0:
    for (size_t i = 0; i < pcb_container.size; i++) {

      int status = pcb_container.table[i].status;
      pid_t pid = pcb_container.table[i].pid;
      int exitCode = pcb_container.table[i].exitCode;

      if (status == 1) {
        printf("[%d] %s %d\n", pid, pid_status[status - 1], exitCode);
      } else {
        printf("[%d] %s\n", pid, pid_status[status - 1]);
      }
    }
    break;
  case 1:
    int count = 0;
    for (size_t i = 0; i < pcb_container.size; i++) {
      if (pcb_container.table[i].status == 1) {
        count++;
      }
    }
    printf("Total exited processes: %d\n", count);
    break;
  case 2:
    count = 0;
    for (size_t i = 0; i < pcb_container.size; i++) {
      if (pcb_container.table[i].status == 2) {
        count++;
      }
    }
    printf("Total running processes: %d\n", count);
    break;
  case 3:
    count = 0;
    for (size_t i = 0; i < pcb_container.size; i++) {
      if (pcb_container.table[i].status == 3) {
        count++;
      }
    }
    printf("Total terminating processes: %d\n", count);
    break;
  default:
    fprintf(stderr, "Wrong command\n");
    break;
  }
}

static void command_wait(pid_t pid) {
  for (size_t i = 0; i < pcb_container.size; i++) {
    if (pcb_container.table[i].pid == pid &&
        pcb_container.table[i].status == 2) {
      int status;
      pid_t p = waitpid(pid, &status, 0);

      if (WIFEXITED(status)) {
        pcb_update_status(&pcb_container, pid, 1, WEXITSTATUS(status));
      }
    }
  }
}

static void command_terminate(pid_t pid) {
  for (size_t i = 0; i < pcb_container.size; i++) {
    if (pcb_container.table[i].pid == pid &&
        pcb_container.table[i].status == 2) {
      kill(pid, SIGTERM);
      pcb_update_status(&pcb_container, pid, 3, -1);
    }
  }
}

static void command_fg(/* pass necessary parameters*/) {

  /******* FILL IN THE CODE *******/

  // if the {PID} status is stopped
  // Print “[PID] resumed”
  // Use kill() to send SIGCONT to {PID} to get it continue and wait for it
  // After the process terminate, update status and exit code (call
  // proc_update_status())
}

/*******************************************************************************
 * Program Execution
 ******************************************************************************/

static void command_exec(char *program, char **args, int argc) {
  // printf("executing %s with %d arguments\n", program, argc);
  int ret = access(program, R_OK | X_OK);
  if (ret == -1) {
    fprintf(stderr, "%s not found\n", program);
    return;
  }

  bool background = false;
  if (argc > 1) {
    if (strcmp(args[argc - 2], "&") == 0) {
      background = true;
      args[argc - 2] = NULL;
    }
  }

  char *inputFile = NULL;
  char *outputFile = NULL;
  char *errorFile = NULL;

  for (int i = 0; i < argc - 1; i++) {
    if (args[i] == NULL)
      continue;
    if (strcmp(args[i], "<") == 0) {
      inputFile = args[i + 1];
      args[i] = NULL;
      args[i + 1] = NULL;
    } else if (strcmp(args[i], ">") == 0) {
      outputFile = args[i + 1];
      args[i] = NULL;
      args[i + 1] = NULL;
    } else if (strcmp(args[i], "2>") == 0) {
      errorFile = args[i + 1];
      args[i] = NULL;
      args[i + 1] = NULL;
    }
  }

  pid_t pid;
  if ((pid = fork()) == 0) {
    if (outputFile != NULL) {
      int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }

    if (inputFile != NULL) {
      int ret = access(inputFile, R_OK);
      if (ret == -1) {
        fprintf(stderr, "%s does not exist\n", inputFile);
        exit(1);
      }
      int fd = open(inputFile, O_RDONLY);
      dup2(fd, STDIN_FILENO);
      close(fd);
    }

    if (errorFile != NULL) {
      int fd = open(errorFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      dup2(fd, STDERR_FILENO);
      close(fd);
    }

    int err = execv(program, args);
    if (err) {
        // TODO: print error message
    }

  } else {
    pcb_add(&pcb_container, pid);

    if (background) {
      printf("Child [%d] in background\n", pid);
    } else {
      int status;
      waitpid(pid, &status, 0);
      // Update the status and exit code of the process in the PCBTable
      if (WIFEXITED(status)) {
        pcb_update_status(&pcb_container, pid, 1, WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        pcb_update_status(&pcb_container, pid, 1, WTERMSIG(status));
      }
    }
  }
}

/*******************************************************************************
 * Command Processor
 ******************************************************************************/

static void command(char *command, char **args, int argc) {
  if (strcmp(command, "info") == 0) {
    if (args[1] == NULL || args[2] != NULL) {
      fprintf(stderr, "Wrong command\n");
      return;
    }
    command_info(atoi(args[1]));
  } else if (strcmp(command, "wait") == 0) {
    if (args[1] == NULL || args[2] != NULL) {
      fprintf(stderr, "Wrong command\n");
      return;
    }
    command_wait(atoi(args[1]));
  } else if (strcmp(command, "terminate") == 0) {
    if (args[1] == NULL || args[2] != NULL) {
      fprintf(stderr, "Wrong command\n");
      return;
    }
    command_terminate(atoi(args[1]));
  } else if (strcmp(command, "fg") == 0) {
    command_fg();
  } else {
    for (int i = 0; i < argc; i++) {
      if (args[i] == NULL || strcmp(args[i], ";") == 0) {
        args[i] = NULL;
        if (command == NULL) {
          return;
        }
        command_exec(command, args, i + 1);
        command = args[i + 1];
        args = args + i + 1;
        argc = argc - i - 1;
        i = 0;
      }
    }
  }
}

/*******************************************************************************
 * High-level Procedure
 ******************************************************************************/

void pcb_init(struct PCBContainer *container) { container->size = 0; }

void my_init(void) {
  // use signal() with SIGTSTP to setup a signalhandler for ctrl+z : ex4
  // use signal() with SIGINT to setup a signalhandler for ctrl+c  : ex4

  pcb_init(&pcb_container);
}

void collect_zombie() {
  for (size_t i = 0; i < pcb_container.size; i++) {
    if (pcb_container.table[i].status != 1) {
      int status;
      pid_t p = waitpid(pcb_container.table[i].pid, &status, WNOHANG);
      if (p == pcb_container.table[i].pid) {
        if (WIFEXITED(status)) {
          pcb_update_status(&pcb_container, pcb_container.table[i].pid, 1,
                            WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
          pcb_update_status(&pcb_container, pcb_container.table[i].pid, 1,
                            WTERMSIG(status));
        }
      }
    }
  }
}

void my_process_command(size_t num_tokens, char **tokens) {
  assert(num_tokens > 0);
  collect_zombie();
  char *program = tokens[0];
  command(program, tokens, num_tokens);
}

void my_quit(void) {
  for (size_t i = 0; i < pcb_container.size; i++) {
    if (pcb_container.table[i].status == 2 ||
        pcb_container.table[i].status == 4) {
      int status;
      waitpid(pcb_container.table[i].pid, &status, WNOHANG);

      if (WIFEXITED(status) || WIFSIGNALED(status)) {
        // this process has already exited
        // no need to kill it
        continue;
      }

      kill(pcb_container.table[i].pid, SIGTERM);
      printf("Killing [%d]\n", pcb_container.table[i].pid);
    }
  }

  printf("\nGoodbye\n");
}
