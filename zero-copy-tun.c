#include "zerocopy.h"

static int isExit = 0;

void signal_handler(int sig)
{
    isExit = 1;
}

int tun_create(char if_name[IFNAMSIZ], const char *wanted_name)
{
    struct ifreq ifr;
    int          fd;
    int          err;

    fd = open("/dev/net/tun", O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "tun module not present. See https://sk.tl/2RdReigK\n");
        return -1;
    }
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    snprintf(ifr.ifr_name, IFNAMSIZ, "%s", wanted_name == NULL ? "" : wanted_name);
    if (ioctl(fd, TUNSETIFF, &ifr) != 0) {
        err = errno;
        (void) close(fd);
        errno = err;
        return -1;
    }
    snprintf(if_name, IFNAMSIZ, "%s", ifr.ifr_name);
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return fd;
}
void do_poll(int fd, int pipefd[2]) {
    off64_t in = 0;
    ssize_t len = 0;
    while (!isExit)
    {
       if (len = splice(fd, &in, pipefd[1], NULL, 1500, SPLICE_F_MOVE | SPLICE_F_NONBLOCK) > (ssize_t) 0) 
       {
           printf("recv:%ld", len);
       }
    }

}
int main(void) {
    char if_name[IFNAMSIZ];
    int pipefd[2];
    int fd = tun_create(if_name, "tun-0");
    if (fd < 0) {
        goto exit;
    }
    if (pipe(pipefd) < 0) {
        goto exit;
    }


    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);

    do_poll(fd, pipefd);
exit:
    close(fd);
    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}