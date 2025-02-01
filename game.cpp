#include "game.hpp"

db::Game::Game() {
    m_cars = std::unordered_map<car_id_t, Car*>{};
}
