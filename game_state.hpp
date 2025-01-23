#pragma once

#include <SFML/Network/Packet.hpp>

enum class NetworkAction : std::uint8_t {
    Update,
    Connect,
    Disconnect,
};

sf::Packet& operator<<(sf::Packet& packet, NetworkAction action) {
    return packet << static_cast<std::uint8_t>(action);
}

sf::Packet& operator>>(sf::Packet& packet, NetworkAction& action) {
    std::uint8_t new_action;
    packet >> new_action;
    action = static_cast<NetworkAction>(new_action);
    return packet;
}

class GameState {
public:
    std::unordered_map<std::string, Car*> token_car_map;

    std::vector<sf::Packet> serialize() {
        std::vector<sf::Packet> packets;
        sf::Packet packet;
        for (auto& ct : token_car_map) {
            packet << NetworkAction::Update << ct.first << ct.second;
            packets.push_back(std::move(packet));
        }
        return packets;
    }

    void deserialize(sf::Packet& packet) {
        NetworkAction currentAction;
        if (packet >> currentAction) {
            switch (currentAction) {
                case NetworkAction::Update: {
                    std::string token;
                    if (packet >> token) {
                        if (token_car_map[token] == NULL || token_car_map[token] == nullptr) {
                            Car* car = new Car();
                            if (packet >> *car) {
                                token_car_map[token] = car;
                            } else {
                                std::cout << "Failed to deserialize car from packet" << std::endl;
                                return;
                            }
                        } else {
                            packet >> *token_car_map[token];
                        }
                        // If we don't have this token it must be a new car
                        std::cout << "Updated Car: " << token << *token_car_map[token] << std::endl;
                    } else {
                        std::cout << "Failed to deserialize packet with token: " << token << std::endl;
                    }
                    break;
                }
                case NetworkAction::Connect: {
                    break;
                }
                case NetworkAction::Disconnect: {
                    std::string token;
                    if (packet >> token) {
                        token_car_map.erase(token);
                    }
                    break;
                }
                default: {
                    std::cout << "Received an invalid GameStateNetworkAction on deserialize" << std::endl;
                    break;
                }
            }
        } else {
            std::cout << "Failed to deserialize GameStateNetworkAction" << std::endl;
        }
    }
};
