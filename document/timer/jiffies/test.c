#include <linux/module.h>
#include <linux/delay.h>

void my_mdelay(int msec)
{
    unsigned long expire = jiffies + msecs_to_jiffies(msec);

    while(time_is_after_jiffies(expire))
        ;
}

int __init test_init(void)
{
    printk("jifffies is %lu\n", jiffies);

    /*msleep(2000);*/
    /*mdelay(2000);*/
    my_mdelay(2000);
    printk("jifffies is %lu\n", jiffies);

    return 0;
}


void __exit test_exit(void)
{
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tao");

