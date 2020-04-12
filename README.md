# [Lightsocks-c++](https://github.com/maxlv7/lightsocks-c++)

一个轻量级网络混淆代理，基于 SOCKS5 协议，可用来代替 Shadowsocks(?)。

- 只专注于混淆，用最简单高效的混淆算法达到目的；
- 基于`c++ 11`实现,使用高效的[libevent](https://github.com/libevent/libevent)网络库；
- 使用`cmake`构建,方便实现跨平台

> 本项目为 [你也能写个 Shadowsocks](https://github.com/gwuhaolin/blog/issues/12) 的 c++ 实现
> 作者实现了 GO 版本 **[Lightsocks](https://github.com/gwuhaolin/lightsocks)**

> **c++初学者,代码写的不好,多多海涵~**

## 编译
### CMake (Windows)

Install CMake: <https://www.cmake.org>
针对windows的编译安装,我已经提供了libevent2.1.11的64位编译版本，位于lib/event
```
$ md build && cd build
$ cmake -G "Visual Studio 16 2019" -A x64 ..   
$ cmake --build . --config Release 

```

### CMake (Linux)
请先安装libevent2.1.11
<https://github.com/libevent/libevent>
Linux:克隆本项目<https://github.com/maxlv7/LightSocks-cxx.git>，进入项目主目录。
```
$ mkdir build && cd build 
$ cmake ..
$ make
```
如果不出意外的话,将会在当前目录下生成两个可执行文件：`LightSocks-client`和`LightSocks-server`

## 使用

### 客户端使用
直接运行生成的可执行文件,程序会自动读取当前目录下的config.json
如果没有config.json文件，那么程序会在当前目录下自动生成config.json
其各字段含义如下:

```
{
  "ip": "23.102.255.234", //远程服务器地址
  "remote_port": 7009, //远程服务器端口
  "listen_port": 7878, //监听的本地地址
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
### 服务端使用
直接运行生成的可执行文件,程序会自动读取当前目录下的config_server.json
如果没有config_server.json文件，那么程序会在当前目录下自动生成默认的config_server.json
其各字段含义如下:
```
{
  "listen_port": 7009, //监听的本地地址
  "password": "******" //密码
}
```
如果一切配置无误,那么启动程序就会看到：
```
[2020-04-12 16:25:28.212] [info] 成功加载配置文件[config_server.json]
[2020-04-12 16:25:28.213] [info] 读取到监听地址:[::]:7009
[2020-04-12 16:25:28.213] [info] 读取到密码:nq9Bib/tgSfiVvrRPjWUU9PCcRnmBKBiLSkF3brP9PnwxH5lDYymyd+7zB7hKKg6nypHSI37vFTjrYPXWAn9iuTvZqm9sR9tW6IsTzzVsEpXkRQBLtyVcxrn+EP+QpaX6wOOUPak26Px1CE22CIHC6XKrg8xAIa589rlzaclThAwFXmHMjOEmCZG2ZxfwUSS6qx6aXhgbvfgqky2cFWba5DpyIUdxQw7OS9kG2rWBn+Pi4KyAkB3P7j1UfxJiG8gtxhsNwo0DscrzrPuOKGZq3VLJJqTXF59XXLyWXydY+xoEcN26BwXI4BSy7XGEwj/Ft6+PRJ0Z3vARdJN0FphtA==
[2020-04-12 16:25:28.214] [info] 监听在: 127.0.0.1:7009
```
## 特别感谢
<https://github.com/libevent/libevent>

<https://github.com/gwuhaolin/lightsocks>

<https://github.com/linw1995/lightsocks-python>

<https://github.com/ReneNyffenegger/cpp-base64>

<https://github.com/bblanchon/ArduinoJson>

<https://github.com/LeeReindeer/lightsocks-c>

<https://github.com/gabime/spdlog>

## 已知问题
1. server端内存泄漏(原因不明)

