#include "client.hpp"

#include "network_manager.hpp"
#include "menu.hpp"
#include "player.hpp"
#include "car.hpp"

db::Client::Client(sf::RenderWindow& window, Game* g, NetworkManager* nm, Player* p) : m_game(g), m_network(nm), m_player(p) {
    if (!m_bgTexture.loadFromFile("assets/images/background.png") || !m_carTexture.loadFromFile("assets/images/car.png")) {
        std::cout << "Failed to load asset image" << std::endl;
        return;  // Error loading textures
    }

    sf::Font font;
    if (!font.openFromFile("assets/fonts/arial.ttf")) {
        std::cout << "Failed to load font asset." << std::endl;
        return;  // Error loading font
    }

    m_bgTexture.setSmooth(true);
    m_carTexture.setSmooth(true);

    m_bgSprite = new sf::Sprite(m_bgTexture);
    m_carSprite = new sf::Sprite(m_carTexture);
    m_bgSprite->setScale({ 2.0f, 2.0f });
    m_carSprite->setOrigin({ 22.0f, 22.0f });

    m_gameView = sf::View(sf::FloatRect({ 0.0f, 0.0f }, { 1920.0f, 1080.0f }));
    m_gameView.zoom(0.75);

    m_menu = new Menu(font);

    m_menuView = sf::View(m_menu->getFirstTextPosition(), sf::Vector2f(window.getSize()) / 2.0f);
    m_carSprite->setPosition(m_player->getPos());

    m_menu->setOnSelect([&](size_t choice) {
        switch (choice) {
            // Single Player
            case 0: {
                m_game->addPlayer(m_player);
                break;
            }
            // Single Player with Bots
            case 1: {
                m_game->setIsBotGame(true);
                m_game->addPlayer(m_player);
                for (int i = 0; i < 3; i++) { // Adding 3 bots
                    m_game->addPlayer(new Car(sf::Vector2f(window.getSize().x / 2.0f + i * 50, window.getSize().y / 2.0f), 0.0f, 12.0f, PlayerIdentity::generateToken()));
                }
                break;
            }
            // Host
            case 2: {
                if (m_network->setupServer()) {
                    std::cout << "Server started on port " << PORT << std::endl;
                    m_game->setIsServer(true);
                    m_game->setIsMultiplayer(true);
                    std::cout << "Started hosting as token: " << m_player->getId() << std::endl;
                    m_game->addPlayer(m_player);
                }
                break;
            }
            // Join
            case 3: {
                if (m_network->connectToServer("127.0.0.1")) { // localhost for testing
                    std::cout << "Connected to server\n";
                    m_game->setIsMultiplayer(true);
                    std::cout << "Connected as token: " << m_player->getId() << std::endl;
                    m_game->addPlayer(m_player);
                    for (auto& packet : m_network->serialize(*m_game, m_player->getId())) {
                        m_network->sendData(packet);
                    }
                }
                break;
            }
            // Exit
            case 4: {
                if (m_network->isConnected()) {
                    m_network->disconnect(m_player->getId());
                }
                mp_window->close();
                break;
            }
        }
    });

    mp_window = &window;
}

void db::Client::run() {
    std::chrono::steady_clock::time_point lastSyncTime = std::chrono::steady_clock::now();
    while (mp_window->isOpen() && mp_window->hasFocus()) {
        pollWindowEvents();
        if (m_game->isMultiplayer()) {
            // Handle network events
            for (auto& packet : m_network->receiveData()) {
                m_network->deserialize(packet, *m_game, m_player->getId());
            }

            // Sync game state periodically
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> diff = now - lastSyncTime;

            if (diff.count() > 0.05) { // Sync every 50ms
                if (m_game->isServer()) {
                    for (auto& packet : m_network->serialize(*m_game, m_player->getId())) {
                        m_network->broadcast(packet);
                    }
                } else {
                    for (auto& packet : m_network->serialize(*m_game, m_player->getId())) {
                        m_network->sendData(packet);
                    }
                }
                lastSyncTime = now;
            }
        }

        if (!m_menu->isVisible()) {
            m_player->handleInput();
            m_player->move();

            // Invisible barrier for car driving outside of background image
            auto gb = m_bgSprite->getGlobalBounds();
            auto cp = m_player->getPos();
            if (cp.x >= gb.size.x) {
                m_player->setPos(gb.size.x, cp.y);
            } else if (cp.x < 0) {
                m_player->setPos(0, cp.y);
            }
            if (cp.y >= gb.size.y) {
                m_player->setPos(cp.x, gb.size.y);
            } else if (m_player->getPos().y < 0) {
                m_player->setPos(cp.x, 0);
            }

            m_gameView.setCenter(m_player->getPos());

            // Moving AI cars
            if (m_game->isBotGame()) {
                for (auto& tk : m_game->getCars()) {
                    if (tk.first == m_player->getId()) {
                        continue;
                    }

                    tk.second->move();
                    tk.second->findNextCheckpoint();
                }
            }
        }
        update();
        render();
    }
}

void db::Client::pollWindowEvents() {
    while (const std::optional event = mp_window->pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            if (m_network->isConnected()) {
                m_network->disconnect(m_player->getId());
            }
            mp_window->close();
            return;
        }
        m_menu->handleInput(event);
    }
}

void db::Client::update() {
    static sf::Clock clock;
    m_game->tick(clock.restart().asSeconds());
    // m_menu.update();
}

void db::Client::render() {
        mp_window->clear();

        sf::Vector2f halfGameViewSize = m_gameView.getSize() / 2.0f;
        float backgroundWidth = m_bgSprite->getGlobalBounds().size.x;
        float backgroundHeight = m_bgSprite->getGlobalBounds().size.y;
        
        // Limit camera movement along the x-axis
        if (m_gameView.getCenter().x - halfGameViewSize.x < 0) { // Left edge of the screen
            m_gameView.move({ halfGameViewSize.x - m_gameView.getCenter().x, 0 });
        } else if (m_gameView.getCenter().x + halfGameViewSize.x > backgroundWidth) {
            m_gameView.move({ backgroundWidth - halfGameViewSize.x - m_gameView.getCenter().x, 0 });
        }
          
        // Limit camera movement along the y-axis
        if (m_gameView.getCenter().y - halfGameViewSize.y < 0) {
            m_gameView.move({ 0, halfGameViewSize.y - m_gameView.getCenter().y });
        } else if (m_gameView.getCenter().y + halfGameViewSize.y > backgroundHeight) {
            m_gameView.move({ 0, backgroundHeight - halfGameViewSize.y - m_gameView.getCenter().y });
        }
        mp_window->draw(*m_bgSprite);

        for (auto& tk : m_game->getCars()) {
            m_carSprite->setColor(tk.second->getColor());
            m_carSprite->setPosition(tk.second->getPos());
            m_carSprite->setRotation(sf::degrees(tk.second->getAngle() * 180.0f / PI));
            mp_window->draw(*m_carSprite);
        }

        if (m_menu->isVisible()) {
            mp_window->setView(m_menuView);
            m_menu->draw(*mp_window);
        } else {
            mp_window->setView(m_gameView);
        }
        mp_window->display();
}
