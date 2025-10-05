// main.cpp
// Código base (frutas, bombas, hielo, niveles) + sistema de objetivos mostrado arriba a la derecha
// Nivel1: eliminar 20 frutas de un tipo aleatorio (entre 5 normales).
// Nivel2: eliminar 3 hielos.
// Nivel3: conseguir 500 puntos.
// El progreso se actualiza en tiempo real y se muestra en pantalla final.

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <string>

using namespace std;

// ---------------------- Fruit & subclasses (con animación) ----------------------
class Board; // forward

class Fruit {
protected:
    sf::Sprite sprite;
    int type;

    // Animación lineal
    sf::Vector2f moveStart;
    sf::Vector2f moveEnd;
    float moveDurationMs = 0.f;
    sf::Clock moveClock;
    bool moving = false;

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

    bool isMoving() const { return moving; }

    sf::Vector2f getHalfSize() const {
        sf::FloatRect b = sprite.getLocalBounds();
        return { b.width / 2.f, b.height / 2.f };
    }

    int getType() const { return type; }
    void setType(int t) { type = t; }

    void setScale(float sx, float sy) { sprite.setScale(sx, sy); }
    void resetVisual() { sprite.setScale(1.f, 1.f); }
};

class NormalFruit : public Fruit {
public:
    NormalFruit(sf::Texture& tex, sf::Vector2f pos, int t) : Fruit(tex, pos, t) {}
    void onMatch(Board* /*board*/, int /*r*/, int /*c*/) override {
        // default: add score handled by Board after deletion
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
    void onMatch(Board* /*board*/, int /*r*/, int /*c*/) override {
        // handled by Board (special rules)
    }
};

// ---------------------- Board ----------------------
class Board {
private:
    static constexpr int SIZE = 8;
    static constexpr int NUM_NORMAL = 5;   // textures 0..4 are normal fruit types
    static constexpr int BOMB_INDEX = 5;   // index texture for bomb
    static constexpr int ICE_INDEX = 6;    // index for ice
    static constexpr int NUM_TEXTURES = 7; // total loaded textures

    Fruit* matrix[SIZE][SIZE];
    vector<sf::Texture> textures;

    // layout logical
    const float originX = 122.f;
    const float originY = 107.f;
    const float cellW = 69.f;
    const float cellH = 60.f;

    // cleaning flags
    bool marked[SIZE][SIZE];
    bool comboMarked[SIZE][SIZE];
    bool forcedBreak[SIZE][SIZE];
    bool removalPending = false;
    bool cleaningInProgress = false;
    bool countScoreDuringCleaning = true;

    // swap tracking for ice rules
    int lastSwapR1 = -1, lastSwapC1 = -1;
    int lastSwapR2 = -1, lastSwapC2 = -1;

    // gameplay
    int remainingMoves = 4;
    int score = 0;
    int level = 1;

    // gravity animation
    bool gravityAnimating = false;
    float gravityDurationMs = 900.f; // requested ~900 ms

    const float MARK_SCALE = 1.35f;
    const float SELECT_SCALE = 1.35f;

    int selectedRow = -1, selectedCol = -1;

    // ---------------- Ajustes de probabilidades por nivel ----------------
    int initBombChance[4] = { 0, 5, 3, 1 };   // índice 1..3 válido
    int initIceChance[4] = { 0, 5, 12, 20 };
    int refillBombChance[4] = { 0, 5, 3, 1 };
    int refillIceChance[4] = { 0, 0, 0, 0 };  // no generar hielo en refill según tu requerimiento

    // ----------------- Objetivo del nivel -----------------
    // kinds: 1 = eliminar tipo normal X (count), 2 = eliminar hielos (count), 3 = alcanzar puntos
    int objectiveKind = 0;
    int objectiveTarget = 0;      // target count or target points
    int objectiveProgress = 0;    // current progress
    int objectiveTargetFruit = -1; // for kind==1, which normal fruit (0..NUM_NORMAL-1)

    // helpers
    void initObjectiveForLevel(int lvl) {
        objectiveKind = 0;
        objectiveTarget = 0;
        objectiveProgress = 0;
        objectiveTargetFruit = -1;
        if (lvl == 1) {
            objectiveKind = 1;
            objectiveTarget = 20;
            // seleccionar aleatoriamente tipo normal [0..NUM_NORMAL-1]
            objectiveTargetFruit = rand() % NUM_NORMAL;
        }
        else if (lvl == 2) {
            objectiveKind = 2;
            objectiveTarget = 3;
        }
        else if (lvl == 3) {
            objectiveKind = 3;
            objectiveTarget = 500;
            objectiveProgress = score; // starts at 0
        }
    }

