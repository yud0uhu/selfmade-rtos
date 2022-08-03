
#include <stdio.h>
#include <stdlib.h>
#include "minios.hpp"
#include "syscall.hpp"
#include <queue>
#include <iostream>

/* task control block */

class tcb
{
  void (*task)(void);             /* タスク関数の実行開始アドレス */
  unsigned char task_id;          /* タスクの識別子 */
  volatile unsigned short status; /* ready, suspend runの3つの状態を持つ */
  volatile int priority;          /* タスクの優先度(0~9) */
public:
  tcb()
  {
    task_id = 0;
    priority = 0;
  }
  tcb(unsigned char t, volatile int p)
  {
    task_id = t;
    priority = p;
  }

  unsigned char getTaskId() const { return task_id; }
  volatile int getPriority() const { return priority; }
};

bool operator<(const tcb &a, const tcb &b)
{
  return a.getPriority() < b.getPriority();
}

void wait_task(unsigned short interval)
{
  unsigned short flags;
  flags = (1 << run_que); /* フラグ操作のための変数を定義  */
  ready_que &= ~flags;    /* レディー・フラグを落とす */
  suspend_que &= ~flags;  /* サスペンド・フラグを落とす */
  wait_que |= flags;      /* ウェイト・フラグを立てる */
}

void create_task(unsigned char task_id, void (*task)(void), volatile int priority)
{
  std::priority_queue<tcb> tcb_que;
  tcb_que.push(tcb(task_id, priority));

  std::cout << "Priorities: ";
  while (!tcb_que.empty())
  {
    std::cout << tcb_que.top().getPriority() << "\n";
    tcb_que.pop();
  }
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