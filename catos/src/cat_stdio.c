/**
 * @file cat_stdio.c
 * @author 文佳源 (648137125@qq.com)
 * @brief 标准输入输出
 * @version 0.1
 * @date 2022-07-16
 * 
 * Copyright (c) 2023
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2022-07-16 <td>支持十进制和字符串输入输出
 * <tr><td>v1.0 <td>文佳源 <td>2023-6-4   <td>支持16进制输出
 * <tr><td>v1.0 <td>文佳源 <td>2025-3-9   <td>修复cat_sprintf()使用时width大于零时无限循环写内存的bug
 * </table>
 */

#include "cat_stdio.h"
#include "cat_lib.h"

/************* macros and defines **************/
#define CAT_INT_LONG    10
#define MAX_STRNG_LEN   300
#define CAT_FLOAT_LONG  42

typedef enum
{
    CAT_ERROR_TYPE_NO_MATCH_ERROR_TYPE = 0,
    CAT_ERROR_TYPE_ARG_TYPE_NOT_SUPPORT,
    CAT_ERROR_END,
} cat_error_type_t;

/************* local vars **************/
static const cat_u8 *_cat_error_msg[] = 
{
    (const cat_u8 *)"[cat_stdio] no match error type !!\r\n",
    (const cat_u8 *)"[cat_stdio:cat_print]type not support !!\r\n",
};

/************* static **************/
static cat_i32 _cat_print_string(const cat_u8 *str, cat_i32 width);
static cat_i32 _cat_print_error(cat_error_type_t type);
static cat_i32 _cat_print_int(cat_i32 num, cat_i32 width);
static cat_err   _cat_print_float(cat_float num, cat_u8 keep, cat_i32 width);

static cat_i32 _cat_sprint_string(cat_u8 *buf, cat_u32 *buf_idx_ptr, const cat_u8 *str);
static cat_i32 _cat_sprint_int(cat_u8 *buf, cat_u32 *buf_idx_ptr, cat_i32 num, cat_i32 width);
static cat_err _cat_sprint_float(cat_u8 *buf, cat_u32 *buf_idx_ptr, cat_float num, cat_u8 keep, cat_i32 width);

static cat_i32 _cat_scan_string(cat_u8 *str_dest, cat_u32 buf_len);
static cat_i32 _cat_scan_int(cat_i32 *dest);


static cat_i32 _cat_print_string(const cat_u8 *str, cat_i32 width)
{
    cat_i32 ret = CAT_EOK;
    cat_u8 i = 0;

    if(CAT_NULL == str)
    {
        ret = CAT_ERROR;
    }
    else
    {
        while('\0' != *str)
        {
            cat_putchar(*str);
            str++;
            i++;
        }
    }

    /* 用空字符填充 */
    width = width - i;
    while(width > 0)
    {
        cat_putchar(' ');
        width--;
    }

    return ret;
}

static cat_i32 _cat_print_error(cat_error_type_t type)
{
    cat_i32 ret = CAT_EOK;
    cat_putchar('\r');
    cat_putchar('\n');

    /* 检查参数type范围 */
    if(
        (CAT_ERROR_TYPE_NO_MATCH_ERROR_TYPE >= type) ||
        (CAT_ERROR_END                      <= type)    
    )
    {
        _cat_print_string(_cat_error_msg[CAT_ERROR_TYPE_NO_MATCH_ERROR_TYPE], 0);
    }
    else
    {
        _cat_print_string(_cat_error_msg[type], 0);
    }

    return ret;
}

static cat_i32 _cat_print_int(cat_i32 num, cat_i32 width)
{
    cat_i32 ret = CAT_EOK;
    /* cat_u32 max val = 4294967295 */
    cat_u8 buf[CAT_INT_LONG] = {0};
    cat_u8 i = 0;

    if(num < 0)
    {
        cat_putchar('-');
        num = -num;
    }

    do
    {
        buf[i++] = (cat_u8)(num % 10);
        num = num / 10;

    }while((0 != num) && (i < CAT_INT_LONG));

    /* 如果有对齐要求则先输出空格 */
    width = width - i;
    while(width > 0)
    {
        cat_putchar(' ');
        width--;
    }

    while(i > 0)
    {
        cat_putchar(buf[--i] + '0');
    }

    return ret;
}

