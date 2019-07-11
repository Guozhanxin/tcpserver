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

#define DBG_TAG    "tcpserv"
#ifdef  PKG_TCPSERVER_DEBUG
#define DBG_LVL    DBG_LOG
#else
#define DBG_LVL    DBG_INFO
#endif
#include <rtdbg.h>

static tcpclient_t tcpserver_add_cli(struct tcpserver *serv, int fd_cli)
{
    tcpclient_t client = RT_NULL;
    unsigned long ul = 1;
    int ret;
    RT_ASSERT(serv);

    LOG_D("client %d id connected", fd_cli);

    client = (tcpclient_t)rt_calloc(1, sizeof(struct tcpclient));
    if (client == RT_NULL)
    {
        LOG_E("client calloc failed!");
        goto __exit;
    }

    client->server = serv;
    client->sock = fd_cli;
    client->event = rt_event_create(TCPSERVER_NAME, RT_IPC_FLAG_FIFO);
    if (client->event == RT_NULL)
    {
        LOG_E("client event create failed!");
        goto __exit;
    }

    /* set socket to be non-blocking */
    ret = ioctlsocket(fd_cli, FIONBIO, (unsigned long *)&ul);
    if (ret < 0)
    {
        LOG_E("set socket non-blocking failed");
    }

    /* add new client to cli_list */
    serv->cli_list[fd_cli] = client;

    /* update fd_max */
    FD_SET(fd_cli, &serv->read_set);
    if (fd_cli > serv->fd_max)
    {
        serv->fd_max = fd_cli;
    }

    /* notify new client */
    rt_mb_send(serv->mailbox, (rt_uint32_t)client);
    if (serv->tcpserver_event_notify)
    {
        serv->tcpserver_event_notify(client, TCPSERVER_EVENT_CONNECT);
    }
    return client;

__exit:
    if (client->event)
        rt_event_delete(client->event);
    if (client)
        rt_free(client);

    return RT_NULL;
}

static void tcpserver_del_cli(tcpclient_t client)
{
    int max = -1;
    int i = 0;
    struct tcpserver *serv = client->server;

    RT_ASSERT(client);

    /* remove client from read_set */
    FD_CLR(client->sock, &serv->read_set);

    /* update fd_max */
    for (i = 0; i <= serv->fd_max; ++i)
    {
        if (!FD_ISSET(i, &serv->read_set))
        {
            continue;
        }
        if (max < i)
        {
            max = i;
        }
    }
    serv->fd_max = max;

    /* close client socket */
    closesocket(client->sock);

    /* remove client from client_list */
    serv->cli_list[client->sock] = RT_NULL;

    /* notify disconnect */
    if (serv->tcpserver_event_notify)
    {
        serv->tcpserver_event_notify(client, TCPSERVER_EVENT_DISCONNECT);
    }

    /* free memory */
    rt_event_delete(client->event);
    rt_free(client);
    LOG_D("socket:%d,closed", client->sock);
}

static void tcpserver_thread_entry(void *parameter)
{
    struct tcpserver *server = parameter;
    fd_set  read_set, write_set;
    int ret_sel, ret;
    unsigned long ul = 1;
    struct timeval time = {0};

    server->state = TCPSERVER_STATE_RUN;
    server->fd_max = server->sock;
    time.tv_sec =  1;
    time.tv_usec = 0;

    /* set socket to be non-blocking */
    ret = ioctlsocket(server->sock, FIONBIO, (unsigned long *)&ul);
    if (ret < 0)
    {
        LOG_E("set socket ctl failed");
    }

    FD_ZERO(&server->read_set);
    FD_ZERO(&server->write_set);
    FD_SET(server->sock, &server->read_set);

    while (1)
    {
        read_set = server->read_set;
        write_set = server->write_set;

        ret_sel = select(server->fd_max + 1, &read_set, &write_set, NULL, (void *)&time);

        /* detection stop mark */
        if (server->state == TCPSERVER_STATE_STOP)
        {
            LOG_D("server thread exit.");
            return;
        }
        /* select error or timeout */
        if (ret_sel <= 0)
        {
            continue;
        }

        if (FD_ISSET(server->sock, &read_set))
        {
            /* accept new client */
            struct sockaddr_in peer;
            int fd_cli_new;
            socklen_t len;

            len = sizeof(peer);

            fd_cli_new = accept(server->sock, (struct sockaddr *)&peer, &len);
            if (fd_cli_new < 0)
            {
                LOG_E("accept error");
                continue;
            }
            /* add client */
            tcpserver_add_cli(server, fd_cli_new);
        }
        else    /*  */
        {
            /* received data */
            int i = 0;

            for (i = 0; i <= server->fd_max; i++)
            {
                if (!FD_ISSET(i, &read_set))
                {
                    continue;
                }
                if (server->cli_list[i])
                {
                    rt_event_send(server->cli_list[i]->event, TCPSERVER_EVENT_RECV);
                    if (server->tcpserver_event_notify)
                    {
                        server->tcpserver_event_notify(server->cli_list[i], TCPSERVER_EVENT_RECV);
                    }
                }
            }
        }
    }
}

