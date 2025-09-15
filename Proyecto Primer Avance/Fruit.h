#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
using namespace std;

// Initializes a single Fruit on the board.
class Fruit {
private:
    sf::Sprite sprite;
    int type;

public:
    // Constructor:
    // Sets the origin to the center so that scaling and movements keep the sprite centered.
    Fruit(sf::Texture& texture, sf::Vector2f positionTopLeft, int fruitType) {
        sprite.setTexture(texture);
        // origin in center so scaling keeps the sprite centered
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        sprite.setPosition(positionTopLeft.x + bounds.width / 2.f, positionTopLeft.y + bounds.height / 2.f);
        type = fruitType;
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    void setPosition(float xTopLeft, float yTopLeft) {
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setPosition(xTopLeft + bounds.width / 2.f, yTopLeft + bounds.height / 2.f);
    }

    int getType() const { return type; }
    void setType(int newType) { type = newType; }

    // Modifies visual size.
    void setScale(float sx, float sy) { sprite.setScale(sx, sy); }

    void resetVisual() { sprite.setScale(1.f, 1.f); }
};