/**
 * @brief 打印浮点数
 * 
 * @param  num              要打印的浮点数
 * @param  keep             要保留的小数位数
 * @param  width            输出宽度
 * @return cat_err 
 */
static cat_err _cat_print_float(cat_float num, cat_u8 keep, cat_i32 width)
{
    cat_err ret = CAT_EOK;
    /* cat_float : [+/- 1.18 * 10^-38, +/- 3.4 * 10^38], 算上符号, 小数点和末尾符号一共 42 个字符 */
    cat_u8 buf[CAT_FLOAT_LONG] = {0};
    cat_u8 i = 0;

    /* 最少保留一位小数 */
    if(0 == keep)
    {
        keep = 1;
    }
    

    if(num < 0)
    {
        cat_putchar('-');
        num = -num;
    }

    cat_i32 int_part = (cat_i32)num;
    num -= int_part;
    if(0 == int_part)
    {
        buf[i++] = 0;
    }
    else
    {
        do
        {
            buf[i++] = (cat_u8)(int_part % 10);
            int_part = int_part / 10;
        }while((0 != int_part) && (i < CAT_FLOAT_LONG));
    }

    /* 计算保留位数的截止处(还要多计算一位) */
    cat_u32 keep_end = i + keep + 1;
    
    if(i < CAT_INT_LONG - 1)
    {
        buf[i++] = '.';

        if(0.0f == num)
        {
            buf[i++] = 0;
        }
        else
        {
            do
            {
                num *= 10;
                int_part = (cat_i32)num;
                buf[i++] = int_part;
                num -= int_part;
            }while((num != 0.0f) && (i <= keep_end)); /* 多算一位是为了四舍五入 */

            while(i <= keep_end)
            {
                buf[i++] = 0;
            }

            /* 减掉用于四舍五入的那一位 */
            i--;

            if(buf[keep_end] >= 5)
            {

                do
                {
                    keep_end--;
                    buf[keep_end] = (buf[keep_end] + 1) % 10;
                }while(
                    (0   == buf[keep_end]) &&
                    ('.' != buf[keep_end])
                );

                if('.' == buf[keep_end])
                {
                    /* 跳过小数点 */
                    keep_end--;

                    if(0 == buf[keep_end + 2])
                    {
                        do
                        {
                            keep_end--;
                            buf[keep_end] = (buf[keep_end] + 1) % 10;
                        }while(
                            (0 == buf[keep_end]) &&
                            (0 != keep_end)
                        );
                    }
                }
            } /* buf[keep_end] > 5 */
        }
    }
    else
    {
        CAT_SYS_PRINTF("[printf-float] ERROR: buf overflow!\r\n");
    }

    /* 如果有对齐要求则先输出空格 */
    width = width - i;
    while(width > 0)
    {
        cat_putchar(' ');
        width--;
    }

    /* 找到小数点位置, 整数部分 buf 要倒序输出, 小数部分 buf 要正序输出 */
    cat_u8 int_idx = 0, dec_idx = 0;
    while(('.' != buf[int_idx]) && (int_idx < CAT_INT_LONG))
    {
        int_idx++;
    }

    /* 小数点位置下一个就是小数开始的位置 */
    dec_idx = int_idx + 1;

    /* 输出整数部分 */
    while(int_idx > 0)
    {
        cat_putchar(buf[--int_idx] + '0');
    }

    cat_putchar('.');

    /* 输出小数部分 */
    while(dec_idx < i)
    {
        cat_putchar(buf[dec_idx++] + '0');
    }

    return ret;
}

