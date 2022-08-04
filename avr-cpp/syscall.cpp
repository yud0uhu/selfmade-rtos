
#include <stdio.h>
#include <stdlib.h>
#include "minios.hpp"
#include "syscall.hpp"
#include <queue>
#include <iostream>
#include <functional>
#include <memory>
#include <set>
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
      std::cout << (*it).task_id << std::endl;
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
