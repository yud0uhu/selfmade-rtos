#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "minios.hpp"
#include "syscall.hpp"

#include <stdbool.h>

#include <string>

/* task functions */
void task_a(void)
{
    printf("taskA");
}

void task_b(void)
{
    printf("taskB");
}

void task_c(void)
{
    printf("taskC");
}

void Scheduler::scheduler_(void)
{
    TCB *ppre_task_;
    TCB *pnew_task_;

    if (prunning_task_ != ready_list_.ptop_)
    {
        ppre_task_ = prunning_task_;
        pnew_task_ = ready_list_.ptop_;
    }
    ppre_task_->state_ = READY;
}

int main(void)
{
    TCB taskA(task_a, 150, 256);
    TCB taskB(task_b, 200, 256);

    return 0;
}