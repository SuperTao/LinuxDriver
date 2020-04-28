#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>

struct task_struct *t;

int thread_main(void *data)
{
    printk("pid = %d\n", t->pid);
    while(1)
    {
        if(kthread_should_stop())
            break;

        /*printk("thread running.\n");*/
        msleep(3000);
    }

    return 123;
}

int __init test_init(void)
{
	//两中方法都可以创建线程
/*	// 第二个参数可以是传入线程的数据
 *    t = kthread_create(thread_main, NULL, "my_thread%d", 0);
 *    if(IS_ERR(t))
 *        return PTR_ERR(t);
 *
 *    wake_up_process(t);
 */
    t = kthread_run(thread_main, NULL, "my_thread%d", 0);
    if(IS_ERR(t))
        return PTR_ERR(t);

    return 0;
}



void __exit test_exit(void)
{
    int ret;
    ret = kthread_stop(t);
    printk("ret = %d\n", ret);
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tao Liu");
