#include "network_manager.hpp"

#include "game.hpp"
#include "car.hpp"

bool db::NetworkManager::setupServer() {
    if (listener.listen(PORT) != sf::Socket::Status::Done) {
        return false;
    }
    listener.setBlocking(false); // Non-blocking for multiple clients
    return true;
}

bool db::NetworkManager::connectToServer(const std::string& host) {
    if (socket.connect(*sf::IpAddress::resolve(host), PORT, sf::seconds(5)) != sf::Socket::Status::Done) {
        return false;
    }
    socket.setBlocking(false);
    connected = true;
    return true;
}

void db::NetworkManager::disconnect(player_id_t id) {
    std::cout << "Networkmanager.disconnect" << std::endl;
    sf::Packet packet;
    packet << NetworkAction::Disconnect << id;
    if (socket.send(packet) != sf::Socket::Status::Done) {
        std::cout << "Failed to send disconnect message to server" << std::endl;
        return;
    }
    socket.disconnect();
    connected = false;
}

void db::NetworkManager::sendData(sf::Packet& packet) {
#ifdef DUNEBUGGIES_NET_DEBUG
    std::cout << "Sending data: " << packet.getData() << std::endl;
#endif
    if (connected) {
        auto status = socket.send(packet);
        if (status != sf::Socket::Status::Done) {
#ifdef DUNEBUGGIES_NET_DEBUG
            std::cout << "Failed to send socket data with status: " << int(status) << std::endl;
#endif
        }
    }
}

void db::NetworkManager::broadcast(sf::Packet& packet) {
#ifdef DUNEBUGGIES_NET_DEBUG
    std::cout << "Broadcasting data: " << packet.getData() << std::endl;
#endif
    for (auto& client : clients) {
        auto status = client.send(packet);
        if (status != sf::Socket::Status::Done) {
#ifdef DUNEBUGGIES_NET_DEBUG
            std::cout << "Failed to send broadcast data" << std::endl;
#endif
        }
    }
}

std::vector<sf::Packet> db::NetworkManager::receiveData() {
    std::vector<sf::Packet> packets;
    if (connected) {
        sf::Packet packet;
        if (socket.receive(packet) != sf::Socket::Status::Done) {
#ifdef DUNEBUGGIES_NET_DEBUG
            std::cout << "Failed to receive server packet" << std::endl;
#endif
            return packets;
        }
        packets.push_back(std::move(packet));
    }
    else { // Server mode, check for new connections and data from clients
        sf::TcpSocket newClient;
        newClient.setBlocking(false);

        if (listener.accept(newClient) == sf::Socket::Status::Done) {
            clients.push_back(std::move(newClient));
            std::cout << "New client connected. Total clients: " << clients.size() << std::endl;
        }

        for (auto& client : clients) {
            sf::Packet packet;
            if (client.receive(packet) != sf::Socket::Status::Done) {
#ifdef DUNEBUGGIES_NET_DEBUG
                std::cout << "Failed to receive client packet" << std::endl;
#endif
                continue;
            }
#ifdef DUNEBUGGIES_NET_DEBUG
            std::cout << "Received data: " << packet.getData() << std::endl;
#endif
            packets.push_back(std::move(packet));
        }
    }
    return packets;
}

std::vector<sf::Packet>& db::NetworkManager::serialize(db::Game& game, player_id_t player_id) {
    std::vector<sf::Packet> packets = {};

    sf::Packet packet;
    if (game.isServer()) {
        for (auto& p : game.getCars()) {
            packet << NetworkAction::Update << p.first << p.second;
            packets.push_back(std::move(packet));
        }
    }
    else {
        packet << NetworkAction::Update << player_id << game.getCars()[player_id];
        packets.push_back(std::move(packet));
    }

    return packets;
}

void db::NetworkManager::deserialize(sf::Packet& packet, Game& game, player_id_t player_id) {
    NetworkAction currentAction;
    player_id_t netpid;

    if (!(packet >> currentAction >> netpid)) {
        return;
    }

    switch (currentAction) {
    case NetworkAction::Update: {
        // Don't update ourself
        if (netpid == player_id) {
            return;
        }

        if (game.getCars()[netpid] == NULL || game.getCars()[netpid] == nullptr) {
            Car* car = new Car();
            packet >> *car;
            game.getCars()[netpid] = car;
        }
        else {
            packet >> *game.getCars()[netpid];
        }
        // If we don't have this token it must be a new car
        std::cout << "Updated Car: " << netpid << *game.getCars()[netpid] << std::endl;
        break;
    }
    case NetworkAction::Connect: {
        break;
    }
    case NetworkAction::Disconnect: {
        // token_car_map.erase(token);
        break;
    }
    default: {
        std::cout << "Received an invalid GameStateNetworkAction on deserialize" << std::endl;
        break;
    }
    }
    return;
}
