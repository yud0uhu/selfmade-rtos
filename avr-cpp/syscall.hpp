#ifndef INCLUDED_syscall_hpp_
#define INCLUDED_syscall_hpp_

#define OFF 0
#define ON OFF + 1

#define TASKA_INTERVAL 10
#define TASKB_INTERVAL 45
#define TASKC_INTERVAL 20

#define TASK_ID_MAX 3

#define TASK_ID0 0
#define TASK_ID1 1
#define TASK_ID2 2

typedef unsigned char TaskId;
typedef unsigned char Priority;

enum TaskState
{
    RUNNING,
    WAITING,
    READY
};

/* task control block */
class TCB
{
public:
    TCB(void (*function)(), Priority priority, int stack_size);
    TaskId task_id_;    /* タスクの識別子 0~255の整数値*/
    TaskState state_;   /* running, waiting, readyの3つの状態を持つ */
    Priority priority_; /* タスクの優先度(0~9) */
    int *pstask_;
    TCB *pnext_;
    void (*entry_point_)(void); /* 関数ポインタの実行開始アドレス */
private:
    TaskId next_task_id_;
};

class TCBList
{
public:
    TCBList();
    void insert_(TCB *pTask);
    TCB *remove_(TCB *pTask);
    TCB *ptop_;
};

// /* systemcall */
class Scheduler
{
public:
    Scheduler();
    void start_();
    void scheduler_();

    TCB *prunning_task_;
    TCBList ready_list_;
};
#endif