#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

#define GPIO_KEY	90

static irqreturn_t irq_handler(int irq, void *arg)
{
    int state = (gpio_get_value_cansleep(GPIO_KEY) ? 1 : 0) ^ 1;
	pr_err("%s, %d, state: %d\n", __func__, __LINE__, state);

    return IRQ_HANDLED;
}

int __init irq_test_init(void)
{
	// 获取gpio的中断号
	int irq = gpio_to_irq(GPIO_KEY);

	pr_err("%s, %d\n", __func__, __LINE__);
	// 申请GPIO的使用权
	gpio_request(GPIO_KEY, "gpio_key");
	// 设置为输入
	gpio_direction_input(GPIO_KEY);
	// 设置中断处理函数，中断触发方式为上升沿和下降沿
	request_irq(irq, irq_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_irq", NULL);

	return 0;
}

void __exit irq_test_exit(void)
{
    free_irq(gpio_to_irq(GPIO_KEY), NULL);
}

module_init(irq_test_init);
module_exit(irq_test_exit);
