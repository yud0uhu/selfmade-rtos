
#include <stdio.h>
#include <stdlib.h>
#include "minios.hpp"
#include "syscall.hpp"
#include <queue>
#include <iostream>
#include <functional>
#include <memory>
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

std::priority_queue<PriorityQue> tcb_que;

bool operator<(const PriorityQue &a, const PriorityQue &b)
{
  return a.priority < b.priority;
}

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
  tcb_que.push(PriorityQue(task_id, status, task, priority));
}

void start_task(unsigned char task_id, unsigned short status)
{
  unsigned short flags;
  flags = (1 << task_id);
  if (status == READY)
  {
    ready_que |= flags;
  }
  if (status == SUSPEND)
  {
    suspend_que |= flags;
  }
  if (status == WAIT)
  {
    wait_que |= flags;
  }
}

void scheduling(void)
{
  while (!tcb_que.empty())
  {
    /* Debug
     * std::cout << (int)tcb_que.top().task_id << ": " << tcb_que.top().priority << "\n";
     */
    void (*run)() = tcb_que.top().task_;
    run();
    tcb_que.pop();
  }
}