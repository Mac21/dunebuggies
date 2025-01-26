#pragma once

#include <SFML/Network/Packet.hpp>

namespace db {
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

        GameState() : isMultiplayer(false), isServer(false), isBotGame(false) {}

        std::vector<sf::Packet> serialize(std::string player_id) {
            std::vector<sf::Packet> packets;
            sf::Packet packet;
            if (isServer) {
                for (auto& ct : token_car_map) {
                    packet << NetworkAction::Update << ct.first << *ct.second;
                    packets.push_back(std::move(packet));
                }
            }
            else {
                packet << NetworkAction::Update << player_id << *token_car_map[player_id];
                packets.push_back(std::move(packet));
            }
            return packets;
        }

        void deserialize(sf::Packet& packet, std::string player_id) {
            NetworkAction currentAction;
            std::string token;

            if (!(packet >> currentAction >> token)) {
                return;
            }

            switch (currentAction) {
                case NetworkAction::Update: {
                    // Don't update ourself
                    if (token == player_id) {
                        return;
                    }

                    if (token_car_map[token] == NULL || token_car_map[token] == nullptr) {
                        Car* car = new Car();
                        packet >> *car;
                        token_car_map[token] = car;
                    }
                    else {
                        packet >> *token_car_map[token];
                    }
                    // If we don't have this token it must be a new car
                    std::cout << "Updated Car: " << token << *token_car_map[token] << std::endl;
                    break;
                }
                case NetworkAction::Connect: {
                    break;
                }
                case NetworkAction::Disconnect: {
                    token_car_map.erase(token);
                    break;
                }
                default: {
                    std::cout << "Received an invalid GameStateNetworkAction on deserialize" << std::endl;
                    break;
                }
            }
        }

        void setIsMultiplayer(bool yn) { isMultiplayer = yn; }
        void setIsServer(bool yn) { isServer = yn; }
        void setIsBotGame(bool yn) { isBotGame = yn; }

        bool IsMultiplayer() const { return isMultiplayer; }
        bool IsServer() const { return isServer; }
        bool IsBotGame() const { return isBotGame; }
    private:
        bool isMultiplayer;
        bool isServer;
        bool isBotGame;
    };
}