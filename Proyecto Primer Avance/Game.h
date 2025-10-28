#include "Board.h"
#include "Lista.h"
// Game (UI + loop)
#ifndef GAME_H
#define GAME_H
#include <fstream>


// ---------------- Clase Jugador ----------------
class Jugador {
public:
    std::string nombre;
    int puntaje;
    int nivel;

    Jugador() : nombre(""), puntaje(0), nivel(0) {}
    Jugador(const std::string& n, int p, int l) : nombre(n), puntaje(p), nivel(l) {}

    bool operator==(const Jugador& other) const {
        return nombre == other.nombre;
    }

    std::string toString() const {
        return nombre + " (" + std::to_string(puntaje) + ")";
    }
};


// ---------------- Clase Game ----------------
class Game {
private:
    // Ventana y UI básica
    sf::RenderWindow window{ sf::VideoMode(800, 600), "Match-3" };
    sf::Font font;
    sf::Text scoreText, movesText, levelText;
    Board board;

    // Backgrounds
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Texture menuTexture;
    sf::Sprite menuSprite;
    sf::Texture userTexture;
    sf::Sprite userSprite;
    int nivel;

    //Progreso
    sf::Text level1,level2,level3,level4,level5;
    sf::RectangleShape blev1, blev2, blev3, blev4, blev5,reset;




    // Timing
    sf::Clock cleaningClock;
    sf::Time cleaningDelay = sf::milliseconds(800);

    // Users UI
    sf::Text userLabel, listaLabel,Tprogress;
    sf::RectangleShape userButton, progress;
    std::string nombre;                // texto que escribe el usuario actualmente
    sf::Text text, nombreU, pointU;    // textos auxiliares
    sf::Clock cursorClock;             // cursor parpadeante
    bool isTyping = false;

    // Players stored in Lista<Jugador>
    Lista<Jugador> players;
    size_t playersCount = 0;
    const size_t maxPlayers = 8;
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

    // Next level button
    sf::RectangleShape nextLevelButton;
    sf::Text nextLevelText;

    // Menu button on Game Over
    sf::RectangleShape menuButton;
    sf::Text menuButtonText;

    bool hardSelected = false;

    enum class GameState { USERS, MENU,PROGRESS, PLAYING, GAME_OVER };
    GameState state = GameState::USERS;

    // Archivo XML donde se guardan jugadores
    const std::string playersFilePath = "players.xml";


    // ---------------- Helpers UTF-8 ----------------
    static void eraseLastUtf8Char(std::string& s) {
        if (s.empty()) return;
        size_t i = s.size() - 1;
        while (i > 0 && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80) --i;
        s.erase(i);
    }

    // ---------------- Helpers XML (escape/unescape) ----------------
    static std::string escapeXml(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default: out += c;
            }
        }
        return out;
    }

    static std::string unescapeXml(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '&') {
                if (s.compare(i, 5, "&amp;") == 0) { out += '&'; i += 4; }
                else if (s.compare(i, 4, "&lt;") == 0) { out += '<'; i += 3; }
                else if (s.compare(i, 4, "&gt;") == 0) { out += '>'; i += 3; }
                else if (s.compare(i, 6, "&quot;") == 0) { out += '"'; i += 5; }
                else if (s.compare(i, 6, "&apos;") == 0) { out += '\''; i += 5; }
                else {
                    out += '&';
                }
            }
            else {
                out += s[i];
            }
        }
        return out;
    }

    // ---------------- Guardar / Cargar XML (sin librerías) ----------------
    void savePlayersToFile() {
        std::ofstream f(playersFilePath, std::ofstream::out | std::ofstream::trunc);
        if (!f.is_open()) {
            std::cerr << "No se pudo abrir " << playersFilePath << " para guardar.\n";
            return;
        }
        f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        f << "<players>\n";

        Nodo<Jugador>* nodo = players.getCabeza();
        while (nodo) {
            std::string nameEsc = escapeXml(nodo->dato.nombre);
            int score = nodo->dato.puntaje;
            int level= nodo->dato.nivel;
            f << "  <player>\n";
            f << "    <name>" << nameEsc << "</name>\n";
            f << "    <score>" << score << "</score>\n";
            f << "    <level>" << level << "</level>\n";
            f << "  </player>\n";
            nodo = nodo->siguiente;
        }

        f << "</players>\n";
        f.close();
    }
    int level = 0;
    void loadPlayersFromFile() {
        std::ifstream f(playersFilePath);
        if (!f.is_open()) {
            // no existe => comenzamos con lista vacía
            return;
        }

        // Leer todo a string
        std::string content;
        f.seekg(0, std::ios::end);
        content.reserve((size_t)f.tellg());
        f.seekg(0, std::ios::beg);
        content.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.close();

        players.vaciar();
        playersCount = 0;

        size_t pos = 0;
        while (playersCount < maxPlayers) {
            size_t pstart = content.find("<player", pos);
            if (pstart == std::string::npos) break;
            size_t pclose = content.find(">", pstart);
            if (pclose == std::string::npos) break;
            size_t pend = content.find("</player>", pclose);
            if (pend == std::string::npos) break;

            std::string block = content.substr(pclose + 1, pend - (pclose + 1));

            std::string name;
            size_t n1 = block.find("<name>");
            size_t n2 = block.find("</name>");
            if (n1 != std::string::npos && n2 != std::string::npos && n2 > n1) {
                name = block.substr(n1 + 6, n2 - (n1 + 6));
                name = unescapeXml(name);
            }

            int score = 0;
            size_t s1 = block.find("<score>");
            size_t s2 = block.find("</score>");
            if (s1 != std::string::npos && s2 != std::string::npos && s2 > s1) {
                std::string sc = block.substr(s1 + 7, s2 - (s1 + 7));
                try { score = std::stoi(sc); }
                catch (...) { score = 0; }
            }


   
            size_t l1 = block.find("<level>");
            size_t l2 = block.find("</level>");
            if (l1 != std::string::npos && l2 != std::string::npos && l2 > l1) {
                std::string sc = block.substr(l1 + 7, l2 - (l1 + 7));
                try { level = std::stoi(sc); }
                catch (...) { level = 0; }
            }

           

            if (!name.empty()) {
                players.agregarFinal(Jugador(name, score,level));
                playersCount++;
            }

            pos = pend + 9; // después de </player>
        }
        
    }
    int getlevel() { return level; }
