/**
 * @file str_test.c
 * @author 文佳源 (648137125@qq.com)
 * @brief 测试字符串
 * @version 0.1
 * @date 2025-03-27
 *
 * Copyright (c) 2025
 *
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2025-05-21 <td>内容
 * </table>
 */
#include "catos.h"

#include "../tests_config.h"
#if (CATOS_CAT_SHELL_ENABLE == 1 && TESTS_LIB_STR == 1)
#include "cat_shell.h"
#include "cat_stdio.h"
void *do_test_atoi(void *arg)
{
    CAT_ASSERT(arg);

    cat_i32 result = 0;
    cat_shell_instance_t *inst = (cat_shell_instance_t *)arg;

    if (inst->buffer.arg_num != 2)
    {
        cat_printf("[do_test_atoi] usage: test_atoi [STRING]\r\n");
    }
    else
    {
        cat_atoi(&result, inst->buffer.args[1]);
        cat_printf("arg[%d]: %s->", 1, inst->buffer.args[1]);
        cat_printf("%d\r\n", result);
    }

    return CAT_NULL;
}
CAT_DECLARE_CMD(test_atoi, test atoi, do_test_atoi);

void *do_test_atof(void *arg)
{
    CAT_ASSERT(arg);

    cat_double result = 0;
    cat_shell_instance_t *inst = (cat_shell_instance_t *)arg;

    if (inst->buffer.arg_num != 2)
    {
        cat_printf("[do_test_atoi] usage: test_atoi [STRING]\r\n");
    }
    else
    {
        cat_atof(&result, inst->buffer.args[1]);
        cat_printf("arg[%d]: %s->", 1, inst->buffer.args[1]);
        cat_printf("%f\r\n", result);
    }

    return CAT_NULL;
}
CAT_DECLARE_CMD(test_atof, test atof, do_test_atof);
#endif