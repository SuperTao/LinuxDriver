#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static int my_show(struct seq_file *file, void *data)
{
    seq_printf(file, "hello %s, proc read test\n", "tao");
	return 0;
}

static int test_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_show, NULL);
}

struct file_operations fops = {
    .owner      = THIS_MODULE,
    .open       = test_open,
    .release    = single_release,
    .read       = seq_read,
    .llseek     = seq_lseek,
};

struct proc_dir_entry *proc_dir;
int __init proc_init(void)
{
    //在proc文件系统上创建空目录
    proc_dir = proc_mkdir("proc_test_dir", NULL);
    if(!proc_dir)
        return -EFAULT;

    //在proc文件系统指定目录上创建文件
    proc_create("proc_test", 0644, proc_dir, &fops);
	// 另外一种创建方法create_proc_entry()现在已经被淘汰，就不写了。

    return 0;
}

void __exit proc_exit(void)
{
    //先删除proc文件系统上的目录
    remove_proc_entry("proc_test", proc_dir);
    //删除proc文件系统上proc_test目录的文件
    remove_proc_entry("proc_test_dir", NULL);
}

module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tao");
