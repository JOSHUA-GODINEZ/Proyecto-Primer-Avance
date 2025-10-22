#include "Fruit.h"
class Board {
private:
    static constexpr int SIZE = 8;
    static constexpr int NUM_NORMAL = 5;   // textures 0..4 are normal fruit types
    static constexpr int BOMB_INDEX = 5;   // index texture for bomb
    static constexpr int ICE_INDEX = 6;    // index for ice
    static constexpr int NUM_TEXTURES = 12; // total loaded textures

    Fruit* matrix[8][8];
    sf::Texture textures[12];

    // layout logical
    const float originX = 121.f;
    const float originY = 109.f;
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
    int remainingMoves = 3; // fixes: default 10 and no variation by difficulty
    int score = 0;
    int level = 1;
    int super = -1;       
    int superR = -1;      
    int superC = -1;      

    // gravity animation
    bool gravityAnimating = false;
    float gravityDurationMs = 500.f; 

    // swap animation
    bool swapAnimating = false;
    bool pendingSwapCleaning = false;
    float swapDurationMs = 200.f; // duración base del swap en ms



    const float MARK_SCALE = 1.5f;
    const float SELECT_SCALE = 1.25f;

    int selectedRow = -1, selectedCol = -1;

    //Ajustes de probabilidades por nivel
    int initBombChance[4] = { 0, 5, 3, 1 };   // índice 1..3 válido
    int initIceChance[4] = { 0, 5, 12, 20 };
    int refillBombChance[4] = { 0, 5, 3, 1 };


