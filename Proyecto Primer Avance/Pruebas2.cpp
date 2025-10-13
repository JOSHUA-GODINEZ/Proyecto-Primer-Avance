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

    bool popActive = false;
    float popElapsedMs = 0.f;
    float popDurationMs = 220.f;   // duración total de la animación
    float popStartScale = 1.f;
    float popTargetScale = 1.35f;  // MARK_SCALE o lo que quieras
    bool pendingRemoval = false;

  //  bool popping = false;       // indica si está en animación pop
    sf::Clock popClock;
   // float popDuration = 200.f;  // duración total de la animación en ms
    float startScale = 1.f;     // escala inicial
    float maxScale = 1.5f;      // escala máxima antes de eliminar



    bool removalGrowing = false;
    sf::Clock removalClock;
    float removalDurationMs = 220.f; // duración de la animación de grow (ms)
    float removalStartScale = 1.f;
    float removalTargetScale = 1.6f; // escala máxima antes de borrar
    bool removalFade = false; // false si no quieres fade, true para fade out
    bool removalFinishedFlag = false; // indica que la animación acabó

   
    float popDuration = 200.f; // duración total en ms
    float popElapsed = 0.f;
    float popMaxScale = 1.5f;
    float originalScale = 1.f;

    sf::Sprite sprite;
    int type;

    // Animación lineal
    sf::Vector2f moveStart;
    sf::Vector2f moveEnd;
    float moveDurationMs = 0.f;
    sf::Clock moveClock;
    bool moving = false;






    float currentScale = 1.f;     // escala actual
    float targetScale = 2.f;      // escala máxima deseada
    float scaleStep = 0.05f;      // cuánto aumenta por frame
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


    bool isPendingRemoval() const { return pendingRemoval; }


    void animatePop() {
        float startScale = 1.0f;    // escala inicial
        float endScale = 1.8f;      // escala máxima
        float step = 0.01f;         // velocidad del crecimiento (más pequeño = más lento)
      
        for (float s = startScale; s <= endScale; s += step) {
            setScale(s, s);
            // pausa breve para suavizar la animación
          
        }
      
        
    }

    // Inicia la animación pop (no bloqueante)
   // duration_ms: duración total en milisegundos
   // targetScale: escala final alcanzada al completar la animación
    void startPop(float duration_ms = 220.f, float targetScale = 1.5f) {
        popActive = true;
        popElapsedMs = 0.f;
        popDurationMs = std::max(1.0f, duration_ms);
        popStartScale = sprite.getScale().x; // asumimos x==y
        popTargetScale = targetScale;
        pendingRemoval = true;
    }

    // Llamar cada frame con delta en ms.
    // Devuelve true si la animación terminó en este frame (alcanzó target).
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

    // Opcional: detener prematuramente y restaurar escala original
    void stopPop(bool restore = true) {
        popActive = false;
        if (restore) sprite.setScale(popStartScale, popStartScale);
    }





    // Iniciar la animación de "grow then delete"
    void startGrowRemoval(float duration_ms = 220.f, float targetScale = 1.6f, bool fade = false) {
        if (removalGrowing) return; // ya en proceso
        removalGrowing = true;
        removalFinishedFlag = false;
        removalDurationMs = duration_ms;
        removalTargetScale = targetScale;
        removalFade = fade;
        removalClock.restart();

        // tomar la escala actual como inicio (puede ser SELECT_SCALE o MARK_SCALE)
        sf::Vector2f sc = sprite.getScale();
        removalStartScale = sc.x; // asumimos sx==sy
    }

    // Llamar cada frame para actualizar la animación
    void updateRemovalGrow() {
        if (!removalGrowing) return;
        float elapsed = removalClock.getElapsedTime().asMilliseconds();
        float t = elapsed / removalDurationMs;
        if (t >= 1.f) {
            // acabado: fijar escala objetivo y marcar terminado
            sprite.setScale(removalTargetScale, removalTargetScale);
            removalGrowing = false;
            removalFinishedFlag = true;
            // si quieres fade al final, lo podrías aplicar aquí
            if (removalFade) {
                sf::Color c = sprite.getColor();
                c.a = 0;
                sprite.setColor(c);
            }
            return;
        }
        // easing (ease-out quad) para que el crecimiento sea más natural
        float e = 1.f - (1.f - t) * (1.f - t);
        float scale = removalStartScale + (removalTargetScale - removalStartScale) * e;
        sprite.setScale(scale, scale);

        // opcional: fade progresivo (si removalFade==true)
        if (removalFade) {
            float alpha = 255.f * (1.f - e);
            sf::Color c = sprite.getColor();
            c.a = static_cast<sf::Uint8>(std::max(0.f, std::min(255.f, alpha)));
            sprite.setColor(c);
        }
    }

    bool isRemovalGrowing() const { return removalGrowing; }
    bool removalFinished() const { return removalFinishedFlag; }

    bool isMoving() const { return moving; }

    sf::Vector2f getHalfSize() const {
        sf::FloatRect b = sprite.getLocalBounds();
        return { b.width / 2.f, b.height / 2.f };
    }

    int getType() const { return type; }
    void setType(int t) { type = t; }

    void setScale(float sx, float sy) { sprite.setScale(sx, sy); }

    void resetVisual() { sprite.setScale(1.f, 1.f); }


    void startPopRemoval(float duration_ms, float max_scale) {
        popping = true;
        popDuration = duration_ms;
        popElapsed = 0.f;
        popMaxScale = max_scale;
        originalScale =1.5f ; // asumimos escala X=Y
        setScale(originalScale, originalScale); // reinicia escala
    }
    bool updatePopRemoval1() {
        if (!popping) return false;
        float t = popClock.getElapsedTime().asMilliseconds() / popDuration;
        if (t >= 1.f) {
            popping = false;  // terminó la animación
            return true;      // indica que se debe eliminar
        }

        // easing (ease-out quad)
        float e = 1.f - (1.f - t) * (1.f - t);
        float currentScale = startScale + (maxScale - startScale) * e;
        sprite.setScale(currentScale, currentScale);
        return false; // aún no eliminar
    }
    // Llamar cada frame
    // dt_ms es tiempo transcurrido desde el último frame (puedes calcularlo con sf::Clock)
    bool updatePopRemoval(float dt_ms) {
        if (!popping) return false;

        popElapsed += dt_ms;
        float t = popElapsed / popDuration;
        if (t > 1.f) t = 1.f;

        // interpolación lineal entre originalScale y popMaxScale
        float newScale = originalScale + (popMaxScale - originalScale) * t;
        setScale(newScale, newScale);

        if (t >= 1.f) {
            popping = false;
            return true; // indica que la animación terminó y puede eliminarse
        }
        return false;
    }



    void startPop() {
        popping = true;
        currentScale = 1.f;
        setScale(currentScale, currentScale);
    }

    void updatePopAnimation() {
        if (!popping) return;

        currentScale += scaleStep;
        if (currentScale > targetScale) currentScale = targetScale;

        setScale(currentScale, currentScale);
    }

    bool isPopping() const { return popping; }
    void stopPop() { popping = false; } // para detener si quieres
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
class SuperFruit : public Fruit {
public:
    SuperFruit(sf::Texture& tex, const sf::Vector2f& pos, int logicalType)
        : Fruit(tex, pos, logicalType) {
    }