    // llama cuando se borra una fruta (antes de delete) para actualizar objetivo
    void incrementObjectiveOnDelete(int fruitType) {
        if (objectiveKind == 1) {
            // solo contar si es un normal del tipo objetivo
            if (fruitType >= 0 && fruitType < NUM_NORMAL && fruitType == objectiveTargetFruit) {
                objectiveProgress++;
            }
        }
        else if (objectiveKind == 2) {
            if (fruitType == ICE_INDEX) objectiveProgress++;
        }
        else if (objectiveKind == 3) {
            // para objetivo de puntos, el progreso está ligado al score; lo actualizaremos fuera
        }
    }

public:
    Board(int startLevel = 1) {
        srand(static_cast<unsigned int>(time(nullptr)));

        textures.resize(NUM_TEXTURES);

        // Ajusta rutas según tu máquina:
        if (!textures[0].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Apple.png")) cerr << "Failed to load textures[0]\n";
        if (!textures[1].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Orange.png")) cerr << "Failed to load textures[1]\n";
        if (!textures[2].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Banana.png")) cerr << "Failed to load textures[2]\n";
        if (!textures[3].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Watermelon.png")) cerr << "Failed to load textures[3]\n";
        if (!textures[4].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Grape.png")) cerr << "Failed to load textures[4]\n";
        if (!textures[5].loadFromFile("C:\\Joshua\\practica\\Sprites\\preuba bomba.jpg")) cerr << "Failed to load textures[5]\n";
        if (!textures[6].loadFromFile("C:\\Joshua\\practica\\Sprites\\hielo pureba.jpg")) cerr << "Failed to load textures[6]\n";

        level = startLevel;
        initObjectiveForLevel(level);

        // init arrays & fill randomly
        for (int r = 0; r < SIZE; ++r)
            for (int c = 0; c < SIZE; ++c) {
                matrix[r][c] = nullptr;
                marked[r][c] = comboMarked[r][c] = forcedBreak[r][c] = false;
            }

        // Ahora: inicializar usando probabilidades del nivel actual (aquí el hielo sí puede aparecer)
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                sf::Vector2f center = cellCenter(r, c);
                int roll = rand() % 100;
                if (roll < initBombChance[level]) {
                    matrix[r][c] = new BombFruit(textures[BOMB_INDEX], center, BOMB_INDEX);
                }
                else if (roll < initBombChance[level] + initIceChance[level]) {
                    matrix[r][c] = new IceFruit(textures[ICE_INDEX], center, ICE_INDEX);
                }
                else {
                    int t = rand() % NUM_NORMAL;
                    matrix[r][c] = new NormalFruit(textures[t], center, t);
                }
            }
        }

        // Stabilize initial board: remove combos but do NOT remove ice due to adjacency
        bool cont = true;
        while (cont) {
            if (clearCombosOnce(false)) {
                // apply instant gravity for init (no animation)
                applyGravityInstantForInit();
            }
            else cont = false;
        }
    }

    ~Board() {
        for (int r = 0; r < SIZE; ++r)
            for (int c = 0; c < SIZE; ++c)
                if (matrix[r][c]) { delete matrix[r][c]; matrix[r][c] = nullptr; }
    }

    void setLevel(int lvl) {
        if (lvl < 1) lvl = 1;
        if (lvl > 3) lvl = 3;
        level = lvl;
        initObjectiveForLevel(level);
    }

    Fruit* createFruitByType(int logicalType, sf::Texture& tex, sf::Vector2f pixelCenter) {
        if (logicalType == BOMB_INDEX) return new BombFruit(tex, pixelCenter, logicalType);
        if (logicalType == ICE_INDEX) return new IceFruit(tex, pixelCenter, logicalType);
        return new NormalFruit(tex, pixelCenter, logicalType);
    }

    sf::Vector2f cellCenter(int row, int col) const {
        // center inside logical cell
        return { col * cellW + originX + cellW / 2.f, row * cellH + originY + cellH / 2.f };
    }

    void markCellForRemoval(int r, int c, bool forceBreakIce = false) {
        if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) return;
        marked[r][c] = true;
        if (forceBreakIce) forcedBreak[r][c] = true;
        removalPending = true;
    }

    bool detectCombinationsWithoutRemoving() {
        // horizontals
        for (int r = 0; r < SIZE; ++r) {
            int count = 1;
            for (int c = 1; c < SIZE; ++c) {
                if (matrix[r][c] && matrix[r][c - 1] && matrix[r][c]->getType() == matrix[r][c - 1]->getType()) {
                    ++count;
                    if (count >= 3) return true;
                }
                else count = 1;
            }
        }
        // verticals
        for (int c = 0; c < SIZE; ++c) {
            int count = 1;
            for (int r = 1; r < SIZE; ++r) {
                if (matrix[r][c] && matrix[r - 1][c] && matrix[r][c]->getType() == matrix[r - 1][c]->getType()) {
                    ++count;
                    if (count >= 3) return true;
                }
                else count = 1;
            }
        }
        return false;
    }

    bool formsCombinationWhenSwapping(int r1, int c1, int r2, int c2) {
        Fruit* tmp = matrix[r1][c1];
        matrix[r1][c1] = matrix[r2][c2];
        matrix[r2][c2] = tmp;
        bool has = detectCombinationsWithoutRemoving();
        tmp = matrix[r1][c1];
        matrix[r1][c1] = matrix[r2][c2];
        matrix[r2][c2] = tmp;
        return has;
    }

    // clear combos once; two modes: initialization (countScore==false) or game (true)
    bool clearCombosOnce(bool countScore) {
        if (!countScore) {
            // initialization: remove normal combos but skip ice removal
            bool anyRemoval = false;
            bool markedLocal[SIZE][SIZE] = { false };

            // horizontals
            for (int r = 0; r < SIZE; ++r) {
                int c = 0;
                while (c < SIZE) {
                    if (!matrix[r][c]) { ++c; continue; }
                    int t = matrix[r][c]->getType();
                    int k = c + 1;
                    while (k < SIZE && matrix[r][k] && matrix[r][k]->getType() == t) ++k;
                    int run = k - c;
                    if (run >= 3) {
                        for (int s = c; s < k; ++s) {
                            if (matrix[r][s]->getType() == ICE_INDEX) continue;
                            markedLocal[r][s] = true;
                            anyRemoval = true;
                        }
                    }
                    c = k;
                }
            }

            // verticals
            for (int c = 0; c < SIZE; ++c) {
                int r = 0;
                while (r < SIZE) {
                    if (!matrix[r][c]) { ++r; continue; }
                    int t = matrix[r][c]->getType();
                    int k = r + 1;
                    while (k < SIZE && matrix[k][c] && matrix[k][c]->getType() == t) ++k;
                    int run = k - r;
                    if (run >= 3) {
                        for (int s = r; s < k; ++s) {
                            if (matrix[s][c]->getType() == ICE_INDEX) continue;
                            markedLocal[s][c] = true;
                            anyRemoval = true;
                        }
                    }
                    r = k;
                }
            }

            if (anyRemoval) {
                for (int r = 0; r < SIZE; ++r)
                    for (int c = 0; c < SIZE; ++c)
                        if (markedLocal[r][c] && matrix[r][c]) {
                            // actualizar objetivo si aplica (nivel 1 no cuenta hielo porque los excluimos)
                            incrementObjectiveOnDelete(matrix[r][c]->getType());
                            delete matrix[r][c];
                            matrix[r][c] = nullptr;
                        }
            }
            return anyRemoval;
        }

        // GAME MODE:
        if (!removalPending) {
            bool anyRemoval = false;
            bool markedLocal[SIZE][SIZE] = { false };
            bool comboLocal[SIZE][SIZE] = { false };

            // horizontals
            for (int r = 0; r < SIZE; ++r) {
                int c = 0;
                while (c < SIZE) {
                    if (!matrix[r][c]) { ++c; continue; }
                    int t = matrix[r][c]->getType();
                    int k = c + 1;
                    while (k < SIZE && matrix[r][k] && matrix[r][k]->getType() == t) ++k;
                    int run = k - c;
                    if (run >= 3) {
                        for (int s = c; s < k; ++s) {
                            markedLocal[r][s] = true;
                            if (matrix[r][s]->getType() == ICE_INDEX) {
                                bool wasSwapped = (r == lastSwapR1 && s == lastSwapC1) || (r == lastSwapR2 && s == lastSwapC2);
                                comboLocal[r][s] = wasSwapped;
                            }
                            else comboLocal[r][s] = true;
                        }
                        anyRemoval = true;
                    }
                    c = k;
                }
            }

            // verticals
            for (int c = 0; c < SIZE; ++c) {
                int r = 0;
                while (r < SIZE) {
                    if (!matrix[r][c]) { ++r; continue; }
                    int t = matrix[r][c]->getType();
                    int k = r + 1;
                    while (k < SIZE && matrix[k][c] && matrix[k][c]->getType() == t) ++k;
                    int run = k - r;
                    if (run >= 3) {
                        for (int s = r; s < k; ++s) {
                            markedLocal[s][c] = true;
                            if (matrix[s][c]->getType() == ICE_INDEX) {
                                bool wasSwapped = (s == lastSwapR1 && c == lastSwapC1) || (s == lastSwapR2 && c == lastSwapC2);
                                comboLocal[s][c] = wasSwapped;
                            }
                            else comboLocal[s][c] = true;
                        }
                        anyRemoval = true;
                    }
                    r = k;
                }
            }

            // if there was a combo and last swapped cells are ice, mark them too (case: swap ice -> combo elsewhere)
            if (anyRemoval) {
                if (lastSwapR1 != -1) {
                    if (lastSwapR1 >= 0 && lastSwapR1 < SIZE && lastSwapC1 >= 0 && lastSwapC1 < SIZE) {
                        if (matrix[lastSwapR1][lastSwapC1] && matrix[lastSwapR1][lastSwapC1]->getType() == ICE_INDEX) {
                            markedLocal[lastSwapR1][lastSwapC1] = true;
                            comboLocal[lastSwapR1][lastSwapC1] = true;
                        }
                    }
                }
                if (lastSwapR2 != -1) {
                    if (lastSwapR2 >= 0 && lastSwapR2 < SIZE && lastSwapC2 >= 0 && lastSwapC2 < SIZE) {
                        if (matrix[lastSwapR2][lastSwapC2] && matrix[lastSwapR2][lastSwapC2]->getType() == ICE_INDEX) {
                            markedLocal[lastSwapR2][lastSwapC2] = true;
                            comboLocal[lastSwapR2][lastSwapC2] = true;
                        }
                    }
                }
            }

            if (anyRemoval) {
                for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) {
                    marked[r][c] = markedLocal[r][c];
                    comboMarked[r][c] = comboLocal[r][c];
                    if (marked[r][c] && matrix[r][c]) matrix[r][c]->setScale(MARK_SCALE, MARK_SCALE);
                }
                removalPending = true;
                return true;
            }
            return false;
        }
        else {
            // removalPending == true: execute onMatch chain (bombs call markCellForRemoval and may force ice break)
            bool anyRemoval = false;
            bool processed[SIZE][SIZE] = { false };

            bool progress = true;
            while (progress) {
                progress = false;
                for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) {
                    if (marked[r][c] && !processed[r][c] && matrix[r][c]) {
                        bool isIce = (matrix[r][c]->getType() == ICE_INDEX);
                        if (isIce && !comboMarked[r][c] && !forcedBreak[r][c]) {
                            processed[r][c] = true;
                            continue;
                        }
                        // call onMatch (bomb will mark neighbors etc)
                        matrix[r][c]->onMatch(this, r, c);
                        processed[r][c] = true;
                        progress = true;
                    }
                }
            }

            // delete marked cells but respect ice rules (only delete ice if comboMarked or forcedBreak)
            for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) {
                if (marked[r][c] && matrix[r][c]) {
                    bool isIce = (matrix[r][c]->getType() == ICE_INDEX);
                    if (isIce && !comboMarked[r][c] && !forcedBreak[r][c]) continue;

                    // **Antes de eliminar** actualizar el objetivo si procede
                    incrementObjectiveOnDelete(matrix[r][c]->getType());

                    delete matrix[r][c];
                    matrix[r][c] = nullptr;
                    anyRemoval = true;
                    score += 10; // add 10 points per fruit removed

                    // Si el objetivo es de puntos, actualizamos el progreso en cada eliminación
                    if (objectiveKind == 3) objectiveProgress = score;
                }
            }

            // reset flags and visuals
            for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) {
                marked[r][c] = comboMarked[r][c] = forcedBreak[r][c] = false;
                if (matrix[r][c]) matrix[r][c]->resetVisual();
            }

