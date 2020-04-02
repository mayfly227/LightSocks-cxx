//
// Created by Li on 2020/4/1.
//

#ifndef SBPROXY_CLIENT_H
#define SBPROXY_CLIENT_H

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

#include <signal.h>

#endif

#include "spdlog/spdlog.h"

using std::string;

class client {
public:

    static void setConfig(const char *ip, unsigned short local_port, unsigned short remote_port, string password);

public:
    static void local_event_cb(bufferevent *bev, short what, void *ctx);

    static void local_read_cb(bufferevent *bev, void *arg);

//    void local_write_cb(bufferevent *bev, void *arg);
    static void remote_event_cb(bufferevent *bev, short what, void *ctx);

    static void remote_read_cb(bufferevent *bev, void *arg);

//    void remote_write_cb(bufferevent *bev, void *arg);
    static void close_on_finished_write_cb(struct bufferevent *bev, void *ctx);

    static void listen_cb(evconnlistener *ev, evutil_socket_t s, sockaddr *sin, int sin_len, void *arg);

    static void init();

    static void clear();


public:
    static event_base *base_loop;
    static const char *ip;
    static unsigned short local_port;
    static unsigned short remote_port;
    static unsigned char password[256];

};


#endif //SBPROXY_CLIENT_H
