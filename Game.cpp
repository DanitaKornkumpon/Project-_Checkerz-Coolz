#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <algorithm>

const int BOARD_SIZE = 10;
const int TOTAL_TILES = 100;
const float TILE_SIZE = 80.0f;

enum PieceType { EMPTY = 0, P1 = 1, P2 = 2, P1_KING = 3, P2_KING = 4, OBSTACLE = 5 };

struct Move {
    int toIndex;
    std::vector<int> capturedIndices;
    std::vector<int> pathIndices;
    int score() const { return capturedIndices.size(); }
};

class CheckersBoard {
public:
    std::vector<int> board;

    const int maxEnemies = 2;
    int turnsSinceLastSpawn = 0;
    int turnsSinceLastMove = 0;
    const int SPAWN_COOLDOWN = 6;
    const int MOVE_COOLDOWN = 3;

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

    void reset() {
        std::fill(board.begin(), board.end(), EMPTY);
        turnsSinceLastSpawn = 0;
        turnsSinceLastMove = 0;
        initializeBoard();
    }

    void countPieces(int& p1Count, int& p2Count) {
        p1Count = 0;
        p2Count = 0;
        for (int piece : board) {
            if (piece == P1 || piece == P1_KING) p1Count++;
            if (piece == P2 || piece == P2_KING) p2Count++;
        }
    }

    void findCaptures(int idx, int p, std::vector<int> currentCaptured, std::vector<int> currentPath, std::vector<Move>& allMoves, std::vector<int>& tempBoard) {
        int r = idx / BOARD_SIZE;
        int c = idx % BOARD_SIZE;
        bool foundAnyCapture = false;

        int dr[] = { -2, -2, 2, 2 };
        int dc[] = { -2, 2, -2, 2 };

        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i], nc = c + dc[i];
            int mr = r + dr[i] / 2, mc = c + dc[i] / 2;

            if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
                int targetIdx = nr * BOARD_SIZE + nc;
                int midIdx = mr * BOARD_SIZE + mc;
                int midPiece = tempBoard[midIdx];

                bool isEnemy = false;
                if (p == P1 || p == P1_KING) {
                    isEnemy = (midPiece == P2 || midPiece == P2_KING || midPiece == OBSTACLE);
                }
                else if (p == P2 || p == P2_KING) {
                    isEnemy = (midPiece == P1 || midPiece == P1_KING || midPiece == OBSTACLE);
                }

