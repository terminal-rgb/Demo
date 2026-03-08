#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int gpio_export(int pin)
{
    char buffer[8];
    int len = snprintf(buffer, sizeof(buffer), "%d", pin);

    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1)
        return -1;

    write(fd, buffer, len);
    close(fd);
    return 0;
}

int gpio_direction(int pin, const char *dir)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer),"/sys/class/gpio/gpio%d/direction", pin);

    int fd = open(buffer, O_WRONLY);
    if (fd == -1)
        return -1;

    write(fd, dir, strlen(dir));
    close(fd);
    return 0;
}

int gpio_write(int pin, int val)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", pin);

    int fd = open(buffer, O_WRONLY);
    if (fd == -1)
        return -1;

    if (val == 1)
        write(fd, "1", 1);
    else
        write(fd, "0", 1);

    close(fd);
    return 0;
}

int gpio_read(int pin)
{
    char buffer[64];
    char value[4];

    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", pin);

    int fd = open(buffer, O_RDONLY);
    if (fd == -1)
        return -1;

    if (read(fd, value, 1) == -1)
    {
        close(fd);
        return -1;
    }

    close(fd);

    if (value[0] == '1')
        return 1;

    return 0;
}

int gpio_unexport(int pin)
{
    char buffer[8];
    int len = snprintf(buffer, sizeof(buffer), "%d", pin);

    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1)
        return -1;

    write(fd, buffer, len);
    close(fd);
    return 0;
}

int main()
{
    int gpio_pin = 55;

    printf("Start Luckfox GPIO program\n");

    if (gpio_export(gpio_pin) == -1)
    {
        printf("Khong the export GPIO%d\n", gpio_pin);
        return -1;
    }

    usleep(100000);

    gpio_direction(gpio_pin, "out");

    for(int i = 0; i < 5; i++)
    {
        gpio_write(gpio_pin, 1);
        printf("LED ON\n");
        sleep(1);

        gpio_write(gpio_pin, 0);
        printf("LED OFF\n");
        sleep(1);
    }

    gpio_unexport(gpio_pin);

    printf("Completed\n");
    return 0;
}