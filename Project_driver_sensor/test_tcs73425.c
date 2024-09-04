#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DEVICE_PATH "/dev/tcs34725" // Đường dẫn thiết bị của TCS34725
#define TCS34725_IOCTL_MAGIC 't'
#define TCS34725_IOCTL_READ_COLOR _IOR(TCS34725_IOCTL_MAGIC, 1, int[4])

int main() {
    int fd;
    int color_data[4]; // Array to hold the color data: [clear, red, green, blue]

    // Open the device
    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }

    // Read color data
    if (ioctl(fd, TCS34725_IOCTL_READ_COLOR, &color_data) < 0) {
        perror("Failed to read color data");
        close(fd);
        return errno;
    }

    // Print color data
    printf("Clear: %d\n", color_data[0]);
    printf("Red: %d\n", color_data[1]);
    printf("Green: %d\n", color_data[2]);
    printf("Blue: %d\n", color_data[3]);

    // Close the device
    close(fd);
    return 0;
}