                if (tempBoard[targetIdx] == EMPTY && isEnemy) {
                    tempBoard[midIdx] = EMPTY;
                    currentCaptured.push_back(midIdx);
                    currentPath.push_back(targetIdx);

                    findCaptures(targetIdx, p, currentCaptured, currentPath, allMoves, tempBoard);

                    tempBoard[midIdx] = midPiece;
                    currentCaptured.pop_back();
                    currentPath.pop_back();
                    foundAnyCapture = true;
                }
            }
        }
        if (!foundAnyCapture && !currentCaptured.empty()) {
            allMoves.push_back({ idx, currentCaptured, currentPath });
        }
    }

    std::vector<Move> getBestCaptures(int startIndex) {
        std::vector<Move> allMoves;
        std::vector<int> tempBoard = board;
        findCaptures(startIndex, board[startIndex], {}, { startIndex }, allMoves, tempBoard);

        if (allMoves.empty()) return {};
        int maxScore = 0;
        for (auto& m : allMoves) maxScore = std::max(maxScore, m.score());

        std::vector<Move> bestMoves;
        for (auto& m : allMoves) {
            if (m.score() == maxScore) bestMoves.push_back(m);
        }
        return bestMoves;
    }

    int countEnemies() {
        int count = 0;
        for (int p : board) if (p == OBSTACLE) count++;
        return count;
    }

    void spawnEnemy() {
        turnsSinceLastSpawn++;
        if (turnsSinceLastSpawn >= SPAWN_COOLDOWN && countEnemies() < maxEnemies) {
            if (std::rand() % 100 < 40) {
                std::vector<int> targetSlots;
                for (int i = 30; i < 70; ++i) {
                    int r = i / 10, c = i % 10;
                    if ((r + c) % 2 != 0 && board[i] == EMPTY) {
                        targetSlots.push_back(i);
                    }
                }
                if (!targetSlots.empty()) {
                    board[targetSlots[std::rand() % targetSlots.size()]] = OBSTACLE;
                    turnsSinceLastSpawn = 0;
                    std::cout << "Neutral Enemy spawned!" << std::endl;
                }
            }
        }
    }

    void moveEnemies() {
        turnsSinceLastMove++;
        if (turnsSinceLastMove >= MOVE_COOLDOWN) {
            std::vector<int> currentEnemies;
            for (int i = 0; i < TOTAL_TILES; ++i) if (board[i] == OBSTACLE) currentEnemies.push_back(i);

            for (int currIdx : currentEnemies) {
                int er = currIdx / 10, ec = currIdx % 10;
                int targetIdx = -1; float minDist = 999.0f;

                for (int j = 0; j < TOTAL_TILES; ++j) {
                    if (board[j] == P1 || board[j] == P2 || board[j] == P1_KING || board[j] == P2_KING) {
                        float d = std::sqrt(std::pow((j / 10) - er, 2) + std::pow((j % 10) - ec, 2));
                        if (d < minDist) { minDist = d; targetIdx = j; }
                    }
                }

                if (targetIdx != -1) {
                    int tr = targetIdx / 10, tc = targetIdx % 10;
                    int dr = (tr > er) ? 1 : -1;
                    int dc = (tc > ec) ? 1 : -1;

                    int nr = er + dr, nc = ec + dc;
                    if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
                        int nIdx = nr * 10 + nc;
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
    sf::RenderWindow window(sf::VideoMode(800, 800), "Checkers - Red's Turn (P1)");
    CheckersBoard game;
    int selectedIndex = -1;
    std::vector<Move> availableCaptures;
    int currentTurn = P1;
    bool isGameOver = false;

    // --- โหลดรูปภาพจบเกม ---
    sf::Texture endgameTexture;
    bool hasEndgameImage = endgameTexture.loadFromFile("endgame.jpg");
    sf::Sprite endgameSprite;
    sf::FloatRect startButtonArea;

    if (hasEndgameImage) {
        endgameSprite.setTexture(endgameTexture);

        // คำนวณสเกลแกน X และ Y แยกกัน เพื่อบังคับให้ภาพขยายเต็ม 800x800
        float scaleX = 800.0f / endgameTexture.getSize().x;
        float scaleY = 800.0f / endgameTexture.getSize().y;

        endgameSprite.setScale(scaleX, scaleY);
        endgameSprite.setPosition(0.0f, 0.0f); // จัดให้ชิดมุมซ้ายบนสุด

        // ปรับตำแหน่ง Hitbox ของปุ่ม START! ใหม่ให้ตรงกับภาพที่ถูกยืด
        startButtonArea = sf::FloatRect(330.0f, 480.0f, 140.0f, 70.0f);
    }

    sf::Font font;
    font.loadFromFile("arial.ttf");

    auto endTurn = [&]() {
        selectedIndex = -1;
        availableCaptures.clear();
        game.spawnEnemy();
        game.moveEnemies();
        currentTurn = (currentTurn == P1) ? P2 : P1;

        int p1Count = 0, p2Count = 0;
        game.countPieces(p1Count, p2Count);

        if (p1Count <= 1 || p2Count <= 1) {
            isGameOver = true;
            if (p1Count <= 1) window.setTitle("Checkers - Green Wins! (Click START! to Restart)");
            if (p2Count <= 1) window.setTitle("Checkers - Red Wins! (Click START! to Restart)");
        }
        else {
            if (currentTurn == P1) window.setTitle("Checkers - Red's Turn (P1)");
            else window.setTitle("Checkers - Green's Turn (P2)");
        }
        };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {

                if (isGameOver) {
                    if (startButtonArea.contains(event.mouseButton.x, event.mouseButton.y)) {
                        game.reset();
                        isGameOver = false;
                        currentTurn = P1;
                        selectedIndex = -1;
                        availableCaptures.clear();
                        window.setTitle("Checkers - Red's Turn (P1)");
                    }
                    continue;
                }

                int col = event.mouseButton.x / TILE_SIZE;
                int row = event.mouseButton.y / TILE_SIZE;
                int clickedIndex = row * BOARD_SIZE + col;

                if (col >= 0 && col < BOARD_SIZE && row >= 0 && row < BOARD_SIZE) {
                    if (game.board[clickedIndex] != EMPTY && game.board[clickedIndex] != OBSTACLE) {
                        int p = game.board[clickedIndex];
                        if ((currentTurn == P1 && (p == P1 || p == P1_KING)) ||
                            (currentTurn == P2 && (p == P2 || p == P2_KING))) {
                            selectedIndex = clickedIndex;
                            availableCaptures = game.getBestCaptures(selectedIndex);
                        }
                    }
                    else if (selectedIndex != -1) {
                        bool moved = false;
                        for (auto& m : availableCaptures) {
                            if (m.toIndex == clickedIndex) {
                                game.board[clickedIndex] = game.board[selectedIndex];
                                game.board[selectedIndex] = EMPTY;
                                for (int capIdx : m.capturedIndices) game.board[capIdx] = EMPTY;
                                moved = true;
                                break;
                            }
                        }

                        if (!moved && availableCaptures.empty()) {
                            int rDiff = (clickedIndex / 10) - (selectedIndex / 10);
                            int cDiff = (clickedIndex % 10) - (selectedIndex % 10);
                            if (std::abs(rDiff) == 1 && std::abs(cDiff) == 1) {
                                int p = game.board[selectedIndex];
                                if ((p == P1 && rDiff == 1) || (p == P2 && rDiff == -1) || p == P1_KING || p == P2_KING) {
                                    game.board[clickedIndex] = game.board[selectedIndex];
                                    game.board[selectedIndex] = EMPTY;
                                    moved = true;
                                }
                            }
                        }

                        if (moved) {
                            endTurn();
                        }
                        else {
                            selectedIndex = -1;
                            availableCaptures.clear();
                        }
                    }
                }
            }
        }

        window.clear();

        for (int i = 0; i < TOTAL_TILES; i++) {
            sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            rect.setPosition((i % 10) * TILE_SIZE, (i / 10) * TILE_SIZE);

            if (((i / 10 + i % 10) % 2 != 0)) rect.setFillColor(sf::Color(139, 69, 19));
            else rect.setFillColor(sf::Color(245, 222, 179));

            if (selectedIndex != -1 && !isGameOver) {
                for (auto& m : availableCaptures) {
                    for (int pathIdx : m.pathIndices) {
                        if (i == pathIdx) rect.setFillColor(sf::Color(0, 255, 255, 120));
                    }
                    if (i == m.toIndex) rect.setFillColor(sf::Color(255, 255, 0, 180));
                }
            }
            window.draw(rect);

            int piece = game.board[i];
            if (piece != EMPTY) {
                sf::CircleShape circle(TILE_SIZE / 2 - 8);
                circle.setPosition((i % 10) * TILE_SIZE + 8, (i / 10) * TILE_SIZE + 8);

                if (piece == P1 || piece == P1_KING) {
                    circle.setFillColor(sf::Color(255, 51, 51));
                    circle.setOutlineThickness(2);
                    circle.setOutlineColor(sf::Color(204, 0, 0));
                }
                else if (piece == P2 || piece == P2_KING) {
                    circle.setFillColor(sf::Color(0, 153, 76));
                    circle.setOutlineThickness(2);
                    circle.setOutlineColor(sf::Color(0, 255, 128));
                }
                else if (piece == OBSTACLE) {
                    circle.setFillColor(sf::Color::Magenta);
                    circle.setOutlineThickness(3);
                    circle.setOutlineColor(sf::Color::Cyan);
                }

                if (i == selectedIndex && !isGameOver) {
                    circle.setOutlineThickness(4);
                    circle.setOutlineColor(sf::Color::Yellow);
                }
                window.draw(circle);
            }
        }

        if (isGameOver) {
            sf::RectangleShape overlay(sf::Vector2f(800.0f, 800.0f));
            overlay.setFillColor(sf::Color(0, 0, 0, 200));
            window.draw(overlay);

            if (hasEndgameImage) {
                window.draw(endgameSprite);
            }
            else {
                sf::Text text;
                text.setFont(font);
                text.setString("GAME OVER\nClick center to Restart");
                text.setCharacterSize(50);
                text.setFillColor(sf::Color::White);
                text.setPosition(sf::Vector2f(150.0f, 350.0f));
                window.draw(text);

                startButtonArea = sf::FloatRect(0.0f, 0.0f, 800.0f, 800.0f);
            }
        }

        window.display();
    }
    return 0;
}
