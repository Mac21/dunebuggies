#pragma once

class GameState {
public:
    std::unordered_map<std::string, Car*> cars;
    std::vector<std::string> playerTokens; // Link cars to players for identification

    std::vector<sf::Packet> serialize() {
        std::vector<sf::Packet> packets;
        sf::Packet packet;
        for (size_t i = 0; i < playerTokens.size(); ++i) {
            packet << playerTokens[i] << *cars[playerTokens[i]];
            packets.push_back(std::move(packet));
        }
        return packets;
    }

    void deserialize(sf::Packet& packet) {
        std::string token;

        if (packet >> token) {
            if (cars[token] == NULL) {
                Car* car = new Car();
                packet >> *car;
                cars[token] = car;
            } else {
                packet >> *cars[token];
            }
            // If we don't have this token it must be a new car
            std::cout << "Updated car " << token << *cars[token] << std::endl;
        } else {
            std::cout << "Failed to deserialize packet with token: " << token << std::endl;
        }
    }
};
