#pragma once

#include <sstream>
#include <string>
#include <vector>

template <typename Out>
void split(const std::string &s, char delim, Out result) {
        std::istringstream iss(s);
        std::string item;
        while (std::getline(iss, item, delim)) {
                if (!item.empty())
                        *result++ = item;
        }
}

std::vector<std::string> split(const std::string &s, char delim);
