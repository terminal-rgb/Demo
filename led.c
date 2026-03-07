#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int gpio_export(int pin)
{
    char buffer[4];
    int len = snprintf(buffer, sizeof(buffer),%d, pin);
    int fd = open("sys/class/gpio/export", O_WRONLY);
    if fd == -1
    {
        return -1;
    }
    write(fd, buffer, len);
    close(fd);
    return 0;
}

int gpio_direction(int pin, const char *dir)
{
    char buffer[64];
    int path = snprintf(buffer, sizeof(buffer), "sys/class/gpio/gpio%d/direction",pin);
    int fd = open(path, O_WRONLY);
    if fd == -1 return -1;
    write(fd, dir, strlen(dir));
    close(fd);
    return 0;
}

int gpio_write(int pin, int val)
{
    char buffer[64];
    int path = snprintf(buffer, sizeof(buffer), "sys/class/gpio/gpio%d/value",pin);
    int fd = open(path, O_WRONLY);
    if fd == -1 return -1;
    if val == 1
    {
        write(fd, '1', 1);
    }
    else if(val == 0)
    {
        write(fd, "0", 1);
    }
    close(fd);
    return 0;
}

int gpio_read(int pin, int val)
{
    char buffer[64];
    char value[4];
    snprinf(buffer, sizeof(buffer),"sys/class/gpio/gpio%d/value",pin);
    int fd = open(path, O_RDONLY);
    if fd == -1 return -1;
    if (read(fd, value, 1) == -1
    {
        close(fd);
        return -1
    }
    close(fd);
    if read(fd, value[0], "1")
    {
        return 1;
    }
    return 0;

}

int gpio_unexport(int pin)
{
    char buffer[4];
    int len = snprinf(buffer, sizeof(buffer), %d, pin);
    int fd = open("sys/class/gpio/gpio%d/unexport",pin);
    if fd == -1 return -1;
    write(fd, len, sizeof(len));
    close(fd);
    return 0;
}

int main
{
    int gpio_pin = 55;
    printf("Start this Luckfox programme");
    if(gpio_export(gpio_pin) == -1)
    {
        printf("Chân gpio%d chưa được khởi tạo",gpio_pin);
        return -1;
    }
    usleep(1);
    gpio_direction(gpio_pin, "out")
    for(int i=0; i<5 i++)
    {
        if(gpio_write(gpio_pin, 1)
        {
            printf("Led On\n");
            sleep(1);
        }
        else if(gpio_write(gpio_pin, 0))
        {
            printf(Led Off\n);
            sleep(1);
        }
    }
    gpio_unexport(gpio_pin);
    printf("Completed\n");
}