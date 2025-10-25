#include "Board.h"
#include "Lista.h"
// Game (UI + loop)
#ifndef GAME_H
#define GAME_H

      // fmod

// Clase Jugador que se almacenará en la lista
class Jugador {
public:
    std::string nombre;
    int puntaje;

    Jugador() : nombre(""), puntaje(0) {}
    Jugador(const std::string& n, int p = 0) : nombre(n), puntaje(p) {}

    // operador == para eliminar por valor si se desea
    bool operator==(const Jugador& other) const {
        return nombre == other.nombre && puntaje == other.puntaje;
    }

    // utilidad para depuración
    std::string toString() const {
        return nombre + " (" + std::to_string(puntaje) + ")";
    }
};

// Game (UI + loop)
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
    sf::Texture userTexture;
    sf::Sprite userSprite;

    // timing cleaning
    sf::Clock cleaningClock;
    sf::Time cleaningDelay = sf::milliseconds(800);

    // Users UI
    sf::Text userLabel;
    sf::RectangleShape userButton;
    std::string nombre;         // texto que escribe el usuario actualmente
    sf::Text text;              // texto que se dibuja dentro del botón
    sf::Clock cursorClock;      // cursor parpadeante
    bool isTyping = false;

    // Players stored in Lista<Jugador>
    Lista<Jugador> players;     // lista enlazada genérica
    size_t playersCount = 0;    // mantener conteo (Lista no ofrece tamaño)
    const size_t maxPlayers = 10;
    int selectedPlayerIndex = -1;

    // Menu UI
    sf::RectangleShape playButton;
    sf::Text playButtonText;
    sf::Text titleText;
    sf::Text dificulty, easy, hard, outText, playerT;
    sf::RectangleShape easyB, hardB, outB, playersBox;

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


    // difficulty
    bool hardSelected = false;

    enum class GameState { USERS, MENU, PLAYING, GAME_OVER };
    GameState state = GameState::USERS;

    // Helper: borrar último carácter UTF-8
    static void eraseLastUtf8Char(std::string &s) {
        if (s.empty()) return;
        size_t i = s.size() - 1;
        while (i > 0 && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80) --i;
        s.erase(i);
    }

