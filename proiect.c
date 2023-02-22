#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

///ps -eo pid,ppid,comm > processes.txt

struct proc_node {
    pid_t pid;
    pid_t ppid;
    pid_t* children_pids;
    int pchildren_count;
};

int count_pchildren(pid_t pid) {
    char command[64];
    int pchildren_count = 0;
    sprintf(command, "pgrep -P %d | wc -l", pid);
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        return -1;
    }
    fscanf(fp, "%d", &pchildren_count);
    pclose(fp);
    return pchildren_count;
}

int update_process(pid_t pid, struct proc_node *current_process) {
    char command[100];
    sprintf(command, "ps -o pid=,ppid= -p %d", pid);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        return -1;
    }

    char line[256];
    if (fgets(line, sizeof(line), fp) != NULL) {
        char *token = strtok(line, " ");
        current_process->pid = atoi(token);
        token = strtok(NULL, " ");
        current_process->ppid = atoi(token);
    }
    pclose(fp);

    sprintf(command, "pgrep -P %d", pid);
    fp = popen(command, "r");
    if (fp == NULL) {
        return -1;
    }

    current_process->children_pids = (pid_t *) malloc(current_process->pchildren_count * sizeof(pid_t));
    int i = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (i < current_process->pchildren_count) {
            current_process->children_pids[i] = atoi(line);
            i++;
        }
    }
    current_process->pchildren_count = i;
    pclose(fp);

    return 0;
}

bool check_if_exists(pid_t pid) {
    return kill(pid,0) == 0;
}

void show(struct proc_node *p, int level) {
 printf("NIVEL -----------> %d \n", level);
 printf("PID -----------> %d\n", p->pid);
 printf("PPID -----------> %d\n", p->ppid);
 printf("Numar copii ----------> %d\n", p->pchildren_count);
 printf("\n\n\n");
}
void dfs(pid_t pid, pid_t ppid, int level)
{
   if(check_if_exists(pid) == false)
    return;

    struct proc_node *p = (struct proc_node*) malloc(sizeof(struct proc_node));

   int pchildren_count = count_pchildren(pid);
   if (pchildren_count < 0) return;
   p->pchildren_count = pchildren_count;
   p->children_pids = (pid_t)malloc(sizeof(pid_t) * pchildren_count);
   p->pid = pid;
   p->ppid = ppid;

   int ret = update_process(pid,p);

   if(ret < 0) {
   printf("Procesul nu mai exista\n");
   return;
   }

   show(p,level);

   int i=0;
   for(i=0; i< pchildren_count; i++) {
    dfs(p->children_pids[i], pid, level+1);
   }

  free(p->children_pids);
  free(p);
}

int main(int argc, char** argv)
{
    pid_t root_pid = atoi(argv[1]);
    if(check_if_exists(root_pid) == false)
    {
    printf("Procesul nu exista!\n");
    return 0;
    }
    printf("Procesul exista!\n");
    dfs(root_pid, -1, 0);
}
