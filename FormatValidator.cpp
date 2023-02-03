#include "FormatValidator.hpp"

bool FormatValidator::isTargetChar(const char dst, const char src) {
    return (dst == src);
}

bool FormatValidator::isLetter(const char dst) {
    return (tolower(dst) >= 'a' && tolower(dst) <= 'z');
}

bool FormatValidator::isDigit(const char dst) {
    return (dst >= '0' && dst <= '9');
}

bool FormatValidator::isSpecial(const char dst) {
    return ((dst >= '[' && dst <= '`') || (dst >= '{' && dst <= '}'));
}

bool FormatValidator::isValidNickname(const string& nickname) {
    string::const_iterator it = nickname.begin();

    if (!isLetter(*it) && !isSpecial(*it)) return false;

    ++it;
    for (; it != nickname.end(); ++it) {
        if (isLetter(*it) || isDigit(*it) || isSpecial(*it) || isTargetChar(*it, '-')) continue;

        return false;
    }
    return true;
}
