#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
using namespace std;
// Initializes a single gem on the board.
class Gem {
private:
    sf::Sprite sprite;
    int tipo;          

public:
    // Constructor:
    // Ajusta el origen al centro para que las escalas y movimientos
  
    Gem(sf::Texture& textura, sf::Vector2f posicionTopLeft, int tipoGem) {
        sprite.setTexture(textura);
        // origen en el centro para que el escalado mantenga centrado el sprite
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        sprite.setPosition(posicionTopLeft.x + bounds.width / 2.f, posicionTopLeft.y + bounds.height / 2.f);
        tipo = tipoGem;
    }


    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }


    void setPosition(float xTopLeft, float yTopLeft) {
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setPosition(xTopLeft + bounds.width / 2.f, yTopLeft + bounds.height / 2.f);
    }

   
    int getTipo() const { return tipo; }
    void setTipo(int nuevoTipo) { tipo = nuevoTipo; }

    // Modifica el tamaño visual.
    void setScale(float sx, float sy) { sprite.setScale(sx, sy); }

    void resetVisual() { sprite.setScale(1.f, 1.f); }
};

// Esta clase guarda el estado del tablero (8x8), las texturas,
// el puntaje, los movimientos y toda la lógica del juego:
// - detectar combinaciones
// - eliminar gemas
// - aplicar gravedad
// - rellenar gemas nuevas
// - seleccionar e intercambiar gemas
//
class Board {
private:
    Gem* matriz[8][8];   
    sf::Texture texturas[5];
    int movimientosRestantes = 3, puntaje = 0;
    
    // Parar el tiempo cuando limpia el tablero
    bool cleaningInProgress = false;        
    bool contarPuntajeEnCleaning = true;    
    // Acciones de Gem
    bool marked[8][8]; 
    bool removalPending = false; 
    const float MARK_SCALE = 1.35f;  
    const float SELECT_SCALE = 1.35f; 

    int selectedRow = -1; 
    int selectedCol = -1; 

    // coordenadas fijas para usar como sistema lógico de referencia
   
    const float originX = 117.f;
    const float originY = 100.f;
    const float cellW = 69.f; // ancho de celda
    const float cellH = 60.f; // alto de celda

public:
   
    // Inicializa variables, carga texturas, crea la matriz con gemas aleatorias
    // y limpia combos iniciales (sin puntuar) hasta que el tablero quede estable.
    Board() {
        srand(time(nullptr));
       texturas[0].loadFromFile("C:/Joshua/practica/Sprites/imagen_80x50.png");
       texturas[1].loadFromFile("C:/Joshua/practica/Sprites/orange_80x50.png");
       texturas[2].loadFromFile("C:/Joshua/practica/Sprites/assets_task_01k242ywmqejmrtpc1fanwmbef_1754631571_img_1.png");
       texturas[3].loadFromFile("C:/Joshua/practica/Sprites/assets_task_01k2433kxffdws9dppcbqme0ev_1754631736_img_1.png");
       texturas[4].loadFromFile("C:/Joshua/practica/Sprites/assets_task_01k242rgp9fs3scwggxmte3ds5_1754631366_img_1.png");
        // Cada celda recibe una Gem nueva con textura aleatoria.
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                int tipo = std::rand() % 5;
                matriz[i][j] = new Gem(texturas[tipo], sf::Vector2f(j * cellW + originX, i * cellH + originY), tipo);
            }
        }
        // Esto evita que el tablero empiece con combinaciones ya resueltas.
        bool sigue = true;
        while (sigue) {
            if (limpiarCombosUnaVez(false)) {
                aplicarGravedadYReemplazar();
            }
            else {
                sigue = false;
            }
        }
    }
    bool detectarCombinacionesSinEliminar() {
        // horizontales
        for (int i = 0; i < 8; ++i) {
            int contador = 1;
            for (int j = 1; j < 8; ++j) {
                if (matriz[i][j] != nullptr && matriz[i][j - 1] != nullptr && (*matriz[i][j]).getTipo() == (*matriz[i][j - 1]).getTipo()) {
                    ++contador;
                    if (contador >= 3) return true; // si encontramos 3 iguales seguidos, hay combo
                }
                else {
                    contador = 1; 
                }
            }
        }
        // verticales
        for (int j = 0; j < 8; ++j) {
            int contador = 1;
            for (int i = 1; i < 8; ++i) {
                if (matriz[i][j] != nullptr && matriz[i - 1][j] != nullptr && (*matriz[i][j]).getTipo() == (*matriz[i - 1][j]).getTipo()) {
                    ++contador;
                    if (contador >= 3) return true;
                }
                else {
                    contador = 1;
                }
            }
        }
        return false; // no se encontró ninguna secuencia
    }
