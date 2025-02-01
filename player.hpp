#pragma once

#include "car.hpp"

namespace db {
    typedef uint8_t car_id_t;

    class Player : public Car {
    public:
        Player(sf::Vector2f startPos = sf::Vector2f(0, 0)) : Car(startPos) {}

        void handleInput() {
            updateSpeed();
            updateDirection();
        }
    private:
        void updateSpeed() {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                m_speed += acceleration;
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                m_speed += -brake;
            } else {
                m_speed += -deceleration;
            }
            m_speed = std::clamp(m_speed, 0.0f, maxSpeed);
        }

        void updateDirection() {
            if (m_speed != 0.0f) {
                float speedFactor = m_speed / maxSpeed;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) m_angle += turnSpeed * speedFactor;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) m_angle -= turnSpeed * speedFactor;
            }
        }

        const float maxSpeed = 12.0f;
        const float acceleration = 0.2f;
        const float brake = 0.3f;
        const float deceleration = 0.15f;
        const float turnSpeed = 0.08f;
    };
}