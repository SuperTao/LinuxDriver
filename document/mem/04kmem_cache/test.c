#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>

struct kmem_cache *kc;
void *p[5];
int __init test_init(void)
{
    int i;
    kc = kmem_cache_create("kc", 16, 0, SLAB_HWCACHE_ALIGN, NULL);
    if(!kc)
        return -ENOMEM;

    for(i = 0; i < 5; i++)
    {
        p[i] = kmem_cache_alloc(kc, GFP_KERNEL);
        printk("p[%d] = %p.\n", i, p[i]);
    }

    return 0;
}


void __exit test_exit(void)
{
    int i;

    for(i = 0; i < 5; i++)
        kmem_cache_free(kc, p[i]);
    kmem_cache_destroy(kc);
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");


