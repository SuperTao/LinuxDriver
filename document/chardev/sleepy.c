
#include <linux/module.h>
#include <linux/init.h>

#include <linux/sched.h>  /* current and everything */
#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/types.h>  /* size_t */
#include <linux/wait.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

MODULE_LICENSE("Dual BSD/GPL");

struct cdev *cdev;
static int sleepy_major = 255;
dev_t devno;
static struct class *my_class;
// 声明等待队列
static DECLARE_WAIT_QUEUE_HEAD(wq);
static int flag = 0;

ssize_t sleepy_read (struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	printk(KERN_DEBUG "process %i (%s) going to sleep\n",
			current->pid, current->comm);
	// 进入睡眠，唤醒条件flag != 0的时候唤醒。
	/* 读的时候，会进入睡眠，需要其他的程序唤醒
	 * 这里是在write函数中进行唤醒
	 */
	wait_event_interruptible(wq, flag != 0);
	flag = 0;
	printk(KERN_DEBUG "awoken %i (%s)\n", current->pid, current->comm);
	return 0; /* EOF */
}

ssize_t sleepy_write (struct file *filp, const char __user *buf, size_t count,
		loff_t *pos)
{
	printk(KERN_DEBUG "process %i (%s) awakening the readers...\n",
			current->pid, current->comm);
	/* 写操作，更改标志，并试图唤醒工作队列的程序
	 */
	flag = 1;
	wake_up_interruptible(&wq);
	return count; /* succeed, to avoid retrial */
}

struct file_operations sleepy_fops = {
	.owner = THIS_MODULE,
	.read =  sleepy_read,
	.write = sleepy_write,
};

int sleepy_init(void)
{
	int result;
	
	devno = MKDEV(sleepy_major, 0);
	result = register_chrdev_region(devno, 1, "sleepy");
	
	cdev = cdev_alloc();
	cdev->owner = THIS_MODULE;
	cdev_init(cdev, &sleepy_fops);
	cdev_add(cdev, devno, 1);
	
	my_class = class_create(THIS_MODULE, "sleepy");
	device_create(my_class, NULL, devno, NULL, "sleepy");
	
	return result;
}

void sleepy_cleanup(void)
{
	device_destroy(my_class, MKDEV(sleepy_major, 0));
	class_destroy(my_class);
	cdev_del(cdev);
	unregister_chrdev_region(devno, 1);
	return;
}

module_init(sleepy_init);
module_exit(sleepy_cleanup);
/*
 * 安装模块之后，打开两个窗口
 * 运行 cat /dev/sleep， cat程序会阻塞
 * 另一个窗口运行echo hello > /dev/sleep， cat进程结束。
 * log输出：
 *  process 3146 (cat) going to sleep
 *  process 3186 (bash) awakening the readers...
 *  awoken 3146 (cat)     // cat结束，但是如果write里面的flag不更改，还是0，那么不会有这条输出
 */

