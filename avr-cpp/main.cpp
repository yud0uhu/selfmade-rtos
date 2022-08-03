#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "minios.hpp"
#include "syscall.hpp"

#include <stdbool.h>

#include <string>
#include <iostream>

void task_a(void)
{
    // digitalWrite(LED_BUILTIN, HIGH);
    // wait_task(TASKA_INTERVAL);
    // printf("タスク%d", tcb[TASK_ID0].TASK_ID);
    // printf("優先度%d\n", tcb[TASK_ID0].PRIORITY);
    // Serial.print("taskA");
}

void task_b(void)
{
    // digitalWrite(LED_BUILTIN, HIGH);
    // wait_task(TASKB_INTERVAL);
    // printf("タスク%d", tcb[TASK_ID1].TASK_ID);
    // printf("優先度%d\n", tcb[TASK_ID1].PRIORITY);
    // Serial.print("taskB");
}

void task_c(void)
{
    // digitalWrite(LED_BUILTIN, HIGH);
    // wait_task(TASKC_INTERVAL);
    // printf("タスク%d", tcb[TASK_ID2].TASK_ID);
    // printf("優先度%d\n", tcb[TASK_ID2].PRIORITY);
    // Serial.print("taskC");
}

// bool enque(struct TCB *q, int data)
// {
//     int i;

//     if(q->num >= 128) {
//         fprintf(stderr, "Error: queue is full.\n");
//         return false;
//     }
//     for(i=q->num-1; i>=0 && data < q->data[i]; i--){
//         q->data[i+1] = q->data[i];
//     }
//     q->data[i+1] = data;
//     q->num++;
//     return true;
// }

// bool dequeue(struct TCB *q, int *data)
// {
//   int i;
//   if (q->num == 0) {
//     fprintf(stderr, "Error: queue is empty.\n");
//     return false;
//   }
//   *data = q->data[0];
//   for (i = 0; i < q->num; i++) {
//     q->data[i] = q->data[i + 1];
//   }

//   q->num--;

//   return true;
// }

/* initialize */
void init_os()
{
    ready_que = 0;
    suspend_que = 0;
    wait_que = 0;
    run_que = 0;
}

// void swap_task(TCB *a, TCB *b)
// {
//     TCB temp;
//     temp = *a;
//     *a = *b;
//     *b = temp;
//     return;
// }

// void scheduling(void)
// {
//     run_que = TASK_ID0;

//     while (ON)
//     {
//         while (run_que > 0)
//         {
//             int i = (run_que - 1) / 2;
//             if (tcb[run_que].PRIORITY > tcb[i].PRIORITY)
//             {
//                 swap_task(&tcb[run_que], &tcb[i]);
//             }
//             run_que = i;
//             (*tcb[run_que].task)();
//             run_que++;
//             if (run_que == TASK_ID_MAX)
//             {
//                 run_que = TASK_ID0;
//             }
//         }
//     }
// }

int main(void)
{
    init_os();

    create_task(TASK_ID0, task_a, 20);
    create_task(TASK_ID1, task_b, 50);
    create_task(TASK_ID2, task_c, 40);

    start_task(TASK_ID0, READY);
    start_task(TASK_ID1, SUSPEND);
    start_task(TASK_ID2, SUSPEND);

    // scheduling();
    return 0;
}