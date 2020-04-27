#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/spinlock.h>
#include <linux/string.h>

struct input_dev *input;

#define GPIO_KEY	90

static irqreturn_t my_key_interrupt(int irq, void *dev_id)
{
    int state = (gpio_get_value_cansleep(GPIO_KEY) ? 1 : 0) ^ 1;
	pr_err("%s, %d, state: %d\n", __func__, __LINE__, state);
	// 上报按键按键的状态，按下还是释放
	input_report_key(input, KEY_VOLUMEDOWN, !!state);
	//input_report_key(input, KEY_VOLUMEUP, !!state);
	// 立刻更新按键的状态
	input_sync(input);

	return IRQ_HANDLED;
}

static const struct of_device_id my_key_of_match[] = {
	{ .compatible = "tao,gpio-key", .data = NULL},
};

static int my_key_open(struct input_dev *input)
{
	pr_err("%s, %d\n", __func__, __LINE__);

	return 0;
}

static void my_key_close(struct input_dev *input)
{
	pr_err("%s, %d\n", __func__, __LINE__);
}

static int my_key_remove(struct platform_device *pdv)
{
	struct gpio_keys_drvdata *ddata = platform_get_drvdata(pdv);

	pr_err("%s, %d\n", __func__, __LINE__);

	input_unregister_device(input);

	return 0;
}

static int my_key_probe(struct platform_device *pdev)
{
	// 获取gpio的中断号
	int irq = gpio_to_irq(GPIO_KEY);
	int error = 0;

	pr_err("%s, %d\n", __func__, __LINE__);
	// 申请GPIO的使用权
	gpio_request(GPIO_KEY, "gpio_key");
	// 设置为输入
	gpio_direction_input(GPIO_KEY);
	// 设置中断处理函数，中断触发方式为上升沿和下降沿
	request_irq(irq, my_key_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_irq", NULL);

	// 申请input设备空间
	input = input_allocate_device();
	if (!input) {
		pr_err("failed to allocate input device\n");
		return -ENOMEM;
	}

	input->name = "my_key";			// 设备名称
	input->phys = "my_key/input0";
	input->dev.parent = &pdev->dev;
	input->open = my_key_open;
	input->close = my_key_close;
	input->id.bustype = BUS_HOST;	// 总线类型

	input->id.vendor = 0x0001;		// 生厂商编号
	input->id.product = 0x0001;		// 产品编号
	input->id.version = 0x0100;		// 版本编号
	/* 设置这个输入事件支持的事件类型，和对应的code
	 * 从之前的设备树中可以看到VOLUMEUP对应的是115
	 * 注册之后，在gpio中断函数中发送对应的input事件才有效
	 */
	input_set_capability(input, EV_KEY, KEY_VOLUMEDOWN);
	//input_set_capability(input, EV_KEY, 115);
	//input_set_capability(input, EV_KEY, KEY_VOLUMEUP);
	// 这样注册以后，按键的功能就是VOLUMEUP是一样的。
	// 同样可以增大音量
	error = input_register_device(input);
	if (error) {
		pr_err("Unable to register input device, error: %d\n",
			error);
		goto err_remove_group;
	}

	return 0;

err_remove_group:
	return error;
}

static struct platform_driver my_key_device_driver = {
	.probe		= my_key_probe,
	.remove     = my_key_remove,
	.driver		= {
		.name	= "gpio-key",
		.owner  = THIS_MODULE,
		.of_match_table = my_key_of_match,
	}
};

static int __init my_key_init(void)
{
	pr_err("%s, %d\n", __func__, __LINE__);
	return platform_driver_register(&my_key_device_driver);
}

static void __exit my_key_exit(void)
{
	pr_err("%s, %d\n", __func__, __LINE__);
	platform_driver_unregister(&my_key_device_driver);
}

module_init(my_key_init);
module_exit(my_key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tao");
MODULE_DESCRIPTION("simple key input demo");
