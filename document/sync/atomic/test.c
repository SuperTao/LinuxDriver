#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>

struct test_s {
	struct miscdevice misc_dev;
	atomic_t v;
    spinlock_t lock;
};

static int my_misc_open(struct inode *node, struct file *file);
static int my_misc_release(struct inode *node, struct file *file);


static struct file_operations misc_fops = {
	.owner		= THIS_MODULE,
	.open       = my_misc_open,
	.release    = my_misc_release,
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
	struct test_s *p;
	pr_err("%s,%d", __func__, __LINE__);

    file->private_data = &test_t;
	p = &test_t;
	// 从原子类型的变量中减去1，并判断结果是否为0，如果为0，返回真，否则返回假。
	if(!atomic_dec_and_test(&p->v))
    {
		pr_err("%s,%d", __func__, __LINE__);
        atomic_inc(&p->v);
        return -EAGAIN;
    }

    return 0;
}

static int my_misc_release(struct inode *node, struct file *file)
{
	struct test_s *p = file->private_data;
	pr_err("%s,%d", __func__, __LINE__);
	// 原子类型变量增加1
	atomic_inc(&p->v);
    return 0;
}

static int __init my_misc_init(void)
{
    int rc = 0;

	pr_err("%s,%d", __func__, __LINE__);
	// 设置原子类型的变量的值为i
	atomic_set(&test_t.v, 1);

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
