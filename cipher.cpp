//
// Created by Li on 2020/4/9.
//

#include "cipher.h"
#include <iostream>

void Cipher::decrypt(bufferevent *self, bufferevent *partner) {
    unsigned char d_password[256] = {0};
    for (auto i = 0; i < 256; i++) {
        d_password[Cipher::password[i]] = i;
    }
    while (true) {
        unsigned char encode_data[2048]{0};
        auto data_len = bufferevent_read(self, encode_data, sizeof(encode_data) - 1);
        if (data_len <= 0) {
            break;
        }
        unsigned char temp_data[2048]{0};
        for (auto i = 0; i < data_len; i++) {
            temp_data[i] = d_password[encode_data[i]];
        }
        bufferevent_write(partner, temp_data, data_len);
    }
}

void Cipher::encrypt(bufferevent *self, bufferevent *partner) {
    while (true) {
        unsigned char data[2048]{0};
        auto data_len = bufferevent_read(partner, data, sizeof(data) - 1);
        if (data_len <= 0) {
            break;
        }
        unsigned char temp_data[2048]{0};
        for (auto i = 0; i < data_len; i++) {
            temp_data[i] = Cipher::password[data[i]];
        }
        bufferevent_write(self, temp_data, data_len);
    }
}

void Cipher::encrypt_byte(unsigned char bytes[], int len) {
    unsigned char t_bytes[32] = {0};
    for (int k = 0; k < len; k++) {
        t_bytes[k] = bytes[k];
    }
    for (int i = 0; i < len; ++i) {
        bytes[i] = Cipher::password[t_bytes[i]];
    }
}

void Cipher::decrypt_bytes(unsigned char bytes[], int len) {
    unsigned char r_password[256] = {0};
    unsigned char t_bytes[32]{0};
    for (int k = 0; k < len; k++) {
        t_bytes[k] = bytes[k];
    }
    for (auto j = 0; j < 256; j++) {
        r_password[Cipher::password[j]] = j;
    }
    for (int i = 0; i < len; i++) {
        bytes[i] = r_password[t_bytes[i]];
    }
}

unsigned char Cipher::password[256]{0};