static cat_i32 _cat_sprint_string(cat_u8 *buf, cat_u32 *buf_idx_ptr, const cat_u8 *str)
{
    cat_i32 ret = CAT_EOK;

    if(CAT_NULL == str)
    {
        ret = CAT_ERROR;
    }
    else
    {
        while('\0' != *str)
        {
            //cat_putchar(*str);
            buf[(*buf_idx_ptr)++] = *str;
            str++;
        }
    }

    return ret;
}
static cat_i32 _cat_sprint_int(cat_u8 *buf, cat_u32 *buf_idx_ptr, cat_i32 num, cat_i32 width)
{
    cat_i32 ret = CAT_EOK;
    /* cat_u32 max val = 4294967295 */
    cat_u8 int_buf[CAT_INT_LONG] = {0};
    cat_u8 i = 0;

    if(num < 0)
    {
        //cat_putchar('-');
        buf[(*buf_idx_ptr)++] = '-';
        num = -num;
    }

    do
    {
        int_buf[i++] = (cat_u8)(num % 10);
        num = num / 10;

    }while((0 != num) && (i < CAT_INT_LONG));

    /* 如果有对齐要求则先输出空格 */
    width = width - i - 1;
    while(width > 0)
    {
        //cat_putchar(' ');
        buf[(*buf_idx_ptr)++] = ' ';

        /**
         * 2025/03/09 文佳源发现
         * bugfix: 这里width没有减少, 导致这里一直循环到爆炸
         * 在调试平衡车时发现
         */
        width--;
    }

    while(i > 0)
    {
        //cat_putchar(int_buf[--i] + '0');
        buf[(*buf_idx_ptr)++] = int_buf[--i] + '0';
    }

    return ret;
}

static cat_err _cat_sprint_float(cat_u8 *buf, cat_u32 *buf_idx_ptr, cat_float num, cat_u8 keep, cat_i32 width)
{
    cat_err ret = CAT_EOK;
    /* cat_float : [+/- 1.18 * 10^-38, +/- 3.4 * 10^38], 算上符号, 小数点和末尾符号一共 42 个字符 */
    cat_u8 float_buf[CAT_FLOAT_LONG] = {0};
    cat_u8 i = 0;

    /* 最少保留一位小数 */
    if(0 == keep)
    {
        keep = 1;
    }
    

    if(num < 0)
    {
        //cat_putchar('-');
        buf[(*buf_idx_ptr)++] = '-';
        num = -num;
    }

    cat_i32 int_part = (cat_i32)num;
    num -= int_part;
    if(0 == int_part)
    {
        float_buf[i++] = 0;
    }
    else
    {
        do
        {
            float_buf[i++] = (cat_u8)(int_part % 10);
            int_part = int_part / 10;
        }while((0 != int_part) && (i < CAT_FLOAT_LONG));
    }

    /* 计算保留位数的截止处(还要多计算一位) */
    cat_u32 keep_end = i + keep + 1;
    
    if(i < CAT_INT_LONG - 1)
    {
        float_buf[i++] = '.';

        if(0.0f == num)
        {
            float_buf[i++] = 0;
        }
        else
        {
            do
            {
                num *= 10;
                int_part = (cat_i32)num;
                float_buf[i++] = int_part;
                num -= int_part;
            }while((num != 0.0f) && (i <= keep_end)); /* 多算一位是为了四舍五入 */

            while(i <= keep_end)
            {
                float_buf[i++] = 0;
            }

            /* 减掉用于四舍五入的那一位 */
            i--;

            if(float_buf[keep_end] >= 5)
            {

                do
                {
                    keep_end--;
                    float_buf[keep_end] = (float_buf[keep_end] + 1) % 10;
                }while(
                    (0   == float_buf[keep_end]) &&
                    ('.' != float_buf[keep_end])
                );

                if('.' == float_buf[keep_end])
                {
                    /* 跳过小数点 */
                    keep_end--;

                    if(0 == float_buf[keep_end + 2])
                    {
                        do
                        {
                            keep_end--;
                            float_buf[keep_end] = (float_buf[keep_end] + 1) % 10;
                        }while(
                            (0 == float_buf[keep_end]) &&
                            (0 != keep_end)
                        );
                    }
                }
            } /* float_buf[keep_end] > 5 */
        }
    }
    else
    {
        CAT_SYS_PRINTF("[printf-float] ERROR: float_buf overflow!\r\n");
    }

    /* 如果有对齐要求则先输出空格 */
    width = width - i;
    while(width > 0)
    {
        // cat_putchar(' ');
        buf[(*buf_idx_ptr)++] = ' ';
        width--;
    }

    /* 找到小数点位置, 整数部分 buf 要倒序输出, 小数部分 buf 要正序输出 */
    cat_u8 int_idx = 0, dec_idx = 0;
    while(('.' != float_buf[int_idx]) && (int_idx < CAT_INT_LONG))
    {
        int_idx++;
    }

    /* 小数点位置下一个就是小数开始的位置 */
    dec_idx = int_idx + 1;

    /* 输出整数部分 */
    while(int_idx > 0)
    {
        // cat_putchar(buf[--int_idx] + '0');
        buf[(*buf_idx_ptr)++] = float_buf[--int_idx] + '0';
    }

    // cat_putchar('.');
    buf[(*buf_idx_ptr)++] = '.';

    /* 输出小数部分 */
    while(dec_idx < i)
    {
        // cat_putchar(buf[dec_idx++] + '0');
        buf[(*buf_idx_ptr)++] = float_buf[dec_idx++] + '0';
    }

    return ret;
}

