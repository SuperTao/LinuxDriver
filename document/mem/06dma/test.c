#include <linux/module.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>

dma_addr_t phys;    //物理地址  physical
void *virt;         //虚拟地址  virtual
int __init test_init(void)
{
    int val;

    virt = dma_alloc_coherent(NULL, 100, &phys, GFP_KERNEL);
    if(!virt)
        return -ENOMEM;

    printk("phys = %#x\n", phys);
    printk("virt = %p\n", virt);

    *(int *)virt = 11223344;

    /*virt = phys + PAGE_OFFSET - PHYS_OFFSET*/
    val = *(int *)(phys + PAGE_OFFSET - PHYS_OFFSET);
    printk("val = %d\n", val);



    return 0;
}


void __exit test_exit(void)
{
    dma_free_coherent(NULL, 100, virt, phys);
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");

