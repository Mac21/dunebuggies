#include "menu.hpp"

db::Menu::Menu(sf::Font& font, sf::Color fill_color, sf::Color selected_color) : m_font(font), m_fill_color(fill_color), m_selected_color(selected_color) {
    m_options = { "Single Player", "Single Player with Bots", "Host", "Join", "Exit" };
    for (auto& option : m_options) {
        sf::Text text(m_font, option, 24);
        text.setFillColor(m_fill_color);
        m_texts.push_back(std::move(text));
    }
    m_selected = 0;
    updateTextPositions();
}

void db::Menu::draw(sf::RenderWindow& rw) {
    for (size_t i = 0; i < m_texts.size(); i++) {
        if (i == m_selected) {
            m_texts[i].setStyle(sf::Text::Bold | sf::Text::Underlined);
            m_texts[i].setFillColor(m_selected_color);
        }
        else {
            m_texts[i].setStyle(sf::Text::Regular);
            m_texts[i].setFillColor(m_fill_color);
        }
        rw.draw(m_texts[i]);
    }
}

void db::Menu::handleInput(std::optional<sf::Event> event) {
    if (event->is<sf::Event::KeyPressed>()) {
        auto kpe = event->getIf<sf::Event::KeyPressed>();
        switch (kpe->code) {
        case sf::Keyboard::Key::Up: {
            if (m_selected > 0) {
                m_selected--;
            }
            break;
        }
        case sf::Keyboard::Key::Down: {
            if (m_selected < m_texts.size() - 1) {
                m_selected++;
            }
            break;
        }
        case sf::Keyboard::Key::Escape: {
            hide(!m_is_hidden);
            break;
        }
        case sf::Keyboard::Key::Enter: {
            if (m_onSelect) {
                m_onSelect(m_selected);
                hide(true);
            }
            break;
        }
        }
    }
}

void db::Menu::updateTextPositions() {
    float y = 300.0f;
    for (auto& text : m_texts) {
        text.setPosition({ 500, y });
        y += 50.0f;
    }
}
