#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

struct timer_list timer;
void timer_main(unsigned long data)
{
	// 打印传入的数据
    printk("timer expire! data = %lu\n", data);

    /*
     *if(timer_pending(&timer))
     *    printk("timer_main: timer pending\n");
     *else
     *    printk("timer_main: timer NOT pending\n");
     */

    if(in_interrupt())
        printk("in interrupt context.\n");
    if(in_softirq())
        printk("in softirq context.\n");
    if(in_irq())
        printk("in irq context.\n");

    /*msleep(3000);*/
    /*
     *mdelay(3000);
     *printk("timer fn end\n");
     */
    /*mod_timer(&timer, jiffies + HZ);*/
}

int __init test_init(void)
{
	// 设置定时器调用的函数，以及传入的参数
    setup_timer(&timer, timer_main, 11223344);

    //设置时间,并激活定时器
    mod_timer(&timer, jiffies + 3 * HZ);	// 定时3秒
    mod_timer(&timer, jiffies + 1 * HZ);	// 定时1秒
	// 判断一个定时器是否被添加到了内核链表中以等待被调度运行
    if(timer_pending(&timer))
        printk("timer pending\n");
    else
        printk("timer NOT pending\n");

    return 0;
}


void __exit test_exit(void)
{
    /*del_timer(&timer);*/
    del_timer_sync(&timer);
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tao");