    // Declaración: onMatch override (definiremos la implementación después del Board)
    void onMatch(Board* board, int row, int col) override;
};



// ---------------------- Board ----------------------
class Board {
private:
    static constexpr int SIZE = 8;
    static constexpr int NUM_NORMAL = 5;   // textures 0..4 are normal fruit types
    static constexpr int BOMB_INDEX = 5;   // index texture for bomb
    static constexpr int ICE_INDEX = 6;    // index for ice
    static constexpr int NUM_TEXTURES = 12; // total loaded textures

    Fruit* matrix[SIZE][SIZE];
    vector<sf::Texture> textures;

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
    int super = -1;            // ya lo tienes, -1 = ninguno
    int superR = -1;      // fila donde queremos generar la super-fruta
    int superC = -1;      // columna donde queremos generar la super-fruta

    // gravity animation
    bool gravityAnimating = false;
    float gravityDurationMs = 1000.f; // requested ~900 ms


    // swap animation
    bool swapAnimating = false;
    bool pendingSwapCleaning = false;
    float swapDurationMs = 120.f; // duración base del swap en ms (ajústalo)

    sf::Clock popClock;

    const float MARK_SCALE = 1.5f;
    const float SELECT_SCALE = 1.25f;

    int selectedRow = -1, selectedCol = -1;

