#pragma once

#include <SFML/System/Angle.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/Graphics/Color.hpp>

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
        friend class Player;

        Car(sf::Vector2f startPos = sf::Vector2f(0, 0), float startAngle = 0.0f, float startSpeed = 0.0f, car_id_t id)
            : m_position(startPos),
            m_speed(startSpeed),
            m_angle(startAngle),
            m_currentCheckpoint(0),
            m_color(rand() % 255, rand() % 255, rand() % 255),
            m_id(id) {}

        void move() {
            m_position.x += std::sin(m_angle) * m_speed;
            m_position.y -= std::cos(m_angle) * m_speed;
        }

        void findNextCheckpoint() {
            const auto& target = CHECKPOINTS[m_currentCheckpoint];
            float beta = m_angle - std::atan2(target.x - m_position.x, -target.y + m_position.y);

            if (std::sin(beta) < 0) {
                m_angle += 0.005f * m_speed;
            }
            else {
                m_angle -= 0.005f * m_speed;
            }

            if (squaredDistance(target) < 625.0f) { // 25 * 25
                m_currentCheckpoint = (m_currentCheckpoint + 1) % NUM_CHECKPOINTS;
            }
        }

        void setId(car_id_t i) { m_id = i; }
        car_id_t getId() { return m_id; }
        void setColor(sf::Color c) { m_color = c; }
        sf::Color getColor() const { return m_color; }
        void setPos(float x, float y) {
            m_position.x = x;
            m_position.y = y;
        }
        sf::Vector2f getPos() const { return m_position; }
        void setAngle(float a) { m_angle = a; }
        float getAngle() const { return m_angle; }
    private:
        float squaredDistance(const sf::Vector2f& other) const {
            sf::Vector2f diff = m_position - other;
            return diff.x * diff.x + diff.y * diff.y;
        }

        sf::Vector2f m_position;
        sf::Color m_color;
        float m_speed = 0.0f;
        float m_angle = 0.0f;
        int m_currentCheckpoint = 0;
        car_id_t m_id;
    };

    std::ostream& operator<<(std::ostream& os, const Car& car) {
        os << " color: " << car.getColor().toInteger() << " x: " << car.getPos().x << ", y: " << car.getPos().y << ", angle: " << car.getAngle();
        return os;
    }

    sf::Packet& operator<<(sf::Packet& packet, const Car& car) {
        return packet << car.getColor().toInteger() << car.getPos().x << car.getPos().y << car.getAngle();
    }

    sf::Packet& operator>>(sf::Packet& packet, Car& car) {
        std::uint32_t color;
        float pos_x;
        float pos_y;
        float angle;
        packet >> color >> pos_x >> pos_y >> angle;
        car.setColor(sf::Color(color));
        car.setPos(pos_x, pos_y);
        car.setAngle(angle);
        return packet;
    }

}