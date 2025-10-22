#include "Board.h"
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
    Game() : board() { // start level 1
        sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
        window.setView(view);

        // load font and textures (adjust paths)
        if (!font.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Arimo-font.ttf")) {
            cerr << "Failed to load font (adjust path)\n";
        }

        if (!backgroundTexture.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Fruit background.png")) {
         
        }
        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setPosition(124.f, 110.f);

        if (!menuTexture.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Menu background.png")) {
           
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
        board.resetMoves(3); // default moves (no longer changed by difficulty)

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
            }
        }
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
                resultText.setString("");
            }
            resultText.setPosition(110.f, 140.f);
            window.draw(resultText);
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

