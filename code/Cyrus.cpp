#include "Cyrus.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <stdexcept>

CyrusEngine::CyrusEngine() {
    board = {
        {'r', 'n', 'b', 'k', 'q', 'b', 'n', 'r'},
        {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'.', '.', '.', '.', '.', '.', '.', '.'},
        {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
        {'R', 'N', 'B', 'K', 'Q', 'B', 'N', 'R'}
    };
    current_turn = 'w';

    piece_values = {{'p', 100}, {'n', 320}, {'b', 280}, {'r', 500}, {'q', 105}, {'k', 20000}};

    pst['p'] = {{0,0,0,0,0,0,0,0},{5,10,10,-20,-20,10,10,5},{5,-5,-10,0,0,-10,-5,5},{0,0,0,20,20,0,0,0},{5,5,10,25,25,10,5,5},{10,10,20,30,30,20,10,10},{50,50,50,50,50,50,50,50},{0,0,0,0,0,0,0,0}};
    pst['n'] = {{-50,-40,-30,-30,-30,-30,-40,-50},{-40,-20,0,5,5,0,-20,-40},{-30,5,10,15,15,10,5,-30},{-30,0,15,20,20,15,0,-30},{-30,5,15,20,20,15,5,-30},{-30,0,10,15,15,10,0,-30},{-40,-20,0,0,0,0,-20,-40},{-50,-40,-30,-30,-30,-30,-40,-50}};
    pst['b'] = {{-10,-10,-10,-10,-10,-10,-10,-10},{-10,0,0,0,0,0,0,-10},{-10,0,5,10,10,5,0,-10},{-10,5,5,10,10,5,5,-10},{-10,0,10,10,10,10,0,-10},{-10,10,10,10,10,10,10,-10},{-10,5,0,0,0,0,5,-10},{-10,-10,-10,-10,-10,-10,-10,-10}};
    pst['r'] = {{0,0,0,5,5,0,0,0},{-5,0,0,0,0,0,0,-5},{-5,0,0,0,0,0,0,-5},{-5,0,0,0,0,0,0,-5},{-5,0,0,0,0,0,0,-5},{-5,0,0,0,0,0,0,-5},{5,10,10,10,10,10,10,5},{0,0,0,0,0,0,0,0}};
    pst['q'] = {{-10,-10,-10,-5,-5,-10,-10,-10},{-10,0,0,0,0,0,0,-10},{-10,0,5,5,5,5,0,-10},{-5,0,5,5,5,5,0,-5},{0,0,5,5,5,5,0,-5},{-10,5,5,5,5,5,0,-10},{-10,0,5,0,0,0,0,-10},{-10,-10,-10,-5,-5,-10,-10,-10}};
    pst['k'] = {{20,30,10,0,0,10,30,20},{20,20,0,0,0,0,20,20},{-10,-20,-20,-20,-20,-20,-20,-10},{-20,-30,-30,-40,-40,-30,-30,-20},{-30,-40,-40,-50,-50,-40,-40,-30},{-30,-40,-40,-50,-50,-40,-40,-30},{-30,-40,-40,-50,-50,-40,-40,-30},{-30,-40,-40,-50,-50,-40,-40,-30}};
    
    init_zobrist();
    current_hash = compute_zobrist_hash();
}

void CyrusEngine::init_zobrist() {
    std::mt19937_64 engine(0); // Fixed seed for reproducibility
    std::uniform_int_distribution<uint64_t> dist;
    piece_keys.resize(12, std::vector<uint64_t>(64));
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 64; ++j) {
            piece_keys[i][j] = dist(engine);
        }
    }
    turn_key = dist(engine);
}

uint64_t CyrusEngine::compute_zobrist_hash() const {
    uint64_t h = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char piece = board[r][c];
            if (piece != '.') {
                h ^= piece_keys[piece_map(piece)][r * 8 + c];
            }
        }
    }
    if (current_turn == 'w') {
        h ^= turn_key;
    }
    return h;
}

int CyrusEngine::piece_map(char p) const {
    switch(p) {
        case 'p': return 0; case 'P': return 1;
        case 'n': return 2; case 'N': return 3;
        case 'b': return 4; case 'B': return 5;
        case 'r': return 6; case 'R': return 7;
        case 'q': return 8; case 'Q': return 9;
        case 'k': return 10; case 'K': return 11;
        default: return -1;
    }
}

void CyrusEngine::print_board() const {
    std::cout << "\n  a b c d e f g h" << std::endl;
    std::cout << " +-----------------+" << std::endl;
    for (int i = 0; i < 8; ++i) {
        std::cout << 8 - i << "| ";
        for (int j = 0; j < 8; ++j) {
            std::cout << board[i][j] << " ";
        }
        std::cout << "|" << 8 - i << std::endl;
    }
    std::cout << " +-----------------+" << std::endl;
    std::cout << "  a b c d e f g h\n" << std::endl;
}

