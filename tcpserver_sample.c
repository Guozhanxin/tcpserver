/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-07-11     flybreak     the first version
 */

#include "tcpserver.h"
#include <string.h>

#define DBG_TAG   "tcpserv"
#define DBG_LVL   DBG_LOG
#include <rtdbg.h>

static struct tcpserver *serv;
static rt_uint8_t buf[1024];

static void tcpserver_event_notify(tcpclient_t client, rt_uint8_t event)
{
    int ret;
    switch (event)
    {
    case TCPSERVER_EVENT_CONNECT:
        LOG_D("client connect:%d", client->sock);
        break;
    case TCPSERVER_EVENT_RECV:
        ret = tcpserver_recv(client, buf, 1024, -1);
        if (ret > 0)
        {
            ret = tcpserver_send(client, buf, ret, 0);
        }
        break;
    case TCPSERVER_EVENT_DISCONNECT:
        LOG_D("client disconnect:%d", client->sock);
        break;
    default:
        break;
    }
}

static int tcpserver(int argc, char **argv)
{
    if (argc != 3)
    {
        rt_kprintf("Usage:./select_server [ip] [port]\n");
        return -1;
    }
    serv = tcpserver_create(argv[1], atoi(argv[2]));

    tcpserver_set_notify_callback(serv, tcpserver_event_notify);

    return 0;
}
MSH_CMD_EXPORT(tcpserver, server start)

static int tcpserver_stop(int argc, char **argv)
{
    tcpserver_destroy(serv);
    return 0;
}
MSH_CMD_EXPORT(tcpserver_stop, tcpserver stop)
