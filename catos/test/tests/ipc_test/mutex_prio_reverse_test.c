/**
 * @file mutex_prio_reverse_test.c
 * @author 文佳源 (648137125@qq.com)
 * @brief 测试互斥量解决优先级反转的功能
 * 1. 本测试用例主要为验证互斥量对于优先级反转的处理, 建立了三个任务
 * 2. 原理:
 * - 2.1 假设三个任务优先级为t1>t2>t3, t3先执行且持有资源r1 ->
 * t3被t2抢占 -> t2被t1抢占 -> t1因也需要资源r1, 因此进入等待状态;
 * - 2.2 因此表现为t2与t1无资源争抢的情况下, t2优先占有了cpu, 此为优先级反转
 * - 2.3 本os使用优先级继承方式解决优先级反转, 原理为在更高优先级申请已被低优
 * 先级任务持有的互斥量时, 让该持有互斥量的任务暂时将优先级提高至申请的高优先
 * 级任务的优先级相同值, 待释放时再置回原值; 在2.1的例子中即t3暂时提高优先级至
 * 和t1相同, 释放后再还原, 这样t3就会抢占t2并且执行完释放后t1立即得到cpu
 * 3. 使用方法和现象: 任务启动后运行结束观察三个任务的开始事件和结束时间
 * !注意统计的 cost 是任务从开始执行有效任务到完成的总时间, 其中还可能由于调度
 * 和获取信号量等操作有耗时
 * 3.1->若 USE_MUTEX == 0 则未使用互斥量, 结果为:
 * [0] == test-0 == "mutex_prio_reverse" test mpr
 * [0] [t3] start running
 * [0] [t3] -->geting mutex
 * [0] [t3] <--got mutex
 * [100] [t2] start running
 * [200] [t1] start running
 * [200] [t1] -->geting mutex
 * [1104] [t2] end, cost 10040 ms
 * [1507] [t3] end, cost 15070 ms
 * [1508] [t1] <--got mutex
 * [1608] [t1] end, cost 14080 ms
 * 3.2->若 USE_MUTEX == 1 则使用了互斥量, 结果为:
 * [0] == test-0 == "mutex_prio_reverse" test mpr
 * [0] [t3] start running
 * [0] [t3] -->geting mutex
 * [0] [t3] <--got mutex
 * [100] [t2] start running
 * [200] [t1] start running
 * [200] [t1] -->geting mutex
 * [680] [t3] end, cost 6800 ms
 * [680] [t1] <--got mutex
 * [796] [t1] end, cost 5960 ms
 * [1855] [t2] end, cost 17550 ms
 * 3.3 可以看出不处理优先级反转时t1会再t2结束才能拿到cpu
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

#define USE_MUTEX 1
#define IPC_TEST_TASK_STACK_SIZE (2048)

cat_task_t ipc_test_task1;
cat_task_t ipc_test_task2;
cat_task_t ipc_test_task3;

cat_u8 ipc_test_task1_env[IPC_TEST_TASK_STACK_SIZE];
cat_u8 ipc_test_task2_env[IPC_TEST_TASK_STACK_SIZE];
cat_u8 ipc_test_task3_env[IPC_TEST_TASK_STACK_SIZE];

#if (USE_MUTEX == 0)
    cat_sem_t test_sem; /* 创建为二值信号量以作为未处理优先级反转的互斥量 */
    #define ENTER_CRITICAL() cat_sem_get(&test_sem, 100000)
    #define EXIT_CRITICAL()  cat_sem_post(&test_sem)
#else
    cat_mutex_t test_mutex;
    #define ENTER_CRITICAL() cat_mutex_get(&test_mutex, 100000)
    #define EXIT_CRITICAL()  cat_mutex_post(&test_mutex)
#endif

#define NOPS_PER_MS (100000 / 18)
#define _RUN_NOP() __asm__ __volatile__ ("nop"::)
/**
 * @brief 忙等
 * 
 * @param  ms               要等待的毫秒数
 */
void busy_wait_ms(cat_u32 ms)
{
    /** 
     * 测试10^5个nop要跑180个tick(即约1800ms) 
     * 因此速率为(10^5)/18 nop/ms
     */
    cat_ubase run_times = ms * NOPS_PER_MS;

    while(run_times > 0)
    {
        _RUN_NOP();
        run_times--;
    }
}

void ipc_t1_entry(void *arg)
{
    (void)arg;

#if 1
    cat_task_delay_ms(2000);

    cat_ubase start_tick = catos_get_systick();
    cat_kprintf("[t1] start running\r\n");
    cat_kprintf("[t1] -->geting mutex\r\n");
    ENTER_CRITICAL();
    cat_kprintf("[t1] <--got mutex\r\n");

    /* 等待一秒钟 */
    busy_wait_ms(1000);

    cat_ubase end_tick = catos_get_systick();
    cat_kprintf("[t1] end, cost %d ms\r\n", (end_tick - start_tick) * CATOS_SYSTICK_MS);
    EXIT_CRITICAL();
#else
    /* 调试忙等的功能 */
    cat_ubase start_tick = catos_get_systick();

    /* 忙等 */
    #define WAIT_TIME 1000
    busy_wait_ms(WAIT_TIME);

    cat_ubase end_tick = catos_get_systick();
    cat_kprintf("busy wait %d cost %d ms\r\n", WAIT_TIME, (end_tick - start_tick) * CATOS_SYSTICK_MS);
#endif
}

void ipc_t2_entry(void *arg)
{
    (void)arg;

    cat_task_delay_ms(1000);

    cat_ubase start_tick = catos_get_systick();
    cat_kprintf("[t2] start running\r\n");

    /* 等待10秒钟 */
    busy_wait_ms(10000);

    cat_ubase end_tick = catos_get_systick();
    cat_kprintf("[t2] end, cost %d ms\r\n", (end_tick - start_tick) * CATOS_SYSTICK_MS);
}

void ipc_t3_entry(void *arg)
{
    (void)arg;

    cat_ubase start_tick = catos_get_systick();
    cat_kprintf("[t3] start running\r\n");
    cat_kprintf("[t3] -->geting mutex\r\n");
    ENTER_CRITICAL();
    cat_kprintf("[t3] <--got mutex\r\n");

    /* 等待5秒钟 */
    busy_wait_ms(5000);
    
    cat_ubase end_tick = catos_get_systick();
    cat_kprintf("[t3] end, cost %d ms\r\n", (end_tick - start_tick) * CATOS_SYSTICK_MS);
    EXIT_CRITICAL();
}

void mutex_prio_reverse_test(void)
{
    CAT_TEST_INFO(mutex_prio_reverse, test mpr);
#if (USE_MUTEX == 0)
    cat_sem_init(&test_sem, 1, 1);
#else
    cat_mutex_init(&test_mutex);
#endif

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
        2,
        ipc_test_task2_env,
        IPC_TEST_TASK_STACK_SIZE);

    cat_task_create(
        (const uint8_t *)"ipc_t3",
        &ipc_test_task3,
        ipc_t3_entry,
        CAT_NULL,
        3,
        ipc_test_task3_env,
        IPC_TEST_TASK_STACK_SIZE);
}
