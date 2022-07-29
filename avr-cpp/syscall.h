#include  "minios.h"

#define OFF 0
#define ON OFF + 1

#define TASKA_INTERVAL 10
#define TASKB_INTERVAL 45
#define TASKC_INTERVAL 20

#define SUSPEND 0
#define WAIT SUSPEND + 1
#define READY SUSPEND + 2

volatile unsigned short ready_que;
volatile unsigned short suspend_que;
volatile unsigned char run_que;
volatile unsigned short wait_que;

/* task control block */
typedef struct
{
    void (*task)(void);             /* タスク関数の実行開始アドレス */
    unsigned char TASK_ID;          /* タスクの識別子 */
    volatile unsigned short STATUS; /* ready, suspend runの3つの状態を持つ */
    volatile int PRIORITY;          /* タスクの優先度(0~9) */
} TCB;

#define TASK_ID_MAX 3

#define TASK_ID0 0
#define TASK_ID1 1
#define TASK_ID2 2

TCB tcb[TASK_ID_MAX];
