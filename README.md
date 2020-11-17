# TCP server

[中文页](README_ZH.md) | English

## Introduction

This software package is a TCP server that supports multi-client connection and communication. Using a simple API, you can create a server and communicate with different clients.

This server has the following characteristics:

- Simultaneous operation of multiple servers
- Multiple network card support, support binding to specific IP
- Support blocking reception and non-blocking notification callback reception.
- Support independent exchange of data with different clients.

## Instructions for use

### Dependence

- RT-Thread 3.1.0+
- SAL
- DFS (select)

### Get the package

To use the tcpserver software package, you need to select it in the RT-Thread package management. The specific path is as follows:

```
RT-Thread online packages --->
    IoT-internet of things --->
        [*] TCP Server:A TCP server that supports multiple clients
            (tcpserv) tcpserver name
            (4096) tcpserver stack size
            (12) tcpserver thread priority
            (512) Maximum possible socket usage
            (5) Number of clients supported
            [] Enable debugging features
            [*] Enable sample
                Version (latest) --->
```

- **tcpserver name**: Configure the name of tcpserver
- **tcpserver stack size**: Configure the size of the thread stack
- **tcpserver thread priority**: Configure thread priority
- **Maximum possible socket usage**: the maximum value of sockets that the system may use
- **Number of clients supported**: The number of clients that support simultaneous connections
- **Enable debugging features**: Enable debugging features
- **Enable sample**: Enable sample code
- **Version**: software package version selection

### Using packages

#### API details

##### Create server

Create a TCP server and pass in the ip address and port number of the server.

```
struct tcpserver *tcpserver_create(const char *ip, rt_uint16_t port);
```

**Function Parameters**

| Parameters | Description |
| -------- | --------------------------------- |
| ip | ip address to bind |
| port | Port number to bind |
| **Back** | **——** |
|> 0 | Success, return a tcp server handle. |
| = 0 | Failure |

##### Destroy the server

Destroy a TCP server and reclaim resources.

```
rt_err_t tcpserver_destroy(struct tcpserver *server);
```

**Function Parameters**

| Parameters | Description |
| -------- | ------------------------- |
| server | The handle of the tcp server to be destroyed |
| **Back** | **——** |
| = 0 | Success |
| <0 | Failure |

##### Get the client

Obtain a TCP client, and block a client.

```
tcpclient_t tcpserver_accept(struct tcpserver *server, rt_int32_t timeout);
```

**Function Parameters**

| Parameters | Description |
| -------- | ------------------------------------------------ |
| server | tcp server handle |
| timeout | Waiting timeout time, RT_WAITING_FOREVER means waiting forever. |
| **Return** | **——** |
|> 0 | Success, return a tcp client handle. |
| = 0 | Failure |

##### Close the client

Close a TCP client.

```
rt_err_t tcpserver_close(tcpclient_t client);
```

**Function Parameters**

| Parameters | Description |
| -------- | ---------------- |
| client | tcp client handle |
| **Back** | **——** |
| = 0 | Success. |
| <0 | Failure |

##### Receive data from the client

Receive data from the client.

```
rt_size_t tcpserver_recv(tcpclient_t client, void *buf, rt_size_t size, rt_int32_t timeout);
```

**Function Parameters**

| Parameters | Description |
| -------- | ------------------------------------------------ |
| client | tcp client handle |
| buf | buffer address |
| size | Buffer size |
| timeout | Waiting timeout time, RT_WAITING_FOREVER means waiting forever. |
| **Back** | **——** |
|> 0 | Success, return the received data length. |
| = 0 | Failure |

##### Send data to the client

Send data to the client.

```
rt_size_t tcpserver_send(tcpclient_t client, void *buf, rt_size_t size, rt_int32_t timeout);
```

**Function Parameters**

| Parameters | Description |
| -------- | ---------------------------- |
| client | tcp client handle |
| buf | buffer address |
| size | Buffer size |
| timeout | Waiting timeout time, **not yet implemented** |
| **Return** | **——** |
|> 0 | Success, the length of the data sent. |
| = 0 | Failure |

##### Set event notification callback function

Set the event notification callback function of the server.

```
void tcpserver_set_notify_callback(struct tcpserver *server,
                                   void (*tcpserver_event_notify)(tcpclient_t client, rt_uint8_t event));
```

**Function Parameters**

| Parameters | Description |
| ---------------------- | ---------------- |
| server | tcp server handle |
| tcpserver_event_notify | Function pointer to be set |
| **Back** | **——** |

#### Run the sample program

The software package comes with a sample program for the echo server. After opening the sample program of the software package, you can use it.

Compile, download and run, there will be two commands, `tcpserver` and `tcpserver_stop`. They are creating TCP server and closing TCP server.

First enter the IP address and port number to create a server, and then you can use the network debugging assistant to communicate with this server. Enter `tcpserver_stop` to shut down the server.

```
 \ | /
-RT-Thread Operating System
 / | \ 3.1.3 build Jul 11 ​​2019
 2006-2019 Copyright by rt-thread team
lwIP-2.0.2 initialized!
[4] I/sal.skt: Socket Abstraction Layer initialize success.
msh />ifconfig
network interface device: e0 (Default)
MTU: 1500
MAC: 00 04 a3 12 34 56
FLAGS: UP LINK_UP INTERNET_UP DHCP_ENABLE ETHARP BROADCAST IGMP
ip address: 192.168.12.117 # Local IP address
gw address: 192.168.10.1
net mask: 255.255.0.0
dns server #0: 192.168.10.1
dns server #1: 223.5.5.5
msh />tcpserver 192.168.12.117 5000 # Create server
msh />[173861] D/tcpserv: client connect:5 # The client connects successfully
[180220] D/tcpserv: client disconnect:5 # The client disconnects
msh />tcpserver_stop # Shut down the server
msh />
```

## Precautions

- The timeout mechanism of the tcpserver_send function has not been implemented yet
- If the number of supported clients has not been reached, but the total connection is wrong, it may be limited by lwip.

## contact information

- Maintainer: [flybreak](guozhanxin@rt-thread.com)
- Homepage: [tcpserver](<https://github.com/Guozhanxin/tcpserver>)
