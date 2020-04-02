//
// Created by itsuy on 2020/4/1.
//

#ifndef SBPROXY_UTIL_H
#define SBPROXY_UTIL_H

#include <string>
#include "ArduinoJson-v6.15.0.hpp"

using ArduinoJson::DynamicJsonDocument;
using std::string;

class Util {
public:
    static int readJsonConfig(const char *filename, DynamicJsonDocument &doc);
};


#endif //SBPROXY_UTIL_H
