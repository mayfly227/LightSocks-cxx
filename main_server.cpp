#include "server.h"
#include "spdlog/spdlog.h"
#include "ArduinoJson-v6.15.0.hpp"
#include "util.h"

#ifndef _WIN32

#include <csignal>
#include <fstream>

#endif
using ArduinoJson::DynamicJsonDocument;
using ArduinoJson::serializeJson;

int main(int argc, char **argv) {
#ifndef _WIN32
    if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        spdlog::warn("管道信号发生...");
    }
#endif
    spdlog::set_level(spdlog::level::info);

    const char *config_name = "config_server.json";
    DynamicJsonDocument doc{1024};
    auto ret = Util::readJsonConfig(config_name, doc);
    if (!ret) {
        spdlog::warn("加载配置文件[{}]失败", config_name);
        std::ofstream outfile;
        doc["listen_port"]=7009;
        doc["password"]=Util::genPassword();
        outfile.open("config_server.json");
        serializeJson(doc,outfile);
        outfile.close();
        doc.clear();
        spdlog::info("已在当前文件夹下自动生成了[config_server.json]", config_name);
    }
    //read config from json
    spdlog::info("成功加载配置文件[{}]", config_name);
    const int listen_port = doc["listen_port"];
    const char *password = doc["password"];
    spdlog::info("读取到监听地址:[::]:{}", listen_port);
    spdlog::info("读取到密码:{}", password);

    server::setConfig(listen_port, password);
    server::run();
    server::clear();
}