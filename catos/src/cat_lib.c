/**
 * @file cat_lib.c
 * @author 文佳源 (648137125@qq.com)
 * @brief 基础功能库
 * @version 0.1
 * @date 2025-01-25
 * 
 * Copyright (c) 2025
 * 
 * @par 修订历史
 * <table>
 * <tr><th>版本 <th>作者 <th>日期 <th>修改内容
 * <tr><td>v1.0 <td>文佳源 <td>2025-01-25 <td>内容
 * <tr><td>v1.1 <td>文佳源 <td>2025-03-05 <td>1.修改链表类型结构体，去掉了"链表节点数"成员;2.增加了移除节点时清除节点前后指针,以表示自身未处于任何链表中;3.增加了插入节点时判断节点是否未处于任何链表中(出现表示致命错误,assert循环);
 * </table>
 */
#include "cat_lib.h"
#include "cat_stdio.h"

/****************** macros *******************/
#define CAT_SUPPORT_INT_LONG 10
/*************************************/

/****************** funcs *******************/
/* 位图 START */
cat_u32 cat_bitmap_get_wide(void)
{
    return 32;
}

void cat_bitmap_init(cat_bitmap *bitmap)
{
    bitmap->bitmap = 0;
}

void cat_bitmap_set(cat_bitmap *bitmap, cat_u32 pos)
{
    bitmap->bitmap |= (1 << pos);
}

void cat_bitmap_clr(cat_bitmap *bitmap, cat_u32 pos)
{
    bitmap->bitmap &= ~(1 << pos);
}

cat_u32 cat_bitmap_get_first_set(cat_bitmap *bitmap)
{
    static const cat_u8 quick_find_table[] = 
    {
        0xffu,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0x00to0x0F*/
        4u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0x10to0x1F*/
        5u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0x20to0x2F*/
        4u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0x30to0x3F*/
        6u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0x40to0x4F*/
        4u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0x50to0x5F*/
        5u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0x60to0x6F*/
        4u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0x70to0x7F*/
        7u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0x80to0x8F*/
        4u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0x90to0x9F*/
        5u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0xA0to0xAF*/
        4u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0xB0to0xBF*/
        6u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0xC0to0xCF*/
        4u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0xD0to0xDF*/
        5u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u,/*0xE0to0xEF*/
        4u,0u,1u,0u,2u,0u,1u,0u,3u,0u,1u,0u,2u,0u,1u,0u /*0xF0to0xFF*/
    };

    if(bitmap->bitmap & 0xff)
    {
        return quick_find_table[bitmap->bitmap & 0xff];
    }
    else if(bitmap->bitmap & 0xff00)
    {
        return quick_find_table[(bitmap->bitmap >> 8) & 0xff] + 8;
    }
    else if(bitmap->bitmap & 0xff0000)
    {
        return quick_find_table[(bitmap->bitmap >> 16) & 0xff] + 16;
    }
    else if(bitmap->bitmap & 0xff000000)
    {
        return quick_find_table[(bitmap->bitmap >> 24) & 0xff] + 24;
    }
    else
    {
        return cat_bitmap_get_wide();
    }
}
/* 位图 END */
/* 链表 START */

void cat_list_init(struct _cat_list_t *list)
{
    list->FIRST_NODE = &(list->head_node);
    list->LAST_NODE  = &(list->head_node);
}

void cat_list_node_init(struct _cat_node_t *node)
{
    node->next_node = node;
    node->pre_node = node;
}

cat_u32 cat_list_count(struct _cat_list_t *list)
{
    cat_u32 cnt = 0;

    /**
     * bugfix: 原写法等于跳过了第一个节点
     * 原: cat_node_t *node = list->FIRST_NODE;
     * 改: cat_node_t *node = &(list->head_node);
     */
    cat_node_t *node = &(list->head_node);
    while(node->next_node != &(list->head_node))
    {
        node = node->next_node;
        cnt++;
    }

    return cnt;
}

