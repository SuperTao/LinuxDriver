### spinlock自旋锁

* 加锁

spin_lock(&lock);       阻塞版本

spin_trylock(&lock);    不阻塞版本

* 解锁

spin_unlock(&lock);

* 使用场合:

保持锁时间很短

执行环境不允许睡眠

等待锁的过程中不休眠，而是占着CPU空转，优点是避免了上下文切换的开销，缺点是该CPU空转属于浪费，

spinlock适合用来保护快进快出的临界区。

自旋锁可以使用在中断处理程序中(此处不能使用信号量，因为他们会导致睡眠)。

### semaphore信号量

* 定义

struct semaphore sem;

* 初始化

sema_init(&test.sem, 1);

* 获取信号量

down(&p->sem);

down_interruptible(&p->sem);

down_trylock(&p->sem);

* 释放信号量

up(&p->sem);

* 定义一个信号量,并初始化

DEFINE_SEMAPHORE(name);

信号量的使用主要是用来保护共享资源，使得资源在一个时刻只有一个进程（线程）所拥有。

信号量的值为正的时候，说明它空闲。所测试的线程可以锁定而使用它。若为0，说明它被占用，测试的线程要进入睡眠队列中，等待被唤醒。

### mutex互斥锁

* 定义

struct mutex lock;

* 初始化

mutex_init(&lock);

* 加锁

mutex_lock(&lock);

mutex_lock_interruptible(&lock);

成功返回0

mutex_trylock(&lock);

成功返回1

* 解锁

mutex_unlock(&lock);

* 定义一个互斥锁,并初始化

DEFINE_MUTEX(mutexname);

互斥体实现了“互相排斥”（mutual exclusion）同步的简单形式（所以名为互斥体(mutex)）。互斥体禁止多个线程同时进入受保护的代码“临界区”（critical section）。因此，在任意时刻，只有一个线程被允许进入这样的代码保护区。mutex实际上是count=1情况下的semaphore。

信号量/互斥体允许进程睡眠属于睡眠锁，自旋锁则不允许调用者睡眠，而是让其循环等待:

1. 信号量和读写信号量适合于保持时间较长的情况，它们会导致调用者睡眠，因而自旋锁适合于保持时间非常短的情况

2. 自旋锁可以用于中断，不能用于进程上下文(会引起死锁)。而信号量不允许使用在中断中，而可以用于进程上下文

3. 自旋锁保持期间是抢占失效的，自旋锁被持有时，内核不能被抢占，而信号量和读写信号量保持期间是可以被抢占的

### atomic原子操作

atomic_t val;

atomic_t * v = &val;

读,修改原子变量的值

atomic_read(v);

atomic_set(v, i);

atomic_inc(v);      ---> v += 1;

atomic_dec(v);      ---> v -= 1;

atomic_inc_and_test(v)      v += 1,判断结果是否为0

atomic_dec_and_test(v)      v -= 1,判断结果是否为0

atomic_inc_return(v)

atomic_dec_return(v)

atomic_sub_and_test(i, v)

原子操作指在执行过程中不会被别的代码中断的操作。

### completion完成量

* 定义

struct completion com;

* 初始化

init_completion(&com);

* 等待完成

wait_for_completion(&com);

wait_for_completion_interruptible(&com);

* 通知完成量

complete(&com);

complete_all(&com);

* 定义一个完成量,并初始化

DECLARE_COMPLETION(work)

一般信号量的的处理会限制在一个函数内，但是有时会函数A的处理的前提条件是函数B，A必须等待B处理后才能继续，可以用信号量来进行处理，但linux kernel提供complete的方式。