#pragma once

#include <cstdint>
#include <cstdlib>

namespace db {
    typedef std::uint8_t player_id_t;

    // Simple security for player identification
    class PlayerIdentity {
    public:
        player_id_t static generateToken() {
            player_id_t id;
            id = rand() % UINT8_MAX;
            return id;
        }
    };
}