public:
    Game() : board() {
        // Configuración view
        sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
        window.setView(view);

        // Fuente (ajusta ruta si hace falta)
        if (!font.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Arimo-font.ttf")) {
            std::cerr << "Failed to load font (ajusta la ruta)\n";
        }

        // Cargar texturas (ajusta rutas)
        userTexture.loadFromFile("C:\\Joshua\\practica\\Sprites\\paisaje.jpg");
        backgroundTexture.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Fruit background.png");
        menuTexture.loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Menu background.png");

        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setPosition(124.f, 110.f);

        menuSprite.setTexture(menuTexture);
        menuSprite.setPosition(0.f, 0.f);
        menuSprite.setScale(1.1f, 1.45f);

        userSprite.setTexture(userTexture);
        userSprite.setPosition(170.f, 0.f);
        userSprite.setScale(1.1f, 1.45f);

        // Textos score/moves
        scoreText.setFont(font); scoreText.setCharacterSize(20); scoreText.setFillColor(sf::Color::Black); scoreText.setPosition(20.f, 30.f);
        movesText.setFont(font); movesText.setCharacterSize(20); movesText.setFillColor(sf::Color::Black); movesText.setPosition(10.f, 10.f);
        levelText.setFont(font); levelText.setCharacterSize(20); levelText.setFillColor(sf::Color::Black); levelText.setPosition(520.f, 10.f);

        // Users UI
        userLabel.setFont(font);
        userLabel.setCharacterSize(40);
        userLabel.setString("Ingrese Jugador:");
        userLabel.setFillColor(sf::Color::Black);
        userLabel.setPosition(240.f, 10.f);

        userButton.setSize(sf::Vector2f(240.f, 40.f));
        userButton.setPosition(270.f, 70.f);
        userButton.setFillColor(sf::Color(200, 200, 200, 255));
        userButton.setOutlineThickness(3.f);
        userButton.setOutlineColor(sf::Color::Black);

        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::Black);
        text.setPosition(userButton.getPosition().x + 8.f, userButton.getPosition().y + 6.f);
        //Progreso

 
        level1.setFont(font);
        level1.setCharacterSize(20);
        level1.setFillColor(sf::Color::Black);

        level1.setPosition(200.f, 200.f);
       
       
        level2.setFont(font);
        level2.setCharacterSize(20);
        level2.setFillColor(sf::Color::Black);

        level2.setPosition(200.f, 250.f);

       
        level3.setFont(font);
        level3.setCharacterSize(20);
        level3.setFillColor(sf::Color::Black);
       
        level3.setPosition(200.f, 300.f);

  
        level4.setFont(font);
        level4.setCharacterSize(20);
        level4.setFillColor(sf::Color::Black);
 
        level4.setPosition(200.f, 350.f);


        level5.setFont(font);
        level5.setCharacterSize(20);
        level5.setFillColor(sf::Color::Black);

        level5.setPosition(200.f, 400.f);


     
        
        blev1.setSize(sf::Vector2f(30.f, 30.f));
        blev1.setFillColor(sf::Color(100, 200, 255));
        blev1.setOutlineThickness(2.f);
        blev1.setOutlineColor(sf::Color::Black);


        blev2.setSize(sf::Vector2f(30.f, 30.f));
        blev2.setFillColor(sf::Color(100, 200, 255));
        blev2.setOutlineThickness(2.f);
        blev2.setOutlineColor(sf::Color::Black);
       

        blev3.setSize(sf::Vector2f(30.f, 30.f));
        blev3.setFillColor(sf::Color(100, 200, 255));
        blev3.setOutlineThickness(2.f);
        blev3.setOutlineColor(sf::Color::Black);
     

        blev4.setSize(sf::Vector2f(30.f, 30.f));
        blev4.setFillColor(sf::Color(100, 200, 255));
        blev4.setOutlineThickness(2.f);
        blev4.setOutlineColor(sf::Color::Black);
       

        blev5.setSize(sf::Vector2f(30.f, 30.f));
        blev5.setFillColor(sf::Color(100, 200, 255));
        blev5.setOutlineThickness(2.f);
        blev5.setOutlineColor(sf::Color::Black);
    




        // Labels de la lista
        nombreU.setFont(font);
        nombreU.setCharacterSize(20);
        nombreU.setFillColor(sf::Color::Black);
        nombreU.setPosition(180.f, 155.f);
        nombreU.setString("Nombre");

        pointU.setFont(font);
        pointU.setCharacterSize(20);
        pointU.setFillColor(sf::Color::Black);
        pointU.setPosition(375.f, 155.f);
        pointU.setString("Puntos");

        listaLabel.setFont(font);
        listaLabel.setCharacterSize(30);
        listaLabel.setFillColor(sf::Color::Black);
        listaLabel.setPosition(250.f, 120.f);
        listaLabel.setString("Lista de jugadores");

        // Menu UI: play button, title, difficulty, boxes...
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

        progress.setSize(sf::Vector2f(220.f, 60.f));
        progress.setPosition(500.f, 500.f);
        progress.setFillColor(sf::Color(200, 200, 200, 255));
        progress.setOutlineThickness(3.f);
        progress.setOutlineColor(sf::Color::Black);

        Tprogress.setFont(font);
        Tprogress.setCharacterSize(42);
        Tprogress.setString("Progreso");
        Tprogress.setFillColor(sf::Color::Black);
        Tprogress.setPosition(progress.getPosition().x + 10.f, progress.getPosition().y);

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

        // GAME OVER UI
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

        // NEXT LEVEL
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

        // Default difficulty
        hardSelected = false;
        easyB.setFillColor(sf::Color(100, 255, 100, 255));
        hardB.setFillColor(sf::Color(200, 200, 200, 255));
        board.resetMoves(1);

        window.setFramerateLimit(60);

        // Cargar players desde archivo (si existe)
        loadPlayersFromFile();
        if (playersCount > 0) {
            Nodo<Jugador>* first = players.getCabeza();
            if (first) {
                nombre = first->dato.nombre;
                selectedPlayerIndex = 0;
                board.setLevelPublic(first->dato.nivel);
                board.resetBoard();
            }
        }

    }

    ~Game() {
        // Guardar players al destruir (por si el cierre no pasó por Event::Closed)
        savePlayersToFile();
    }

    // ---------------- Process events ----------------
    void processEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                savePlayersToFile();
                window.close();
            }

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
                if (event.text.unicode >= 32) { // caracteres imprimibles incluido espacio
                    if (nombre.size() < 14) { // límite de bytes aproximado
                        sf::Uint32 codepoint = event.text.unicode;
                        if (codepoint < 0x80) {
                            nombre.push_back(static_cast<char>(codepoint));
                        }
                        else if (codepoint < 0x800) {
                            nombre.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
                            nombre.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
                        }
                        else if (codepoint < 0x10000) {
                            nombre.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
                            nombre.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
                            nombre.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
                        }
                        else {
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
                }
                else if (event.key.code == sf::Keyboard::Return) {
                    // finalizar edición y guardar en la lista
                    isTyping = false;
                    if (!nombre.empty()) {
                        if (playersCount < maxPlayers) {
                            players.agregarFinal(Jugador(nombre,0,1)); // empieza con 0
                            playersCount++;
                            selectedPlayerIndex = static_cast<int>(playersCount) - 1;
                            savePlayersToFile();
                        }
                    }

                }
                else if (event.key.code == sf::Keyboard::Escape) {
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
                    }
                    else {
                        if (isTyping) isTyping = false;
                    }

                    // Parámetros de layout — usar las mismas posiciones que en render()
                    float playersX = 180.f;
                    float playersY = 180.f;
                    float playerW = 280.f, playerH = 36.f;
                    float buttonW = 50.f, buttonH = 35.f;
                    float gapY = 10.f;
                    float btn1X = playersX + playerW + 8.f;    // botón MENU (azul)
                    float btn2X = playersX + playerW + 65.f;   // botón Eliminar (rojo)

                    Nodo<Jugador>* nodo = players.getCabeza();
                    for (size_t i = 0; i < playersCount && nodo != nullptr; ++i, nodo = nodo->siguiente) {
                        float y = playersY + i * (playerH + gapY);
                        sf::FloatRect nameRect(playersX, y, playerW, playerH);
                        sf::FloatRect btnRect(btn1X, y, buttonW, buttonH);
                        sf::FloatRect btnRect2(btn2X, y, buttonW, buttonH);

                        //if (nameRect.contains(world)) {
                        //    // seleccionar nombre (solo visual)
                        //    nombre = nodo->dato.nombre;
             
                        //    selectedPlayerIndex = static_cast<int>(i);
                        //    break;
                        //}

                        if (btnRect.contains(world)) {
                            // botón azul: seleccionar y pasar a MENU
                            nombre = nodo->dato.nombre;
                            selectedPlayerIndex = static_cast<int>(i);
                            int playerLevel = nodo->dato.nivel;
                            board.setLevelPublic(playerLevel);
                            board.resetBoard();

                            state = GameState::MENU; // o GameState::PLAYING si prefieres iniciar ya
                            break;
                        }

                        if (btnRect2.contains(world)) {
                            // botón rojo: eliminar ESTE jugador
                            // Guardar el nombre antes de eliminar para comparaciones/ajustes
                            Jugador toRemove = nodo->dato;

                            bool removed = players.eliminar(toRemove);
                            if (removed) {
                                if (playersCount > 0) playersCount--;

                                if (selectedPlayerIndex == static_cast<int>(i)) {
                                    selectedPlayerIndex = -1;
                                    nombre.clear();
                                }
                                else if (selectedPlayerIndex > static_cast<int>(i)) {
                                    selectedPlayerIndex--;
                                }
                                
                                // persistir inmediatamente
                                savePlayersToFile();
                            }
                            // romper para no iterar con puntero invalidado
                            break;
                        }
                    }
                }
                else if (state == GameState::MENU) {
                   
                    if (playButton.getGlobalBounds().contains(world)) {
                        if (!nombre.empty()) {
                           
                            board.resetBoard();
                            state = GameState::PLAYING;
                        }
                    }
                    if (progress.getGlobalBounds().contains(world)) {
                        state = GameState::PROGRESS;

                           
                          
                        
                    }
                    if (playersBox.getGlobalBounds().contains(world)) {
                        state = GameState::USERS;
                    }

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
                    if (outB.getGlobalBounds().contains(world)) {
                        savePlayersToFile();
                        window.close();
                    }
                }
                else if (state == GameState::PLAYING) {
                    if (board.isCleaning()) {
                        // ignore clicks mientras limpia
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
                else if (state == GameState::PROGRESS) {
                
                    if (blev1.getGlobalBounds().contains(world)) {
                        state = GameState::PLAYING;

                        if (selectedPlayerIndex >= 0 && static_cast<size_t>(selectedPlayerIndex) < playersCount) {
                            Nodo<Jugador>* nodo = players.getCabeza();
                            size_t idx = 0;
                            while (nodo != nullptr && idx < static_cast<size_t>(selectedPlayerIndex)) {
                                nodo = nodo->siguiente;
                                ++idx;
                            }
                            if (nodo != nullptr) {

                                // sumamos al jugador
                                nodo->dato.nivel--;
                                board.setLevelPublic(1);
                                board.resetBoard();
                                savePlayersToFile(); // guardamos en el XML
                            }
                        }
                    }
                    if (blev2.getGlobalBounds().contains(world)) {
                        state = GameState::PLAYING;

                        if (selectedPlayerIndex >= 0 && static_cast<size_t>(selectedPlayerIndex) < playersCount) {
                            Nodo<Jugador>* nodo = players.getCabeza();
                            size_t idx = 0;
                            while (nodo != nullptr && idx < static_cast<size_t>(selectedPlayerIndex)) {
                                nodo = nodo->siguiente;
                                ++idx;
                            }
                            if (nodo != nullptr) {

                                // sumamos al jugador
                                nodo->dato.nivel--;
                                board.setLevelPublic(2);
                                board.resetBoard();
                                savePlayersToFile(); // guardamos en el XML
                            }
                        }
                    } if (blev3.getGlobalBounds().contains(world)) {
                        state = GameState::PLAYING;

                        if (selectedPlayerIndex >= 0 && static_cast<size_t>(selectedPlayerIndex) < playersCount) {
                            Nodo<Jugador>* nodo = players.getCabeza();
                            size_t idx = 0;
                            while (nodo != nullptr && idx < static_cast<size_t>(selectedPlayerIndex)) {
                                nodo = nodo->siguiente;
                                ++idx;
                            }
                            if (nodo != nullptr) {

                                // sumamos al jugador
                                nodo->dato.nivel--;
                                board.setLevelPublic(3);
                                board.resetBoard();
                                savePlayersToFile(); // guardamos en el XML
                            }
                        }
                    } if (blev4.getGlobalBounds().contains(world)) {
                        state = GameState::PLAYING;

                        if (selectedPlayerIndex >= 0 && static_cast<size_t>(selectedPlayerIndex) < playersCount) {
                            Nodo<Jugador>* nodo = players.getCabeza();
                            size_t idx = 0;
                            while (nodo != nullptr && idx < static_cast<size_t>(selectedPlayerIndex)) {
                                nodo = nodo->siguiente;
                                ++idx;
                            }
                            if (nodo != nullptr) {

                                // sumamos al jugador
                                nodo->dato.nivel--;
                                board.setLevelPublic(4);
                                board.resetBoard();
                                savePlayersToFile(); // guardamos en el XML
                            }
                        }

                    } if (blev5.getGlobalBounds().contains(world)) {
                        state = GameState::PLAYING;

                        if (selectedPlayerIndex >= 0 && static_cast<size_t>(selectedPlayerIndex) < playersCount) {
                            Nodo<Jugador>* nodo = players.getCabeza();
                            size_t idx = 0;
                            while (nodo != nullptr && idx < static_cast<size_t>(selectedPlayerIndex)) {
                                nodo = nodo->siguiente;
                                ++idx;
                            }
                            if (nodo != nullptr) {

                                // sumamos al jugador
                                nodo->dato.nivel--;
                                board.setLevelPublic(5);
                                board.resetBoard();
                                savePlayersToFile(); // guardamos en el XML
                            }
                        }
                    }
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                
                }
                else if (state == GameState::GAME_OVER) {
                 //   Nodo<Jugador>* nodo = players.getCabeza();
                    if (retryButton.getGlobalBounds().contains(world)) {
                      
                        board.resetBoard();
                        state = GameState::PLAYING;
                        return;
                    }
                    if (leaveButton.getGlobalBounds().contains(world)) {
                        if (selectedPlayerIndex >= 0 && static_cast<size_t>(selectedPlayerIndex) < playersCount) {
                            Nodo<Jugador>* nodo = players.getCabeza();
                            size_t idx = 0;
                            while (nodo != nullptr && idx < static_cast<size_t>(selectedPlayerIndex)) {
                                nodo = nodo->siguiente;
                                ++idx;
                            }
                            if (nodo != nullptr) {
                                
                                // sumamos al jugador
                                int playerLevel = nodo->dato.nivel++;
                                board.setLevelPublic(playerLevel+1);
                                board.resetBoard();;
                                savePlayersToFile(); // guardamos en el XML
                            }
                        }
                       
                        window.close();
                        return;
                    }
                    if (menuButton.getGlobalBounds().contains(world)) {
                       // board.setLevelPublic(1);



                        if (selectedPlayerIndex >= 0 && static_cast<size_t>(selectedPlayerIndex) < playersCount) {
                            Nodo<Jugador>* nodo = players.getCabeza();
                            size_t idx = 0;
                            while (nodo != nullptr && idx < static_cast<size_t>(selectedPlayerIndex)) {
                                nodo = nodo->siguiente;
                                ++idx;
                            }
                            if (nodo != nullptr) {
                                if (hardSelected == true) {
                                    nodo->dato.nivel--;
                                }
                                // sumamos al jugador
                                int playerLevel = nodo->dato.nivel++;
                                board.setLevelPublic(playerLevel+1);
                                board.resetBoard();;
                                savePlayersToFile(); // guardamos en el XML
                            }
                        }






                        board.resetBoard();
                        state = GameState::MENU;
                        hardSelected = false;
                        easyB.setFillColor(sf::Color(100, 255, 100, 255));
                        hardB.setFillColor(sf::Color(200, 200, 200, 255));
                        board.resetMoves(1);
                        board.setReiniciar();


                        

                        return;
                    }
                    if (board.getLevel() < 3 && nextLevelButton.getGlobalBounds().contains(world)) {
                        bool objectiveComplete = board.isObjectiveComplete();
                        bool canAdvance = (!hardSelected) || objectiveComplete;
                        if (canAdvance) {
                            if (selectedPlayerIndex >= 0 && static_cast<size_t>(selectedPlayerIndex) < playersCount) {
                                Nodo<Jugador>* nodo = players.getCabeza();
                                size_t idx = 0;
                                while (nodo != nullptr && idx < static_cast<size_t>(selectedPlayerIndex)) {
                                    nodo = nodo->siguiente;
                                    ++idx;
                                }
                                if (nodo != nullptr) {

                                    // sumamos al jugador
                                    int playerLevel = nodo->dato.nivel++;
                                    board.setLevelPublic(playerLevel+1);
                                    board.resetBoard();;
                                    savePlayersToFile(); // guardamos en el XML
                                }
                            }
                           // int playerLevel = nodo->dato.nivel++;
                           // board.setLevelPublic(playerLevel);
                            //board.resetBoard();
                            state = GameState::PLAYING;
                            return;
                        }
                    }
                }
            } // fin MouseButtonPressed
        } // fin while pollEvent
    }

    // ---------------- Update ----------------
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
                // Sumar puntaje del tablero al jugador seleccionado
                if (selectedPlayerIndex >= 0 && static_cast<size_t>(selectedPlayerIndex) < playersCount) {
                    Nodo<Jugador>* nodo = players.getCabeza();
                    size_t idx = 0;
                    while (nodo != nullptr && idx < static_cast<size_t>(selectedPlayerIndex)) {
                        nodo = nodo->siguiente;
                        ++idx;
                    }

                    if (nodo != nullptr) {
                        int partidaScore = board.getAcumulateScore();
                        nodo->dato.puntaje = partidaScore;
                        // sumamos al jugador
                       
                        savePlayersToFile(); // guardamos en el XML
                    }
                }

                state = GameState::GAME_OVER;
            }


            scoreText.setString("PUNTUACIÓN: " + std::to_string(board.getScore()));
            movesText.setString("MOVIMIENTOS: " + std::to_string(board.getRemainingMoves()));

            levelText.setString("Nivel: " + std::to_string(board.getLevel()));
            levelText.setPosition(360.f, 10.f);
            levelText.setCharacterSize(25);
        }
    }

    // ---------------- Render ----------------
    void render() {
        window.clear(sf::Color::White);

        if (state == GameState::USERS) {
            if (menuTexture.getSize().x != 0 && menuTexture.getSize().y != 0) window.draw(menuSprite);

            sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
            overlay.setFillColor(sf::Color(255, 255, 255, 200));
            overlay.setPosition(0.f, 0.f);
            window.draw(overlay);

            if (userTexture.getSize().x != 0 && userTexture.getSize().y != 0) window.draw(userSprite);
            window.draw(userLabel);
            window.draw(userButton);
            window.draw(listaLabel);
            window.draw(nombreU);
            window.draw(pointU);

            // Cursor parpadeante
            bool drawCursor = isTyping && (std::fmod(cursorClock.getElapsedTime().asSeconds(), 1.0f) < 0.5f);
            std::string display = nombre;
            if (drawCursor) display += "_";
            text.setString(display);
            window.draw(text);

            // DIBUJAR LISTA DE JUGADORES
            float playersX = 180.f;
            float playersY = 180.f;
            float playerW = 280.f, playerH = 36.f;
            float buttonW = 50.f, buttonH = 35.f;
            float gapY = 10.f;
            float btn1X = playersX + playerW + 8.f;
            float btn2X = playersX + playerW + 65.f;

            sf::Text playerNameText;
            playerNameText.setFont(font);
            playerNameText.setCharacterSize(20);
            playerNameText.setFillColor(sf::Color::Black);

            sf::Text Points;
            Points.setFont(font);
            Points.setCharacterSize(20);
            Points.setFillColor(sf::Color::Black);

            Nodo<Jugador>* nodo = players.getCabeza();
            for (size_t i = 0; i < playersCount && nodo != nullptr; ++i, nodo = nodo->siguiente) {
                float y = playersY + i * (playerH + gapY);

                // rectángulo del nombre
                sf::RectangleShape nameRect(sf::Vector2f(playerW, playerH));
                nameRect.setPosition(playersX, y);
                nameRect.setFillColor(sf::Color(220, 220, 220));
                nameRect.setOutlineThickness(2.f);
                nameRect.setOutlineColor(sf::Color::Black);
                window.draw(nameRect);

                // texto del nombre
                playerNameText.setString(nodo->dato.nombre);
                playerNameText.setPosition(playersX + 6.f, y + 6.f);
                window.draw(playerNameText);

                // puntos (del jugador)
                Points.setString(std::to_string(nodo->dato.puntaje));
                Points.setPosition(380.f, y + 6.f);
                window.draw(Points);

                // botón azul (MENU)
                sf::RectangleShape btn(sf::Vector2f(buttonW, buttonH));
                btn.setPosition(btn1X, y);
                btn.setFillColor(sf::Color(100, 200, 255));
                btn.setOutlineThickness(2.f);
                btn.setOutlineColor(sf::Color::Black);
                window.draw(btn);

                sf::Text btnText;
                btnText.setFont(font);
                btnText.setCharacterSize(14);
                btnText.setFillColor(sf::Color::Black);
                btnText.setString("MENU");
                float bx = btn1X + (buttonW - btnText.getLocalBounds().width) / 2.f - 4.f;
                float by = y + (buttonH - btnText.getCharacterSize()) / 2.f - 2.f;
                btnText.setPosition(bx, by);
                window.draw(btnText);

                // botón rojo Eliminar
                sf::RectangleShape btn2(sf::Vector2f(buttonW, buttonH));
                btn2.setPosition(btn2X, y);
                btn2.setFillColor(sf::Color(200, 0, 0));
                btn2.setOutlineThickness(2.f);
                btn2.setOutlineColor(sf::Color::Black);
                window.draw(btn2);

                sf::Text btnText2;
                btnText2.setFont(font);
                btnText2.setCharacterSize(12);
                btnText2.setFillColor(sf::Color::Black);
                btnText2.setString("Eliminar");
                float bx2 = btn2X + (buttonW - btnText2.getLocalBounds().width) / 2.f - 4.f;
                float by2 = y + (buttonH - btnText2.getCharacterSize()) / 2.f - 2.f;
                btnText2.setPosition(bx2, by2);
                window.draw(btnText2);


                sf::RectangleShape btn3(sf::Vector2f(buttonW, buttonH));
                btn3.setPosition(btn2X+55, y);
                btn3.setFillColor(sf::Color(200, 100, 50));
                btn3.setOutlineThickness(2.f);
                btn3.setOutlineColor(sf::Color::Black);
                window.draw(btn3);

                sf::Text btnText3;
                btnText3.setFont(font);
                btnText3.setCharacterSize(12);
                btnText3.setFillColor(sf::Color::Black);
                btnText3.setString("Reiniciar");
                float bx3 = btn2X+60 + (buttonW - btnText3.getLocalBounds().width) / 2.f - 4.f;
                float by3 = y + (buttonH - btnText3.getCharacterSize()) / 2.f - 2.f;
                btnText3.setPosition(bx3, by3);
                window.draw(btnText3);


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
            window.draw(progress);
            window.draw(Tprogress);
            if (!nombre.empty()) {
                sf::Text current;
                current.setFont(font);
                current.setCharacterSize(20);
                current.setFillColor(sf::Color::Black);
                current.setString("Jugador: " + nombre);
                current.setPosition(5.f, 10.f);
                window.draw(current);
            }
        }
        else if (state == GameState::PROGRESS) {
            if (menuTexture.getSize().x != 0 && menuTexture.getSize().y != 0) window.draw(menuSprite);

            sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
            overlay.setFillColor(sf::Color(255, 255, 255, 200));
            overlay.setPosition(0.f, 0.f);
            window.draw(overlay);
            int playerscore=0;
           // int level = 0;
            if (userTexture.getSize().x != 0 && userTexture.getSize().y != 0) window.draw(userSprite);
            string comp;
            if (selectedPlayerIndex >= 0 && static_cast<size_t>(selectedPlayerIndex) < playersCount) {
                Nodo<Jugador>* nodo = players.getCabeza();
                size_t idx = 0;
                while (nodo != nullptr && idx < static_cast<size_t>(selectedPlayerIndex)) {
                    nodo = nodo->siguiente;
                    ++idx;
                }
                if (nodo != nullptr) {

                    // sumamos al jugador
                     playerscore = nodo->dato.puntaje;
                     nivel= nodo->dato.nivel;
                 //   board.setLevelPublic(playerLevel + 1);
                  // guardamos en el XML
                }
            }
         /*   int nivel1;
            int c = 0;
            if (c == 0) {
                 nivel1 = nivel;
            }
            c++;*/
            if (nivel>0) {
                level1.setString("Nivel 1: Bloqueado   " + std::to_string(playerscore));
                level2.setString("Nivel 2: Bloqueado   " + std::to_string(playerscore));
                level3.setString("Nivel 3: Bloqueado   " + std::to_string(playerscore));
                level4.setString("Nivel 4: Bloqueado   " + std::to_string(playerscore));
                level5.setString("Nivel 5: Bloqueado   " + std::to_string(playerscore));
            }
           if(nivel > 1) {
                level2.setString("Nivel 2: Bloqueado   " + std::to_string(playerscore));
                level3.setString("Nivel 3: Bloqueado   " + std::to_string(playerscore));
                level4.setString("Nivel 4: Bloqueado   " + std::to_string(playerscore));
                level5.setString("Nivel 5: Bloqueado   " + std::to_string(playerscore));
                level1.setString("Nivel 1: Completado   " + std::to_string(playerscore));
                blev1.setPosition(500, 200);
                window.draw(blev1);
               
            }
          if (nivel > 2) {
                level3.setString("Nivel 3: Bloqueado   " + std::to_string(playerscore));
                level4.setString("Nivel 4: Bloqueado   " + std::to_string(playerscore));
                level5.setString("Nivel 5: Bloqueado   " + std::to_string(playerscore));
                level1.setString("Nivel 1: Desbloqueado   " + std::to_string(playerscore));
                level2.setString("Nivel 2: Desbloqueado   " + std::to_string(playerscore));
                blev2.setPosition(500, 250);
                window.draw(blev2);
           }
          
            if (nivel > 3) {
                level4.setString("Nivel 4: Bloqueado   " + std::to_string(playerscore));
                level5.setString("Nivel 5: Bloqueado   " + std::to_string(playerscore));
                level1.setString("Nivel 1: Desbloqueado   " + std::to_string(playerscore));
                level2.setString("Nivel 2: Desbloqueado   " + std::to_string(playerscore));
                level3.setString("Nivel 3: Desbloqueado   " + std::to_string(playerscore));
                blev3.setPosition(500, 300);
                window.draw(blev3);
            }
       
           
                
            if (nivel > 4) {
                level5.setString("Nivel 5: Bloqueado   " + std::to_string(playerscore));
                level1.setString("Nivel 1: Desbloqueado   " + std::to_string(playerscore));
                level2.setString("Nivel 2: Desbloqueado   " + std::to_string(playerscore));
                level3.setString("Nivel 3: Desbloqueado   " + std::to_string(playerscore));
                level4.setString("Nivel 4: Desbloqueado   " + std::to_string(playerscore));
                blev4.setPosition(500, 350);
                window.draw(blev4);
            }
            if (nivel > 5) {
                level5.setString("Nivel 5: Desbloqueado   " + std::to_string(playerscore));
                level1.setString("Nivel 1: Desbloqueado   " + std::to_string(playerscore));
                level2.setString("Nivel 2: Desbloqueado   " + std::to_string(playerscore));
                level3.setString("Nivel 3: Desbloqueado   " + std::to_string(playerscore));
                level4.setString("Nivel 4: Desbloqueado   " + std::to_string(playerscore));
                blev5.setPosition(500, 400);
                window.draw(blev5);
            }


            window.draw(level1);
            window.draw(level2);
            window.draw(level3);
            window.draw(level4);
            window.draw(level5);
       
            //window.draw(btn1);
        
        }
        else if (state == GameState::PLAYING) {
            if (menuTexture.getSize().x != 0 && menuTexture.getSize().y != 0) window.draw(menuSprite);
            if (backgroundTexture.getSize().x != 0 && backgroundTexture.getSize().y != 0) window.draw(backgroundSprite);
            board.draw(window);

            window.draw(scoreText);
            window.draw(movesText);
            window.draw(levelText);

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
            }
            else resultText.setString("");
            resultText.setPosition(110.f, 140.f);
            window.draw(resultText);

            if (board.getLevel() < 3) {
                if (!hardSelected) {
                    window.draw(nextLevelButton);
                    window.draw(nextLevelText);
                }
                else {
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

    // ---------------- Run ----------------
    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }
};

#endif // GAME_H