static cat_i32 _cat_scan_string(cat_u8 *str_dest, cat_u32 buf_len)
{
    cat_i32 ret = CAT_EOK;
    cat_u32 i = 0;

    do
    {
        str_dest[i++] = cat_getchar();
    } while (
        ('\r' != str_dest[i-1]) &&
        ('\n' != str_dest[i-1]) &&
        ('\0' != str_dest[i-1]) &&
        (i < buf_len - 1)
    );

    str_dest[i] = '\0';

    return ret;
}

static cat_i32 _cat_scan_int(cat_i32 *dest)
{
    cat_i32 ret = CAT_EOK;
    cat_u8 is_negative = 0;
    cat_u8 tmp_char = 0;
    cat_i32 tmp_int = 0;

    tmp_char = cat_getchar();
    if('-' == tmp_char)
    {
        is_negative = 1;
        tmp_char = cat_getchar();
    }
    else if('+' == tmp_char)
    {
        tmp_char = cat_getchar();
    }


    while(
        ('0' <= tmp_char) &&
        ('9' >= tmp_char)
    )
    {
        tmp_int = (tmp_int * 10) + (tmp_char - '0');
        tmp_char = cat_getchar();
    }

    if(1 == is_negative)
    {
        tmp_int = -tmp_int;
    }

    *dest = tmp_int;

    return ret;
}

/************* func **************/
#include "cat_device.h"

static cat_device_t *_stdio_dev = CAT_NULL;

cat_u8 cat_stdio_set_device(const cat_u8 *name)
{
    cat_u8 ret = CAT_EOK;

    _stdio_dev = cat_device_get(name);
    CAT_ASSERT(CAT_NULL != _stdio_dev);

    if(CAT_NULL != _stdio_dev)
    {
        ret = cat_device_init(_stdio_dev);
    }
    if(CAT_EOK == ret)
    {
        ret = cat_device_open(_stdio_dev, CAT_DEVICE_MODE_RDWR);
    }

    return ret;
}

cat_u8 cat_stdio_is_device_is_set(void)
{
    cat_u8 ret = 0;

    if(CAT_NULL != _stdio_dev)
    {
        ret = 1;
    }

    return ret;
}

cat_u8 cat_getchar(void)
{
    cat_u8 ret = 0;
    
#if 0
    cat_bsp_uart_receive_byte((cat_u8 *)&ret);
#else
    cat_device_read(_stdio_dev, 0, &ret, 1);
#endif

    return ret;
}

cat_u8 cat_putchar(cat_u8 ch)
{
#if 0
    cat_bsp_uart_transmit_byte((cat_u8 *)&ch);
#else
    cat_device_write(_stdio_dev, 0, &ch, 1);
#endif

    return ch;
}