cat_bool cat_list_is_in(cat_list_t *list, cat_node_t *node)
{
    cat_bool ret = CAT_FALSE;

    cat_node_t *p;
    CAT_LIST_FOREACH_NO_REMOVE(list, p)
    {
        if(p == node)
        {
            ret = CAT_TRUE;
            break;
        }
    }

    return ret;
}

struct _cat_node_t *cat_list_first(struct _cat_list_t *list)
{
    struct _cat_node_t *node = CAT_NULL;
    
    if(list->FIRST_NODE != &(list->head_node))
    {
        node = list->FIRST_NODE;
    }

    return node;
}

struct _cat_node_t *cat_list_last(struct _cat_list_t *list)
{
    struct _cat_node_t *node = CAT_NULL;

    if(list->LAST_NODE != &(list->head_node))
    {
        node = list->LAST_NODE;
    }

    return node;
}

void cat_list_remove_node(struct _cat_list_t *list, struct _cat_node_t *node)
{
    (void)list;
    /* 也许什么时候需要验证节点是否属于该链表? 先留着吧, 就当说明一下从哪个链表移除 */

    /* 修改前后节点指针 */
    node->pre_node->next_node = node->next_node;
    node->next_node->pre_node = node->pre_node;

    node->next_node = node;
    node->pre_node = node;
}

void cat_list_remove_all(struct _cat_list_t *list)
{
    struct _cat_node_t *next_node;

    for(next_node = list->FIRST_NODE; next_node != &(list->head_node); )
    {
        /* 目前要移除的节点 */
        struct _cat_node_t *current_node = next_node;

        /* next_node前移,为下次循环做准备(要在移除current_node前赋值, 因为当前的current_node和next_node指向同一个节点, 等移除后该节点的next指针已经指向自己,就无法去往下一个节点) */
        next_node = next_node->next_node;

        /* 移除current_node */
        current_node->next_node = current_node;
        current_node->pre_node = current_node;
    }

    /* 将链表头节点指向自己 */
    list->FIRST_NODE = &(list->head_node);
    list->LAST_NODE  = &(list->head_node);
}

void cat_list_add_first(struct _cat_list_t *list, struct _cat_node_t *node)
{
    CAT_ASSERT(node->next_node == node);
    CAT_ASSERT(node->pre_node == node);

    node->pre_node = list->FIRST_NODE->pre_node;
    node->next_node = list->FIRST_NODE;

    list->FIRST_NODE->pre_node = node;
    list->FIRST_NODE = node;
}

void cat_list_add_last(struct _cat_list_t *list, struct _cat_node_t *node)
{
    CAT_ASSERT(node->next_node == node);
    CAT_ASSERT(node->pre_node == node);

    node->next_node = &(list->head_node);
    node->pre_node = list->LAST_NODE;

    list->LAST_NODE->next_node = node;
    list->LAST_NODE = node;
}

struct _cat_node_t *cat_list_remove_first(struct _cat_list_t *list)
{
    struct _cat_node_t *node = CAT_NULL;

    if(list->FIRST_NODE != &(list->head_node))
    {
        node = list->FIRST_NODE;

        cat_list_remove_node(list, node);
    }
    return node;
}

void cat_list_insert_after(
    struct _cat_list_t *list, 
    struct _cat_node_t *node_after, 
    struct _cat_node_t *node_to_insert
)
{
    (void)list;

    CAT_ASSERT(node_to_insert->next_node == node_to_insert);
    CAT_ASSERT(node_to_insert->pre_node == node_to_insert);

    node_to_insert->pre_node = node_after;
    node_to_insert->next_node = node_after->next_node;

    node_after->next_node->pre_node = node_to_insert;
    node_after->next_node = node_to_insert;
}
/* 链表 END */
/* 字符串相关 START */
cat_i32 cat_strcmp(const cat_u8 *str1, const cat_u8 *str2)
{
    cat_i32 ret = CAT_EOK;

    if((str1 == CAT_NULL) || (str2 == CAT_NULL))
    {
        CAT_SYS_PRINTF("[cat_strcmp] at least one arg is CAT_NULL!\r\n");
    }
    else
    {
        while((*str1 == *str2) && (*str1 != '\0'))
        {
            str1++;
            str2++;
        }
        
        if((*str1 == '\0') && (*str2 == '\0'))
        {
            ret = CAT_EOK;
        }
        else
        {
            ret = CAT_ERROR;
        }
    }

    return ret;
}

