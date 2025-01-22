#pragma once

#include <iostream>

constexpr int PORT = 9075;

class NetworkManager {
public:
    NetworkManager() : connected(false) {}

    bool setupServer() {
        if (listener.listen(PORT) != sf::Socket::Status::Done) return false;
        listener.setBlocking(false); // Non-blocking for multiple clients
        return true;
    }

    bool connectToServer(const std::string& host) {
        if (socket.connect(*sf::IpAddress::resolve(host), PORT, sf::seconds(5)) != sf::Socket::Status::Done) return false;
        socket.setBlocking(false);
        connected = true;
        return true;
    }

    void sendData(sf::Packet& packet) {
        std::cout << "Sending data: " << packet.getData() << std::endl;
        if (connected) {
            auto status = socket.send(packet);
            if (status != sf::Socket::Status::Done) {
                std::cout << "Failed to send socket data with status: " << int(status) << std::endl;
            }
        }
    }

    void broadcast(sf::Packet& packet) {
        std::cout << "Broadcasting data: " << packet.getData() << std::endl;
        for (auto& client : clients) {
            auto status = client.send(packet);
            if (status != sf::Socket::Status::Done) {
                std::cout << "Failed to send broadcast data" << std::endl;
            }
        }
    }

    std::vector<sf::Packet> receiveData() {
        std::vector<sf::Packet> packets;
        if (connected) {
            sf::Packet packet;
            if (socket.receive(packet) != sf::Socket::Status::Done) {
                std::cout << "Failed to receive packet" << std::endl;
            }
            packets.push_back(std::move(packet));
        } else { // Server mode, check for new connections and data from clients
            sf::TcpSocket newClient;
            newClient.setBlocking(false);
            if (listener.accept(newClient) == sf::Socket::Status::Done) {
                clients.push_back(std::move(newClient));
                std::cout << "New client connected. Total clients: " << clients.size() << std::endl;
            }
            for (auto& client : clients) {
                sf::Packet packet;
                if (client.receive(packet) != sf::Socket::Status::Done) {
                    continue;
                    std::cout << "Failed to receive packet" << std::endl;
                }
                std::cout << "Received data: " << packet.getData() << std::endl;
                packets.push_back(std::move(packet));
            }
        }
        return packets;
    }

    bool isConnected() const { return connected; }
private:
    sf::TcpListener listener;
    sf::TcpSocket socket;
    std::vector<sf::TcpSocket> clients;
    bool connected;
};
