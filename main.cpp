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

// Constants
constexpr int NUM_CHECKPOINTS = 8;
constexpr float PI = 3.14159265358979323846f;
constexpr float CAR_RADIUS = 32.0f;
constexpr int PORT = 9075;
constexpr int MAX_CLIENTS = 4;

// Checkpoint coordinates
const std::vector<sf::Vector2f> CHECKPOINTS = {
    {300, 610}, {1270, 430}, {1380, 2380}, {1900, 2460},
    {1970, 1700}, {2550, 1680}, {2560, 3150}, {500, 3300}
};

// Helper for serialization/deserialization
std::string gSerialize(sf::Vector2f vec) {
    return std::to_string(vec.x) + "," + std::to_string(vec.y);
}

sf::Vector2f gDeserialize(std::string str) {
    std::istringstream iss(str);
    float x, y;
    char comma;
    iss >> x >> comma >> y;
    return {x, y};
}

// Car class
class Car {
public:
    sf::Vector2f position;
    float speed, angle;
    int currentCheckpoint;

    Car(sf::Vector2f startPos = sf::Vector2f(0, 0), float startAngle = 0.0f) 
        : position(startPos), speed(12.0f), angle(startAngle), currentCheckpoint(0) {}

    void move() {
        position.x += std::sin(angle) * speed;
        position.y -= std::cos(angle) * speed;
    }

    void findNextCheckpoint() {
        const auto& target = CHECKPOINTS[currentCheckpoint];
        float beta = angle - std::atan2(target.x - position.x, -target.y + position.y);

        if (std::sin(beta) < 0)
            angle += 0.005f * speed;
        else
            angle -= 0.005f * speed;

        if (squaredDistance(target) < 625.0f)  // 25 * 25
            currentCheckpoint = (currentCheckpoint + 1) % NUM_CHECKPOINTS;
    }

private:
    float squaredDistance(const sf::Vector2f& other) const {
        sf::Vector2f diff = position - other;
        return diff.x * diff.x + diff.y * diff.y;
    }
};

// Player class
class Player : public Car {
public:
    const float maxSpeed = 12.0f;
    const float acceleration = 0.2f;
    const float deceleration = 0.3f;
    const float turnSpeed = 0.08f;

    Player(sf::Vector2f startPos = sf::Vector2f(0, 0)) : Car(startPos) {}

    void handleInput() {
        updateSpeed();
        updateDirection();
    }

private:
    void updateSpeed() {
        float delta = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ? acceleration 
                    : (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ? -acceleration : -deceleration);
        speed += speed < 0 ? -delta : delta;  
        speed = std::clamp(speed, -maxSpeed, maxSpeed);
    }

    void updateDirection() {
        if (speed != 0.0f) {
            float speedFactor = speed / maxSpeed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) angle += turnSpeed * speedFactor;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) angle -= turnSpeed * speedFactor;
        }
    }
};

// Menu class
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
            case sf::Keyboard::Key::Up:
                if (selected > 0) selected--;
                break;
            case sf::Keyboard::Key::Down:
                if (selected < texts.size() - 1) selected++;
                break;
            case sf::Keyboard::Key::Escape:
                hide(!is_hidden);
                break;
            case sf::Keyboard::Key::Enter:
                if (onSelect) {
                    onSelect(selected);
                    hide(true);
                }
                break;
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

// Network Manager for both client and server operations
class NetworkManager {
public:
    NetworkManager() : connected(false), buffer() {}

    bool setupServer() {
        if (listener.listen(PORT) != sf::Socket::Status::Done) return false;
        listener.setBlocking(false); // Non-blocking for multiple clients
        return true;
    }

    bool connectToServer(const std::string& host) {
        if (socket.connect(*sf::IpAddress::resolve(host), PORT, sf::seconds(5)) != sf::Socket::Status::Done) return false;
        connected = true;
        return true;
    }

    void sendData(const std::string& data) {
        std::cout << "Sending data: " << data << std::endl;
        if (connected) {
            auto status = socket.send(data.c_str(), data.size());
            if (status != sf::Socket::Status::Done) {
                std::cout << "Failed to send socket data with status: " << int(status) << std::endl;
            }
        }
    }

    std::vector<std::string> receiveData() {
        std::vector<std::string> data;
        if (connected) {
            char buffer[1024];
            std::size_t received;
            if (socket.receive(buffer, sizeof(buffer), received) == sf::Socket::Status::Done) {
                data.emplace_back(buffer, received);
            }
        } else { // Server mode, check for new connections and data from clients
            sf::TcpSocket newClient;
            newClient.setBlocking(false);
            if (listener.accept(newClient) == sf::Socket::Status::Done) {
                clients.push_back(std::move(newClient));
                std::cout << "New client connected. Total clients: " << clients.size() << std::endl;
            }
            std::size_t received;
            for (auto& client : clients) {
                if (client.receive(buffer, sizeof(buffer), received) == sf::Socket::Status::Done) {
                    data.emplace_back(buffer, received);
                }
            }
        }
        if (data.data() != NULL) {
            std::cout << "Received data: " << data.data()->c_str() << std::endl;
        }
        return data;
    }

    bool isConnected() const { return connected; }

    void broadcast(const std::string& data) {
        std::cout << "Broadcasting data: " << data << std::endl;
        for (auto& client : clients) {
            auto status = client.send(data.c_str(), data.size());
            if (status != sf::Socket::Status::Done) {
                std::cout << "Failed to send broadcast data" << std::endl;
            }
        }
    }

private:
    sf::TcpListener listener;
    sf::TcpSocket socket;
    std::vector<sf::TcpSocket> clients;
    char buffer[1024];
    bool connected;
};