cat_i32 cat_strcpy(cat_u8 *dest, const cat_u8 *src, cat_u32 dest_len)
{
    //cat_u8 *p = src;
    cat_i32 ret = CAT_EOK;
    CAT_ASSERT(CAT_NULL != dest);
    CAT_ASSERT(CAT_NULL != src);
    cat_u32 i = 0;
    while(
        (src[i] != '\0') &&
        (i < dest_len)
    )
    {
        dest[i] = src[i];
        i++;
    }

    return ret;
}

cat_u32 cat_strlen(const cat_u8 *src)
{
    cat_u32 i = 0;

    while('\0' != src[i])
    {
        i++;
    }

    return i;
}

cat_i32 cat_atoi(cat_i32 *dest, const cat_u8 *src)
{
    CAT_ASSERT(dest);
    CAT_ASSERT(src);

    cat_i32 ret = CAT_EOK;
    cat_i32 temp = 0;
    cat_u8 sign = src[0];

    if(('-' == *src) || ('+' == *src))
    {
        src++;
    }
    while('\0' != *src)
    {
        if((*src < '0') || (*src > '9'))
        {
            ret = CAT_ERROR;
            break;
        }
        temp = temp * 10 + (*src - '0');
        src++;
    }
    if('-' == sign)
    {
        temp = -temp;
    }
		
		*dest = temp;

    return ret;
}

cat_i32 cat_itoa(cat_u8 *dest, cat_i32 src)
{
	(void)dest;
	(void)src;
	cat_i32 ret = CAT_EOK;

#if 0
    cat_u32 tmp = 0;
    /* cat_u32 max val = 4294967295 */
    cat_u8 buf[CAT_SUPPORT_INT_LONG] = {0};
    cat_u8 i = 0;

    if(num < 0)
    {
        *(dest++) = '-';
        num = -num;
    }

    /* 计算每一位的数字大小 */
    do
    {
        buf[i++] = num % 10;
        num = num / 10;

    }while((0 != num) && (i < CAT_SUPPORT_INT_LONG));

    /* 记录长度 */
    ret = i;

    while(i > 0)
    {
        *(dest++) = buf[--i] + '0';
    }
    *(dest) = '\0';
#endif //#if 0

	return ret;
}

/**
 * @brief 十六进制转有符号十进制
 * 
 * @param dest 目标buffer
 * @param src  源字符串
 * @return cat_i32 
 */
cat_i32 cat_htoi(cat_u32 *dest, const cat_u8 *src)
{
    CAT_ASSERT(dest);
    CAT_ASSERT(src);

    cat_i32 ret = CAT_EOK;
    cat_u32 temp = 0;

    //0x12
    if(
        (src[0] != '0') ||
        ((src[1] != 'x') && (src[1] != 'X'))
    )
    {
        ret = CAT_ERROR;
    }
    else
    {
        src += 2; //跳过0x
        while('\0' != *src)
        {
            if((*src >= '0') && (*src <= '9'))
            {
                temp = temp * 16 + (*src - '0');
                src++;
            }
            else if((*src >= 'A') && (*src <= 'F'))
            {
                temp = temp * 16 + (*src - 'A' + 10);
                src++;
            }
            else if((*src >= 'a') && (*src <= 'f'))
            {
                temp = temp * 16 + (*src - 'a' + 10);
                src++;
            }
            else
            {
                ret = CAT_ERROR;
                break;
            }
        } /* while */
    } /* else */

    if(CAT_EOK == ret)
    {
        *dest = temp;
    }

    return ret;
}

