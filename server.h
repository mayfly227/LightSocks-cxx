//
// Created by itsuy on 2020/4/4.
//

#ifndef LIGHTSOCKS_SERVER_H
#define LIGHTSOCKS_SERVER_H

#include <iostream>
#include "lib/base64/base64.h"

#ifdef _WIN32

#include "event2/event.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "event2/listener.h"

#else

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>

#endif

#ifdef _WIN32

#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
#else

#include <csignal>

#endif

#include "spdlog/spdlog.h"

using std::string;

class server {
public:

//    static void setConfig(unsigned short listen_port, string password);

public:
    static void local_event_cb(bufferevent *bev, short what, void *ctx);

    static void local_read_cb(bufferevent *bev, void *arg);

//    static void local_write_cb(bufferevent *bev, void *arg);
    static void remote_event_cb(bufferevent *bev, short what, void *ctx);

    static void remote_read_cb(bufferevent *bev, void *arg);

//    static void remote_write_cb(bufferevent *bev, void *arg);

//    static void close_on_finished_write_cb(struct bufferevent *bev, void *ctx);

    static void listen_cb(evconnlistener *ev, evutil_socket_t s, sockaddr *sin, int sin_len, void *arg);

    static void dns_cb(int errcode,evutil_addrinfo *addr,void *ctx);
    static void run();

    static void clear();


public:
    static event_base *base_loop;
    static evdns_base *base_dns_loop;
    static unsigned short listen_port;

};


#endif //LIGHTSOCKS_SERVER_H
