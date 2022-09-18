// #include "Adafruit_LEDBackpack.h"
#include <Arduino.h>

#include <iostream>
#include <functional>
#include <memory>
#include <set>

#define OFF 0
#define ON OFF + 1

#define TASKA_INTERVAL 10
#define TASKB_INTERVAL 45
#define TASKC_INTERVAL 20

#define SUSPEND 0
#define WAIT SUSPEND + 1
#define READY SUSPEND + 2

#define TASK_ID_MAX 3

#define TASK_ID0 0
#define TASK_ID1 1
#define TASK_ID2 2

int seconds = 0;

volatile unsigned short ready_que;
volatile unsigned short suspend_que;
volatile unsigned char run_que;
volatile unsigned short wait_que;
void create_task(unsigned char task_id, volatile unsigned short status, void (*task)(void), volatile int priority);
void delete_task(unsigned char task_id);
void suspend_task(unsigned char task_id);
void ready_task(unsigned char task_id);
void sleep_task(void);
void wait_task(unsigned short interval);
void scheduling(void);
void init_os(void);

#define R 47.0

int iPWM = 128;
int iTarget = 98;

void setup()
{
  Serial.begin(9600);
  pinMode(3, OUTPUT);
}

/* task control block */

class TCB
{
public:
  void (*task_)(void);            /* 関数ポインタの実行開始アドレス */
  unsigned char task_id;          /* タスクの識別子 */
  volatile unsigned short status; /* ready, suspend runの3つの状態を持つ */
  volatile int priority;          /* タスクの優先度(0~9) */
  void setTask(void (*task)(void))
  {
    task_ = task;
  }
};

class PriorityQue : public TCB
{
public:
  PriorityQue(unsigned char _task_id, volatile unsigned short _status, void (*task)(void), volatile int _priority)
  {
    this->task_id = _task_id;
    this->status = _status;
    this->setTask(task);
    this->priority = _priority;
  }
};

std::multiset<PriorityQue> tcb_que;

// bool operator<(const PriorityQue &a, const PriorityQue &b)
// {
//   return a.priority < b.priority;
// }

void wait_task(unsigned short interval)
{
  unsigned short flags;
  flags = (1 << run_que); /* フラグ操作のための変数を定義  */
  ready_que &= ~flags;    /* レディー・フラグを落とす */
  suspend_que &= ~flags;  /* サスペンド・フラグを落とす */
  wait_que |= flags;      /* ウェイト・フラグを立てる */
}

void create_task(unsigned char task_id, volatile unsigned short status, void (*task)(void), volatile int priority)
{
  tcb_que.insert(PriorityQue(task_id, status, task, priority));
}

void delete_task(unsigned char task_id)
{
  for (auto it = tcb_que.begin(); it != tcb_que.end();)
  {
    if ((*it).task_id == task_id)
    {
      it = tcb_que.erase(it);
    }
    else
    {
      ++it;
      // arx::cout << (*it).task_id << arx::endl;
    }
  }
}

void scheduling(void)
{
  for (auto it = tcb_que.rbegin(); it != tcb_que.rend();)
  {
    void (*run)() = it->task_;
    run();
    ++it;
  }
}

/* task functions */
void task_a(void)
{
  // digitalWrite(0, HIGH);
  wait_task(TASKA_INTERVAL);
  // digitalWrite(0, LOW);
  Serial.print("taskA");
  delay(50);
}

void task_b(void)
{
  digitalWrite(LED_BUILTIN, HIGH);
  wait_task(TASKB_INTERVAL);
  Serial.print("taskB");
}

void task_c(void)
{
  digitalWrite(LED_BUILTIN, HIGH);
  wait_task(TASKB_INTERVAL);
  Serial.print("taskC");
}

void task_d(void)
{
  // int iM1 = analogRead(A1);
  // iPWM = iPWM - (int)((float)(iM1 - iTarget) / 5.0);
  // analogWrite(3, iPWM);
  // float fCurrent = (float)iM1 / 1.023 * 5.0 / R;
  // Serial.print(fCurrent);
  // Serial.println(" mA");
  // delay(50);
}

/* initialize */
void init_os()
{
  ready_que = 0;
  suspend_que = 0;
  wait_que = 0;
  run_que = 0;
}

int main(void)
{
  init_os();

  create_task(TASK_ID0, READY, task_a, 2);
  create_task(TASK_ID1, READY, task_b, 5);
  create_task(TASK_ID2, READY, task_c, 1);

  // delete_task(TASK_ID1);

  scheduling();
  return 0;
}