/**
 * @brief 十进制转十六进制字符串
 * 
 * @param dest 目标buffer
 * @param src  源字符串
 * @return cat_i32 
 * 
 * note: 先从低位到高位计算出字符串，再反转除 "0x" 的字符串
 */
cat_i32 cat_itoh(cat_u8 *dest, cat_u32 src)
{
    CAT_ASSERT(dest);

    cat_i32 ret = CAT_EOK;
    cat_u8 temp = 0;
    cat_u8 *start;

    start = dest + 2;   /* 16进制的起始位置 */

    *(dest++) = '0';
    *(dest++) = 'x';

    while(src != 0)
    {
        temp = src % 16;
        src /= 16;
        if(temp < 10)
        {
            *dest = temp + '0';
        }
        else
        {
            *dest = temp - 10 + 'A';
        }
        
        dest++;
    }

    *(dest--) = '\0';   /* 这之后要反转字符串，所以避开 "\0" */

    while(start < dest)
    {
        temp   = *start;
        *start = *dest;
        *dest  = temp;

        start++;
        dest--;
    }

    return ret;
}

/**
 * @brief 格式化内存块
 * 
 * @param start_addr    内存块起始地址
 * @param fill_with     填充的字符
 * @param size          内存块大小
 * @return void* 
 */
void *cat_memset(void *start_addr, cat_u8 fill_with, cat_u32 size)
{
/* 最大块长度(与架构位数有关) */
#define _CAT_LONG_BLK_SZ     (sizeof(cat_ubase))
/* 未对齐(目标地址低于最大块长度的位有非零位) */
#define _CAT_UNALIGNED(_addr)    ((cat_ubase)_addr & (_CAT_LONG_BLK_SZ - 1))
/* 内存块太小 */
#define _CAT_TOO_SMALL(_size)    ((_size) < _CAT_LONG_BLK_SZ)

    /**< 对齐的地址 */
    cat_ubase *aligned_addr;     
    /**< 没对齐或者碎片地址 */
    cat_u8 *piece_addr = (cat_u8 *)start_addr;   
    /**< 4/8字节填充缓冲 */    
    cat_ubase word_fill_buff;    

    if(
        !(_CAT_TOO_SMALL(size)) &&
        !_CAT_UNALIGNED(start_addr)
    )
    {
        aligned_addr = start_addr;

        if(_CAT_LONG_BLK_SZ == 4)
        {
            /* word_fill_buff = |fill_with|fill_with|fill_with|fill_with| */
            word_fill_buff = (cat_u32)(fill_with << 8) | fill_with;
            word_fill_buff |= (word_fill_buff << 16);
        }
        else
        {
            /* 目前只支持32位 */
            // CAT_ASSERT(_CAT_LONG_BLK_SZ == 4);
            /* 64位 */
            cat_u8 i;
            for(i=0; i<_CAT_LONG_BLK_SZ; i++)
            {
                word_fill_buff  = (fill_with << 8) | fill_with;
            }
        }

        /* 按每次4*_CAT_LONG_BLK_SZ个字节填充 */
        while(size >= (_CAT_LONG_BLK_SZ << 2))
        {
            *aligned_addr++ = word_fill_buff;
            *aligned_addr++ = word_fill_buff;
            *aligned_addr++ = word_fill_buff;
            *aligned_addr++ = word_fill_buff;
            size -= (_CAT_LONG_BLK_SZ << 2);
        }
        /* 按每次_CAT_LONG_BLK_SZ个字节填充 */
        while(size >= _CAT_LONG_BLK_SZ)
        {
            *aligned_addr++ = word_fill_buff;
            size -= _CAT_LONG_BLK_SZ;
        }

        /* 获取剩余的内存起始地址 */
        piece_addr = (cat_u8 *)aligned_addr;
    }

    while(size--)
    {
        *piece_addr++ = (cat_u8)fill_with;
    }

    return start_addr;

/* 取消宏定义 */
#undef _CAT_LONG_BLK_SZ
#undef _CAT_UNALIGNED
#undef _CAT_TOO_SMALL
}

