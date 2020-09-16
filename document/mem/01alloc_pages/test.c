#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>

struct page *p;
/*void *virt = NULL;*/
unsigned long virt;
int __init test_init(void)
{
    /*
     *printk("order = %d\n", get_order(1234));
     *printk("order = %d\n", get_order(5000));
     */
/*
 *    p = alloc_pages(GFP_KERNEL, get_order(1234));
 *    if(!p)
 *        return -ENOMEM;
 *
 *    virt = page_address(p);
 *    printk("virt = %p.\n", virt);
 */
    virt = __get_free_pages(GFP_KERNEL, get_order(1234));
    if(!virt)
        return -ENOMEM;

    printk("virt = %p.\n", (void *)virt);


    return 0;
}


void __exit test_exit(void)
{
    /*__free_pages(p, get_order(1234));*/
    free_pages(virt, get_order(1234));
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");

