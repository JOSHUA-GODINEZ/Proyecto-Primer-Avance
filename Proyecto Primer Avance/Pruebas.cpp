// main.cpp
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>

using namespace std;

// Forward declaration
class Board;

// ------------------------
// Fruit & subclasses
// ------------------------
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

    // Contextual reaction when this fruit is matched/removed
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

class NormalFruit : public Fruit {
public:
    NormalFruit(sf::Texture& tex, sf::Vector2f pos, int t) : Fruit(tex, pos, t) {}
    void onMatch(Board* /*board*/, int /*row*/, int /*col*/) override {
        // Normal fruit: no side effects (scoring done by Board)
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
    void onMatch(Board* /*board*/, int /*row*/, int /*col*/) override {
        // Ice special handled by Board (mostly blocking direct removal unless combo or forced)
    }
};

// ------------------------
// Board
// ------------------------
class Board {
private:
    static constexpr int SIZE = 8;
    static constexpr int NUM_NORMAL = 5;      // textures 0..4
    static constexpr int BOMB_INDEX = 5;      // textures[5]
    static constexpr int ICE_INDEX = 6;       // textures[6]
    static constexpr int NUM_TEXTURES = 7;    // total textures loaded

    Fruit* matrix[SIZE][SIZE];
    std::vector<sf::Texture> textures;
    int remainingMoves = 3;
    int score = 0;
    int level = 1;

    bool cleaningInProgress = false;
    bool countScoreDuringCleaning = true;

    bool marked[SIZE][SIZE];
    bool comboMarked[SIZE][SIZE];   // true if the mark originated from a combo detection
    bool forcedBreak[SIZE][SIZE];   // true if some effect (e.g. bomb) forces ice to break
    bool removalPending = false;

    const float MARK_SCALE = 1.35f;
    const float SELECT_SCALE = 1.35f;

    int selectedRow = -1;
    int selectedCol = -1;

    // Layout
    const float originX = 117.f;
    const float originY = 100.f;
    const float cellW = 69.f;
    const float cellH = 60.f;

public:
    Board() {
        srand(static_cast<unsigned int>(time(nullptr)));

        // init pointers and flags
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                matrix[i][j] = nullptr;
                marked[i][j] = false;
                comboMarked[i][j] = false;
                forcedBreak[i][j] = false;
            }
        }

