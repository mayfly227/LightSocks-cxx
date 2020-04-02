//
// Created by itsuy on 2020/4/1.
//

#include "util.h"
#include <fstream>

using std::ifstream;

int Util::readJsonConfig(const char *filename, DynamicJsonDocument &doc) {
    ifstream infile;
    infile.open(filename);
    if (!infile.is_open()) {
        return -1;
    }
    deserializeJson(doc, infile);
    infile.close();
    return 0;
}
