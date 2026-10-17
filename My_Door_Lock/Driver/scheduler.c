#include "scheduler.h"

typedef struct
{
    void (*task_func)(void);
    unsigned long int rate_ms;
    unsigned long int last_ms;
} task_t;

task_t scheduler_task[] = {
        {key_task,10,0},
        {lock_proc,10,0},
        {lcd_task,100,0}
};

unsigned char task_num;

void scheduler_init()
{
    task_num = sizeof(scheduler_task) / sizeof(task_t);
}

void scheduler_run()
{
    unsigned char i;
    for (i = 0; i < task_num; i++)
    {
        uint32_t now_time = GetTick();
        if (now_time >= (scheduler_task[i].rate_ms + scheduler_task[i].last_ms))
        {
            scheduler_task[i].last_ms = now_time;
            scheduler_task[i].task_func();
        }
    }
}
