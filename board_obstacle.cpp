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
    const int maxEnemies = 2;

    // ระบบกำหนดเวลา (Cooldown)
    int turnsSinceLastSpawn = 0;
    int turnsSinceLastMove = 0;
    const int SPAWN_COOLDOWN = 6; // รอถึงจะสุ่มเกิด
    const int MOVE_COOLDOWN = 3;  // ขยับ 1 ครั้ง ทุกๆกี่ตา

    CheckersBoard() {
        board.resize(TOTAL_TILES, EMPTY);
        std::srand(static_cast<unsigned int>(std::time(0)));
        initializeBoard();
    }

    void initializeBoard() {
        for (int i = 0; i < 40; i++) {
            int r = i / 10, c = i % 10;
            if ((r + c) % 2 != 0) board[i] = P1;
        }
        for (int i = 60; i < 100; i++) {
            int r = i / 10, c = i % 10;
            if ((r + c) % 2 != 0) board[i] = P2;
        }
    }

    int countEnemies() {
        int count = 0;
        for (int p : board) if (p == OBSTACLE) count++;
        return count;
    }

    // สุ่มเกิดเฉพาะช่องสีเข้มในโซนกลาง และห้ามทับใคร
    void spawnEnemy() {
        turnsSinceLastSpawn++;
        if (turnsSinceLastSpawn >= SPAWN_COOLDOWN && countEnemies() < maxEnemies) {
            if (std::rand() % 100 < 40) { // โอกาส 40% เมื่อถึงกำหนด
                std::vector<int> targetSlots;
                for (int i = 30; i < 70; ++i) { // แถว 4-7
                    int r = i / 10, c = i % 10;
                    if ((r + c) % 2 != 0 && board[i] == EMPTY) {
                        targetSlots.push_back(i);
                    }
                }
                if (!targetSlots.empty()) {
                    board[targetSlots[std::rand() % targetSlots.size()]] = OBSTACLE;
                    turnsSinceLastSpawn = 0;
                    std::cout << "Neutral Enemy spawned on Dark Tile!" << std::endl;
                }
            }
        }
    }

    // เดินเฉพาะช่องสีเข้ม (ทะแยง) เพื่อไล่ล่าหมากที่ใกล้ที่สุด
    void moveEnemies() {
        turnsSinceLastMove++;
        if (turnsSinceLastMove >= MOVE_COOLDOWN) {
            std::vector<int> currentEnemies;
            for (int i = 0; i < TOTAL_TILES; ++i) if (board[i] == OBSTACLE) currentEnemies.push_back(i);

            for (int currIdx : currentEnemies) {
                int er = currIdx / 10, ec = currIdx % 10;
                int targetIdx = -1; float minDist = 999.0f;

                // ค้นหาเป้าหมาย
                for (int j = 0; j < TOTAL_TILES; ++j) {
                    if (board[j] >= 1 && board[j] <= 4) {
                        float d = std::sqrt(std::pow((j / 10) - er, 2) + std::pow((j % 10) - ec, 2));
                        if (d < minDist) { minDist = d; targetIdx = j; }
                    }
                }

                if (targetIdx != -1) {
                    int tr = targetIdx / 10, tc = targetIdx % 10;
                    // กำหนดทิศทางแบบทะแยงเสมอเพื่อให้ลงช่องสีเข้ม (Step 1,1)
                    int dr = (tr > er) ? 1 : -1;
                    int dc = (tc > ec) ? 1 : -1;

                    int nr = er + dr, nc = ec + dc;
                    if (nr >= 0 && nr < 10 && nc >= 0 && nc < 10) {
                        int nIdx = nr * 10 + nc;
                        // เดินไปทับ = กิน / เดินไปที่ว่าง = ย้ายที่
                        board[currIdx] = EMPTY;
                        board[nIdx] = OBSTACLE;
                    }
                }
            }
            turnsSinceLastMove = 0;
        }
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 800), "Checkers - Obstacle Survival Mode");
    CheckersBoard game;
    int selectedIndex = -1;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int col = event.mouseButton.x / TILE_SIZE;
                int row = event.mouseButton.y / TILE_SIZE;
                if (col >= 0 && col < 10 && row >= 0 && row < 10) {
                    int clickedIndex = row * BOARD_SIZE + col;

                    if (game.board[clickedIndex] == P1 || game.board[clickedIndex] == P2) {
                        selectedIndex = clickedIndex;
                    }
                    else if (selectedIndex != -1 && game.board[clickedIndex] == EMPTY) {
                        int fr = selectedIndex / 10, fc = selectedIndex % 10, tr = clickedIndex / 10, tc = clickedIndex % 10;
                        int rd = tr - fr, cd = tc - fc;
                        int piece = game.board[selectedIndex];
                        bool valid = false; int cap = -1;

                        // เดินปกติ
                        if (std::abs(rd) == 1 && std::abs(cd) == 1) {
                            if (piece == P1 && rd == 1) valid = true;
                            if (piece == P2 && rd == -1) valid = true;
                        }
                        // กระโดดกิน (หมากผู้เล่น หรือ ศัตรูสีม่วง)
                        else if (std::abs(rd) == 2 && std::abs(cd) == 2) {
                            int mid = (fr + rd / 2) * 10 + (fc + cd / 2);
                            if (piece == P1 && (game.board[mid] == P2 || game.board[mid] == OBSTACLE)) { valid = true; cap = mid; }
                            else if (piece == P2 && (game.board[mid] == P1 || game.board[mid] == OBSTACLE)) { valid = true; cap = mid; }
                        }

                        if (valid) {
                            game.board[clickedIndex] = piece;
                            game.board[selectedIndex] = EMPTY;
                            if (cap != -1) game.board[cap] = EMPTY;

                            // อัปเดตระบบศัตรูหลังจบตา
                            game.spawnEnemy();
                            game.moveEnemies();

                            selectedIndex = -1;
                        }
                        else { selectedIndex = -1; }
                    }
                }
            }
        }

        window.clear();
        for (int i = 0; i < TOTAL_TILES; ++i) {
            int r = i / 10, c = i % 10;
            sf::RectangleShape tile(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            tile.setPosition(c * TILE_SIZE, r * TILE_SIZE);
            tile.setFillColor(((r + c) % 2 != 0) ? sf::Color(139, 69, 19) : sf::Color(245, 222, 179));
            window.draw(tile);

            if (game.board[i] != EMPTY) {
                sf::CircleShape p(TILE_SIZE / 2.0f - 8.0f);
                p.setPosition(c * TILE_SIZE + 8.0f, r * TILE_SIZE + 8.0f);

                if (game.board[i] == P1) p.setFillColor(sf::Color::Red);
                else if (game.board[i] == P2) p.setFillColor(sf::Color(0, 153, 76));
                else if (game.board[i] == OBSTACLE) {
                    p.setFillColor(sf::Color::Magenta); // ศัตรูสีม่วง
                    p.setOutlineThickness(3.0f);
                    p.setOutlineColor(sf::Color::Cyan);
                }

                if (i == selectedIndex) {
                    p.setOutlineThickness(4.0f);
                    p.setOutlineColor(sf::Color::Yellow);
                }
                window.draw(p);
            }
        }
        window.display();
    }
    return 0;
}
