/**
 * @file mem_test.c
 * @author 文佳源 (648137125@qq.com)
 * @brief 测试动态内存管理功能
 * 1. 该测试用例主要测试内存分配和释放
 * 2. 原理:-
 * 3. 使用方法和现象:系统启动后利用shell命令test_mem进行测
 * 试, 观察分配和释放现象
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

#define TEST_TITLE(info) \
    do{ \
        CAT_KPRINTF("========= TEST %s =========\r\n", #info); \
    }while(0)

void test_mem(void)
{
    cat_u8 *mem1, *mem2, *mem3, *mem4, *mem5;

    cat_mem_print_info();

    /* 测试分配 */
    CAT_TEST_INFO(malloc, test malloc);
    mem1 = cat_malloc(512);
    mem2 = cat_malloc(64);
    mem3 = cat_malloc(1024);
    mem4 = cat_malloc(128);
    cat_mem_print_info();

    /* 展示内存碎片的出现 */
    CAT_TEST_INFO(fragment, show how fra appear);
    CAT_TEST_INFO(free, test free);
    CAT_KPRINTF("->free mem2\r\n");
    mem5 = cat_malloc(256);
    if(CAT_NULL == mem5)
    {
        /* 此时空闲总空间大于304字节, 但由于被分成两部分, 无法被分配, 称为内存碎片 */
        CAT_KPRINTF("fail to malloc mem5:256bytes\r\n");
    }
    cat_mem_print_info();

    /* 测试释放 */
    cat_free(mem2);
    cat_mem_print_info();
    CAT_KPRINTF("->free mem3\r\n");
    cat_free(mem3);
    cat_mem_print_info();

    CAT_KPRINTF("<--free all mem\r\n");
    cat_free(mem1);
    cat_free(mem4);
    cat_mem_print_info();

}

#if (CATOS_ENABLE_CAT_SHELL == 1)
#include "cat_shell.h"
#include "cat_stdio.h"
#include "port.h"

void *do_test_mem(void *arg)
{
    (void)arg;
	
    test_mem();
		
	return CAT_NULL;
}
CAT_DECLARE_CMD(test_mem, test mem, do_test_mem);
#endif