cat_i32 cat_scanf(const cat_u8 *format, ...)
{
    cat_i32 ret = CAT_EOK;
    va_list ap;
    cat_u8 *p = CAT_NULL;

    if(CAT_NULL == format)
    {
        ret = CAT_ERROR;
        return ret;
    }

    /* 获取参数 */
    va_start(ap, format);

    /* 挨个处理 */
    p = (cat_u8 *)format;
    while('\0' != *p)
    {
        if('%' != *p)
        {
            //cat_putchar(*p);
            p++;
            continue;
        }
        else
        {
            p++;
        }

        switch (*p)
        {
        case 'd':
            p++;
            //_cat_print_int(va_arg(ap, cat_i32));
            _cat_scan_int(va_arg(ap, cat_i32 *));
            break;

        case 's':
            p++;
            //_cat_print_string(va_arg(ap, cat_u8 *));
            _cat_scan_string(va_arg(ap, cat_u8 *), MAX_STRNG_LEN);
            break;
        
        default:
            /* 可以考虑单独定义一个输出纯字符串的函数，可以打印错误信息 */
            _cat_print_error(CAT_ERROR_TYPE_ARG_TYPE_NOT_SUPPORT);
            ret = CAT_ERROR;
            break;
        }

        if(CAT_ERROR == ret)
        {
            break;
        }
    }

    va_end(ap);

    // cat_putchar('\r');
    // cat_putchar('\n');

    return ret;
}

cat_i32 cat_printf(const cat_u8 *format, ...)
{
    cat_i32 ret = CAT_EOK;
    va_list ap;
    cat_u8 *p = CAT_NULL;          /**< 用来遍历format字符串 */

    cat_u8 wid_buf[5] = {0};   /**< 用于保存宽度的字符串 */
    cat_i32 width = 0;          /**< 输出宽度(目前仅用于整型输出) */

    cat_u8 keep_buf[5] = {0};  /**< 用于保存保留小数位数的字符串 */
    cat_i32 keep = 0;           /**< 保留位数 */
        

    cat_u8 hex_str[11] = {0};  /**< 用于保存转为十六进制的字符串*/

    if(CAT_NULL == format)
    {
        ret = CAT_ERROR;
        return ret;
    }

    /* 获取参数 */
    va_start(ap, format);

    /* 挨个处理 */
    p = (cat_u8 *)format;
    while('\0' != *p)
    {
        if('%' != *p)
        {
            /* 发现'%'就开始处理格式化输出内容 */
            cat_putchar(*p);
            p++;
            continue;
        }
        else
        {
            p++;
        }

        /* 获得宽度数据(如果有) */
        /* 清除上一次的宽度数据 */
        width = 0;
        while(
            ((*p) >= '0') &&
            ((*p) <= '9')
        )
        {
            wid_buf[width++] = *p;
            p++;
        }
        if(width > 0)
        {
            wid_buf[width] = '\0';
            cat_atoi(&width, wid_buf);

            /* 之前这里忘记清零了 */
            wid_buf[0] = '\0';
        }

        /* 获得小数保留位数, 0 和 1 均保留一位小数 */
        keep = 0; /* 必须在外面复位, 否则如果在一次非零 keep 之后
                   * 未声明保留位数就会整成上一次的, 缺省保留一位小数 */
        if('.' == *p)
        {
            p++;
            
            while(
                ((*p) >= '0') &&
                ((*p) <= '9')
            )
            {
                keep_buf[keep++] = *p;
                p++;
            }
            if(keep > 0)
            {
                cat_i32 keep_int;

                keep_buf[keep] = '\0';
                cat_atoi(&keep_int, keep_buf);

                keep = (cat_u8)keep_int;
                
                keep_buf[0] = '\0';
            }
        }

        switch (*p)
        {
        case 'd':
        {
            p++;
            _cat_print_int(va_arg(ap, cat_i32), width);
            width = 0;
            break;
        }
        case 's':
        {
            p++;
            _cat_print_string(va_arg(ap, cat_u8 *), width);
            break;
        }
        case 'x':
        {
            p++;
            cat_itoh(hex_str, va_arg(ap, cat_u32));
            if('\0' != hex_str[2])
            {
                _cat_print_string(hex_str, width);
            }
            else
            {
                _cat_print_string(hex_str, width);
                cat_putchar('0');
            }
            break;
        }
        case 'f':
        {
            p++;
            _cat_print_float(va_arg(ap, cat_double), keep, width);
            break;
        }
            

        default:
            /* 可以考虑单独定义一个输出纯字符串的函数，可以打印错误信息 */
            _cat_print_error(CAT_ERROR_TYPE_ARG_TYPE_NOT_SUPPORT);
            ret = CAT_ERROR;
            break;
        }

        if(CAT_ERROR == ret)
        {
            break;
        }
    }

    va_end(ap);

    return ret;
}

