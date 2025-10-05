#pragma once
#include "Board.h"
// Manages the game and its menus
class Game {
private:
    sf::RenderWindow window{ sf::VideoMode(800, 600), "Match-3" };
    sf::Font font;
    sf::Text scoreText;
    sf::Text movesText;
    sf::Text level;
    Board board;

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Texture backgroundTexture1;
    sf::Sprite backgroundSprite1;

    // delay between cleaning steps
    sf::Clock cleaningClock;
    sf::Time cleaningDelay = (sf::milliseconds(900));

    // MENU UI
    sf::RectangleShape playButton;
    sf::Text playButtonText;
    sf::Text titleText;
    sf::Text dificulty, easy,hard,out;
    sf::RectangleShape easyB, hardB,outB;
    // GAME_OVER UI
    sf::RectangleShape retryButton;
    sf::Text retryButtonText;
    sf::RectangleShape leaveButton;
    sf::Text leaveButtonText;
    sf::Text gameOverText;

    // Game state: MENU, PLAYING or GAME_OVER
    enum class GameState { MENU, PLAYING, GAME_OVER };
    GameState state = GameState::MENU;

public:

    Game() {

        // set logical view to 800x600 (same as original design)
        sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
        window.setView(view);

        // load resources (font and backgrounds)
        font.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Arimo-font.ttf");

        backgroundTexture.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Fruit background.png");
        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setPosition(124.f, 110.f);

        backgroundTexture1.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Menu background.png");
        backgroundSprite1.setTexture(backgroundTexture1);
        backgroundSprite1.setPosition(0.f, 0.f);
        backgroundSprite1.setScale(1.1, 1.45);

        // configure texts (score and moves)
        scoreText.setFont(font);
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(sf::Color::Black);
        scoreText.setPosition(20.f, 10.f);

        movesText.setFont(font);
        movesText.setCharacterSize(20);
        movesText.setFillColor(sf::Color::Black);
        movesText.setPosition(300.f, 10.f);

        // MENU UI: central "PLAY" button
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
        easy.setPosition(easyB.getPosition().x + 40, easyB.getPosition().y );


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

        out.setFont(font);
        out.setCharacterSize(40);
        out.setString("SALIR ");
        out.setFillColor(sf::Color::Black);
        out.setPosition(outB.getPosition().x + 40, outB.getPosition().y);



        // GAME_OVER UI: Retry and Exit buttons
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

    // Collects SFML events and acts according to the current game state
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::Resized) {
                // if window resized, readjust logical view 800x600
                sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
                window.setView(view);
                if (backgroundTexture1.getSize().x != 0 && backgroundTexture1.getSize().y != 0) {
                    backgroundSprite1.setScale(800.f / (float)backgroundTexture1.getSize().x, 600.f / (float)backgroundTexture1.getSize().y);
                }
                continue;
            }

            // EVENTS BY STATE
            if (state == GameState::MENU) {
                // in main menu: click on PLAY button
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
                // ignore clicks if cleaning.
                if (board.isCleaning()) continue;

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    if (!board.hasMoves()) {
                        continue;
                    }

                    int x = event.mouseButton.x;
                    int y = event.mouseButton.y;

                    // Convert from pixels to logical coordinates using the current view
                    auto rc = board.screenToCell(window, x, y);
                    int row = rc.first;
                    int col = rc.second;

                    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
                        board.selectOrSwap(row, col);
                    }
                }
            }
            else if (state == GameState::GAME_OVER) {
                // on game over screen: Retry and Exit buttons
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

    // Logic executed every frame (cleaning timing, end-of-game check, texts)
    void update() {
        if (state == GameState::PLAYING) {
            // If board is in cleaning phase, advance steps every cleaningDelay
            if (board.isCleaning()) {
                if (cleaningClock.getElapsedTime() >= cleaningDelay) {
                    board.stepCleaning();
                    cleaningClock.restart();
                }
            }

            // detect end of game: no moves and not cleaning
            if (!board.hasMoves() && !board.isCleaning()) {
                state = GameState::GAME_OVER;
            }

            // update on-screen texts with current values
            scoreText.setString("PUNTUACIÓN: " + to_string(board.getScore()));
            movesText.setString("MOVIMIENTOS RESTANTES: " + to_string(board.getRemainingMoves()));
            movesText.setPosition(490.f, 10.f);
            level.setString("NIvel: " + to_string(board.getLevel()));
            level.setPosition(300.f, 10.f);
        }
    }

    // Draw UI and board according to the current state
    void render() {
        window.clear(sf::Color::White);

        if (state == GameState::MENU) {
            if (backgroundTexture1.getSize().x != 0 && backgroundTexture1.getSize().y != 0) window.draw(backgroundSprite1);

            // title + button
            window.draw(titleText);
            window.draw(playButton);
            window.draw(playButtonText);
            window.draw(dificulty);
            window.draw(easyB);
            window.draw(easy);
            window.draw(hardB);
            window.draw(hard);
            window.draw(outB);
            window.draw(out);
        }
        else if (state == GameState::PLAYING) {
            if (backgroundTexture1.getSize().x != 0 && backgroundTexture1.getSize().y != 0) window.draw(backgroundSprite1);
            if (backgroundTexture.getSize().x != 0 && backgroundTexture.getSize().y != 0) window.draw(backgroundSprite);

            // draw board (fruits)
            board.draw(window);

            // draw texts if font loaded correctly
            window.draw(scoreText);
            window.draw(movesText);
        }
        else if (state == GameState::GAME_OVER) {
            if (backgroundTexture1.getSize().x != 0 && backgroundTexture1.getSize().y != 0) window.draw(backgroundSprite1);

            // simple semi-transparent overlay
            sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
            overlay.setFillColor(sf::Color(255, 255, 255, 200));
            overlay.setPosition(0.f, 0.f);
            window.draw(overlay);

            // draw game over menu
            window.draw(gameOverText);
            window.draw(retryButton);
            window.draw(retryButtonText);
            window.draw(leaveButton);
            window.draw(leaveButtonText);

            // show final score
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

    // run: main game loop (events -> update -> render)
    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }
};


