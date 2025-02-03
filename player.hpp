#pragma once

#include "car.hpp"

namespace db {
    class Player : public Car {
    public:
        Player(sf::Vector2f startPos = sf::Vector2f(CHECKPOINTS[0]));

        void handleInput();
    private:
        void updateSpeed();
        void updateDirection();

        const float maxSpeed = 12.0f;
        const float acceleration = 0.2f;
        const float brake = 0.3f;
        const float deceleration = 0.15f;
        const float turnSpeed = 0.08f;
    };
}
