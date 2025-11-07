#include "Game.h"
//#include "Jugador.h"

// ---------------------- BombFruit onMatch implementation ----------------------
void BombFruit::onMatch(Board* board, int row, int col) {
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = row + dr;
            int nc = col + dc;
            board->markCellForRemoval(nr, nc, true, true);  // Agregado: animate = true
        }
    }
}
int Board::getTypeAt(int r, int c) const {
    if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) return -1;
    if (!matrix[r][c]) return -1;
    return matrix[r][c]->getType();
}

void SuperFruit::onMatch(Board* board, int row, int col) {
    int fullType = getType();
    int idx = -1;
    if (fullType >= 7 && fullType <= 11) idx = fullType - 7;
    else if (fullType >= 0 && fullType <= 4) idx = fullType;

    if (idx < 0) {
        return;
    }

    int N = board->getSize();

    auto tryMark = [&](int r, int c) {
        if (r >= 0 && r < N && c >= 0 && c < N) {
            board->markCellForRemoval(r, c, true, true);  // animate = true

        }
        };

    switch (idx) {
    case 0:

        for (int c = 0; c < N; ++c) tryMark(row, c);
        break;

    case 1:

        for (int r = 0; r < N; ++r) tryMark(r, col);
        break;

    case 2:

        for (int c = 0; c < N; ++c) tryMark(row, c);  // Fila
        for (int r = 0; r < N; ++r) tryMark(r, col);  // Columna
        break;

    case 3:

        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc)
                tryMark(row + dr, col + dc);
        break;

    case 4:

        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                int t = board->getTypeAt(r, c);
                if (t == idx) tryMark(r, c);
            }
        }
        break;
    }
}

int main() {
    Game game;
    game.run();
}