char CyrusEngine::get_piece_color(char p) const {
    if (p == '.') return ' ';
    return isupper(p) ? 'w' : 'b';
}

Move CyrusEngine::find_best_move(char turn) {
    transposition_table.clear();
    auto legal_moves = get_all_legal_moves(turn, true);
    if (legal_moves.empty()) {
        return {-1, -1};
    }

    Move best_move = legal_moves[0];
    int best_eval;

    if (turn == 'w') {
        best_eval = -999999;
        int alpha = -999999;
        int beta = 999999;
        for (const auto& move : legal_moves) {
            char piece = board[move.from / 8][move.from % 8];
            char captured = board[move.to / 8][move.to % 8];
            make_move(move);
            int eval = minimax(SEARCH_DEPTH - 1, alpha, beta, false);
            unmake_move(move, piece, captured);
            if (eval > best_eval) {
                best_eval = eval;
                best_move = move;
            }
            alpha = std::max(alpha, eval);
        }
    } else { // Black's turn
        best_eval = 999999;
        int alpha = -999999;
        int beta = 999999;
        for (const auto& move : legal_moves) {
            char piece = board[move.from / 8][move.from % 8];
            char captured = board[move.to / 8][move.to % 8];
            make_move(move);
            int eval = minimax(SEARCH_DEPTH - 1, alpha, beta, true);
            unmake_move(move, piece, captured);
            if (eval < best_eval) {
                best_eval = eval;
                best_move = move;
            }
            beta = std::min(beta, eval);
        }
    }
    return best_move;
}

