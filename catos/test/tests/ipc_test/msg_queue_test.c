/**
 * @file msg_queue_test.c
 * @author 文佳源 (648137125@qq.com)
 * @brief 测试消息队列功能
 * 1. 本测试用例主要测试消息队列功能, 建立了三个任务
 * 2. 原理:-
 * 3. 使用方法和现象: 
 * @version 0.1
 * @date 2025-03-08
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2025-03-08 <td>内容
 * </table>
 */
#include "catos.h"
#include "../cat_func_test.h"

#define IPC_TEST_TASK_STACK_SIZE (2048)
#define MQ_SIZE 1024

typedef struct _msg_content_t
{
    cat_u32 low_data;
    cat_u32 high_data;
} msg_content_t;

struct _cat_task_t ipc_test_task1;
struct _cat_task_t ipc_test_task2;
struct _cat_task_t ipc_test_task3;

cat_u8 ipc_test_task1_env[IPC_TEST_TASK_STACK_SIZE];
cat_u8 ipc_test_task2_env[IPC_TEST_TASK_STACK_SIZE];
cat_u8 ipc_test_task3_env[IPC_TEST_TASK_STACK_SIZE];

cat_mq_t test_mq;
cat_u8   test_space[MQ_SIZE];

#define TEST

void ipc_t1_entry(void *arg)
{
    (void)arg;

    cat_u32 msg_send_cnt = 0;
    msg_content_t content;

    for (;;)
    {
        cat_task_delay_ms(1000);
        cat_kprintf("[t1]-->msg send(%d)\r\n", msg_send_cnt++);
        
        content.high_data = msg_send_cnt;
        content.low_data  = msg_send_cnt+2;

        cat_mq_send(&test_mq, &content, sizeof(msg_content_t), 10000);
    }
}

cat_u32 wakingup_times = 0;
void ipc_t2_entry(void *arg)
{
    (void)arg;

    msg_content_t recv_content;

    for (;;)
    {
        cat_kprintf("[t2]-->t2 wait\r\n");

        /* 等待唤醒 */
        cat_mq_recv(&test_mq, &recv_content, sizeof(msg_content_t), 10000);
        if (cat_task_get_error() == CAT_ETIMEOUT)
        {
            cat_kprintf("[t2][ERROR] t2 timeout\r\n");
        }
        else
        {
            cat_kprintf("[t2]<--t2 recv content:low=%d, high=%d\r\n", recv_content.low_data, recv_content.high_data);
        }
    }
}

void ipc_t3_entry(void *arg)
{
    (void)arg;

    msg_content_t recv_content;

    for (;;)
    {
        cat_kprintf("[t3]-->t3 wait\r\n");

        /* 等待唤醒 */
        cat_mq_recv(&test_mq, &recv_content, sizeof(msg_content_t), 10000);
        if (cat_task_get_error() == CAT_ETIMEOUT)
        {
            cat_kprintf("[t3][ERROR] t3 timeout\r\n");
        }
        else
        {
            cat_kprintf("[t3]<--t3 recv content:low=%d, high=%d\r\n", recv_content.low_data, recv_content.high_data);
        }
    }
}

void mq_test(void)
{
    CAT_TEST_INFO(mq_test, test mq);
    cat_err err = cat_mq_init(&test_mq, test_space, MQ_SIZE, sizeof(msg_content_t));
    CAT_ASSERT(CAT_EOK == err);

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
