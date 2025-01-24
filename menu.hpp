#pragma once

namespace db {
    class Menu {
    public:
        Menu(sf::Font& font, sf::Color fill_color = sf::Color::White, sf::Color selected_color = sf::Color::Black) : font(font),
                                                                                                                     fill_color(fill_color),
                                                                                                                     selected_color(selected_color) {
            options = { "Single Player", "Single Player with Bots", "Host", "Join", "Exit" };
            for (auto& option : options) {
                sf::Text text(font, option, 24);
                text.setFillColor(fill_color);
                texts.push_back(std::move(text));
            }
            selected = 0;
            updateTextPositions();
        }

        void draw(sf::RenderWindow& window) {
            for (size_t i = 0; i < texts.size(); i++) {
                if (i == selected) {
                    texts[i].setStyle(sf::Text::Bold | sf::Text::Underlined);
                    texts[i].setFillColor(selected_color);
                } else {
                    texts[i].setStyle(sf::Text::Regular);
                    texts[i].setFillColor(fill_color);
                }
                window.draw(texts[i]);
            }
        }

        bool isVisible() const {
            return !is_hidden;
        }

        void hide(bool yn) {
            is_hidden = yn;
        }

        void handleInput(std::optional<sf::Event> event) {
            if (event->is<sf::Event::KeyPressed>()) {
                auto kpe = event->getIf<sf::Event::KeyPressed>();
                switch (kpe->code) {
                    case sf::Keyboard::Key::Up: {
                        if (selected > 0) {
                            selected--;
                        }
                        break;
                    }
                    case sf::Keyboard::Key::Down: {
                        if (selected < texts.size() - 1) {
                            selected++;
                        }
                        break;
                    }
                    case sf::Keyboard::Key::Escape: {
                        hide(!is_hidden);
                        break;
                    }
                    case sf::Keyboard::Key::Enter: {
                        if (onSelect) {
                            onSelect(selected);
                            hide(true);
                        }
                        break;
                    }
                }
            }
        }

        void setOnSelect(std::function<void(int)> callback) {
            onSelect = callback;
        }

        sf::Vector2f getFirstTextPosition() {
            return texts[1].getPosition();
        }

    private:
        void updateTextPositions() {
            float y = 300.0f;
            for (auto& text : texts) {
                text.setPosition({ 500, y });
                y += 50.0f;
            }
        }

        sf::Color fill_color;
        sf::Color selected_color;
        bool is_hidden;
        sf::Font& font;
        std::vector<std::string> options;
        std::vector<sf::Text> texts;
        size_t selected;
        std::function<void(int)> onSelect;
    };
}