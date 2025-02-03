#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/System/Angle.hpp>
#include <iostream>
#include <cmath>

#include "player_identity.hpp"

namespace db {
    // Checkpoint coordinates
    constexpr int NUM_CHECKPOINTS = 8;

    // TODO: Fix checkpoints since start position has changed to what I have deemed the start line.
    const std::vector<sf::Vector2f> CHECKPOINTS = {
        {270, 1900}, // Start line
        {500, 3300},
        {1270, 430},
        {1380, 2380},
        {1900, 2460},
        {1970, 1700},
        {2550, 1680},
        {2560, 3150},
    };

    class Car {
    public:
        Car(sf::Vector2f startPos = sf::Vector2f(CHECKPOINTS[0]), float startAngle = 0.0f, float startSpeed = 0.0f, player_id_t id = PlayerIdentity::generateToken());
        void move();
        void findNextCheckpoint();

        void setId(player_id_t i) { m_id = i; }
        virtual player_id_t getId() { return m_id; }
        void setColor(sf::Color c) { m_color = c; }
        sf::Color getColor() const { return m_color; }
        void setPos(float x, float y) {
            m_position.x = x;
            m_position.y = y;
        }
        sf::Vector2f getPos() const { return m_position; }
        void setAngle(float a) { m_angle = a; }
        float getAngle() const { return m_angle; }
    protected:
        sf::Vector2f m_position;
        sf::Color m_color;
        float m_speed = 0.0f;
        float m_angle = 0.0f;
        int m_currentCheckpoint = 0;
        player_id_t m_id;
    private:
        float squaredDistance(const sf::Vector2f& other) const;
    };

    static std::ostream& operator<<(std::ostream& os, const Car& car) {
        os << " color: " << car.getColor().toInteger() << " x: " << car.getPos().x << ", y: " << car.getPos().y << ", angle: " << car.getAngle();
        return os;
    }

    static sf::Packet& operator<<(sf::Packet& packet, const Car& car) {
        return packet << car.getColor().toInteger() << car.getPos().x << car.getPos().y << car.getAngle();
    }

    static sf::Packet& operator>>(sf::Packet& packet, Car& car) {
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
