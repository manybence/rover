#include <iostream>
#include <string>

class Base64 {
private:
    static const char b64e[64];
    static const int b64d[256];

public:
    static String b64ConvertInt(int value, int length);
    static int b64ConvertString(const String& s);
};

// Definition of static members outside the class

    const char Base64::b64e[64] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
        'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
        'y', 'z', '+', '/'
    };

const int Base64::b64d[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0, 62,  0,  0 , 0, 63,  0,  1,
    2,  3,  4,  5,  6,  7,  8,  9,  0,  0,
    0,  0,  0,  0,  0, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
   25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
   35,  0,  0,  0,  0,  0,  0, 36, 37, 38,
   39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
   49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
   59, 60, 61,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0
};


// Implementation of functions remains the same
String Base64::b64ConvertInt(int value, int length) {
    String result;
    if (length == 5) {
        result += b64e[(value & 1056964608) >> 24];
        result += b64e[(value & 16515072) >> 18];
        result += b64e[(value & 258048) >> 12];
        result += b64e[(value & 4032) >> 6];
        result += b64e[(value & 63)];
    } else if (length == 4) {
        result += b64e[(value & 16515072) >> 18];
        result += b64e[(value & 258048) >> 12];
        result += b64e[(value & 4032) >> 6];
        result += b64e[(value & 63)];
    } else if (length == 3) {
        result += b64e[(value & 258048) >> 12];
        result += b64e[(value & 4032) >> 6];
        result += b64e[(value & 63)];
    } else if (length == 2) {
        result += b64e[(value & 4032) >> 6];
        result += b64e[(value & 63)];
    } else {
        result += b64e[(value & 63)];
    }
    return result;
}

int Base64::b64ConvertString(const String& s) {
    int n = s.length();
    if (n == 5) {
        return (b64d[(unsigned char)s[0]] << 24) +
               (b64d[(unsigned char)s[1]] << 18) +
               (b64d[(unsigned char)s[2]] << 12) +
               (b64d[(unsigned char)s[3]] << 6) +
               b64d[(unsigned char)s[4]];
    } else if (n == 4) {
        return (b64d[(unsigned char)s[0]] << 18) +
               (b64d[(unsigned char)s[1]] << 12) +
               (b64d[(unsigned char)s[2]] << 6) +
               b64d[(unsigned char)s[3]];
    } else if (n == 3) {
        return (b64d[(unsigned char)s[0]] << 12) +
               (b64d[(unsigned char)s[1]] << 6) +
               b64d[(unsigned char)s[2]];
    } else if (n == 2) {
        return (b64d[(unsigned char)s[0]] << 6) +
               b64d[(unsigned char)s[1]];
    } else if (n == 1) {
        return b64d[(unsigned char)s[0]];
    }

    return 0;
}