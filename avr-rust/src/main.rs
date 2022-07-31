use std::println;

#[allow(dead_code)]
pub struct TCB {
    task:fn(),
    task_id: u8,
    status: u64,
    priority: u128
}


pub fn new (task: fn(),task_id:u8,status:u64,priority:u128)->TCB{
    TCB { task:task,task_id: task_id,status:status,priority: priority}
}
pub fn tcb_task(task:fn()){
    task();
}

static TASK_ID0:u8=0;
static TASK_ID1:u8=1;
static TASK_ID2:u8=2;

static SUSPEND:u64=0;
static WAIT:u64 = SUSPEND+1;
static READY:u64 = SUSPEND+2;

// static OFF:bool=false;
// static ON:bool= true;

struct Que {
    ready_que:u8,
    suspend_que:u8,
    wait_que:u8,
    run_que:u8
}


/* system call */
pub fn create_task(task:fn(),task_id:u8,priority:u128) {
    TCB{task:task,task_id:task_id,status: SUSPEND,priority:priority};
}

fn start_task(task_id:u8, status: u64) {

   let tmp = 1 << task_id; // task_idを1右にシフト 01(1) -> 00(0) 10(2) -> 01(1), 11(3) -> 01(1)
   let mut que:Que = Que{ready_que:0,suspend_que:0,wait_que:0,run_que:0};

    if status == READY
    {
        que.ready_que |= &tmp; // ビットOR後に代入
    }
    if status == SUSPEND
    {
        que.suspend_que |= &tmp;
    } 
    if status == WAIT
    {
        que.wait_que |= &tmp;
    }
}

pub fn wait_task()
{
    let flags:u8;
    let mut que:Que = Que{ready_que:0,suspend_que:0,wait_que:0,run_que:0};
    flags = 1 << que.run_que; /* フラグ操作のための変数を定義  */
    que.ready_que &= !flags;    /* レディー・フラグを落とす */
    que.suspend_que &= !flags;  /* サスペンド・フラグを落とす */
    que.wait_que |= flags;      /* ウェイト・フラグを立てる */
}

pub fn task_a()
{
    println!("taskA");
}

pub fn task_b()
{
    println!("taskB");
}

pub fn task_c()
{
    println!("taskC");
}

/* interrupt handler */
fn main() {
    let mut que:Que = Que{ready_que:0,suspend_que:0,wait_que:0,run_que:0};

    create_task(task_a,TASK_ID0,9);
    create_task(task_b,TASK_ID1,9);
    create_task(task_c,TASK_ID2,9);
    
    start_task(TASK_ID0, READY);
    start_task(TASK_ID1, SUSPEND);
    start_task(TASK_ID2, SUSPEND);
    
    loop {
        if que.run_que == 3 {
            {
                que.run_que = TASK_ID0;
            }
        }
    }
    
} 

// TODO: 構造体配列でスケジューリングを実装