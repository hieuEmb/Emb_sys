#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DRIVER_NAME "tcs34725_driver"
#define CLASS_NAME "tcs34725"
#define DEVICE_NAME "tcs34725"

#define TCS34725_ADDR 0x29
#define TCS34725_REG_ENABLE 0x00
#define TCS34725_REG_ATIME 0x01
#define TCS34725_REG_CONTROL 0x0F
#define TCS34725_REG_ID 0x12
#define TCS34725_REG_CDATAL 0x14
#define TCS34725_REG_CDATAH 0x15
#define TCS34725_REG_RDATAL 0x16
#define TCS34725_REG_RDATAH 0x17
#define TCS34725_REG_GDATAL 0x18
#define TCS34725_REG_GDATAH 0x19
#define TCS34725_REG_BDATAL 0x1A
#define TCS34725_REG_BDATAH 0x1B

#define TCS34725_IOCTL_MAGIC 't'
#define TCS34725_IOCTL_READ_COLOR _IOR(TCS34725_IOCTL_MAGIC, 1, int[4])

static struct i2c_client *tcs34725_client;
static struct class* tcs34725_class = NULL;
static struct device* tcs34725_device = NULL;
static int major_number;

static int tcs34725_read_color(struct i2c_client *client, int *data)
{
    u8 buf[8];
    if (i2c_smbus_read_i2c_block_data(client, TCS34725_REG_CDATAL, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read color data\n");
        return -EIO;
    }

    data[0] = (buf[1] << 8) | buf[0]; // Clear channel
    data[1] = (buf[3] << 8) | buf[2]; // Red channel
    data[2] = (buf[5] << 8) | buf[4]; // Green channel
    data[3] = (buf[7] << 8) | buf[6]; // Blue channel

    return 0;
}

static long tcs34725_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int color_data[4];

    switch (cmd) {
        case TCS34725_IOCTL_READ_COLOR:
            if (tcs34725_read_color(tcs34725_client, color_data) < 0)
                return -EIO;
            if (copy_to_user((int __user *)arg, color_data, sizeof(color_data)))
                return -EFAULT;
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

static int tcs34725_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "TCS34725 device opened\n");
    return 0;
}

static int tcs34725_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "TCS34725 device closed\n");
    return 0;
}

static struct file_operations fops = {
    .open = tcs34725_open,
    .unlocked_ioctl = tcs34725_ioctl,
    .release = tcs34725_release,
};

static int tcs34725_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;

    tcs34725_client = client;

    // Enable the sensor
    ret = i2c_smbus_write_byte_data(client, TCS34725_REG_ENABLE, 0x03);
    if (ret < 0) {
        printk(KERN_ERR "Failed to enable TCS34725\n");
        return ret;
    }

    // Set integration time
    ret = i2c_smbus_write_byte_data(client, TCS34725_REG_ATIME, 0xFF);
    if (ret < 0) {
        printk(KERN_ERR "Failed to set integration time\n");
        return ret;
    }

    // Set control register
    ret = i2c_smbus_write_byte_data(client, TCS34725_REG_CONTROL, 0x00);
    if (ret < 0) {
        printk(KERN_ERR "Failed to set control register\n");
        return ret;
    }

    // Create a char device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register a major number\n");
        return major_number;
    }

    tcs34725_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(tcs34725_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to register device class\n");
        return PTR_ERR(tcs34725_class);
    }

    tcs34725_device = device_create(tcs34725_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(tcs34725_device)) {
        class_destroy(tcs34725_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to create the device\n");
        return PTR_ERR(tcs34725_device);
    }

    printk(KERN_INFO "TCS34725 driver installed\n");
    return 0;
}

static void tcs34725_remove(struct i2c_client *client)
{
    device_destroy(tcs34725_class, MKDEV(major_number, 0));
    class_unregister(tcs34725_class);
    class_destroy(tcs34725_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_INFO "TCS34725 driver removed\n");
}

static const struct i2c_device_id tcs34725_id[] = {
    { "tcs34725", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, tcs34725_id);

static const struct of_device_id tcs34725_of_match[] = {
    { .compatible = "ams,tcs34725", },
    { },
};
MODULE_DEVICE_TABLE(of, tcs34725_of_match);

static struct i2c_driver tcs34725_driver = {
    .driver = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(tcs34725_of_match),
    },
    .probe      = tcs34725_probe,
    .remove     = tcs34725_remove,
    .id_table   = tcs34725_id,
};

static int __init tcs34725_init(void)
{
    printk(KERN_INFO "Initializing TCS34725 driver\n");
    return i2c_add_driver(&tcs34725_driver);
}

static void __exit tcs34725_exit(void)
{
    printk(KERN_INFO "Exiting TCS34725 driver\n");
    i2c_del_driver(&tcs34725_driver);
}

module_init(tcs34725_init);
module_exit(tcs34725_exit);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("TCS34725 I2C Client Driver with IOCTL Interface");
MODULE_LICENSE("GPL");

