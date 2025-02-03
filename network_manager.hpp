#pragma once

#include <SFML/Network.hpp>

#include "player_identity.hpp"
#include "game.hpp"

namespace db {
    constexpr int PORT = 9075;

    enum class NetworkAction : std::uint8_t {
        Update,
        Connect,
        Disconnect,
    };

    static sf::Packet& operator<<(sf::Packet& packet, db::NetworkAction action) {
        return packet << static_cast<std::uint8_t>(action);
    }

    static sf::Packet& operator>>(sf::Packet& packet, db::NetworkAction& action) {
        std::uint8_t new_action;
        packet >> new_action;
        action = static_cast<db::NetworkAction>(new_action);
        return packet;
    }

    class NetworkManager {
    public:
        NetworkManager() : connected(false), packets() {}
        bool setupServer();
        bool connectToServer(const std::string& host);
        void disconnect(player_id_t id);
        void sendData(sf::Packet& packet);
        void broadcast(sf::Packet& packet);

        std::vector<sf::Packet>& receiveData();
        std::vector<sf::Packet>& serialize(Game& game, player_id_t player_id);
        void deserialize(sf::Packet& packet, Game& game, player_id_t player_id);

        bool isConnected() const { return connected; }
    private:
        sf::TcpListener listener;
        sf::TcpSocket socket;
        std::vector<sf::TcpSocket> clients;
        std::vector<sf::Packet> packets;
        bool connected;
    };
}
