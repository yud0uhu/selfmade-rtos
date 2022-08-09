#define OFF 0
#define ON OFF + 1

#define TASKA_INTERVAL 10
#define TASKB_INTERVAL 45
#define TASKC_INTERVAL 20

#define TASK_ID_MAX 32

#define TASK_ID0 0
#define TASK_ID1 1
#define TASK_ID2 2

typedef unsigned char TaskId;
typedef unsigned char Priority;

#include <stdio.h>
#include <stdlib.h>

enum TaskState
{
    RUNNING,
    READY,
    SUSPEND
};

/* task control block */
class Task
{
public:
    TaskId task_id_;                                                            /* タスクの識別子 0~255の整数値*/
    Task(TaskId task_id, void (*function)(), Priority priority, int stacksize); /* This (usually) specifies the amount of stack space that is to be allocated to the task. */
    TaskState state_;                                                           /* running, ready, suspendの3つの状態を持つ */
    Priority priority_;                                                         /* タスクの優先度(0~9) */
    int *pstack_;                                                               /* タスクのスタック領域の先頭アドレスを保持 */
    Task *pnext_;                                                               /* Pointer to next task */
    void (*task_handler_)(void);                                                /* 関数ポインタの実行開始アドレス */
};

class TaskList
{
public:
    TaskList();
    Task *delete_task(Task *pTask_);
    void create_task(Task *pTask_);

    Task *phead_;
};

TaskList::TaskList()
{
    phead_ = NULL;
}

class Scheduler
{
public:
    Scheduler();
    void start();
    void scheduler();
    static Task *pRunningTask;
    static TaskList readyList;
};

Scheduler os;
Task *Scheduler::pRunningTask = NULL;
TaskList Scheduler::readyList;

void run(Task *pTask)
{
    pTask->task_handler_();
    os.pRunningTask = NULL;
    os.scheduler();
};

Scheduler::Scheduler(void)
{
    return;
}

void Scheduler::start(void)
{
    scheduler();
}

void Scheduler::scheduler(void)
{
    Task *pOldTask;
    Task *pNewTask;

    if (pRunningTask != readyList.phead_)
    {
        pOldTask = pRunningTask;
        pNewTask = readyList.phead_;

        pNewTask->state_ = RUNNING;
        pRunningTask = pNewTask;
    }

    pRunningTask->task_handler_;
}

void TaskList::create_task(Task *pTask)
{
    Task **ppPrev_ = &this->phead_;
    if (*ppPrev_ == NULL)
    {
        *ppPrev_ == pTask;
        return;
    }
    while (*ppPrev_ != NULL && pTask->priority_ <= (*ppPrev_)->priority_)
    {
        ppPrev_ = &(*ppPrev_)->pnext_;
    }

    if (ppPrev_ == &(this->phead_) &&
        pTask->priority_ > (*ppPrev_)->priority_)
    {
        pTask->pnext_ = (*ppPrev_);
        this->phead_ = pTask;
    }
    else
    {
        pTask->pnext_ = (*ppPrev_)->pnext_;
        *ppPrev_ = pTask;
    }
}

// void contextInit(void (*run)(Task *), Task *, int *pStackTop);
Task::Task(TaskId task_id, void (*function)(), Priority priority, int stack_size)
{
    stack_size /= sizeof(int);
    task_id_ = task_id;
    state_ = READY;
    priority_ = priority;
    task_handler_ = function;
    pstack_ = new int[stack_size];
    pnext_ = NULL;
    os.readyList.create_task(this);

    // contextInit(run, this, pstack_ + stack_size);
    os.scheduler();
}

void task_a(void)
{
    while (1)
    {
        printf("taskA");
    }
    return;
}
void task_b(void)
{
    while (1)
    {
        printf("taskB");
    }
    return;
}

Task taskA(TASK_ID0, task_a, 150, 512);
Task taskB(TASK_ID1, task_b, 120, 512);

int main(int argc, char *argv[])
{
    os.start();
}