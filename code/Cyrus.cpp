#include "Cyrus.h"
#include <iostream>
#include <algorithm>

// --- Constants and Helpers ---
const int PAWN = 0, KNIGHT = 2, BISHOP = 4, ROOK = 6, QUEEN = 8, KING = 10;
const int WHITE_P = 1, BLACK_P = 0; // etc. for all pieces

// --- Board Implementation ---
Board::Board() {
    // Initialize bitboards to 0
    for(int i = 0; i < 12; ++i) pieceBitboards[i] = 0ULL;
    for(int i = 0; i < 3; ++i) occupancyBitboards[i] = 0ULL;

    // Setup initial position (FEN string: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w)
    // This is a simplified setup. A full FEN parser would be better.
    pieceBitboards[ROOK] = (1ULL << 0) | (1ULL << 7);
    pieceBitboards[KNIGHT] = (1ULL << 1) | (1ULL << 6);
    pieceBitboards[BISHOP] = (1ULL << 2) | (1ULL << 5);
    pieceBitboards[QUEEN] = (1ULL << 3);
    pieceBitboards[KING] = (1ULL << 4);
    pieceBitboards[PAWN] = 0xFFULL << 8;

    pieceBitboards[ROOK+1] = (1ULL << 56) | (1ULL << 63);
    pieceBitboards[KNIGHT+1] = (1ULL << 57) | (1ULL << 62);
    pieceBitboards[BISHOP+1] = (1ULL << 58) | (1ULL << 61);
    pieceBitboards[QUEEN+1] = (1ULL << 59);
    pieceBitboards[KING+1] = (1ULL << 60);
    pieceBitboards[PAWN+1] = 0xFFULL << 48;

    // Setup occupancy bitboards
    occupancyBitboards[WHITE] = pieceBitboards[PAWN+1] | pieceBitboards[ROOK+1] | pieceBitboards[KNIGHT+1] | pieceBitboards[BISHOP+1] | pieceBitboards[QUEEN+1] | pieceBitboards[KING+1];
    occupancyBitboards[BLACK] = pieceBitboards[PAWN] | pieceBitboards[ROOK] | pieceBitboards[KNIGHT] | pieceBitboards[BISHOP] | pieceBitboards[QUEEN] | pieceBitboards[KING];
    occupancyBitboards[2] = occupancyBitboards[WHITE] | occupancyBitboards[BLACK];

    sideToMove = WHITE;
}

void Board::printBoard() {
    std::cout << "\n  a b c d e f g h\n";
    std::cout << " +-----------------+\n";
    for (int r = 7; r >= 0; --r) {
        std::cout << r + 1 << "| ";
        for (int f = 0; f < 8; ++f) {
            int sq = r * 8 + f;
            char pieceChar = '.';
            for(int piece = 0; piece < 12; ++piece) {
                if ((pieceBitboards[piece] >> sq) & 1) {
                    // This is a simplified mapping
                    const char* pieceChars = "pnbrqkPNBRQK";
                    pieceChar = pieceChars[piece];
                    break;
                }
            }
            std::cout << pieceChar << " ";
        }
        std::cout << "|" << r + 1 << "\n";
    }
    std::cout << " +-----------------+\n";
    std::cout << "  a b c d e f g h\n\n";
}

// A very simplified makeMove for demonstration
void Board::makeMove(const Move& move) {
    // This needs to be fully implemented: move piece, handle captures, update occupancy
    // ...
    sideToMove = (sideToMove == WHITE) ? BLACK : WHITE;
}


// --- Engine Implementation ---
Engine::Engine() {}

Move Engine::findBestMove(Board& board) {
    // A simplified search for demonstration
    auto moves = generateMoves(board);
    if (moves.empty()) {
        return {-1, -1}; // No legal moves
    }
    // For now, just return a random legal move
    return moves[rand() % moves.size()];
}

int Engine::evaluate(const Board& board) {
    // A very simple evaluation: material count
    int score = 0;
    score += __builtin_popcountll(board.pieceBitboards[PAWN+1]) * 100;
    score -= __builtin_popcountll(board.pieceBitboards[PAWN]) * 100;
    score += __builtin_popcountll(board.pieceBitboards[KNIGHT+1]) * 320;
    score -= __builtin_popcountll(board.pieceBitboards[KNIGHT]) * 320;
    // ... add for all pieces
    return (board.sideToMove == WHITE) ? score : -score;
}


// This is a placeholder for a full search algorithm
int Engine::search(Board& board, int depth, int alpha, int beta, bool maximizingPlayer) {
    if (depth == 0) {
        return evaluate(board);
    }
    // ... Minimax with alpha-beta would go here
    return 0;
}

// Generate moves (simplified example)
std::vector<Move> Engine::generateMoves(const Board& board) {
    std::vector<Move> moves;
    generatePawnMoves(board, moves);
    generateKnightMoves(board, moves);
    // ... generate moves for other pieces
    return moves;
}

void Engine::generatePawnMoves(const Board& board, std::vector<Move>& moves) {
    Bitboard pawns = board.pieceBitboards[board.sideToMove == WHITE ? PAWN+1 : PAWN];
    while(pawns) {
        int from = __builtin_ctzll(pawns);
        // Simplified: just push forward one square
        int to = (board.sideToMove == WHITE) ? from + 8 : from - 8;
        if (to >= 0 && to < 64) {
             // Check if 'to' square is empty
             if (!((board.occupancyBitboards[2] >> to) & 1)) {
                moves.push_back({from, to});
             }
        }
        pawns &= pawns - 1; // Clear least significant bit
    }
}

void Engine::generateKnightMoves(const Board& board, std::vector<Move>& moves) {
    // Implementation would be similar, using pre-calculated attack maps for knights
}