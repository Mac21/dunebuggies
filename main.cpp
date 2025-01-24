#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <cmath>
#include <deque>
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <sstream>
#include <chrono>
#include <algorithm>

#include "player_identity.hpp"
#include "player.hpp"
#include "car.hpp"
#include "game_state.hpp"
#include "menu.hpp"
#include "network_manager.hpp"

// Constants
constexpr float PI = 3.14159265358979323846f;
constexpr float CAR_RADIUS = 32.0f;

int main() {
    srand(time(NULL));

    sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "Racing Game");
    window.setFramerateLimit(60);

    sf::Texture backgroundTexture, carTexture;
    if (!backgroundTexture.loadFromFile("images/background.png") || 
        !carTexture.loadFromFile("images/car.png")) {
        return -1;  // Error loading textures
    }
    backgroundTexture.setSmooth(true);
    carTexture.setSmooth(true);

    sf::Sprite background(backgroundTexture), carSprite(carTexture);
    background.setScale({ 2.0f, 2.0f });
    carSprite.setOrigin({ 22.0f, 22.0f });

    sf::View gameView(sf::FloatRect({ 0.0f, 0.0f }, { 1920.0f, 1080.0f }));
    gameView.zoom(0.75);

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        return -1;  // Error loading font
    }

    Menu menu(font);
    sf::View menuView(menu.getFirstTextPosition(), sf::Vector2f(window.getSize()) / 2.0f);
    NetworkManager network;
    GameState gameState;
    gameState.isServer = false;
    gameState.isMultiplayer = false;
    gameState.isBotGame = false;
    Player player(sf::Vector2f(window.getSize()) / 2.0f);
    carSprite.setPosition(player.position);

    menu.setOnSelect([&](int choice) {
        switch (choice) {
            // Single Player
            case 0: {
                gameState.token_car_map.insert({ player.id, &player });
                break;
            }
            // Single Player with Bots
            case 1: {
                gameState.isBotGame = true;
                gameState.token_car_map.insert({ player.id, &player });
                for (int i = 0; i < 3; i++) { // Adding 3 bots
                    auto token = PlayerIdentity::generateToken();
                    gameState.token_car_map.insert({ token, new Car(sf::Vector2f(window.getSize().x / 2.0f + i * 50, window.getSize().y / 2.0f)) });
                }
                break;
            }
            // Host
            case 2: {
                if (network.setupServer()) {
                    std::cout << "Server started on port " << PORT << std::endl;
                    gameState.isServer = true;
                    gameState.isMultiplayer = true;
                    std::cout << "Started hosting as token: " << player.id << std::endl;
                    gameState.token_car_map.insert({ player.id, &player });
                }
                break;
            }
            // Join
            case 3: {
                if (network.connectToServer("127.0.0.1")) { // localhost for testing
                    std::cout << "Connected to server\n";
                    gameState.isMultiplayer = true;
                    std::cout << "Connected as token: " << player.id << std::endl;
                    gameState.token_car_map.insert({ player.id, &player });
                    for (auto& packet : gameState.serialize(player.id)) {
                        network.sendData(packet);
                    }
                }
                break;
            }
            // Exit
            case 4: {
                if (network.isConnected()) {
                    network.disconnect(player.id);
                }
                window.close();
                break;
            }
        }
    });

    std::chrono::steady_clock::time_point lastSyncTime = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            menu.handleInput(event);
        }

        if (gameState.isMultiplayer) {
            // Handle network events
            for (auto& packet : network.receiveData()) {
                gameState.deserialize(packet, player.id);
            }

            // Sync game state periodically
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> diff = now - lastSyncTime;
            if (diff.count() > 0.05) { // Sync every 50ms
                if (gameState.isServer) {
                    for (auto& packet : gameState.serialize(player.id)) {
                        network.broadcast(packet);
                    }
                } else {
                    for (auto& packet : gameState.serialize(player.id)) {
                        network.sendData(packet);
                    }
                }
                lastSyncTime = now;
            }
        }

        if (!menu.isVisible()) {
            player.handleInput();
            player.move();

            // Moving AI cars
            if (gameState.isBotGame) {
                for (auto& tk : gameState.token_car_map) {
                    if (tk.first == player.id) {
                        continue;
                    }

                    tk.second->move();
                    tk.second->findNextCheckpoint();
                }
            }
        }

        window.clear(sf::Color::Black);
        gameView.setCenter(player.position);
        // For clients, interpolate or predict position based on last known state
        // if () { TODO: add interpolation logic
        // }
        window.draw(background);

        for (auto& tk : gameState.token_car_map) {
            carSprite.setColor(tk.second->color);
            carSprite.setPosition(tk.second->position);
            carSprite.setRotation(sf::degrees(tk.second->angle * 180.0f / PI));
            window.draw(carSprite);
        }

        if (menu.isVisible()) {
            window.setView(menuView);
            menu.draw(window);
        }
        else {
            window.setView(gameView);
        }
        window.display();
    }

    return 0;
}