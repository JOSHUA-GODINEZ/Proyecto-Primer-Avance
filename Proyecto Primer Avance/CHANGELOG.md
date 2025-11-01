# CHANGELOG


 Primer Avance Versión 1.0 

Fecha:

Del 31 de julio hasta el 16 de setiembre del 2025

Descripción general:
Implementación inicial del sistema de juego básico con tablero y lógica principal.

Cambios principales:

Creación de la clase Board con un tablero de 8x8.

Gestión de gemas normales con textura y posición en celdas.

Implementación de detección de combinaciones y eliminación básica.

Efecto de gravedad para rellenar espacios vacíos.

Regeneración de gemas nuevas al caer.

Interfaz simple con SFML mostrando el tablero.

Puntaje básico y contador de movimientos.

Segundo Avance Versión 2.0

Fecha:

Del 25 de setiembre hasta el 20 de Octubre del 2025

Descripción general:
Refactorización completa con Programación Orientada a Objetos, herencia, polimorfismo y gemas especiales.

Cambios principales:

Reestructuración del código en clases: Game, Board, Fruit y subclases (NormalFruit, BombFruit, IceFruit, SuperFruit).

Implementación de jerarquía con métodos virtuales (draw(), onMatch(), getType()).

Nuevas gemas poderosas creadas al combinar 4 o más frutas.

Lógica de efectos especiales al activarlas (bomba, línea completa, hielo, etc.).

Sistema de objetivos por nivel, mostrando progreso visual en pantalla.

Añadidos obstáculos (bloques de hielo) que requieren más de un match para eliminarse.

Animaciones de:

Intercambio entre frutas.

Eliminación con efecto de expansión.

Caída por gravedad.

Tres niveles diferentes con objetivos y dificultad creciente.

Manejo de errores con validaciones y uso de excepciones.

Mejoras visuales y mayor robustez en la interfaz.

Proyecto Final Versión 3.0

Fecha:

Del 25 Octubre 2025 hasta el 12 de noviembre del 2025

Descripción general:
Entrega final del proyecto con persistencia, menús, sistema de progreso y documentación profesional.

Cambios principales:

Implementación del sistema de carga y guardado de jugadores en XML (nombre, puntaje y nivel).

Sistema de niveles cargados desde archivo con configuración y objetivos dinámicos.

Ranking de jugadores con persistencia local y reinicio individual.

Menú principal, de usuario, progreso y juego, todos gestionados por estados (GameState):

USERS – Creación y selección de jugador.

MENU – Opciones principales, dificultad y salida.

PROGRESS – Niveles desbloqueados, puntajes y reinicio.

PLAYING – Tablero activo del juego.

GAME_OVER – Fin de partida con opciones de reinicio o siguiente nivel.

Sistema de niveles desbloqueables con progreso guardado.

Informe técnico PDF y README.md con instrucciones y diagramas.

Limpieza final del código y manejo de memoria.

Versión compilada funcional (Windows).