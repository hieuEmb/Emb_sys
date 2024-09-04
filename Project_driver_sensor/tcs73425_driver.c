#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#define DRIVER_NAME "tcs34725_driver"

// Địa chỉ I2C của TCS34725
#define TCS34725_ADDR 0x29

// Địa chỉ các thanh ghi
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

// Khai báo con trỏ tcs34725_client trỏ tới cấu trúc i2c_client
static struct i2c_client *tcs34725_client;

// Hàm đọc dữ liệu từ cảm biến TCS34725
static int tcs34725_read_data(struct i2c_client *client)
{
    u8 buf[8];
    u16 c, r, g, b;

    // Đọc dữ liệu từ các thanh ghi
    if (i2c_smbus_read_i2c_block_data(client, TCS34725_REG_CDATAL, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read color data\n");
        return -EIO;
    }

    // Kết hợp các byte cao và thấp để tạo thành giá trị 16-bit
    c = (buf[1] << 8) | buf[0];
    r = (buf[3] << 8) | buf[2];
    g = (buf[5] << 8) | buf[4];
    b = (buf[7] << 8) | buf[6];

    // In ra dữ liệu màu
    printk(KERN_INFO "Color Data: C=%d, R=%d, G=%d, B=%d\n", c, r, g, b);

    return 0;
}

// Hàm này được gọi khi I2C phát hiện thiết bị
static int tcs34725_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;

    // Kích hoạt cảm biến và thiết lập thời gian tích hợp
    ret = i2c_smbus_write_byte_data(client, TCS34725_REG_ENABLE, 0x03); // Kích hoạt cảm biến
    if (ret < 0) {
        printk(KERN_ERR "Failed to enable TCS34725\n");
        return ret;
    }

    ret = i2c_smbus_write_byte_data(client, TCS34725_REG_ATIME, 0xFF); // Thiết lập thời gian tích hợp
    if (ret < 0) {
        printk(KERN_ERR "Failed to set integration time\n");
        return ret;
    }

    ret = i2c_smbus_write_byte_data(client, TCS34725_REG_CONTROL, 0x00); // Thiết lập gain
    if (ret < 0) {
        printk(KERN_ERR "Failed to set control register\n");
        return ret;
    }

    tcs34725_client = client; // Gán con trỏ client
    ret = tcs34725_read_data(client); // Gọi hàm đọc dữ liệu
    if (ret < 0) {
        return ret;
    }

    printk(KERN_INFO "TCS34725 driver installed\n");
    return 0;
}

static void tcs34725_remove(struct i2c_client *client)
{
    printk(KERN_INFO "TCS34725 driver removed\n");
}

// Định danh thiết bị
static const struct i2c_device_id tcs34725_id[] = {
    { "tcs34725", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, tcs34725_id);

// Định nghĩa cấu trúc driver I2C
static struct i2c_driver tcs34725_driver = {
    .driver = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
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
MODULE_DESCRIPTION("TCS34725 I2C Client Driver");
MODULE_LICENSE("GPL");

