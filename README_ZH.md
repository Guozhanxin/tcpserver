# TCP server

中文页 | [English](README.md)

## 简介

本软件包是一个支持多客户端连接通讯的 TCP 服务器，使用简单的 API 就可以实现服务器的创建，并与不同的客户端通讯。

本服务器有以下特点：

- 多服务器同时运行
- 多网卡支持，支持绑定到特定的 IP
- 支持阻塞接收和非阻塞的通知回调接收。
- 支持与不同的客户端独立的交换数据。

## 使用说明

### 依赖

- RT-Thread 3.1.0+
- SAL
- DFS (select)

### 获取软件包

使用 tcpserver 软件包需要在 RT-Thread 的包管理中选中它，具体路径如下：

```
RT-Thread online packages --->
    IoT - internet of things --->
        [*] TCP Server:A TCP server that supports multiple clients
            (tcpserv) tcpserver name
            (4096) tcpserver stack size
            (12)  tcpserver thread priority
            (512) Maximum possible socket usage
            (5)   Number of clients supported
            [ ]   Enable debugging features
            [*]   Enable sample
                Version (latest)  --->
```

- **tcpserver name**： 配置 tcpserver 的名称
- **tcpserver stack size**：配置线程栈的大小
- **tcpserver thread priority**：配置线程的优先级
- **Maximum possible socket usage**：系统可能用到的 socket 的最大值
- **Number of clients supported**：支持同时连接的客户端数目
- **Enable debugging features**：开启调试功能
- **Enable sample**：开启示例代码
- **Version**：软件包版本选择

### 使用软件包

#### API 详解

##### 创建服务端

创建一个 TCP 的服务端，传入服务端的 ip 地址和端口号。

```
struct tcpserver *tcpserver_create(const char *ip, rt_uint16_t port);
```

**函数参数**

| 参数     | 描述                              |
| -------- | --------------------------------- |
| ip       | 要绑定的 ip 地址                  |
| port     | 要绑定的 端口号                   |
| **返回** | **——**                            |
| >  0     | 成功，返回一个 tcp 服务端的句柄。 |
| = 0      | 失败                              |

##### 销毁服务端

销毁一个 TCP 的服务端，并回收资源。

```
rt_err_t tcpserver_destroy(struct tcpserver *server);
```

**函数参数**

| 参数     | 描述                      |
| -------- | ------------------------- |
| server   | 要销毁的 tcp 服务端的句柄 |
| **返回** | **——**                    |
| =  0     | 成功                      |
| < 0      | 失败                      |

##### 获取客户端

获取一个 TCP 的客户端，阻塞式的获取一个客户端。

```
tcpclient_t tcpserver_accept(struct tcpserver *server, rt_int32_t timeout);
```

**函数参数**

| 参数     | 描述                                              |
| -------- | ------------------------------------------------- |
| server   | tcp 服务端的句柄                                  |
| timeout  | 等待的超时时间，RT_WAITING_FOREVER 表示永久等待。 |
| **返回** | **——**                                            |
| >  0     | 成功，返回一个 tcp 客户端的句柄。                 |
| = 0      | 失败                                              |

##### 关闭客户端

关闭一个 TCP 的客户端。

```
rt_err_t tcpserver_close(tcpclient_t client);
```

**函数参数**

| 参数     | 描述             |
| -------- | ---------------- |
| client   | tcp 客户端的句柄 |
| **返回** | **——**           |
| =  0     | 成功。           |
| < 0      | 失败             |

##### 从客户端接收数据

从客户端接收数据。

```
rt_size_t tcpserver_recv(tcpclient_t client, void *buf, rt_size_t size, rt_int32_t timeout);
```

**函数参数**

| 参数     | 描述                                              |
| -------- | ------------------------------------------------- |
| client   | tcp 客户端的句柄                                  |
| buf      | 缓冲区地址                                        |
| size     | 缓冲区大小                                        |
| timeout  | 等待的超时时间，RT_WAITING_FOREVER 表示永久等待。 |
| **返回** | **——**                                            |
| >  0     | 成功，返回接收到的数据长度。                      |
| = 0      | 失败                                              |

##### 向客户端发送数据

向客户端发送数据。

```
rt_size_t tcpserver_send(tcpclient_t client, void *buf, rt_size_t size, rt_int32_t timeout);
```

**函数参数**

| 参数     | 描述                         |
| -------- | ---------------------------- |
| client   | tcp 客户端的句柄             |
| buf      | 缓冲区地址                   |
| size     | 缓冲区大小                   |
| timeout  | 等待的超时时间，**暂未实现** |
| **返回** | **——**                       |
| >  0     | 成功，已发送的数据长度。     |
| = 0      | 失败                         |

##### 设定事件通知回调函数

设定服务器的事件通知回调函数。

```
void tcpserver_set_notify_callback(struct tcpserver *server,
                                   void (*tcpserver_event_notify)(tcpclient_t client, rt_uint8_t event));
```

**函数参数**

| 参数                   | 描述             |
| ---------------------- | ---------------- |
| server                 | tcp 服务端的句柄 |
| tcpserver_event_notify | 要设定的函数指针 |
| **返回**               | **——**           |

#### 运行示例程序

软件包自带一个回显服务器的示例程序。开启软件包的示例程序之后，就可以使用。

编译下载运行，会出现两个命令，`tcpserver`、`tcpserver_stop`。分别是创建 TCP server 和 关闭 TCP server。

先输入 IP 地址和端口号 创建一个服务器，然后，就可以使用网络调试助手和此服务器通信了。输入`tcpserver_stop`关闭服务器。

```
 \ | /
- RT -     Thread Operating System
 / | \     3.1.3 build Jul 11 2019
 2006 - 2019 Copyright by rt-thread team
lwIP-2.0.2 initialized!
[4] I/sal.skt: Socket Abstraction Layer initialize success.
msh />ifconfig
network interface device: e0 (Default)
MTU: 1500
MAC: 00 04 a3 12 34 56
FLAGS: UP LINK_UP INTERNET_UP DHCP_ENABLE ETHARP BROADCAST IGMP
ip address: 192.168.12.117            # 本机 IP 地址
gw address: 192.168.10.1
net mask  : 255.255.0.0
dns server #0: 192.168.10.1
dns server #1: 223.5.5.5
msh />tcpserver 192.168.12.117 5000   # 创建服务器
msh />[173861] D/tcpserv: client connect:5  # 客户端连接成功
[180220] D/tcpserv: client disconnect:5     # 客户端断开连接
msh />tcpserver_stop                  # 关闭服务器
msh />
```

## 注意事项

- tcpserver_send 函数的超时机制还没有实现
- 如果还没有到达设定的支持客户端的数目，但是总连接出错，可能是 lwip 的限制。

## 联系人信息

- 维护人: [flybreak](guozhanxin@rt-thread.com)

- 主页：[tcpserver](<https://github.com/Guozhanxin/tcpserver>)