        // prepare textures vector
        textures.resize(NUM_TEXTURES);
        bool allGood = true;
        if (!textures[0].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Apple.png")) { cerr << "Error loading textures[0]\n"; allGood = false; }
        if (!textures[1].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Orange.png")) { cerr << "Error loading textures[1]\n"; allGood = false; }
        if (!textures[2].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Banana.png")) { cerr << "Error loading textures[2]\n"; allGood = false; }
        if (!textures[3].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Watermelon.png")) { cerr << "Error loading textures[3]\n"; allGood = false; }
        if (!textures[4].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Grape.png")) { cerr << "Error loading textures[4]\n"; allGood = false; }
        if (!textures[5].loadFromFile("C:\\Joshua\\practica\\Sprites\\preuba bomba.jpg")) { cerr << "Error loading textures[5]\n"; allGood = false; }
        if (!textures[6].loadFromFile("C:\\Joshua\\practica\\Sprites\\hielo pureba.jpg")) { cerr << "Error loading textures[6]\n"; allGood = false; }

        if (!allGood) {
            cerr << "Warning: some textures failed to load. Visuals may be broken.\n";
        }

        // Fill board with fruits (some chance for bomb/ice)
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                sf::Vector2f pos(j * cellW + originX, i * cellH + originY);
                int roll = std::rand() % 100;
                if (roll < 5) { // 5% bomb
                    matrix[i][j] = createFruitByType(BOMB_INDEX, textures[BOMB_INDEX], pos);
                }
                else if (roll < 10) { // 5% ice
                    matrix[i][j] = createFruitByType(ICE_INDEX, textures[ICE_INDEX], pos);
                }
                else {
                    int t = std::rand() % NUM_NORMAL;
                    matrix[i][j] = createFruitByType(t, textures[t], pos);
                }
            }
        }

        // Remove initial combos until stable (no scoring)
        bool cont = true;
        while (cont) {
            if (clearCombosOnce(false)) {
                applyGravityAndReplace();
            }
            else {
                cont = false;
            }
        }
    }

    ~Board() {
        for (int i = 0; i < SIZE; ++i)
            for (int j = 0; j < SIZE; ++j)
                if (matrix[i][j] != nullptr) delete matrix[i][j];
    }

    // Factory: create correct subclass
    Fruit* createFruitByType(int logicalType, sf::Texture& tex, sf::Vector2f pos) {
        if (logicalType == BOMB_INDEX) return new BombFruit(tex, pos, logicalType);
        if (logicalType == ICE_INDEX) return new IceFruit(tex, pos, logicalType);
        return new NormalFruit(tex, pos, logicalType);
    }

    // mark a cell for removal. If 'forceBreakIce' is true, ice there can be broken even if not combo.
    void markCellForRemoval(int r, int c, bool forceBreakIce = false) {
        if (r >= 0 && r < SIZE && c >= 0 && c < SIZE) {
            marked[r][c] = true;
            if (forceBreakIce) forcedBreak[r][c] = true;
            removalPending = true;
        }
    }

    // detect combos without removing
    bool detectCombinationsWithoutRemoving() {
        // horizontals
        for (int i = 0; i < SIZE; ++i) {
            int count = 1;
            for (int j = 1; j < SIZE; ++j) {
                if (matrix[i][j] != nullptr && matrix[i][j - 1] != nullptr && matrix[i][j]->getType() == matrix[i][j - 1]->getType()) {
                    ++count;
                    if (count >= 3) return true;
                }
                else count = 1;
            }
        }
        // verticals
        for (int j = 0; j < SIZE; ++j) {
            int count = 1;
            for (int i = 1; i < SIZE; ++i) {
                if (matrix[i][j] != nullptr && matrix[i - 1][j] != nullptr && matrix[i][j]->getType() == matrix[i - 1][j]->getType()) {
                    ++count;
                    if (count >= 3) return true;
                }
                else count = 1;
            }
        }
        return false;
    }

    bool formsCombinationWhenSwapping(int r1, int c1, int r2, int c2) {
        Fruit* temp = matrix[r1][c1];
        matrix[r1][c1] = matrix[r2][c2];
        matrix[r2][c2] = temp;
        bool hasCombo = detectCombinationsWithoutRemoving();
        temp = matrix[r1][c1];
        matrix[r1][c1] = matrix[r2][c2];
        matrix[r2][c2] = temp;
        return hasCombo;
    }

    // clear combos once (two modes as previously: init vs game)
    bool clearCombosOnce(bool countScore) {
        if (!countScore) {
            // initialization: delete combos without scoring
            bool anyRemoval = false;
            bool markedLocal[SIZE][SIZE] = { false };
            bool comboLocal[SIZE][SIZE] = { false }; // marcas que vienen de combos

            // horizontals
            for (int i = 0; i < SIZE; ++i) {
                int j = 0;
                while (j < SIZE) {
                    if (matrix[i][j] == nullptr) { ++j; continue; }
                    int t = matrix[i][j]->getType();
                    int k = j + 1;
                    while (k < SIZE && matrix[i][k] != nullptr && matrix[i][k]->getType() == t) ++k;
                    int runLen = k - j;
                    if (runLen >= 3) {
                        // marcar la run
                        for (int s = j; s < k; ++s) {
                            markedLocal[i][s] = true;
                            comboLocal[i][s] = true; // proviene de combo
                            // además marcar hielos alrededor de cada celda de la run
                            for (int dr = -1; dr <= 1; ++dr) {
                                for (int dc = -1; dc <= 1; ++dc) {
                                    int nr = i + dr;
                                    int nc = s + dc;
                                    if (nr < 0 || nr >= SIZE || nc < 0 || nc >= SIZE) continue;
                                    if (matrix[nr][nc] != nullptr && matrix[nr][nc]->getType() == ICE_INDEX) {
                                        markedLocal[nr][nc] = true;
                                        comboLocal[nr][nc] = true;
                                    }
                                }
                            }
                        }
                        anyRemoval = true;
                    }
                    j = k;
                }
            }

            // verticals
            for (int j = 0; j < SIZE; ++j) {
                int i = 0;
                while (i < SIZE) {
                    if (matrix[i][j] == nullptr) { ++i; continue; }
                    int t = matrix[i][j]->getType();
                    int k = i + 1;
                    while (k < SIZE && matrix[k][j] != nullptr && matrix[k][j]->getType() == t) ++k;
                    int runLen = k - i;
                    if (runLen >= 3) {
                        for (int s = i; s < k; ++s) {
                            markedLocal[s][j] = true;
                            comboLocal[s][j] = true;
                            for (int dr = -1; dr <= 1; ++dr) {
                                for (int dc = -1; dc <= 1; ++dc) {
                                    int nr = s + dr;
                                    int nc = j + dc;
                                    if (nr < 0 || nr >= SIZE || nc < 0 || nc >= SIZE) continue;
                                    if (matrix[nr][nc] != nullptr && matrix[nr][nc]->getType() == ICE_INDEX) {
                                        markedLocal[nr][nc] = true;
                                        comboLocal[nr][nc] = true;
                                    }
                                }
                            }
                        }
                        anyRemoval = true;
                    }
                    i = k;
                }
            }

            if (anyRemoval) {
                for (int i = 0; i < SIZE; ++i) {
                    for (int j = 0; j < SIZE; ++j) {
                        if (markedLocal[i][j] && matrix[i][j] != nullptr) {
                            delete matrix[i][j];
                            matrix[i][j] = nullptr;
                        }
                        // opcional: si quieres propagar comboMarked durante init, asigna comboMarked aquí también
                        comboMarked[i][j] = comboLocal[i][j];
                    }
                }
            }
            return anyRemoval;
        }

        // GAME MODE
        if (!removalPending) {
            // detect and mark
            bool anyRemoval = false;
            bool markedLocal[SIZE][SIZE] = { false };
            bool comboLocal[SIZE][SIZE] = { false };

            // mark horizontals
            for (int i = 0; i < SIZE; ++i) {
                int j = 0;
                while (j < SIZE) {
                    if (matrix[i][j] == nullptr) { ++j; continue; }
                    int t = matrix[i][j]->getType();
                    int k = j + 1;
                    while (k < SIZE && matrix[i][k] != nullptr && matrix[i][k]->getType() == t) ++k;
                    int runLen = k - j;
                    if (runLen >= 3) {
                        for (int s = j; s < k; ++s) {
                            markedLocal[i][s] = true;
                            comboLocal[i][s] = true; // viene de combo
                            // marcar hielos alrededor de cada celda de la run
                            for (int dr = -1; dr <= 1; ++dr) {
                                for (int dc = -1; dc <= 1; ++dc) {
                                    int nr = i + dr;
                                    int nc = s + dc;
                                    if (nr < 0 || nr >= SIZE || nc < 0 || nc >= SIZE) continue;
                                    if (matrix[nr][nc] != nullptr && matrix[nr][nc]->getType() == ICE_INDEX) {
                                        markedLocal[nr][nc] = true;
                                        comboLocal[nr][nc] = true;
                                    }
                                }
                            }
                        }
                        anyRemoval = true;
                    }
                    j = k;
                }
            }

            // mark verticals
            for (int j = 0; j < SIZE; ++j) {
                int i = 0;
                while (i < SIZE) {
                    if (matrix[i][j] == nullptr) { ++i; continue; }
                    int t = matrix[i][j]->getType();
                    int k = i + 1;
                    while (k < SIZE && matrix[k][j] != nullptr && matrix[k][j]->getType() == t) ++k;
                    int runLen = k - i;
                    if (runLen >= 3) {
                        for (int s = i; s < k; ++s) {
                            markedLocal[s][j] = true;
                            comboLocal[s][j] = true; // viene de combo
                            // marcar hielos alrededor de cada celda de la run
                            for (int dr = -1; dr <= 1; ++dr) {
                                for (int dc = -1; dc <= 1; ++dc) {
                                    int nr = s + dr;
                                    int nc = j + dc;
                                    if (nr < 0 || nr >= SIZE || nc < 0 || nc >= SIZE) continue;
                                    if (matrix[nr][nc] != nullptr && matrix[nr][nc]->getType() == ICE_INDEX) {
                                        markedLocal[nr][nc] = true;
                                        comboLocal[nr][nc] = true;
                                    }
                                }
                            }
                        }
                        anyRemoval = true;
                    }
                    i = k;
                }
            }

            if (anyRemoval) {
                for (int i = 0; i < SIZE; ++i) {
                    for (int j = 0; j < SIZE; ++j) {
                        marked[i][j] = markedLocal[i][j];
                        // copia comboLocal -> comboMarked para indicar origen combo
                        comboMarked[i][j] = comboLocal[i][j];
                        if (marked[i][j] && matrix[i][j] != nullptr) {
                            matrix[i][j]->setScale(MARK_SCALE, MARK_SCALE);
                        }
                    }
                }
                removalPending = true;
                return true;
            }
            return false;
        }
        else {
            // removalPending == true: process onMatch (chain reactions), then delete
            bool anyRemoval = false;

            // processed flags to avoid repeating onMatch for same cell
            bool processed[SIZE][SIZE] = { false };

            bool newProcessing = true;
            while (newProcessing) {
                newProcessing = false;
                for (int i = 0; i < SIZE; ++i) {
                    for (int j = 0; j < SIZE; ++j) {
                        if (marked[i][j] && !processed[i][j] && matrix[i][j] != nullptr) {
                            bool isIce = (matrix[i][j]->getType() == ICE_INDEX);
                            // Ejecutar onMatch si:
                            //  - no es hielo; o
                            //  - es hielo y fue marcado por combo; o
                            //  - es hielo y fue marcado con fuerza (p.ej. por bomba)
                            if (isIce && !comboMarked[i][j] && !forcedBreak[i][j]) {
                                // hielo marcado por bomba u otro efecto no rompible -> no ejecutar onMatch
                                processed[i][j] = true;
                                continue;
                            }
                            matrix[i][j]->onMatch(this, i, j);
                            processed[i][j] = true;
                            newProcessing = true;
                        }
                    }
                }
            }

            // Now delete all marked fruits and add score
            for (int i = 0; i < SIZE; ++i) {
                for (int j = 0; j < SIZE; ++j) {
                    if (marked[i][j] && matrix[i][j] != nullptr) {
                        bool isIce = (matrix[i][j]->getType() == ICE_INDEX);
                        if (isIce && !comboMarked[i][j] && !forcedBreak[i][j]) {
                            // skip deletion: ice not eligible
                            continue;
                        }
                        delete matrix[i][j];
                        matrix[i][j] = nullptr;
                        anyRemoval = true;
                        score += 10; // per fruit
                    }
                }
            }

            // clear marks and reset visuals
            for (int i = 0; i < SIZE; ++i) {
                for (int j = 0; j < SIZE; ++j) {
                    marked[i][j] = false;
                    comboMarked[i][j] = false;
                    forcedBreak[i][j] = false;
                    if (matrix[i][j] != nullptr) matrix[i][j]->resetVisual();
                }
            }

            removalPending = false;
            return anyRemoval;
        }
    }

    void applyGravityAndReplace() {
        for (int col = 0; col < SIZE; ++col) {
            // gravity
            for (int row = SIZE - 1; row >= 0; --row) {
                if (matrix[row][col] == nullptr) {
                    int k = row - 1;
                    while (k >= 0 && matrix[k][col] == nullptr) --k;
                    if (k >= 0) {
                        matrix[row][col] = matrix[k][col];
                        matrix[k][col] = nullptr;
                        matrix[row][col]->setPosition(col * cellW + originX, row * cellH + originY);
                        matrix[row][col]->resetVisual();
                    }
                }
            }
            // fill empty
            for (int row = 0; row < SIZE; ++row) {
                if (matrix[row][col] == nullptr) {
                    int roll = std::rand() % 100;
                    sf::Vector2f pos(col * cellW + originX, row * cellH + originY);
                    if (roll < 5) {
                        matrix[row][col] = createFruitByType(BOMB_INDEX, textures[BOMB_INDEX], pos);
                    }
                    else if (roll < 10) {
                        matrix[row][col] = createFruitByType(ICE_INDEX, textures[ICE_INDEX], pos);
                    }
                    else {
                        int t = std::rand() % NUM_NORMAL;
                        matrix[row][col] = createFruitByType(t, textures[t], pos);
                    }
                }
            }
        }
    }

    // swap attempts
    bool swapCells(int r1, int c1, int r2, int c2) {
        if (r1 < 0 || r1 >= SIZE || c1 < 0 || c1 >= SIZE ||
            r2 < 0 || r2 >= SIZE || c2 < 0 || c2 >= SIZE) return false;

        int dr = abs(r1 - r2), dc = abs(c1 - c2);
        if (!((dr == 1 && dc == 0) || (dr == 0 && dc == 1))) return false;

        if (!formsCombinationWhenSwapping(r1, c1, r2, c2)) return false;

        Fruit* temp = matrix[r1][c1];
        matrix[r1][c1] = matrix[r2][c2];
        matrix[r2][c2] = temp;

        if (matrix[r1][c1] != nullptr) matrix[r1][c1]->setPosition(c1 * cellW + originX, r1 * cellH + originY);
        if (matrix[r2][c2] != nullptr) matrix[r2][c2]->setPosition(c2 * cellW + originX, r2 * cellH + originY);

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
        bool didSomething = clearCombosOnce(countScoreDuringCleaning);
        if (didSomething) {
            if (removalPending) {
                return true;
            }
            else {
                applyGravityAndReplace();
                return true;
            }
        }
        else {
            cleaningInProgress = false;
            return false;
        }
    }

    bool isCleaning() const { return cleaningInProgress; }

    void clearCombosRepeatedly(bool countScore = true) {
        bool cont = true;
        while (cont) {
            if (clearCombosOnce(countScore)) {
                applyGravityAndReplace();
            }
            else cont = false;
        }
    }

    int getScore() const { return score; }
    int getRemainingMoves() const { return remainingMoves; }
    bool hasMoves() const { return remainingMoves > 0; }
    void setLevel(int l) { level = l; }
    int getLevel() const { return level; }
    void resetMoves(int n) { remainingMoves = n; }

    // Selection and swapping, plus bomb-on-click behavior
    void selectOrSwap(int row, int col) {
        if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) return;
        if (matrix[row][col] == nullptr) return;

        // If no selection yet
        if (selectedRow == -1) {
            // If clicked a bomb -> detonate immediately (consume no selection)
            BombFruit* asBomb = dynamic_cast<BombFruit*>(matrix[row][col]);
            if (asBomb != nullptr) {
                // mark own cell for removal and force-break neighbors
                markCellForRemoval(row, col, true); // own cell forced
                asBomb->onMatch(this, row, col);    // mark neighbors with force
                removalPending = true;
                startCleaning(true);
                return;
            }

            // otherwise select normally
            selectedRow = row;
            selectedCol = col;
            matrix[selectedRow][selectedCol]->setScale(SELECT_SCALE, SELECT_SCALE);
            return;
        }

        // If clicked same cell -> deselect
        if (selectedRow == row && selectedCol == col) {
            if (matrix[selectedRow][selectedCol] != nullptr)
                matrix[selectedRow][selectedCol]->resetVisual();
            selectedRow = -1; selectedCol = -1;
            return;
        }

        // reset visual and attempt swap
        if (matrix[selectedRow][selectedCol] != nullptr)
            matrix[selectedRow][selectedCol]->resetVisual();

        bool ok = swapCells(selectedRow, selectedCol, row, col);

        selectedRow = -1; selectedCol = -1;
    }

    pair<int, int> screenToCell(sf::RenderWindow& win, int mouseX, int mouseY) {
        sf::Vector2f world = win.mapPixelToCoords(sf::Vector2i(mouseX, mouseY));
        int col = int(std::floor((world.x - originX) / cellW));
        int row = int(std::floor((world.y - originY) / cellH));
        return { row, col };
    }

    void resetBoard() {
        remainingMoves = 5;
        score = 0;
        cleaningInProgress = false;
        countScoreDuringCleaning = true;
        removalPending = false;
        selectedRow = -1; selectedCol = -1;

        for (int i = 0; i < SIZE; ++i)
            for (int j = 0; j < SIZE; ++j) {
                if (matrix[i][j] != nullptr) {
                    delete matrix[i][j];
                    matrix[i][j] = nullptr;
                }
                marked[i][j] = false;
                comboMarked[i][j] = false;
                forcedBreak[i][j] = false;
            }

        // create new matrix with random fruits
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                int  t1 = rand() % 20;
                if (t1 == 5) {
                    matrix[i][j] = new BombFruit(textures[5], sf::Vector2f(j * cellW + originX, i * cellH + originY), t1);
                }
                else {
                    int t = std::rand() % 5;
                    matrix[i][j] = new Fruit(textures[t], sf::Vector2f(j * cellW + originX, i * cellH + originY), t);
                }
            }
        }

