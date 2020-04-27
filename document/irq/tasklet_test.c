#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#define GPIO_KEY	90

struct tasklet_struct task;
spinlock_t lock;

int critical(const char *s, spinlock_t *lock)
{
    int i;
    unsigned long flag;
    static int cnt = 0;

    /*spin_lock(lock);*/
    /*local_irq_disable();*/
    /*local_irq_save(flag);*/
    /*spin_lock_irq(lock);*/
    /*spin_lock_irqsave(lock, flag);*/
    spin_lock_bh(lock); //bottom half   关闭软中断,加锁

    for(i = 0; i < 3; i++)
    {
		pr_err("%s, %d, count = %d, %s\n", __func__, __LINE__, cnt++, s);
        mdelay(1000);
    }

    spin_unlock_bh(lock);
    /*spin_unlock_irqrestore(lock, flag);*/
    /*spin_unlock_irq(lock);*/
    /*local_irq_restore(flag);*/
    /*local_irq_enable();*/
    /*spin_unlock(lock);*/

    return 0;
}

void task_main(unsigned long data)
{
	// 取出参数
	spinlock_t *p = (spinlock_t *)data;
	pr_err("%s, %d\n", __func__, __LINE__);

	critical("softirq.\n", p);
}

static irqreturn_t irq_handler(int irq, void *arg)
{
    int state = (gpio_get_value_cansleep(GPIO_KEY) ? 1 : 0) ^ 1;
	// 中断传入的参数
	spinlock_t *p = (spinlock_t *)arg;

	pr_err("%s, %d, state: %d\n", __func__, __LINE__, state);
	// 调用队列，并传入参数,至于队列里面的任务什么时候运行，由cpu调度
	tasklet_schedule(&task);

    return IRQ_HANDLED;
}

int __init tasklet_test_init(void)
{
	// 获取gpio的中断号
	int irq = gpio_to_irq(GPIO_KEY);
	// 初始化tasklet,定义处理函数，传入参数
	tasklet_init(&task, task_main, (unsigned long)&lock);
	// 初始化自旋锁
	spin_lock_init(&lock);

	pr_err("%s, %d\n", __func__, __LINE__);
	// 申请GPIO的使用权
	gpio_request(GPIO_KEY, "gpio_key");
	// 设置为输入
	gpio_direction_input(GPIO_KEY);
	// 设置中断处理函数，中断触发方式为上升沿和下降沿, 将锁作为参数传入
	request_irq(irq, irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_irq", &lock);

	return 0;
}

void __exit tasklet_test_exit(void)
{
    free_irq(gpio_to_irq(GPIO_KEY), NULL);
}

module_init(tasklet_test_init);
module_exit(tasklet_test_exit);

