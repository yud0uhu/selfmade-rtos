#ifndef INCLUDED_minios_h_

#define INCLUDED_minios_h_

/* task control block */
typedef struct TCB
{
    void (*task)(void);             /* タスク関数の実行開始アドレス */
    unsigned char TASK_ID;          /* タスクの識別子 */
    volatile unsigned short STATUS; /* ready, suspend runの3つの状態を持つ */
    volatile int PRIORITY;          /* タスクの優先度(0~9) */
    struct TCB *next;               /* 次の構造体を示すポインタ */
} TCB;

/* systemcall */
void create_task(unsigned char task_id, void (*task)(void), volatile int priority);
void start_task(unsigned char task_id, unsigned short status);
void suspend_task(unsigned char task_id);
void ready_task(unsigned char task_id);
void sleep_task(void);
void wait_task(unsigned short interval);
void scheduling(void);

#endif