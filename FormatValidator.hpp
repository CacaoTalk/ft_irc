#pragma once

#ifndef FORMATVALIDATOR_HPP
# define FORMATVALIDATOR_HPP

#include <string>

using namespace std;

struct FormatValidator {
    static bool isTargetChar(const char dst, const char src);
    static bool isLetter(const char dst);
    static bool isDigit(const char dst);
    static bool isSpecial(const char dst);

    static bool isValidNickname(const string& nickname);
    static bool isValidChannelname(const string& channelname);
};

#endif