    sf::Sprite manzanaD;

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
            objectiveTarget = 10;
            // seleccionar aleatoriamente tipo normal [0..NUM_NORMAL-1]
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
        if (!textures[5].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Bomb12.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[6].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\cubo hielo3.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[7].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\manzanaD1.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[8].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Naranja de oro12.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[9].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Banano dorado12.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[10].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\sandia dorada12.png")) cerr << "Failed to load textures[5]\n";
        if (!textures[11].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Unvas doradas12.png")) cerr << "Failed to load textures[5]\n";
      
  
       

        level = startLevel;
        initObjectiveForLevel(level);
        remainingMoves = 3;
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
                            // NO actualizar objetivo aquí: inicialización del tablero no cuenta
                            // (antes: incrementObjectiveOnDelete(matrix[r][c]->getType()); )

                            // liberar fruta (si usas punteros crudos)
                            delete matrix[r][c];
                            matrix[r][c] = nullptr;

                            // si migraste a unique_ptr usa:
                            // matrix[r][c].reset();
                        }
            }

            return anyRemoval;
        }

        // GAME MODE: detectar combos
        if (!removalPending) {
            bool anyRemoval = false;
            bool markedLocal[SIZE][SIZE] = { false };
            bool comboLocal[SIZE][SIZE] = { false };

            // -----------------------------------
            // Horizontales
            // -----------------------------------
            for (int r = 0; r < SIZE; ++r) {
                int c = 0;
                while (c < SIZE) {
                    if (!matrix[r][c]) { ++c; continue; }
                    int t = matrix[r][c]->getType();
                    int k = c + 1;
                    while (k < SIZE && matrix[r][k] && matrix[r][k]->getType() == t) ++k;
                    int run = k - c;

                    if (run >= 3) {
                        // Marcar frutas para eliminar
                        for (int s = c; s < k; ++s) {
                            int cellType = matrix[r][s]->getType();
                            bool wasSwapped = (r == lastSwapR1 && s == lastSwapC1) || (r == lastSwapR2 && s == lastSwapC2);

                            if (cellType == ICE_INDEX) {
                                if (wasSwapped) {
                                    markedLocal[r][s] = true;
                                    comboLocal[r][s] = true;
                                    anyRemoval = true;
                                }
                            }
                            else {
                                markedLocal[r][s] = true;
                                comboLocal[r][s] = true;
                                anyRemoval = true;
                            }
                        }

                        // -----------------------------
                        // Super-fruta: horizontal → fila r, columna central del run
                        // -----------------------------
                        if (run >= 4) {
                            super = t; // guarda el tipo de fruta
                            superR = r;
                            superC = c + (run - 1) / 2; // columna central (si par, toma el de la izquierda)
                        }
                    }
                    c = k;
                }
            }

            // -----------------------------------
            // Verticales
            // -----------------------------------
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

                        // -----------------------------
                        // Super-fruta: vertical → columna c, fila más baja del run
                        // -----------------------------
                        if (run >= 4) {
                            super = t; // guarda el tipo de fruta
                            superR = r + run - 1; // fila más baja
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
                       
                          //  matrix[r][c]->startMoveTo(200, 100, 100.f);
                       

                            matrix[r][c]->startPop(500.f, MARK_SCALE); // ejemplo: 220ms hasta MARK_SCALE

                            

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
                        // call onMatch (bomb will mark neighbors etc)
                    
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
                        // no se elimina este hielo (no fue combo ni forced)
                        continue;
                    }

                    // Si esta celda es la posición objetivo de la SUPER y hay un super pendiente:
                    if (super >= 0 && super <= 4 && r == superR && c == superC) {
                        // reemplazamos la fruta por la super-fruta INMEDIATAMENTE
                        // (no contamos puntos por esta celda; es la nueva super)
                        delete matrix[r][c];
                        matrix[r][c] = new SuperFruit(textures[7 + super], cellCenter(r, c), super);

                        // opcional: ajustar visual (por ejemplo, destacar)
                        matrix[r][c]->resetVisual();

                        // consumimos el super y reseteamos la posición objetivo
                        super = -1;
                        superR = superC = -1;

                        // no marcamos anyRemoval por esta celda (porque no fue realmente removida)
                        // (si quieres que la creación de la super cuente como un "evento" podrías setear anyRemoval=true)
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
    // Public helpers for SuperFruit actions
 // en Board (public)
int getSize() const { return SIZE; }
int getTypeAt(int r, int c) const;




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
            // Si hay un super pendiente, forzamos su creación en la celda objetivo
          

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
    //void updateAnimations() {
    //    bool anyMoving = false;
    //    // update all fruits (swaps and gravity)
    //    for (int r = 0; r < SIZE; ++r) {
    //        for (int c = 0; c < SIZE; ++c) {
    //            if (matrix[r][c]) {
    //                matrix[r][c]->updateMove();
    //                if (matrix[r][c]->isMoving()) anyMoving = true;
    //            }
    //        }
    //    }

    //    // If nothing is moving, check flags
    //    if (!anyMoving) {
    //        // if gravity was animating, stop it
    //        if (gravityAnimating) gravityAnimating = false;

    //        // if swap animating just finished, trigger cleaning
    //        if (swapAnimating) {
    //            swapAnimating = false;
    //            if (pendingSwapCleaning) {
    //                pendingSwapCleaning = false;
    //                // we start cleaning phase now that swap animation ended
    //                startCleaning(true);
    //            }
    //        }
    //    }
    //}




    // en Board (miembro)
    sf::Clock animClock; // inicializar en constructor si quieres

    //void updateAnimations() {
    //    // calcula delta en ms desde la última llamada
    //    float deltaMs = animClock.restart().asMilliseconds();

    //    bool anyMoving = false;
    //    // Actualiza movimientos lineales (startMoveTo / updateMove) si los tienes
    //    for (int r = 0; r < SIZE; ++r) {
    //        for (int c = 0; c < SIZE; ++c) {
    //            if (matrix[r][c]) {
    //                // si tienes movimiento por gravedad / swap
    //                matrix[r][c]->updateMove();
    //                if (matrix[r][c]->isMoving()) anyMoving = true;

    //                // actualizar pop animation (si está activa)
    //                if (matrix[r][c]->isPopActive()) {
    //                    bool finished = matrix[r][c]->updatePop(deltaMs);
    //                    // aquí NO borramos: tú decides qué hacer cuando termine
    //                    // pero si quieres detectar finishes, podrías marcar una flag o poner en una lista
    //                    (void)finished;
    //                }
    //            }
    //        }
    //    }   if (!anyMoving) {
    //        // if gravity was animating, stop it
    //        if (gravityAnimating) gravityAnimating = false;

    //        // if swap animating just finished, trigger cleaning
    //        if (swapAnimating) {
    //            swapAnimating = false;
    //            if (pendingSwapCleaning) {
    //                pendingSwapCleaning = false;
    //                // we start cleaning phase now that swap animation ended
    //                startCleaning(true);
    //            }
    //        }
    //    }

    //    // ... El resto de tu lógica de animación (gravedad, swap) sigue como antes
    //    if (!anyMoving) gravityAnimating = false; // o según tu flujo
    //}



    void updateAnimations() {
        // delta desde la última llamada en ms
        float deltaMs = animClock.restart().asMilliseconds();

        bool anyMoving = false;
        // Actualiza movimiento (gravedad / swap) si aplicas
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (matrix[r][c]) {
                    // updateMove existente
                    matrix[r][c]->updateMove();
                    if (matrix[r][c]->isMoving()) anyMoving = true;

                    // actualizar animación pop si activa
                    if (matrix[r][c]->isPopActive()) {
                        bool finished = matrix[r][c]->updatePop(deltaMs);
                        // NO borramos aquí (tú controlas la eliminación). 
                        // Si quieres reaccionar a la finalización, puedes comprobar 'finished' y guardar coords.
                        //(void)finished; // evitar warning si no lo usás ahora
                    }
               
                    else if (matrix[r][c] && matrix[r][c]->isPendingRemoval()) {
                        // Animación completada, ahora aplicamos efectos de la bomba
                        if (BombFruit* b = dynamic_cast<BombFruit*>(matrix[r][c])) {
                            markCellForRemoval(r, c, true);   // marcamos su celda y vecinos
                            b->onMatch(this, r, c);           // detona bomba
                        }
                         // eliminar fruta
                      
                    }
                


                }
            }
        }

        if (!anyMoving) {
            // if gravity was animating, stop it
            if (gravityAnimating) gravityAnimating = false;

            // if swap animating just finished, trigger cleaning
            if (swapAnimating) {
                swapAnimating = false;
                if (pendingSwapCleaning) {
                    pendingSwapCleaning = false;
                    // we start cleaning phase now that swap animation ended
                    startCleaning(true);
                }
            }
        }
        // comportamiento previo con gravityAnimating / swapAnimating no cambia
        // por ejemplo:
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

        // swap pointers in matrix immediately (logical board changes)
        Fruit* tmp = matrix[r1][c1];
        matrix[r1][c1] = matrix[r2][c2];
        matrix[r2][c2] = tmp;

        // compute centers
        sf::Vector2f center1 = cellCenter(r1, c1);
        sf::Vector2f center2 = cellCenter(r2, c2);

        // compute duration based on speedMultiplier if present
        float dur = swapDurationMs;
#ifdef BOARD_HAS_SPEED_MULTIPLIER
        dur = swapDurationMs / speedMultiplier; // si usas speedMultiplier
#endif

        // start move animations for both fruits (if exist)
        if (matrix[r1][c1]) matrix[r1][c1]->startMoveTo(center1.x, center1.y, dur);
        if (matrix[r2][c2]) matrix[r2][c2]->startMoveTo(center2.x, center2.y, dur);

        // mark that a swap animation is happening; delay cleaning until animation ends
        swapAnimating = true;
        pendingSwapCleaning = true;

        --remainingMoves;
        // no startCleaning() aquí: esperaremos a que termine la animación y lo haremos en updateAnimations()
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
                // 1. iniciar animación pop
                matrix[row][col]->startPop(200.f, MARK_SCALE);
                removalPending = true;
                startCleaning(true);
                // 2. detonar la bomba (efecto)
          

                // 3. marcar para limpieza
               
          



                // después de asBomb->onMatch(this, row, col);, itera vecinos y si están marcados inicia la animación
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        int nr = row + dr, nc = col + dc;
                        if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE && matrix[nr][nc]) {
                            // start pop solo si la celda fue marcada por la detonacion
                            // (markCellForRemoval ya fue llamado por onMatch)
                            // para evitar duplicados, startPop debe ser idempotente (interna lo ignora si ya popActive)
                            matrix[nr][nc]->startPop(300.f, MARK_SCALE);
                        }
                    }
                }

              

                // 4. iniciar limpieza (pero deja que updatePop corra al menos un frame)
          

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
        remainingMoves = 5; // fixed default
        score = 0;
        super = 0;
        cleaningInProgress = false;
        removalPending = false;
        lastSwapR1 = lastSwapC1 = lastSwapR2 = lastSwapC2 = -1;
        gravityAnimating = false;
        super = -1;
        superR = superC = -1;

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
    void resetMoves(int n) { remainingMoves = n; } // kept for interface but not used for difficulty
    int getLevel() const { return level; }
    void setLevelPublic(int l) { setLevel(l); } // wrapper pública usada por Game

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


