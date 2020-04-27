#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

#define DEVICE_NAME "my_platform_chrdev"

#define CDEV_MAJOR 254
static int cdev_major = CDEV_MAJOR;
static int cdev_minor = 22;
dev_t devno;

struct my_platform_drvdata {
	struct platform_device *pdev;
	struct cdev *cdev;
	uint32_t size;
	char name[64];
};

static struct class *my_class;

static ssize_t my_platform_chrdev_read(struct file *filep,char *buf,size_t len,loff_t *off)
{
	pr_err("%s,%d", __func__, __LINE__);
	return len;
}

static ssize_t my_platform_chrdev_write(struct file *filep,const char *buf,size_t len,loff_t *off)
{
	pr_err("%s,%d", __func__, __LINE__);
	return len;
}

static int my_platform_chrdev_open(struct inode *inode,struct file *filep)
{
	pr_err("%s,%d", __func__, __LINE__);
	// open的时候可以将自己的私有数据保存在private_data指针中，那么在read_write的时候，也可以通过这个指针访问对应的数据
	// 有时候还需要在这里面识别次设备号
//	filep->private_data = xxxx;
	return 0;
}

static int my_platform_chrdev_release(struct inode *inode,struct file *filep)
{
	pr_err("%s,%d", __func__, __LINE__);
	return 0;
}

static struct file_operations platform_chrdev_fops = {
	.owner = THIS_MODULE,
	.read = my_platform_chrdev_read,
	.write = my_platform_chrdev_write,
	.open = my_platform_chrdev_open,
	.release = my_platform_chrdev_release,
};

static int32_t my_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	struct my_platform_drvdata *drvdata;
	struct device_node *node;
	const char *name;
	struct device *dev;

	pr_err("%s,%d", __func__, __LINE__);
	// 申请空间
	drvdata = kzalloc(sizeof(struct my_platform_drvdata), GFP_KERNEL);
	if (!drvdata) {
		pr_err("%s,%d: failed to malloc drvdata", __func__, __LINE__);
		rc = -ENOMEM;
		goto err;
	}


/*********************** 读取设备树参数 ***************************/
	node = pdev->dev.of_node;
	if (node == NULL) {
		pr_err("%s,%d: failed to parse my driver dtsi ", __func__, __LINE__);
		return -ENODEV;
	}
	// 读取设备树的参数	
	rc = of_property_read_u32(node, "tao,size", &drvdata->size);
	if (rc < 0) {
		pr_err("%s,%d: failed to read size", __func__, __LINE__);
		rc = -EINVAL;
		goto err_dt;
	}

	// 读取设备树的参数	
	rc = of_property_read_string(node, "tao,name", &name);
	if (rc < 0) {
		pr_err("%s,%d: failed to read name", __func__, __LINE__);
		rc = -EINVAL;
		goto err_dt;
	}

	memset(drvdata->name, 0, sizeof(drvdata->name));
	memcpy(drvdata->name, name, strlen(name));
	pr_err("%s,%d, name:%s", __func__, __LINE__, drvdata->name);
/*****************************************************************/

	// 将私有数据保存到pdev中
	platform_set_drvdata(pdev, drvdata);

	if (cdev_major) {
		devno = MKDEV(cdev_major, cdev_minor);
		rc = register_chrdev_region(devno, 1, DEVICE_NAME);
		pr_err("%s,%d, major: %d", __func__, __LINE__, cdev_major);
	} else {
		rc = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
		cdev_major = MAJOR(devno);
		pr_err("%s,%d, major: %d", __func__, __LINE__, cdev_major);
	}
	// 添加字符设备
	drvdata->cdev = cdev_alloc();
	drvdata->cdev->owner = THIS_MODULE;
	cdev_init(drvdata->cdev, &platform_chrdev_fops);
	cdev_add(drvdata->cdev, devno, 1);
	my_class = class_create(THIS_MODULE, "platform_chardev0");
	dev = device_create(my_class, NULL, devno, NULL, "platform_chardev0");
	pr_err("%s,%d", __func__, __LINE__);

	return 0;

err_dt:
	kfree(drvdata);
	drvdata = NULL;
err:
	return rc;
}

static int32_t my_platform_remove(struct platform_device *pdev)
{
	// 获取私有数据
    struct my_platform_drvdata *drvdata = platform_get_drvdata(pdev);
	class_destroy(my_class);
	cdev_del(drvdata->cdev);
	unregister_chrdev_region(devno, 1);
	kfree(drvdata);
	return 0;
}

static int my_platform_suspend(struct platform_device *pdev,pm_message_t state)
{
	return 0;
}

static int my_platform_resume(struct platform_device *pdev)
{
	return 0;
}

static void my_platform_shutdown(struct platform_device *pdev)
{
	return;
}

// 与设备树匹配
static const struct of_device_id my_platform_dt_match[] = {
	{.compatible = "tao,my_platform", .data = NULL},
};

struct platform_driver my_platform_device_driver={
	.driver = {
		.name = "tao,my_platform",
		.owner = THIS_MODULE,
		.of_match_table = my_platform_dt_match,
	},
	.remove = my_platform_remove,
	.suspend  = my_platform_suspend,
	.resume = my_platform_resume,
	.shutdown = my_platform_shutdown ,
	.probe = my_platform_probe,
};

static int __init my_platform_init(void)
{
	int32_t rc = 0;

	pr_err("%s,%d", __func__, __LINE__);
	rc = platform_driver_register(&my_platform_device_driver);

	return 0;
}

static void __init my_platform_exit(void)
{
	pr_err("%s,%d", __func__, __LINE__);
	platform_driver_unregister(&my_platform_device_driver);

	return;
}

module_init(my_platform_init);
module_exit(my_platform_exit);

MODULE_DESCRIPTION("Tao platform driver test");
MODULE_AUTHOR("Tao");
MODULE_LICENSE("GPL");
