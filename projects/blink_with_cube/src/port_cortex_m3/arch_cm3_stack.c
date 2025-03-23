/**
 * @file arch_cm3_stack.c
 * @author 文佳源 (648137125@qq.com)
 * @brief cortex-m3的cpu相关可移植接口实现
 * @version 0.1
 * @date 2023-06-04
 * 
 * Copyright (c) 2023
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2023-06-04 <td>新建文件，从board目录将cpu相关内容放到这里
 * </table>
 */

#include "catos.h"

cat_u32 cat_context_from_task_sp_ptr;
cat_u32 cat_context_to_task_sp_ptr;

/* 栈初始化 */
/**< 异常触发时自动保存的寄存器 */
struct _exception_stack_frame
{
  cat_u32 r0;
  cat_u32 r1;
  cat_u32 r2;
  cat_u32 r3;
  cat_u32 r12;
  cat_u32 lr;  /**< r14 */
  cat_u32 pc;  /**< r15 */
  cat_u32 psr;
};

struct _stack_frame
{
  /**< 需要自行保存的寄存器 */
  cat_u32 r4;
  cat_u32 r5;
  cat_u32 r6;
  cat_u32 r7;
  cat_u32 r8;
  cat_u32 r9;
  cat_u32 r10;
  cat_u32 r11;

  struct _exception_stack_frame exeption_stack_frame;
};

/**
 * @brief 栈初始化
 * 
 * @param task_entry    任务入口函数地址
 * @param parameter     参数
 * @param stack_addr    栈起始地址
 * @param exit          任务退出函数地址
 * @return cat_u8*     初始化后的栈顶地址
 */
cat_u8 *cat_hw_stack_init(void (*task_entry)(void *), void *arg, cat_u8 *stack_addr, void  (*exit_func)(void *))
{
  struct _stack_frame *stack_frame;
  cat_u8             *stack;
  cat_u32            i;

  /* 先加上4字节再8字节向下取整对齐(相当于四舍五入) */
  stack = stack_addr + sizeof(cat_u32);
  stack = (cat_u8 *)CAT_ALIGN_DOWN((cat_u32)stack, 8);

  /* 栈向上生长? */
  stack -= sizeof(struct _stack_frame);
  stack_frame = (struct _stack_frame *)stack;

  /* 初始化栈帧中所有寄存器 */
  for(i = 0; i < (sizeof(struct _stack_frame) / sizeof(cat_u32)); i++)
  {
      ((cat_u32 *)stack_frame)[i] = 0xdeadbeef;
  }

  stack_frame->exeption_stack_frame.r0  = (cat_u32)arg;
  stack_frame->exeption_stack_frame.r1  = 0;
  stack_frame->exeption_stack_frame.r2  = 0;
  stack_frame->exeption_stack_frame.r3  = 0;
  stack_frame->exeption_stack_frame.r12 = 0;
  stack_frame->exeption_stack_frame.lr  = (cat_u32)exit_func;
  stack_frame->exeption_stack_frame.pc  = (cat_u32)task_entry;
  stack_frame->exeption_stack_frame.psr = (cat_u32)(1 << 24);

  /* 返回当前栈指针 */
  return stack;
}

#define SCB_CFSR        (*(volatile const unsigned *)0xE000ED28) /* Configurable Fault Status Register */
#define SCB_HFSR        (*(volatile const unsigned *)0xE000ED2C) /* HardFault Status Register */
#define SCB_MMAR        (*(volatile const unsigned *)0xE000ED34) /* MemManage Fault Address register */
#define SCB_BFAR        (*(volatile const unsigned *)0xE000ED38) /* Bus Fault Address Register */
#define SCB_AIRCR       (*(volatile unsigned long *)0xE000ED0C)  /* Reset control Address Register */
#define SCB_RESET_VALUE 0x05FA0004                               /* Reset value, write to SCB_AIRCR can reset cpu */

#define SCB_CFSR_MFSR   (*(volatile const unsigned char*)0xE000ED28)  /* Memory-management Fault Status Register */
#define SCB_CFSR_BFSR   (*(volatile const unsigned char*)0xE000ED29)  /* Bus Fault Status Register */
#define SCB_CFSR_UFSR   (*(volatile const unsigned short*)0xE000ED2A) /* Usage Fault Status Register */

static void usage_fault_track(void)
{
    CAT_SYS_PRINTF("usage fault:\r\n");
    CAT_SYS_PRINTF("SCB_CFSR_UFSR:%2x ", SCB_CFSR_UFSR);

    if(SCB_CFSR_UFSR & (1<<0))
    {
        /* [0]:UNDEFINSTR */
        CAT_SYS_PRINTF("UNDEFINSTR ");
    }

    if(SCB_CFSR_UFSR & (1<<1))
    {
        /* [1]:INVSTATE */
        CAT_SYS_PRINTF("INVSTATE ");
    }

    if(SCB_CFSR_UFSR & (1<<2))
    {
        /* [2]:INVPC */
        CAT_SYS_PRINTF("INVPC ");
    }

    if(SCB_CFSR_UFSR & (1<<3))
    {
        /* [3]:NOCP */
        CAT_SYS_PRINTF("NOCP ");
    }

    if(SCB_CFSR_UFSR & (1<<8))
    {
        /* [8]:UNALIGNED */
        CAT_SYS_PRINTF("UNALIGNED ");
    }

    if(SCB_CFSR_UFSR & (1<<9))
    {
        /* [9]:DIVBYZERO */
        CAT_SYS_PRINTF("DIVBYZERO ");
    }

    CAT_SYS_PRINTF("\r\n");
}

