#ifndef INCLUDED_minios_h_

#define INCLUDED_minios_h_

/* systemcall */
void create_task(unsigned char task_id, void (*task)(void), volatile int priority);
void start_task(unsigned char task_id, unsigned short status);
void suspend_task(unsigned char task_id);
void ready_task(unsigned char task_id);
void sleep_task(void);
void wait_task(unsigned short interval);
void scheduling(void);
void init_os(void);
#endif