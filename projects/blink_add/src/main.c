/**
 * @file main.c
 * @author 文佳源 (648137125@qq.com)
 * @brief 寄存器led闪烁源文件
 * @version 0.1
 * @date 2025-03-21
 *
 * Copyright (c) 2025
 *
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2025-03-21 <td>内容
 * </table>
 */
#include "main.h"

#define BLINK_TASK_STACK_SIZE (2048)                   /* blink任务栈大小 */
static cat_task_t blink_task;                          /* blink任务结构体 */
static cat_u8 blink_task_stack[BLINK_TASK_STACK_SIZE]; /* blink任务栈 */

static void set_system_clk(void);

/* blink任务函数 */
static void blink_task_entry(void *arg)
{
    (void)arg;                                                                                                  /* 开启GPIOC时钟 */

    RCC_APB2ENR |= (1 << 4); /* 位4对应GPIOC时钟使能 */ /* 配置PC13为输出模式（通用推挽输出，最大速度50MHz） */ /* 清除CNF13[1:0]和MODE13[1:0]位，然后设置MODE13[1:0]=01（输出模式，最大速度10MHz） */
    GPIOC_CRH &= ~(0xF << 20);                                                                                  /* 清除位20-23 */
    GPIOC_CRH |= (1 << 20);                                                                                     /* 设置MODE13[1]=1 */

    while (1)
    {
        GPIOC_ODR &= ~(1 << 13); /* 点亮LED（PC13低电平有效） */
        cat_task_delay_ms(1000);
        GPIOC_ODR |= (1 << 13); /* 关闭LED */
        cat_task_delay_ms(1000);
    }
}

int main(void)
{

    set_system_clk();

    catos_init();

    cat_task_create(
        "blink_task",
        &blink_task,
        blink_task_entry,
        CAT_NULL,
        0,
        blink_task_stack,
        BLINK_TASK_STACK_SIZE
    );

    catos_start_schedule();

    while (1);
}

static void set_system_clk(void)
{
    /* 开启HSE */
    RCC_CR |= 1 << 16;
    while(0 == (RCC_CR & (1 << 17)));

    /* 开启预取指缓存 */
    FLASH_ACR |= (1 << 5);
    FLASH_ACR &= (cat_u32)(~0x03);
    FLASH_ACR |= (1 << 1);

    /* 设置AHB、APB2、APB1 */
    RCC_CFGR |= 0x4 << 8; /* 0x00000400=ABH不分频、APB2不分频、APB1二分频 */

    /* 设置PLL倍频 */
    RCC_CFGR |= 1 << 16; /* PLL<--HSE作为输入 */
    cat_u8 PLL = 9-2; /* 9倍 */
    RCC_CFGR |= PLL << 18;
    
    /* 使能PLL */
    RCC_CR |= 1 << 24;
    while(0 == (RCC_CR & (1 << 25)));

    /* 选择PLL为系统时钟 */
    RCC_CFGR |= 2 << 0;

    /* 等待系统时钟设置完成 */
    cat_u32 finish = 0;
    do{
        finish = RCC_CFGR & 0x0c;
    }while(finish != 0x08);

    /* 设置向量表偏移 */
    SCB_VTOR = (cat_u32)0x08000000;
}

/* 串口设备实例(这个例子没用，可以自己写) */
static cat_device_t uart1_dev =
    {
        .type = CAT_DEVICE_TYPE_CHAR,
        .init = CAT_NULL,
        .open = CAT_NULL,
        .close = CAT_NULL,
        .read = CAT_NULL,
        .write = CAT_NULL,
        .ctrl = CAT_NULL,

        .pri_data = CAT_NULL
    };

/* 注册串口设备 */
cat_u8 cat_debug_uart_register(void)
{
    cat_u8 err = cat_device_register(
        &uart1_dev,
        "debug_uart",
        CAT_DEVICE_MODE_RDWR
    );

    CAT_ASSERT(CAT_EOK == err);

    return err;
}

/* 启动文件里调用了这个 */
void SystemInit(void)
{
}

/* 时钟中断服务函数 */
void SysTick_Handler(void)
{
    cat_intr_systemtick_handler();
}
