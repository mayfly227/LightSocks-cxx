﻿#include <event2/util.h>
#include <array>
#include <event2/dns.h>

#include "server.h"
#include "cipher.h"

#define ENCRYPT

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif
struct Socks5ctx {
    int flag{};
    bufferevent *self{}; //这个self是客户端
    bufferevent *partner{}; //这个是远程客户端
    int request_port{};
    std::string request_ip;
};


void server::local_event_cb(bufferevent *bev, short what, void *ctx) {

    spdlog::debug("本地event_cb(what)-->: {0:x} ", what);
    auto context = static_cast<Socks5ctx *>(ctx);
    if (!context) {
        spdlog::warn("没有context了local_event...");
        return;
    }
    //读到文件结尾或者发生错误
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        spdlog::debug("进入本地逻辑");
        if (what & BEV_EVENT_ERROR) {
            spdlog::debug("本地操作时发生错误");
        }
        if (context->partner) {
            bufferevent_free(context->partner);
        }
        bufferevent_free(context->self);
        delete context;
        context = nullptr;
    }

}

void server::local_read_cb(bufferevent *bev, void *arg) {
    auto context = static_cast<Socks5ctx *>(arg);
    if (!context) {
        spdlog::warn("没有context了local_read_cb...");
        return;
    }
    if (context->flag == 0) {
        //socks5握手包
        unsigned char request[64] = {0};
        auto data_len = bufferevent_read(context->self, request, sizeof(request) - 1);
#ifdef ENCRYPT
        Cipher::decrypt_bytes(request, data_len);
#endif
        //解析socks5请求
        if (request[0] == 0x05 && request[1] == 0x01 && request[2] == 0x00) {
            context->flag = 1;
            unsigned char success[] = {0x05, 0x00};
#ifdef ENCRYPT
            Cipher::encrypt_byte(success, sizeof(success));
#endif
            bufferevent_write(context->self, success, sizeof(success));
        }
    } else if (context->flag == 1) {
        //读取建立连接的信息
        unsigned char connect[64] = {0};
        auto data_len = bufferevent_read(context->self, connect, sizeof(connect) - 1);
        //如果小于7
        if (data_len < 7) { return; }
#ifdef ENCRYPT
        Cipher::decrypt_bytes(connect, data_len);
#endif
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
#ifdef ENCRYPT
                unsigned char reply_encrypt[7] = {0};
                for (int k = 0; k < 7; k++) {
                    reply_encrypt[k] = reply[k];
                }
                for (int i = 0; i < 7; i++) {
                    reply[i] = Cipher::password[reply_encrypt[i]];
                }
#endif
                context->flag = 2;
                bufferevent_write(bev, reply, 7);
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
                spdlog::debug("正在解析域名{}", hostname.c_str());
                evdns_getaddrinfo(base_dns_loop, hostname.c_str(), nullptr, &addrInfo, dns_cb, context);
            }
                break;
            case 0x04:
                //ipv6
                spdlog::warn("暂时不支持ipv6!");
                break;
            default:
                spdlog::warn("error!");
                break;
        }
        //socks5握手完成,开始转发真正的数据
    } else if (context->flag == 2) {
        //转发数据到远程服务器
        auto partner = context->partner;

        if (!partner) {
            //获取当前连接客户端的evbuffer
            evbuffer *local = bufferevent_get_input(context->self);
            auto len = evbuffer_get_length(local);
            //把数据从缓冲区移除
            spdlog::debug("local drain 调用");
            evbuffer_drain(local, len);
            return;
        }
        // TODO 解密数据返回到服务端
#ifdef ENCRYPT
        Cipher::decrypt(context->self, context->partner);
#else
        evbuffer *dst,*src;
        src = bufferevent_get_input(context->self);
        dst = bufferevent_get_output(context->partner);
        evbuffer_add_buffer(dst, src); //这个是复制了一份
#endif
    }

}

void server::remote_event_cb(bufferevent *bev, short what, void *ctx) {
    //bug fixed
    spdlog::debug("远程event_cb(what)-->: {0:x} ", what);
    auto context = static_cast<Socks5ctx *>(ctx);
    if (!context) {
        spdlog::warn("没有context了 remote_event...");
        return;
    }
    if (what & BEV_EVENT_CONNECTED) {
        spdlog::debug("远程服务端tcp握手成功...");
        context->flag = 2;
        // TODO bug fixed
        auto fd = bufferevent_getfd(context->self);
        if (fd > 0) {
            unsigned char reply[7] = {0x05, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00};
#ifdef ENCRYPT
            Cipher::encrypt_byte(reply, sizeof(reply));
#endif
            //返回握手包
            bufferevent_write(context->self, reply, sizeof(reply));
        } else {
            bufferevent_free(context->self);
            bufferevent_free(context->partner);
            delete context;
            context = nullptr;
            return;
        }
        return;
    }
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        spdlog::debug("进入远程逻辑...");
        if (what & BEV_EVENT_ERROR) {
            spdlog::debug("服务端发生错误...");
        }
        if (context->self) {
            bufferevent_free(context->self);
        }
        bufferevent_free(context->partner);
        delete context;
        context = nullptr;
    } else {
        if (what & BEV_EVENT_TIMEOUT && what & BEV_EVENT_READING) {
            spdlog::debug("服务端读取数据端超时...");
            if (context->self) {
                bufferevent_free(context->self);
            }
            bufferevent_free(context->partner);
            delete context;
            context = nullptr;
        }
    }
}

