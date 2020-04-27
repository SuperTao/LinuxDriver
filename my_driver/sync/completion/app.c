#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    int fd, ret;
    char buf[1024];

    fd = open(argv[1], O_RDWR);
    if(-1 == fd)
    {
        perror("open");
        exit(1);
    }

    ret = read(fd, buf, sizeof(buf));
    if(-1 == ret)
        perror("read");

    close(fd);
    return 0;
}
