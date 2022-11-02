// 通信処理のヘッダファイル
#include "defines.h"
#include "intr.h"
#include "interrupt.h"
#include "lib.h"
#include "rtl8019.h"
#include "ethernet.h"
#include "netdrv.h"

#include "ip.h"
#include "icmp.h"

// OSシステムコールのヘッダファイル
void *kz_kmalloc(int size);
int kz_kmfree(void *p);
int kz_send(kz_msgbox_id_t id, int size, char *p);

// OSサービスコールのヘッダファイル
void *kx_kmalloc(int size);
int kx_kmfree(void *p);
int kx_send(kz_msgbox_id_t id, int size, char *p);

// TODO: サービスコールとシステムコールの違いを掘り下げる

// Atmega328P Timer1による実装
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#define TASK_ID_MAX 10
#define TASK_PRIORITY_NUMBER 10

typedef volatile uint8_t Priority;

void all_set_task(struct task_define *tasks);
void create_task(volatile unsigned char task_id, Priority priority, void (*task)(void));
void context_switch(void);
unsigned char get_top_ready_id(void);
void os_start(void);
struct TCB *readyqueue_deque(Priority priority);
void readyqueue_enque(struct TCB *p_tcb_);
void schedule(void);
void timer_create(void);
void task_reload(void);
void task_start(void);
void init_all(void);

struct task_define
{
    volatile unsigned char task_id_;
    Priority priority_;
    void (*task_handler_)(void);
};
struct task_define tasks[] = {};

enum TaskState
{
    RUNNING,
    READY,
    SUSPEND
};

typedef struct TCB
{
    volatile unsigned char task_id_;
    TaskState state_;
    Priority priority_;
    struct TCB *p_prev_;
    struct TCB *p_next_;
    void (*task_handler_)(void);
} tcb_t;

tcb_t TCB[TASK_ID_MAX];

tcb_t ready_que[TASK_PRIORITY_NUMBER];
tcb_t *current;

struct TCB *readyqueue_deque(Priority priority)
{
    struct TCB *p_ready_que_ = &ready_que[priority];

    struct TCB *p_tcb_;

    p_tcb_ = p_ready_que_->p_next_;

    if (p_tcb_)
    {
        p_ready_que_->p_next_ = p_tcb_->p_next_;
        if (p_ready_que_->p_next_ == 0)
        {
            p_ready_que_->p_prev_ = 0;
        }
        p_tcb_->p_next_ = 0;
    }

    return p_tcb_;
}

void readyqueue_enque(struct TCB *p_tcb_)
{
    tcb_t *p_ready_que_ = &ready_que[p_tcb_->priority_];

    p_tcb_->p_next_ = 0;

    if (p_ready_que_->p_prev_)
    {
        p_ready_que_->p_prev_->p_next_ = p_tcb_;
    }
    else
    {
        p_ready_que_->p_next_ = p_tcb_;
    }
    p_ready_que_->p_prev_ = p_tcb_;
}

void schedule(void)
{
    uint8_t i;
    for (i = 0; i < TASK_ID_MAX; i++)
    {
        current = readyqueue_deque(i);
        if (current)
        {
            return;
        }
    }
}

void create_task(volatile unsigned char task_id, Priority priority, void (*task)(void))
{
    if (task_id >= TASK_ID_MAX)
        exit(1);

    current = &TCB[task_id];
    current->task_id_ = task_id;
    current->priority_ = priority;
    current->task_handler_ = task;
    current->state_ = READY;

    readyqueue_enque(current);

    return;
}

void context_switch(void)
{
    schedule();
    if (current->state_ == READY)
    {
        readyqueue_enque(current);
        current->state_ = RUNNING;
    }
    else
        task_reload();
    schedule();
}

unsigned char get_top_ready_id(void)
{
    Priority priority_;
    for (priority_ = 0; priority_ < TASK_PRIORITY_NUMBER; priority_++)
    {
        if (ready_que[priority_].p_next_ != 0)
        {
            return ready_que[priority_].p_next_->task_id_;
        }
    }
    return 0;
}

void os_start(void)

{
    if (get_top_ready_id() == 0)
    {
        task_reload();
    }
    current->task_id_ = get_top_ready_id();
    current->state_ = READY;
    context_switch();
    task_start();
    sei();
}

void task_start()
{
    if (current->task_id_ != 0)
    {
        (*current->task_handler_)();
    }
}

void timer_init(void)
{
    TCNT1 = 3036;
}

