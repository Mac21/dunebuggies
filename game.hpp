#pragma once

#include <SFML/Window.hpp>
#include <unordered_map>

#include "player_identity.hpp"

#include "car.hpp"

namespace db {
	class Game {
	public:
        Game();
        bool start();
        void shutdown();

        void handleEvent(const sf::Event& event);
        void handleInput(const sf::Keyboard::Scan sc);
        void tick(float dt);
        void render();

        enum class State {
            Quit = 0,
            Inactive,
            Connecting,
            Lobby,
            Preparing,
            Playing
        };
        static std::string StateAsStr(State s);

        void setIsMultiplayer(bool b) { m_isMultiplayer = b; }
        bool isMultiplayer() const { return m_isMultiplayer; }
        void setIsServer(bool b) { m_isServer = b; }
        bool isServer() const { return m_isServer; }
        void setIsBotGame(bool b) { m_isBotGame = b; }
        bool isBotGame() const { return m_isBotGame; }

        void addPlayer(Car* p);
        std::unordered_map<player_id_t, Car*> getCars() { return m_cars; }
	private:
        bool m_isMultiplayer = false;
        bool m_isServer = false;
        bool m_isBotGame = false;
        std::unordered_map<player_id_t, Car*> m_cars;

        State m_currentState;
	};
}
