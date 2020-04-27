#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/kdev_t.h>

#define DEVICE_NAME "my_chrdev"

#define CDEV_MAJOR 255
static int cdev_major = CDEV_MAJOR;

struct cdev *cdev;
dev_t devno;

static struct class *my_class;
struct device *dev;

struct my_private_data {
	unsigned long size;
	char buffer[20];
};

static struct my_private_data *pdata;

static ssize_t my_chrdev_size_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    ssize_t ret = 0;
	// 获取私有数据
    struct my_private_data *data = dev_get_drvdata(dev);

	pr_err("%s,%d, size: %d", __func__, __LINE__, data->size);

    ret = snprintf(buf, sizeof(int), "%d\n", data->size);

    return ret;
}

static ssize_t my_chrdev_size_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
    ssize_t ret = 0;

    struct my_private_data *data = dev_get_drvdata(dev);

	pr_err("%s,%d, size: %s", __func__, __LINE__, *buf);

	kstrtoul(buf, 10, &data->size);

    return size;
}

static DEVICE_ATTR(size, S_IRUSR|S_IWUSR, my_chrdev_size_show, my_chrdev_size_store);

static ssize_t my_chrdev_buffer_show(struct device *dev,
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

static ssize_t my_chrdev_buffer_store(struct device *dev,
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
static DEVICE_ATTR(buffer, S_IRUSR|S_IWUSR, my_chrdev_buffer_show, my_chrdev_buffer_store);

static struct attribute *my_chrdev_attrs[] = {
	&dev_attr_size.attr,
	&dev_attr_buffer.attr,
	NULL,
};

static const struct attribute_group my_chrdev_attrs_group = {
	.attrs = my_chrdev_attrs,
};

static ssize_t my_chrdev_read(struct file *filep,char *buf,size_t len,loff_t *off)
{
	pr_err("%s,%d", __func__, __LINE__);
	return len;
}

static ssize_t my_chrdev_write(struct file *filep,const char *buf,size_t len,loff_t *off)
{
	pr_err("%s,%d", __func__, __LINE__);
	return len;
}

static int my_chrdev_open(struct inode *inode,struct file *filep)
{
	pr_err("%s,%d", __func__, __LINE__);
	// open的时候可以将自己的私有数据保存在private_data指针中，那么在read_write的时候，也可以通过这个指针访问对应的数据
	// 有时候还需要在这里面识别次设备号
	filep->private_data = pdata;
	return 0;
}

static int my_chrdev_release(struct inode *inode,struct file *filep)
{
	pr_err("%s,%d", __func__, __LINE__);
	return 0;
}

static struct file_operations chrdev_fops = {
	.owner = THIS_MODULE,
	.read = my_chrdev_read,
	.write = my_chrdev_write,
	.open = my_chrdev_open,
	.release = my_chrdev_release,
};

static int __init my_chrdev_init(void)
{
	int rc = 0;

	pr_err("%s,%d", __func__, __LINE__);

	/* 申请设备号
	 * major为0，表示动态分配，非0表示静态分配
	 * 前面一定定义了major的值，所以这里是静态分配
	 */
	if (cdev_major) {
		// 设备编号，保存主设备号和次设备号
		devno = MKDEV(cdev_major, 0);
		// 静态分配设备号
		/* int register_chrdev_region(dev_t first, unsigned int count, const char *name); 
		 * first是申请的字符设备编号，count是连续设备的编号个数，
		 * name是和设备编号范围关联的设备名称, 将出现在/proc/devices和sysfs中
		 */
		rc = register_chrdev_region(devno, 1, DEVICE_NAME);
		pr_err("%s,%d, major: %d", __func__, __LINE__, cdev_major);
	} else {
		// 动态分配设备号
		/* int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, const char *name);
		 * dev是申请的字符设备编号，firstminor是要分配的设备的次设备号的起始值，count是连续设备的编号个数，name是设备名称
		 */
		rc = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
		// 获取主设备号
		cdev_major = MAJOR(devno);
		pr_err("%s,%d, major: %d", __func__, __LINE__, cdev_major);
	}
	// 申请字符设备空间，其实也可以不适用指针，直接定义一个变量,那么这个设备申请就可以省略
	// 把cdv结构嵌入到你自己封装的设备结构中
	cdev = cdev_alloc();
	// 字符设备结构体指向当前的模块
	cdev->owner = THIS_MODULE;
	// struect cdev结构体和file_operations结构体绑定
	cdev_init(cdev, &chrdev_fops);
	// 注册字符设备驱动
	// 注册之后可以在/proc/devices中读取到这个设备
	cdev_add(cdev, devno, 1);
   /* 可以申请私有数据，然后在open的时候，保存到filep->private_data指针中
    */
	// 可以用mknod /dev/my_chrdev c 255 0 创建字符设备节点
	// 为了省去手动创建设备节点，创建class,并将class注册到内核中
	// 在/sys/class/下创建chardev0的类
	my_class = class_create(THIS_MODULE, "chardev0");
	// 用户空间的udev会响应device_create函数，去/sys/class下寻找对应的类，在/dev下自动创建节点
	dev = device_create(my_class, NULL, devno, NULL, "chardev0");
	// 创建属性值, dev是device_create函数返回的struct device,  /sys/class/chrdev0/chrdev0/size
	pr_err("%s,%d", __func__, __LINE__);

	// 创建属性值
	rc = sysfs_create_group(&dev->kobj, &my_chrdev_attrs_group);
	pr_err("%s,%d", __func__, __LINE__);
	if (rc) {
		pr_err("%s,%d sys file create failed\n", __func__, __LINE__);
	}

	// 申请私有数据的空间, 并赋初值
    pdata = kzalloc(sizeof(struct my_private_data), GFP_KERNEL);
	pdata->size = 100;
	memset(pdata->buffer, 0, sizeof(pdata->buffer));
	memcpy(pdata->buffer, "tao", 4);

	// 将私有数据添加到struct device中
	dev_set_drvdata(dev, pdata);

	return rc;
}

static void __exit my_chrdev_exit(void)
{
	pr_err("%s,%d", __func__, __LINE__);
	// 删除创建的属性值
	sysfs_remove_group(&dev->kobj, &my_chrdev_attrs_group);
	
	// 首先取消掉对应的节点
	device_destroy(my_class, MKDEV(cdev_major, 0));
	// 再删除对应的类
	class_destroy(my_class);
	// 取消注册驱动
	cdev_del(cdev);
	// 释放设备号
	unregister_chrdev_region(devno, 1);
	kfree(pdata);
    return;
}

module_init(my_chrdev_init);
module_exit(my_chrdev_exit);

MODULE_DESCRIPTION("TAO char driver");
MODULE_AUTHOR("TAO LIU");
MODULE_LICENSE("GPL");
