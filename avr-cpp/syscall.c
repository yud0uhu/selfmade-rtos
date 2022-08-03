#include <stdio.h>
#include <stdlib.h>

#include  "minios.h"
#include "syscall.h"

void wait_task(unsigned short interval)
{
  unsigned short flags;
  flags = (1 << run_que); /* フラグ操作のための変数を定義  */
  ready_que &= ~flags;    /* レディー・フラグを落とす */
  suspend_que &= ~flags;  /* サスペンド・フラグを落とす */
  wait_que |= flags;      /* ウェイト・フラグを立てる */
}

void create_task(unsigned char task_id, void (*task)(void),int priority)
{
  tcb[task_id].task = task;
  tcb[task_id].TASK_ID = task_id;
  tcb[task_id].PRIORITY = priority;
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