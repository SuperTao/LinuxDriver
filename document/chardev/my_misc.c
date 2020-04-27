#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <soc/qcom/socinfo.h>

struct my_private_data {
	unsigned long size;
	char buffer[20];
};
static struct my_private_data *pdata;

static ssize_t my_misc_size_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    ssize_t ret = 0;
	// 获取私有数据
    struct my_private_data *data = dev_get_drvdata(dev);

	pr_err("%s,%d, size: %d", __func__, __LINE__, data->size);

    ret = snprintf(buf, sizeof(int), "%d\n", data->size);

    return ret;
}

static ssize_t my_misc_size_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
    ssize_t ret = 0;

    struct my_private_data *data = dev_get_drvdata(dev);

	pr_err("%s,%d, size: %s", __func__, __LINE__, *buf);

	kstrtoul(buf, 10, &data->size);

    return size;
}

static DEVICE_ATTR(size, S_IRUSR|S_IWUSR, my_misc_size_show, my_misc_size_store);

static ssize_t my_misc_buffer_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    ssize_t ret = 0;

	// 获取私有数据
    struct my_private_data *data = dev_get_drvdata(dev);
	// snprintf会把'\0'也算上，所以要+1才行
    ret = snprintf(buf, strlen(data->buffer) + 1,"%s\n", data->buffer);

	pr_err("%s,%d, length: %d, %s", __func__, __LINE__, strlen(data->buffer), data->buffer);
	pr_err("%s,%d, length: %d, buf: %s", __func__, __LINE__, strlen(buf), buf);

    return ret;
}

static ssize_t my_misc_buffer_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
    ssize_t ret = 0;

    struct my_private_data *data = dev_get_drvdata(dev);

	memset(data->buffer, 0, sizeof(data->buffer));
	memcpy(data->buffer, buf, size);

	pr_err("%s,%d", __func__, __LINE__);

    return size;
}
static DEVICE_ATTR(buffer, S_IRUSR|S_IWUSR, my_misc_buffer_show, my_misc_buffer_store);

static struct attribute *my_misc_attrs[] = {
	&dev_attr_size.attr,
	&dev_attr_buffer.attr,
	NULL,
};

static const struct attribute_group my_misc_attrs_group = {
	.attrs = my_misc_attrs,
};

static int my_misc_open(struct inode *node, struct file *filp)
{
	// 可以将私有数据保存到filep->private_data中
    //filp->private_data = misc;
	pr_err("%s,%d", __func__, __LINE__);
    return 0;
}

static int my_misc_release(struct inode *node, struct file *filp)
{
	pr_err("%s,%d", __func__, __LINE__);
    return 0;
}

static ssize_t my_misc_read(struct file *file, char __user *buf,
        size_t count, loff_t *ppos)
{
	pr_err("%s,%d", __func__, __LINE__);

    return count;
}

static ssize_t my_misc_write(struct file *file, const char *buf,
        size_t count, loff_t *ppos)
{
	pr_err("%s,%d", __func__, __LINE__);
    return count;
}

static const struct file_operations my_misc_fops = {
    .owner                  = THIS_MODULE,
    .open                   = my_misc_open,
    .release                = my_misc_release,
    .read                   = my_misc_read,
    .write                  = my_misc_write,
};

static struct miscdevice my_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "my_misc",
    .fops = &my_misc_fops,
};

static int __init my_misc_init(void)
{
    int rc = 0;

	pr_err("%s,%d", __func__, __LINE__);
	/* 创建misc设备节点，会创建/sys/class/misc/my_misc/, /dev/my_misc设备节点
	 */
    rc = misc_register(&my_misc_dev);
    if(rc < 0) {
		pr_err("%s,%d, fail to register misc\n", __func__, __LINE__);
		goto err_reg;
    }
	pr_err("%s,%d", __func__, __LINE__);
	rc = sysfs_create_group(&my_misc_dev.this_device->kobj, &my_misc_attrs_group);
	pr_err("%s,%d", __func__, __LINE__);
	if (rc) {
		pr_err("%s,%d sys file create failed\n", __func__, __LINE__);
		goto err;
	}
	// 申请私有数据的空间, 并赋初值
    pdata = kzalloc(sizeof(struct my_private_data), GFP_KERNEL);
	pdata->size = 100;
	memset(pdata->buffer, 0, sizeof(pdata->buffer));
	memcpy(pdata->buffer, "tao", 4);

	pr_err("%s,%d", __func__, __LINE__);
	dev_set_drvdata(my_misc_dev.this_device, pdata);
	pr_err("%s,%d", __func__, __LINE__);

err:
	pr_err("%s,%d", __func__, __LINE__);

err_reg:	
    return rc;
}

static void __exit my_misc_exit(void)
{
    misc_deregister(&my_misc_dev);

	pr_err("%s,%d", __func__, __LINE__);


    return;
}

module_init(my_misc_init);
module_exit(my_misc_exit);

MODULE_DESCRIPTION("TAO MISC driver");
MODULE_AUTHOR("TAO LIU");
MODULE_LICENSE("GPL");
