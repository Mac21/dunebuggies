#pragma once

#include <SFML/Window.hpp>
#include <SFML/System/Clock.hpp>

#include "game.hpp"

namespace db {
    class Client {
    public:
        Client() = delete;
        bool init(sf::RenderWindow& w);
        void run();
	private:
        void update();
        void render();
        void pollWindowEvents();

        sf::RenderWindow* mp_window = nullptr;
        sf::View m_gameView;
        sf::View m_menuView;
        sf::Texture m_backgroundTexture;
        sf::Texture m_carTexture;
        sf::Sprite m_carSprite;
        sf::Sprite m_background;

        Menu m_menu;
        Game m_game;
    };
}
