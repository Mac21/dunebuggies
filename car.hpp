#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Graphics/Color.hpp>

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

    Car(sf::Vector2f startPos = sf::Vector2f(0, 0), float startAngle = 0.0f) 
        : position(startPos),
          speed(0.0f),
          angle(startAngle),
          currentCheckpoint(0),
          color(rand() % 255, rand() % 255, rand() % 255) {}

    void move() {
        position.x += std::sin(angle) * speed;
        position.y -= std::cos(angle) * speed;
    }

    void findNextCheckpoint() {
        const auto& target = CHECKPOINTS[currentCheckpoint];
        float beta = angle - std::atan2(target.x - position.x, -target.y + position.y);

        if (std::sin(beta) < 0)
            angle += 0.005f * speed;
        else
            angle -= 0.005f * speed;

        if (squaredDistance(target) < 625.0f)  // 25 * 25
            currentCheckpoint = (currentCheckpoint + 1) % NUM_CHECKPOINTS;
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
