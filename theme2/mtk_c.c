#include "mtk_c.h"

//global variables
TASK_ID_TYPE new_task;
TASK_ID_TYPE next_task;
TASK_ID_TYPE curr_task;


void init_kernel(){  
    
    for(int i=1; i<=NUMTASK;i++){
        task_tab[i] = empty_task_tab;
    }
    ready = NULLTASKID;
    *(int *)0x084 = (int)(pv_handler);  // set trap #1 interrupt vector
    for(int j=0; j<NUMSEMAPHORE; j++){
        semaphore[j].count = 1;
        semaphore[j].task_list = NULLTASKID;
    }
}

void set_task(void* new_task_addr){
    int i=1;
    while(i <= NUMTASK){
        if(task_tab[i].task_addr == 0){
            new_task = (TASK_ID_TYPE)i;
            task_tab[new_task].task_addr = new_task_addr;
            task_tab[new_task].status = 1;
            task_tab[new_task].stack_ptr = init_stack(new_task);
            addq(&ready, new_task);
            break;
        }
        else{
            i++;
        }
    }
}

void* init_stack(TASK_ID_TYPE id)
{
	int *ssp = &stacks[id-1].sstack[STKSIZE];
	unsigned short int *sr_ssp;
	int i=0;	

	*(--ssp) = task_tab[id].task_addr;
	sr_ssp = ssp;
	*(--sr_ssp)=0;	//so it only uses 2*4 bytes since this is unsigned short int
	ssp = sr_ssp;

	while(i<15)
	{
		*(--ssp)=0;	//skip 15*4 bytes
		i++;
	}
	*(--ssp) = &stacks[id-1].ustack[STKSIZE];

	return (void*)ssp;
}


void begin_sch(){
    curr_task = removeq(&ready);
    init_timer();
    first_task();
}


void addq(TASK_ID_TYPE* ptr, TASK_ID_TYPE id){
    TASK_ID_TYPE tmp = *ptr;
    if (tmp == NULLTASKID)  *ptr=id;    //update the value of address of ptr to id
    else{
        TASK_ID_TYPE j = task_tab[tmp].next;
        while(j!=NULLTASKID){
            tmp=j;
            j=task_tab[tmp].next;
        }
        task_tab[tmp].next = id;
    }
    task_tab[id].next = NULLTASKID;
}


TASK_ID_TYPE removeq(TASK_ID_TYPE* ptr)
{
    TASK_ID_TYPE id;
    if(*ptr == NULLTASKID){
		return NULLTASKID;
	} else {
		TASK_ID_TYPE top = *ptr;
		*ptr = task_tab[top].next;
		return top;
	}
}


void p_body(int id){
    semaphore[id].count--;
    if (semaphore[id].count < 0){
        sleep(id);
    }
}


void v_body(int id){
	semaphore[id].count++;
	if(semaphore[id].count <= 0){
        wakeup(id);
    }
}


void sched()
{
	next_task = removeq(&ready);
	if(next_task == NULLTASKID)
		while(1);   //if next_task is nulltaskid, infinite loop
}


void sleep(int ch){
    addq(&semaphore[ch].task_list, curr_task);
	sched();
	swtch();    //calling subroutine from asm
}


void wakeup(int ch){
    addq(&ready, removeq(&semaphore[ch].task_list));
}