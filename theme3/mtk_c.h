#ifndef MTK_C_H
#define MTK_C_H

#define NULLTASKID 0       // End of queue
#define NUMTASK 5          // Max number of tasks
#define NUMSEMAPHORE 5
#define STKSIZE 1024       // 1KB stack size

// Subroutines and variables from assembly
extern unsigned int runtime;
extern int id;
extern void P(int sem_id);
extern void V(int sem_id);
extern void pv_handler();
extern void init_timer();
extern void swtch();
extern void first_task();

typedef int TASK_ID_TYPE;

typedef struct {
    int count;
    TASK_ID_TYPE task_list;
} SEMAPHORE_TYPE;

typedef struct {
    char ustack[STKSIZE];
    char sstack[STKSIZE];
} STACK_TYPE;

typedef struct {
    void (*task_addr)();
    void *stack_ptr;
    int priority;
    int status; // 0: not used, 1: using, 2: over
    TASK_ID_TYPE next;
} TCB_TYPE;

// Extern declarations for global variables
extern FILE *comout[2];
extern FILE *comin[2];
extern SEMAPHORE_TYPE semaphore[NUMSEMAPHORE];
extern STACK_TYPE stacks[NUMTASK];
extern TCB_TYPE task_tab[NUMTASK + 1];

extern TASK_ID_TYPE curr_task;
extern TASK_ID_TYPE next_task;
extern TASK_ID_TYPE new_task;
extern TASK_ID_TYPE ready;

// Function prototypes
TASK_ID_TYPE removeq(TASK_ID_TYPE *ptr);
void *init_stack(TASK_ID_TYPE id);
void addq(TASK_ID_TYPE *top_of_list, TASK_ID_TYPE task_id);
void sched();
void wakeup(int ch);
void sleep(int ch);
void p_body();
void v_body();
void init_kernel();
void init_io();
void set_task(void *new_task_addr);
void begin_sch();
int fcntl(int fd, int cmd, ...);
int inkey(bool ch);

#endif // MTK_C_H

