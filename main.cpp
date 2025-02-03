#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <sstream>
#include <vector>

#include "car.hpp"
#include "client.hpp"
#include "game.hpp"
#include "menu.hpp"
#include "network_manager.hpp"
#include "player.hpp"
#include "player_identity.hpp"

int main(int argc, char** argv) {
    srand(time(NULL));

    sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "Dunebuggies");
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);

    db::Client client(window, new db::Game(), new db::NetworkManager(), new db::Player());
    if (client.isReady()) {
        client.run();
    }

    return 0;
}