public:
    Game() : board() {
        sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
        window.setView(view);

        // Cargar fuente (ajusta ruta si es necesario)
        if (!font.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Arimo-font.ttf")) {
            std::cerr << "Failed to load font (ajusta la ruta)\n";
        }

        // Cargar texturas (rutas de ejemplo)
        userTexture.loadFromFile("C:\\Joshua\\practica\\Sprites\\granja6.png");
        backgroundTexture.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Fruit background.png");
        menuTexture.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Menu background.png");

        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setPosition(124.f, 110.f);

        menuSprite.setTexture(menuTexture);
        menuSprite.setPosition(0.f, 0.f);
        menuSprite.setScale(1.1f, 1.45f);

        userSprite.setTexture(userTexture);
        userSprite.setPosition(0.f, 0.f);
        userSprite.setScale(1.1f, 1.45f);

        // score/moves texts
        scoreText.setFont(font); scoreText.setCharacterSize(20); scoreText.setFillColor(sf::Color::Black); scoreText.setPosition(20.f, 30.f);
        movesText.setFont(font); movesText.setCharacterSize(20); movesText.setFillColor(sf::Color::Black); movesText.setPosition(10.f, 10.f);
        levelText.setFont(font); levelText.setCharacterSize(20); levelText.setFillColor(sf::Color::Black); levelText.setPosition(520.f, 10.f);

        // Users UI
        userLabel.setFont(font);
        userLabel.setCharacterSize(40);
        userLabel.setString("Ingrese Jugador:");
        userLabel.setFillColor(sf::Color::Black);
        userLabel.setPosition(250.f, 10.f);

        userButton.setSize(sf::Vector2f(220.f, 40.f));
        userButton.setPosition(290.f, 70.f);
        userButton.setFillColor(sf::Color(200, 200, 200, 255));
        userButton.setOutlineThickness(3.f);
        userButton.setOutlineColor(sf::Color::Black);

        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::Black);
        text.setPosition(userButton.getPosition().x + 8.f, userButton.getPosition().y + 6.f);


        // Menu UI
        playButton.setSize(sf::Vector2f(250.f, 85.f));
        playButton.setPosition(280.f, 150.f);
        playButton.setFillColor(sf::Color(100, 255, 100, 255));

        playButtonText.setFont(font);
        playButtonText.setCharacterSize(50);
        playButtonText.setString("JUGAR");
        playButtonText.setFillColor(sf::Color::Black);
        playButtonText.setPosition(playButton.getPosition().x + 40.f, playButton.getPosition().y + 10);

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
        hard.setString("DIFICIL");
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

        playersBox.setSize(sf::Vector2f(220.f, 60.f));
        playersBox.setPosition(80.f, 500.f);
        playersBox.setFillColor(sf::Color(200, 200, 200, 255));
        playersBox.setOutlineThickness(3.f);
        playersBox.setOutlineColor(sf::Color::Black);

        playerT.setFont(font);
        playerT.setCharacterSize(42);
        playerT.setString("Jugadores");
        playerT.setFillColor(sf::Color::Black);
        playerT.setPosition(playersBox.getPosition().x + 10.f, playersBox.getPosition().y);

        // GAME OVER
        retryButton.setSize(sf::Vector2f(180.f, 60.f));
        retryButton.setOrigin(retryButton.getSize() / 2.f);
        retryButton.setPosition(230.f, 460.f);
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
        board.resetMoves(3); // default moves

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

            // Texto ingresado (TextEntered) mientras se escribe
            if (event.type == sf::Event::TextEntered && isTyping) {
                if (event.text.unicode >= 32) { // espacio o printable
                    if (nombre.size() < 80) { // límite de bytes aproximado
                        sf::Uint32 codepoint = event.text.unicode;
                        if (codepoint < 0x80) {
                            nombre.push_back(static_cast<char>(codepoint));
                        } else if (codepoint < 0x800) {
                            nombre.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
                            nombre.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
                        } else if (codepoint < 0x10000) {
                            nombre.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
                            nombre.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
                            nombre.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
                        } else {
                            nombre.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
                            nombre.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
                            nombre.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
                            nombre.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
                        }
                    }
                }
            }

            // Teclas especiales mientras se escribe
            if (event.type == sf::Event::KeyPressed && isTyping) {
                if (event.key.code == sf::Keyboard::BackSpace) {
                    eraseLastUtf8Char(nombre);
                } else if (event.key.code == sf::Keyboard::Return) {
                    // finalizar edición y guardar en la lista
                    isTyping = false;
                    if (!nombre.empty()) {
                        // evitar duplicados: opcional (aquí permitimos duplicados)
                        if (playersCount < maxPlayers) {
                            players.agregarFinal(Jugador(nombre, 0));
                            playersCount++;
                            selectedPlayerIndex = static_cast<int>(playersCount) - 1;
                        } else {
                            // si ya hay 10, sobrescribimos el último (opcional)
                            // eliminamos el primero y agregamos al final para mantener rotación
                            // Lista no tiene eliminar por índice, así que eliminamos por valor del primer nodo
                            Nodo<Jugador>* cabeza = players.getCabeza();
                            if (cabeza) {
                                Jugador primero = cabeza->dato;
                                players.eliminar(primero); // eliminar primer nodo
                                players.agregarFinal(Jugador(nombre, 0));
                                // playersCount se mantiene igual
                                selectedPlayerIndex = static_cast<int>(playersCount) - 1;
                            }
                        }
                        // pasar a MENU
                       // state = GameState::MENU;
                    }
                } else if (event.key.code == sf::Keyboard::Escape) {
                    isTyping = false;
                }
            }

            // Mouse handling
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f world = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                if (state == GameState::USERS) {
                    if (userButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
                        isTyping = true;
                        nombre.clear();
                        cursorClock.restart();
                    } else {
                        if (isTyping) isTyping = false;
                    }

                    float playersX = 480.f;              // x inicio lista
                    float playersY = 200.f;               // y inicio
                    float playerW = 200.f, playerH = 36.f;
                    float buttonW = 60.f, buttonH = 36.f;
                    float gapY = 10.f;

                    // recorrer la lista enlazada y calcular rects
                    Nodo<Jugador>* nodo = players.getCabeza();
                    for (size_t i = 0; i < playersCount && nodo != nullptr; ++i, nodo = nodo->siguiente) {
                        float y = playersY + i * (playerH + gapY);
                        sf::FloatRect nameRect(playersX, y, playerW, playerH);
                        sf::FloatRect btnRect(playersX + playerW + 8.f, y, buttonW, buttonH);

                        if (nameRect.contains(world)) {
                            nombre = nodo->dato.nombre;
                            selectedPlayerIndex = static_cast<int>(i);
                            break;
                        }
                        if (btnRect.contains(world)) {
                            // acción del botón: selecciona el jugador y puede iniciar menú/partida
                            nombre = nodo->dato.nombre;
                            selectedPlayerIndex = static_cast<int>(i);
                            // por defecto nos quedamos en MENU; si quieres iniciar jugar:
                            state = GameState::MENU;
                            break;
                        }
                    }
                   
                }
                else if (state == GameState::MENU) {
                    if (playButton.getGlobalBounds().contains(world)) {
                        // Si se presiona JUGAR, iniciar jugando con el nombre seleccionado (si hay)
                        if (!nombre.empty()) {
                            board.resetBoard();
                            state = GameState::PLAYING;
                        }
                    }

                    if (playersBox.getGlobalBounds().contains(world)) { state = GameState::USERS; }


                    if (easyB.getGlobalBounds().contains(world)) {
                        hardSelected = false;
                        easyB.setFillColor(sf::Color(100, 255, 100, 255));
                        hardB.setFillColor(sf::Color(200, 200, 200, 255));
                    }
                    if (hardB.getGlobalBounds().contains(world)) {
                        hardSelected = true;
                        hardB.setFillColor(sf::Color(100, 255, 100, 255));
                        easyB.setFillColor(sf::Color(200, 200, 200, 255));
                    }
                    if (outB.getGlobalBounds().contains(world)) window.close();

                    // detectar clicks sobre la lista de jugadores a la derecha
                   
                }
                else if (state == GameState::PLAYING) {
                    if (board.isCleaning()) {
                        // ignore clicks mientras limpia
                    } else {
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
                        return;
                    }
                    if (leaveButton.getGlobalBounds().contains(world)) {
                        window.close();
                        return;
                    }
                    if (menuButton.getGlobalBounds().contains(world)) {
                        board.setLevelPublic(1);
                        board.resetBoard();
                        state = GameState::MENU;
                        hardSelected = false;
                        easyB.setFillColor(sf::Color(100, 255, 100, 255));
                        hardB.setFillColor(sf::Color(200, 200, 200, 255));
                        board.resetMoves(5);
                        return;
                    }
                    if (board.getLevel() < 3 && nextLevelButton.getGlobalBounds().contains(world)) {
                        bool objectiveComplete = board.isObjectiveComplete();
                        bool canAdvance = (!hardSelected) || objectiveComplete;
                        if (canAdvance) {
                            int newLevel = board.getLevel() + 1;
                            board.setLevelPublic(newLevel);
                            board.resetBoard();
                            state = GameState::PLAYING;
                            return;
                        }
                    }
                }
            } // fin MouseButtonPressed
        } // fin while pollEvent
    }

    void update() {
        if (state == GameState::PLAYING) {
            board.updateAnimations();
            if (board.isGravityAnimating()) {
                board.updateAnimations();
            }
            else if (board.isCleaning()) {
                if (cleaningClock.getElapsedTime() >= cleaningDelay) {
                    board.stepCleaning();
                    cleaningClock.restart();
                }
            }
            if (!board.hasMoves() && board.isIdle()) {
                state = GameState::GAME_OVER;
            }
            scoreText.setString("PUNTUACIÓN: " + std::to_string(board.getScore()));
            movesText.setString("MOVIMIENTOS: " + std::to_string(board.getRemainingMoves()));

            levelText.setString("Nivel: " + std::to_string(board.getLevel()));
            levelText.setPosition(360.f, 10.f);
            levelText.setCharacterSize(25);
        }
    }

    void render() {
        window.clear(sf::Color::White);
        if (state == GameState::USERS) {
            if (userTexture.getSize().x != 0 && userTexture.getSize().y != 0) window.draw(userSprite);
            window.draw(userLabel);
            window.draw(userButton);

            // preparar cadena a mostrar (cursor parpadeante)
            bool drawCursor = isTyping && (std::fmod(cursorClock.getElapsedTime().asSeconds(), 1.0f) < 0.5f);
            std::string display = nombre;
            if (drawCursor) display += "_";
            text.setString(display);
            window.draw(text);

           

            // DIBUJAR LISTA DE JUGADORES A LA DERECHA
            float playersX = 480.f;
            float playersY = 200.f;
            float playerW = 200.f, playerH = 36.f;
            float buttonW = 60.f, buttonH = 36.f;
            float gapY = 10.f;
            sf::Text playerNameText;
            playerNameText.setFont(font);
            playerNameText.setCharacterSize(20);
            playerNameText.setFillColor(sf::Color::Black);

            Nodo<Jugador>* nodo = players.getCabeza();
            for (size_t i = 0; i < playersCount && nodo != nullptr; ++i, nodo = nodo->siguiente) {
                float y = playersY + i * (playerH + gapY);

                // rectángulo del nombre
                sf::RectangleShape nameRect(sf::Vector2f(playerW, playerH));
                nameRect.setPosition(playersX, y);
                nameRect.setFillColor(sf::Color(220, 220, 220));
                nameRect.setOutlineThickness(2.f);
                if (static_cast<int>(i) == selectedPlayerIndex) nameRect.setOutlineColor(sf::Color::Green);
                else nameRect.setOutlineColor(sf::Color::Black);
                window.draw(nameRect);

                // texto del nombre dentro del rectángulo
                playerNameText.setString(nodo->dato.nombre);
                playerNameText.setPosition(playersX + 6.f, y + 6.f);
                window.draw(playerNameText);

                // botón a la derecha
                sf::RectangleShape btn(sf::Vector2f(buttonW, buttonH));
                btn.setPosition(playersX + playerW + 8.f, y);
                btn.setFillColor(sf::Color(100, 200, 255));
                btn.setOutlineThickness(2.f);
                btn.setOutlineColor(sf::Color::Black);
                window.draw(btn);

                // texto del botón ("MENU")
                sf::Text btnText;
                btnText.setFont(font);
                btnText.setCharacterSize(18);
                btnText.setFillColor(sf::Color::Black);
                btnText.setString("MENU");
                float bx = playersX + playerW + 8.f + (buttonW - btnText.getLocalBounds().width) / 2.f - 4.f;
                float by = y + (buttonH - btnText.getCharacterSize()) / 2.f - 2.f;
                btnText.setPosition(bx, by);
                window.draw(btnText);

            }


        }
        else if (state == GameState::MENU) {
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
            window.draw(playersBox);
            window.draw(playerT);

            if (!nombre.empty()) {
                sf::Text current;
                current.setFont(font);
                current.setCharacterSize(20);
                current.setFillColor(sf::Color::Black);
                current.setString("Jugador: " + nombre);
                current.setPosition(10.f, 10.f);
                window.draw(current);
            }
            // Mostrar el nombre actualmente seleccionado (arriba a la derecha)
          
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

            std::string desc = board.getObjectiveDescription();
            std::string prog = board.getObjectiveProgressText();

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

            window.draw(menuButton);
            window.draw(menuButtonText);

            bool objectiveComplete = board.isObjectiveComplete();
            bool objectiveExists = !board.getObjectiveDescription().empty();

            sf::Text resultText;
            resultText.setFont(font);
            resultText.setCharacterSize(25);
            resultText.setFillColor(sf::Color::Black);
            if (objectiveExists) {
                if (objectiveComplete) resultText.setString("OBJETIVO CUMPLIDO");
                else resultText.setString("OBJETIVO FALLIDO");
            } else resultText.setString("");
            resultText.setPosition(110.f, 140.f);
            window.draw(resultText);

            if (board.getLevel() < 3) {
                if (!hardSelected) {
                    window.draw(nextLevelButton);
                    window.draw(nextLevelText);
                } else {
                    if (objectiveComplete) {
                        window.draw(nextLevelButton);
                        window.draw(nextLevelText);
                    }
                }
            }

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
            finalScore.setString("Puntaje Final: " + std::to_string(board.getScore()));
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

#endif // GAME_H

