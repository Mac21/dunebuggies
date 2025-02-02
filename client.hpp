#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include "game.hpp"
#include "menu.hpp"
#include "network_manager.hpp"
#include "player.hpp"

namespace db {
    // Constants
    constexpr float PI = 3.14159265358979323846f;

    class Client {
    public:
        Client() = delete;
        Client(sf::RenderWindow&, Game*, NetworkManager*, Player*);
        void run();

        const std::string& getHostAddr() { return m_hostAddr; }
        void setHostAddr(const std::string& ha) { m_hostAddr = ha; }
        bool isReady() {
            return mp_window != nullptr &&
                   m_game != nullptr &&
                   m_menu != nullptr &&
                   m_network != nullptr;
        }
        db::NetworkManager& getNetworkManager() { return *m_network; }
	private:
        void update();
        void render();
        void pollWindowEvents();

        sf::View m_gameView;
        sf::View m_menuView;
        sf::Texture m_bgTexture;
        sf::Texture m_carTexture;

        sf::RenderWindow* mp_window = nullptr;
        sf::Sprite* m_carSprite = nullptr;
        sf::Sprite* m_bgSprite = nullptr;
        Menu* m_menu = nullptr;
        Game* m_game = nullptr;
        NetworkManager* m_network = nullptr;
        Player* m_player = nullptr;

        std::string m_hostAddr = "127.0.0.1";
    };
}
