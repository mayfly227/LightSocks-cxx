//
// Created by Li on 2020/4/9.
//

#ifndef LIGHTSOCKS_CIPHER_H
#define LIGHTSOCKS_CIPHER_H

#include <string>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

class Cipher {
public:
    Cipher() = default;

public:
    // TODO bug fixed
    static void encrypt_byte(unsigned char bytes[], int len);

    static void decrypt_bytes(unsigned char bytes[], int len);

    // TODO bug fixed
    static void decrypt(bufferevent *self, bufferevent *partner);

    static void encrypt(bufferevent *self, bufferevent *partner);

public:
    static unsigned char password[256];
};


#endif //LIGHTSOCKS_CIPHER_H
