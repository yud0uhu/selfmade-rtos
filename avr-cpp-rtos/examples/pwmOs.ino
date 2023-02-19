/* Arduino Uno */

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

/*タイマーハンドラ用*/
#include <avr/io.h>
#include <avr/interrupt.h>

void os_start(void);
int timer_handler();
void timer_create(unsigned int tick);
void delete_task(unsigned char task_id);

void TaskRelay(void *pvParameters);
void TaskPWM(void *pvParameters);
void TaskDisplay(void *pvParameters);

unsigned char i = 0;
#define R 47.0

/* タスクの定義 */
#define THRESHOLD 100
#define RELAY_DELAY 0
#define PWM_DELAY 0
int iTarget = 80;
int iPWM = 128;
float fP_error = 0.0;
bool flg = true;
bool previous_flag = true;

enum TaskState
{
    RUNNING,
    READY,
    SUSPEND
};

void create_task(unsigned char task_id, Priority priority, void (*task)(void));
/* task control block */
typedef struct TCB
{
    unsigned char task_id_; /* タスクの識別子 0~255の整数値*/
    TaskState state_;       /* running, ready, suspendの3つの状態を持つ */
    Priority priority_;     /* タスクの優先度(0~9) */
    struct TCB *p_prev_;    /* 前のタスク */
    // uint32_t *p_stack_;  /* TODO 各タスクのスタック領域の先頭アドレスを保持 */
    struct TCB *p_next_; /* 次のタスク*/
    // uint32_t time_up; /* TODO タイマーレジスタの状態をスタックに保存*/
    void (*task_handler_)(void);
} tcb_t;

tcb_t TCB[TASK_ID_MAX];
typedef struct
{
    unsigned char running;
    int32_t prev;
} sysinfo_t;

static sysinfo_t _sysinfo;
// 優先度ごとのリスト
tcb_t ready_que[10];
// 待ち状態のタスクを登録するリスト
tcb_t queue_waitng;

void create_task(unsigned char task_id, Priority priority, void (*task)(void))
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
void delete_task(unsigned char task_id)
{
    tcb_t *p_task = &TCB[task_id];
    p_task->p_prev_->p_next_ = p_task->p_next_;
    p_task->p_next_->p_prev_ = p_task->p_prev_;
    p_task->p_prev_ = 0;
    p_task->p_next_ = 0;
}
unsigned char get_top_ready_id(void)
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
    _sysinfo.running = get_top_ready_id();
    _sysinfo.prev = _sysinfo.running;
    TCB[_sysinfo.running].state_ = RUNNING;
    sei();
}
TaskId run_que;
int timer_handler()
{
    while (1)
    {
        run_que = _sysinfo.running;
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

void timer_create(unsigned int tick)
{
    TCCR1A = 0; // 初期化
    TCCR1B = 0; // 初期化
    OCR1A = tick;
    TCCR1B |= (1 << CS12) | (1 << WGM12); // CS12 -> 1(prescaler -> 256)   CTC mode on
    // OCIEA -> 1 (enable OCR1A Interrupt)   OCIEB -> 1 (enable OCR1B Interript)
    TIMSK1 = (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect)
{
    timer_handler();
}

void setup()
{
    pinMode(4, OUTPUT);
    pinMode(3, OUTPUT);
    Serial.begin(9600);

    create_task(0, 3, TaskPWM);
    create_task(1, 2, TaskDisplay);
    create_task(2, 1, TaskRelay);
    os_start();

    // クロック周波数ベースのタイマ割り込み処理を初期化
    // timer_createの引数にカウンターの周期を指定
    // 62500*256(prescaler)/16MHz = 1秒
    // 250000*256/16000000 = 4s
    timer_create(30);
    while (1)
        ; // 無限ループ（割込み待ち）
}

void TaskRelay(void *pvParameters)
{ // Relay control
    (void)pvParameters;
    if (iPWM < THRESHOLD - 1)
        flg = true;
    if (iPWM > THRESHOLD)
        flg = false;
    switch (flg)
    {
    case true:
        digitalWrite(4, HIGH);
        if (previous_flag == false)
        {
            previous_flag = true;
            Serial.println("RELAY ON Control");
        }
        break;
    case false:
        digitalWrite(4, LOW);
        if (previous_flag == true)
        {
            previous_flag = false;
            Serial.println("RELAY OFF Control");
        }
        break;
    }
}
void TaskPWM(void *pvParameters)
{ // PWM control
    (void)pvParameters;
    int iMonitor = analogRead(A1);
    fP_error = (float)(iMonitor - iTarget) / 3;
    iPWM -= (int)(fP_error);
    if (iPWM > 255)
        iPWM = 255;
    if (iPWM < 0)
        iPWM = 0;
    analogWrite(3, iPWM);
    Serial.println("PWM Control");
}
void TaskDisplay(void *pvParameters)
{ // PWM control
    (void)pvParameters;
    Serial.println("D");
}

void loop()
{
}