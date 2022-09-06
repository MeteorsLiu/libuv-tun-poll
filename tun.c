#include "tun.h"

struct Buffer 
{
    unsigned char data[1500];
    struct iovec vec;
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
    struct tunContext *t = (struct tunContext *)handle->data;
    // zero copy to pipe buffer
    off64_t in_off = 0;
    ssize_t len = 0;
    len = splice(t->fd, &in_off, t->pipefd[1], NULL, 1500, SPLICE_F_MOVE | SPLICE_F_MORE);
    
    if (len < 0) {
        printf("pipe error");
        exit_uv();
        return;
    }
    // reset the buffer
    memset(t->buf->vec.iov_base, 0, t->buf->vec.iov_len);
    // zero copy to user space buffer from pipe buffer
    vmsplice(t->pipefd[0], &t->buf->vec, len, SPLICE_F_MOVE | SPLICE_F_GIFT);
    printf("Recv: %d", t->buf->vec.iov_len);

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

    return fd;
}



int main(void) {
    struct tunContext t;
    struct Buffer b;
    b.vec.iov_len = 1500;
    b.vec.iov_base = b.data;
    t.fd = tun_create(t.if_name, "tun-0");
    if (t.fd < 0) {
        goto exit;
    }
    t.buf = &b;
    if (pipe(t.pipefd) < 0) {
        goto exit;
    }
    uv_poll_init(uv_default_loop(), &t.poll_handle, t.fd);
    uv_poll_start(&t.poll_handle, UV_READABLE, on_read);
    uv_default_loop()->data = (void *)&t;
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
exit:
    close(t.fd);
    if (t.pipefd[0] > 0 && t.pipefd[1] > 0) {
        close(t.pipefd[0]);
        close(t.pipefd[1]);
    }
    return 0;
}