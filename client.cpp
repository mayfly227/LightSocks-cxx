//
// Created by itsuy on 2020/4/1.
//


#include "client.h"


void client::local_event_cb(bufferevent *bev, short what, void *ctx) {

    spdlog::debug("本地event_cb(what)-->: {0:x} ", what);
    auto partner = static_cast<bufferevent *>(ctx);
    //读到文件结尾或者发生错误
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        if (what & BEV_EVENT_ERROR) {
        }
        if (partner) {
            // flash all pending data
            local_read_cb(bev, partner);
            auto len = evbuffer_get_length(bufferevent_get_output(partner));
            spdlog::debug("local len-->[{}]", len);
            if (evbuffer_get_length(bufferevent_get_output(partner))) {
                /* We still have to flush data from the other
                 * side, but when that's done, close the other
                 * side. */
                bufferevent_setcb(partner, 0, close_on_finished_write_cb, remote_event_cb, 0);
                bufferevent_disable(partner, EV_READ);
            } else {
                auto partner_fd = bufferevent_getfd(partner);
                auto bev_fd = bufferevent_getfd(bev);
                bufferevent_free(partner);
            }
        }
        bufferevent_free(bev);
    } else {
        if (what & (BEV_EVENT_TIMEOUT | BEV_EVENT_READING)) {
            bufferevent_free(bev);
            bufferevent_free(partner);
            return;
        }
        if (what & BEV_EVENT_READING) {
            bufferevent_free(bev);
            bufferevent_free(partner);
            return;
        } else {
            bufferevent_free(bev);
            bufferevent_free(partner);
            return;
        }
    }
}

void client::local_read_cb(bufferevent *bev, void *arg) {
    auto partner = static_cast<bufferevent *>(arg);
    //cout << "[本地read_cb]" << endl;
    evbuffer *local, *dst;
    //获取当前连接客户端的evbuffer
    local = bufferevent_get_input(bev);
    auto len = evbuffer_get_length(local);
    if (!partner) {
        //把数据从缓冲区移除
        spdlog::debug("local drain 调用");
        evbuffer_drain(local, len);
    }
    //加密数据
    auto temp = evbuffer_new();
    while (len > 0) {
        unsigned char data[1024]{0};
        auto data_len = bufferevent_read(bev, data, sizeof(data) - 1);
        unsigned char temp_data[1024]{0};
        for (auto i = 0; i < data_len; i++) {
            temp_data[i] = password[data[i]];
        }
        evbuffer_add(temp, temp_data, data_len);
        len = evbuffer_get_length(local);
    }
    dst = bufferevent_get_output(partner);
    // local->dst(local->remote) 相当于把本地流量转发到远程
    evbuffer_add_buffer(dst, temp); //这个是复制了一份
    evbuffer_free(temp);
}

void client::remote_event_cb(bufferevent *bev, short what, void *ctx) {
    spdlog::debug("远程event_cb(what)-->: {0:x} ", what);
    auto partner = static_cast<bufferevent *>(ctx);

    if (what & BEV_EVENT_CONNECTED) {
        spdlog::debug("远程服务端tcp握手成功...");
        return;
    }

    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        if (what & BEV_EVENT_ERROR) {
        }
        if (partner) {
            // flash all pending data
            auto lens = evbuffer_get_length(bufferevent_get_output(partner));
            remote_read_cb(bev, partner);
            if (evbuffer_get_length(bufferevent_get_output(partner))) {
                /* We still have to flush data from the other
                 * side, but when that's done, close the other
                 * side. */
                bufferevent_setcb(partner, 0, close_on_finished_write_cb, local_event_cb, 0);
                bufferevent_disable(partner, EV_READ);
            } else {
                auto partner_fd = bufferevent_getfd(partner);
                auto bev_fd = bufferevent_getfd(bev);
                bufferevent_free(partner);
            }
        }
        bufferevent_free(bev);
    } else {
        if (what & BEV_EVENT_TIMEOUT && what & BEV_EVENT_READING) {
            bufferevent_free(bev);
            bufferevent_free(partner);
            return;
        }
    }
}

