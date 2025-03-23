/**
 * @file cat_stdio.h
 * @author mio_wen (648137125@qq.com)
 * @brief 标准输入输出
 * @version 0.1
 * @date 2022-07-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef CAT_STDIO_H
#define CAT_STDIO_H

#include <stdarg.h>
#include "cat_intr.h"

#include "catos_config.h"
#include "catos_types.h"

#if (CATOS_ENABLE_SYS_PRINTF == 1)

/* 定义文字颜色编码 START */
#define COLOR_NONE          "\33[0m"      /* 清除颜色，该码之后的打印恢复原色 */

#define COLOR_RED           "\33[31m"    /* 红色 */
#define COLOR_LIGHT_RED     "\33[1;31m"    /* 亮红色 */
#define COLOR_GREEN         "\33[1;32m"    /* 绿色 */
#define COLOR_LIGHT_GREEN   "\33[32m"    /* 亮绿色 */
#define COLOR_YELLOW        "\33[1;33m"    /* 黄色 */
/* 定义文字颜色编码 END   */

/* 系统输入输出宏 */
#define CAT_KPRINTF(_fmt, ...) \
    do{ \
        cat_printf((const cat_u8 *)"[%d] ", catos_get_systick()); \
        cat_printf((const cat_u8 *)_fmt, ##__VA_ARGS__); \
    }while(0)

#define CAT_SYS_PRINTF(_fmt, ...) \
    cat_printf((const cat_u8 *)_fmt, ##__VA_ARGS__)

#define CAT_SYS_SCANF(_fmt, ...) \
    cat_scanf((const cat_u8 *)_fmt, ##__VA_ARGS__)

#define CAT_SYS_PUTCHAR(_ch) \
    cat_putchar(_ch)

#define CAT_SYS_GETCHAR() \
    cat_getchar()

#define SPRINTF(_buf, _fmt, ...) \
    cat_sprintf((cat_u8 *)_buf, CAT_TRUE, (const cat_u8 *)_fmt, ##__VA_ARGS__)

/* 调试输出宏 */    
#if( CATOS_ENABLE_DEBUG_PRINTF == 1)
    #define CAT_DEBUG_PRINTF(_fmt, ...) \
        cat_printf((const cat_u8 *)_fmt, ##__VA_ARGS__)
#else //#if( DEBUG_PRINT_ENABLE == 1)
    #define CAT_DEBUG_PRINTF(_fmt, ...)
#endif //#if( DEBUG_PRINT_ENABLE == 1)
/* PUBLIC FUNC DECL START */
/**
 * @brief 设置stdio输入输出设备
 * 
 * @param  name             设备名称
 * @return cat_u8          成功失败
 */
cat_u8 cat_stdio_set_device(const cat_u8 *name);

/**
 * @brief 检查stdio设备是否设置
 * 
 * @return cat_u8 0：未设置
 *                 1：已设置
 */
cat_u8 cat_stdio_is_device_is_set(void);

cat_u8 cat_getchar(void);
cat_u8 cat_putchar(cat_u8 ch);
cat_i32 cat_scanf(const cat_u8 *format, ...);
cat_i32 cat_printf(const cat_u8 *format, ...);
cat_i32 cat_sprintf(cat_u8 *buf, cat_bool with_end, const cat_u8 *format, ...);

/* PUBLIC FUNC DECL END */
#else
    #define CAT_KPRINTF(_fmt, ...)
#endif /* #if (CATOS_ENABLE_SYS_PRINTF == 1) */
/* 错误处理 */
#define CAT_ASSERT(_expr) \
    do{ \
        if(!(_expr)) \
        { \
            CAT_KPRINTF("%s:%d %s assert failed !\r\n", __FILE__, __LINE__, #_expr); \
            while(1); \
        } \
    }while(0)

#define CAT_FALTAL_ERROR(_msg) \
    do{ \
        cat_irq_disable();\
        CAT_KPRINTF("%s:%d faltal error:%s !\r\n", __FILE__, __LINE__, _msg); \
        while(1); \
        cat_irq_enable();\
    }while(0)

#define CAT_PRINT_ERROR(_err_msg) \
    do{ \
        CAT_KPRINTF(COLOR_LIGHT_RED); \
        CAT_SYS_PRINTF(_err_msg); \
        CAT_SYS_PRINTF(COLOR_NONE); \
    }while(0)
#endif /* #define CAT_STDIO_H */
