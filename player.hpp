#pragma once

#include "car.hpp"

class Player : public Car {
public:
    const float maxSpeed = 12.0f;
    const float acceleration = 0.2f;
    const float deceleration = 0.3f;
    const float turnSpeed = 0.08f;

    Player(sf::Vector2f startPos = sf::Vector2f(0, 0)) : Car(startPos) {}

    void handleInput() {
        updateSpeed();
        updateDirection();
    }

private:
    void updateSpeed() {
        float delta = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ? acceleration 
                    : (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ? -acceleration : -deceleration);
        speed += speed < 0 ? -delta : delta;  
        speed = std::clamp(speed, -maxSpeed, maxSpeed);
    }

    void updateDirection() {
        if (speed != 0.0f) {
            float speedFactor = speed / maxSpeed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) angle += turnSpeed * speedFactor;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) angle -= turnSpeed * speedFactor;
        }
    }
};
