/**
 * @file sem_test.c
 * @author 文佳源 (648137125@qq.com)
 * @brief 测试信号量功能
 * 1. 本测试用例主要测试信号量功能, 建立了三个任务
 * 2. 原理:-
 * 3. 使用方法和现象: 其中任务1(t1)每过一秒钟就释放一次信号量, t2和t3循环
 * 等待该信号量
 * @version 0.1
 * @date 2025-03-06
 *
 * Copyright (c) 2025
 *
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2025-03-06 <td>内容
 * </table>
 */
#include "catos.h"
#include "../cat_func_test.h"

#define IPC_TEST_TASK_STACK_SIZE (2048)

struct _cat_task_t ipc_test_task1;
struct _cat_task_t ipc_test_task2;
struct _cat_task_t ipc_test_task3;

cat_u8 ipc_test_task1_env[IPC_TEST_TASK_STACK_SIZE];
cat_u8 ipc_test_task2_env[IPC_TEST_TASK_STACK_SIZE];
cat_u8 ipc_test_task3_env[IPC_TEST_TASK_STACK_SIZE];

cat_sem_t test_sem;

#define TEST

void ipc_t1_entry(void *arg)
{
    (void)arg;

    cat_u32 sem_post_cnt = 0;

    for (;;)
    {
        cat_task_delay_ms(1000);
        cat_kprintf("[t1]-->sem post(%d)\r\n", sem_post_cnt++);
        cat_sem_post(&test_sem);
    }
}

cat_u32 wakingup_times = 0;
void ipc_t2_entry(void *arg)
{
    (void)arg;

    cat_u32 t2_notified_times = 0;

    for (;;)
    {
        cat_kprintf("[t2]-->t2 wait\r\n");

        /* 等待唤醒 */
        cat_sem_get(&test_sem, 10000);
        if (cat_task_get_error() == CAT_ETIMEOUT)
        {
            cat_kprintf("[t2][ERROR] t2 timeout\r\n");
        }
        else
        {
            cat_kprintf("[t2]<--t2 notified (%d)\r\n", t2_notified_times++);
        }
        // cat_task_yield();
    }
}

void ipc_t3_entry(void *arg)
{
    (void)arg;

    cat_u32 t3_notified_times = 0;

    for (;;)
    {
        cat_kprintf("[t3]-->t3 wait\r\n");

        /* 等待唤醒 */
        cat_sem_get(&test_sem, 10000);
        if (cat_task_get_error() == CAT_ETIMEOUT)
        {
            cat_kprintf("[t3][ERROR] t3 timeout\r\n");
        }
        else
        {
            cat_kprintf("[t3]<--t3 notified (%d)\r\n", t3_notified_times++);
        }
        // cat_task_yield();
    }
}

void sem_test(void)
{
    CAT_TEST_INFO(sem_test, test sem);
    cat_sem_init(&test_sem, 0, 5);

    cat_task_create(
        (const uint8_t *)"ipc_t1",
        &ipc_test_task1,
        ipc_t1_entry,
        CAT_NULL,
        1,
        ipc_test_task1_env,
        IPC_TEST_TASK_STACK_SIZE);

    cat_task_create(
        (const uint8_t *)"ipc_t2",
        &ipc_test_task2,
        ipc_t2_entry,
        CAT_NULL,
        1,
        ipc_test_task2_env,
        IPC_TEST_TASK_STACK_SIZE);

    cat_task_create(
        (const uint8_t *)"ipc_t3",
        &ipc_test_task3,
        ipc_t3_entry,
        CAT_NULL,
        1,
        ipc_test_task3_env,
        IPC_TEST_TASK_STACK_SIZE);
}
