#ifndef CYRUS_H
#define CYRUS_H

#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>

// Type aliases for clarity
using Bitboard = uint64_t;

enum Color { WHITE, BLACK };

struct Move {
    int from;
    int to;
    // Add flags for promotion, capture etc. if needed
};

class Board {
public:
    Bitboard pieceBitboards[12]; // p, P, n, N, ...
    Bitboard occupancyBitboards[3]; // WHITE, BLACK, BOTH
    Color sideToMove;

    Board();
    void printBoard();
    void makeMove(const Move& move);
    // ... other board-related functions
};

class Engine {
public:
    Engine();
    Move findBestMove(Board& board);

private:
    // This is where you'd implement your search, evaluation, etc.
    int evaluate(const Board& board);
    int search(Board& board, int depth, int alpha, int beta, bool maximizingPlayer);
    std::vector<Move> generateMoves(const Board& board);

    // Simplified move generation for demonstration
    void generatePawnMoves(const Board& board, std::vector<Move>& moves);
    void generateKnightMoves(const Board& board, std::vector<Move>& moves);
};

#endif // CYRUS_H