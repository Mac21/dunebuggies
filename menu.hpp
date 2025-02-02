#pragma once

#include <functional>

#include <SFML/Graphics.hpp>

namespace db {
    class Menu {
    public:
        Menu() = delete;
        Menu(sf::Font& font, sf::Color fill_color = sf::Color::White, sf::Color selected_color = sf::Color::Black);

        void draw(sf::RenderWindow& window);
        void hide(bool yn) { m_is_hidden = yn; }
        void handleInput(std::optional<sf::Event> event);

        void setOnSelect(std::function<void(size_t)> callback) { m_onSelect = callback; }
        bool isVisible() const { return !m_is_hidden; }
        sf::Vector2f getFirstTextPosition() { return m_texts[1].getPosition(); }
    private:
        void updateTextPositions();

        bool m_is_hidden;
        sf::Color m_fill_color;
        sf::Color m_selected_color;

        sf::Font m_font;
        std::vector<std::string> m_options;
        std::vector<sf::Text> m_texts;
        size_t m_selected;
        std::function<void(size_t)> m_onSelect;
    };
}