#ifndef TUN_H
#define _GNU_SOURCE
#include <stdint.h>
#include <uv.h>
#include <signal.h>
#include <stdio.h>
#include <net/if.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

typedef struct Buffer * buffer;

void on_read(uv_poll_t* handle, int status, int events);

#endif
