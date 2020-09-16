使用Linux挺久了，梳理一份关于Linux驱动的文档。

# 字符设备
	
*  [字符设备注册](./document/chardev/字符设备注册.md)
	
*  [设备节点创建](./document/chardev/设备节点创建.md)
	
*  [sysfs属性添加](./document/chardev/sysfs属性节点创建.md)

*  [proc文件添加](./document/chardev/proc/proc_create.c)

*  [阻塞IO操作](./document/chardev/sleepy.c)

# 杂项设备

*  [杂项设备创建](./document/chardev/杂项设备创建.md)

# 平台总线驱动

*  [平台设备创建](./document/platform/平台设备创建.md)

# 并发和竞态

*  [spinlock自旋锁](./document/sync/spinlock/test.c)

*  [semaphore信号量](./document/sync/semaphore/test.c)

*  [mutex互斥量](./document/sync/mutex/test.c)

*  [atomic原子操作](./document/sync/atomic/test.c)

*  [completion完成量](./document/sync/completion/test.c)  

*  [锁的区别](./document/sync/README.md)

# 时钟滴答和定时器

* [时钟滴答jiffies](./document/timer/jiffies/test.c)

* [定时器](./document/timer/timer/test.c)

# 内存分配

* [alloc_pages](./document/mem/01alloc_pages/test.c)

* [kmalloc](./document/mem/02kmalloc/test.c)

* [vmalloc](./document/mem/03vmalloc/test.c)

* [kmem_cache](./document/mem/04kmem_cache/test.c)

* [memory address](./document/mem/05mem/test.c)

* [dma](./document/mem/06dma/test.c)

* [err处理](./document/mem/07err/test.c)

#  线程

*  [thread](./document/thread/test.c)

#  中断处理

*  [gpio按键中断](./document/irq/irq_test.c)

*  [中断上下部](./document/irq/中断上下部.md)

*  [tasklet](./document/irq/tasklet_test.c)

*  [workqueue](./document/irq/workqueue_thread_test.c)

*  [tasklet&workqueue区别](./document/irq/tasklet_workqueue的区别.md)

# 输入子系统

*  [input输入子系统](./document/input/input输入子系统.md)

*  [i2c输入子系统](./document/input/i2c输入子系统.md)
