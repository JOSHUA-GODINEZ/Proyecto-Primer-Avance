#pragma once
#include "Gem.h"
// Guarda el estado del tablero (8x8) y las acciones de las gemas
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
        texturas[0].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Apple.png");
        texturas[1].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Orange.png");
        texturas[2].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Banana.png");
        texturas[3].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Watermelon.png");
        texturas[4].loadFromFile("C:\\Joshua\\Proyecto Primer Avance\\Assets\\Sprite Grape.png");
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