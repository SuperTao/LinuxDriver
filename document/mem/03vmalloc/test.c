#include <linux/module.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>

void *virt = NULL;
int __init test_init(void)
{
    virt = vmalloc(0x800000);
    if(!virt)
        return -ENOMEM;

    printk("virt = %p.\n", virt);

    return 0;
}


void __exit test_exit(void)
{
    vfree(virt);
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");


