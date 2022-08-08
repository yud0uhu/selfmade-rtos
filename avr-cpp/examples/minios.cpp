#define OFF 0
#define ON OFF + 1

#define TASKA_INTERVAL 10
#define TASKB_INTERVAL 45
#define TASKC_INTERVAL 20

#define TASK_ID_MAX 32

#define TASK_ID0 0
#define TASK_ID1 1
#define TASK_ID2 2

typedef unsigned char TaskId;
typedef unsigned char Priority;

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
enum TaskState
{
    RUNNING,
    WAITING,
    READY
};

/* task control block */
class Task
{
public:
    TaskId task_id_;    /* タスクの識別子 0~255の整数値*/
    TaskState state_;   /* running, waiting, readyの3つの状態を持つ */
    Priority priority_; /* タスクの優先度(0~9) */
    // int *pstack_;               /* タスクのスタック領域の先頭アドレスを保持 */
    Task *pnext_;                /* レディー・キューへの接続用ポインタ */
    void (*task_handler_)(void); /* 関数ポインタの実行開始アドレス */
    static void create_task(TaskId task_id, TaskState state, Priority priority, void (*task_handler)());
    static int pop_task(Task *phead_, Task *ptail_);
    static void delete_task(void);
    Task *init_task(void);
};
static Task *pcurrent; /* カレント・タスク */
static unsigned int readyque_bitmap;

typedef struct
{
    Task *phead_;
    Task *ptail_;
} ReadyQue;
static ReadyQue ready_que;
static void scheduler(void);

Task *Task::init_task(void)
{
    struct Task *ptask_ = (Task *)malloc(sizeof(Task));
    ready_que.phead_ = NULL;
    ready_que.ptail_ = NULL;

    return ptask_;
}

void Task::create_task(TaskId task_id, TaskState state, Priority priority, void (*task_handler)())
{

    struct Task *pnew = (Task *)malloc(sizeof(Task));
    pnew->pnext_ = NULL;
    pnew->task_id_ = task_id;
    pnew->state_ = state;
    pnew->priority_ = priority;
    pnew->task_handler_ = task_handler;
    // redey_que.ptail_->pnext_ = pnew; // セグフォ
    ready_que.ptail_ = pnew;
    return;
}

bool task_is_empty(void)
{
    return ready_que.phead_ == NULL;
}

int Task::pop_task(Task *phead_, Task *ptail_)
{
    struct Task *pold_;

    if (phead_ == NULL)
        return -1; /*アンダーフロー*/

    // phead_ = phead_->pnext_; // セグフォ
    int priority = pold_->priority_;
    free(pold_);

    return priority;
}

static void scheduler(void)
{
#if TASK_ID_MAX > 32
#error ビットマップを配列化する必要あり
#endif
    unsigned int bitmap = readyque_bitmap;
    int n = 0;
    static const int bitmap2num[16] =
        {
            -32, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
    if (!(bitmap & 0xffff))
    {
        bitmap >>= 16;
        n += 16;
    }
    if (!(bitmap & 0xff))
    {
        bitmap >>= 8;
        n += 8;
    }
    if (!(bitmap & 0xf))
    {
        bitmap >>= 4;
        n += 4;
    }
    n += bitmap2num[bitmap & 0xf];
    if (n < 0)
    {
        // exit(0);
    }
    // TODO:タスク操作と紐付け
}

void task_a(void)
{
    printf("taskId:%d", pcurrent->task_id_);
    printf("priority:%d", pcurrent->priority_);
    return;
}
void task_b(void)
{
    printf("%d", pcurrent->task_id_);
    printf("priority:%d", pcurrent->priority_);
    return;
}
static Task task;
int main(int argc, char *argv[])
{
    Task task;
    Task *phead_;
    Task *ptail_;

    // タスクを初期化
    task.init_task();

    // タスクを作成
    task.create_task(1, READY, 0, task_a);
    task.create_task(2, READY, 2, task_b);

    while (ON)
    {
        if (task_is_empty)
        {
            printf("priority:%d", task.pop_task(phead_, ptail_));
            // (*(pcurrent->task_handler_))();
            // 優先度に応じてスケジューリング処理
        }
        else
        {
            exit(0);
            printf("threads is NULL");
        }
        return 0;
    }
}