        bool cont = true;
        while (cont) {
            if (clearCombosOnce(false)) applyGravityAndReplace();
            else cont = false;
        }
    }

    // Drawing
    void draw(sf::RenderWindow& window) {
        for (int i = 0; i < SIZE; ++i)
            for (int j = 0; j < SIZE; ++j)
                if (matrix[i][j] != nullptr)
                    matrix[i][j]->draw(window);
    }
};

// ------------------------
// BombFruit onMatch implementation
// ------------------------
void BombFruit::onMatch(Board* board, int row, int col) {
    // mark the 8 neighbors, forcing ice to break (3x3 excluding center)
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = row + dr;
            int nc = col + dc;
            board->markCellForRemoval(nr, nc, true); // forceBreak = true
        }
    }
}

// ------------------------
// Game class (UI + loop)
// ------------------------
class Game {
private:
    sf::RenderWindow window{ sf::VideoMode(800, 600), "Match-3" };
    sf::Font font;
    sf::Text scoreText;
    sf::Text movesText;
    sf::Text levelText;
    Board board;

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Texture backgroundTexture1;
    sf::Sprite backgroundSprite1;

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

    enum class GameState { MENU, PLAYING, GAME_OVER };
    GameState state = GameState::MENU;

public:
    Game() {
        sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
        window.setView(view);

        if (!font.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Arimo-font.ttf")) {
            cerr << "Error loading font\n";
        }

        if (!backgroundTexture.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Fruit background.png")) {
            // warning handled earlier
        }
        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setPosition(124.f, 110.f);

        if (!backgroundTexture1.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Menu background.png")) {
            // ignore
        }
        backgroundSprite1.setTexture(backgroundTexture1);
        backgroundSprite1.setPosition(0.f, 0.f);
        backgroundSprite1.setScale(1.1f, 1.45f);

        // texts
        scoreText.setFont(font);
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(sf::Color::Black);
        scoreText.setPosition(20.f, 10.f);

        movesText.setFont(font);
        movesText.setCharacterSize(20);
        movesText.setFillColor(sf::Color::Black);
        movesText.setPosition(300.f, 10.f);

        levelText.setFont(font);
        levelText.setCharacterSize(20);
        levelText.setFillColor(sf::Color::Black);
        levelText.setPosition(520.f, 10.f);

        // menu UI
        playButton.setSize(sf::Vector2f(200.f, 80.f));
        playButton.setPosition(300.f, 150.f);
        playButton.setFillColor(sf::Color::Green);

        playButtonText.setFont(font);
        playButtonText.setCharacterSize(50);
        playButtonText.setString("JUGAR");
        playButtonText.setFillColor(sf::Color::Black);
        playButtonText.setPosition(playButton.getPosition().x + 20, playButton.getPosition().y + 10);

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
        easy.setPosition(easyB.getPosition().x + 40, easyB.getPosition().y);

        hardB.setSize(sf::Vector2f(220.f, 60.f));
        hardB.setPosition(500.f, 350.f);
        hardB.setFillColor(sf::Color(200, 200, 200, 255));
        hardB.setOutlineThickness(3.f);
        hardB.setOutlineColor(sf::Color::Black);

        hard.setFont(font);
        hard.setCharacterSize(45);
        hard.setString("DIFICIL ");
        hard.setFillColor(sf::Color::Black);
        hard.setPosition(hardB.getPosition().x + 40, hardB.getPosition().y);

        outB.setSize(sf::Vector2f(200.f, 55.f));
        outB.setPosition(300.f, 450.f);
        outB.setFillColor(sf::Color(200, 200, 200, 255));
        outB.setOutlineThickness(3.f);
        outB.setOutlineColor(sf::Color::Black);

        outText.setFont(font);
        outText.setCharacterSize(40);
        outText.setString("SALIR ");
        outText.setFillColor(sf::Color::Black);
        outText.setPosition(outB.getPosition().x + 40, outB.getPosition().y);

        // game over UI
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
        retryButtonText.setPosition(retryButton.getPosition().x - 74, retryButton.getPosition().y - 20 + 6.f);

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
        leaveButtonText.setPosition(leaveButton.getPosition().x - 40, leaveButton.getPosition().y - 25 + 6.f);

        gameOverText.setFont(font);
        gameOverText.setCharacterSize(44);
        gameOverText.setString("FIN DEL JUEGO");
        gameOverText.setFillColor(sf::Color::Black);
        gameOverText.setPosition(230.f, 80.f);

        window.setFramerateLimit(60);
    }

    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::Resized) {
                sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
                window.setView(view);
                if (backgroundTexture1.getSize().x != 0 && backgroundTexture1.getSize().y != 0) {
                    backgroundSprite1.setScale(800.f / (float)backgroundTexture1.getSize().x, 600.f / (float)backgroundTexture1.getSize().y);
                }
                continue;
            }

            if (state == GameState::MENU) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f world = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    if (playButton.getGlobalBounds().contains(world)) {
                        state = GameState::PLAYING;
                    }
                    if (easyB.getGlobalBounds().contains(world)) {
                        easyB.setFillColor(sf::Color::Green);
                        hardB.setFillColor(sf::Color(200, 200, 200, 255));
                    }
                    if (hardB.getGlobalBounds().contains(world)) {
                        hardB.setFillColor(sf::Color::Green);
                        easyB.setFillColor(sf::Color(200, 200, 200, 255));
                    }
                    if (outB.getGlobalBounds().contains(world)) {
                        window.close();
                    }
                }
            }
            else if (state == GameState::PLAYING) {
                if (board.isCleaning()) continue;

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
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
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f world = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    if (retryButton.getGlobalBounds().contains(world)) {
                        board.resetBoard();
                        state = GameState::PLAYING;
                    }
                    else if (leaveButton.getGlobalBounds().contains(world)) {
                        window.close();
                    }
                }
            }
        }
    }

    void update() {
        if (state == GameState::PLAYING) {
            if (board.isCleaning()) {
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
            movesText.setPosition(300.f, 10.f);
            levelText.setString("Nivel: " + to_string(board.getLevel()));
            levelText.setPosition(520.f, 10.f);
        }
    }

    void render() {
        window.clear(sf::Color::White);

        if (state == GameState::MENU) {
            if (backgroundTexture1.getSize().x != 0 && backgroundTexture1.getSize().y != 0) window.draw(backgroundSprite1);
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
            if (backgroundTexture1.getSize().x != 0 && backgroundTexture1.getSize().y != 0) window.draw(backgroundSprite1);
            if (backgroundTexture.getSize().x != 0 && backgroundTexture.getSize().y != 0) window.draw(backgroundSprite);

            board.draw(window);

            window.draw(scoreText);
            window.draw(movesText);
            window.draw(levelText);
        }
        else if (state == GameState::GAME_OVER) {
            if (backgroundTexture1.getSize().x != 0 && backgroundTexture1.getSize().y != 0) window.draw(backgroundSprite1);

            sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
            overlay.setFillColor(sf::Color(255, 255, 255, 200));
            overlay.setPosition(0.f, 0.f);
            window.draw(overlay);

            window.draw(gameOverText);
            window.draw(retryButton);
            window.draw(retryButtonText);
            window.draw(leaveButton);
            window.draw(leaveButtonText);

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

// ------------------------
// main
// ------------------------
int main() {
    Game g;
    g.run();
    return 0;
}
