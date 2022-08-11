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
typedef unsigned int Priority;

#include <stdio.h>
#include <stdlib.h>

enum TaskState
{
    RUNNING,
    READY,
    SUSPEND
};

struct Context
{
    int restTime;
    TaskState state_;
};

/* task control block */
class TCB
{
public:
    TaskId task_id_;                                                           /* タスクの識別子 0~255の整数値*/
    TCB(TaskId task_id, void (*function)(), Priority priority, int stacksize); /* This (usually) specifies the amount of stack space that is to be allocated to the task. */
    TaskState state_;                                                          /* running, ready, suspendの3つの状態を持つ */
    Priority priority_;                                                        /* タスクの優先度(0~9) */
    int *pstack_;                                                              /* タスクのスタック領域の先頭アドレスを保持 */
    TCB *pnext_;                                                               /* Pointer to next task */
    void (*task_handler_)(void);                                               /* 関数ポインタの実行開始アドレス */
    Context context;
};
void contextInit(Context *, void (*run)(TCB *), TCB *, int *pStackTop);
void ContextSwitch(Context *pOldContext, Context *pNewContext);

void ContextSwitch(Context *pOldContext, Context *pNewContext)
{
    Context *pStackContext;
    pStackContext = pOldContext;
    // コンテキストスタックの存在判定
    if (pStackContext != NULL)
    {
        pNewContext = pStackContext;
        printf("%d", pNewContext->state_);
        printf("%d", pOldContext->state_);
    }
}
class TCBList
{
public:
    TCBList();
    TCB *delete_task(TCB *pTCB);
    void create_task(TCB *pTCB);

    TCB *phead_;
};

TCBList::TCBList()
{
    phead_ = NULL;
}

class Scheduler
{
public:
    Scheduler();
    void start();
    void scheduler();
    static TCB *pRunningTask;
    static TCBList readyList;
};

TCB *Scheduler::pRunningTask = NULL;
TCBList Scheduler::readyList;
Scheduler os;
void run(TCB *pTCB);

void run(TCB *pTCB)
{
    pTCB->task_handler_();
    os.readyList.delete_task(pTCB);
    os.pRunningTask = NULL;
    delete pTCB->pstack_;
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
    TCB *pOldTask;
    TCB *pNewTask;

    printf("readyList.phead_ = %d ", readyList.phead_);

    //
    // If there is a higher-priority ready task, switch to it.
    //
    if (pRunningTask != readyList.phead_)
    {
        printf("pRunningTask != readyList.phead_");
        pOldTask = pRunningTask;
        pNewTask = readyList.phead_;

        pNewTask->state_ = RUNNING;
        pRunningTask = pNewTask;
    }

    if (pOldTask == NULL)
    {
        printf("pOldTask == NULL");
        // ContextSwitch(NULL, &pNewTask->context);
    }
    else
    {
        // pOldTask->state_ = READY; // セグフォ
        // ContextSwitch(&pOldTask->context, &pNewTask->context);
    }
}

void TCBList::create_task(TCB *pTCB)
{
    TCB **ppPrev_ = &this->phead_;
    if ((*ppPrev_) == NULL)
    {
        (*ppPrev_) == pTCB;
        return;
    }
    while ((*ppPrev_) != NULL && pTCB->priority_ <= (*ppPrev_)->priority_)
    {
        ppPrev_ = &(*ppPrev_)->pnext_;
    }

    if (ppPrev_ == &(this->phead_) &&
        pTCB->priority_ > (*ppPrev_)->priority_)
    {
        pTCB->pnext_ = (*ppPrev_);
        this->phead_ = pTCB;
    }
    else
    {
        pTCB->pnext_ = (*ppPrev_)->pnext_;
        (*ppPrev_) = pTCB;
    }
}

TCB *TCBList::delete_task(TCB *pTCB)
{
    TCB **ppPrev = &this->phead_;

    while (*ppPrev != NULL && *ppPrev != pTCB)
    {
        ppPrev = &(*ppPrev)->pnext_;
    }

    if (*ppPrev == NULL)
    {
        return (NULL);
    }

    *ppPrev = pTCB->pnext_;

    return (pTCB);
}

TCB::TCB(TaskId task_id, void (*function)(), Priority priority, int stack_size)
{
    stack_size /= sizeof(int);
    task_id_ = task_id;
    state_ = READY;
    priority_ = priority;
    task_handler_ = function;
    pstack_ = new int[stack_size];
    pnext_ = NULL;

    // contextInit(&context, run, this, pstack_ + stack_size);

    os.readyList.create_task(this);
    // run(this);

    os.scheduler();
}

void task_a(void)
{
    printf("taskA");

    // digitalWrite(13, HIGH);
    // delay(100);
    // digitalWrite(13, LOW);
    // delay(300);

    return;
}
void task_b(void)
{
    printf("taskB");
}
void task_c(void)
{
    printf("taskC");
}

int main(int argc, char *argv[])
{

    TCB taskA(TASK_ID0, task_a, 150, 512);
    TCB taskB(TASK_ID1, task_b, 120, 512);
    TCB taskC(TASK_ID1, task_c, 90, 512);
    os.start();
}