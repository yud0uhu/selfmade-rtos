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
typedef unsigned int Priority;

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
/*タイマーハンドラ用*/
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>

void init_task_all(void);
void task_init(TaskId id);
void os_start(void);
int timer_handler();

enum TaskState
{
    RUNNING,
    READY,
    SUSPEND
};

void create_task(TaskId task_id, Priority priority, void (*task)(void));
/* task control block */
typedef struct TCB
{
    TaskId task_id_;     /* タスクの識別子 0~255の整数値*/
    TaskState state_;    /* running, ready, suspendの3つの状態を持つ */
    Priority priority_;  /* タスクの優先度(0~9) */
    struct TCB *p_prev_; /* 前のタスク */
    // uint32_t *p_stack_;  /* 各タスクのスタック領域の先頭アドレスを保持 */
    struct TCB *p_next_; /* 次のタスク*/
    // uint32_t time_up;
    void (*task_handler_)(void);
} tcb_t;

tcb_t TCB[TASK_ID_MAX];
typedef struct
{
    TaskId running;
    // int32_t systime;
    int32_t prev;
} sysinfo_t;

static sysinfo_t _sysinfo;
// 優先度ごとのリスト
tcb_t ready_que[10];
// 待ち状態のタスクを登録するリスト
tcb_t queue_waitng;

// vTaskCode, "NAME", STACK_SIZE, NULL, tskIDLE_PRIORITY, 　&xHandle
void create_task(TaskId task_id, Priority priority, void (*task)(void))
{
    tcb_t *p_task = &TCB[task_id];
    p_task->priority_ = priority;
    p_task->task_handler_ = task;
    tcb_t *p_ptr = &ready_que[p_task->priority_];

    // キューの末尾のタスクが空になるまで探索
    while (p_ptr->p_next_ != 0)
    {
        p_ptr = p_ptr->p_next_;
    }
    // キューの末尾に追加
    p_ptr->p_next_ = p_task;
    p_task->p_prev_ = p_ptr;
    p_task->p_next_ = 0;
}

// 待ちキュー
void delete_task(TaskId task_id)
{
    tcb_t *p_task = &TCB[task_id];
    p_task->p_prev_->p_next_ = p_task->p_next_;
    p_task->p_next_->p_prev_ = p_task->p_prev_;
    p_task->p_prev_ = 0;
    p_task->p_next_ = 0;
}

TaskId get_top_ready_id(void)
{
    Priority priority;
    for (priority = 0; priority < 10; priority++)
    {
        if (ready_que[priority].p_next_ != 0)
        {
            return ready_que[priority].p_next_->task_id_;
        }
    }
    return 0;
}

void init_task_all()
{
    TaskId task_id;
    for (task_id = 0; task_id < TASK_ID_MAX; task_id++)
    {
        task_init(task_id);
    }
}

void task_init(TaskId id)
{
    TCB[id].task_id_ = id;
    TCB[id].state_ = READY;
}

void os_start(void)
{
    // init_task_all();
    _sysinfo.running = get_top_ready_id();
    _sysinfo.prev = _sysinfo.running;
    TCB[_sysinfo.running].state_ = RUNNING;
    timer_handler();
}

TaskId run_que;
int timer_handler()
{ // 優先順位順にタスクを実行

    run_que = _sysinfo.running;
    while (run_que <= 2)
    {
        tcb_t temp;
        if (TCB[run_que + 1].priority_ > TCB[run_que].priority_)
        {
            temp = TCB[run_que + 1];
            TCB[run_que + 1] = TCB[run_que];
            TCB[run_que] = temp;
            TCB[run_que].state_ = RUNNING;
        }

        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGALRM);
        pthread_sigmask(SIG_BLOCK, &sigset, NULL);

        // 1秒毎に通知
        const struct timeval interval = {1, 0};
        const struct itimerval itimer = {interval, interval};
        int ret = setitimer(ITIMER_REAL, &itimer, NULL);
        if (ret != 0)
        {
            perror("setitimer error");
            return -1;
        }

        int i;
        for (i = 0; i < 10; i++)
        {
            int signo;
            sigwait(&sigset, &signo);

            struct timeval tv;
            gettimeofday(&tv, NULL);
            char timebuf[26];
            ctime_r(&tv.tv_sec, timebuf);
            *strchr(timebuf, '\n') = '\0';
            printf("[%s] [%06d] signal[%d] recieved.\n",
                   timebuf, tv.tv_usec, signo);
            if (i % 4 == 3)
            {
                unsigned int waittime = 7;
                printf("wait %d seconds.\n", waittime);
                sleep(waittime);
            }
            if (run_que > 2)
            {
                run_que = 0;
            }
            printf("taskId = %d : ", run_que);
            (*TCB[run_que].task_handler_)();
            TCB[run_que].state_ = READY;
            run_que = run_que + 1;
        }
    }

    return 0;
}

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

int main(void)
{
    // vTaskCode, "NAME", STACK_SIZE, NULL, tskIDLE_PRIORITY, 　&xHandle
    create_task(0, 3, task_a);
    create_task(1, 5, task_b);
    create_task(2, 2, task_c);
    os_start();
}