void timer_create(void)
{
    cli();
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TCNT1 = 3036;
    TCCR1B |= _BV(CS12);  // 256分周, CTCモード
    TIMSK1 |= _BV(TOIE1); // オーバーフロー割り込みを許可
}

ISR(TIMER1_OVF_vect)
{
    os_start();
}

void init_all(void)
{
    memset(TCB, 0, sizeof(TCB));
    memset(current, 0, sizeof(current));
}

void task_reload()
{
    all_set_task(tasks);
}

/* ネットワーク機能の実装 */
/* 受信割り込みハンドラ */
static void netdrv_intr(void)
{
    struct netbuf *pkt;
    if (rtl8019_checkintr(0))
    {
        pkt = kx_kmalloc(DEFAULT_NETBUF_SIZE);
        memset(pkt, 0, DEFAULT_NETBUF_SIZE);
        /* ETHERNET_CMD_RECV: メッセージ通信で送信するコマンド
         * このメッセージボックスは、ethernetタスクによって受信される
         */
        pkt->cmd = ETHERNET_CMD_RECV;

        /*
         * ethernetフレームが14バイトで４の倍数でないので，ethernetフレーム以降が
         * ４バイトアラインメントされるように，データの先頭を２バイト空ける．
         */
        pkt->top = pkt->data + 2;

        pkt->size = rtl8019_recv(0, pkt->top);
        if (pkt->size > 0)
            /* MSGBOX_ID_ETHPROC: フレームを受信するメッセージボックス */
            kx_send(MSGBOX_ID_ETHPROC, 0, (char *)pkt);
        else
            kx_kmfree(pkt);
    }
}

/* ethernetタスクの受信部分 */

static int ethernet_recv(struct netbuf *pkt)
{
    struct ethernet_header *ethhdr = (struct ethernet_header *)pkt->top;

    if (!initialized)
        return 0;

    if (!(ethhdr->dst_addr[0] & 1) &&
        memcmp(ethhdr->dst_addr, my_macaddr, MACADDR_SIZE))
        return 0;

    pkt->top += ETHERNET_HEADER_SIZE;
    pkt->size -= ETHERNET_HEADER_SIZE;

    // ARPパケットならばarpタスクに、IPパケットならばipタスクに受信したパケットを転送する
    switch (ethhdr->type)
    {
    case ETHERNET_TYPE_ARP:
        pkt->cmd = ARP_CMD_RECV;
        kz_send(MSGBOX_ID_ARPPROC, 0, (char *)pkt);
        break;
    case ETHERNET_TYPE_IP:
        pkt->cmd = IP_CMD_RECV;
        kz_send(MSGBOX_ID_IPPROC, 0, (char *)pkt);
        break;
    default:
        return 0;
    }

    return 1;
}

static int ethernet_proc(struct netbuf *buf)
{
    int ret = 0;
    switch (buf->cmd)
    {
    case ETHERNET_CMD_MACADDR:
        memcpy(my_macaddr, buf->option.common.macaddr.addr, MACADDR_SIZE);
        buf->cmd = ARP_CMD_MACADDR;
        kz_send(MSGBOX_ID_ARPPROC, 0, (char *)buf);
        initialized++;
        ret = 1;
        break;
        // フレーム受信の時
    case ETHERNET_CMD_RECV:
        ret = ethernet_recv(buf);
        break;
    case ETHERNET_CMD_SEND:
        ret = ethernet_send(buf);
        break;
    default:
        break;
    }
    return ret;
}

int ethernet_main(int argc, char *argv[])
{
    struct netbuf *buf;
    int ret;

    buf = kz_kmalloc(sizeof(*buf));
    buf->cmd = NETDRV_CMD_USE;
    kz_send(MSGBOX_ID_NETPROC, 0, (char *)buf);

    while (1)
    {
        // kz_recv()でメッセージの受信を待ち
        kz_recv(MSGBOX_ID_ETHPROC, NULL, (char **)&buf);
        // ethernet_proc()に受信したメッセージを渡す
        // フレーム受信ならethernet_recv()が呼ばれる
        ret = ethernet_proc(buf);
        if (!ret)
            kz_kmfree(buf);
    }

    return 0;
}

