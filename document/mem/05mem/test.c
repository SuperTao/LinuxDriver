#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>

int __init test_init(void)
{
    printk("high_memory = %p\n", high_memory);
    printk("VMALLOC_START = %#lx\n", VMALLOC_START);
    printk("VMALLOC_END = %#lx\n", VMALLOC_END);
    printk("PAGE_OFFSET = %#lx\n", PAGE_OFFSET);
    printk("PHYS_OFFSET = %#lx\n", PHYS_OFFSET);

    return 0;
}


void __exit test_exit(void)
{
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");