rt_size_t tcpserver_send(tcpclient_t client, void *buf, rt_size_t size, rt_int32_t timeout)
{
    return send(client->sock, buf, size, 0);
}

rt_size_t tcpserver_recv(tcpclient_t client, void *buf, rt_size_t size, rt_int32_t timeout)
{
    rt_uint32_t e;
    rt_int32_t recv_size;
    RT_ASSERT(client);
    RT_ASSERT(buf != RT_NULL);

    if (rt_event_recv(client->event, TCPSERVER_EVENT_RECV,
                      RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                      timeout, &e) == RT_EOK)
    {
        recv_size = recv(client->sock, buf, size, 0);

        if (recv_size > 0)
        {
            return recv_size;
        }
        else if (recv_size == 0)
        {
            LOG_D("recv 0");
            tcpserver_del_cli(client);
        }
        else if (recv_size < 0)
        {
            if (!(recv_size == EINTR || recv_size == EWOULDBLOCK || recv_size == EAGAIN))
            {
                LOG_E("recv error");
                tcpserver_del_cli(client);
            }
        }
    }
    return 0;
}

tcpclient_t tcpserver_accept(struct tcpserver *server, rt_int32_t timeout)
{
    tcpclient_t cli;

    RT_ASSERT(server != RT_NULL);
    if (rt_mb_recv(server->mailbox, (rt_ubase_t *)&cli, timeout) == RT_EOK)
    {
        return cli;
    }
    else
    {
        return RT_NULL;
    }
}

rt_err_t tcpserver_close(tcpclient_t client)
{
    tcpserver_del_cli(client);
    return RT_EOK;
}

void tcpserver_set_notify_callback(struct tcpserver *server, void (*tcpserver_event_notify)(tcpclient_t client, rt_uint8_t event))
{
    server->tcpserver_event_notify = tcpserver_event_notify;
}

static rt_err_t tcpserver_start(struct tcpserver *server)
{
    struct sockaddr_in addr;
    int ret_bind, ret_listen;
    int opt = 1;

    server->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server->sock < 0)
    {
        LOG_E("socket create failed");
        return -1;
    }

    if (server->ip)
        addr.sin_addr.s_addr = inet_addr(server->ip);
    else
        addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server->port);

    /* set server socket port multiplexing */
    setsockopt(server->sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));

    ret_bind = bind(server->sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret_bind < 0)
    {
        LOG_E("bind failed");
        goto __exit;
    }

    ret_listen = listen(server->sock, TCPSERVER_CLI_NUM);
    if (ret_listen < 0)
    {
        LOG_E("listen failed");
        goto __exit;
    }

    server->thread = rt_thread_create(TCPSERVER_NAME,
                                      tcpserver_thread_entry, server,
                                      TCPSERVER_STACK_SIZE, TCPSERVER_PRIO, 10);

    if (server->thread != NULL)
        rt_thread_startup(server->thread);
    else
    {
        LOG_E("thread create failed");
        goto __exit;
    }
    return RT_EOK;
__exit:
    if (server->sock)
        closesocket(server->sock);
    return -1;
}

struct tcpserver *tcpserver_create(const char *ip, rt_uint16_t port)
{
    struct tcpserver *server = RT_NULL;

    server = (struct tcpserver *)rt_calloc(1, sizeof(struct tcpserver));
    if (server == RT_NULL)
    {
        LOG_E("no memory for tcp server");
        return RT_NULL;
    }
    server->mailbox = rt_mb_create(TCPSERVER_NAME, TCPSERVER_CLI_NUM, RT_IPC_FLAG_FIFO);
    if (server->mailbox == RT_NULL)
    {
        LOG_E("no memory for mailbox");
        goto __exit;
    }
    server->cli_list = (tcpclient_t *)rt_calloc(1, sizeof(tcpclient_t) * TCPSERVER_SOCKET_MAX);
    if (server->cli_list == RT_NULL)
    {
        LOG_E("no memory for cli_list");
        goto __exit;
    }

    server->ip = ip;
    server->port = port;

    if (tcpserver_start(server) != RT_EOK)
    {
        LOG_E("tcp server start failed");
        goto __exit;
    }

    return server;

__exit:
    LOG_E("error!");
    if (server->mailbox)
        rt_mb_delete(server->mailbox);
    if (server->cli_list)
        rt_free(server->cli_list);
    if (server)
        rt_free(server);
    return RT_NULL;
}

rt_err_t tcpserver_destroy(struct tcpserver *server)
{
    int i;

    /* wait for the select thread to exit */
    server->state = TCPSERVER_STATE_STOP;
    while (server->thread->stat != RT_THREAD_CLOSE)
    {
        rt_thread_mdelay(100);
    }

    /* close all clients */
    for (i = 0; i <= server->fd_max; i++)
    {
        if (!FD_ISSET(i, &server->read_set))
        {
            continue;
        }
        if (i != server->sock)
        {
            tcpserver_close(server->cli_list[i]);
        }
    }

    /* close server socket */
    closesocket(server->sock);

    /* free memory */
    if (server->mailbox)
        rt_mb_delete(server->mailbox);
    if (server->cli_list)
        rt_free(server->cli_list);
    if (server)
        rt_free(server);

    return RT_EOK;
}
