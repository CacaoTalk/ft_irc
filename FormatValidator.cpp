#include "FormatValidator.hpp"

/**
 * @brief Compare whether char <A> and char <B> are the same.
 * 
 * @param dst Character A to be compared
 * @param src Character B to be compared
 * @return true : Character <A> and <B> is same, if is not
 * @return false 
 */
bool FormatValidator::isTargetChar(const char dst, const char src) {
    return (dst == src);
}

/**
 * @brief Check char <A> is letter(As defined in the IRC RFC documentation).
 * 
 * @param dst Character A to be compared
 * @return true : Character <A> is letter, if is not
 * @return false 
 */
bool FormatValidator::isLetter(const char dst) {
    return (tolower(dst) >= 'a' && tolower(dst) <= 'z');
}

/**
 * @brief Check char <A> is digit(As defined in the IRC RFC documentation).
 * 
 * @param dst Character A to be compared
 * @return true : Character <A> is digit, if is not
 * @return false 
 */
bool FormatValidator::isDigit(const char dst) {
    return (dst >= '0' && dst <= '9');
}

/**
 * @brief Check char <A> is special character(As defined in the IRC RFC documentation).
 * 
 * @param dst Character A to be compared
 * @return true : Character <A> is special character, if is not
 * @return false 
 */
bool FormatValidator::isSpecial(const char dst) {
    return ((dst >= '[' && dst <= '`') || (dst >= '{' && dst <= '}'));
}

/**
 * @brief Check that the given nickname fits the rule required by the IRC Protocol.
 * 
 * @param nickname Given nickname to be checked
 * @return true : Given nickname fits the rule, if is not
 * @return false 
 */
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

/**
 * @brief Check that the given channel name fits the rule required by the IRC Protocol.
 * 
 * @param nickname Given channel name to be checked
 * @return true : Given channel name fits the rule, if is not
 * @return false 
 */
bool FormatValidator::isValidChannelname(const string& channelname) {
    string::const_iterator it = channelname.begin();

    for (; it != channelname.end(); ++it) {
        if (!isTargetChar(*it, 7)) continue;

        return false;
    }
    return true;
}
