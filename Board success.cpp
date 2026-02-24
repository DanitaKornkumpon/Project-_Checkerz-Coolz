#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

const int BOARD_SIZE = 10;
const int TOTAL_TILES = 100;
const float TILE_SIZE = 80.0f;

enum PieceType { EMPTY = 0, P1 = 1, P2 = 2, P1_KING = 3, P2_KING = 4 };

struct Move {
    int toIndex;
    std::vector<int> capturedIndices;
    int score() const { return capturedIndices.size(); }
};

class CheckersBoard {
public:
    std::vector<PieceType> board;
    CheckersBoard() {
        board.resize(TOTAL_TILES, EMPTY);
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

    // ฟังก์ชันหาการกินที่ยาวที่สุด (Recursive DFS)
    void findCaptures(int idx, PieceType p, std::vector<int> currentCaptured, std::vector<Move>& allMoves, std::vector<PieceType>& tempBoard) {
        int r = idx / BOARD_SIZE;
        int c = idx % BOARD_SIZE;
        bool foundAnyCapture = false;

        // ทิศทางที่เดินได้ (กินถอยหลังได้เสมอสำหรับทุกหมาก)
        int dr[] = {-2, -2, 2, 2};
        int dc[] = {-2, 2, -2, 2};

        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i], nc = c + dc[i];
            int mr = r + dr[i]/2, mc = c + dc[i]/2;

            if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
                int targetIdx = nr * BOARD_SIZE + nc;
                int midIdx = mr * BOARD_SIZE + mc;
                
                PieceType midPiece = tempBoard[midIdx];
                bool isEnemy = (p == P1 || p == P1_KING) ? (midPiece == P2 || midPiece == P2_KING) : (midPiece == P1 || midPiece == P1_KING);

                if (tempBoard[targetIdx] == EMPTY && isEnemy) {
                    // จำลองการกิน
                    tempBoard[midIdx] = EMPTY;
                    currentCaptured.push_back(midIdx);
                    
                    findCaptures(targetIdx, p, currentCaptured, allMoves, tempBoard);
                    
                    // คืนค่า (Backtrack)
                    tempBoard[midIdx] = midPiece;
                    currentCaptured.pop_back();
                    foundAnyCapture = true;
                }
            }
        }

        if (!foundAnyCapture && !currentCaptured.empty()) {
            allMoves.push_back({idx, currentCaptured});
        }
    }

    std::vector<Move> getBestCaptures(int startIndex) {
        std::vector<Move> allMoves;
        std::vector<PieceType> tempBoard = board;
        findCaptures(startIndex, board[startIndex], {}, allMoves, tempBoard);

        if (allMoves.empty()) return {};

        // หาจำนวนที่กินได้มากที่สุด
        int maxScore = 0;
        for (auto& m : allMoves) maxScore = std::max(maxScore, m.score());

        // กรองเอาเฉพาะเส้นทางที่ได้แต้มสูงสุด
        std::vector<Move> bestMoves;
        for (auto& m : allMoves) {
            if (m.score() == maxScore) bestMoves.push_back(m);
        }
        return bestMoves;
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 800), "Checkers Max Capture Logic");
    CheckersBoard game;
    int selectedIndex = -1;
    std::vector<Move> availableCaptures;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int col = event.mouseButton.x / TILE_SIZE;
                int row = event.mouseButton.y / TILE_SIZE;
                int clickedIndex = row * BOARD_SIZE + col;

                if (game.board[clickedIndex] != EMPTY) {
                    selectedIndex = clickedIndex;
                    availableCaptures = game.getBestCaptures(selectedIndex);
                } 
                else if (selectedIndex != -1) {
                    bool moved = false;
                    
                    // ตรวจสอบว่าเป็นการกินที่ถูกต้อง (ต้องเป็นทางที่ยาวที่สุด)
                    if (!availableCaptures.empty()) {
                        for (auto& m : availableCaptures) {
                            if (m.toIndex == clickedIndex) {
                                game.board[clickedIndex] = game.board[selectedIndex];
                                game.board[selectedIndex] = EMPTY;
                                for (int capIdx : m.capturedIndices) game.board[capIdx] = EMPTY;
                                moved = true;
                                break;
                            }
                        }
                    } else {
                        // ถ้าไม่มีการกิน ให้เช็คการเดินปกติ (Logic เดิมของคุณ)
                        int rDiff = (clickedIndex / 10) - (selectedIndex / 10);
                        int cDiff = (clickedIndex % 10) - (selectedIndex % 10);
                        if (std::abs(rDiff) == 1 && std::abs(cDiff) == 1) {
                            // เช็คทิศทางเดินปกติ (P1 ลง, P2 ขึ้น)
                            if ((game.board[selectedIndex] == P1 && rDiff == 1) || (game.board[selectedIndex] == P2 && rDiff == -1)) {
                                game.board[clickedIndex] = game.board[selectedIndex];
                                game.board[selectedIndex] = EMPTY;
                                moved = true;
                            }
                        }
                    }
                    if (moved) selectedIndex = -1;
                }
            }
        }

        window.clear();
        // ส่วนการวาด 
        for (int i = 0; i < TOTAL_TILES; i++) {
            sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            rect.setPosition((i % 10) * TILE_SIZE, (i / 10) * TILE_SIZE);
            rect.setFillColor(((i/10 + i%10) % 2 != 0) ? sf::Color(139,69,19) : sf::Color(245,222,179));
            window.draw(rect);

            if (game.board[i] != EMPTY) {
                sf::CircleShape circle(TILE_SIZE/2 - 5);
                circle.setPosition((i % 10) * TILE_SIZE + 5, (i / 10) * TILE_SIZE + 5);
                circle.setFillColor(game.board[i] == P1 ? sf::Color::Red : sf::Color::Green);
                if (i == selectedIndex) circle.setOutlineThickness(3), circle.setOutlineColor(sf::Color::Yellow);
                window.draw(circle);
            }
        }
        window.display();
    }
    return 0;
}