int Board::getTypeAt(int r, int c) const {
    if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) return -1;
    if (!matrix[r][c]) return -1;
    return matrix[r][c]->getType();
}

void SuperFruit::onMatch(Board* board, int row, int col) {
    int type = getType(); // logical type 0..4
    int N = board->getSize();

    // Helper lambda: marcar si está dentro del tablero
    auto tryMark = [&](int r, int c) {
        if (r >= 0 && r < N && c >= 0 && c < N) {
            board->markCellForRemoval(r, c, true); // force removal
        }
        };

    switch (type) {
    case 0: // textura 7 -> entire ROW
        for (int c = 0; c < N; ++c) tryMark(row, c);
        break;

    case 1: // textura 8 -> entire COLUMN
        for (int r = 0; r < N; ++r) tryMark(r, col);
        break;

    case 2: // textura 9 -> CROSS (row + col)
        for (int c = 0; c < N; ++c) tryMark(row, c);
        for (int r = 0; r < N; ++r) tryMark(r, col);
        break;

    case 3: // textura 10 -> 3x3 area centered
        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc)
                tryMark(row + dr, col + dc);
        break;

    case 4: // textura 11 -> all fruits of same logical type
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                int t = board->getTypeAt(r, c);
                if (t == type) tryMark(r, c);
            }
        }
        break;

    default:
        break;
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
    sf::Time cleaningDelay = sf::milliseconds(800);

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

    // Menu button in Game Over screen
    sf::RectangleShape menuButton;
    sf::Text menuButtonText;

    // Track difficulty selection: true == hard selected; false == easy selected
    bool hardSelected = false;

    enum class GameState { MENU, PLAYING, GAME_OVER };
    GameState state = GameState::MENU;

