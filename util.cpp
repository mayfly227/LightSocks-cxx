//
// Created by itsuy on 2020/4/1.
//

#include "util.h"
#include <fstream>
#include <random>
#include <base64/base64.h>

using std::ifstream;

int Util::readJsonConfig(const char *filename, DynamicJsonDocument &doc) {
    ifstream infile;
    infile.open(filename);
    if (!infile.is_open()) {
        return 0;
    }
    deserializeJson(doc, infile);
    infile.close();
    return 1;
}

std::string Util::genPassword() {
    std::random_device rd; // 将用于为随机数引擎获得种子
    std::mt19937 gen(rd()); // 以播种标准 mersenne_twister_engine
    std::uniform_int_distribution<> dis(0, 255); //0-255
    unsigned char pwd[256] = {};
    int index = 0;
    while (true) {
        bool flag = true;
        auto num = dis(gen);
        for (auto j = 0; j < index; j++) {
            if (num == pwd[j]) {
                flag = false;
                break;
            }
        }
        if (flag) {
            pwd[index] = num;
            index++;
        }
        if (index == 256) {
            break;
        }
    }
    std::string p = base64_encode(pwd, 256);
    return p;
}