static void bus_fault_track(void)
{
    CAT_SYS_PRINTF("bus fault:\r\n");
    CAT_SYS_PRINTF("SCB_CFSR_BFSR:%2x ", SCB_CFSR_BFSR);

    if(SCB_CFSR_BFSR & (1<<0))
    {
        /* [0]:IBUSERR */
        CAT_SYS_PRINTF("IBUSERR ");
    }

    if(SCB_CFSR_BFSR & (1<<1))
    {
        /* [1]:PRECISERR */
        CAT_SYS_PRINTF("PRECISERR ");
    }

    if(SCB_CFSR_BFSR & (1<<2))
    {
        /* [2]:IMPRECISERR */
        CAT_SYS_PRINTF("IMPRECISERR ");
    }

    if(SCB_CFSR_BFSR & (1<<3))
    {
        /* [3]:UNSTKERR */
        CAT_SYS_PRINTF("UNSTKERR ");
    }

    if(SCB_CFSR_BFSR & (1<<4))
    {
        /* [4]:STKERR */
        CAT_SYS_PRINTF("STKERR ");
    }

    if(SCB_CFSR_BFSR & (1<<7))
    {
        CAT_SYS_PRINTF("SCB->BFAR:%8x\r\n", SCB_BFAR);
    }
    else
    {
        CAT_SYS_PRINTF("\r\n");
    }
}

static void mem_manage_fault_track(void)
{
    CAT_SYS_PRINTF("mem manage fault:\r\n");
    CAT_SYS_PRINTF("SCB_CFSR_MFSR:%2x ", SCB_CFSR_MFSR);

    if(SCB_CFSR_MFSR & (1<<0))
    {
        /* [0]:IACCVIOL */
        CAT_SYS_PRINTF("IACCVIOL ");
    }

    if(SCB_CFSR_MFSR & (1<<1))
    {
        /* [1]:DACCVIOL */
        CAT_SYS_PRINTF("DACCVIOL ");
    }

    if(SCB_CFSR_MFSR & (1<<3))
    {
        /* [3]:MUNSTKERR */
        CAT_SYS_PRINTF("MUNSTKERR ");
    }

    if(SCB_CFSR_MFSR & (1<<4))
    {
        /* [4]:MSTKERR */
        CAT_SYS_PRINTF("MSTKERR ");
    }

    if(SCB_CFSR_MFSR & (1<<7))
    {
        /* [7]:MMARVALID */
        CAT_SYS_PRINTF("SCB->MMAR:%8x\r\n", SCB_MMAR);
    }
    else
    {
        CAT_SYS_PRINTF("\r\n");
    }
}

static void hard_fault_track(void)
{
    if(SCB_HFSR & (1UL<<1))
    {
        /* [1]:VECTBL, Indicates hard fault is caused by failed vector fetch. */
        CAT_SYS_PRINTF("failed vector fetch\r\n");
    }

    if(SCB_HFSR & (1UL<<30))
    {
        /* [30]:FORCED, Indicates hard fault is taken because of bus fault,
                        memory management fault, or usage fault. */
        if(SCB_CFSR_BFSR)
        {
            bus_fault_track();
        }

        if(SCB_CFSR_MFSR)
        {
            mem_manage_fault_track();
        }

        if(SCB_CFSR_UFSR)
        {
            usage_fault_track();
        }
    }

    if(SCB_HFSR & (1UL<<31))
    {
        /* [31]:DEBUGEVT, Indicates hard fault is triggered by debug event. */
        CAT_SYS_PRINTF("debug event\r\n");
    }
}

/**
 * fault exception handling
 */
void catos_hard_fault_deal(struct _stack_frame *stack)
{
    CAT_KPRINTF("\r\n\r\n*** HardFault occurred ***\r\n");
    cat_u32 exc_ret = stack->exeption_stack_frame.psr; /* 这里其实是最后压栈的lr */
    CAT_KPRINTF("exrt: %8x\r\n", exc_ret);
    stack = (struct _stack_frame *)((cat_ubase)stack-4);
    CAT_KPRINTF("r00 : %8x\r\n", stack->exeption_stack_frame.r0);
    CAT_KPRINTF("r01 : %8x\r\n", stack->exeption_stack_frame.r1);
    CAT_KPRINTF("r02 : %8x\r\n", stack->exeption_stack_frame.r2);
    CAT_KPRINTF("r03 : %8x\r\n", stack->exeption_stack_frame.r3);
    CAT_KPRINTF("r12 : %8x\r\n", stack->exeption_stack_frame.r12);
    CAT_KPRINTF(" lr : %8x\r\n", stack->exeption_stack_frame.lr);
    CAT_KPRINTF(" pc : %8x\r\n", stack->exeption_stack_frame.pc);
    CAT_KPRINTF("psr : %8x\r\n", stack->exeption_stack_frame.psr);

    cat_task_t *current_task = cat_task_get_current();

    CAT_KPRINTF("cat_task_current=%x\r\n", (cat_ubase)current_task);

    if(CAT_NULL != current_task)
    {
        CAT_KPRINTF("hard fault on thread: %s\r\n", current_task->task_name);
    }
    else
    {
        CAT_KPRINTF("hard fault before first task start\r\n");
    }

    hard_fault_track();
    
    while (1);
}
