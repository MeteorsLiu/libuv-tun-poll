#ifndef TUN_H

#include <stdint.h>
#include <uv.h>
#include <signal.h>
#include <stdio.h>
#include <net/if.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include <stdlib.h>
#define _GNU_SOURCE 
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <string.h>

#include <unistd.h>

typedef struct Buffer * buffer;

void on_read(uv_poll_t* handle, int status, int events);

#endif
