#include "util.h"
#include "client.h"
#include "ArduinoJson-v6.15.0.hpp"
#include "spdlog/spdlog.h"
#include <fstream>

using ArduinoJson::DynamicJsonDocument;

int main(int argc, char const *argv[]) {
    //解析json配置文件
    spdlog::set_level(spdlog::level::info);
    const char *config_name = "config.json";
    DynamicJsonDocument doc{1024};
    auto ret = Util::readJsonConfig(config_name, doc);
    if (ret) {
        spdlog::warn("加载配置文件[{}]失败", config_name);
        doc.clear();
        std::ofstream outfile;
        const char config_json[] = R"({"ip":"23.102.255.234","remote_port":7009,"local_port":7878,"password":"******"})";
        outfile.open("config.json");
        outfile << config_json;
        spdlog::info("已在当前文件夹自动生成了[config.json]", config_name);
        outfile.close();
#ifdef _WIN32
        system("pause");
#endif
        return 0;
    }
    spdlog::info("成功加载配置文件[{}]", config_name);

    const char *ip = doc["ip"];
    const char *password = doc["password"];
    const int local_port = doc["local_port"];
    const int remote_port = doc["remote_port"];
    spdlog::info("读取到IP:{}", ip);
    spdlog::info("读取到密码:{}", password);
    spdlog::info("读取到本地端口:{}", local_port);
    spdlog::info("读取到远程端口:{}", remote_port);
    client::setConfig(ip, local_port, remote_port, password);
    doc.clear();
    client::init();
    client::clear();
    return 0;
}
