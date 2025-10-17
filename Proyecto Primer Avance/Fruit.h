#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>

using namespace std;

// ---------------------- Fruit & subclasses (con animación) ----------------------
class Board; 

class Fruit {
protected:
    bool popActive = false;
    float popElapsedMs = 0.f;
    float popDurationMs = 220.f;   // duración total de la animación
    float popStartScale = 1.f;
    float popTargetScale = 1.35f;  // MARK_SCALE o lo que quieras
    bool pendingRemoval = false;

    sf::Sprite sprite;
    int type;

    // Animación lineal
    sf::Vector2f moveStart;
    sf::Vector2f moveEnd;
    float moveDurationMs = 0.f;
    sf::Clock moveClock;
    bool moving = false;

    float currentScale = 1.f;
    bool popping = false;
public:
    Fruit() : type(-1) {}
    Fruit(sf::Texture& texture, sf::Vector2f pixelCenter, int fruitType) : type(fruitType) {
        sprite.setTexture(texture);
        sprite.setScale(1.f, 1.f);
        sf::FloatRect b = sprite.getLocalBounds();
        sprite.setOrigin(b.width / 2.f, b.height / 2.f);
        sprite.setPosition(pixelCenter);
    }
    virtual ~Fruit() {}

    virtual void draw(sf::RenderWindow& win) { win.draw(sprite); }
    virtual void onMatch(Board* board, int row, int col) { /* default no extra effect */ }

    // Pixel position helpers
    sf::Vector2f getPixelPosition() const { return sprite.getPosition(); }
    void setPixelPosition(float x, float y) { sprite.setPosition(x, y); }

    // start move to pixel coords in duration_ms milliseconds
    void startMoveTo(float tx, float ty, float duration_ms) {
        moveStart = sprite.getPosition();
        moveEnd = { tx, ty };
        moveDurationMs = duration_ms;
        moveClock.restart();
        moving = true;
    }

    void updateMove() {
        if (!moving) return;
        float elapsed = moveClock.getElapsedTime().asMilliseconds();
        if (moveDurationMs <= 0.f) {
            sprite.setPosition(moveEnd);
            moving = false;
            return;
        }
        float t = elapsed / moveDurationMs;
        if (t >= 1.f) {
            sprite.setPosition(moveEnd);
            moving = false;
        }
        else {
            sf::Vector2f p = moveStart + (moveEnd - moveStart) * t;
            sprite.setPosition(p);
        }
    }

    void startPop(float duration_ms = 220.f, float targetScale = 1.5f) {
        popActive = true;
        popElapsedMs = 0.f;
        popDurationMs = std::max(1.0f, duration_ms);
        popStartScale = sprite.getScale().x; // asumimos x==y
        popTargetScale = targetScale;
        pendingRemoval = true;
    }

    bool updatePop(float deltaMs) {
        if (!popActive) return false;
        popElapsedMs += deltaMs;
        float t = popElapsedMs / popDurationMs;
        if (t >= 1.f) t = 1.f;

        // easing: ease-out quad (mejor sensación)
        float e = 1.f - (1.f - t) * (1.f - t);
        float newScale = popStartScale + (popTargetScale - popStartScale) * e;
        sprite.setScale(newScale, newScale);

        if (t >= 1.f) {
            popActive = false;
            return true; // terminó
        }
        return false; // aún en progreso
    }

    bool isPopActive() const { return popActive; }

    bool isMoving() const { return moving; }


    int getType() const { return type; }
    void setType(int t) { type = t; }

    void setScale(float sx, float sy) { sprite.setScale(sx, sy); }

    void resetVisual() { sprite.setScale(1.f, 1.f); }

};

class NormalFruit : public Fruit {
public:
    NormalFruit(sf::Texture& tex, sf::Vector2f pos, int t) : Fruit(tex, pos, t) {}
    void onMatch(Board*, int, int) override {

    }
};

class BombFruit : public Fruit {
public:
    BombFruit(sf::Texture& tex, sf::Vector2f pos, int t) : Fruit(tex, pos, t) {}
    void onMatch(Board* board, int row, int col) override;
};

class IceFruit : public Fruit {
public:
    IceFruit(sf::Texture& tex, sf::Vector2f pos, int t) : Fruit(tex, pos, t) {}
    void onMatch(Board*, int, int) override {

    }
};
class SuperFruit : public Fruit {
public:
    SuperFruit(sf::Texture& tex, const sf::Vector2f& pos, int logicalType)
        : Fruit(tex, pos, logicalType) {
    }
    void onMatch(Board* board, int row, int col) override;
};



