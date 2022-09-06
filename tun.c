#include "tun.h"

struct Buffer 
{
    unsigned char data[1500];
    ssize_t len;
};
struct tunContext
{
    int fd;
    char   if_name[IFNAMSIZ];
    int pipefd[2];
    buffer buf;
    uv_poll_t poll_handle;
};


void exit_uv() {
    uv_stop(uv_default_loop());
    uv_loop_close(uv_default_loop());
}



void signal_handler(int sig)
{
   exit_uv();
}

void on_read(uv_poll_t* handle, int status, int events) {
     ctx t = (ctx)handle->data;

    while ((t->buf->len = read(t->fd, t->buf->data, 1500)) < (ssize_t) 0)
        ;
        
    printf("Recv: %ld\n", t->buf->len);
}



int tun_create(char if_name[IFNAMSIZ], const char *wanted_name)
{
    struct ifreq ifr;
    int          fd;
    int          err;

    fd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
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

    return fd;
}



int main(void) {
    ctx t = malloc(sizeof(struct tunContext));
    buffer b = malloc(sizeof(struct Buffer));
    t->fd = tun_create(t->if_name, "tun-0");
    if (t->fd < 0) {
        goto exit;
    }
    t->buf = b;
    if (pipe(t->pipefd) < 0) {
        goto exit;
    }
    uv_poll_init(uv_default_loop(), &t->poll_handle, t->fd);
    uv_poll_start(&t->poll_handle, UV_READABLE, on_read);
    t->poll_handle.data = (void *)t;
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
exit:
    printf("Exit");
    close(t->fd);
    if (t->pipefd[0] > 0 && t->pipefd[1] > 0) {
        close(t->pipefd[0]);
        close(t->pipefd[1]);
    }
    free(t);
    free(b);
    return 0;
}