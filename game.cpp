#include "game.hpp"
#include "car.hpp"

#include <algorithm>

db::Game::Game() : m_currentState(Game::State::Inactive) {
    m_cars = std::unordered_map<player_id_t, Car*>{};
}

bool db::Game::start() {
    return true;
}

void db::Game::shutdown() {
    return;
}

void db::Game::handleEvent(const sf::Event& e) {
    return;
}

void db::Game::handleInput(sf::Keyboard::Scan sc) {
    return;
}

void db::Game::tick(float dt) {
    return;
}

void db::Game::render() {
    return;
}

std::string db::Game::StateAsStr(State s) {
    switch (s) {
    case State::Quit: return "Quit";
    case State::Inactive: return "Inactive";
    case State::Connecting: return "Connecting";
    case State::Lobby: return "Lobby";
    case State::Preparing: return "Preparing";
    case State::Playing: return "Playing";
    }
    return "INVALID STATE";
}

bool db::Game::hasPlayer(player_id_t id) {
    const auto it = std::find_if(m_cars.begin(), m_cars.end(), [&](const auto p) { return p.first == id; });
    return it != m_cars.end();
}

void db::Game::addPlayer(Car* p) {
    if (p == nullptr) {
        return;
    }
    m_cars.insert({ p->getId(), p });
}
