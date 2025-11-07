# Proyecto Primer Avance

 Descripción general

Match-3 Fruit Game es un juego tipo puzzle en el que el jugador intercambia frutas adyacentes para formar combinaciones de tres o más del mismo tipo.
Cada combinación elimina las frutas implicadas, aplica efectos especiales (como bombas o superfrutas) y genera nuevas piezas mediante gravedad.
El juego incluye varios niveles con objetivos diferentes, sistema de puntuación y guardado de progreso en un archivo XML.

Características principales

Tablero 8×8 con frutas animadas.

Detección de combinaciones horizontales y verticales.

Gravedad, regeneración y cascadas automáticas.

Frutas especiales: Bomb, Ice y SuperFruit.

Objetivos por nivel y conteo de movimientos.

Sistema de jugadores con guardado automático (players.xml).

Interfaz con varios estados: Usuarios, Menú, Progreso, Juego y Game Over.


' Clase base Fruit
class Fruit {
  # bool popActive = false
  # float popElapsedMs = 0.f
  # float popDurationMs = 200.f
  # float popStartScale = 1.f
  # float popTargetScale = 1.35f
  # bool pendingRemoval = false

  # sf::Sprite sprite
  # int type

  # sf::Vector2f moveStart
  # sf::Vector2f moveEnd
  # float moveDurationMs = 0.f
  # sf::Clock moveClock
  # bool moving = false

  # float currentScale = 1.f
  # bool popping = false

  + Fruit()
  + Fruit(sf::Texture& texture, sf::Vector2f pixelCenter, int fruitType)
  + virtual ~Fruit()

  + virtual void draw(sf::RenderWindow& win)
  + virtual void onMatch(Board* board, int row, int col)

  + sf::Vector2f getPixelPosition() const
  + void setPixelPosition(float x, float y)

  + void startMoveTo(float tx, float ty, float duration_ms)
  + void updateMove()

  + void startPop(float duration_ms = 100.f, float targetScale = 1.5f)
  + bool updatePop(float deltaMs)
  + bool isPopActive() const
  + bool isMoving() const

  + int getType() const
  + void setType(int t)
  + void setScale(float sx, float sy)
  + void resetVisual()
}



' Subclases
class NormalFruit {
  + NormalFruit(sf::Texture& tex, sf::Vector2f pos, int t)
  + void onMatch(Board*, int, int) override
}

class BombFruit {
  + BombFruit(sf::Texture& tex, sf::Vector2f pos, int t)
  + void onMatch(Board* board, int row, int col) override
}

class IceFruit {
  + IceFruit(sf::Texture& tex, sf::Vector2f pos, int t)
  + void onMatch(Board*, int, int) override
}

class SuperFruit {
  + SuperFruit(sf::Texture& tex, const sf::Vector2f& pos, int logicalType)
  + void onMatch(Board* board, int row, int col) override
}

 Herencia
NormalFruit --|> Fruit
BombFruit --|> Fruit
IceFruit --|> Fruit
SuperFruit --|> Fruit

 Dependencia hacia Board (usa puntero en onMatch)
Fruit ..> Board : uses
BombFruit ..> Board : uses
SuperFruit ..> Board : uses



              +----------------+
              |     Board      |
              +----------------+
                     ^
                     |
             <<uses | (onMatch(Board*))
                     |