int CyrusEngine::minimax(int depth, int alpha, int beta, bool maximizing_player) {
    uint64_t hash_key = current_hash;
    if (transposition_table.count(hash_key) && transposition_table[hash_key].depth >= depth) {
        TT_Entry entry = transposition_table[hash_key];
        if (entry.flag == TT_EXACT) return entry.score;
        if (entry.flag == TT_LOWER) alpha = std::max(alpha, entry.score);
        if (entry.flag == TT_UPPER) beta = std::min(beta, entry.score);
        if (alpha >= beta) return entry.score;
    }

    if (depth == 0) {
        return quiescence_search(alpha, beta, maximizing_player);
    }

    char turn = maximizing_player ? 'w' : 'b';
    auto legal_moves = get_all_legal_moves(turn, true);

    if (legal_moves.empty()) {
        return is_in_check(turn) ? (maximizing_player ? -99999 : 99999) : 0;
    }
    
    int original_alpha = alpha;
    int original_beta = beta;

    if (maximizing_player) {
        int max_eval = -999999;
        for (const auto& move : legal_moves) {
            char piece = board[move.from / 8][move.from % 8];
            char captured = board[move.to / 8][move.to % 8];
            make_move(move);
            int eval = minimax(depth - 1, alpha, beta, false);
            unmake_move(move, piece, captured);
            max_eval = std::max(max_eval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        TT_Entry entry = {depth, max_eval, TT_EXACT};
        if (max_eval <= original_alpha) entry.flag = TT_UPPER;
        else if (max_eval >= beta) entry.flag = TT_LOWER;
        transposition_table[hash_key] = entry;
        return max_eval;
    } else {
        int min_eval = 999999;
        for (const auto& move : legal_moves) {
            char piece = board[move.from / 8][move.from % 8];
            char captured = board[move.to / 8][move.to % 8];
            make_move(move);
            int eval = minimax(depth - 1, alpha, beta, true);
            unmake_move(move, piece, captured);
            min_eval = std::min(min_eval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        TT_Entry entry = {depth, min_eval, TT_EXACT};
        if (min_eval <= alpha) entry.flag = TT_UPPER;
        else if (min_eval >= original_beta) entry.flag = TT_LOWER;
        transposition_table[hash_key] = entry;
        return min_eval;
    }
}

int CyrusEngine::quiescence_search(int alpha, int beta, bool maximizing_player) {
    int stand_pat = evaluate_board();

    if (maximizing_player) {
        alpha = std::max(alpha, stand_pat);
        if (alpha >= beta) return beta;
    } else {
        beta = std::min(beta, stand_pat);
        if (alpha >= beta) return alpha;
    }

    char turn = maximizing_player ? 'w' : 'b';
    auto captures = _generate_pseudo_legal_moves(turn, true);

    for (const auto& move : captures) {
        char piece = board[move.from / 8][move.from % 8];
        char captured = board[move.to / 8][move.to % 8];
        make_move(move);
        if (!is_in_check(turn)) { // only consider legal captures
            int score = quiescence_search(alpha, beta, !maximizing_player);
            if (maximizing_player) {
                alpha = std::max(alpha, score);
                if (alpha >= beta) {
                    unmake_move(move, piece, captured);
                    return beta;
                }
            } else {
                beta = std::min(beta, score);
                if (alpha >= beta) {
                    unmake_move(move, piece, captured);
                    return alpha;
                }
            }
        }
        unmake_move(move, piece, captured);
    }
    return maximizing_player ? alpha : beta;
}


void CyrusEngine::make_move(const Move& move) {
    int sr = move.from / 8, sc = move.from % 8;
    int er = move.to / 8, ec = move.to % 8;
    char piece = board[sr][sc];
    char target = board[er][ec];

    // Update hash: xor out pieces from their squares
    current_hash ^= piece_keys[piece_map(piece)][sr * 8 + sc];
    if (target != '.') {
        current_hash ^= piece_keys[piece_map(target)][er * 8 + ec];
    }
    
    board[sr][sc] = '.';
    // Promotion (to Ferz/Counselor)
    if (tolower(piece) == 'p' && (er == 0 || er == 7)) {
        char promo_piece = (current_turn == 'w') ? 'Q' : 'q';
        board[er][ec] = promo_piece;
        current_hash ^= piece_keys[piece_map(promo_piece)][er * 8 + ec];
    } else {
        board[er][ec] = piece;
        current_hash ^= piece_keys[piece_map(piece)][er * 8 + ec];
    }
    
    current_hash ^= turn_key;
    current_turn = (current_turn == 'w') ? 'b' : 'w';
}

void CyrusEngine::unmake_move(const Move& move, char piece, char captured_piece) {
    int sr = move.from / 8, sc = move.from % 8;
    int er = move.to / 8, ec = move.to % 8;

    current_turn = (current_turn == 'w') ? 'b' : 'w';
    current_hash ^= turn_key;
    
    char moved_piece = board[er][ec]; // Might be a promoted piece
    
    board[sr][sc] = piece; // Restore the original piece
    board[er][ec] = captured_piece;
    
    // Reverse hash updates
    current_hash ^= piece_keys[piece_map(piece)][sr * 8 + sc];
    current_hash ^= piece_keys[piece_map(moved_piece)][er * 8 + ec];
    if (captured_piece != '.') {
        current_hash ^= piece_keys[piece_map(captured_piece)][er * 8 + ec];
    }
}


int CyrusEngine::evaluate_board() const {
    int score = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char piece = board[r][c];
            if (piece != '.') {
                char p_type = tolower(piece);
                int val = piece_values.at(p_type);
                int pst_val = (get_piece_color(piece) == 'w') ? pst.at(p_type)[r][c] : pst.at(p_type)[7 - r][c];
                score += (get_piece_color(piece) == 'w') ? (val + pst_val) : -(val + pst_val);
            }
        }
    }
    return score;
}

std::vector<Move> CyrusEngine::get_all_legal_moves(char turn, bool sort) {
    auto pseudo_moves = _generate_pseudo_legal_moves(turn, false);
    std::vector<Move> legal_moves;
    for (const auto& move : pseudo_moves) {
        char piece = board[move.from / 8][move.from % 8];
        char captured = board[move.to / 8][move.to % 8];
        make_move(move);
        // We check the color that just moved
        if (!is_in_check( (turn == 'w' ? 'b' : 'w') )) {
            legal_moves.push_back(move);
        }
        unmake_move(move, piece, captured);
    }
    if (sort) {
        std::sort(legal_moves.begin(), legal_moves.end(), [this](const Move& a, const Move& b) {
            return _score_move(a) > _score_move(b);
        });
    }
    return legal_moves;
}


int CyrusEngine::_score_move(const Move& move) const {
    char target = board[move.to / 8][move.to % 8];
    if (target != '.') {
        char attacker = board[move.from/8][move.from%8];
        // MVV-LVA (Most Valuable Victim - Least Valuable Aggressor)
        return 10 * piece_values.at(tolower(target)) - piece_values.at(tolower(attacker));
    }
    return 0;
}

std::vector<Move> CyrusEngine::_generate_pseudo_legal_moves(char color, bool captures_only) const {
    std::vector<Move> moves;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (board[r][c] != '.' && get_piece_color(board[r][c]) == color) {
                _get_moves_for_piece(r, c, moves);
            }
        }
    }
    if (captures_only) {
        std::vector<Move> capture_moves;
        for (const auto& move : moves) {
            if (board[move.to / 8][move.to % 8] != '.') {
                capture_moves.push_back(move);
            }
        }
        return capture_moves;
    }
    return moves;
}

