# [Lightsocks-c++](https://github.com/maxlv7/lightsocks-c++)

一个轻量级网络混淆代理，基于 SOCKS5 协议，可用来代替 Shadowsocks(?)。

- 只专注于混淆，用最简单高效的混淆算法达到目的；
- 基于c++ 11实现,使用高效的[libevent](https://github.com/libevent/libevent)网络库；

> 本项目为 [你也能写个 Shadowsocks](https://github.com/gwuhaolin/blog/issues/12) 的 c++ 实现
> 作者实现了 GO 版本 **[Lightsocks](https://github.com/gwuhaolin/lightsocks)**

> **重要提示：目前只实现了client,服务端正在开发之中...**

> **代码写的不好,多多海涵~**

# 编译
## CMake (Windows)

Install CMake: <https://www.cmake.org>
针对windows的编译安装,我已经提供了libevent2.1.11的64位编译版本，位于lib/event
```
$ md build && cd build
$ cmake -G "Visual Studio 16 2019" -A x64 ..   
$ cmake --build . --config Release 

```

## CMake (Linux)
请先安装libevent2.1.11
<https://github.com/libevent/libevent>
```
$ mkdir build && cd build 
$ cmake ..
$ make
```
# 使用
直接运行生成的可执行文件,程序会自动读取当前目录下的config.json
如果没有config.json文件，那么程序会在当前目录下自动生成config.json
其各字段含义如下:
```
{
  "ip": "23.102.255.234", //远程服务器地址
  "remote_port": 7009, //远程服务器端口
  "local_port": 7878, //监听的本地地址
  "password": "******" //密码
}
```
如果一切配置无误,那么启动程序就会看到：
```
[2020-04-02 14:55:25.967] [info] 成功加载配置文件[config.json]
[2020-04-02 14:55:25.968] [info] 读取到IP:23.102.255.234
[2020-04-02 14:55:25.968] [info] 读取到密码:******
[2020-04-02 14:55:25.968] [info] 读取到本地端口:7878
[2020-04-02 14:55:25.968] [info] 读取到远程端口:7009
[2020-04-02 14:55:25.969] [info] 成功监听地址-->socks5://127.0.0.1:7878
```

## 感谢
<https://github.com/libevent/libevent>
<https://github.com/gwuhaolin/lightsocks>
<https://github.com/linw1995/lightsocks-python>
<https://github.com/ReneNyffenegger/cpp-base64>
<https://github.com/bblanchon/ArduinoJson>
<https://github.com/LeeReindeer/lightsocks-c>

