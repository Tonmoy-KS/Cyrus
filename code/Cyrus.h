#ifndef CYRUS_H
#define CYRUS_H

#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <chrono>

// Represents a single move (from square 0-63, to square 0-63)
struct Move {
    int from;
    int to;
    char promotion = ' '; // For potential future use, not in Shatranj

    bool operator==(const Move& other) const {
        return from == other.from && to == other.to;
    }
};

// For the transposition table
enum TT_Flag { TT_EXACT, TT_LOWER, TT_UPPER };
struct TT_Entry {
    int depth;
    int score;
    TT_Flag flag;
};

class CyrusEngine {
public:
    CyrusEngine();
    void print_board() const;
    Move find_best_move(char turn);
    void make_move(const Move& move);
    std::vector<Move> get_all_legal_moves(char turn, bool sort = false);
    bool is_in_check(char color) const;
    bool is_game_over(char turn);
    std::string get_game_over_message(char turn);

    // Board representation and turn
    std::vector<std::vector<char>> board;
    char current_turn = 'w';

private:
    // --- Internal Logic ---
    int minimax(int depth, int alpha, int beta, bool maximizing_player);
    int quiescence_search(int alpha, int beta, bool maximizing_player);
    int evaluate_board() const;
    void unmake_move(const Move& move, char piece, char captured_piece);

    // --- Move Generation ---
    std::vector<Move> _generate_pseudo_legal_moves(char color, bool captures_only) const;
    void _get_moves_for_piece(int r, int c, std::vector<Move>& moves) const;
    void _get_pawn_moves(int r, int c, char color, std::vector<Move>& moves) const;
    void _get_faras_moves(int r, int c, char color, std::vector<Move>& moves) const; // Knight
    void _get_fil_moves(int r, int c, char color, std::vector<Move>& moves) const;   // Elephant
    void _get_ferz_moves(int r, int c, char color, std::vector<Move>& moves) const;  // Counselor
    void _get_shah_moves(int r, int c, char color, std::vector<Move>& moves) const;  // King
    void _get_sliding_moves(int r, int c, char color, const std::vector<std::pair<int, int>>& deltas, std::vector<Move>& moves) const;

    // --- Helpers ---
    char get_piece_color(char p) const;
    std::pair<int, int> find_king(char color) const;
    int _score_move(const Move& move) const;

    // --- AI Configuration ---
    static const int SEARCH_DEPTH = 4;
    std::chrono::steady_clock::time_point search_start_time;

    // --- Zobrist Hashing & Transposition Table ---
    void init_zobrist();
    uint64_t compute_zobrist_hash() const;
    uint64_t current_hash;
    std::vector<std::vector<uint64_t>> piece_keys;
    uint64_t turn_key;
    std::unordered_map<uint64_t, TT_Entry> transposition_table;
    int piece_map(char p) const;

    // --- Evaluation Data ---
    std::unordered_map<char, int> piece_values;
    std::unordered_map<char, std::vector<std::vector<int>>> pst;
};

#endif // CYRUS_H