// Solo detecta si hay combo(no intercambia)
    bool formaCombinacionAlIntercambiar(int f1, int c1, int f2, int c2) {
        Gem* temp = matriz[f1][c1];
        matriz[f1][c1] = matriz[f2][c2];
        matriz[f2][c2] = temp;
        bool hayCombo = detectarCombinacionesSinEliminar();
        temp = matriz[f1][c1];
        matriz[f1][c1] = matriz[f2][c2];
        matriz[f2][c2] = temp;
        return hayCombo;
    }

    // Esta función detecta combos y los  marca y elimina.
    // Dos modos el primero antes de empezar(no cuenta puntaje)
    // El segundo cuando se esta jugando (si cuenta pumtaje)
    //     * Si removalPending == false: marca las gemas (cambia su escala) y pone removalPending=true
    //     * Si removalPending == true: elimina las gemas marcadas, suma puntaje, resetea marcas.
    bool limpiarCombosUnaVez(bool contarPuntaje) {
        if (!contarPuntaje) {
              // Modo Inicializacion (contarpuntaje == false)

            bool huboEliminacion = false;
            bool marcadasLocal[8][8] = { false };

            // horizontales: escanear cada fila buscando "runs" del mismo tipo
            for (int i = 0; i < 8; ++i) {
                int j = 0;
                while (j < 8) {
                    if (matriz[i][j] == nullptr) { ++j; continue; } // salta celdas vacías
                    int tipo = (*matriz[i][j]).getTipo();
                    int k = j + 1;
                   
                    while (k < 8 && matriz[i][k] != nullptr && (*matriz[i][k]).getTipo() == tipo) ++k;
                    int runLen = k - j;
                    if (runLen >= 3) {
               
                        for (int t = j; t < k; ++t) marcadasLocal[i][t] = true;
                        huboEliminacion = true;
                    }
                    j = k; 
                }
            }

            // verticales: mismo procedimiento por columnas
            for (int j = 0; j < 8; ++j) {
                int i = 0;
                while (i < 8) {
                    if (matriz[i][j] == nullptr) { ++i; continue; }
                    int tipo = (*matriz[i][j]).getTipo();
                    int k = i + 1;
                    while (k < 8 && matriz[k][j] != nullptr && (*matriz[k][j]).getTipo() == tipo) ++k;
                    int runLen = k - i;
                    if (runLen >= 3) {
                        for (int t = i; t < k; ++t) marcadasLocal[t][j] = true;
                        huboEliminacion = true;
                    }
                    i = k;
                }
            }

            // si hubo eliminaciones, borrar físicamente las gemas marcadas
            if (huboEliminacion) {
                for (int i = 0; i < 8; ++i) {
                    for (int j = 0; j < 8; ++j) {
                        if (marcadasLocal[i][j] && matriz[i][j] != nullptr) {
                            delete matriz[i][j];     
                            matriz[i][j] = nullptr; 
                        }
                    }
                }
            }

            return huboEliminacion; // true si se eliminó al menos una gema
        }

        // MODO JUEGO (contarPuntaje == true)
       
        if (!removalPending) {
         //detectar y marcar
            bool huboEliminacion = false;
            bool marcadasLocal[8][8] = { false };

            // marcar horizontales
            for (int i = 0; i < 8; ++i) {
                int j = 0;
                while (j < 8) {
                    if (matriz[i][j] == nullptr) { ++j; continue; }
                    int tipo = (*matriz[i][j]).getTipo();
                    int k = j + 1;
                    while (k < 8 && matriz[i][k] != nullptr && (*matriz[i][k]).getTipo() == tipo) ++k;
                    int runLen = k - j;
                    if (runLen >= 3) {
                        for (int t = j; t < k; ++t) marcadasLocal[i][t] = true;
                        huboEliminacion = true;
                    }
                    j = k;
                }
            }

            // marcar verticales
            for (int j = 0; j < 8; ++j) {
                int i = 0;
                while (i < 8) {
                    if (matriz[i][j] == nullptr) { ++i; continue; }
                    int tipo = (*matriz[i][j]).getTipo();
                    int k = i + 1;
                    while (k < 8 && matriz[k][j] != nullptr && (*matriz[k][j]).getTipo() == tipo) ++k;
                    int runLen = k - i;
                    if (runLen >= 3) {
                        for (int t = i; t < k; ++t) marcadasLocal[t][j] = true;
                        huboEliminacion = true;
                    }
                    i = k;
                }
            }
            if (huboEliminacion) {
                for (int i = 0; i < 8; ++i) {
                    for (int j = 0; j < 8; ++j) {
                        marked[i][j] = marcadasLocal[i][j];
                        if (marked[i][j] && matriz[i][j] != nullptr) {
                            (*matriz[i][j]).setScale(MARK_SCALE, MARK_SCALE); // agrandar para marcar
                        }
                    }
                }
                removalPending = true; 
                return true; // indica que hubo trabajo (pero aún no se borró)
            }
            return false;
        }
        else {
       // eliminar las marcadas y sumar puntaje
            bool huboEliminacion = false;
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    if (marked[i][j] && matriz[i][j] != nullptr) {
                        delete matriz[i][j];
                        matriz[i][j] = nullptr;
                        huboEliminacion = true;
                        puntaje += 10; // suma 10 puntos por cada gema eliminada
                    }
                }
            }

            // limpiar marcas y resetear el aspecto visual de las gemas que quedaron
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    marked[i][j] = false;
                    if (matriz[i][j] != nullptr) (*matriz[i][j]).resetVisual();
                }
            }

            removalPending = false; // terminamos la fase de eliminación
            return huboEliminacion; // indica si se eliminó al menos una gema
        }
    }

    // Hace que las gemas "caigan"  y rellenacon gemas nuevas arriba cuando hay celdas vacías.
    void aplicarGravedadYReemplazar() {
        for (int col = 0; col < 8; ++col) {
          // Aplica gravedad
            for (int fila = 7; fila >= 0; --fila) {
                if (matriz[fila][col] == nullptr) {
                    int k = fila - 1;
                    while (k >= 0 && matriz[k][col] == nullptr) --k;
                    if (k >= 0) {
                        // mover el puntero a la posición inferior
                        matriz[fila][col] = matriz[k][col];
                        matriz[k][col] = nullptr;
                        (*matriz[fila][col]).setPosition(col * cellW + originX, fila * cellH + originY);
                        (*matriz[fila][col]).resetVisual();
                    }
                }
            }
            // rellena espacios vacios
            for (int fila = 0; fila < 8; ++fila) {
                if (matriz[fila][col] == nullptr) {
                    int tipo = std::rand() % 5;
                    matriz[fila][col] = new Gem(texturas[tipo], sf::Vector2f(col * cellW + originX, fila * cellH + originY), tipo);
                }
            }
        }
    }
    // Destructor: liberar memoria de todas las gemas
    ~Board() {
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                if (matriz[i][j] != nullptr) delete matriz[i][j];
    }

    // Dibuja todas las gemas en pantalla.
    void draw(sf::RenderWindow& window) {
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                if (matriz[i][j] != nullptr)
                    (*matriz[i][j]).draw(window);
    }

    // Intenta intercambiar dos celdas adyacentes si se forma una combinacion.
   // inicia el proceso de limpieza (startCleaning).
    bool intercambiar(int f1, int c1, int f2, int c2) {
      
        if (f1 < 0 || f1 >= 8 || c1 < 0 || c1 >= 8 ||
            f2 < 0 || f2 >= 8 || c2 < 0 || c2 >= 8) return false;

        // comprobar adyacencia.
        int df = f1 - f2; if (df < 0) df *= -1;
        int dc = c1 - c2; if (dc < 0) dc *= -1;
        if (!((df == 1 && dc == 0) || (df == 0 && dc == 1))) return false;

        // validar si el swap produce combo
        if (!formaCombinacionAlIntercambiar(f1, c1, f2, c2)) return false;

       
        Gem* temp = matriz[f1][c1];
        matriz[f1][c1] = matriz[f2][c2];
        matriz[f2][c2] = temp;
        // actualizar posiciones visuales (top-left coords)
        if (matriz[f1][c1] != nullptr) (*matriz[f1][c1]).setPosition(c1 * cellW + originX, f1 * cellH + originY);
        if (matriz[f2][c2] != nullptr) (*matriz[f2][c2]).setPosition(c2 * cellW + originX, f2 * cellH + originY);

        --movimientosRestantes;
        startCleaning(true);    
        return true;
    }


    // Marca que el tablero entra en modo "limpieza" y define si debe
    // contarse puntaje durante la limpieza.
    void startCleaning(bool contarP) {
        cleaningInProgress = true;
       
    }

  
    // Ejecuta un paso de la limpieza (se llama repetidamente con delay
    bool stepCleaning() {
        if (!cleaningInProgress) return false;
        bool hubo = limpiarCombosUnaVez(contarPuntajeEnCleaning);
        if (hubo) {
            if (removalPending) {
                return true;
            }
            else {
                aplicarGravedadYReemplazar();
                return true;
            }
        }
        else {
            cleaningInProgress = false; // ya no hay nada que limpiar
            return false;
        }
    }

    // consulta si hay limpieza en curso.
    bool isCleaning() const { return cleaningInProgress; }

   
    //  ejecuta limpieza completa hasta que no queden combos.
    void limpiarCombosRepetidamente(bool contarPuntaje = true) {
        bool sigue = true;
        while (sigue) {
            if (limpiarCombosUnaVez(contarPuntaje)) {
                aplicarGravedadYReemplazar();
            }
            else sigue = false;
        }
    }

    // Accesores para puntaje y movimientos
    int getPuntaje() const { return puntaje; }
    int getMovimientosRestantes() const { return movimientosRestantes; }
    bool quedanMovimientos() const { return movimientosRestantes > 0; }

    void resetMovimientos(int n) { movimientosRestantes = n; }


    // Lógica de control de selección por clicks:
    // - primer click: selecciona la gema (la escala)
    // - segundo click:
    //    * si es la misma celda -> deselecciona
    //    * si es adyacente -> intenta intercambiar (y si es válido, procesa limpieza)
    void selectOrSwap(int fila, int col) {
        if (selectedRow == -1) {
            selectedRow = fila; selectedCol = col;
            (*matriz[selectedRow][selectedCol]).setScale(SELECT_SCALE, SELECT_SCALE);
          
        }
        else {
            bool ok = intercambiar(selectedRow, selectedCol, fila, col);
            (*matriz[selectedRow][selectedCol]).resetVisual();
            selectedRow = -1; selectedCol = -1;
            
        }
    }


    // Se utiliza para convertir la posición del ratón en índices de fila/columna (SMFL).
    pair <int, int> screenToCell(sf::RenderWindow& win, int mouseX, int mouseY) {
        sf::Vector2f world = win.mapPixelToCoords(sf::Vector2i(mouseX, mouseY));
        int col = int(std::floor((world.x - originX) / cellW));
        int row = int(std::floor((world.y - originY) / cellH));
        return { row, col };
    }

    // Elimina el tablero y crea otro como en el constructor
    void resetBoard(int newMoves = 20) {
        movimientosRestantes = newMoves;
        puntaje = 0;
        cleaningInProgress = false;
        contarPuntajeEnCleaning = true;
        removalPending = false;
        selectedRow = -1; selectedCol = -1;
      

           // borrar gemas actuales (liberar memoria)
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (matriz[i][j] != nullptr) {
                    delete matriz[i][j];
                    matriz[i][j] = nullptr;
                }
            }
        }

        // crear nueva matriz con gemas aleatorias
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                int tipo = std::rand() % 5;
                matriz[i][j] = new Gem(texturas[tipo], sf::Vector2f(j * cellW + originX, i * cellH + originY), tipo);
            }
        }

      
        bool sigue = true;
        while (sigue) {
            if (limpiarCombosUnaVez(false)) {
                aplicarGravedadYReemplazar();
            }
            else {
                sigue = false;
            }
        }
    }
};


