
#include <stdio.h>
#include <stdlib.h>
#include "minios.hpp"
#include "syscall.hpp"
#include <iostream>

void run(TCB *ptcb)
{
  ptcb->entry_point_();

  // minios.ready_list_.remove_(ptcb);
  delete ptcb->pstask_;

  // scheduler();
}

TCB::TCB(void (*function)(), Priority priority, int stack_size)
{
  // タスク固有のデータの初期化をする
  stack_size /= sizeof(int);
  task_id_ = TCB::next_task_id_++;
  state_ = READY;
  priority_ = priority;
  entry_point_ = function;
  pstask_ = new int[stack_size];
  pnext_ = NULL;

  // TODO:タスクを実行可能リストに挿入する

  // スケジューリングを開始する
}
