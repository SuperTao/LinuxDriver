机器里有一个tsu6721芯片，是通过I2C通信的。通过读取其中的几个寄存器来验证i2c通信。

源码参考[my_i2c](../my_driver/my_i2c.c)

### 添加设备树

arch/arm64/boot/dts/IDH60/msm8953-mtp.dtsi

```
&i2c_2 {
    status = "okay"; 

       tsu6721@25 {
               reg = <0x25>;
               compatible = "ti,tsu6721";
               tsu6721,irq-gpio = <&tlmm 97 0x02>; /* IRQF_TRIGGER_FALLING */
       };  
};
```

### 编写驱动

通过i2c_add_driver()注册i2c驱动。

驱动通过和"ti,tsu6721"匹配，运行probe函数。

probe函数运行后，可以通过struct i2c_client的addr获取设备地址25。

通信只需要填充struct i2c_msg结构体，然后通过i2c_transfer()发送即可。

### 查看效果

`logcat -b kernel | grep "tsu6721"`

```
01-01 05:37:54.992     0     0 E         : tsu6721_init, 199
01-01 05:37:54.993     0     0 E tsu6721_probe, 147: DEVICEID: 12
01-01 05:37:54.995     0     0 E tsu6721_probe, 149: CONTROL: 1f
01-01 05:37:54.999     0     0 E tsu6721_probe, 153: CONTROL: 1e
```