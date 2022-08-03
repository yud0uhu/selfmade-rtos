#include <stdio.h>
#include <stdlib.h>

#include  "minios.h"
#include "syscall.h"

void task_a(void)
{
    digitalWrite(LED_BUILTIN, HIGH);
    wait_task(TASKA_INTERVAL);
    // printf("タスク%d",tcb[TASK_ID0].TASK_ID);
    // printf("優先度%d\n",tcb[TASK_ID0].PRIORITY);
    // Serial.print("taskA");
}

void task_b(void)
{
    digitalWrite(LED_BUILTIN, HIGH);
    wait_task(TASKB_INTERVAL);
    // printf("タスク%d",tcb[TASK_ID1].TASK_ID);
    // printf("優先度%d\n",tcb[TASK_ID1].PRIORITY);
    // Serial.print("taskB");
}

void task_c(void)
{
    digitalWrite(LED_BUILTIN, HIGH);
    wait_task(TASKC_INTERVAL);
    // printf("タスク%d",tcb[TASK_ID2].TASK_ID);
    // printf("優先度%d\n",tcb[TASK_ID2].PRIORITY);
    // Serial.print("taskC");
}

/* initialize */
void init_os(void)
{
    ready_que = 0;
    suspend_que = 0;
    wait_que = 0;
    run_que = 0;
}

void swap_task(TCB* a, TCB* b) {
	TCB temp;
	temp = *a;
	*a = *b;
	*b = temp;
	return;
}

void scheduling(void)
{
    run_que = TASK_ID0;

    while (ON)
    {
        if (tcb[run_que].PRIORITY < tcb[run_que+1].PRIORITY)
        {
            swap_task(&tcb[run_que],&tcb[run_que+1]);
        }
        (*tcb[run_que].task)();
        run_que++;
        if(run_que == TASK_ID_MAX) {
            run_que = TASK_ID0;
        }
    }
}

int main(void){
    init_os();
    create_task(TASK_ID0, task_a, 20);
    create_task(TASK_ID1, task_b, 50);
    create_task(TASK_ID2, task_c, 40);

    start_task(TASK_ID0, READY);
    start_task(TASK_ID1, SUSPEND);
    start_task(TASK_ID2, SUSPEND);

    scheduling();
}