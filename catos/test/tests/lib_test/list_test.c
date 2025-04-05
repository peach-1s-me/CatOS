#include "catos.h"
#include "../cat_func_test.h"

#define ITEM_NUM 5

typedef struct _item_t
{
    cat_u32 value;
    cat_node_t node;
} item_t;

cat_list_t test_list_var;
item_t items[ITEM_NUM];


void test_list_func(void)
{
    cat_kprintf("test init\r\n");
    cat_list_init(&test_list_var);

    cat_u32 i;
    for(i=0; i<ITEM_NUM; i++)
    {
        cat_list_node_init(&(items[i].node));
        items[i].value = i;
    }

    CAT_TEST_INFO(cat_list_add_first, test add);
    for(i=0; i<ITEM_NUM; i++)
    {
        cat_list_add_first(&test_list_var, &(items[i].node));
    }

    cat_u32 cnt = cat_list_count(&test_list_var);
    cat_kprintf("add %d nodes\r\n", cnt);

    CAT_TEST_INFO(CAT_LIST_FOREACH, test for_each);
    cat_node_t *p = CAT_NULL;
    item_t *item = CAT_NULL;
    cat_kprintf("list=");
    CAT_LIST_FOREACH_NO_REMOVE(&test_list_var, p)
    {
        item = CAT_GET_CONTAINER(p, item_t, node);
        cat_kprintf("%d->", item->value);
    }
    cat_kprintf("\r\n");

    cat_list_remove_node(&(items[3].node));
    cat_kprintf("list=");
    CAT_LIST_FOREACH_NO_REMOVE(&test_list_var, p)
    {
        item = CAT_GET_CONTAINER(p, item_t, node);
        cat_kprintf("%d ", item->value);
    }
    cat_kprintf("\r\n");

    cat_list_remove_all(&test_list_var);
    cnt = cat_list_count(&test_list_var);
    cat_kprintf("there are %d nodes\r\n", cnt);

#if 0
    /* 测试两次插入同一节点会导致该节点产生自环，因此再进行遍历会死循环 */
    CAT_TEST_INFO(add, test double add);
    cat_list_add_first(&test_list_var, &(items[2].node));
    cat_list_add_first(&test_list_var, &(items[2].node));

    cat_kprintf("list=");
    CAT_LIST_FOREACH_NO_REMOVE(&test_list_var, p)
    {
        item = CAT_GET_CONTAINER(p, item_t, node);
        cat_kprintf("%d ", item->value);
    }
    cat_kprintf("\r\n");
#endif


}

#if (CATOS_CAT_SHELL_ENABLE == 1)
#include "cat_shell.h"
#include "cat_stdio.h"
void *do_test_list(void *arg)
{
    (void)arg;

    test_list_func();

    return CAT_NULL;
}
CAT_DECLARE_CMD(test_list, test list, do_test_list);
#endif
