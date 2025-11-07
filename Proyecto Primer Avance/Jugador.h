#pragma once
// ---------------- Clase Jugador ----------------
class Jugador {
public:
    std::string nombre;
    int puntaje;
    int nivel;
    int level1, level2, level3, level4, level5;
    Jugador() : nombre(""), puntaje(0), nivel(0) { level1 = 0; level2 = 0; level3 = 0; level4 = 0; level5 = 0; }
    Jugador(const std::string& n, int p, int l, int l1, int l2, int l3, int l4, int l5) : nombre(n), puntaje(p), nivel(l), level1(l1), level2(l2), level3(l3), level4(l4), level5(l5) {

    }

    bool operator==(const Jugador& other) const {
        return nombre == other.nombre;
    }
};