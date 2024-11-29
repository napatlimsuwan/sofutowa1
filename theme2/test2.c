#include <stdio.h>
#include "mtk_c.h"

#define MAX 1024

void task1()
{
	P(1);
	for(int i = 1; i <= 20; ++i){
		printf("\n1 %d", i);
	}
	V(1);
	while(1){
		printf("\ntask1 finished");
	}
}

void task2()
{	
	P(1);
	for(int i = 1; i <= 20; ++i){
		printf("\n2 %d", i);
	}
	V(1);
	while(1){
		printf("\ntask2 finished");
	}
}

void task3()
{
	P(1);
	for(int i = 1; i <= 20; ++i){
		printf("\n3 %d", i);
	}
	V(1);
	while(1){
		printf("\ntask3 finished");
	}
}

void main()
{
	init_kernel();
	
	set_task(task1);
    	set_task(task2);
    	set_task(task3);

    	begin_sch();
	
}

