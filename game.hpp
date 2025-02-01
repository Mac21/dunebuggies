#pragma once

#include <unordered_map>

#include "player_identity.hpp"
#include "player.hpp"
#include "car.hpp"
#include "game_state.hpp"
#include "menu.hpp"
#include "network_manager.hpp"

namespace db {
	class Game {
	public:
        bool start();
        void shutdown();

        void handleEvent(const sf::Event& event);
        void handleInput(const Keyboard& keyboard);
        void tick(float dt);
        void render();

        void setIsMultiplayer(bool b) { m_isMultiplayer = b; }
        bool isMultiplayer() const { return m_isMultiplayer; }
        void setIsServer(bool b) { m_isServer = b; }
        bool isServer() const { return m_isServer; }
        void setIsBotGame(bool b) { m_isBotGame = b; }
        bool isBotGame() const { return m_isBotGame; }

        int addPlayer(Car* p);

        std::unordered_map<car_id_t, Car*> getCars() const { return m_cars; }

        GameState getGameState() { return m_gameState; }
        NetworkManager getNetworkManager() { return m_network; }
	private:
        bool m_isMultiplayer = false;
        bool m_isServer = false;
        bool m_isBotGame = false;
        std::unordered_map<car_id_t, Car*> m_cars;

        NetworkManager m_network;
        GameState m_gameState;
	};
}
