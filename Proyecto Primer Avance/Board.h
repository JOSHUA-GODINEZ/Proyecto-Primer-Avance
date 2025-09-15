#pragma once
#include "Fruit.h"
// Stores the board state (8x8) and fruit actions
class Board {
private:
    Fruit* matrix[8][8];
    sf::Texture textures[5];
    int remainingMoves = 3, score = 0;

    // Stop the cleaning process when clearing the board
    bool cleaningInProgress = false;
    bool countScoreDuringCleaning = true;
    // Fruit actions
    bool marked[8][8];
    bool removalPending = false;
    const float MARK_SCALE = 1.35f;
    const float SELECT_SCALE = 1.35f;

    int selectedRow = -1;
    int selectedCol = -1;

    // fixed coordinates used as logical reference system
    const float originX = 117.f;
    const float originY = 100.f;
    const float cellW = 69.f; // cell width
    const float cellH = 60.f; // cell height

public:

    // Initialize variables, load textures, create matrix with random fruits
    // and clear initial combos (without scoring) until the board is stable.
    Board() {
        srand(time(nullptr));
        textures[0].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Apple.png");
        textures[1].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Orange.png");
        textures[2].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Banana.png");
        textures[3].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Watermelon.png");
        textures[4].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Grape.png");
        // Each cell receives a new Fruit with a random texture.
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                int t = std::rand() % 5;
                matrix[i][j] = new Fruit(textures[t], sf::Vector2f(j * cellW + originX, i * cellH + originY), t);
            }
        }
        // This prevents the board from starting with already-resolved combinations.
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

    // Detects if there are any combinations without removing them.
    bool detectCombinationsWithoutRemoving() {
        // horizontals
        for (int i = 0; i < 8; ++i) {
            int count = 1;
            for (int j = 1; j < 8; ++j) {
                if (matrix[i][j] != nullptr && matrix[i][j - 1] != nullptr && (*matrix[i][j]).getType() == (*matrix[i][j - 1]).getType()) {
                    ++count;
                    if (count >= 3) return true; // found 3 equal in a row -> combo
                }
                else {
                    count = 1;
                }
            }
        }
        // verticals
        for (int j = 0; j < 8; ++j) {
            int count = 1;
            for (int i = 1; i < 8; ++i) {
                if (matrix[i][j] != nullptr && matrix[i - 1][j] != nullptr && (*matrix[i][j]).getType() == (*matrix[i - 1][j]).getType()) {
                    ++count;
                    if (count >= 3) return true;
                }
                else {
                    count = 1;
                }
            }
        }
        return false; // no sequence found
    }

    // Only detects whether swapping creates a combo (does not swap permanently)
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

    // This function detects combos and either marks or removes them.
    // Two modes:
    //  * initialization (countScore == false): removes combos without scoring
    //  * game mode (countScore == true):
    //      - if removalPending == false: marks fruits (changes scale) and sets removalPending=true
    //      - if removalPending == true: removes marked fruits, adds score, resets marks.
    bool clearCombosOnce(bool countScore) {
        if (!countScore) {
            // Initialization mode (countScore == false)
            bool anyRemoval = false;
            bool markedLocal[8][8] = { false };

            // horizontals: scan each row for runs of same type
            for (int i = 0; i < 8; ++i) {
                int j = 0;
                while (j < 8) {
                    if (matrix[i][j] == nullptr) { ++j; continue; } // skip empty cells
                    int t = (*matrix[i][j]).getType();
                    int k = j + 1;
                    while (k < 8 && matrix[i][k] != nullptr && (*matrix[i][k]).getType() == t) ++k;
                    int runLen = k - j;
                    if (runLen >= 3) {
                        for (int s = j; s < k; ++s) markedLocal[i][s] = true;
                        anyRemoval = true;
                    }
                    j = k;
                }
            }

            // verticals: same procedure by columns
            for (int j = 0; j < 8; ++j) {
                int i = 0;
                while (i < 8) {
                    if (matrix[i][j] == nullptr) { ++i; continue; }
                    int t = (*matrix[i][j]).getType();
                    int k = i + 1;
                    while (k < 8 && matrix[k][j] != nullptr && (*matrix[k][j]).getType() == t) ++k;
                    int runLen = k - i;
                    if (runLen >= 3) {
                        for (int s = i; s < k; ++s) markedLocal[s][j] = true;
                        anyRemoval = true;
                    }
                    i = k;
                }
            }

            // if removals, delete the fruits physically
            if (anyRemoval) {
                for (int i = 0; i < 8; ++i) {
                    for (int j = 0; j < 8; ++j) {
                        if (markedLocal[i][j] && matrix[i][j] != nullptr) {
                            delete matrix[i][j];
                            matrix[i][j] = nullptr;
                        }
                    }
                }
            }

            return anyRemoval; // true if at least one fruit was removed
        }

        // GAME MODE (countScore == true)
        if (!removalPending) {
            // detect and mark
            bool anyRemoval = false;
            bool markedLocal[8][8] = { false };

            // mark horizontals
            for (int i = 0; i < 8; ++i) {
                int j = 0;
                while (j < 8) {
                    if (matrix[i][j] == nullptr) { ++j; continue; }
                    int t = (*matrix[i][j]).getType();
                    int k = j + 1;
                    while (k < 8 && matrix[i][k] != nullptr && (*matrix[i][k]).getType() == t) ++k;
                    int runLen = k - j;
                    if (runLen >= 3) {
                        for (int s = j; s < k; ++s) markedLocal[i][s] = true;
                        anyRemoval = true;
                    }
                    j = k;
                }
            }

            // mark verticals
            for (int j = 0; j < 8; ++j) {
                int i = 0;
                while (i < 8) {
                    if (matrix[i][j] == nullptr) { ++i; continue; }
                    int t = (*matrix[i][j]).getType();
                    int k = i + 1;
                    while (k < 8 && matrix[k][j] != nullptr && (*matrix[k][j]).getType() == t) ++k;
                    int runLen = k - i;
                    if (runLen >= 3) {
                        for (int s = i; s < k; ++s) markedLocal[s][j] = true;
                        anyRemoval = true;
                    }
                    i = k;
                }
            }

            if (anyRemoval) {
                for (int i = 0; i < 8; ++i) {
                    for (int j = 0; j < 8; ++j) {
                        marked[i][j] = markedLocal[i][j];
                        if (marked[i][j] && matrix[i][j] != nullptr) {
                            (*matrix[i][j]).setScale(MARK_SCALE, MARK_SCALE); // enlarge to mark
                        }
                    }
                }
                removalPending = true;
                return true; // indicates there is work (but not yet removed)
            }
            return false;
        }
        else {
            // remove marked fruits and add score
            bool anyRemoval = false;
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    if (marked[i][j] && matrix[i][j] != nullptr) {
                        delete matrix[i][j];
                        matrix[i][j] = nullptr;
                        anyRemoval = true;
                        score += 10; // add 10 points per fruit removed
                    }
                }
            }

            // clear marks and reset visual aspect of remaining fruits
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    marked[i][j] = false;
                    if (matrix[i][j] != nullptr) (*matrix[i][j]).resetVisual();
                }
            }

            removalPending = false; // finished removal phase
            return anyRemoval; // whether at least one fruit was removed
        }
    }

    // Makes fruits "fall" and fills with new fruits on top when there are empty cells.
    void applyGravityAndReplace() {
        for (int col = 0; col < 8; ++col) {
            // Apply gravity
            for (int row = 7; row >= 0; --row) {
                if (matrix[row][col] == nullptr) {
                    int k = row - 1;
                    while (k >= 0 && matrix[k][col] == nullptr) --k;
                    if (k >= 0) {
                        // move pointer to lower position
                        matrix[row][col] = matrix[k][col];
                        matrix[k][col] = nullptr;
                        (*matrix[row][col]).setPosition(col * cellW + originX, row * cellH + originY);
                        (*matrix[row][col]).resetVisual();
                    }
                }
            }
            // fill empty spaces
            for (int row = 0; row < 8; ++row) {
                if (matrix[row][col] == nullptr) {
                    int t = std::rand() % 5;
                    matrix[row][col] = new Fruit(textures[t], sf::Vector2f(col * cellW + originX, row * cellH + originY), t);
                }
            }
        }
    }

    // Destructor: free memory of all fruits
    ~Board() {
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                if (matrix[i][j] != nullptr) delete matrix[i][j];
    }

    // Draw all fruits on screen.
    void draw(sf::RenderWindow& window) {
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                if (matrix[i][j] != nullptr)
                    (*matrix[i][j]).draw(window);
    }

    // Tries to swap two adjacent cells if a combination is formed.
    // starts the cleaning process (startCleaning).
    bool swapCells(int r1, int c1, int r2, int c2) {

        if (r1 < 0 || r1 >= 8 || c1 < 0 || c1 >= 8 ||
            r2 < 0 || r2 >= 8 || c2 < 0 || c2 >= 8) return false;

        // check adjacency.
        int dr = r1 - r2; if (dr < 0) dr *= -1;
        int dc = c1 - c2; if (dc < 0) dc *= -1;
        if (!((dr == 1 && dc == 0) || (dr == 0 && dc == 1))) return false;

        // validate if the swap produces a combo
        if (!formsCombinationWhenSwapping(r1, c1, r2, c2)) return false;

        Fruit* temp = matrix[r1][c1];
        matrix[r1][c1] = matrix[r2][c2];
        matrix[r2][c2] = temp;
        // update visual positions (top-left coords)
        if (matrix[r1][c1] != nullptr) (*matrix[r1][c1]).setPosition(c1 * cellW + originX, r1 * cellH + originY);
        if (matrix[r2][c2] != nullptr) (*matrix[r2][c2]).setPosition(c2 * cellW + originX, r2 * cellH + originY);

        --remainingMoves;
        startCleaning(true);
        return true;
    }

    // Marks that the board enters "cleaning" mode and defines whether
    // scoring should be counted during cleaning.
    void startCleaning(bool countScore) {
        cleaningInProgress = true;
        countScoreDuringCleaning = countScore;
    }

    // Executes one step of cleaning (called repeatedly with delay)
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
            cleaningInProgress = false; // nothing left to clean
            return false;
        }
    }

    // Query if cleaning is in progress.
    bool isCleaning() const { return cleaningInProgress; }

    // execute full cleaning until no combos remain.
    void clearCombosRepeatedly(bool countScore = true) {
        bool cont = true;
        while (cont) {
            if (clearCombosOnce(countScore)) {
                applyGravityAndReplace();
            }
            else cont = false;
        }
    }

    // Accessors for score and moves
    int getScore() const { return score; }
    int getRemainingMoves() const { return remainingMoves; }
    bool hasMoves() const { return remainingMoves > 0; }

    void resetMoves(int n) { remainingMoves = n; }

    // Selection control logic by clicks:
    // - first click: selects the fruit (scales it)
    // - second click:
    //    * if same cell -> deselect
    //    * if adjacent -> attempts to swap (and if valid, processes cleaning)
    void selectOrSwap(int row, int col) {
        if (selectedRow == -1) {
            selectedRow = row; selectedCol = col;
            (*matrix[selectedRow][selectedCol]).setScale(SELECT_SCALE, SELECT_SCALE);
        }
        else {
            bool ok = swapCells(selectedRow, selectedCol, row, col);
            (*matrix[selectedRow][selectedCol]).resetVisual();
            selectedRow = -1; selectedCol = -1;
        }
    }

    // Converts mouse position to row/col indices (SMFL).
    pair<int, int> screenToCell(sf::RenderWindow& win, int mouseX, int mouseY) {
        sf::Vector2f world = win.mapPixelToCoords(sf::Vector2i(mouseX, mouseY));
        int col = int(std::floor((world.x - originX) / cellW));
        int row = int(std::floor((world.y - originY) / cellH));
        return { row, col };
    }

    // Deletes the board and creates a new one like in the constructor
    void resetBoard(int newMoves = 20) {
        remainingMoves = newMoves;
        score = 0;
        cleaningInProgress = false;
        countScoreDuringCleaning = true;
        removalPending = false;
        selectedRow = -1; selectedCol = -1;

        // delete current fruits (free memory)
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (matrix[i][j] != nullptr) {
                    delete matrix[i][j];
                    matrix[i][j] = nullptr;
                }
            }
        }

        // create new matrix with random fruits
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                int t = std::rand() % 5;
                matrix[i][j] = new Fruit(textures[t], sf::Vector2f(j * cellW + originX, i * cellH + originY), t);
            }
        }

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
};