void server::remote_read_cb(bufferevent *bev, void *arg) {
    auto context = static_cast<Socks5ctx *>(arg);
    if (!context) {
        spdlog::warn("没有context了 remote_read_cb...");
        return;
    }
    if (!context->self) {
        evbuffer *remote;
        //获取服务端的evbuffer
        remote = bufferevent_get_input(context->partner);
        auto len = evbuffer_get_length(remote);
        //把数据从缓冲区移除
        evbuffer_drain(remote, len);
    }
#ifdef ENCRYPT
    // TODO need bug fixed
    Cipher::encrypt(context->self, context->partner);
#else
    evbuffer *src,*dst;
    src = bufferevent_get_input(context->partner);
    dst = bufferevent_get_output(context->self);
    evbuffer_add_buffer(dst, src);
#endif
}

void server::listen_cb(evconnlistener *lev, evutil_socket_t s, sockaddr *sin, int sin_len, void *arg) {
//    spdlog::info("客户端有新的连接...");
    char ip[128]{0};
    auto r_sa = (sockaddr_in *) sin;
    evutil_inet_ntop(r_sa->sin_family, &(r_sa->sin_addr.s_addr), ip, sin_len);
    spdlog::info("New connection from {}:{}", ip, r_sa->sin_port);

    auto base = evconnlistener_get_base(lev);
    //监听本地客户端,用本地客户端连接的socket
    auto bufev_local = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);
    //本地客户端超时为10s
    timeval local_time_out = {15, 0};
    bufferevent_set_timeouts(bufev_local, &local_time_out, 0);
    //----------------------------------------------------------------------//
    //设置上下文
    auto socks5Ctx = new Socks5ctx();
    //初始化参数
    socks5Ctx->self = bufev_local;
    socks5Ctx->flag = 0;
    //设置回调和事件
    bufferevent_setcb(bufev_local, local_read_cb, nullptr, local_event_cb, socks5Ctx);
    bufferevent_enable(bufev_local, EV_READ | EV_WRITE);
}

void server::setConfig(unsigned short listen_port, std::string password) {
    server::listen_port = listen_port;
    std::string decoded = base64_decode(password);
    auto t = reinterpret_cast<const unsigned char *>(decoded.c_str());
    for (auto i = 0; i < decoded.length(); i++) {
        Cipher::password[i] = (int) t[i];
    }
}

void server::run() {

#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 2);
    auto err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        spdlog::warn("WSAStart启动失败");
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        spdlog::warn("找不到可用的 Winsock.dll");
        WSACleanup();
    } else {
        spdlog::info("Winsock 2.2 dll 加载成功");
    }
#else
    // 无视管道信号,发送数据给已关闭的socket
    if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    }
#endif // _WIN32
    auto base = event_base_new();
    if (base) {
        base_loop = base;
//        evdns_base_config_windows_nameservers()
        auto dns_base = evdns_base_new(base, 1);
        base_dns_loop = dns_base;

    } else { exit(0); }
    //绑定地址
    sockaddr_in sin_local{};
    memset(&sin_local, 0, sizeof(sin_local));
    sin_local.sin_family = AF_INET;
    sin_local.sin_port = htons(listen_port);

    //监听端口，接收tcp流量
    auto ev_listen = evconnlistener_new_bind(
            base,
            listen_cb,
            base,
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, //自动变为non-block
            16,
            (sockaddr *) &sin_local,
            sizeof(sin_local));
    if (ev_listen) {
        spdlog::info("监听在: [::]:{}", listen_port);
        //资源清理
        event_base_dispatch(base_loop);
        evconnlistener_free(ev_listen);
        event_base_free(base_loop);
    } else {
        event_base_free(base_loop);
        spdlog::warn("监听端口[{}]失败!", listen_port);
    }


}

void server::clear() {
#ifdef _WIN32
    WSACleanup();
#endif
}

event_base *server::base_loop = nullptr;
evdns_base *server::base_dns_loop = nullptr;
unsigned short server::listen_port = 7009;

void server::dns_cb(int errcode, evutil_addrinfo *answer, void *ctx) {
    auto context = static_cast<Socks5ctx *>(ctx);
    if (!context) {
        spdlog::warn("dns_cb error!");
        return;
    }
    if (!errcode) {
        char temp_ip[128] = {0};
        auto sin = (sockaddr_in *) answer->ai_addr;
        auto ip = evutil_inet_ntop(answer->ai_family, &sin->sin_addr, temp_ip, sizeof(temp_ip));
        std::string real_ip = ip;
        context->request_ip = real_ip;
        spdlog::debug("Ip:{}-->port:{}", context->request_ip, context->request_port);

        //解析dns完成,连接远程服务器
        auto bufev_remote = bufferevent_socket_new(base_loop, -1, BEV_OPT_CLOSE_ON_FREE);
        sockaddr_in sin_remote{};
        memset(&sin_remote, 0, sizeof(sin_remote));
        sin_remote.sin_family = AF_INET;
        sin_remote.sin_port = htons(context->request_port);
        const char *src = context->request_ip.c_str();
        evutil_inet_pton(AF_INET, src, &sin_remote.sin_addr.s_addr);
        spdlog::debug("建立tcp的ip:{} port:{}", src, context->request_port);
        //服务端服务器超时时间为15s
        timeval t1 = {15, 0};
        bufferevent_set_timeouts(bufev_remote, &t1, nullptr); //读取 写入
        //设置回调
        bufferevent_setcb(bufev_remote, remote_read_cb, nullptr, remote_event_cb, context);
        bufferevent_enable(bufev_remote, EV_READ | EV_WRITE);
        //连接服务端
        auto re = bufferevent_socket_connect(
                bufev_remote,
                (sockaddr *) &sin_remote,
                sizeof(sin_remote));
        if (re == 0) {
            context->partner = bufev_remote;
        } else {
            spdlog::warn("connect事件失败");
            return;
        }
        evutil_freeaddrinfo(answer);
    } else {
        spdlog::info("dns解析出错....");
    }
}