/* ipタスクの受信部分 */
static int ip_recv(struct netbuf *pkt)
{
    struct ip_header *iphdr;
    int hdrlen;

    iphdr = (struct ip_header *)pkt->top;

    if (((iphdr->v_hl >> 4) & 0xf) != 4)
        return 0;
    if (iphdr->dst_addr != my_ipaddr)
        return 0;
    if (iphdr->protocol > PROTOCOL_MAXNUM) /* 未サポートのプロトコル */
        return 0;
    if (!protoinfo[iphdr->protocol].id) /* 登録されていないプロトコル */
        return 0;

    hdrlen = (iphdr->v_hl & 0xf) << 2;

    if (pkt->size > iphdr->length)
        pkt->size = iphdr->length;

    pkt->top += hdrlen;
    pkt->size -= hdrlen;

    pkt->cmd = protoinfo[iphdr->protocol].cmd;
    pkt->option.common.ipaddr.addr = iphdr->src_addr;
    kz_send(protoinfo[iphdr->protocol].id, 0, (char *)pkt);
    return 1;
}

static int ip_proc(struct netbuf *buf)
{
    struct protoinfo *info;
    int ret = 0;

    // パケット受信ならばip_recv()が呼ばれ、登録されているプロトコルに応じて対応するメッセージボックスに転送される
    switch (buf->cmd)
    {
    case IP_CMD_REGPROTO:
        info = &protoinfo[buf->option.ip.regproto.protocol];
        info->cmd = buf->option.ip.regproto.cmd;
        info->id = buf->option.ip.regproto.id;
        break;

    case IP_CMD_RECV:
        ret = ip_recv(buf);
        break;

    case IP_CMD_SEND:
        ret = ip_send(buf);
        break;

    default:
        break;
    }

    return ret;
}

int ip_main(int argc, char *argv[])
{
    struct netbuf *buf;
    int ret;

    buf = kz_kmalloc(sizeof(*buf));
    buf->cmd = ARP_CMD_IPADDR;
    buf->option.common.ipaddr.addr = IPADDR;
    kz_send(MSGBOX_ID_ARPPROC, 0, (char *)buf);

    while (1)
    {
        // kz_recv()でメッセージ受信待ち
        kz_recv(MSGBOX_ID_IPPROC, NULL, (char **)&buf);
        // 受信するとip_proc()が呼ばれる
        ret = ip_proc(buf);
        if (!ret)
            kz_kmfree(buf);
    }
}

/* icmpタスク */
// ICMPのパケット受信の場合にはicmp_recv()が呼ばれる
static int icmp_recv(struct netbuf *pkt)
{
    struct icmp_header *icmphdr;

    icmphdr = (struct icmp_header *)pkt->top;

    /* 正式にはコンソールドライバタスクに依頼する必要があるが，とりあえず */
    puts("ICMP received: ");
    putxval(pkt->option.common.ipaddr.addr, 8);
    puts(" ");
    putxval(icmphdr->type, 2);
    puts(" ");
    putxval(icmphdr->code, 2);
    puts(" ");
    putxval(icmphdr->checksum, 4);
    puts("\n");

    // ICMP Echoならば、icmp_recv()内でicmp_sendpkt()が呼ばれて応答パケットを作成し、ipタスクに転送することで送信依頼をする
    switch (icmphdr->type)
    {
    // ICMP Echo Reply
    case ICMP_TYPE_REPLY:
        break;
    // ICMP Echo Request
    case ICMP_TYPE_REQUEST:
        icmp_sendpkt(pkt->option.common.ipaddr.addr,
                     ICMP_TYPE_REPLY, icmphdr->code,
                     icmphdr->param.id, icmphdr->param.sequence_number,
                     pkt->size - sizeof(struct icmp_header),
                     (char *)icmphdr + sizeof(*icmphdr));
        break;
    }

    return 0;
}
/* ネットワーク機能の実装 */

void LED1(void)
{
    digitalWrite(2, !digitalRead(2));
}

void LED2(void)
{
    digitalWrite(3, !digitalRead(3));
}

void LED3(void)
{
    digitalWrite(4, !digitalRead(4));
}

void all_set_task(struct task_define *tasks)
{
    for (int i = 0; i < sizeof(tasks) + 1; i++)
    {
        create_task(tasks[i].task_id_, tasks[i].priority_, tasks[i].task_handler_);
    }
}
void setup()
{

    Serial.begin(115200);
    init_all();

    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);

    tasks[0] = {2, 2, LED1};
    tasks[1] = {3, 3, LED2};
    tasks[2] = {4, 4, LED3};

    all_set_task(tasks);

    timer_create();
    os_start();

    while (1)
        ; // 無限ループ（割込み待ち）
}

void loop()
{
}