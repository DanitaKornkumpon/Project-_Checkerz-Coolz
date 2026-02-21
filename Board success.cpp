#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

// กำหนดขนาดกระดาน 10x10 และขนาดหน้าจอ
const int BOARD_SIZE = 10;
const int TOTAL_TILES = 100;
const float TILE_SIZE = 80.0f; // ขนาดแต่ละช่อง (800 / 10 = 80 พิกเซล)

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
        // วางหมาก P1 (4 แถวบน)
        for (int i = 0; i < 40; i++) {
            int row = i / 10;
            int col = i % 10;
            if ((row % 2 == 0 && col % 2 == 1) || (row % 2 == 1 && col % 2 == 0)) {
                board[i] = P1;
            }
        }
        // วางหมาก P2 (4 แถวล่าง)
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
    // สร้างหน้าต่างขนาด 800x800
    sf::RenderWindow window(sf::VideoMode(800, 800), "Checkers 10x10 Project");
    CheckersBoard game;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // 1. ล้างหน้าจอ
        window.clear();

        // 2. ลูปวาดกระดาน 10x10 และตัวหมาก
        for (int row = 0; row < BOARD_SIZE; ++row) {
            for (int col = 0; col < BOARD_SIZE; ++col) {

                // --- วาดช่องสี่เหลี่ยม (กระดาน) ---
                sf::RectangleShape square(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                square.setPosition(col * TILE_SIZE, row * TILE_SIZE);

                // สลับสีช่องว่าง (สีไม้เข้ม-อ่อน)
                if ((row % 2 == 0 && col % 2 == 1) || (row % 2 == 1 && col % 2 == 0)) {
                    square.setFillColor(sf::Color(139, 69, 19)); // ช่องสีเข้ม
                }
                else {
                    square.setFillColor(sf::Color(245, 222, 179)); // ช่องสีสว่าง
                }
                window.draw(square);

                // --- วาดวงกลม (ตัวหมาก) ---
                int piece = game.board[row * BOARD_SIZE + col];
                if (piece != EMPTY) {
                    // กำหนดรัศมีให้เล็กกว่าช่องเล็กน้อย
                    sf::CircleShape checker(TILE_SIZE / 2.0f - 8.0f);
                    checker.setPosition(col * TILE_SIZE + 8.0f, row * TILE_SIZE + 8.0f);

                    if (piece == P1) {
                        checker.setFillColor(sf::Color::White); // ผู้เล่น 1 หมากสีขาว
                    }
                    else if (piece == P2) {
                        checker.setFillColor(sf::Color(50, 50, 50)); // ผู้เล่น 2 หมากสีดำ
                        // ขอบหมากเพื่อให้เห็นชัดขึ้น
                        checker.setOutlineThickness(2.0f);
                        checker.setOutlineColor(sf::Color::Black);
                    }
                    window.draw(checker);
                }
            }
        }

        // 3. แสดงผลทั้งหมดขึ้นจอ
        window.display();
    }
    return 0;
}