/**
 * @brief 内存拷贝
 *        待优化为每次赋值多个字节
 * @param  dst              目的地址
 * @param  src              源地址
 * @param  bytes            拷贝长度(字节数)
 * @return void*            目标起始地址
 */
void *cat_memcpy(void *dst, const void *src, cat_ubase bytes)
{
    CAT_ASSERT(dst);
    CAT_ASSERT(src);

    /* TODO: 优化为每次赋值多个字节 */

    cat_u8 *p = (cat_u8 *)dst, *s = (cat_u8 *)src;

    if(p <= s || p > (s + bytes))
    {
        /* 顺序复制时不会出现复制时覆盖还未复制的src数据 */
        while(bytes--)
        {
            *(p++) = *(s++);
        }
    }
    else
    {
        /* 倒着复制 */
        cat_ubase left = bytes;
        while(left > 0)
        {
            p[left - 1] = s[left - 1];
        }
    }

    return dst;
}

/* 字符串相关 END */
/* 环形缓冲区 START */
#define IS_POWER_OF_TWO(_num) \
    ((_num & (_num - 1)) == 0)

#define RINGBUFFER_INDEX_MOVE_FOWARD(_ringbuffer, _index_name) \
    do{ \
        (_ringbuffer)->_index_name = (((_ringbuffer)->_index_name + 1) & ((_ringbuffer)->ring_mask)); \
    }while(0)

/**
 * @brief 检查环形缓冲区是否已满
 * 
 * @param  p_ringbuffer     环形缓冲区结构体指针
 * @return cat_u8      1：已满
 *                          0：未满
 */
static inline cat_u8 cat_ringbuffer_is_full(cat_ringbuffer_t *p_ringbuffer)
{
    cat_u8 ret;

    /**
     * 这里
     * ((p_ringbuffer->head_index - p_ringbuffer->tail_index) & (p_ringbuffer->ring_mask))
     * 这个最外层括号必须加，否则就会把(p_ringbuffer->ring_mask)) == 
        (p_ringbuffer->ring_mask)的结果(一定为真)和前面相减的结果按位与，就错辣！
     */
    ret = (
        ((p_ringbuffer->head_index - p_ringbuffer->tail_index) & (p_ringbuffer->ring_mask)) == 
        (p_ringbuffer->ring_mask)
        );

    return ret;
}

/**
 * @brief 检查环形缓冲区是否为空
 * 
 * @param  p_ringbuffer     环形缓冲区结构体指针
 * @return cat_u8      1：非空
 *                          0；为空
 */
static inline cat_u8 cat_ringbuffer_is_empty(cat_ringbuffer_t *p_ringbuffer)
{
    cat_u8 ret;

    ret = ((p_ringbuffer->head_index) == (p_ringbuffer->tail_index));

    return ret;
}

void cat_ringbuffer_init(cat_ringbuffer_t *p_ringbuffer, cat_u8 *p_buffer_space, cat_u32 buffer_size)
{
    CAT_ASSERT(CAT_NULL != p_ringbuffer);
    CAT_ASSERT(CAT_NULL != p_buffer_space);
    //CAT_ASSERT(buffer_size >= MIN_RINGBUFFER_SIZE);
    /* 如果要使用ring_mask来代替取余操作实现循环，buffer大小必须为2的n次方 */
    CAT_ASSERT(1 == (IS_POWER_OF_TWO(buffer_size)));

    p_ringbuffer->p_buffer       = p_buffer_space;
    p_ringbuffer->ring_mask      = buffer_size - 1;
    p_ringbuffer->tail_index     = 0;
    p_ringbuffer->head_index     = 0;
}

