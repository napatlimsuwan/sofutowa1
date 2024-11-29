#ifndef MTK_C_H
#define MTK_C_H

#define NULLTASKID 0 //キューの終端
#define NUMTASK 5    //最大タスク数
#define NUMSEMAPHORE 5
#define STKSIZE 1024    //1KB?

//subroutines and variables from asm.
extern int id;
extern void pv_handler();
extern void INIT_TIMER();
extern void swtch();
extern void first_task();


typedef int TASK_ID_TYPE;
TASK_ID_TYPE    ready;

typedef struct
{
    int count;
    TASK_ID_TYPE task_list;
} SEMAPHORE_TYPE;

SEMAPHORE_TYPE semaphore[NUMSEMAPHORE];

typedef struct
{
    char ustack[STKSIZE];
    char sstack[STKSIZE];
} STACK_TYPE;

STACK_TYPE stacks[NUMTASK];

typedef struct
{
    void (*task_addr)();
    void *stack_ptr;
    int priority;
    int status; // 0:not used, 1: using, 2: over
    TASK_ID_TYPE next;
} TCB_TYPE;

static const TCB_TYPE empty_task_tab;  //空のタスクテーブル
TCB_TYPE task_tab[NUMTASK + 1];

void addq(TASK_ID_TYPE* top_of_list, TASK_ID_TYPE task_id);
void sched();
void wakeup(int ch);
void sleep(int ch);
void* init_stack(TASK_ID_TYPE id);
void p_body();
void v_body();
void init_kernel();
void set_task(void* new_task_addr);
void begin_sch();

#endif