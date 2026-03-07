#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>

int trang_thai = 1;
pthread_mutex_t key;

typedef struct
{
    char state;
    char kinh_do[20];
    char vi_do[20];
} gps;

int uart_init(const char *path)
{
    struct termios tty;
    int fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd < 0)
    {
        perror("open uart");
        return -1;
    }

    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd, &tty);

    cfmakeraw(&tty);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~PARENB;

    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10;

    tcsetattr(fd, TCSANOW, &tty);

    return fd;
}

void toa_do(char* buffer, gps* GPS)
{
    GPS->state = '\0';
    memset(GPS->kinh_do, 0, sizeof(GPS->kinh_do));
    memset(GPS->vi_do, 0, sizeof(GPS->vi_do));

    char temp[15][20];
    int i = 0;

    char *token = strtok(buffer, ",");

    while(token != NULL && i < 15)
    {
        strcpy(temp[i], token);
        i++;
        token = strtok(NULL,",");
    }

    if(i > 6 && strcmp(temp[0], "$GPRMC") == 0)
    {
        if(temp[2][0] == 'A')
        {
            GPS->state = 'A';

            snprintf(GPS->kinh_do, sizeof(GPS->kinh_do),"%.10s %.2s", temp[3], temp[4]);

            snprintf(GPS->vi_do, sizeof(GPS->vi_do),"%.10s %.2s", temp[5], temp[6]);
        }
    }
}

void* luong_gps(void* arg)
{
    int uart_fd = *(int*)arg;
    gps GPS;
    char buffer[256];

    while(1)
    {
        pthread_mutex_lock(&key);
        int now = trang_thai;
        pthread_mutex_unlock(&key);

        if(now == 0)
            break;

        memset(buffer, 0, sizeof(buffer));

        int byte_read = read(uart_fd, buffer, sizeof(buffer)-1);

        if(byte_read > 0)
        {
            toa_do(buffer, &GPS);

            if(GPS.state == 'A')
            {
                printf("Kinh do: %s\n", GPS.kinh_do);
                printf("Vi do: %s\n", GPS.vi_do);
                printf("-----------------\n");
            }
        }
    }

    return NULL;
}

int main()
{
    pthread_t chinh;
    const char *path = "/dev/ttyS3";

    int uart_fd = uart_init(path);
    if(uart_fd < 0)
        return -1;

    pthread_mutex_init(&key, NULL);

    pthread_create(&chinh, NULL, luong_gps, &uart_fd);

    printf("Chuong trinh dang chay... Nhan Enter de dung\n");
    getchar();

    pthread_mutex_lock(&key);
    trang_thai = 0;
    pthread_mutex_unlock(&key);

    pthread_join(chinh, NULL);

    pthread_mutex_destroy(&key);
    close(uart_fd);

    return 0;
}# Demo
# Demo
