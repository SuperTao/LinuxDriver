写了一个简单的demo，放在kernel中可以直接运行。源码参考[my_key.c](../my_driver/my_key.c)

#### 设备树

手上的机器刚好有一个按键，对应GPIO第90脚。删除原来的GPIO90的定义，以免被其他按键占用。

首先添加设备树，保证platform_driver可以正常的运行probe函数。

```
--- a/arch/arm64/boot/dts/IDH60/msm8953-mtp.dtsi
+++ b/arch/arm64/boot/dts/IDH60/msm8953-mtp.dtsi
@@ -375,6 +375,17 @@
             gpios = <&tlmm 17 33>;
         };
     };
+       tao,gpio-key {
+        compatible = "tao,gpio-key";
+               label = "tao key";
+               gpios = <&tlmm 90 01>;
+        linux,input-type = <1>;
+        linux,code = <115>;
+        debounce-interval = <15>;
+        linux,can-disable;
+        gpio-key,wakeup;
+
+       };

/*
                scanner_rkey {
                        label = "scanner_rkey";
                        gpios = <&tlmm 90 0x1>;
*/
```

### 设置GPIO中断

为了省去步骤，就不写设备树解析的过程了，直接在驱动中设置GPIO。驱动注册之后，与设备树匹配，运行probe函数。

设置GPIO未输入，并设置为中断。
```
	// 获取gpio的中断号
	int irq = gpio_to_irq(GPIO_KEY);
	// 申请GPIO的使用权
	gpio_request(GPIO_KEY, "gpio_key");
	// 设置为输入
	gpio_direction_input(GPIO_KEY);
	// 设置中断处理函数，中断触发方式为上升沿和下降沿
	request_irq(irq, my_key_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_irq", NULL);
```

### 注册input事件

```
	// 申请input设备空间
	input = input_allocate_device();
	if (!input) {
		pr_err("failed to allocate input device\n");
		return -ENOMEM;
	}

	input->name = "my_key";			// 设备名称
	input->phys = "my_key/input0";
	input->dev.parent = &pdev->dev;
	input->open = my_key_open;
	input->close = my_key_close;
	input->id.bustype = BUS_HOST;	// 总线类型

	input->id.vendor = 0x0001;		// 生厂商编号
	input->id.product = 0x0001;		// 产品编号
	input->id.version = 0x0100;		// 版本编号
	/* 设置这个输入事件支持的事件类型，和对应的code
	 * 从之前的设备树中可以看到VOLUMEUP对应的是115
	 * 注册之后，在gpio中断函数中发送对应的input事件才有效
	 */
	input_set_capability(input, EV_KEY, KEY_VOLUMEDOWN);
	//input_set_capability(input, EV_KEY, 115);
	//input_set_capability(input, EV_KEY, KEY_VOLUMEUP);
	// 这样注册以后，按键的功能就是VOLUMEUP是一样的。
	// 同样可以增大音量
	error = input_register_device(input);
```

### input事件如何与gpio关联？

```
input通过如下函数，添加支持的时间，KEY_VOLUMEDOWN，可以调节音量。
	input_set_capability(input, EV_KEY, KEY_VOLUMEDOWN);

gpio中断的时候，发送对应的KEY_VOLUMEDOWN时间，这样就关联上了。
	input_report_key(input, KEY_VOLUMEDOWN, !!state);
	//input_report_key(input, KEY_VOLUMEUP, !!state);
	// 立刻更新按键的状态
	input_sync(input);
```

如果input时间要支持多个按键，可以同样用input_set_capability(input, EV_KEY, KEY_VOLUMEUP);继续添加

采用同一个中断函数，但是需要再中断函数中对不同的按键进行区分，上报不同的键值。

### 查看结果

驱动完成之后，按对应的按键，效果和音量减的效果是一样的。

查看驱动的log:

`logcat -b kernel | grep "my_key"`

```
01-02 03:58:05.292     0     0 E         : my_key_init, 132
01-02 03:58:05.293     0     0 E         : my_key_probe, 75
01-02 03:58:05.293     0     0 I input   : my_key as /devices/platform/soc/soc:tao,gpio-key/input/input3
01-02 03:58:05.293     0     0 D         : counters_connect my_key
01-02 03:58:05.293     0     0 E         : my_key_open, 48
01-02 03:58:05.510     0     0 E         : my_key_interrupt, 32, state: 0
按键按下
04-01 07:21:21.630     0     0 E         : my_key_interrupt, 32, state: 1
按键释放
04-01 07:21:22.385     0     0 E         : my_key_interrupt, 32, state: 0
```

查看新添加的系统在proc下的显示

`cat /proc/bus/input/devices`

```
I: Bus=0019 Vendor=0001 Product=0001 Version=0100	## 和代码的定义相同
N: Name="my_key"							## name
P: Phys=my_key/input0						## phys名称
S: Sysfs=/devices/platform/soc/soc:tao,gpio-key/input/input3
U: Uniq=
H: Handlers=event3 my_key cpufreq		## 中断函数的名称
B: PROP=0
B: EV=3
B: KEY=4000000000000 0

I: Bus=0019 Vendor=0001 Product=0001 Version=0100
N: Name="gpio-keys"
P: Phys=gpio-keys/input0
S: Sysfs=/devices/platform/soc/soc:gpio_keys/input/input4
U: Uniq=
H: Handlers=event4 gpio-keys cpufreq
B: PROP=0
B: EV=3
B: KEY=8000 8000000000000 0
```

查看实时事件：

`getevent -l`

```
could not get driver version for /dev/input/mice, Not a typewriter
add device 1: /dev/input/event2
  name:     "keyremap_virtual"
add device 2: /dev/input/event1
  name:     "goodix-ts"
add device 3: /dev/input/event0
  name:     "qpnp_pon"
add device 4: /dev/input/event4
  name:     "gpio-keys"
add device 5: /dev/input/event3
  name:     "my_key"				## my_key，和代码定义相同
## 按下
/dev/input/event3: EV_KEY       KEY_VOLUMEDOWN       DOWN
/dev/input/event3: EV_SYN       SYN_REPORT           00000000
## 释放
/dev/input/event3: EV_KEY       KEY_VOLUMEDOWN       UP
/dev/input/event3: EV_SYN       SYN_REPORT           00000000
```