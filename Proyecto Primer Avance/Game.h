#pragma once
#include "Board.h"
// Maneja el juego y sus menus
class Game {
private:
    sf::RenderWindow window{ sf::VideoMode(800, 600), "Match-3" };
    sf::Font fuente;
    sf::Text textoPuntaje;
    sf::Text textoMovimientos;
    Board tablero;

    sf::Texture fondoTexture;
    sf::Sprite fondoSprite;
    sf::Texture fondoTexture1;
    sf::Sprite fondoSprite1;

    // delay entre pasos
    sf::Clock cleaningClock;
    sf::Time cleaningDelay = (sf::milliseconds(900));

    // MENU UI
    sf::RectangleShape playButton;
    sf::Text playButtonText;
    sf::Text titleText;

    // GAME_OVER UI
    sf::RectangleShape retryButton;
    sf::Text retryButtonText;
    sf::RectangleShape leaveButton;
    sf::Text leaveButtonText;
    sf::Text gameOverText;

    // Estado del juego: MENU, PLAYING o GAME_OVER
    enum class GameState { MENU, PLAYING, GAME_OVER };
    GameState state = GameState::MENU;

public:

    Game() {

        // fijamos la vista lógica en 800x600 (igual que tu diseño original)
        sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
        window.setView(view);

        // cargar recursos (fuente y fondos)
        fuente.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Arimo-font.ttf");



        fondoTexture.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Fruit background.png");
        fondoSprite.setTexture(fondoTexture);
        fondoSprite.setPosition(124.f, 110.f);


        fondoTexture1.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Menu background.png");
        fondoSprite1.setTexture(fondoTexture1);
        fondoSprite1.setPosition(0.f, 0.f);
        fondoSprite1.setScale(1.1, 1.45);

        // configurar textos (puntaje y movimientos)
        textoPuntaje.setFont(fuente);
        textoPuntaje.setCharacterSize(20);
        textoPuntaje.setFillColor(sf::Color::Black);
        textoPuntaje.setPosition(20.f, 10.f);

        textoMovimientos.setFont(fuente);
        textoMovimientos.setCharacterSize(20);
        textoMovimientos.setFillColor(sf::Color::Black);
        textoMovimientos.setPosition(300.f, 10.f);

        // MENU UI: botón central "JUGAR"
        playButton.setSize(sf::Vector2f(200.f, 80.f));
        playButton.setPosition(300.f, 300.f);
        playButton.setFillColor(sf::Color::Green);

        playButtonText.setFont(fuente);
        playButtonText.setCharacterSize(32);
        playButtonText.setString("JUGAR");
        playButtonText.setFillColor(sf::Color::Black);
        playButtonText.setPosition(playButton.getPosition().x + 47, playButton.getPosition().y + 20);

        titleText.setFont(fuente);
        titleText.setCharacterSize(48);
        titleText.setString("MATCH-3");
        titleText.setFillColor(sf::Color::Black);
        titleText.setPosition(300.f, 100.f);

        // GAME_OVER UI: botones Reintentar y Menu
        retryButton.setSize(sf::Vector2f(180.f, 60.f));
        retryButton.setOrigin(retryButton.getSize() / 2.f);
        retryButton.setPosition(400.f, 340.f);
        retryButton.setFillColor(sf::Color(200, 200, 200));
        retryButton.setOutlineThickness(3.f);
        retryButton.setOutlineColor(sf::Color::Black);

        retryButtonText.setFont(fuente);
        retryButtonText.setCharacterSize(28);
        retryButtonText.setString("Reintentar");
        retryButtonText.setFillColor(sf::Color::Black);
        retryButtonText.setPosition(retryButton.getPosition().x - 65, retryButton.getPosition().y - 25 + 6.f);

        leaveButton.setSize(sf::Vector2f(180.f, 60.f));
        leaveButton.setOrigin(leaveButton.getSize() / 2.f);
        leaveButton.setPosition(400.f, 420.f);
        leaveButton.setFillColor(sf::Color(200, 200, 200));
        leaveButton.setOutlineThickness(3.f);
        leaveButton.setOutlineColor(sf::Color::Black);

        leaveButtonText.setFont(fuente);
        leaveButtonText.setCharacterSize(28);
        leaveButtonText.setString("Salir");
        leaveButtonText.setFillColor(sf::Color::Black);
        leaveButtonText.setPosition(leaveButton.getPosition().x - 30, leaveButton.getPosition().y - 25 + 6.f);

        gameOverText.setFont(fuente);
        gameOverText.setCharacterSize(44);
        gameOverText.setString("FIN DEL JUEGO");
        gameOverText.setFillColor(sf::Color::Black);
        gameOverText.setPosition(230.f, 80.f);

        window.setFramerateLimit(60);
    }

