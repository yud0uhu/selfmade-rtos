#define TASK_ID_MAX 3
#define TASK_ID0 0
#define TASK_ID1 1
#define TASK_ID2 2

typedef unsigned char TaskId;
typedef unsigned int Priority;
typedef unsigned int Tick;
TaskId run_que;

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

void os_start(void);
int timer_handler();
void timer_create(Tick tick);
void delete_task(TaskId task_id);
void create_task(TaskId task_id, Priority priority, void (*task)(void));

/* タスクの定義 */
enum TaskState
{
    RUNNING,
    READY,
    SUSPEND
};

/* task control block */
typedef struct TCB
{
    TaskId task_id_;     /* タスクの識別子 0~255の整数値*/
    TaskState state_;    /* running, ready, suspendの3つの状態を持つ */
    Priority priority_;  /* タスクの優先度(0~9) */
    struct TCB *p_prev_; /* 前のタスク */
    struct TCB *p_next_; /* 次のタスク*/
    void (*task_handler_)(void);
} tcb_t;

tcb_t TCB[TASK_ID_MAX];

typedef struct
{
    unsigned char running_;
    int32_t prev_;
} dispatch_t;

static dispatch_t dispatch;
// 実行可能状態のタスクを登録するリスト
tcb_t ready_que[10];

void create_task(TaskId task_id, Priority priority, void (*task)(void))
{
    tcb_t *p_task = &TCB[task_id];
    p_task->priority_ = priority;
    p_task->task_handler_ = task;
    tcb_t *p_ptr = &ready_que[p_task->priority_];
    p_task->state_ = SUSPEND;

    // レディ・キューの末尾が空になるまでタスクを探索
    while (p_ptr->p_next_ != 0)
    {
        p_ptr = p_ptr->p_next_;
    }
    // レディ・キューの末尾にタスクを追加
    p_ptr->p_next_ = p_task;
    p_task->p_prev_ = p_ptr;
    p_task->p_next_ = 0;
}

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

void os_start(void)
{
    // 次にディスパッチするタスクを登録する
    dispatch.running_ = get_top_ready_id();
    dispatch.prev_ = dispatch.running_;
    TCB[dispatch.running_].state_ = RUNNING;
    sei();
}
int timer_handler()
{
    while (1)
    {
        run_que = dispatch.running_;
        while (run_que <= TASK_ID_MAX - 1)
        {
            // 優先順位順にタスクを実行
            tcb_t temp;
            if (TCB[run_que + 1].priority_ > TCB[run_que].priority_)
            {
                temp = TCB[run_que + 1];
                TCB[run_que + 1] = TCB[run_que];
                TCB[run_que] = temp;
                TCB[run_que].state_ = RUNNING;
            }

            if (run_que > TASK_ID_MAX - 1)
            {
                run_que = TASK_ID0;
            }
            (*TCB[run_que].task_handler_)();
            TCB[run_que].state_ = READY;
            run_que = run_que + 1;
        }
    }
    return 0;
}

void timer_create(Tick tick)
{
    TCCR0A = 0b00000010; // CTC
    TCCR0B = 0b00000001; // 分周なし
    OCR0A = tick;
    TIMSK |= 0b00010000; // 割り込み許可
}

ISR(TIM0_COMPA_vect)
{
    timer_handler();
}

void LED1(void)
{
    digitalWrite(0, !digitalRead(0));
}

void LED2(void)
{
    digitalWrite(1, !digitalRead(1));
}

void LED3(void)
{
    digitalWrite(2, !digitalRead(2));
}

void setup()
{
    pinMode(0, OUTPUT);
    pinMode(1, OUTPUT);
    pinMode(2, OUTPUT);

    create_task(0, 1, LED1);
    create_task(1, 2, LED2);
    create_task(2, 3, LED3);
    os_start();

    timer_create(30);
    while (1)
        ; // 無限ループ（割込み待ち）
}

void loop()
{
}