void cat_ringbuffer_clear(cat_ringbuffer_t *p_ringbuffer)
{
    CAT_ASSERT(CAT_NULL != p_ringbuffer);
    p_ringbuffer->tail_index     = 0;
    p_ringbuffer->head_index     = 0;
}

cat_i32 cat_ringbuffer_put(cat_ringbuffer_t *p_ringbuffer, cat_u8 data)
{
    cat_i32 ret = CAT_ERROR;

    if(0 != cat_ringbuffer_is_full(p_ringbuffer))
    {
        /* 如果满了就覆盖最早的 */
        //CAT_SYS_PRINTF("[ringbuffer] buffer overflow\r\n");

        /* 尾部索引往前走 */
        //p_ringbuffer->tail_index = ((p_ringbuffer->tail_index + 1) & (p_ringbuffer->ring_mask));
        RINGBUFFER_INDEX_MOVE_FOWARD(p_ringbuffer, tail_index);
    }
    else
    {
        ret = CAT_EOK;
    }

    p_ringbuffer->p_buffer[p_ringbuffer->head_index] = data;
    //p_ringbuffer->head_index = ((p_ringbuffer->head_index + 1) & (p_ringbuffer->ring_mask));
    RINGBUFFER_INDEX_MOVE_FOWARD(p_ringbuffer, head_index);

    return ret;
}

cat_i32 cat_ringbuffer_get(cat_ringbuffer_t *p_ringbuffer, cat_u8 *p_data)
{
    cat_i32 ret = CAT_ERROR;

    if(0 == cat_ringbuffer_is_empty(p_ringbuffer))
    {
        /* 不为空才执行 */
        *p_data = p_ringbuffer->p_buffer[p_ringbuffer->tail_index];
        //p_ringbuffer->tail_index = ((p_ringbuffer->tail_index + 1) & (p_ringbuffer->ring_mask));
        RINGBUFFER_INDEX_MOVE_FOWARD(p_ringbuffer, tail_index);

        ret = CAT_EOK;
    }

    return ret;
}

cat_i32 cat_ringbuffer_put_more(cat_ringbuffer_t *p_ringbuffer, const cat_u8 *p_data, cat_u32 size)
{
    cat_i32 ret = 0;
    cat_u32 i = 0;

    for(i=0; i<size; i++)
    {
        ret += cat_ringbuffer_put(p_ringbuffer, p_data[i]);
    }

    if(0 != ret)
    {
        ret = CAT_ERROR;
    }

    return ret;
}

cat_u32 cat_ringbuffer_get_more(cat_ringbuffer_t *p_ringbuffer, cat_u8 *p_data, cat_u32 size)
{
    cat_i32 err = CAT_ERROR;
    cat_u8 *p = CAT_NULL;
    cat_u32 cnt = 0;


    if(0 == cat_ringbuffer_is_empty(p_ringbuffer))
    {
        /* 不为空才继续 */
        p   = p_data;
        cnt = 0;

        do{
            err = cat_ringbuffer_get(p_ringbuffer, p);
            cnt++;
            p++;
        }while(
            (cnt < size) &&
            (CAT_EOK == err)
        );

    }

    return cnt;
}

/* 测试用函数 */
void cat_ringbuffer_print_all(cat_ringbuffer_t *p_ringbuffer)
{
    cat_u32 idx = p_ringbuffer->tail_index;

    CAT_KPRINTF("************\r\n****tail=%2d, head=%2d****\r\n", (p_ringbuffer->tail_index), (p_ringbuffer->head_index));
    while(idx != (p_ringbuffer->head_index))
    {
        CAT_KPRINTF("index=%2d, data=0x%x\r\n", idx, p_ringbuffer->p_buffer[idx]);
        idx = (idx + 1) & (p_ringbuffer->ring_mask);
    }
    CAT_KPRINTF("************\r\n");
}
/* 环形缓冲区 END */
