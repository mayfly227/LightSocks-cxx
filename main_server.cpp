#include "server.h"
#include "spdlog/spdlog.h"
#include "ArduinoJson-v6.15.0.hpp"
#include "util.h"
#include <fstream>

#ifndef _WIN32
#include <csignal>
#endif
#include <fstream>

using ArduinoJson::DynamicJsonDocument;
using ArduinoJson::serializeJson;


static void SignalHandler(int nSigno) {
    signal(nSigno, SignalHandler);
    switch (nSigno) {
        case SIGPIPE:
            printf("Process will not exit\n");
            break;
        default:
            printf("%d signal unregister\n", nSigno);
            break;
    }
}


int main(int argc, char **argv) {
    sigset_t signal_mask;
    sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGPIPE);
    int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, nullptr);
    if (rc != 0) {
        printf("block sigpipe error\n");
    }
    spdlog::set_level(spdlog::level::info);

    const char *config_name = "config_server.json";
    DynamicJsonDocument doc{1024};
    auto ret = Util::readJsonConfig(config_name, doc);
    if (!ret) {
        spdlog::warn("load config file [{}] error!", config_name);
        std::ofstream outfile;
        doc["listen_port"] = 7009;
        doc["password"] = Util::genPassword();
        outfile.open("config_server.json");
        serializeJson(doc, outfile);
        outfile.close();
        spdlog::info("[config_server.json] has been automatically generated in the current folder", config_name);
    }
    //read config from json
    spdlog::info("successful load config file [{}]", config_name);
    const int listen_port = doc["listen_port"];
    const char *password = doc["password"];
    spdlog::info("get listen address:[::]:{}", listen_port);
    spdlog::info("get password:{}", password);
    doc.clear();
    server::setConfig(listen_port, password);
    server::run();
    server::clear();
}