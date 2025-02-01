#pragma once

#include <cstdint>
#include <iostream>

namespace db {
    // Simple security for player identification
    class PlayerIdentity {
    public:
        car_id_t static generateToken() {
            car_id_t id;
            id = rand() % UINT8_MAX;
            return id;
        }
    };
}