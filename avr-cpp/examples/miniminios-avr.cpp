#define OFF 0
#define ON OFF + 1

#define TASKA_INTERVAL 10
#define TASKB_INTERVAL 45
#define TASKC_INTERVAL 20

#define TASK_ID_MAX 3

#define TASK_ID0 0
#define TASK_ID1 1
#define TASK_ID2 2

// typedef unsigned char TaskId;
typedef unsigned int Priority;

#include <stdio.h>
#include <stdlib.h>

/*タイマーハンドラ用*/
#include <avr/io.h>
#include <avr/interrupt.h> //割り込みを使用するため

void task_init(unsigned char id);
void os_start(void);
int timer_handler();
unsigned char i = 0;

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
    // uint32_t *p_stack_;  /* 各タスクのスタック領域の先頭アドレスを保持 */
    struct TCB *p_next_; /* 次のタスク*/
    // uint32_t time_up;
    void (*task_handler_)(void);
} tcb_t;

tcb_t TCB[TASK_ID_MAX];
typedef struct
{
    unsigned char running;
    // int32_t systime;
    int32_t prev;
} sysinfo_t;

static sysinfo_t _sysinfo;
// 優先度ごとのリスト
tcb_t ready_que[10];
// 待ち状態のタスクを登録するリスト
tcb_t queue_waitng;

// vTaskCode, "NAME", STACK_SIZE, NULL, tskIDLE_PRIORITY, 　&xHandle
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

void task_init(unsigned char id)
{
    TCB[id].task_id_ = id;
    TCB[id].state_ = READY;
}

void os_start(void)
{
    _sysinfo.running = get_top_ready_id();
    _sysinfo.prev = _sysinfo.running;
    TCB[_sysinfo.running].state_ = RUNNING;
}
unsigned char run_que;
int timer_handler()
{
    while (1)
    {
        // 優先順位順にタスクを実行
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
void init_timer(void)
{
    // TCCR1A・TCCR1B・TCCR1CはPWMの動作を制御するレジスタ
    // https://garretlab.web.fc2.com/arduino/inside/hardware/arduino/avr/cores/arduino/wiring_analog.c/analogWrite.html
    TCCR1A = 0x00;
    TCCR1B = _BV(WGM12) | _BV(CS11) | _BV(CS10);
    TCCR1C = 0x00;
    // OCR1AH・OCR1AL・OCR1BH・OCR1BLは16ビットの比較レジスタ
    OCR1AH = 0x98;
    OCR1AL = 0x97; // OCR1A=39,063=0x9897
    OCR1BH = 0x0F; // OCR1B=3,906=-0x0F42
    OCR1BL = 0x42;
    /*
      TIMSK1のOCE1AとOCIE1Bを1にすることで割り込みの許可を与える
      _BV()は中身を1に、~_BV()は0にする
    */
    TIMSK1 = (_BV(OCIE1B) | _BV(OCIE1A));
    sei();
}

ISR(TIMER1_COMPA_vect)
{
    timer_handler();
}

ISR(TIMER1_COMPB_vect)
{
    // 何かの割り込みタスク
}
void task_a(void)
{
    Serial.print("taskA");
}
void task_b(void)
{
    Serial.print("taskB");
}
void task_c(void)
{
    Serial.print("taskC");
}
void setup()
{
    pinMode(10, OUTPUT); // 出力に設定
}
int main(void)
{
    Serial.begin(9600);
    DDRB = _BV(DDB5); // PORTB bit5を出力に設定

    create_task(0, 3, task_a);
    create_task(1, 5, task_b);
    create_task(2, 2, task_c);
    os_start();

    // クロック周波数ベースのタイマ割り込み処理を初期化
    init_timer();
    while (1)
        ; // 無限ループ（割込み待ち）
}