#include "util.h"
#include "client.h"
#include "ArduinoJson-v6.15.0.hpp"
#include "spdlog/spdlog.h"
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#endif
using ArduinoJson::DynamicJsonDocument;

int main(int argc, char const *argv[]) {
    //解析json配置文件
    spdlog::set_level(spdlog::level::info);
    const char *config_name = "config.json";
    DynamicJsonDocument doc{1024};
    auto ret = Util::readJsonConfig(config_name, doc);
    if (!ret) {
        spdlog::warn("load config.json[{}] error!", config_name);
        doc.clear();
        std::ofstream outfile;
        const char config_json[] = R"({"ip":"your server ip","remote_port":7009,"local_port":7878,"password":"your server passwrod"})";
        outfile.open("config.json");
        outfile << config_json;
        spdlog::info("[config.json] has been automatically generated in the current folder", config_name);
        outfile.close();
#ifdef _WIN32
        system("pause");
#endif
        return 0;
    }
    spdlog::info("successful load file[{}]", config_name);

    const char *ip = doc["ip"];
    const char *password = doc["password"];
    const int local_port = doc["local_port"];
    const int remote_port = doc["remote_port"];
    spdlog::info("get IP:{}", ip);
    spdlog::info("get password:{}", password);
    spdlog::info("get local port:{}", local_port);
    spdlog::info("get remote port:{}", remote_port);
    client::setConfig(ip, local_port, remote_port, password);
    doc.clear();
#ifdef _WIN32
    PROCESS_INFORMATION pi;	//进程信息
    STARTUPINFO si;			//进程启动信息
    memset(&si, 0, sizeof(STARTUPINFO));
    si.cb = sizeof(si);
    si.wShowWindow = SW_HIDE;
    si.dwFlags = STARTF_USESHOWWINDOW;
    if(CreateProcess(_T("privoxy.exe"), _tcsdup(TEXT("config.txt")), 0, 0, false, 0, 0, 0, &si, &pi)){
        spdlog::info("调用privoxy成功...");
    }
#endif
    client::init();
    client::clear();
    return 0;
}
