#include "car.hpp"

db::Car::Car(sf::Vector2f startPos, float startAngle, float startSpeed, player_id_t id) : m_position(startPos),
                                                                                          m_speed(startSpeed),
                                                                                          m_angle(startAngle),
                                                                                          m_currentCheckpoint(0),
                                                                                          m_color(rand() % 255, rand() % 255, rand() % 255),
                                                                                          m_id(id) {}

void db::Car::move() {
    m_position.x += std::sin(m_angle) * m_speed;
    m_position.y -= std::cos(m_angle) * m_speed;
#ifdef DEBUG
    std::cout << "ID: " << std::to_string(m_id) << " X: " << m_position.x << " Y: " << m_position.y << std::endl;
#endif // DEBUG
}

void db::Car::findNextCheckpoint() {
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

float db::Car::squaredDistance(const sf::Vector2f& other) const {
    sf::Vector2f diff = m_position - other;
    return diff.x * diff.x + diff.y * diff.y;
}