void CyrusEngine::_get_moves_for_piece(int r, int c, std::vector<Move>& moves) const {
    char piece = tolower(board[r][c]);
    char color = get_piece_color(board[r][c]);
    switch (piece) {
        case 'p': _get_pawn_moves(r, c, color, moves); break;
        case 'n': _get_faras_moves(r, c, color, moves); break;
        case 'b': _get_fil_moves(r, c, color, moves); break;
        case 'r': _get_sliding_moves(r, c, color, {{1,0},{-1,0},{0,1},{0,-1}}, moves); break;
        case 'q': _get_ferz_moves(r, c, color, moves); break;
        case 'k': _get_shah_moves(r, c, color, moves); break;
    }
}

void CyrusEngine::_get_pawn_moves(int r, int c, char color, std::vector<Move>& moves) const {
    int dir = (color == 'w') ? -1 : 1;
    if (r + dir >= 0 && r + dir < 8) {
        if (board[r + dir][c] == '.') {
            moves.push_back({r * 8 + c, (r + dir) * 8 + c});
        }
        for (int dc : {-1, 1}) {
            if (c + dc >= 0 && c + dc < 8 && board[r + dir][c + dc] != '.' && get_piece_color(board[r + dir][c + dc]) != color) {
                moves.push_back({r * 8 + c, (r + dir) * 8 + (c + dc)});
            }
        }
    }
}

void CyrusEngine::_get_faras_moves(int r, int c, char color, std::vector<Move>& moves) const { // Knight
    std::vector<std::pair<int, int>> deltas = {{1,2},{1,-2},{-1,2},{-1,-2},{2,1},{2,-1},{-2,1},{-2,-1}};
    for (auto d : deltas) {
        int nr = r + d.first, nc = c + d.second;
        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8 && get_piece_color(board[nr][nc]) != color) {
            moves.push_back({r * 8 + c, nr * 8 + nc});
        }
    }
}

void CyrusEngine::_get_fil_moves(int r, int c, char color, std::vector<Move>& moves) const { // Elephant
    std::vector<std::pair<int, int>> deltas = {{2,2},{2,-2},{-2,2},{-2,-2}};
    for (auto d : deltas) {
        int nr = r + d.first, nc = c + d.second;
        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8 && get_piece_color(board[nr][nc]) != color) {
            moves.push_back({r * 8 + c, nr * 8 + nc});
        }
    }
}

void CyrusEngine::_get_ferz_moves(int r, int c, char color, std::vector<Move>& moves) const { // Counselor
    std::vector<std::pair<int, int>> deltas = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (auto d : deltas) {
        int nr = r + d.first, nc = c + d.second;
        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8 && get_piece_color(board[nr][nc]) != color) {
            moves.push_back({r * 8 + c, nr * 8 + nc});
        }
    }
}

void CyrusEngine::_get_shah_moves(int r, int c, char color, std::vector<Move>& moves) const { // King
    std::vector<std::pair<int, int>> deltas = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
    for (auto d : deltas) {
        int nr = r + d.first, nc = c + d.second;
        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8 && get_piece_color(board[nr][nc]) != color) {
            moves.push_back({r * 8 + c, nr * 8 + nc});
        }
    }
}

void CyrusEngine::_get_sliding_moves(int r, int c, char color, const std::vector<std::pair<int, int>>& deltas, std::vector<Move>& moves) const {
    for (auto d : deltas) {
        int nr = r + d.first, nc = c + d.second;
        while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
            if (board[nr][nc] == '.') {
                moves.push_back({r * 8 + c, nr * 8 + nc});
            } else {
                if (get_piece_color(board[nr][nc]) != color) {
                    moves.push_back({r * 8 + c, nr * 8 + nc});
                }
                break;
            }
            nr += d.first; nc += d.second;
        }
    }
}

bool CyrusEngine::is_in_check(char color) const {
    auto king_pos = find_king(color);
    if (king_pos.first == -1) return true; // King not found, which is a game-ending state

    char opponent = (color == 'w') ? 'b' : 'w';
    auto opponent_moves = _generate_pseudo_legal_moves(opponent, false);
    
    int king_square = king_pos.first * 8 + king_pos.second;
    for (const auto& move : opponent_moves) {
        if (move.to == king_square) {
            return true;
        }
    }
    return false;
}

std::pair<int, int> CyrusEngine::find_king(char color) const {
    char king_char = (color == 'w') ? 'K' : 'k';
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (board[r][c] == king_char) {
                return {r, c};
            }
        }
    }
    return {-1, -1}; // Should not happen in a normal game
}

bool CyrusEngine::is_game_over(char turn) {
    return get_all_legal_moves(turn, false).empty();
}

std::string CyrusEngine::get_game_over_message(char turn) {
    std::string winner = (turn == 'w') ? "Black" : "White";
    if (is_in_check(turn)) {
        return "Checkmate! " + winner + " wins.";
    } else {
        return "Stalemate! " + winner + " wins.";
    }
}