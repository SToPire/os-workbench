在调试`dev`模块时，我遇到这样一个问题：如果不创建任何线程（即只有`dev->init()`创建的`dev_input_task`和`dev_tty_task`两个线程），此时这两个线程会因为`read()`函数的信号量而双双陷入等待，造成`schedule()`函数的死循环。为了解决这个问题只要加入另一个trivial的线程供调度使用即可。

那么为什么这两个线程会被trivial的线程唤醒呢？其实并不是这个线程唤醒了它们，而是在初始化输入设备时注册了键盘中断和时钟中断的处理函数：

```c
  os->on_irq(0, _EVENT_IRQ_IODEV, input_notify);
  os->on_irq(0, _EVENT_IRQ_TIMER, input_notify);
static _Context *input_notify(_Event ev, _Context *context) {
  kmt->sem_signal(&sem_kbdirq);
  return NULL;
}
```

