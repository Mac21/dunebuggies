#include "Client.hpp"

bool db::Client::init(sf::RenderWindow& window) {
    srand(time(NULL));
    mp_window = &window;

    if (!m_backgroundTexture.loadFromFile("images/background.png") || !m_carTexture.loadFromFile("images/car.png")) {
        return false;  // Error loading textures
    }
    m_backgroundTexture.setSmooth(true);
    m_carTexture.setSmooth(true);

    sf::Sprite m_background(m_backgroundTexture), carSprite(m_carTexture);
    m_background.setScale({ 2.0f, 2.0f });
    m_carSprite.setOrigin({ 22.0f, 22.0f });

    sf::View m_gameView(sf::FloatRect({ 0.0f, 0.0f }, { 1920.0f, 1080.0f }));
    m_gameView.zoom(0.75);

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        return false;  // Error loading font
    }

    Menu m_menu(font);
    sf::View m_menuView(m_menu.getFirstTextPosition(), sf::Vector2f(window.getSize()) / 2.0f);
    Player player(sf::Vector2f(window.getSize()) / 2.0f);
    m_carSprite.setPosition(player.getPos());

    m_menu.setOnSelect([&](int choice) {
        switch (choice) {
            // Single Player
            case 0: {
                m_game.addPlayer(&player);
                break;
            }
            // Single Player with Bots
            case 1: {
                m_game.setIsBotGame(true);
                m_game.addPlayer(&player);
                for (int i = 0; i < 3; i++) { // Adding 3 bots
                    m_game.addPlayer(new Car(sf::Vector2f(window.getSize().x / 2.0f + i * 50, window.getSize().y / 2.0f), 0.0f, 12.0f, PlayerIdentity::generateToken()));
                }
                break;
            }
            // Host
            case 2: {
                if (m_game.getNetworkManager().setupServer()) {
                    std::cout << "Server started on port " << PORT << std::endl;
                    m_game.setIsServer(true);
                    m_game.setIsMultiplayer(true);
                    std::cout << "Started hosting as token: " << player.getId() << std::endl;
                    m_game.addPlayer(&player);
                }
                break;
            }
            // Join
            case 3: {
                auto nm = m_game.getNetworkManager();
                if (nm.connectToServer("127.0.0.1")) { // localhost for testing
                    std::cout << "Connected to server\n";
                    m_game.setIsMultiplayer(true);
                    std::cout << "Connected as token: " << player.getId() << std::endl;
                    m_game.addPlayer(&player);
                    for (auto& packet : m_game.getGameState().serialize(player.getId())) {
                        nm.sendData(packet);
                    }
                }
                break;
            }
            // Exit
            case 4: {
                auto nm = m_game.getNetworkManager();
                if (nm.isConnected()) {
                    nm.disconnect(player.getId());
                }
                mp_window->close();
                break;
            }
        }
    });
    return true;
}

void db::Client::run() {
    std::chrono::steady_clock::time_point lastSyncTime = std::chrono::steady_clock::now();
    while (mp_window->isOpen() && mp_window->hasFocus()) {
        pollWindowEvents();
        if (m_game.isMultiplayer()) {
            // Handle network events
            for (auto& packet : m_game.getNetworkManager().receiveData()) {
                m_game.getGameState().deserialize(packet);
            }

            // Sync game state periodically
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> diff = now - lastSyncTime;

            if (diff.count() > 0.05) { // Sync every 50ms
                if (m_game.isServer()) {
                    for (auto& packet : m_game.getGameState().serialize()) {
                        m_game.getNetworkManager().broadcast(packet);
                    }
                } else {
                    for (auto& packet : m_game.getGameState().serialize()) {
                        m_game.getNetworkManager().sendData(packet);
                    }
                }
                lastSyncTime = now;
            }
        }

        if (!m_menu.isVisible()) {
            player.handleInput();
            player.move();

            // Invisible barrier for car driving outside of background image
            auto gb = m_background.getGlobalBounds();
            if (player.getPos().x >= gb.size.x) {
                player.getPos().x = gb.size.x;
            } else if (player.getPos().x < 0) {
                player.getPos().x = 0;
            }
            if (player.getPos().y >= gb.size.y) {
                player.getPos().y = gb.size.y;
            } else if (player.getPos().y < 0) {
                player.getPos().y = 0;
            }

            m_gameView.setCenter(player.getPos());

            // Moving AI cars
            if (m_game.isBotGame()) {
                for (auto& tk : m_game.getCars()) {
                    if (tk.first == player.getId()) {
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
            if (m_game.getNetworkManager().isConnected()) {
                m_game.getNetworkManager().disconnect();
            }
            mp_window->close();
            return;
        }
        m_menu.handleInput(event);
    }
}

void db::Client::update() {
    static sf::Clock clock;
    m_game.tick(clock.restart().asSeconds());
    m_menu.update();
}

void db::Client::render() {
        mp_window->clear();

        sf::Vector2f halfGameViewSize = m_gameView.getSize() / 2.0f;
        float backgroundWidth = m_background.getGlobalBounds().size.x;
        float backgroundHeight = m_background.getGlobalBounds().size.y;
        
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
        mp_window->draw(m_background);

        for (auto& tk : m_game.getCars()) {
            m_carSprite.setColor(tk.second->getColor());
            m_carSprite.setPosition(tk.second->getPos());
            m_carSprite.setRotation(sf::degrees(tk.second->getAngle() * 180.0f / PI));
            mp_window->draw(m_carSprite);
        }

        if (m_menu.isVisible()) {
            mp_window->setView(m_menuView);
            m_menu.draw(mp_window);
        } else {
            mp_window->setView(m_gameView);
        }
        mp_window->display();
}
