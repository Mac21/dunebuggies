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

#include "client.hpp"

// Constants
constexpr float PI = 3.14159265358979323846f;
constexpr float CAR_RADIUS = 32.0f;

int main(int argc, char** argv) {
    sf::RenderWindow window(sf::VideoMode({ 1920, 1080 }), "Dunebuggies");
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);

    db::Client client;
    if (!client.init(window)) {
        return -1;
    }

    client.run();

    return 0;
}