#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "minios.hpp"
#include "syscall.hpp"

#include <stdbool.h>

#include <string>

void task_a(void)
{
    printf("taskA");
    // digitalWrite(LED_BUILTIN, HIGH);
    // wait_task(TASKA_INTERVAL);
    // printf("タスク%d", tcb[TASK_ID0].TASK_ID);
    // printf("優先度%d\n", tcb[TASK_ID0].PRIORITY);
    // Serial.print("taskA");
}

void task_b(void)
{
    printf("taskB");
    // digitalWrite(LED_BUILTIN, HIGH);
    // wait_task(TASKB_INTERVAL);
    // printf("タスク%d", tcb[TASK_ID1].TASK_ID);
    // printf("優先度%d\n", tcb[TASK_ID1].PRIORITY);
    // Serial.print("taskB");
}

void task_c(void)
{
    printf("taskC");
    // digitalWrite(LED_BUILTIN, HIGH);
    // wait_task(TASKC_INTERVAL);
    // printf("タスク%d", tcb[TASK_ID2].TASK_ID);
    // printf("優先度%d\n", tcb[TASK_ID2].PRIORITY);
    // Serial.print("taskC");
}

/* initialize */
void init_os()
{
    ready_que = 0;
    suspend_que = 0;
    wait_que = 0;
    run_que = 0;
}

int main(void)
{
    init_os();

    create_task(TASK_ID0, READY, task_a, 2);
    create_task(TASK_ID1, READY, task_b, 5);
    create_task(TASK_ID2, READY, task_c, 1);

    // delete_task(TASK_ID1);

    scheduling();
    return 0;
}