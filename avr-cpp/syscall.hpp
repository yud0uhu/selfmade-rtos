#include "minios.hpp"

#define OFF 0
#define ON OFF + 1

#define TASKA_INTERVAL 10
#define TASKB_INTERVAL 45
#define TASKC_INTERVAL 20

#define SUSPEND 0
#define WAIT SUSPEND + 1
#define READY SUSPEND + 2

inline volatile unsigned short ready_que;
inline volatile unsigned short suspend_que;
inline volatile unsigned char run_que;
inline volatile unsigned short wait_que;

#define TASK_ID_MAX 3

#define TASK_ID0 0
#define TASK_ID1 1
#define TASK_ID2 2

// TCB tcb[TASK_ID_MAX];
