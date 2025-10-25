#ifndef LISTA_H
#define LISTA_H

#include <iostream>

// Nodo template
template <typename T>
class Nodo {
public:
    T dato;
    Nodo<T>* siguiente;

    Nodo(const T& d) : dato(d), siguiente(nullptr) {}
};

// Lista template (simple lista enlazada)
template <typename T>
class Lista {
private:
    Nodo<T>* cabeza;

public:
    Lista() : cabeza(nullptr) {}

    ~Lista() {
        vaciar();
    }

    bool estaVacia() const {
        return cabeza == nullptr;
    }

    // agregar al inicio
    void agregarInicio(const T& dato) {
        Nodo<T>* nuevo = new Nodo<T>(dato);
        nuevo->siguiente = cabeza;
        cabeza = nuevo;
    }

    // agregar al final
    void agregarFinal(const T& dato) {
        Nodo<T>* nuevo = new Nodo<T>(dato);
        if (estaVacia()) {
            cabeza = nuevo;
            return;
        }
        Nodo<T>* aux = cabeza;
        while (aux->siguiente != nullptr) aux = aux->siguiente;
        aux->siguiente = nuevo;
    }

    // eliminar el primer nodo cuyo dato sea igual (usa operador==)
    // retorna true si se eliminó
    bool eliminar(const T& dato) {
        if (estaVacia()) return false;
        Nodo<T>* actual = cabeza;
        Nodo<T>* anterior = nullptr;
        while (actual != nullptr) {
            if (actual->dato == dato) {
                if (anterior == nullptr) {
                    cabeza = actual->siguiente;
                }
                else {
                    anterior->siguiente = actual->siguiente;
                }
                delete actual;
                return true;
            }
            anterior = actual;
            actual = actual->siguiente;
        }
        return false;
    }

    // vaciar lista
    void vaciar() {
        Nodo<T>* aux = cabeza;
        while (aux != nullptr) {
            Nodo<T>* sig = aux->siguiente;
            delete aux;
            aux = sig;
        }
        cabeza = nullptr;
    }

    // imprimir (útil para debug) — requiere que T tenga operator<<
    void imprimir() const {
        Nodo<T>* aux = cabeza;
        while (aux != nullptr) {
            std::cout << aux->dato << " -> ";
            aux = aux->siguiente;
        }
        std::cout << "NULL\n";
    }

    // obtener puntero a la cabeza (útil para iterar)
    Nodo<T>* getCabeza() const {
        return cabeza;
    }
};

#endif // LISTA_H
