#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>

#define SO_LUONG_LED 3

int trang_thai = 1;

pthread_mutex_t key;

typedef struct 
{
    char state;
    char kinh_do[32];
    char vi_do[32];
}gps_config;

typedef struct 
{
    int id; //stt led
    int pin;
    int sleep_time;
}led_config;


//Khoi tao chan
int gpio_export(int pin)
{
    char buffer[4];
    int len = snprintf(buffer, sizeof(buffer),"%d",pin);
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1)
    {
        perror("Loi: Khong the mo file set chan");
        return -1;
    }
    write(fd, buffer, len);
    close(fd);
    usleep(10000);
    return 0;
}

//direction in(button) or out(led)
int gpio_direction(int pin, const char* dir)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer),"/sys/class/gpio/gpio%d/direction", pin);
    int fd = open(buffer, O_WRONLY);
    if (fd == -1)
    {
        perror("Loi: Khong the set dir");
    }
    write(fd, dir, strlen(dir));
    close(fd);
    return 0;
}

int gpio_value(int pin, int value)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", pin);
    int fd = open(buffer, O_WRONLY);
    if (value == 1)
    {
        write(fd, "1", 1);
    }
    else if (value == 0)
    {
        write(fd, "0", 1);
    }
    else
    {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int read_gpio(int pin)
{
    char buffer[64];
    char value[4];
    snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", pin);
    int fd = open(buffer, O_RDONLY);
    if (read(fd, value, 1) == -1)
    {
        close(fd);
        return -1;
    }
    close(fd);
    if (value[0] == '1')
    {
        return 1;
    }
    return 0;
}

int unexport(int pin)
{
    char buffer[4];
    int len = snprintf(buffer, sizeof(buffer),"%d" ,pin);
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    write(fd, buffer, len);
    close(fd);
    return 0;
}

int uart_init(const char *path)
{
    struct termios tty;
    int fd = open(path, O_RDWR | O_NDELAY | O_NOCTTY);
    fcntl(fd, F_SETFL, 0);

    tcgetattr(fd, &tty);
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    tty.c_cflag &= ~CSIZE;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag |= CS8;
    tty.c_cflag |= CLOCAL | CREAD;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10;

    if(tcsetattr(fd, TCSANOW, &tty)== -1)
    {
        perror("Khong the cau hinh cho file uart");
        return -1;
    }
    return fd;
}

void* luong_led(void* arg)
{
    led_config *Led = (led_config*) arg;
    printf("Led %d chay tren pin %d\n", Led->id, Led->pin);

    while(1)
    {
        pthread_mutex_lock(&key);
        int state_led = trang_thai;
        pthread_mutex_unlock(&key);
        if(state_led == 0)
        {
            break;
        }
      
        gpio_value(Led->pin, 1);
        usleep(Led->sleep_time);
        gpio_value(Led->pin, 0);
        usleep(Led->sleep_time);
    }
    printf("Led %d da dung\n", Led->id);
    return NULL;
}

void toa_do(char* buffer, gps_config *Gps)
{
    char temp[15][20];
    Gps->state = '0';
    strcp(Gps->kinh_do, "0");
    strcp(Gps->vi_do, "0)";
    char* token = strtok(buffer,",");
    int i = 0;

    while(token != NULL && i<15)
    {
        strcpy(temp[i], token);
        i++;
        token = strtok(NULL,","); 
    }

    while(strcmp(temp[0], "$GPRMC") == 0)
    if(i<6 && temp[2][0] == 'A')
    {
        Gps->state = 'A';
        snprintf(Gps->kinh_do, sizeof(Gps->kinh_do), "%s %s", temp[3], temp[4]);
        snprintf(Gps->vi_do, sizeof(Gps->vi_do), "%s %s", temp[5], temp[6]);
    }

}

void* luong_gps(void *arg)
{
    int uart_fd = *(int*) arg;
    gps_config Gps;
    char buffer[64];
    while(1)
    {
        pthread_mutex_lock(&key);
        int gps_status = trang_thai;
        pthread_mutex_unlock(&key);
        if (gps_status == 0)
        {
            break;
        }
        memset(buffer, "\0", sizeof(buffer));
        int byte_read = read(uart_fd, buffer, sizeof(buffer)-1);
        if (byte_read > 0)
        {
            toa_do(buffer, &Gps);
        }

    }

}


int main()
{
    const char* uart_path = "/dev/ttyS3";
    int uart_fd = uart_init(uart_path);

    led_config Led[SO_LUONG_LED]={
    {1, 55, 20000},
    {2, 56, 100000},
    {3, 57, 45000}
    };

    pthread_t threads[SO_LUONG_LED];
    pthread_mutex_init(&key, NULL);

    for(int i=0; i<SO_LUONG_LED; i++)
    {
        gpio_export(Led[i].pin);
        gpio_direction(Led[i].pin, "out");
        pthread_create(&threads[i], NULL, luong_led, &Led[i]);
    }
    pthread_t gps;
    pthread_create(&gps, NULL, luong_gps, &uart_fd);

    printf("Dang chay...Led\n...");
    printf("GPS: %s %s\n", Gps.kinh_do, Gps.vi_do);
    printf("Nhan Enter de thoat");
    getchar();

    pthread_mutex_lock(&key);
    trang_thai = 0;
    pthread_mutex_unlock(&key);

    for (int i=0; i< SO_LUONG_LED; i++)
    {
        pthread_join(threads[i], NULL);
        unexport(Led[i].pin);
    }
    pthread_join(gps, NULL);
    close(uart_fd);
    
    pthread_mutex_destroy(&key);
    printf("Chuong trinh ket thuc");
    return 0;
}