void client::remote_read_cb(bufferevent *bev, void *arg) {
    auto partner = static_cast<bufferevent *>(arg);
    evbuffer *remote, *dst;
    //获取远程客户端的evbuffer
    remote = bufferevent_get_input(bev);
    auto len = evbuffer_get_length(remote);
    if (!partner) {
        //把数据从缓冲区移除
        evbuffer_drain(remote, len);
    }
    //解密数据
    auto temp = evbuffer_new();
    while (len > 0) {
        unsigned char encode_data[1024]{0};
        auto data_len = bufferevent_read(bev, encode_data, sizeof(encode_data) - 1);
        unsigned char t_data[256] = {0};
        unsigned char temp_data[1024]{0};
        for (auto i = 0; i < 256; i++) {
            t_data[password[i]] = i;
        }
        for (auto i = 0; i < data_len; i++) {
            temp_data[i] = t_data[encode_data[i]];
        }
        evbuffer_add(temp, temp_data, data_len);
        len = evbuffer_get_length(remote);
    }
    // 把本地服务器的evbuffer复制到dst(remote -> dst)
    dst = bufferevent_get_output(partner);
    // remote->dst(remote->remote) 相当于把远程流量转发到本地
    evbuffer_add_buffer(dst, temp);
    evbuffer_free(temp);
}

void client::close_on_finished_write_cb(struct bufferevent *bev, void *ctx) {
    struct evbuffer *b = bufferevent_get_output(bev);

    if (evbuffer_get_length(b) == 0) {
        bufferevent_free(bev);
    }
}

void client::listen_cb(evconnlistener *ev, evutil_socket_t s, sockaddr *sin, int sin_len, void *arg) {

    auto base = static_cast<event_base *>(arg);
    //监听本地客户端,用本地客户端连接的socket
    auto bufev_local = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);
    //本地客户端超时为10s
    timeval local_time_out = {0, 0};
    bufferevent_set_timeouts(bufev_local, &local_time_out, 0);
    //----------------------------------------------------------------------//

    //连接到远程客户端
    auto bufev_remote = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    sockaddr_in sin_remote{};
    memset(&sin_remote, 0, sizeof(sin_remote));
    sin_remote.sin_family = AF_INET;
    sin_remote.sin_port = htons(remote_port);
    evutil_inet_pton(AF_INET, ip, &sin_remote.sin_addr.s_addr);

    //远程服务器超时时间为10s
    timeval t1 = {15, 0};
    bufferevent_set_timeouts(bufev_remote, &t1, 0); //读取 写入

    //连接服务端
    auto re = bufferevent_socket_connect(
            bufev_remote,
            (sockaddr *) &sin_remote,
            sizeof(sin_remote));
    if (re == 0) {
        spdlog::debug("connect success!");
    }
    //设置回调和事件

    bufferevent_setcb(bufev_local, local_read_cb, 0, local_event_cb, bufev_remote);
    bufferevent_setcb(bufev_remote, remote_read_cb, 0, remote_event_cb, bufev_local);
    bufferevent_enable(bufev_local, EV_READ | EV_WRITE);
    bufferevent_enable(bufev_remote, EV_READ | EV_WRITE);
}

void client::setConfig(const char *ip, unsigned short local_port, unsigned short remote_port, string password) {
    client::ip = ip;
    client::local_port = local_port;
    client::remote_port = remote_port;
    std::string decoded = base64_decode(password);
    auto t = reinterpret_cast<const unsigned char *>(decoded.c_str());
    for (auto i = 0; i < decoded.length(); i++) {
        client::password[i] = (int) t[i];
    }
}

void client::init() {

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
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
//        return 1;
    }
#endif // _WIN32
    auto base = event_base_new();
    if (base) {
        base_loop = base;
    }
    //绑定地址
    sockaddr_in sin_local{};
    memset(&sin_local, 0, sizeof(sin_local));
    sin_local.sin_family = AF_INET;
    sin_local.sin_port = htons(local_port);

    //监听端口，接收tcp流量
    auto ev_listen = evconnlistener_new_bind(
            base_loop,
            listen_cb,
            base_loop,
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, //自动变为non-block
            16,
            (sockaddr *) &sin_local,
            sizeof(sin_local));
    if (ev_listen) {
        spdlog::info("Listen on-->socks5://127.0.0.1:{}", local_port);
        //资源清理
        event_base_dispatch(base_loop);
        evconnlistener_free(ev_listen);
        event_base_free(base_loop);
    } else {
        spdlog::warn("Listen port [{}] error!", local_port);
    }


}

void client::clear() {
#ifdef _WIN32
    WSACleanup();
#endif
}

event_base *client::base_loop = nullptr;
const char *client::ip = nullptr;
unsigned short client::local_port;
unsigned short client::remote_port;
unsigned char client::password[256]{0};