+-------------------------------------------------------------+
|                          Fruit                              |
|-------------------------------------------------------------|
| # popActive: bool                                           |
| # popElapsedMs: float                                       |
| # popDurationMs: float                                      |
| # popStartScale, popTargetScale                             |
| # pendingRemoval: bool                                      |
| # sprite: sf::Sprite                                        |
| # type: int                                                 |
| # moveStart, moveEnd: sf::Vector2f                          |
| # moveDurationMs: float                                     |
| # moveClock: sf::Clock                                      |
| # moving: bool                                              |
| # currentScale, popping                                      |
|-------------------------------------------------------------|
| + Fruit()                                                   |
| + Fruit(sf::Texture&, sf::Vector2f, int)                    |
| + ~Fruit()                                                  |
| + virtual draw(sf::RenderWindow&)                           |
| + virtual onMatch(Board*, int row, int col)                 |
| + startMoveTo(tx,ty,duration_ms)                            |
| + updateMove()                                               |
| + startPop(duration_ms=100,targetScale=1.5)                 |
| + updatePop(deltaMs): bool                                  |
| + isPopActive(), isMoving(), getType(), setType()           |
| + setScale(), resetVisual()                                 |
+-------------------------------------------------------------+
        /|\         /|\         /|\
         |           |           |
   +-----+--+   +----+----+   +--+-----+
   |Normal  |   | Bomb     |   | Ice   |
   | Fruit  |   | Fruit    |   | Fruit |
   +--------+   +----------+   +-------+
   (override)   (override)      (override)
                      \
                       \
                     +-----------+
                     | SuperFruit|
                     +-----------+
                     (override)




                           +----------------+
                           |     Fruit*     |  <-- (varias subclases)
                           +----------------+
                                   ^
                                   |
 +--------------------------------------------------------------------------+
 |                                 Board                                    |
 |--------------------------------------------------------------------------|
 | - SIZE: const int = 8                                                     |
 | - NUM_NORMAL, BOMB_INDEX, ICE_INDEX, NUM_TEXTURES                         |
 | - Fruit* matrix[8][8]                                                     |
 | - sf::Texture textures[12]                                                |
 | - originX, originY, cellW, cellH                                          |
 | - marked, comboMarked, forcedBreak (matrices SIZE x SIZE)                |
 | - removalPending, cleaningInProgress, countScoreDuringCleaning           |
 | - lastSwapR1/C1, lastSwapR2/C2                                           |
 | - remainingMoves, score, acumalateScore, acumulateSocereLevel[5]         |
 | - level, super, superR, superC, reiniciar                                |
 | - gravityAnimating, gravityDurationMs                                     |
 | - swapAnimating, pendingSwapCleaning, swapDurationMs                      |
 | - MARK_SCALE, SELECT_SCALE, selectedRow/Col                              |
 | - initBombChance[], initIceChance[], refillBombChance[]                  |
 | - objectiveKind/Target/Progress/TargetFruit, animClock                   |
 |--------------------------------------------------------------------------|
 | + Board(), ~Board()                                                       |
 | + initObjectiveForLevel(lvl)                                              |
 | + markCellForRemoval(r,c,forceBreakIce,animate)                           |
 | + detectCombinationsWithoutRemoving()                                     |
 | + clearCombosOnce(countScore)                                             |
 | + applyGravityAndReplace()                                                 |
 | + updateAnimations(), isGravityAnimating()                                |
 | + swapCells(r1,c1,r2,c2), selectOrSwap(row,col)                           |
 | + startCleaning(countScore), stepCleaning(), isCleaning()                 |
 | + screenToCell(win,mouseX,mouseY), resetBoard()                           |
 | + getters/setters (score, level, remainingMoves, objective text/progress) |
 | + draw(win)                                                               |
 +--------------------------------------------------------------------------+


           +---------------------------------------------------------------------------------+
