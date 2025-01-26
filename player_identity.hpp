#pragma once

#include "iostream"

namespace db {
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
                constexpr size_t len = sizeof(alphanum) - 1;
                token += alphanum[rand() % len];
            }
            return token;
        }
    };
}