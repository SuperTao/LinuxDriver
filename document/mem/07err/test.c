#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/err.h>

struct test_t {
    int n;
};

struct test_t *test_create(void)
{
    struct test_t *po;

    po = kmalloc(sizeof(struct test_t), GFP_KERNEL);
    /*po = kmalloc(0x900000, GFP_KERNEL);*/
    if(NULL == po)
        return ERR_PTR(-ENOMEM);

    return po;
}

void test_destroy(struct test_t *p)
{
    kfree(p);
}

struct test_t *p;
int __init test_init(void)
{
    p = test_create();
    if(IS_ERR(p))
        return PTR_ERR(p);

    return 0;
}


void __exit test_exit(void)
{
    test_destroy(p);
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");


