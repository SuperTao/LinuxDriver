#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/firmware.h>
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/bitops.h>

#define TSU6721_WR_INC_FLG              0x80

#define TSU6721_DEVICEID                0x01
#define TSU6721_CONTROL                 0x02
#define TSU6721_INTERRUPT1              0x03
#define TSU6721_INTERRUPT2              0x04
#define TSU6721_INTERRUPTMASK1          0x05
#define TSU6721_INTERRUPTMASK2          0x06
#define TSU6721_ADC                     0x07
#define TSU6721_TIMINGSET1              0x08
#define TSU6721_TIMINGSET2              0x09
#define TSU6721_DEVICETYPE1             0x0A
#define TSU6721_DEVICETYPE2             0x0B
#define TSU6721_BUTTON1                 0x0C
#define TSU6721_BUTTON2                 0x0D
#define TSU6721_MANUAL_S_OR_W1          0x13
#define TSU6721_MANUAL_S_OR_W2          0x14
#define TSU6721_DEVICETYPE3             0x15
#define TSU6721_RESET                   0x1B
#define TSU6721_TIMERSETTING            0x20
#define TSU6721_OCL_OR_OCP_SETTING1     0x21
#define TSU6721_OCL_OR_OCP_SETTING2     0x22
#define TSU6721_DEVICETYPE4             0x23

#define TSU6721_NUM_REGISER   21
#define TSU6721_MAX_ADDR      (TSU6721_DEVICETYPE4 + 1)
#define TSU6721_ALL_REGS      0xff

struct tsu6721_data {
    struct i2c_client *client;
    uint8_t reg_addr;
    uint8_t reg_data[TSU6721_MAX_ADDR];
} ;

static int tsu6721_i2c_read_seq(struct tsu6721_data *pdata, uint8_t reg_addr, uint8_t *reg_data, uint8_t num_reg)
{
    int ret = 0;
    uint8_t buf[2];
    struct i2c_msg msgs[2];
	// 检查传入的寄存器是否合法 
    if ((num_reg < 1) || (num_reg > TSU6721_NUM_REGISER)) {
        return -EINVAL;
    }
	// 寄存器地址	
    buf[0] = reg_addr | TSU6721_WR_INC_FLG;

	// 将i2c的地址和寄存器的地址发送给i2c客户端
	// 设备的i2c地址
    msgs[0].addr = pdata->client->addr,
	// 写操作
    msgs[0].flags = 0,
	// 长度
    msgs[0].len = 1,
	// 要读取的寄存器地址
    msgs[0].buf = &buf[0],

	// 开始读操作
	// 设备的i2c地址
    msgs[1].addr = pdata->client->addr,
	// 读操作
    msgs[1].flags = I2C_M_RD,
	// 读长度
    msgs[1].len = num_reg,
	// 读取的数据保存在reg_data中
    msgs[1].buf = reg_data,

	/* 将i2c数据发送出去,返回值表示操作是否成功。
	 * 后面的2表示通信的次数,这里有2次，一次写操作，一次读操作。
	 * 有一次寻址，发送i2c地址，就表示是一次操作。
	 */
    ret = i2c_transfer(pdata->client->adapter, msgs, 2);
    return ret;
}

// i2c写函数
static int tsu6721_i2c_write_seq(struct tsu6721_data *pdata, uint8_t reg_addr, uint8_t *reg_data, uint8_t num_reg)
{
    int ret = 0;
    struct i2c_msg msg;
    uint8_t buf[TSU6721_NUM_REGISER];

//    if ((num_reg < 1) || (num_reg >= TSU6721_NUM_REGISER)) {
//        return -EINVAL;
//    }
	// 获取设备的寄存器地址
    buf[0] = reg_addr | TSU6721_WR_INC_FLG;
    memcpy(&buf[1], reg_data, num_reg);

	// i2c地址
    msg.addr = pdata->client->addr;
	// 写操作
    msg.flags = 0;
	// 数据长度, 数据长度num_reg，还有一次是发送的寄存器地址
    msg.len = num_reg + 1;
	// 数据
    msg.buf = buf;
	// 发送数据
    ret = i2c_transfer(pdata->client->adapter, &msg, 1);
    return ret;
}

static int tsu6721_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	int err = 0;
    struct tsu6721_data *data;
	uint8_t reg_data;
	// 申请私有数据的空间
    data = devm_kzalloc(&client->dev, sizeof(struct tsu6721_data), GFP_KERNEL);
    if (!data) {
		err = -ENOMEM;
        pr_err("devm_kzalloc failed\n");
        goto probe_exit_err;
    }

	data->client = client;
	data->reg_addr = TSU6721_DEVICEID;

	// 保存数据到i2c client结构体中
    i2c_set_clientdata(client, data);

	tsu6721_i2c_read_seq(data, TSU6721_DEVICEID, &data->reg_data[TSU6721_DEVICEID], 1);
	pr_err("%s, %d: DEVICEID: %x\n", __func__, __LINE__, data->reg_data[TSU6721_DEVICEID]);
	tsu6721_i2c_read_seq(data, TSU6721_CONTROL, &data->reg_data[TSU6721_CONTROL], 1);
	pr_err("%s, %d: CONTROL: %x\n", __func__, __LINE__, data->reg_data[TSU6721_CONTROL]);
    data->reg_data[TSU6721_CONTROL] = 0x1e;
	tsu6721_i2c_write_seq(data, TSU6721_CONTROL, &data->reg_data[TSU6721_CONTROL], 1);
	tsu6721_i2c_read_seq(data, TSU6721_CONTROL, &data->reg_data[TSU6721_CONTROL], 1);
	pr_err("%s, %d: CONTROL: %x\n", __func__, __LINE__, data->reg_data[TSU6721_CONTROL]);

	return 0;

probe_exit_err:
	pr_err("%s, %d\n", __func__, __LINE__);
    i2c_unregister_device(client);

    return err;
}

static int tsu6721_remove(struct i2c_client *client)
{
	// 从i2c client结构体中获取私有数据
    struct tsu6721_data *data = i2c_get_clientdata(client); 
	pr_err("%s, %d\n", __func__, __LINE__);
	// 取消注册i2c设备
    i2c_unregister_device(client);
    return 0;
}

static struct of_device_id tsu6721_match_table[] = {
	// 与设备树匹配
    { .compatible = "ti,tsu6721",},
    { },
};

static const struct i2c_device_id tsu6721_id[] = {
	{"tsu6721", 0},
	{},
};

static struct i2c_driver tsu6721_driver = {
	// probe函数
	.probe = tsu6721_probe,
	.remove = tsu6721_remove,
	.driver = {
		   .name = "tsu6721",
		   .owner = THIS_MODULE,
		.of_match_table = tsu6721_match_table,
		   },
	.id_table = tsu6721_id,
};

int __init tsu6721_init(void)
{
	pr_err("%s, %d\n", __func__, __LINE__);
	return i2c_add_driver(&tsu6721_driver);
}
module_init(tsu6721_init);

void __exit tsu6721_exit(void)
{
	pr_err("%s, %d\n", __func__, __LINE__);
	i2c_del_driver(&tsu6721_driver);
}
module_exit(tsu6721_exit);

MODULE_DESCRIPTION("Texas Instruments tsu6721 driver");
MODULE_LICENSE("GPL v2");
