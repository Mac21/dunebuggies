#pragma once

#include "iostream"

// Simple security for player identification
class PlayerIdentity {
public:
    std::string static generateToken() {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        std::string token;
        for (int i = 0; i < 16; i++) {
            size_t len = (sizeof(alphanum) / sizeof(alphanum[0]));
            token += alphanum[rand() % len];
        }
        return token;
    }
};