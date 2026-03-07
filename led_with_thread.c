#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

int gpio_export(int pin)
{
    char buffer[8];
    int len = snprintf(buffer, sizeof(buffer), "%d", pin);
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) return -1;
    write(fd, buffer, len);
    close(fd);
    return 0;
}

int gpio_direction(int pin, const char *dir)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/direction", pin);
    int fd = open(buffer, O_WRONLY);
    if (fd == -1) return -1;
    write(fd, dir, strlen(dir));
    close(fd);
    return 0;
}

int gpio_valueout(int pin, int value)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", pin);
    int fd = open(buffer, O_WRONLY);
    if (fd == -1) return -1;

    if (value == 0)
        write(fd, "0", 1);
    else
        write(fd, "1", 1);

    close(fd);
    return 0;
}

int gpio_unexport(int pin)
{
    char buffer[8];
    int len = snprintf(buffer, sizeof(buffer), "%d", pin);
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1) return -1;
    write(fd, buffer, len);
    close(fd);
    return 0;
}

int trang_thai_led = 1;
pthread_mutex_t key;

void* blink_led(void *arg)
{
    int pin = *(int*)arg;
    int led_state;

    printf("Luong LED dang chay...\n");

    while(1)
    {
        pthread_mutex_lock(&key);
        led_state = trang_thai_led;
        pthread_mutex_unlock(&key);

        if(led_state == 0)
        {
            gpio_valueout(pin, 0);
            break;
        }

        gpio_valueout(pin, 1);
        usleep(500000);

        gpio_valueout(pin, 0);
        usleep(500000);
    }

    return NULL;
}

int main()
{
    pthread_t thread_led;
    int pin = 55;

    printf("Bat dau test GPIO %d...\n", pin);

    if (gpio_export(pin) == -1)
    {
        printf("Loi: Khong the export!\n");
        return -1;
    }

    usleep(100000);

    gpio_direction(pin, "out");

    pthread_mutex_init(&key, NULL);

    printf("Bat dau voi luong LED\n");

    pthread_create(&thread_led, NULL, blink_led, &pin);

    printf("Nhan Enter de tat LED...\n");
    getchar();

    pthread_mutex_lock(&key);
    trang_thai_led = 0;
    pthread_mutex_unlock(&key);

    pthread_join(thread_led, NULL);

    pthread_mutex_destroy(&key);

    gpio_unexport(pin);

    printf("Hoan thanh va don dep xong!\n");

    return 0;
}