public:
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
        scoreText.setFont(font); scoreText.setCharacterSize(20); scoreText.setFillColor(sf::Color::Black); scoreText.setPosition(20.f, 30.f);
        movesText.setFont(font); movesText.setCharacterSize(20); movesText.setFillColor(sf::Color::Black); movesText.setPosition(10.f, 10.f);
        levelText.setFont(font); levelText.setCharacterSize(20); levelText.setFillColor(sf::Color::Black); levelText.setPosition(520.f, 10.f);

        // Menu UI
        playButton.setSize(sf::Vector2f(250.f, 85.f));
        playButton.setPosition(280.f, 150.f);
        playButton.setFillColor(sf::Color(100, 255, 100, 255));

        playButtonText.setFont(font);
        playButtonText.setCharacterSize(50);
        playButtonText.setString("JUGAR");
        playButtonText.setFillColor(sf::Color::Black);
        playButtonText.setPosition(playButton.getPosition().x + 40.f, playButton.getPosition().y +10);

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
        outB.setFillColor(sf::Color(255, 100, 100, 255));
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
        retryButton.setPosition(230.f,460.f);
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
        leaveButton.setPosition(570.f, 460.f);
        leaveButton.setFillColor(sf::Color(255, 100, 100, 255));
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
        gameOverText.setPosition(230.f, 30.f);

        // NEXT LEVEL button
        nextLevelButton.setSize(sf::Vector2f(250.f, 80.f));
        nextLevelButton.setOrigin(nextLevelButton.getSize() / 2.f);
        nextLevelButton.setPosition(400.f, 270.f);
        nextLevelButton.setFillColor(sf::Color(180, 220, 180));
        nextLevelButton.setOutlineThickness(3.f);
        nextLevelButton.setOutlineColor(sf::Color::Black);

        nextLevelText.setFont(font);
        nextLevelText.setCharacterSize(26);
        nextLevelText.setString("SIGUIENTE NIVEL");
        nextLevelText.setFillColor(sf::Color::Black);
        nextLevelText.setPosition(nextLevelButton.getPosition().x - 107.f, nextLevelButton.getPosition().y - 18.f);

        // MENU button on Game Over
        menuButton.setSize(sf::Vector2f(220.f, 60.f));
        menuButton.setOrigin(menuButton.getSize() / 2.f);
        menuButton.setPosition(400.f, 370.f);
        menuButton.setFillColor(sf::Color(200, 200, 255));
        menuButton.setOutlineThickness(3.f);
        menuButton.setOutlineColor(sf::Color::Black);

        menuButtonText.setFont(font);
        menuButtonText.setCharacterSize(22);
        menuButtonText.setString("MENU PRINCIPAL");
        menuButtonText.setFillColor(sf::Color::Black);
        menuButtonText.setPosition(menuButton.getPosition().x - 95.f, menuButton.getPosition().y - 12.f);

        // Default difficulty: FACIL selected
        hardSelected = false;
        easyB.setFillColor(sf::Color(100, 255, 100, 255));
        hardB.setFillColor(sf::Color(200, 200, 200, 255));
        board.resetMoves(50); // default moves (no longer changed by difficulty)

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
                        // EASY selected (solo visual y flag, no cambio de movimientos)
                        hardSelected = false;
                        easyB.setFillColor(sf::Color(100, 255, 100, 255));
                        hardB.setFillColor(sf::Color(200, 200, 200, 255));
                    }
                    if (hardB.getGlobalBounds().contains(world)) {
                        // HARD selected (solo visual y flag)
                        hardSelected = true;
                        hardB.setFillColor(sf::Color(100, 255, 100, 255));
                        easyB.setFillColor(sf::Color(200, 200, 200, 255));
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
                    // Retry
                    if (retryButton.getGlobalBounds().contains(world)) {
                        board.resetBoard();
                        state = GameState::PLAYING;
                        return;
                    }
                    // Leave
                    if (leaveButton.getGlobalBounds().contains(world)) {
                        window.close();
                        return;
                    }
                    // Menu (vuelve a pantalla principal)
                    if (menuButton.getGlobalBounds().contains(world)) {
                        // volver a menu principal, reset level 1, marcar FACIL por defecto
                        board.setLevelPublic(1);
                        board.resetBoard();
                        state = GameState::MENU;
                        // set difficulty default = easy
                        hardSelected = false;
                        easyB.setFillColor(sf::Color(100, 255, 100, 255));
                        hardB.setFillColor(sf::Color(200, 200, 200, 255));
                        board.resetMoves(5);
                        return;
                    }
                    // Next Level: solo visible si level < 3 y además,
                    // - si hardSelected == false -> funciona siempre
                    // - si hardSelected == true -> solo si objetivo completado
                    if (board.getLevel() < 3 && nextLevelButton.getGlobalBounds().contains(world)) {
                        // if hard selected and objective failed, button is not supposed to be shown/clickable.
                        bool objectiveComplete = board.isObjectiveComplete();
                        bool canAdvance = (!hardSelected) || objectiveComplete;
                        if (canAdvance) {
                            int newLevel = board.getLevel() + 1;
                            board.setLevelPublic(newLevel);
                            board.resetBoard();
                            state = GameState::PLAYING;
                            return;
                        }
                        // otherwise no action (button hidden when failed in hard)
                    }
                }
            }
        }
    }

    void update() {
        if (state == GameState::PLAYING) {
            // update gravity animations each frame if active

            // Always update animations (gravity or swap)
            board.updateAnimations();
     
                // if (board.isGravityAnimating()) {
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
                    movesText.setString("MOVIMIENTOS: " + to_string(board.getRemainingMoves()));


                    levelText.setString("Nivel: " + to_string(board.getLevel()));
                    levelText.setPosition(360.f, 10.f);
                    levelText.setCharacterSize(25);
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
            objTitle.setPosition(650.f, 10.f);

            objProgress.setFont(font);
            objProgress.setCharacterSize(19);
            objProgress.setFillColor(sf::Color::Black);

            string desc = board.getObjectiveDescription();
            string prog = board.getObjectiveProgressText();

            // Si el jugador escogió FACIL, mostrarlos como "secundario"
            
                    objProgress.setString(" " + desc + "\n" + prog);
           
          
            objProgress.setPosition(605.f, 35.f);

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

            // menú (siempre visible)
            window.draw(menuButton);
            window.draw(menuButtonText);

            bool objectiveComplete = board.isObjectiveComplete();
            bool objectiveExists = !board.getObjectiveDescription().empty();

            // Mostrar mensaje de objetivo cumplido/fallido
            sf::Text resultText;
            resultText.setFont(font);
            resultText.setCharacterSize(25);
            resultText.setFillColor(sf::Color::Black);
            if (objectiveExists) {
                if (objectiveComplete) {
                    resultText.setString("OBJETIVO CUMPLIDO");
                }
                else {
                    resultText.setString("OBJETIVO FALLIDO");
                }
            }
            else {
                resultText.setString(""); // sin objetivo
            }
            resultText.setPosition(110.f, 140.f);
            window.draw(resultText);

            // Si el nivel actual es < 3, mostramos botón next level solo si:
            // - Si dificultad EASY: siempre (objetivos son secundarios)
            // - Si dificultad HARD: solo si objetivo completado
            if (board.getLevel() < 3) {
                if (!hardSelected) {
                    // easy: mostrar y permitir avanzar
                    window.draw(nextLevelButton);
                    window.draw(nextLevelText);
                }
                else {
                    // hard: mostrar solo si objetivo completado; si falló, NO mostrar
                    if (objectiveComplete) {
                        window.draw(nextLevelButton);
                        window.draw(nextLevelText);
                    }
                    // else no dibujar el botón
                }
            }

            // mostrar progreso del objetivo en pantalla final
            sf::Text objTitle;
            objTitle.setFont(font);
            objTitle.setCharacterSize(23);
            objTitle.setFillColor(sf::Color::Black);
            objTitle.setString("" + board.getObjectiveDescription());
            objTitle.setPosition(420.f, 140.f);
            window.draw(objTitle);

            sf::Text objProg;
            objProg.setFont(font);
            objProg.setCharacterSize(25);
            objProg.setFillColor(sf::Color::Black);
            objProg.setString("Obtuviste: " + board.getObjectiveProgressText());
            objProg.setPosition(313.f, 183.f);
            window.draw(objProg);

            sf::Text finalScore;
            finalScore.setFont(font);
            finalScore.setCharacterSize(25);
            finalScore.setString("Puntaje Final: " + to_string(board.getScore()));
            finalScore.setFillColor(sf::Color::Black);
            finalScore.setPosition(305.f, 90.f);
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