/**
 * @brief 和printf差不多，只不过将数据放进buf中
 * 
 * @param  buf              要输出到的缓冲区
 * @param  format           格式化字符串
 * @param  with_end         是否带结束符
 * @param  ...              输出的参数列表
 * @return cat_i32          成功失败
 */
cat_i32 cat_sprintf(cat_u8 *buf, cat_bool with_end, const cat_u8 *format, ...)
{
    cat_i32 ret = CAT_EOK;
    va_list ap;
    cat_u8 *p = CAT_NULL;          /**< 用来遍历format字符串 */
    cat_u8 wid_buf[5] = {0};   /**< 用于保存宽度的字符串 */
    cat_i32 width = 0;          /**< 输出宽度(目前仅用于整型输出) */

    cat_u8 keep_buf[5] = {0};  /**< 用于保存保留小数位数的字符串 */
    cat_i32 keep = 0;           /**< 保留位数 */

    cat_u8 hex_str[11] = {0};  /**< 用于保存转为十六进制的字符串 */

    cat_u32 buf_idx = 0;       /**< 访问缓冲区 */

    if(CAT_NULL == format)
    {
        ret = CAT_ERROR;
        return ret;
    }

    /* 获取参数 */
    va_start(ap, format);

    /* 挨个处理 */
    p = (cat_u8 *)format;
    while('\0' != *p)
    {
        if('%' != *p)
        {
            /* 发现'%'就开始处理格式化输出内容 */
            //cat_putchar(*p);
            buf[buf_idx++] = *p;
            p++;
            continue;
        }
        else
        {
            p++;
        }

        /* 获得宽度数据(如果有) */
        if(
            ((*p) >= '0') &&
            ((*p) <= '9')
        )
        {
            wid_buf[width++] = *p;
            p++;
        }
        if(width > 0)
        {
            wid_buf[width] = '\0';
            cat_atoi(&width, wid_buf);
        }

        /* 获得小数保留位数, 0 和 1 均保留一位小数 */
        keep = 0; /* 必须在外面复位, 否则如果在一次非零 keep 之后
                   * 未声明保留位数就会整成上一次的, 缺省保留一位小数 */
        if('.' == *p)
        {
            p++;
            
            while(
                ((*p) >= '0') &&
                ((*p) <= '9')
            )
            {
                keep_buf[keep++] = *p;
                p++;
            }
            if(keep > 0)
            {
                cat_i32 keep_int;

                keep_buf[keep] = '\0';
                cat_atoi(&keep_int, keep_buf);

                keep = (cat_u8)keep_int;
                
                keep_buf[0] = '\0';
            }
        }


        switch (*p)
        {
        case 'd':
        {
            p++;
            _cat_sprint_int(buf, &buf_idx, va_arg(ap, cat_i32), width);
            width = 0;
            break;
        }
        case 's':
        {
            p++;
            _cat_sprint_string(buf, &buf_idx, va_arg(ap, cat_u8 *));
            break;
        }
        case 'x':
        {
            p++;
            cat_itoh(hex_str, va_arg(ap, cat_u32));
            if('\0' != hex_str[2])
            {
                _cat_sprint_string(buf, &buf_idx, hex_str);
            }
            else
            {
                _cat_sprint_string(buf, &buf_idx, hex_str);
                //cat_putchar('0');
                buf[buf_idx++] = '0';
            }
            break;
        }
        case 'f':
        {
            p++;
            _cat_sprint_float(buf, &buf_idx, va_arg(ap, cat_double), keep, width);
            break;
        }

        default:
            /* 可以考虑单独定义一个输出纯字符串的函数，可以打印错误信息 */
            _cat_print_error(CAT_ERROR_TYPE_ARG_TYPE_NOT_SUPPORT);
            ret = CAT_ERROR;
            break;
        }

        if(CAT_ERROR == ret)
        {
            break;
        }
    }

    va_end(ap);

    if(CAT_TRUE == with_end)
    {
        buf[buf_idx] = '\0';
    }

    return ret;
}

