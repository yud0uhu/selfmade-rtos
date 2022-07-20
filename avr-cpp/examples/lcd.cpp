// C++ code
#include "Wire.h"
#include "Adafruit_LiquidCrystal.h"

#include <stdio.h>
#include <stdlib.h>
#include "lcd.hpp"

#define OFF 0
#define ON OFF + 1

#define TASKA_INTERVAL 10
#define TASKB_INTERVAL 45

#define SUSPEND 0
#define WAIT SUSPEND + 1
#define READY SUSPEND + 2

int seconds = 0;

volatile unsigned short ready_que;
volatile unsigned short suspend_que;
volatile unsigned char run_que;
volatile unsigned short wait_que;

Adafruit_LiquidCrystal lcd(10, 9, 8, 7, 6, 5);

void setup()
{
    Serial.begin(9600);
    lcd.begin(16, 2);
    lcd.print("hello world");
}

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

/* task function */
void task_a(void);
void task_b(void);
void task_c(void);

/* system call */
void create_task(unsigned char task_id, void (*task)(void));
void start_task(unsigned char task_id, unsigned short status);
void suspend_task(unsigned char task_id);
void ready_task(unsigned char task_id);
void sleep_task(void);
void wait_task(unsigned short interval);

void wait_task(unsigned short interval)
{
    unsigned short flags;
    flags = (1 << run_que); /* フラグ操作のための変数を定義  */
    ready_que &= ~flags;    /* レディー・フラグを落とす */
    suspend_que &= ~flags;  /* サスペンド・フラグを落とす */
    wait_que |= flags;      /* ウェイト・フラグを立てる */
}

void create_task(unsigned char task_id, void (*task)(void))
{
    tcb[task_id].task = task;
}

void start_task(unsigned char task_id, unsigned short status)
{
    unsigned short tmp;
    tmp = (1 << task_id);
    if (status == READY)
    {
        ready_que |= tmp;
    }
    if (status == SUSPEND)
    {
        suspend_que |= tmp;
    }
    if (status == WAIT)
    {
        wait_que |= tmp;
    }
}

/* task functions */
void task_a(void)
{
    digitalWrite(LED_BUILTIN, HIGH);
    wait_task(TASKA_INTERVAL);
    Serial.print("taskA");
}

void task_b(void)
{
    digitalWrite(LED_BUILTIN, HIGH);
    wait_task(TASKB_INTERVAL);
    // Serial.print("taskB");
}

void task_c(void)
{
    digitalWrite(LED_BUILTIN, HIGH);
    wait_task(TASKB_INTERVAL);
    // Serial.print("taskC");
}

void task_d(void)
{
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print(millis() / 1000);
    // Turn off the blinking cursor:
    lcd.noBlink();
    delay(3000);
    // Turn on the blinking cursor:
    lcd.blink();
    delay(3000);
}

/* initialize */
void init_os(void)
{
    ready_que = 0;
    suspend_que = 0;
    wait_que = 0;
    run_que = 0;
}

/* interrupt handler */
void loop()
{
    TCB tcb_task;
    init_os();
    create_task(TASK_ID0, task_a);
    create_task(TASK_ID1, task_b);
    create_task(TASK_ID2, task_d);

    start_task(TASK_ID0, READY);
    start_task(TASK_ID1, SUSPEND);
    start_task(TASK_ID2, SUSPEND);

    run_que = TASK_ID0;

    while (ON)
    {
        tcb_task = tcb[run_que];
        (*(tcb_task.task))();
        run_que++;
        if (run_que == TASK_ID_MAX)
        {
            run_que = TASK_ID0;
        }
    }
}