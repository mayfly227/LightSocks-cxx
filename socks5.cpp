//
// Created by itsuy on 2020/5/4.
//

#include "socks5.h"
#include "server.h"
#include "cipher.h"
#include <event2/dns.h>

void socks5::handle_first(bufferevent *bev, Socks5ctx *context) {

    //socks5握手包
    unsigned char request[64] = {0};
    auto data_len = bufferevent_read(context->self, request, sizeof(request) - 1);
    Cipher::decrypt_bytes(request, data_len);
    //解析socks5请求
    if (request[0] == 0x05 && request[1] == 0x01 && request[2] == 0x00) {
        unsigned char success[] = {0x05, 0x00};
        Cipher::encrypt_byte(success, sizeof(success));
        bufferevent_write(context->self, success, sizeof(success));

        bufferevent_setcb(bev,handle_second, nullptr,server::local_event_cb, context);
    }else{
        return;
    }
}

void socks5::handle_second(bufferevent *bev, void *arg) {
    auto context = static_cast<Socks5ctx *>(arg);

    unsigned char connect[64] = {0};
    auto data_len = bufferevent_read(context->self, connect, sizeof(connect) - 1);
    //如果小于7
    if (data_len < 7) { return; }
    Cipher::decrypt_bytes(connect, data_len);
    //parse cmd
    if (connect[1] != 0x01) {
        spdlog::warn("only support connect!");
        return;
    }
    //parse ATYP
    switch (connect[3]) {
        //ipv4
        case 0x01: {
            //parse ip
            unsigned char ip2[32];
            char ip_dot[16];
            for (int i = 4; i < data_len - 2; i++) {
                ip2[i - 4] = connect[i];
            }
            const char *ip = evutil_inet_ntop(AF_INET, ip2, ip_dot, 16);
            context->request_ip = ip;
            //parse port
            spdlog::debug("ip is {}", context->request_ip);
            const int port = connect[data_len - 2] << 8 | connect[data_len - 1];
            context->request_port = port;
            unsigned char reply[7]{0x05, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00};
            Cipher::encrypt_byte(reply,7);
            bufferevent_write(bev, reply, 7);
            bufferevent_setcb(bev,handle_last, nullptr,server::local_event_cb, nullptr);
            break;
        }
            //parse domain
        case 0x03: {
            auto domain_size = (int) connect[4];
            std::string hostname;
            for (int i = 5; domain_size != 0; domain_size--) {
                hostname += (char) connect[i];
                i++;
            }
            //异步解析dns
            evutil_addrinfo addrInfo{};
            memset(&addrInfo, 0, sizeof(addrInfo));
            addrInfo.ai_family = AF_INET; /* for ipv4. */
            addrInfo.ai_socktype = SOCK_STREAM;
            addrInfo.ai_protocol = IPPROTO_TCP;
            /* Only return addresses we can use. */
            addrInfo.ai_flags = EVUTIL_AI_ADDRCONFIG;
            //解析端口
            const int port = connect[data_len - 2] << 8 | connect[data_len - 1];
            context->request_port = port;
            //异步解析ip
            spdlog::debug("parse domain{}", hostname.c_str());
            evdns_getaddrinfo(server::base_dns_loop, hostname.c_str(), nullptr, &addrInfo, server::dns_cb, context);
            bufferevent_setcb(bev,handle_last, nullptr,server::local_event_cb, context);
        }
            break;
        case 0x04:
            //ipv6
            spdlog::warn("not support ipv6!");
            break;
        default:
            spdlog::warn("error!");
            break;
    }
}

void socks5::handle_last(bufferevent *bev, void *arg) {
    auto context = static_cast<Socks5ctx *>(arg);

    //转发数据到远程服务器
    auto partner = context->partner;

    if (!partner) {
        //获取当前连接客户端的evbuffer
        evbuffer *local = bufferevent_get_input(context->self);
        auto len = evbuffer_get_length(local);
        //把数据从缓冲区移除
        evbuffer_drain(local, len);
        return;
    }
    Cipher::decrypt(context->self, context->partner);
}




