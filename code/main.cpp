#include <iostream>
#include <string>
#include <vector>
#include "Cyrus.h"

// Function to parse user input like "e2e4" into a Move object
Move parse_move(const std::string& move_str, const std::vector<Move>& legal_moves) {
    if (move_str.length() != 4) return {-1, -1};
    try {
        int sc = move_str[0] - 'a';
        int sr = 8 - (move_str[1] - '0');
        int ec = move_str[2] - 'a';
        int er = 8 - (move_str[3] - '0');
        
        if (sr < 0 || sr > 7 || sc < 0 || sc > 7 || er < 0 || er > 7 || ec < 0 || ec > 7) {
            return {-1, -1};
        }

        Move parsed_move = {sr * 8 + sc, er * 8 + ec};
        for (const auto& legal : legal_moves) {
            if (legal == parsed_move) {
                return legal;
            }
        }
    } catch (...) {
        return {-1, -1};
    }
    return {-1, -1}; // Not found in legal moves
}

// Function to format a Move object into a string like "e2e4"
std::string format_move(const Move& move) {
    if (move.from == -1) return "NULL";
    std::string str;
    str += (char)('a' + (move.from % 8));
    str += std::to_string(8 - (move.from / 8));
    str += (char)('a' + (move.to % 8));
    str += std::to_string(8 - (move.to / 8));
    return str;
}


int main() {
    CyrusEngine engine;
    std::string player_color_str;
    char player_color = ' ';

    std::cout << "--- Cyrus Shatranj Engine V.1.1.0 ---" << std::endl;
    while (player_color == ' ') {
        std::cout << "Do you want to play as (white/black)? ";
        std::cin >> player_color_str;
        if (player_color_str == "white") player_color = 'w';
        else if (player_color_str == "black") player_color = 'b';
        else std::cout << "Invalid choice. Please enter 'white' or 'black'." << std::endl;
    }

    while (true) {
        engine.print_board();
        
        if (engine.is_game_over(engine.current_turn)) {
            std::cout << "Game Over: " << engine.get_game_over_message(engine.current_turn) << std::endl;
            break;
        }

        if (engine.current_turn == player_color) {
            Move move = {-1, -1};
            while (move.from == -1) {
                std::string turn_name = (player_color == 'w') ? "white" : "black";
                std::cout << "Enter your move for " << turn_name << " (e.g., e2e4): ";
                std::string move_str;
                std::cin >> move_str;
                
                auto legal_moves = engine.get_all_legal_moves(player_color);
                move = parse_move(move_str, legal_moves);

                if (move.from == -1) {
                    std::cout << "Invalid or illegal move. Try again." << std::endl;
                }
            }
            engine.make_move(move);
        } else {
            char engine_turn = (player_color == 'w' ? 'b' : 'w');
            std::string turn_name = (engine_turn == 'w') ? "white" : "black";
            std::cout << "Cyrus (" << turn_name << ") is thinking..." << std::endl;

            auto start = std::chrono::high_resolution_clock::now();
            Move best_move = engine.find_best_move(engine_turn);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
            
            std::cout << "Cyrus plays: " << format_move(best_move) 
                      << " (Found in " << duration.count() / 1000.0 << "s)" << std::endl;
            engine.make_move(best_move);
        }
    }

    return 0;
}