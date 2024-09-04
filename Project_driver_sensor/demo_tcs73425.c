#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <errno.h>

// Đường dẫn đến thiết bị I2C
#define I2C_DEVICE "/dev/i2c-1"
#define TCS34725_ADDR 0x29 // Địa chỉ I2C của TCS34725

// Định nghĩa các thanh ghi của TCS34725
#define TCS34725_COMMAND_BIT 0x80
#define TCS34725_ENABLE 0x00
#define TCS34725_ENABLE_AEN 0x02
#define TCS34725_ENABLE_PON 0x01
#define TCS34725_ATIME 0x01
#define TCS34725_CONTROL 0x0F
#define TCS34725_ID 0x12
#define TCS34725_STATUS 0x13
#define TCS34725_CDATAL 0x14
#define TCS34725_RDATAL 0x16
#define TCS34725_GDATAL 0x18
#define TCS34725_BDATAL 0x1A

// Hàm ghi một byte vào thanh ghi của TCS34725
void writeRegister(int file, uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {TCS34725_COMMAND_BIT | reg, value};
    if (write(file, buffer, 2) != 2) {
        perror("Failed to write to the i2c bus");
        close(file);
        exit(1);
    }
}

// Hàm đọc một từ 16-bit từ thanh ghi của TCS34725
uint16_t readRegister16(int file, uint8_t reg) {
    uint8_t buffer[1] = {TCS34725_COMMAND_BIT | reg};
    if (write(file, buffer, 1) != 1) {
        perror("Failed to write to the i2c bus");
        close(file);
        exit(1);
    }

    uint8_t data[2];
    if (read(file, data, 2) != 2) {
        perror("Failed to read from the i2c bus");
        close(file);
        exit(1);
    }

    return (data[1] << 8) | data[0];
}

int main() {
    int file;
    if ((file = open(I2C_DEVICE, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        exit(1);
    }

    if (ioctl(file, I2C_SLAVE, TCS34725_ADDR) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        close(file);
        exit(1);
    }

    // Bật nguồn và cảm biến
    writeRegister(file, TCS34725_ENABLE, TCS34725_ENABLE_PON);
    usleep(3000); // Chờ 3ms để bật nguồn
    writeRegister(file, TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);

    // Đặt thời gian tích hợp
    writeRegister(file, TCS34725_ATIME, 0xFF); // 2.4ms
    // Đặt gain
    writeRegister(file, TCS34725_CONTROL, 0x00); // 1x gain

    // Đọc dữ liệu màu từ cảm biến
    while (1) {
        uint16_t c = readRegister16(file, TCS34725_CDATAL);
        uint16_t r = readRegister16(file, TCS34725_RDATAL);
        uint16_t g = readRegister16(file, TCS34725_GDATAL);
        uint16_t b = readRegister16(file, TCS34725_BDATAL);

        printf("Clear: %d\n", c);
        printf("Red: %d\n", r);
        printf("Green: %d\n", g);
        printf("Blue: %d\n", b);

        usleep(500000); // Chờ 500ms trước khi đọc lại
    }

    close(file);
    return 0;
}

