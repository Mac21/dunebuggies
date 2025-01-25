#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Graphics/Color.hpp>
#include <random>
#include <cmath>

namespace db {
    // Checkpoint coordinates
    constexpr int NUM_CHECKPOINTS = 8;

    const std::vector<sf::Vector2f> CHECKPOINTS = {
        {300, 610},
        {1270, 430},
        {1380, 2380},
        {1900, 2460},
        {1970, 1700},
        {2550, 1680},
        {2560, 3150},
        {500, 3300}
    };

    class Car {
    public:
        sf::Vector2f position;
        sf::Color color;
        float speed, angle;
        int currentCheckpoint;
        sf::Vector2f velocity;
        bool isDrifting = false;

        // Constants for physics and movement
        static constexpr float TURN_SPEED = 0.005f;
        static constexpr float CHECKPOINT_RADIUS_SQUARED = 625.0f; // 25 * 25
        static constexpr float JUMP_FORCE = 10.0f;
        static constexpr float GRAVITY = 0.5f;
        static constexpr float DRIFT_MULTIPLIER = 1.5f;

        Car(sf::Vector2f startPos = sf::Vector2f(0, 0), float startAngle = 0.0f)
            : position(startPos),
              speed(0.0f),
              angle(startAngle),
              currentCheckpoint(0),
              velocity{0, 0}
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 255);
            color = sf::Color(dis(gen), dis(gen), dis(gen));
        }

        void move() {
            updatePhysics();
            sf::Vector2f direction = { std::sin(angle), -std::cos(angle) };
            position += direction * speed;
        }

        void findNextCheckpoint() {
            const auto& target = CHECKPOINTS[currentCheckpoint];
            float beta = angle - std::atan2(target.x - position.x, -target.y + position.y);

            float turnSpeed = TURN_SPEED * (isDrifting ? DRIFT_MULTIPLIER : 1.0f);
            if (std::sin(beta) < 0) {
                angle += turnSpeed * speed;
            } else {
                angle -= turnSpeed * speed;
            }
            angle = std::fmod(angle + 2 * M_PI, 2 * M_PI); // Normalize angle

            if (squaredDistance(target) < CHECKPOINT_RADIUS_SQUARED) {
                currentCheckpoint = (currentCheckpoint + 1) % NUM_CHECKPOINTS;
            }
        }

        void jump() {
            if (position.y == 0) { // Assuming ground level is at y=0, adjust as needed
                velocity.y = -JUMP_FORCE;
            }
        }

        void updatePhysics() {
            // Update position with current velocity
            position += velocity;

            // Apply gravity
            velocity.y += GRAVITY;

            // Ground check (pseudo-code, adjust based on your track design)
            constexpr float groundLevel = 0.0f; // This should be adjusted based on your actual track design
            if (position.y > groundLevel) {
                position.y = groundLevel;
                velocity.y = 0.0f; // Stop falling when on the ground
            }
        }

       virtual void handleInput() {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
                if (speed > 0) { // Only drift when moving
                    isDrifting = true;
                }
                jump(); // Jump if on ground
            } else {
                isDrifting = false;
            }
        }

    private:
        float squaredDistance(const sf::Vector2f& other) const {
            sf::Vector2f diff = position - other;
            return diff.x * diff.x + diff.y * diff.y;
        }
    };

    std::ostream& operator<<(std::ostream& os, const Car& car) {
        os << " color: " << car.color.toInteger() << " x: " << car.position.x << ", y: " << car.position.y << ", angle: " << car.angle;
        return os;
    }

    sf::Packet& operator<<(sf::Packet& packet, const Car& car) {
        return packet << car.color.toInteger() << car.position.x << car.position.y << car.angle;
    }

    sf::Packet& operator>>(sf::Packet& packet, Car& car) {
        std::uint32_t color;
        packet >> color;
        car.color = sf::Color(color);
        return packet >> car.position.x >> car.position.y >> car.angle;
    }

}