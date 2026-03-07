#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int uart_init(const char *uart_path)
{
    int fd = open(fd, uart_path, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) 
    {
        perror("Loi khong the mo cong UART");
        return -1;
    }
    fctnl(fd, F_SETFL, 0);
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0)
    {
        perror("Loi khong the lay cau hinh termios");
        close(fd);
        return -1;
    }
    cfmakeraw(&tty);

    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOP8;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    //terminal control set attribute
    {
        perror("Loi: Khong the luu cau hinh terminos");
        close(fd);
        return -1;
    }
    return fd;
}

int main()
{
    const char *uart_path = "/dev/ttyS3";
    int uart_fd = uart_init(uart_path);
    printf("Da mo thanh cong %s voi toc do B9600\n", uart_path);
    char buffer[256];
    while(1)
    {
        memset(buffer,'\0', sizeof(buffer));
        int bytes_read = read(uart_fd, rx_buffer, sizeof(rx_buffer) - 1);

        if (bytes_read > 0) 
        {
            printf("[Nhan duoc %d bytes]: %s\n", bytes_read, rx_buffer);
        } 
        else if (bytes_read == 0) 
        {
            // Nhờ có VTIME=10 và VMIN=0, khi không có dữ liệu, nó sẽ rớt xuống đây 
            // sau 1 giây thay vì treo cứng vĩnh viễn!
            printf("[Timeout] Khong co du lieu... dang lang nghe tiep.\n");
        } 
        else 
        {
            perror("Loi khi doc UART");
            break;
        }
    }

    close(uart_fd);
    return 0;
}