            // reset last swap (applies only to that swap)
            lastSwapR1 = lastSwapC1 = lastSwapR2 = lastSwapC2 = -1;
            removalPending = false;

            // start gravity animation to refill board (instead of instant drop)
            applyGravityAndReplace();
            return anyRemoval;
        }
    }

    // applyGravityAndReplace: compute final state, create new fruits at top, start animation (gravityDurationMs)
    void applyGravityAndReplace() {
        if (gravityAnimating) return;

        Fruit* finalMatrix[SIZE][SIZE];
        for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) finalMatrix[r][c] = nullptr;

        // compact columns downward and create new fruits for empty spots at top
        for (int c = 0; c < SIZE; ++c) {
            int writeRow = SIZE - 1;
            // move existing fruit pointers down
            for (int r = SIZE - 1; r >= 0; --r) {
                if (matrix[r][c] != nullptr) {
                    finalMatrix[writeRow][c] = matrix[r][c];
                    writeRow--;
                }
            }
            // create new fruit objects for rows 0..writeRow
            for (int r = writeRow; r >= 0; --r) {
                int roll = rand() % 100;
                sf::Vector2f dummy = { 0.f, 0.f };
                // refill during game: según tu petición, hielo NO aparece durante refill
                if (roll < refillBombChance[level]) finalMatrix[r][c] = createFruitByType(BOMB_INDEX, textures[BOMB_INDEX], dummy);
                else {
                    int t = rand() % NUM_NORMAL;
                    finalMatrix[r][c] = createFruitByType(t, textures[t], dummy);
                }
            }
        }

        // preserve previous pointers to decide which fruits are new
        Fruit* prevMatrix[SIZE][SIZE];
        for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) prevMatrix[r][c] = matrix[r][c];

        // reassign matrix to final pointers
        for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) matrix[r][c] = finalMatrix[r][c];

        // For each column, from bottom to top, decide initial position and start move
        for (int c = 0; c < SIZE; ++c) {
            // build list of prev pointers in this column (any order)
            vector<Fruit*> prevList;
            for (int r = 0; r < SIZE; ++r) {
                if (prevMatrix[r][c] != nullptr) prevList.push_back(prevMatrix[r][c]);
            }

            // We'll match bottom-up: for each target row from bottom to top, if that pointer is equal to one of prevList -> reused
            // else it's new
            int newAboveCount = 0;
            // iterate bottom-up
            for (int r = SIZE - 1; r >= 0; --r) {
                Fruit* f = matrix[r][c];
                if (!f) continue;
                sf::Vector2f target = cellCenter(r, c);

                bool existedBefore = false;
                // search prevList for pointer equality
                for (Fruit* pf : prevList) if (pf == f) { existedBefore = true; break; }

                if (existedBefore) {
                    // initial = its current sprite pos (where it was before), then move to target
                    sf::Vector2f init = f->getPixelPosition();
                    // start movement
                    f->startMoveTo(target.x, target.y, gravityDurationMs);
                    // remove this pointer from prevList so not reused again
                    for (auto it = prevList.begin(); it != prevList.end(); ++it) {
                        if (*it == f) { prevList.erase(it); break; }
                    }
                }
                else {
                    // new fruit: place above grid; stagger by newAboveCount to create stacked spawn
                    float initX = target.x;
                    float initY = originY - (newAboveCount + 1) * cellH + cellH / 2.f;
                    f->setPixelPosition(initX, initY);
                    f->startMoveTo(target.x, target.y, gravityDurationMs);
                    newAboveCount++;
                }
            }
        }

        gravityAnimating = true;
    }

    // Immediate gravity used only in init phase so we don't rely on animation loop
    void applyGravityInstantForInit() {
        for (int c = 0; c < SIZE; ++c) {
            int writeRow = SIZE - 1;
            for (int r = SIZE - 1; r >= 0; --r) {
                if (matrix[r][c] != nullptr) {
                    if (writeRow != r) {
                        matrix[writeRow][c] = matrix[r][c];
                        matrix[r][c] = nullptr;
                    }
                    writeRow--;
                }
            }
            for (int r = writeRow; r >= 0; --r) {
                int roll = rand() % 100;
                sf::Vector2f center = cellCenter(r, c);
                if (roll < initBombChance[level]) matrix[r][c] = createFruitByType(BOMB_INDEX, textures[BOMB_INDEX], center);
                else if (roll < initBombChance[level] + initIceChance[level]) matrix[r][c] = createFruitByType(ICE_INDEX, textures[ICE_INDEX], center);
                else {
                    int t = rand() % NUM_NORMAL;
                    matrix[r][c] = createFruitByType(t, textures[t], center);
                }
            }
        }
        // Set pixel positions to targets (center) so sprites look correct
        for (int r = 0; r < SIZE; ++r)
            for (int c = 0; c < SIZE; ++c)
                if (matrix[r][c]) {
                    sf::Vector2f center = cellCenter(r, c);
                    matrix[r][c]->setPixelPosition(center.x, center.y);
                }
    }

    // update animations per-frame (call from Game update)
    void updateAnimations() {
        if (!gravityAnimating) return;
        bool anyMoving = false;
        for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) {
            if (matrix[r][c]) {
                matrix[r][c]->updateMove();
                if (matrix[r][c]->isMoving()) anyMoving = true;
            }
        }
        if (!anyMoving) gravityAnimating = false;
    }

    bool isGravityAnimating() const { return gravityAnimating; }

    // swapCells: check adjacency & whether forms combo; immediate visual swap
    bool swapCells(int r1, int c1, int r2, int c2) {
        if (r1 < 0 || r1 >= SIZE || c1 < 0 || c1 >= SIZE || r2 < 0 || r2 >= SIZE || c2 < 0 || c2 >= SIZE) return false;
        int dr = abs(r1 - r2), dc = abs(c1 - c2);
        if (!((dr == 1 && dc == 0) || (dr == 0 && dc == 1))) return false;

        if (!formsCombinationWhenSwapping(r1, c1, r2, c2)) return false;

        // record swap positions for ice rules
        lastSwapR1 = r1; lastSwapC1 = c1; lastSwapR2 = r2; lastSwapC2 = c2;

        Fruit* tmp = matrix[r1][c1];
        matrix[r1][c1] = matrix[r2][c2];
        matrix[r2][c2] = tmp;

        // update sprite pixel positions immediately to centers of their new cells
        if (matrix[r1][c1]) {
            sf::Vector2f center = cellCenter(r1, c1);
            matrix[r1][c1]->setPixelPosition(center.x, center.y);
        }
        if (matrix[r2][c2]) {
            sf::Vector2f center = cellCenter(r2, c2);
            matrix[r2][c2]->setPixelPosition(center.x, center.y);
        }

        --remainingMoves;
        startCleaning(true);
        return true;
    }

    void startCleaning(bool countScore) {
        cleaningInProgress = true;
        countScoreDuringCleaning = countScore;
    }

    bool stepCleaning() {
        if (!cleaningInProgress) return false;
        // if gravity animating, wait until it finishes
        if (gravityAnimating) return true;
        bool didSomething = clearCombosOnce(countScoreDuringCleaning);
        if (didSomething) {
            // if clearCombosOnce removed something it will call applyGravityAndReplace which starts gravityAnimating
            return true;
        }
        else {
            cleaningInProgress = false;
            return false;
        }
    }

    bool isCleaning() const { return cleaningInProgress; }

    // selection & clicks: first selects (scale), second attempts swap; clicking a bomb detonates instantly
    void selectOrSwap(int row, int col) {
        if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) return;
        if (!matrix[row][col]) return;

        if (selectedRow == -1) {
            BombFruit* asBomb = dynamic_cast<BombFruit*>(matrix[row][col]);
            if (asBomb != nullptr) {
                // detonate: mark own cell (force) and neighbors (force) and start cleaning
                markCellForRemoval(row, col, true);
                asBomb->onMatch(this, row, col);
                removalPending = true;
                startCleaning(true);
                return;
            }
            selectedRow = row; selectedCol = col;
            matrix[selectedRow][selectedCol]->setScale(SELECT_SCALE, SELECT_SCALE);
            return;
        }

        // second click
        if (selectedRow == row && selectedCol == col) {
            if (matrix[selectedRow][selectedCol]) matrix[selectedRow][selectedCol]->resetVisual();
            selectedRow = -1; selectedCol = -1;
            return;
        }

        if (matrix[selectedRow][selectedCol]) matrix[selectedRow][selectedCol]->resetVisual();

        swapCells(selectedRow, selectedCol, row, col);
        selectedRow = -1; selectedCol = -1;
    }

    pair<int, int> screenToCell(sf::RenderWindow& win, int mouseX, int mouseY) {
        sf::Vector2f world = win.mapPixelToCoords(sf::Vector2i(mouseX, mouseY));
        int col = int(std::floor((world.x - originX) / cellW));
        int row = int(std::floor((world.y - originY) / cellH));
        return { row, col };
    }

    void resetBoard() {
        // free current
        for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) if (matrix[r][c]) { delete matrix[r][c]; matrix[r][c] = nullptr; }

        // reset flags & stats
        for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) {
            marked[r][c] = comboMarked[r][c] = forcedBreak[r][c] = false;
        }
        remainingMoves = 5;
        score = 0;
        cleaningInProgress = false;
        removalPending = false;
        lastSwapR1 = lastSwapC1 = lastSwapR2 = lastSwapC2 = -1;
        gravityAnimating = false;

        // reiniciar objetivo para el nivel actual
        initObjectiveForLevel(level);

        // create new matrix with random fruits based on current level: ice only in init, bombs possibly in init
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                sf::Vector2f center = cellCenter(r, c);
                int roll = rand() % 100;
                if (roll < initBombChance[level]) {
                    matrix[r][c] = new BombFruit(textures[BOMB_INDEX], center, BOMB_INDEX);
                }
                else if (roll < initBombChance[level] + initIceChance[level]) {
                    matrix[r][c] = new IceFruit(textures[ICE_INDEX], center, ICE_INDEX);
                }
                else {
                    int t = rand() % NUM_NORMAL;
                    matrix[r][c] = new NormalFruit(textures[t], center, t);
                }
            }
        }

        bool cont = true;
        while (cont) {
            if (clearCombosOnce(false)) {
                applyGravityInstantForInit();
            }
            else cont = false;
        }
    }

    // getters/setters
    int getScore() const { return score; }
    int getRemainingMoves() const { return remainingMoves; }
    bool hasMoves() const { return remainingMoves > 0; }
    void resetMoves(int n) { remainingMoves = n; }
    int getLevel() const { return level; }
    void setLevelPublic(int l) { setLevel(l); } // wrapper pública usada por Game

    // Objetivo queries (para UI)
    string getObjectiveDescription() const {
        if (objectiveKind == 1) {
         
            if (objectiveTargetFruit == 0) {
                return "Eliminar " + to_string(objectiveTarget) + " Manzanas";
            }
            else if (objectiveTargetFruit == 1) {
                return "Eliminar " + to_string(objectiveTarget) + " Naranjas";
            }
            else if (objectiveTargetFruit == 2) {
                return "Eliminar " + to_string(objectiveTarget) + " Bananas";
            }
            else if (objectiveTargetFruit == 3) {
                return "Eliminar " + to_string(objectiveTarget) + " Sandias";
            }
            else if (objectiveTargetFruit == 4) {
                return "Eliminar " + to_string(objectiveTarget) + " Uvas";
            }
        }
        else if (objectiveKind == 2) {
            return "Eliminar " + to_string(objectiveTarget) + " hielos";
        }
        else if (objectiveKind == 3) {
            return "Conseguir " + to_string(objectiveTarget) + " puntos";
        }
        return "";
    }
    string getObjectiveProgressText() const {
        if (objectiveKind == 1 || objectiveKind == 2) {
            return to_string(objectiveProgress) + " / " + to_string(objectiveTarget);
        }
        else if (objectiveKind == 3) {
            return to_string(objectiveProgress) + " / " + to_string(objectiveTarget);
        }
        return "";
    }
    bool isObjectiveComplete() const {
        if (objectiveKind == 0) return true;
        if (objectiveKind == 3) return objectiveProgress >= objectiveTarget;
        return objectiveProgress >= objectiveTarget;
    }

    void draw(sf::RenderWindow& win) {
        for (int r = 0; r < SIZE; ++r)
            for (int c = 0; c < SIZE; ++c)
                if (matrix[r][c]) matrix[r][c]->draw(win);
    }
};

