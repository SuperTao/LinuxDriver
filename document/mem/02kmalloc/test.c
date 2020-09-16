#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>

void *virt = NULL;
int __init test_init(void)
{
	/*
	 * kmalloc使用GFP_KERNEL，有可能会休眠。
	 * 如果在中断中申请内存，需要使用GFP_ATOMIC
	 */
    /*virt = kmalloc(1234, GFP_KERNEL);*/
    /*virt = kmalloc(0x400000, GFP_KERNEL);*/
    virt = kzalloc(0x400000, GFP_KERNEL);
    if(!virt)
        return -ENOMEM;

    printk("virt = %p.\n", virt);

    return 0;
}


void __exit test_exit(void)
{
    kfree(virt);
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");


