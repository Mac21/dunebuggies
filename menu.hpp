#pragma once

class Menu {
public:
    Menu(sf::Font& font) : font(font) {
        options = {"Single Player", "Single Player with Bots", "Host", "Join", "Exit"};
        for (auto& option : options) {
            sf::Text text(font, option, 24);
            text.setFillColor(sf::Color::White);
            texts.push_back(std::move(text));
        }
        selected = 0;
        updateTextPositions();
    }

    void draw(sf::RenderWindow& window) {
        for (size_t i = 0; i < texts.size(); ++i) {
            texts[i].setFillColor(i == selected ? sf::Color::Red : sf::Color::White);
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
        return texts[0].getPosition();
    }

private:
    void updateTextPositions() {
        float y = 200.0f;
        for (auto& text : texts) {
			text.setPosition({ 400, y });
			y += 50.0f;
        }
    }

    bool is_hidden;
    sf::Font& font;
    std::vector<std::string> options;
    std::vector<sf::Text> texts;
    size_t selected;
    std::function<void(int)> onSelect;
};
