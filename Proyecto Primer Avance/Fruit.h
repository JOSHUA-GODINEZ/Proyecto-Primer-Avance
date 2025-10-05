#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
using namespace std;



class Fruit {
protected:
    sf::Sprite sprite;
    int type;
public:
    Fruit() : type(-1) {}
    Fruit(sf::Texture& texture, sf::Vector2f positionTopLeft, int fruitType) : type(fruitType) {
        sprite.setTexture(texture);
        sprite.setScale(1.f, 1.f);
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        sprite.setPosition(positionTopLeft.x + bounds.width / 2.f, positionTopLeft.y + bounds.height / 2.f);
    }

    virtual ~Fruit() {}

    virtual void draw(sf::RenderWindow& win) { win.draw(sprite); }

    // Ahora onMatch recibe contexto (board + coords)
    virtual void onMatch(Board* board, int row, int col) {
        // default: nothing special
    }

    void setPosition(float xTopLeft, float yTopLeft) {
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setPosition(xTopLeft + bounds.width / 2.f, yTopLeft + bounds.height / 2.f);
    }
    void resetVisual() { sprite.setScale(1.f, 1.f); }
    void setScale(float sx, float sy) { sprite.setScale(sx, sy); }
    int getType() const { return type; }
    void setType(int t) { type = t; }
};

// NormalFruit - no cambio funcional aparte de la nueva firma
class NormalFruit : public Fruit {
public:
    NormalFruit(sf::Texture& tex, sf::Vector2f pos, int t) : Fruit(tex, pos, t) {}
   
};

// BombFruit: marca vecinos al explot ar
class BombFruit : public Fruit {
public:
    BombFruit(sf::Texture& tex, sf::Vector2f pos, int t) : Fruit(tex, pos, t) {}
    void onMatch(Board* board, int row, int col) override;
};
   


// IceFruit: ejemplo, por ahora no especial (podrías congelar tablero)
class IceFruit : public Fruit {
public:
    IceFruit(sf::Texture& tex, sf::Vector2f pos, int t) : Fruit(tex, pos, t) {}
   
};
