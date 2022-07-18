#include <stdio.h>
#include <stdlib.h>

#include "minios.h"
#include "syscall.h"

void task_a(void)
{
    // digitalWrite(LED_BUILTIN, HIGH);
    wait_task(TASKA_INTERVAL);
    printf("taskA");
    // Serial.print("taskA");
}

void task_b(void)
{
    // digitalWrite(LED_BUILTIN, HIGH);
    wait_task(TASKB_INTERVAL);
    printf("taskB");
    // Serial.print("taskB");
}

void task_c(void)
{
    // digitalWrite(LED_BUILTIN, HIGH);
    wait_task(TASKB_INTERVAL);
    printf("taskC");
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

int main (void)
{
    TCB tcb_task;
    init_os();
    create_task(TASK_ID0, task_a);
    create_task(TASK_ID1, task_b);
    create_task(TASK_ID2, task_c);

    start_task(TASK_ID0, READY);
    start_task(TASK_ID1, SUSPEND);
    start_task(TASK_ID2, SUSPEND);

    run_que = TASK_ID0;

    while (ON)
    {
        tcb_task = tcb[run_que];
        (*(tcb_task.task))();
        run_que++;
        if (run_que == TASK_ID_MAX)
        {
            run_que = TASK_ID0;
        }
    }
}