|                                    Game                                         |
|---------------------------------------------------------------------------------|
| - sf::RenderWindow window{ sf::VideoMode(800, 600), "Match-3" }                |
| - sf::Font font                                                                 |
| - sf::Text scoreText, movesText, levelText                                      |
| - Board board                                                                   |
|                                                                                 |
| - sf::Texture backgroundTexture                                                 |
| - sf::Sprite backgroundSprite                                                   |
| - sf::Texture menuTexture                                                       |
| - sf::Sprite menuSprite                                                         |
| - sf::Texture userTexture                                                       |
| - sf::Sprite userSprite                                                         |
| - int nivel, nivel1, nivel2, nivel3, nivel4, nivel5                             |
| - bool paso                                                                     |
|                                                                                 |
| - sf::Text level1, level2, level3, level4, level5, total                        |
| - sf::RectangleShape blev1, blev2, blev3, blev4, blev5                          |
| - sf::Text blev1T, blev2T, blev3T, blev4T, blev5T                               |
| - sf::Texture fondo12                                                           |
| - sf::Sprite fondo1                                                             |
| - float playersX = 180.f                                                        |
| - float playersY = 210.f                                                        |
| - float playerW = 280.f, playerH = 36.f                                         |
| - float buttonW = 50.f, buttonH = 35.f                                          |
| - float gapY = 10.f                                                             |
| - float btn1X = playersX + playerW + 8.f                                        |
| - float btn2X = playersX + playerW + 65.f                                       |
| - int level = 0                                                                 |
|                                                                                 |
| - sf::Clock cleaningClock                                                       |
| - sf::Time cleaningDelay = sf::milliseconds(800)                                |
|                                                                                 |
| - sf::Text userLabel, listaLabel, Tprogress                                     |
| - sf::RectangleShape userButton, progress                                       |
| - std::string nombre                                                            |
| - sf::Text text, nombreU, pointU                                                |
| - sf::Clock cursorClock                                                         |
| - bool isTyping = false                                                         |
|                                                                                 |
| - Lista<Jugador> players                                                        |
| - size_t playersCount = 0                                                       |
| - const size_t maxPlayers = 8                                                   |
| - int selectedPlayerIndex = -1                                                  |
| - int playerscore = 0                                                           |
|                                                                                 |
| - sf::RectangleShape playButton                                                 |
| - sf::Text playButtonText                                                       |
| - sf::Text titleText                                                            |
| - sf::Text dificulty, easy, hard, outText, playerT                              |
| - sf::RectangleShape easyB, hardB, outB, playersBox                             |
|                                                                                 |
| - sf::RectangleShape retryButton                                                |
| - sf::Text retryButtonText                                                      |
| - sf::RectangleShape leaveButton                                                |
| - sf::Text leaveButtonText                                                      |
| - sf::Text gameOverText                                                         |
|                                                                                 |
| - sf::RectangleShape nextLevelButton                                            |
| - sf::Text nextLevelText                                                        |
|                                                                                 |
| - sf::RectangleShape menuButton, menuButton1, menuButton2                       |
| - sf::Text menuButtonText, menuButtonText1, menuButtonText2                     |
|                                                                                 |
| - bool pass = false                                                             |
| - bool hardSelected = false                                                     |
|                                                                                 |
| - enum class GameState { USERS, MENU, PROGRESS, PLAYING, GAME_OVER }            |
| - GameState state = GameState::USERS                                            |
|                                                                                 |
| - const std::string playersFilePath = "players.xml"                             |                                                              |
|---------------------------------------------------------------------------------|
| + void ordenarJugadoresPorPuntaje()                                             |
| + void eraseLastUtf8Char(std::string& s)                                        |
| + void savePlayersToFile()                                                      |
| + void loadPlayersFromFile()                                                    |
|                                                                                 |
| + Game()  (constructor)                                                         |
| + ~Game() (destructor)                                                          |
|                                                                                 |
| + void processEvents()                                                          |
| + void update()                                                                 |
| + void render()                                                                 |
| + void run()                                                                    |
+---------------------------------------------------------------------------------+





+------------------------------------------------+
|                   Jugador                      |
|------------------------------------------------|
| + nombre : std::string                         |
| + puntaje : int                                |
| + nivel : int                                  |
| + level1 : int                                 |
| + level2 : int                                 |
| + level3 : int                                 |
| + level4 : int                                 |
| + level5 : int                                 |
| + Jugador()                                    |
| + Jugador(n: const std::string&,               |
|           p: int, l: int,                      |
|           l1: int, l2: int, l3: int,           |
|           l4: int, l5: int)                    |
| + operator==(other: const Jugador&) : bool     |
+------------------------------------------------+

             


+--------------------------------------+
|              Nodo<Q>                 |
|--------------------------------------|
| + dato : Q                           |
| + siguiente : Nodo<Q>*               |
|--------------------------------------|
| + Nodo(d: const Q&)                  |
+--------------------------------------+



+--------------------------------------------------+
|                   Lista<T>                       |
|--------------------------------------------------|
| - cabeza : Nodo<T>*                              |
|--------------------------------------------------|
| + Lista()                                        |
| + ~Lista()                                       |
| + estaVacia() : bool                             |
| + agregarFinal(dato: const T&)                   |
| + eliminar(dato: const T&) : bool                |
| + vaciar()                                       |
| + getCabeza() : Nodo<T>*                         |
+--------------------------------------------------+





