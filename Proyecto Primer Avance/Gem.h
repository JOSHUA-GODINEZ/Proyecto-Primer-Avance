#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
using namespace std;
// Initializes a single gem on the board.

class Gem {
private:
    sf::Sprite sprite;
    int tipo;

public:
    // Constructor:
    // Ajusta el origen al centro para que las escalas y movimientos

    Gem(sf::Texture& textura, sf::Vector2f posicionTopLeft, int tipoGem) {
        sprite.setTexture(textura);
        // origen en el centro para que el escalado mantenga centrado el sprite
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        sprite.setPosition(posicionTopLeft.x + bounds.width / 2.f, posicionTopLeft.y + bounds.height / 2.f);
        tipo = tipoGem;
    }


    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }


    void setPosition(float xTopLeft, float yTopLeft) {
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setPosition(xTopLeft + bounds.width / 2.f, yTopLeft + bounds.height / 2.f);
    }


    int getTipo() const { return tipo; }
    void setTipo(int nuevoTipo) { tipo = nuevoTipo; }

    // Modifica el tamaño visual.
    void setScale(float sx, float sy) { sprite.setScale(sx, sy); }

    void resetVisual() { sprite.setScale(1.f, 1.f); }
};