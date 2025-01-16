#ifndef MTK_C_H
#define MTK_C_H

// Constants
#define NULLTASKID 0       // End of queue
#define NUMTASK 5          // Max number of tasks
#define NUMSEMAPHORE 5
#define STKSIZE 1024       // 1KB stack size

// Type definitions
typedef int TASK_ID_TYPE;

typedef struct {
    int count;             // Semaphore counter
    TASK_ID_TYPE task_list; // Linked list of tasks waiting on the semaphore
} SEMAPHORE_TYPE;

typedef struct {
    char ustack[STKSIZE];  // User stack
    char sstack[STKSIZE];  // System stack
} STACK_TYPE;

typedef struct {
    void (*task_addr)();   // Task entry point
    void *stack_ptr;       // Pointer to the task's stack
    int priority;          // Task priority
    int status;            // Task status: 0 (not used), 1 (active), 2 (completed)
    TASK_ID_TYPE next;     // Next task in the queue
} TCB_TYPE;

// External variables and subroutines from assembly
extern unsigned int runtime;
extern int id;
extern void P(int sem_id);
extern void V(int sem_id);
extern void pv_handler();
extern void init_timer();
extern void swtch();
extern void first_task();

// External declarations for global variables
extern FILE *comout[2];           // Output communication channels
extern FILE *comin[2];            // Input communication channels
extern SEMAPHORE_TYPE semaphore[NUMSEMAPHORE]; // Semaphore array
extern STACK_TYPE stacks[NUMTASK];            // Task stack array
extern TCB_TYPE task_tab[NUMTASK + 1];        // Task control block array

extern TASK_ID_TYPE curr_task;    // Currently running task
extern TASK_ID_TYPE next_task;    // Next task to run
extern TASK_ID_TYPE new_task;     // Newly created task
extern TASK_ID_TYPE ready;        // Ready queue

// Function prototypes
TASK_ID_TYPE removeq(TASK_ID_TYPE *ptr); // Remove a task from a queue
void *init_stack(TASK_ID_TYPE id);        // Initialize a task stack
void addq(TASK_ID_TYPE *top_of_list, TASK_ID_TYPE task_id); // Add a task to a queue
void sched();                             // Scheduler function
void wakeup(int ch);                      // Wake up a sleeping task
void sleep(int ch);                       // Put a task to sleep
void p_body();                            // Body of the P (wait) operation
void v_body();                            // Body of the V (signal) operation
void init_kernel();                       // Initialize the kernel
void init_io();                           // Initialize I/O
void set_task(void *new_task_addr);       // Set a new task
void begin_sch();                         // Begin scheduling
int fcntl(int fd, int cmd, ...);          // File control
int inkey(bool ch);                       // Get input key

#endif // MTK_C_H

