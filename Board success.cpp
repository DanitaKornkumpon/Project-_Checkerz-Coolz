#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>

const int BOARD_SIZE = 10;
const int TOTAL_TILES = 100;
const float TILE_SIZE = 80.0f;

enum PieceType { EMPTY = 0, P1 = 1, P2 = 2, P1_KING = 3, P2_KING = 4, OBSTACLE = 5 };

class CheckersBoard {
public:
    std::vector<int> board;

    CheckersBoard() {
        board.resize(TOTAL_TILES, EMPTY);
        std::srand(static_cast<unsigned int>(std::time(0)));
        initializeBoard();
    }

    void initializeBoard() {
        for (int i = 0; i < 40; i++) {
            int row = i / 10;
            int col = i % 10;
            if ((row % 2 == 0 && col % 2 == 1) || (row % 2 == 1 && col % 2 == 0)) {
                board[i] = P1;
            }
        }
        for (int i = 60; i < 100; i++) {
            int row = i / 10;
            int col = i % 10;
            if ((row % 2 == 0 && col % 2 == 1) || (row % 2 == 1 && col % 2 == 0)) {
                board[i] = P2;
            }
        }
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 800), "Checkers 10x10 Project");
    CheckersBoard game;
    int selectedIndex = -1;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {

                    // แก้ไข Warning conversion float to int ตรงนี้ครับ
                    int col = static_cast<int>(event.mouseButton.x / TILE_SIZE);
                    int row = static_cast<int>(event.mouseButton.y / TILE_SIZE);
                    int clickedIndex = row * BOARD_SIZE + col;

                    if (game.board[clickedIndex] != EMPTY) {
                        selectedIndex = clickedIndex;
                    }
                    else if (selectedIndex != -1 && game.board[clickedIndex] == EMPTY) {

                        int fromRow = selectedIndex / BOARD_SIZE;
                        int fromCol = selectedIndex % BOARD_SIZE;
                        int toRow = clickedIndex / BOARD_SIZE;
                        int toCol = clickedIndex % BOARD_SIZE;

                        int rowDiff = toRow - fromRow;
                        int colDiff = toCol - fromCol;

                        int movingPiece = game.board[selectedIndex];
                        bool isValidMove = false;
                        int capturedIndex = -1;

                        if (std::abs(rowDiff) == 1 && std::abs(colDiff) == 1) {
                            if (movingPiece == P1 && rowDiff == 1) isValidMove = true;
                            if (movingPiece == P2 && rowDiff == -1) isValidMove = true;
                        }
                        else if (std::abs(rowDiff) == 2 && std::abs(colDiff) == 2) {
                            int midRow = fromRow + (rowDiff / 2);
                            int midCol = fromCol + (colDiff / 2);
                            int midIndex = midRow * BOARD_SIZE + midCol;
                            int midPiece = game.board[midIndex];

                            if (movingPiece == P1 && (midPiece == P2 || midPiece == P2_KING)) {
                                isValidMove = true;
                                capturedIndex = midIndex;
                            }
                            else if (movingPiece == P2 && (midPiece == P1 || midPiece == P1_KING)) {
                                isValidMove = true;
                                capturedIndex = midIndex;
                            }
                        }

                        if (isValidMove) {
                            game.board[clickedIndex] = movingPiece;
                            game.board[selectedIndex] = EMPTY;

                            if (capturedIndex != -1) {
                                game.board[capturedIndex] = EMPTY;
                            }

                            selectedIndex = -1;
                        }
                        else {
                            selectedIndex = -1;
                        }
                    }
                }
            }
        }

        window.clear();

        for (int row = 0; row < BOARD_SIZE; ++row) {
            for (int col = 0; col < BOARD_SIZE; ++col) {
                int currentIndex = row * BOARD_SIZE + col;
                sf::RectangleShape square(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                square.setPosition(col * TILE_SIZE, row * TILE_SIZE);

                if ((row % 2 == 0 && col % 2 == 1) || (row % 2 == 1 && col % 2 == 0)) {
                    square.setFillColor(sf::Color(139, 69, 19));
                }
                else {
                    square.setFillColor(sf::Color(245, 222, 179));
                }
                window.draw(square);

                int piece = game.board[currentIndex];
                if (piece != EMPTY) {
                    sf::CircleShape checker(TILE_SIZE / 2.0f - 8.0f);
                    checker.setPosition(col * TILE_SIZE + 8.0f, row * TILE_SIZE + 8.0f);

                    if (piece == P1) {
                        checker.setFillColor(sf::Color(255, 51, 51));
                        checker.setOutlineThickness(2.0f);
                        checker.setOutlineColor(sf::Color(204, 0, 0));
                    }
                    else if (piece == P2) {
                        checker.setFillColor(sf::Color(0, 153, 76));
                        checker.setOutlineThickness(2.0f);
                        checker.setOutlineColor(sf::Color(0, 255, 128));
                    }

                    if (currentIndex == selectedIndex) {
                        checker.setOutlineThickness(4.0f);
                        checker.setOutlineColor(sf::Color::Yellow);
                    }

                    window.draw(checker);
                }
            }
        }
        window.display();
    }
    return 0; // รับรองว่าวงเล็บปิดครบถ้วนแน่นอนครับ!
}
