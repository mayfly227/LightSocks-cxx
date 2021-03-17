'''
socks5 协议
'''
'''
1.客户端发送给服务器的消息格式
+----+----------+----------+
|VER | NMETHODS | METHODS  |
+----+----------+----------+
| 1  |    1     |  1~255   |
+----+----------+----------+
-VER：代表 SOCKS 的版本，SOCKS5 默认为0x05，其固定长度为1个字节；
-NMETHODS：表示第三个字段METHODS的长度，它的长度也是1个字节；
-METHODS：表示客户端支持的验证方式，可以有多种，他的长度是1-255个字节。

目前支持的验证方式共有：
0x00：NO AUTHENTICATION REQUIRED（不需要验证）
0x01：GSSAPI
0x02：USERNAME/PASSWORD（用户名密码）
0x03: to X'7F' IANA ASSIGNED
0x80: to X'FE' RESERVED FOR PRIVATE METHODS
0xFF: NO ACCEPTABLE METHODS（都不支持，没法连接了）
'''

'''
2.响应连接
+----+--------+
|VER | METHOD |
+----+--------+
| 1  |   1    |
+----+--------+
VER：代表 SOCKS 的版本，SOCKS5 默认为0x05，其固定长度为1个字节；
METHOD：代表服务端需要客户端按此验证方式提供的验证信息，其值长度为1个字节，可为上面六种验证方式之一。
'''
'''
3.和目标服务建立连接
+----+-----+-------+------+----------+----------+
|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  |   1   |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
VER：代表 SOCKS 协议的版本，SOCKS 默认为0x05，其值长度为1个字节；
CMD：代表客户端请求的类型，值长度也是1个字节，有三种类型；
CONNECT： 0x01；
BIND： 0x02；
UDP： ASSOCIATE 0x03；
RSV：保留字，值长度为1个字节；
ATYP：代表请求的远程服务器地址类型，值长度1个字节，有三种类型；
IPV4： address: 0x01；
DOMAINNAME: 0x03；
IPV6： address: 0x04；
DST.ADDR：代表远程服务器的地址，根据 ATYP 进行解析，值长度不定；
DST.PORT：代表远程服务器的端口，要访问哪个端口的意思，值长度2个字节。
'''
'''
4.收到客户端的请求后，服务器会返回如下格式的消息：
05000003000000
+----+-----+-------+------+----------+----------+
|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  |   1   |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+
VER：代表 SOCKS 协议的版本，SOCKS 默认为0x05，其值长度为1个字节；
REP代表响应状态码，值长度也是1个字节，有以下几种类型
0x00 succeeded
0x01 general SOCKS server failure
0x02 connection not allowed by ruleset
0x03 Network unreachable
0x04 Host unreachable
0x05 Connection refused
0x06 TTL expired
0x07 Command not supported
0x08 Address type not supported
0x09 to 0xFF unassigned
RSV：保留字，值长度为1个字节
ATYP：代表请求的远程服务器地址类型，值长度1个字节，有三种类型
IP V4 address： 0x01
DOMAINNAME： 0x03
IP V6 address： 0x04
BND.ADDR：表示绑定地址，值长度不定。
BND.PORT： 表示绑定端口，值长度2个字节
'''
