## Learn RTOS

### 前言

如何真正掌握 RTOS？自己动手写个RTOS不就行了。

探索吧！Just for Fun.

### 第一阶段目标

对标FreeRTOS源码，参考野火的《FreeRTOS 内核实现与应用开发实战》.

写一个可以跑起来的Demo，模拟FreeRTOS。

#### [1. list.c 带头双向循环链表](docs/RTOS%20%E7%AC%AC%E4%B8%80%E6%AD%A5%20list.c%20%E5%B8%A6%E5%A4%B4%E5%8F%8C%E5%90%91%E5%BE%AA%E7%8E%AF%E9%93%BE%E8%A1%A8.md)

#### [2. task.c 任务的定义与任务切换的实现](docs/RTOS 第二步 task 创建任务与任务切换的实现.md)

#### [3. 临界段的保护](docs/临界保护.md)

1. [list.c 带头双向循环链表](docs/RTOS 第一步 list.c 带头双向循环链表.md)

2. [task.c 任务的定义与任务切换的实现](docs/RTOS 第二步 task 创建任务与任务切换的实现.md)

3. [临界段的保护](/docs/临界保护.md)

4. 

#### 4. 空闲任务与阻塞延时的实现

任务体内的延时使用的是软件延时，即还是让 CPU空等来达到延时的效果。使用 RTOS 的很大优势就是榨干 CPU 的性能，永远不能让它闲着，任务如果需要延时也就不能再让 CPU 空等来实现延时的效果。RTOS 中的延时叫阻塞延时，即任务需要延时的时候，任务会放弃 CPU的使用权，CPU可以去干其它的事情，当任务延时时间到，重新获取 CPU使用权，任务继续运行，这样就充分地利用了 CPU的资源，而不是干等着。
当任务需要延时，进入阻塞状态，那 CPU又去干什么事情了？如果没有其它任务可以运行，RTOS 都会为 CPU 创建一个空闲任务，这个时候 CPU 就运行空闲任务。在FreeRTOS中，空闲任务是系统在【启动调度器】的时候创建的优先级最低的任务，空闲任务主体主要是做一些系统内存的清理工作。但是为了简单起见，我们本章实现的空闲任务只是对一个全局变量进行计数。鉴于空闲任务的这种特性，在实际应用中，当系统进入空闲任务的时候，可在空闲任务中让单片机进入休眠或者低功耗等操作。

4.1 实现空闲任务

TCB_t IdleTaskTCB;

4.2 实现阻塞延时

4.2.1 vTaskDelay ()函数

阻塞延时的阻塞是指任务调用该延时函数后，任务会被剥离 CPU 使用权，然后进入阻塞状态，直到延时结束，任务重新获取 CPU 使用权才可以继续运行。在任务阻塞的这段时间，CPU可以去执行其它的任务，如果其它的任务也在延时状态，那么 CPU就将运行空闲任务。

4.2.2  vTaskSwitchContext()函数

调用 tashYIELD()会产生PendSV中断，在PendSV中断服务函数中会调用上下文切换函数 vTaskSwitchContext()，该函数的作用是寻找最高优先级的就绪任务，然后更新 pxCurrentTCB。

4.3 SysTick中断服务函数

在任务上下文切换函数 vTaskSwitchContext ()中，会判断每个任务的任务控制块中的延时成员 xTicksToDelay的值是否为 0，如果为 0就要将对应的任务就绪，如果不为 0就继续延时。如果一个任务要延时，一开始 xTicksToDelay 肯定不为 0，当 xTicksToDelay 变为0 的时候表示延时结束，那么 xTicksToDelay 是以什么周期在递减？在哪里递减？在FreeRTOS 中，这个周期由 SysTick 中断提供，操作系统里面的最小的时间单位就是SysTick 的中断周期，我们称之为一个 tick，SysTick 中断服务函数在 port.c 中实现。

4.3.1 xTaskIncrementTick()函数

更新系统时基.

4.4 SysTick初始化函数

SysTick 的中断服务函数要想被顺利执行，则 SysTick 必须先初始化。    

    - vPortSetupTimerInterrupt()函数.

 SysTick 初 始 化 函 数 vPortSetupTimerInterrupt() ， 在xPortStartScheduler()中被调用.

- 系统时钟的大小，因为目前是软件仿真，需要配置成与
  system_ARMCM3.c文件中的 SYSTEM_CLOCK的一样，即等于 25M。如果有具体的硬件，则配置成与硬件的系统时钟一样。

- SysTick 每秒中断多少次，目前配置为 100，即每 10ms中断一次。

##### - 注意

这部分与内核相关的知识比较多，TaskDelay() 与SysTick部分，在多次模拟运行之后，再结合内核部分的知识，相互印证，这里是一个重点。

#### 5. 支持多优先级

现在开始，任务中我们开始加入优先级的功能。在 FreeRTOS 中，数字优先级越小，逻辑优先级也越小，这与隔壁的 RT-Thread和 μC/OS 刚好相反。