// Simple security for player identification
class PlayerIdentity {
public:
    std::string static generateToken() {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        std::string token;
        for (int i = 0; i < 16; i++) {
            size_t len = (sizeof(alphanum) / sizeof(alphanum[0]));
            token += alphanum[rand() % len];
        }
        return token;
    }
};

// Game State for synchronization
class GameState {
public:
    std::unordered_map<std::string, Car*> cars;
    // std::vector<Car*> cars;
    std::vector<std::string> playerTokens; // Link cars to players for identification

    std::string serialize() {
        std::stringstream ss;
        for (size_t i = 0; i < playerTokens.size(); ++i) {
            ss << playerTokens[i] << ":" << gSerialize(cars[playerTokens[i]]->position) << ";" << std::to_string(cars[playerTokens[i]]->angle) << ";";
        }
        return ss.str();
    }

    void deserialize(const std::string& data) {
        std::istringstream iss(data);
        std::string token, posStr, angleStr;
        while (std::getline(iss, token, ':') && std::getline(iss, posStr, ';') && std::getline(iss, angleStr, ';')) {
            std::cout << "Updated car " << token << " state with pos: " << posStr << " angle: " << angleStr << std::endl;
            // If we don't have this token it must be a new car
            auto newPos = gDeserialize(posStr);
            auto newAngle = std::stof(angleStr);
            if (cars[token] == NULL) {
                cars[token] = new Car(newPos, newAngle);
            } else {
                cars[token]->position = newPos;
                cars[token]->angle = newAngle;
            }
        }
    }
};

int main() {
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

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        return -1;  // Error loading font
    }

    Menu menu(font);
    sf::View menuView(menu.getFirstTextPosition(), sf::Vector2f(window.getSize()) / 2.0f);
    NetworkManager network;
    GameState gameState;
    Player player(sf::Vector2f(window.getSize()) / 2.0f);
    carSprite.setPosition(player.position);

    bool isServer = false;
    bool isMultiplayer = false;
    bool isBotGame = false;

    menu.setOnSelect([&](int choice) {
        switch (choice) {
            // Single Player
            case 0: {
                gameState.playerTokens.emplace_back(PlayerIdentity::generateToken());
                gameState.cars.insert({ gameState.playerTokens.back() , &player });
                break;
            }
            // Single Player with Bots
            case 1: {
                isBotGame = true;
                gameState.playerTokens.emplace_back(PlayerIdentity::generateToken());
                gameState.cars.insert({ gameState.playerTokens.back() , &player });
                for (int i = 0; i < 3; i++) { // Adding 3 bots
                    gameState.playerTokens.emplace_back(PlayerIdentity::generateToken());
                    gameState.cars.insert({ gameState.playerTokens.back(), new Car(sf::Vector2f(window.getSize().x / 2.0f + i * 50, window.getSize().y / 2.0f)) });
                }
                break;
            }
            // Host
            case 2: {
                if (network.setupServer()) {
                    std::cout << "Server started on port " << PORT << std::endl;
                    isServer = true;
                    isMultiplayer = true;
                    gameState.playerTokens.emplace_back(PlayerIdentity::generateToken());
                    gameState.cars.emplace(gameState.playerTokens.back(), &player);
                }
                break;
            }
            // Join
            case 3: {
                if (network.connectToServer("127.0.0.1")) { // localhost for testing
                    std::cout << "Connected to server\n";
                    isMultiplayer = true;
                    gameState.playerTokens.emplace_back(PlayerIdentity::generateToken());
                    gameState.cars.emplace(gameState.playerTokens.back(), &player);
                    network.sendData("JOIN:" + gameState.playerTokens.back() + ":" + gSerialize(player.position) + ";" + std::to_string(player.angle) + ";");
                }
                break;
            }
            case 4: {
                window.close();
                break;
            }
        }
    });

    std::chrono::steady_clock::time_point lastSyncTime = std::chrono::steady_clock::now();

    while (window.isOpen()) {
        srand(time(NULL));
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            menu.handleInput(event);
        }

        if (isMultiplayer) {
            // Handle network events
            auto data = network.receiveData();
            for (const auto& d : data) {
                if (isServer) {
                    if (d.substr(0, 5) == "JOIN:") {
                        gameState.deserialize(d.substr(5, d.length()));
                    } else {
                        gameState.deserialize(d);
                    }
                } else {
                    gameState.deserialize(d);
                }
            }

            // Sync game state periodically
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> diff = now - lastSyncTime;
            if (diff.count() > 0.1) { // Sync every 100ms
                if (isServer) {
                    network.broadcast(gameState.serialize());
                } else {
                    network.sendData(gameState.serialize());
                }
                lastSyncTime = now;
            }
        }

        if (!menu.isVisible()) {
            player.handleInput();
            player.move();

            // Moving AI cars
            if (isBotGame) {
                for (auto& car : gameState.cars) {
                    if (car.second == &player) {
                        continue;
                    }

                    car.second->move();
                    car.second->findNextCheckpoint();
                }
            }
        }

        window.clear(sf::Color::Black);
        gameView.setCenter(player.position);
        // For clients, interpolate or predict position based on last known state
        // if () { TODO: add interpolation logic
        // }
        window.draw(background);

        std::vector<sf::Color> colors = { 
            sf::Color::Green,
            sf::Color::Blue,
            sf::Color::Yellow,
            sf::Color::Red
        };

        size_t i = 0;
        for (auto ct : gameState.cars) {
            carSprite.setColor(colors[i++ % colors.size()]);
            carSprite.setPosition(ct.second->position);
            carSprite.setRotation(sf::degrees(ct.second->angle * 180.0f / PI));
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