    // Recoge eventos SFML y actúa según el estado actual del juego
    void processEvents() {
        sf::Event evento;
        while (window.pollEvent(evento)) {
            if (evento.type == sf::Event::Closed) window.close();

            if (evento.type == sf::Event::Resized) {
                // si se redimensiona la ventana, reajustamos la vista lógica 800x600
                sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
                window.setView(view);
                if (fondoTexture1.getSize().x != 0 && fondoTexture1.getSize().y != 0) {
                    fondoSprite1.setScale(800.f / (float)fondoTexture1.getSize().x, 600.f / (float)fondoTexture1.getSize().y);
                }
                continue;
            }

            // EVENTOS SEGUN ESTADO
            if (state == GameState::MENU) {
                // en el menú principal: click en el botón JUGAR
                if (evento.type == sf::Event::MouseButtonPressed && evento.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f world = window.mapPixelToCoords(sf::Vector2i(evento.mouseButton.x, evento.mouseButton.y));
                    if (playButton.getGlobalBounds().contains(world)) {
                        state = GameState::PLAYING;
                    }
                }
            }
            else if (state == GameState::PLAYING) {
                // ignorar clics si está limpiando.
                if (tablero.isCleaning()) continue;

                if (evento.type == sf::Event::MouseButtonPressed && evento.mouseButton.button == sf::Mouse::Left) {
                    if (!tablero.quedanMovimientos()) {
                        continue;
                    }

                    int x = evento.mouseButton.x;
                    int y = evento.mouseButton.y;

                    // Convertir de pixels a coordenadas lógicas mediante la vista actual
                    auto rc = tablero.screenToCell(window, x, y);
                    int fila = rc.first;
                    int col = rc.second;

                    if (fila >= 0 && fila < 8 && col >= 0 && col < 8) {
                        tablero.selectOrSwap(fila, col);
                    }
                }
            }
            else if (state == GameState::GAME_OVER) {
                // en pantalla de game over: botones Reintentar y Menu
                if (evento.type == sf::Event::MouseButtonPressed && evento.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f world = window.mapPixelToCoords(sf::Vector2i(evento.mouseButton.x, evento.mouseButton.y));
                    if (retryButton.getGlobalBounds().contains(world)) {
                        tablero.resetBoard(3);
                        state = GameState::PLAYING;
                    }
                    else if (leaveButton.getGlobalBounds().contains(world)) {
                        window.close();
                    }
                }

            }
        }
    }

    // Lógica que se ejecuta cada frame (temporización de limpieza, comprobación fin de juego, textos)
    void update() {
        if (state == GameState::PLAYING) {
            // Si el tablero está en fase de limpieza, avanza pasos cada cleaningDelay
            if (tablero.isCleaning()) {
                if (cleaningClock.getElapsedTime() >= cleaningDelay) {
                    tablero.stepCleaning();
                    cleaningClock.restart();
                }
            }

            // detectar fin de juego: sin movimientos y no está limpiando
            if (!tablero.quedanMovimientos() && !tablero.isCleaning()) {
                state = GameState::GAME_OVER;
            }

            // actualizar textos en pantalla con valores actuales
            textoPuntaje.setString("Puntaje: " + to_string(tablero.getPuntaje()));
            textoMovimientos.setString("Movimientos: " + to_string(tablero.getMovimientosRestantes()));
            textoMovimientos.setPosition(640.f, 10.f);

        }
    }

    // Dibuja la UI y el tablero según el estado actual
    void render() {
        window.clear(sf::Color::White);

        if (state == GameState::MENU) {
            if (fondoTexture1.getSize().x != 0 && fondoTexture1.getSize().y != 0) window.draw(fondoSprite1);

            // título + botón
            window.draw(titleText);
            window.draw(playButton);
            window.draw(playButtonText);
        }
        else if (state == GameState::PLAYING) {
            if (fondoTexture1.getSize().x != 0 && fondoTexture1.getSize().y != 0) window.draw(fondoSprite1);
            if (fondoTexture.getSize().x != 0 && fondoTexture.getSize().y != 0) window.draw(fondoSprite);

            // dibujar tablero (gemas)
            tablero.draw(window);

            // dibujar textos si la fuente se cargó correctamente

            window.draw(textoPuntaje);
            window.draw(textoMovimientos);

        }
        else if (state == GameState::GAME_OVER) {
            if (fondoTexture1.getSize().x != 0 && fondoTexture1.getSize().y != 0) window.draw(fondoSprite1);

            // overlay simple semi-transparente
            sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
            overlay.setFillColor(sf::Color(255, 255, 255, 200));
            overlay.setPosition(0.f, 0.f);
            window.draw(overlay);

            // dibujar menú de game over
            window.draw(gameOverText);
            window.draw(retryButton);
            window.draw(retryButtonText);
            window.draw(leaveButton);
            window.draw(leaveButtonText);

            // mostrar puntaje final
            sf::Text finalScore;
            finalScore.setFont(fuente);
            finalScore.setCharacterSize(28);
            finalScore.setString("Puntaje final: " + to_string(tablero.getPuntaje()));
            finalScore.setFillColor(sf::Color::Black);

            finalScore.setPosition(300.f, 200.f);
            window.draw(finalScore);
        }


        window.display();
    }
    // run: bucle principal del juego (eventos -> update -> render)
    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();

        }
    }
};