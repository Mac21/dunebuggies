#pragma once

#include "car.hpp"

class Player : public Car {
public:
    const float maxSpeed = 12.0f;
    const float acceleration = 0.2f;
    const float brake = 0.3f;
    const float deceleration = 0.15f;
    const float turnSpeed = 0.08f;
    const std::string id;

    Player(sf::Vector2f startPos = sf::Vector2f(0, 0)) : Car(startPos), id(PlayerIdentity::generateToken()) {}

    void handleInput() {
        updateSpeed();
        updateDirection();
    }

private:
    void updateSpeed() {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            speed += acceleration;
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            speed += -brake;
        } else {
            speed += -deceleration;
        }
        speed = std::clamp(speed, 0.0f, maxSpeed);
    }

    void updateDirection() {
        if (speed != 0.0f) {
            float speedFactor = speed / maxSpeed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) angle += turnSpeed * speedFactor;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) angle -= turnSpeed * speedFactor;
        }
    }
};
