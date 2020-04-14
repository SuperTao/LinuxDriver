sysfs属性节点有两种创建方式。主要是两个函数的区别。

device_create_file是一个函数创建一个属性。

sysfs_create_group运行一次，创建多个属性。

#### device_create_file

int device_create_file(struct device *dev,  const struct device_attribute *attr)

```
struct device *dev;
	dev = device_create(my_class, NULL, devno, NULL, "chardev0");
	// 创建属性值, dev是device_create函数返回的struct device,  /sys/class/chrdev0/chrdev0/size
    rc = device_create_file(dev, &dev_attr_size);
```

源码请参考[my_create_attr.c](../my_driver/my_create_attr.c)。

#### sysfs_create_group

原型如下，需要用到kobject结构体，而struct device中有这个结构体。
int sysfs_create_group(struct kobject *kobj,
               const struct attribute_group *grp)；
```
struct device *dev;		   
	dev = device_create(my_class, NULL, devno, NULL, "chardev0");
	// 创建属性值, dev是device_create函数返回的struct device,  /sys/class/chrdev0/chrdev0/size

	// 创建属性值
	rc = sysfs_create_group(&dev->kobj, &my_chrdev_attrs_group);
```

模块退出，也要删除对应属性。

```
	// 删除创建的属性值
	sysfs_remove_group(&dev->kobj, &my_chrdev_attrs_group);
```

源码请参考[my_create_group_attr.c](../my_driver/my_create_group_attr.c)。

#### 私有数据的访问

在设置完属性之后，可以将自己设置的私有属性，保存到struct device中。
```
	// 将私有数据添加到struct device中
	dev_set_drvdata(dev, pdata);
```

再操作属性值时，通过`deb_get_drvdata`来获取自己的私有数据。

```
static ssize_t my_chrdev_buffer_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
	// 获取私有数据
    struct my_private_data *data = dev_get_drvdata(dev);
```