class Game {
private:
    sf::RenderWindow window{sf::VideoMode(800, 600), "Match-3"};
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
    sf::RectangleShape menuButton;
    sf::Text menuButtonText;
    sf::Text titleText;

    // GAME_OVER UI
    sf::RectangleShape retryButton;
    sf::Text retryButtonText;
    sf::RectangleShape menuButtonFromOver;
    sf::Text menuButtonFromOverText;
    sf::Text gameOverText;

    // Estado del juego: MENU, PLAYING o GAME_OVER
    enum class GameState { MENU, PLAYING, GAME_OVER };
    GameState state = GameState::MENU;

public:
  
    Game(){
     
        // fijamos la vista lógica en 800x600 (igual que tu diseño original)
        sf::View view(sf::FloatRect(0.f, 0.f, 800.f, 600.f));
        window.setView(view);

        // cargar recursos (fuente y fondos)
        if (!fuente.loadFromFile("C:/Users/joshu/Downloads/Arimo/Arimo-VariableFont_wght.ttf")) {
            cout << "No se pudo cargar la fuente (ajusta la ruta)\n";
        }

        if (!fondoTexture.loadFromFile("C:/Joshua/practica/Sprites/Fondo5.png")) {
            std::cout << "Error cargando fondo (Fondo5.png)\n";
        }
        else {
        
            fondoSprite.setTexture(fondoTexture);
            fondoSprite.setPosition(124.f, 110.f);
        }

        if (!fondoTexture1.loadFromFile("C:/Joshua/practica/Sprites/fondo 6.png")) {
            std::cout << "Error cargando fondo1 (fondo 6.png)\n";
        }
        else {
            fondoSprite1.setTexture(fondoTexture1);
            if (fondoTexture1.getSize().x != 0 && fondoTexture1.getSize().y != 0) {
                fondoSprite1.setOrigin(0.f, 0.f);
                fondoSprite1.setScale(800.f / (float)fondoTexture1.getSize().x, 600.f / (float)fondoTexture1.getSize().y);
            }
        }

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
        menuButton.setSize(sf::Vector2f(200.f, 80.f));
        menuButton.setPosition(300.f, 300.f);
        menuButton.setFillColor(sf::Color::Green);

        menuButtonText.setFont(fuente);
        menuButtonText.setCharacterSize(32);
        menuButtonText.setString("JUGAR");
        menuButtonText.setFillColor(sf::Color::Black);
        menuButtonText.setPosition(menuButton.getPosition().x + 47, menuButton.getPosition().y + 20);

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

        menuButtonFromOver.setSize(sf::Vector2f(180.f, 60.f));
        menuButtonFromOver.setOrigin(menuButtonFromOver.getSize() / 2.f);
        menuButtonFromOver.setPosition(400.f, 420.f);
        menuButtonFromOver.setFillColor(sf::Color(200, 200, 200));
        menuButtonFromOver.setOutlineThickness(3.f);
        menuButtonFromOver.setOutlineColor(sf::Color::Black);

        menuButtonFromOverText.setFont(fuente);
        menuButtonFromOverText.setCharacterSize(28);
        menuButtonFromOverText.setString("Salir");
        menuButtonFromOverText.setFillColor(sf::Color::Black);
        menuButtonFromOverText.setPosition(menuButtonFromOver.getPosition().x - 30, menuButtonFromOver.getPosition().y - 25 + 6.f);

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
                    if (menuButton.getGlobalBounds().contains(world)) {
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
                    else if (menuButtonFromOver.getGlobalBounds().contains(world)) {
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
            window.draw(menuButton);
            window.draw(menuButtonText);
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
            window.draw(menuButtonFromOver);
            window.draw(menuButtonFromOverText);

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
int main() {
    Game game;
    game.run();
}
