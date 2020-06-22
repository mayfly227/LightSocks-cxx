//
// Created by itsuye on 2020/5/4.
//

#ifndef LIGHTSOCKS_SOCKS5_H
#define LIGHTSOCKS_SOCKS5_H
#include <event2/bufferevent.h>
#include "server.h"

class socks5 {
public:
    static void handle_first(bufferevent *bev,Socks5ctx *context);
    static void handle_second(bufferevent *bev, void *arg);
    static void handle_last(bufferevent *bev, void *arg);

};


#endif //LIGHTSOCKS_SOCKS5_H