   // Objetivo del nivel 
    // kinds: 1 = eliminar tipo normal X (count), 2 = eliminar hielos (count), 3 = alcanzar puntos
    int objectiveKind = 0;
    int objectiveTarget = 0;      // target count or target points
    int objectiveProgress = 0;    // current progress
    int objectiveTargetFruit = -1; // for kind==1, which normal fruit (0..NUM_NORMAL-1)
    sf::Clock animClock;
public:
    Board() {
        srand((time(0)));


        if (!textures[0].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Apple.png")) cerr << "Failed to load textures[0]\n";
        if (!textures[1].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Orange.png")) cerr << "Failed to load textures[1]\n";
        if (!textures[2].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Banana.png")) cerr << "Failed to load textures[2]\n";
        if (!textures[3].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Watermelon.png")) cerr << "Failed to load textures[3]\n";
        if (!textures[4].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Grape.png")) cerr << "Failed to load textures[4]\n";
        if (!textures[5].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Bomba.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[6].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\cubo hielo.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[7].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\manzanaD1.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[8].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Naranja de oro12.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[9].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Banano dorado12.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[10].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\sandia dorada12.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[11].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Unvas doradas12.png")) cerr << "Failed to load textures[5]\n";


        initObjectiveForLevel(level);

        for (int r = 0; r < SIZE; ++r)
            for (int c = 0; c < SIZE; ++c) {
                matrix[r][c] = nullptr;
                marked[r][c] = comboMarked[r][c] = forcedBreak[r][c] = false;
            }

      
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

    ~Board() {
        for (int r = 0; r < SIZE; ++r)
            for (int c = 0; c < SIZE; ++c)
                if (matrix[r][c] != nullptr) { delete matrix[r][c]; }
    }
    void initObjectiveForLevel(int lvl) {
        objectiveKind = 0;
        objectiveTarget = 0;
        objectiveProgress = 0;
        objectiveTargetFruit = -1;
        if (lvl == 1) {
            objectiveKind = 1;
            objectiveTarget = 10;
            objectiveTargetFruit = rand() % NUM_NORMAL;
        }
        else if (lvl == 2) {
            objectiveKind = 2;
            objectiveTarget = 3;
        }
        else if (lvl == 3) {
            objectiveKind = 3;
            objectiveTarget = 200;
            objectiveProgress = score; // starts at current score
        }
    }

    
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

    // en Board:
    void markCellForRemoval(int r, int c, bool forceBreakIce = false, bool animate = true) {
        if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) return;
        marked[r][c] = true;
        if (forceBreakIce) forcedBreak[r][c] = true;
        removalPending = true;

        if (animate && matrix[r][c]) {
            matrix[r][c]->startPop(500.f, MARK_SCALE);
           
        }
    }

    bool detectCombinationsWithoutRemoving() {
        // horizontals 
        for (int r = 0; r < SIZE; ++r) {
            int count = 1;
            for (int c = 1; c < SIZE; ++c) {
                if (matrix[r][c] && matrix[r][c - 1]
                    && matrix[r][c]->getType() != ICE_INDEX
                    && matrix[r][c - 1]->getType() != ICE_INDEX
                    && matrix[r][c]->getType() == matrix[r][c - 1]->getType()) {
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
                if (matrix[r][c] && matrix[r - 1][c]
                    && matrix[r][c]->getType() != ICE_INDEX
                    && matrix[r - 1][c]->getType() != ICE_INDEX
                    && matrix[r][c]->getType() == matrix[r - 1][c]->getType()) {
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
                            // NO actualizar objetivo aquí: inicialización del tablero no cuenta
                            delete matrix[r][c];
                            matrix[r][c] = nullptr;
                        }
            }

            return anyRemoval;
        }

        // GAME MODE: detectar combos
        if (!removalPending) {
            bool anyRemoval = false;
            bool markedLocal[SIZE][SIZE] = { false };
            bool comboLocal[SIZE][SIZE] = { false };

            // horizontals (robusto para hielo)
            for (int r = 0; r < SIZE; ++r) {
                int c = 0;
                while (c < SIZE) {
                    if (!matrix[r][c]) { ++c; continue; }
                    int t = matrix[r][c]->getType();
                    int k = c + 1;
                    while (k < SIZE && matrix[r][k] && matrix[r][k]->getType() == t) ++k;
                    int run = k - c;

                    if (run >= 3) {
                        bool runIsIce = (t == ICE_INDEX);
                        bool runContainsSwapped = false;
                        if (runIsIce) {
                            for (int s = c; s < k; ++s) {
                                if ((r == lastSwapR1 && s == lastSwapC1) || (r == lastSwapR2 && s == lastSwapC2)) {
                                    runContainsSwapped = true;
                                    break;
                                }
                            }
                        }

                        // marcar las celdas del run según reglas (si es hielo y no contiene swapped -> ignorar)
                        if (!runIsIce || runContainsSwapped) {
                            for (int s = c; s < k; ++s) {
                                int cellType = matrix[r][s]->getType();
                                bool wasSwapped = (r == lastSwapR1 && s == lastSwapC1) || (r == lastSwapR2 && s == lastSwapC2);

                                if (cellType == ICE_INDEX) {
                                    // solo marcar hielo si vino del swap
                                    if (wasSwapped) {
                                        markedLocal[r][s] = true;
                                        comboLocal[r][s] = true;
                                        anyRemoval = true;
                                    }
                                }
                                else {
                                    // fruta normal / bomba -> marcar normalmente
                                    markedLocal[r][s] = true;
                                    comboLocal[r][s] = true;
                                    anyRemoval = true;
                                }
                            }
                        }

                        if (run >= 4) {
                            super = t;
                            superR = r;
                            superC = c + (run - 1) / 2;
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
                        bool runIsIce = (t == ICE_INDEX);
                        bool runContainsSwapped = false;
                        if (runIsIce) {
                            for (int s = r; s < k; ++s) {
                                if ((s == lastSwapR1 && c == lastSwapC1) || (s == lastSwapR2 && c == lastSwapC2)) {
                                    runContainsSwapped = true;
                                    break;
                                }
                            }
                        }

                        if (!runIsIce || runContainsSwapped) {
                            for (int s = r; s < k; ++s) {
                                int cellType = matrix[s][c]->getType();
                                bool wasSwapped = (s == lastSwapR1 && c == lastSwapC1) || (s == lastSwapR2 && c == lastSwapC2);

                                if (cellType == ICE_INDEX) {
                                    if (wasSwapped) {
                                        markedLocal[s][c] = true;
                                        comboLocal[s][c] = true;
                                        anyRemoval = true;
                                    }
                                }
                                else {
                                    markedLocal[s][c] = true;
                                    comboLocal[s][c] = true;
                                    anyRemoval = true;
                                }
                            }
                        }
                        if (run >= 4) {
                            super = t;
                            superR = r + run - 1;
                            superC = c;
                        }
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
                for (int r = 0; r < SIZE; ++r) {
                    for (int c = 0; c < SIZE; ++c) {
                        marked[r][c] = markedLocal[r][c];
                        comboMarked[r][c] = comboLocal[r][c];
                        if (marked[r][c] && matrix[r][c]) {

                            matrix[r][c]->startPop(500.f, MARK_SCALE);

                        }
                    }
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
                        else {
                            matrix[r][c]->onMatch(this, r, c);

                            processed[r][c] = true;
                            progress = true;
                        }
                    }
                }


                // delete marked cells but respect ice rules (only delete ice if comboMarked or forcedBreak)
               // DELETE / REPLACE marked cells but respect ice rules
    // MARCADO -> iniciar animación (no borrar inmediatamente)
                // DELETE / REPLACE marked cells but respect ice rules
                for (int r = 0; r < SIZE; ++r) {
                    for (int c = 0; c < SIZE; ++c) {
                        if (!marked[r][c] || !matrix[r][c]) continue;

                        bool isIce = (matrix[r][c]->getType() == ICE_INDEX);
                        if (isIce && !comboMarked[r][c] && !forcedBreak[r][c]) {
                            continue;
                        }

                        // Si esta celda es la posición objetivo de la SUPER y hay un super pendiente:
                        if (super >= 0 && super <= 4 && r == superR && c == superC) {
                            delete matrix[r][c];
                            matrix[r][c] = new SuperFruit(textures[7 + super], cellCenter(r, c), super);

                            // opcional: ajustar visual (por ejemplo, destacar)
                           // matrix[r][c]->resetVisual()
                            super = -1;
                            superR = superC = -1;
                            continue;
                        }

                        // Caso normal: actualizar objetivo y eliminar la fruta
                        incrementObjectiveOnDelete(matrix[r][c]->getType());

                        // eliminar y contar puntos
                        delete matrix[r][c];
                        matrix[r][c] = nullptr;
                        anyRemoval = true;
                        score += 10;

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

                // Priorizar creación de "super" si hay uno pendiete (super == 0..4)

                if (roll < refillBombChance[level]) {
                    finalMatrix[r][c] = createFruitByType(BOMB_INDEX, textures[BOMB_INDEX], dummy);
                }
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
    int getSuperCount() const { return super; }
    int getSize() const { return SIZE; }
    int getTypeAt(int r, int c) const;


    // Devuelve true si alguna fruta está en animación "pop"
    bool hasActivePops() const {
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (matrix[r][c] && matrix[r][c]->isPopActive()) return true;
            }
        }
        return false;
    }

    // Devuelve true si hay cualquier animación/limpieza en progreso que deba bloquear el fin del nivel
    bool isIdle() const {
        if (cleaningInProgress) return false;
        if (gravityAnimating) return false;
        if (swapAnimating) return false;
        if (hasActivePops()) return false;
        return true;
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

        for (int r = 0; r < SIZE; ++r)
            for (int c = 0; c < SIZE; ++c)
                if (matrix[r][c]) {
                    sf::Vector2f center = cellCenter(r, c);
                    matrix[r][c]->setPixelPosition(center.x, center.y);
                }
    }

    void updateAnimations() {
        float deltaMs = animClock.restart().asMilliseconds();  // Como está ahora
        bool anyMoving = false;

        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (matrix[r][c]) {
                    matrix[r][c]->updateMove();  // Actualiza movimientos
                    if (matrix[r][c]->isMoving()) anyMoving = true;

                    if (matrix[r][c]->isPopActive()) {
                        bool finished = matrix[r][c]->updatePop(deltaMs);  // Pasa deltaMs
                        if (finished) {
                            // Código existente para manejar el final de la animación
                            matrix[r][c]->onMatch(this, r, c);
                            markCellForRemoval(r, c, false);
                            startCleaning(true);
                        }
                    }
                }
            }
        }

        if (!anyMoving) {
            if (gravityAnimating) gravityAnimating = false;
            if (swapAnimating) {
                swapAnimating = false;
                if (pendingSwapCleaning) {
                    pendingSwapCleaning = false;
                    startCleaning(true);
                }
            }
        }
    }
    bool isGravityAnimating() const { return gravityAnimating; }

    bool swapCells(int r1, int c1, int r2, int c2) {
        if (r1 < 0 || r1 >= SIZE || c1 < 0 || c1 >= SIZE || r2 < 0 || r2 >= SIZE || c2 < 0 || c2 >= SIZE) return false;
        int dr = abs(r1 - r2), dc = abs(c1 - c2);
        if (!((dr == 1 && dc == 0) || (dr == 0 && dc == 1))) return false;

        if (matrix[r1][c1] && matrix[r2][c2] &&
            matrix[r1][c1]->getType() == ICE_INDEX && matrix[r2][c2]->getType() == ICE_INDEX) {
            return false;
        }

        if (!formsCombinationWhenSwapping(r1, c1, r2, c2)) return false;

        lastSwapR1 = r1; lastSwapC1 = c1; lastSwapR2 = r2; lastSwapC2 = c2;

        Fruit* tmp = matrix[r1][c1];
        matrix[r1][c1] = matrix[r2][c2];
        matrix[r2][c2] = tmp;

        sf::Vector2f center1 = cellCenter(r1, c1);
        sf::Vector2f center2 = cellCenter(r2, c2);

        float dur = swapDurationMs;

        if (matrix[r1][c1]) matrix[r1][c1]->startMoveTo(center1.x, center1.y, dur);
        if (matrix[r2][c2]) matrix[r2][c2]->startMoveTo(center2.x, center2.y, dur);

        swapAnimating = true;
        pendingSwapCleaning = true;

        --remainingMoves;
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

    void selectOrSwap(int row, int col) {
        if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) return;
        if (!matrix[row][col]) return;

        if (selectedRow == -1) {
            BombFruit* asBomb = dynamic_cast<BombFruit*>(matrix[row][col]);
            if (asBomb != nullptr) {

                matrix[row][col]->startPop(500.f, MARK_SCALE);

                removalPending = true;
                startCleaning(true);

                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        int nr = row + dr, nc = col + dc;
                        if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE && matrix[nr][nc]) {

                            matrix[nr][nc]->startPop(500.f, MARK_SCALE);
                        }
                    }
                }
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
        for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) if (matrix[r][c]) { delete matrix[r][c]; matrix[r][c] = nullptr; }

        for (int r = 0; r < SIZE; ++r) for (int c = 0; c < SIZE; ++c) {
            marked[r][c] = comboMarked[r][c] = forcedBreak[r][c] = false;
        }
        remainingMoves = 5;
        score = 0;
        super = 0;
        cleaningInProgress = false;
        removalPending = false;
        lastSwapR1 = lastSwapC1 = lastSwapR2 = lastSwapC2 = -1;
        gravityAnimating = false;
        super = -1;
        superR = superC = -1;

        initObjectiveForLevel(level);

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
    void setLevelPublic(int l) { setLevel(l); }

    // Objetivo queries (para UI)
    string getObjectiveDescription() const {
        if (objectiveKind == 1) {
            if (objectiveTargetFruit == 0)
                return "Eliminar " + to_string(objectiveTarget) + " Manzanas";

            else if (objectiveTargetFruit == 1) {
                return "Eliminar " + to_string(objectiveTarget) + " Naranjas ";
            }
            else if (objectiveTargetFruit == 2) {
                return "Eliminar " + to_string(objectiveTarget) + " Bananas ";
            }
            else if (objectiveTargetFruit == 3) {
                return "Eliminar " + to_string(objectiveTarget) + " Sandías ";
            }
            else if (objectiveTargetFruit == 4) {
                return "Eliminar " + to_string(objectiveTarget) + " Uvas ";
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
