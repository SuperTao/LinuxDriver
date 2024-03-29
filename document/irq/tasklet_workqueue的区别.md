转载：https://www.cnblogs.com/alantu2018/p/8527205.html

```
软中断、tasklet和工作队列并不是Linux内核中一直存在的机制，而是由更早版本的内核中的“下半部”（bottom half）演变而来。
下半部的机制实际上包括五种，但2.6版本的内核中，下半部和任务队列的函数都消失了，只剩下了前三者。
介绍这三种下半部实现之前，有必要说一下上半部与下半部的区别。
上半部指的是中断处理程序，下半部则指的是一些虽然与中断有相关性但是可以延后执行的任务。
举个例子：在网络传输中，网卡接收到数据包这个事件不一定需要马上被处理，适合用下半部去实现；但是用户敲击键盘这样的事件就必须马上被响应，应该用中断实现。
两者的主要区别在于：中断不能被相同类型的中断打断，而下半部依然可以被中断打断；中断对于时间非常敏感，而下半部基本上都是一些可以延迟的工作。
由于二者的这种区别，所以对于一个工作是放在上半部还是放在下半部去执行，可以参考下面4条：

如果一个任务对时间非常敏感，将其放在中断处理程序中执行。
如果一个任务和硬件相关，将其放在中断处理程序中执行。
如果一个任务要保证不被其他中断（特别是相同的中断）打断，将其放在中断处理程序中执行。
其他所有任务，考虑放在下半部去执行。
有写内核任务需要延后执行，因此才有的下半部，进而实现了三种实现下半部的方法。这就是本文要讨论的软中断、tasklet和工作队列。


1. 软中断
软中断作为下半部机制的代表，是随着SMP（share memory processor）的出现应运而生的，它也是tasklet实现的基础（tasklet实际上只是在软中断的基础上添加了一定的机制）。
软中断一般是“可延迟函数”的总称，有时候也包括了tasklet（请读者在遇到的时候根据上下文推断是否包含tasklet）。它的出现就是因为要满足上面所提出的上半部和下半部的区别，
使得对时间不敏感的任务延后执行，而且可以在多个CPU上并行执行，使得总的系统效率可以更高。它的特性包括：

产生后并不是马上可以执行，必须要等待内核的调度才能执行。软中断不能被自己打断(即单个cpu上软中断不能嵌套执行)，只能被硬件中断打断（上半部）。
可以并发运行在多个CPU上（即使同一类型的也可以）。所以软中断必须设计为可重入的函数（允许多个CPU同时操作），因此也需要使用自旋锁来保其数据结构。

2. tasklet
由于软中断必须使用可重入函数，这就导致设计上的复杂度变高，作为设备驱动程序的开发者来说，增加了负担。而如果某种应用并不需要在多个CPU上并行执行，
那么软中断其实是没有必要的。因此诞生了弥补以上两个要求的tasklet。它具有以下特性：
a）一种特定类型的tasklet只能运行在一个CPU上，不能并行，只能串行执行。
b）多个不同类型的tasklet可以并行在多个CPU上。
c）软中断是静态分配的，在内核编译好之后，就不能改变。但tasklet就灵活许多，可以在运行时改变（比如添加模块时）。
tasklet是在两种软中断类型的基础上实现的，因此如果不需要软中断的并行特性，tasklet就是最好的选择。也就是说tasklet是软中断的一种特殊用法，即延迟情况下的串行执行。

3. 工作队列
从上面的介绍看以看出，软中断运行在中断上下文中，因此不能阻塞和睡眠，而tasklet使用软中断实现，当然也不能阻塞和睡眠。
但如果某延迟处理函数需要睡眠或者阻塞呢？没关系工作队列就可以如您所愿了。
把推后执行的任务叫做工作（work），描述它的数据结构为work_struct ，这些工作以队列结构组织成工作队列（workqueue），
其数据结构为workqueue_struct ，而工作线程就是负责执行工作队列中的工作。系统默认的工作者线程为events。
工作队列(work queue)是另外一种将工作推后执行的形式。工作队列可以把工作推后，交由一个内核线程去执行—这个下半部分总是会在进程上下文执行，
但由于是内核线程，其不能访问用户空间。最重要特点的就是工作队列允许重新调度甚至是睡眠。
通常，在工作队列和软中断/tasklet中作出选择非常容易。可使用以下规则：
- 如果推后执行的任务需要睡眠，那么只能选择工作队列。
- 如果推后执行的任务需要延时指定的时间再触发，那么使用工作队列，因为其可以利用timer延时(内核定时器实现)。
- 如果推后执行的任务需要在一个tick之内处理，则使用软中断或tasklet，因为其可以抢占普通进程和内核线程，同时不可睡眠。
- 如果推后执行的任务对延迟的时间没有任何要求，则使用工作队列，此时通常为无关紧要的任务。
实际上，工作队列的本质就是将工作交给内核线程处理，因此其可以用内核线程替换。但是内核线程的创建和销毁对编程者的要求较高，
而工作队列实现了内核线程的封装，不易出错，所以我们也推荐使用工作队列。

```
