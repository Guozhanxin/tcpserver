/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-07-11     flybreak     the first version
 */

#ifndef _TCPSERVER__
#define _TCPSERVER__

#include <rtthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>

/* configurable options */
#ifndef PKG_USING_TCPSERVER
#define TCPSERVER_NAME         "tcpserv"
#define TCPSERVER_STACK_SIZE   4096
#define TCPSERVER_PRIO         12
#define TCPSERVER_SOCKET_MAX   1024
#define TCPSERVER_CLI_NUM      5
#endif

/* run state */
#define TCPSERVER_STATE_INIT   0
#define TCPSERVER_STATE_RUN    1
#define TCPSERVER_STATE_STOP   2

/* event flag */
#define TCPSERVER_EVENT_CONNECT      (1 << 0)
#define TCPSERVER_EVENT_DISCONNECT   (1 << 1)
#define TCPSERVER_EVENT_RECV         (1 << 2)

typedef struct tcpclient *tcpclient_t;

struct tcpserver
{
    const char   *ip;
    rt_uint16_t  port;
    int          sock;
    rt_uint8_t   state;

    fd_set  read_set;
    fd_set  write_set;
    int     fd_max;

    rt_mailbox_t mailbox;
    rt_thread_t  thread;

    tcpclient_t *cli_list;

    void (*tcpserver_event_notify)(tcpclient_t client, rt_uint8_t event);
};

struct tcpclient
{
    struct tcpserver *server;
    int sock;

    rt_event_t event;
};

struct tcpserver *tcpserver_create(const char *ip, rt_uint16_t port);
rt_err_t tcpserver_destroy(struct tcpserver *server);

tcpclient_t tcpserver_accept(struct tcpserver *server, rt_int32_t timeout);
rt_err_t tcpserver_close(tcpclient_t client);

rt_size_t tcpserver_recv(tcpclient_t client, void *buf, rt_size_t size, rt_int32_t timeout);
rt_size_t tcpserver_send(tcpclient_t client, void *buf, rt_size_t size, rt_int32_t timeout); /* timeout unrealized */

void tcpserver_set_notify_callback(struct tcpserver *server,
                                   void (*tcpserver_event_notify)(tcpclient_t client, rt_uint8_t event));

#endif