// ---------------------- BombFruit onMatch implementation ----------------------
void BombFruit::onMatch(Board* board, int row, int col) {
    // mark neighbors and force ice break
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = row + dr;
            int nc = col + dc;
            board->markCellForRemoval(nr, nc, true);
        }
    }
}

// ---------------------- Game (UI + loop) ----------------------
class Game {
private:
    sf::RenderWindow window{ sf::VideoMode(800, 600), "Match-3" };
    sf::Font font;
    sf::Text scoreText, movesText, levelText;
    Board board;

    // backgrounds
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Texture menuTexture;
    sf::Sprite menuSprite;

    // timing cleaning
    sf::Clock cleaningClock;
    sf::Time cleaningDelay = sf::milliseconds(900);

    // Menu UI
    sf::RectangleShape playButton;
    sf::Text playButtonText;
    sf::Text titleText;
    sf::Text dificulty, easy, hard, outText;
    sf::RectangleShape easyB, hardB, outB;

    // Game over UI
    sf::RectangleShape retryButton;
    sf::Text retryButtonText;
    sf::RectangleShape leaveButton;
    sf::Text leaveButtonText;
    sf::Text gameOverText;

    // Next level button (aparece en GAME_OVER si level < 3)
    sf::RectangleShape nextLevelButton;
    sf::Text nextLevelText;

public:
    enum class GameState { MENU, PLAYING, GAME_OVER };
    GameState state = GameState::MENU;
    Game() : board(1) { // start level 1
        sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
        window.setView(view);

        // load font and textures (adjust paths)
        if (!font.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Arimo-font.ttf")) {
            cerr << "Failed to load font (adjust path)\n";
        }

        if (!backgroundTexture.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Fruit background.png")) {
            // optional: ignore
        }
        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setPosition(124.f, 110.f);

        if (!menuTexture.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Menu background.png")) {
            // optional
        }
        menuSprite.setTexture(menuTexture);
        menuSprite.setPosition(0.f, 0.f);
        menuSprite.setScale(1.1f, 1.45f);

        // score/moves texts
        scoreText.setFont(font); scoreText.setCharacterSize(20); scoreText.setFillColor(sf::Color::Black); scoreText.setPosition(20.f, 10.f);
        movesText.setFont(font); movesText.setCharacterSize(20); movesText.setFillColor(sf::Color::Black); movesText.setPosition(300.f, 10.f);
        levelText.setFont(font); levelText.setCharacterSize(20); levelText.setFillColor(sf::Color::Black); levelText.setPosition(520.f, 10.f);

        // Menu UI
        playButton.setSize(sf::Vector2f(200.f, 80.f));
        playButton.setPosition(300.f, 150.f);
        playButton.setFillColor(sf::Color::Green);

        playButtonText.setFont(font);
        playButtonText.setCharacterSize(50);
        playButtonText.setString("JUGAR");
        playButtonText.setFillColor(sf::Color::Black);
        playButtonText.setPosition(playButton.getPosition().x + 20.f, playButton.getPosition().y + 10.f);

        titleText.setFont(font);
        titleText.setCharacterSize(60);
        titleText.setString("MATCH-3");
        titleText.setFillColor(sf::Color::Black);
        titleText.setPosition(270.f, 20.f);

        dificulty.setFont(font);
        dificulty.setCharacterSize(40);
        dificulty.setString("DIFICULTAD");
        dificulty.setFillColor(sf::Color::Black);
        dificulty.setPosition(290.f, 270.f);

        easyB.setSize(sf::Vector2f(220.f, 60.f));
        easyB.setPosition(80.f, 350.f);
        easyB.setFillColor(sf::Color(200, 200, 200, 255));
        easyB.setOutlineThickness(3.f);
        easyB.setOutlineColor(sf::Color::Black);

        easy.setFont(font);
        easy.setCharacterSize(45);
        easy.setString("FACIL");
        easy.setFillColor(sf::Color::Black);
        easy.setPosition(easyB.getPosition().x + 40.f, easyB.getPosition().y);

        hardB.setSize(sf::Vector2f(220.f, 60.f));
        hardB.setPosition(500.f, 350.f);
        hardB.setFillColor(sf::Color(200, 200, 200, 255));
        hardB.setOutlineThickness(3.f);
        hardB.setOutlineColor(sf::Color::Black);

        hard.setFont(font);
        hard.setCharacterSize(45);
        hard.setString("DIFICIL ");
        hard.setFillColor(sf::Color::Black);
        hard.setPosition(hardB.getPosition().x + 40.f, hardB.getPosition().y);

        outB.setSize(sf::Vector2f(200.f, 55.f));
        outB.setPosition(300.f, 450.f);
        outB.setFillColor(sf::Color(200, 200, 200, 255));
        outB.setOutlineThickness(3.f);
        outB.setOutlineColor(sf::Color::Black);

        outText.setFont(font);
        outText.setCharacterSize(40);
        outText.setString("SALIR ");
        outText.setFillColor(sf::Color::Black);
        outText.setPosition(outB.getPosition().x + 40.f, outB.getPosition().y);

        // GAME OVER
        retryButton.setSize(sf::Vector2f(180.f, 60.f));
        retryButton.setOrigin(retryButton.getSize() / 2.f);
        retryButton.setPosition(400.f, 340.f);
        retryButton.setFillColor(sf::Color(200, 200, 200));
        retryButton.setOutlineThickness(3.f);
        retryButton.setOutlineColor(sf::Color::Black);

        retryButtonText.setFont(font);
        retryButtonText.setCharacterSize(23);
        retryButtonText.setString("REINTENTAR");
        retryButtonText.setFillColor(sf::Color::Black);
        retryButtonText.setPosition(retryButton.getPosition().x - 74.f, retryButton.getPosition().y - 20.f + 6.f);

        leaveButton.setSize(sf::Vector2f(180.f, 60.f));
        leaveButton.setOrigin(leaveButton.getSize() / 2.f);
        leaveButton.setPosition(400.f, 420.f);
        leaveButton.setFillColor(sf::Color(200, 200, 200));
        leaveButton.setOutlineThickness(3.f);
        leaveButton.setOutlineColor(sf::Color::Black);

        leaveButtonText.setFont(font);
        leaveButtonText.setCharacterSize(28);
        leaveButtonText.setString("SALIR");
        leaveButtonText.setFillColor(sf::Color::Black);
        leaveButtonText.setPosition(leaveButton.getPosition().x - 40.f, leaveButton.getPosition().y - 25.f + 6.f);

        gameOverText.setFont(font);
        gameOverText.setCharacterSize(44);
        gameOverText.setString("FIN DEL JUEGO");
        gameOverText.setFillColor(sf::Color::Black);
        gameOverText.setPosition(230.f, 80.f);

        // NEXT LEVEL button
        nextLevelButton.setSize(sf::Vector2f(220.f, 60.f));
        nextLevelButton.setOrigin(nextLevelButton.getSize() / 2.f);
        nextLevelButton.setPosition(400.f, 260.f);
        nextLevelButton.setFillColor(sf::Color(180, 220, 180));
        nextLevelButton.setOutlineThickness(3.f);
        nextLevelButton.setOutlineColor(sf::Color::Black);

        nextLevelText.setFont(font);
        nextLevelText.setCharacterSize(24);
        nextLevelText.setString("SIGUIENTE NIVEL");
        nextLevelText.setFillColor(sf::Color::Black);
        nextLevelText.setPosition(nextLevelButton.getPosition().x - 90.f, nextLevelButton.getPosition().y - 12.f);

        window.setFramerateLimit(60);
    }

    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::Resized) {
                sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
                window.setView(view);
                if (menuTexture.getSize().x != 0 && menuTexture.getSize().y != 0) {
                    menuSprite.setScale(800.f / (float)menuTexture.getSize().x, 600.f / (float)menuTexture.getSize().y);
                }
                continue;
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f world = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

                if (state == GameState::MENU) {
                    if (playButton.getGlobalBounds().contains(world)) state = GameState::PLAYING;
                    if (easyB.getGlobalBounds().contains(world)) {
                        easyB.setFillColor(sf::Color::Green);
                        hardB.setFillColor(sf::Color(200, 200, 200, 255));
                        board.resetMoves(10);
                    }
                    if (hardB.getGlobalBounds().contains(world)) {
                        hardB.setFillColor(sf::Color::Green);
                        easyB.setFillColor(sf::Color(200, 200, 200, 255));
                        board.resetMoves(5);
                    }
                    if (outB.getGlobalBounds().contains(world)) window.close();
                }
                else if (state == GameState::PLAYING) {
                    if (board.isCleaning()) {
                        // ignore clicks while cleaning (like earlier)
                    }
                    else {
                        if (!board.hasMoves()) continue;
                        int x = event.mouseButton.x;
                        int y = event.mouseButton.y;
                        auto rc = board.screenToCell(window, x, y);
                        int row = rc.first, col = rc.second;
                        if (row >= 0 && row < 8 && col >= 0 && col < 8) {
                            board.selectOrSwap(row, col);
                        }
                    }
                }
                else if (state == GameState::GAME_OVER) {
                    if (retryButton.getGlobalBounds().contains(world)) {
                        board.resetBoard();
                        state = GameState::PLAYING;
                    }
                    else if (leaveButton.getGlobalBounds().contains(world)) {
                        window.close();
                    }
                    else if (board.getLevel() < 3 && nextLevelButton.getGlobalBounds().contains(world)) {
                        int newLevel = board.getLevel() + 1;
                        board.setLevelPublic(newLevel);
                        board.resetBoard();
                        state = GameState::PLAYING;
                    }
                }
            }
        }
    }

    void update() {
        if (state == GameState::PLAYING) {
            // update gravity animations each frame if active
            if (board.isGravityAnimating()) {
                board.updateAnimations();
            }
            else if (board.isCleaning()) {
                if (cleaningClock.getElapsedTime() >= cleaningDelay) {
                    board.stepCleaning();
                    cleaningClock.restart();
                }
            }

            if (!board.hasMoves() && !board.isCleaning()) {
                state = GameState::GAME_OVER;
            }

            scoreText.setString("PUNTUACIÓN: " + to_string(board.getScore()));
            movesText.setString("MOVIMIENTOS RESTANTES: " + to_string(board.getRemainingMoves()));
            movesText.setPosition(490.f, 10.f);

            levelText.setString("Nivel: " + to_string(board.getLevel()));
            levelText.setPosition(300.f, 10.f);
        }
    }

    void render() {
        window.clear(sf::Color::White);

        if (state == GameState::MENU) {
            if (menuTexture.getSize().x != 0 && menuTexture.getSize().y != 0) window.draw(menuSprite);
            window.draw(titleText);
            window.draw(playButton);
            window.draw(playButtonText);
            window.draw(dificulty);
            window.draw(easyB);
            window.draw(easy);
            window.draw(hardB);
            window.draw(hard);
            window.draw(outB);
            window.draw(outText);
        }
        else if (state == GameState::PLAYING) {
            if (menuTexture.getSize().x != 0 && menuTexture.getSize().y != 0) window.draw(menuSprite);
            if (backgroundTexture.getSize().x != 0 && backgroundTexture.getSize().y != 0) window.draw(backgroundSprite);

            board.draw(window);

            // draw texts
            window.draw(scoreText);
            window.draw(movesText);
            window.draw(levelText);

            // draw objective top-right
            sf::Text objTitle, objProgress;
            objTitle.setFont(font);
            objTitle.setCharacterSize(18);
            objTitle.setFillColor(sf::Color::Black);
            objTitle.setString("OBJETIVO:");
            objTitle.setPosition(560.f, 10.f);

            objProgress.setFont(font);
            objProgress.setCharacterSize(16);
            objProgress.setFillColor(sf::Color::Black);
            objProgress.setString(board.getObjectiveDescription() + "\n" + board.getObjectiveProgressText());
            objProgress.setPosition(560.f, 35.f);

            window.draw(objTitle);
            window.draw(objProgress);
        }
        else if (state == GameState::GAME_OVER) {
            if (menuTexture.getSize().x != 0 && menuTexture.getSize().y != 0) window.draw(menuSprite);

            sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
            overlay.setFillColor(sf::Color(255, 255, 255, 200));
            overlay.setPosition(0.f, 0.f);
            window.draw(overlay);

            window.draw(gameOverText);
            window.draw(retryButton);
            window.draw(retryButtonText);
            window.draw(leaveButton);
            window.draw(leaveButtonText);

            // Si el nivel actual es < 3, mostramos botón next level
            if (board.getLevel() < 3) {
                window.draw(nextLevelButton);
                window.draw(nextLevelText);
            }

            // mostrar progreso del objetivo en pantalla final
            sf::Text objTitle;
            objTitle.setFont(font);
            objTitle.setCharacterSize(20);
            objTitle.setFillColor(sf::Color::Black);
            objTitle.setString("Objetivo cumplido: " + board.getObjectiveProgressText());
            objTitle.setPosition(260.f, 300.f);
            window.draw(objTitle);

            sf::Text finalScore;
            finalScore.setFont(font);
            finalScore.setCharacterSize(28);
            finalScore.setString("Final Score: " + to_string(board.getScore()));
            finalScore.setFillColor(sf::Color::Black);
            finalScore.setPosition(300.f, 200.f);
            window.draw(finalScore);
        }

        window.display();
    }

    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }
};

// ---------------------- main ----------------------
int main() {
    Game g;
    g.run();
    return 0;
}
