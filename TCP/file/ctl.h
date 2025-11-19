#ifndef __CTL_H__
#define __CTL_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

extern void recv_n(int sock, void *buf, int len);
extern void send_n(int sock, void *buf, int len);
extern void handle_put(int client_fd);
extern void handle_get(int client_fd);
extern void do_put(int sock, char *filename);
extern void do_get(int sock, char *filename);
#endif
