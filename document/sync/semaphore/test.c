#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>

struct test_s {
	struct miscdevice misc_dev;
	struct semaphore sem;
    spinlock_t lock;
};

static int my_misc_open(struct inode *node, struct file *file);
static int my_misc_release(struct inode *node, struct file *file);
static ssize_t my_misc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static ssize_t my_misc_write(struct file *file, const char *buf, size_t count, loff_t *ppos);


static struct file_operations misc_fops = {
	.owner		= THIS_MODULE,
	.open       = my_misc_open,
	.release    = my_misc_release,
	.write      = my_misc_write,
	.read		= my_misc_read,
};

static struct test_s test_t = {
    .misc_dev = {
		.name  = "my_misc",
		.minor = MISC_DYNAMIC_MINOR,
		.fops = &misc_fops,
    },
};

static int my_misc_open(struct inode *node, struct file *file)
{
	pr_err("%s,%d", __func__, __LINE__);

    file->private_data = &test_t;

    return 0;
}

static int my_misc_release(struct inode *node, struct file *file)
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
	int i, ret;
    static int cnt = 0;
    char kbuf[count + 1];
    /*test_t *p = file->private_data;*/

	pr_err("%s,%d", __func__, __LINE__);
    ret = copy_from_user(kbuf, buf, count);
    if(ret)
        return -EFAULT;
    kbuf[count] = '\0';

	// 信号量值减1
    //加不了锁,睡眠等待
    down(&test_t.sem);
/*	// 加不了锁，就直接退出
    if(down_trylock(&test_t.sem))
        return -EAGAIN;
*/
    for(i = 0; i < 3; i++)
    {
		pr_err("count = %d, %s", cnt++, kbuf);
        /*mdelay(10);*/
        msleep(10);
    }
	// 信号量值加1
    up(&test_t.sem);

    return count;
}

static int __init my_misc_init(void)
{
    int rc = 0;

	pr_err("%s,%d", __func__, __LINE__);
	// 初始化信号量，并设置可以同时几个程序可以获得锁。
	// 设置成1和2的结果是不一样的。
	sema_init(&test_t.sem, 2);
	//sema_init(&test_t.sem, 1);

    rc = misc_register(&test_t.misc_dev);
    if(rc < 0) {
		pr_err("%s,%d, fail to register misc\n", __func__, __LINE__);
    }

    return rc;
}

static void __exit my_misc_exit(void)
{
	pr_err("%s,%d", __func__, __LINE__);

    misc_deregister(&test_t.misc_dev);

    return;
}

module_init(my_misc_init);
module_exit(my_misc_exit);

MODULE_DESCRIPTION("TAO driver");
MODULE_AUTHOR("TAO LIU");
MODULE_LICENSE("GPL");
