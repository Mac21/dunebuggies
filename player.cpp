#include "player.hpp"

db::Player::Player(sf::Vector2f startPos) : Car(startPos) {}

void db::Player::handleInput() {
    updateSpeed();
    updateDirection();
}

void db::Player::updateSpeed() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
        m_speed += acceleration;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
        m_speed += -brake;
    }
    else {
        m_speed += -deceleration;
    }
    m_speed = std::clamp(m_speed, 0.0f, maxSpeed);
}

void db::Player::updateDirection() {
    if (m_speed != 0.0f) {
        float speedFactor = m_speed / maxSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) m_angle += turnSpeed * speedFactor;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) m_angle -= turnSpeed * speedFactor;
    }
}
