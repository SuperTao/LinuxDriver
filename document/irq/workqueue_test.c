#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#define GPIO_KEY	90

#define NUM     2

struct work_struct t[NUM];

struct workqueue_struct *wq;

void work_main(struct work_struct *work)
{
    int i;
    static int cnt;

    for(i = 0; i < 3; i++)
    {
        pr_err("work, count = %d\n", cnt++);
        //在进程上下文执行,可以sleep
        msleep(1000);
    }
}

static irqreturn_t irq_handler(int irq, void *arg)
{
    //int state = (gpio_get_value_cansleep(GPIO_KEY) ? 1 : 0) ^ 1;
    static int i;

    if(i == NUM)
        i = 0;

    if(!schedule_work(&t[i++]))
        pr_err("work is already in the queue.\n");

    return IRQ_HANDLED;
}

int __init tasklet_test_init(void)
{
	// 获取gpio的中断号
	int irq = gpio_to_irq(GPIO_KEY);

    int ret, i;

    //初始化工作结构, 工作队列及回调函数
    for(i = 0; i < NUM; i++)
        INIT_WORK(&t[i], work_main);

	pr_err("%s, %d\n", __func__, __LINE__);
	// 申请GPIO的使用权
	gpio_request(GPIO_KEY, "gpio_key");
	// 设置为输入
	gpio_direction_input(GPIO_KEY);
	// 设置中断处理函数，中断触发方式为上升沿和下降沿, 将锁作为参数传入
	ret = request_irq(irq, irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_irq", NULL);
	if (ret) {
		pr_err("%s, %d request irq fail\n", __func__, __LINE__);
		goto err1;

	}
	return ret;

err1:
    destroy_workqueue(wq);
    return ret;
}

void __exit tasklet_test_exit(void)
{
    free_irq(gpio_to_irq(GPIO_KEY), NULL);
    destroy_workqueue(wq);
}

module_init(tasklet_test_init);
module_exit(tasklet_test_exit);

