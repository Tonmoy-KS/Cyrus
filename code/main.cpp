#include <iostream>
#include "Cyrus.h"

int main() {
    Board board;
    Engine engine;

    std::cout << "--- C++ Cyrus Engine ---" << std::endl;
    board.printBoard();

    // A simple game loop
    for(int i = 0; i < 10; ++i) { // Play 10 moves for demo
        std::cout << (board.sideToMove == WHITE ? "White" : "Black") << " to move." << std::endl;
        Move bestMove = engine.findBestMove(board);

        if (bestMove.from == -1) {
            std::cout << "No legal moves. Game over." << std::endl;
            break;
        }

        std::cout << "Engine plays: "
                  << (char)('a' + bestMove.from % 8) << (bestMove.from / 8 + 1)
                  << (char)('a' + bestMove.to % 8) << (bestMove.to / 8 + 1)
                  << std::endl;

        board.makeMove(bestMove); // This needs to be fully implemented for a real game
        board.printBoard();
    }

    return 0;
}