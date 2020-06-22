#include <event2/util.h>
#include <event2/dns.h>

#include "server.h"
#include "cipher.h"
#include "socks5.h"

#define ENCRYPT

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif


void server::local_event_cb(bufferevent *bev, short what, void *ctx) {

    spdlog::debug("local event_cb(what)-->: {0:x} ", what);
    auto context = static_cast<Socks5ctx *>(ctx);
    if (!context) {
        spdlog::warn("with no context in local_event...");
        return;
    }
    //读到文件结尾或者发生错误
    if (what & BEV_EVENT_TIMEOUT) {
        spdlog::debug("本地操作时发生错误");
        if (context->partner) {
            bufferevent_free(context->partner);
            context->partner = nullptr;
        }
        bufferevent_free(context->self);
        context->self = nullptr;
        delete context;
        context = nullptr;
    }
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        if (what & BEV_EVENT_ERROR) {
            spdlog::debug("local error!");
        }
        if (context->partner) {
            bufferevent_free(context->partner);
            context->partner = nullptr;
        }
        bufferevent_free(context->self);
        context->self = nullptr;
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
    socks5::handle_first(bev, context);

}

void server::remote_event_cb(bufferevent *bev, short what, void *ctx) {
    //bug fixed
    spdlog::debug("remote event_cb(what)-->: {0:x} ", what);
    auto context = static_cast<Socks5ctx *>(ctx);
    if (!context) {
        spdlog::warn("没有context了 remote_event...");
        return;
    }
    if (what & BEV_EVENT_CONNECTED) {
        spdlog::debug("remote server tcp handshake successful...");
        // bug: 这个时候有可能已经没有Local的context了
        if (!context->self) {
            spdlog::warn("with no local context...");
            return;
        }
        unsigned char reply[7] = {0x05, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00};
#ifdef ENCRYPT
        Cipher::encrypt_byte(reply, sizeof(reply));
#endif
        //返回握手包
        bufferevent_write(context->self, reply, sizeof(reply));
        return;
    }
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        if (what & BEV_EVENT_ERROR) {
            spdlog::debug("remote server error...");
        }
        if (context->self) {
            bufferevent_free(context->self);
            context->self = nullptr;
        }
        bufferevent_free(context->partner);
        context->partner = nullptr;
        delete context;
        context = nullptr;
    } else {
        if (what & BEV_EVENT_TIMEOUT) {
            spdlog::debug("remote server read data timeout...");
            if (context->self) {
                std::cout << "context:" << context->self << std::endl;
                bufferevent_free(context->self);
                context->self = nullptr;
            }
            bufferevent_free(bev);
            context->partner = nullptr;
            delete context;
            context = nullptr;
        }
    }
}

void server::remote_read_cb(bufferevent *bev, void *arg) {
    auto context = static_cast<Socks5ctx *>(arg);
    if (!context) {
        spdlog::warn("with no context in remote_read_cb...");
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
    Cipher::encrypt(context->self, context->partner);
#else
    evbuffer *src,*dst;
    src = bufferevent_get_input(context->partner);
    dst = bufferevent_get_output(context->self);
    evbuffer_add_buffer(dst, src);
#endif
}

void server::listen_cb(evconnlistener *lev, evutil_socket_t s, sockaddr *sin, int sin_len, void *arg) {
    char ip[128]{0};
    auto r_sa = (sockaddr_in *) sin;
    evutil_inet_ntop(r_sa->sin_family, &(r_sa->sin_addr.s_addr), ip, sin_len);
    spdlog::info("New connection from {}:{}", ip, r_sa->sin_port);

    auto base = evconnlistener_get_base(lev);
    //监听本地客户端,用本地客户端连接的socket
    auto bufev_local = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    //本地客户端超时为15s(BUG)
    timeval local_time_out = {0, 0};
    bufferevent_set_timeouts(bufev_local, &local_time_out, 0);
    //----------------------------------------------------------------------//
    //设置上下文
    Socks5ctx *socks5Ctx = new Socks5ctx();
    //初始化参数
    socks5Ctx->self = bufev_local;
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
        spdlog::info("Listen on: [::]:{}", listen_port);
        //资源清理
        event_base_dispatch(base_loop);
        evconnlistener_free(ev_listen);
        event_base_free(base_loop);
    } else {
        event_base_free(base_loop);
        spdlog::warn("Listen port [{}] error!", listen_port);
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
    if (errcode) {
        spdlog::info("dns解析出错....");
        spdlog::warn("%s", evutil_gai_strerror(errcode));
    } else {
        char temp_ip[128] = {0};
        auto sin = (sockaddr_in *) answer->ai_addr;
        auto ip = evutil_inet_ntop(answer->ai_family, &sin->sin_addr, temp_ip, sizeof(temp_ip));
        //可能解析不成功
        if (!ip) {
            spdlog::warn("ip resolve error!");
            exit(0);
        }
//        std::string real_ip = temp_ip;
//        context->request_ip = real_ip;
        std::cout << ip << std::endl;
        context->request_ip = std::string(temp_ip);
        spdlog::debug("Ip:{}-->port:{}", context->request_ip, context->request_port);

        //解析dns完成,连接远程服务器
        auto bufev_remote = bufferevent_socket_new(base_loop, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
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
            evutil_freeaddrinfo(answer);
        }
    }
}

