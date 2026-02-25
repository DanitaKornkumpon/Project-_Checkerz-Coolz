#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <bitset>
#include <random>
#include <unordered_map>

const int BOARD_SIZE = 10;
const int TOTAL_TILES = 100;
const float TILE_SIZE = 80.0f;

enum PieceType { EMPTY = 0, P1 = 1, P2 = 2, P1_KING = 3, P2_KING = 4, OBSTACLE = 5 };

struct Move {
    int toIndex;
    std::vector<int> capturedIndices;
    std::vector<int> pathIndices;
    int score() const { return static_cast<int>(capturedIndices.size()); }
};

struct PieceStats {
    int forward = 0;
    int backward = 0;
    int left = 0;
    int right = 0;
};

struct BitboardState {
    uint64_t p1 = 0;
    uint64_t p2 = 0;
    uint64_t p1_kings = 0;
    uint64_t p2_kings = 0;
    uint64_t obstacles = 0;
    uint64_t empty = 0;
};

struct BoardState {
    std::vector<int> board;
    std::vector<PieceStats> statsBoard;
    BitboardState bitboard;
    bool inUse = false;

    BoardState() {
        board.resize(TOTAL_TILES, EMPTY);
        statsBoard.resize(TOTAL_TILES);
    }
};

class StatePool {
private:
    std::vector<BoardState> pool;
    std::vector<BoardState*> freeList;

public:
    StatePool(int poolSize) {
        pool.resize(poolSize);
        for (int i = 0; i < poolSize; ++i) {
            freeList.push_back(&pool[i]);
        }
    }
    BoardState* acquire() {
        if (freeList.empty()) return nullptr;
        BoardState* state = freeList.back();
        freeList.pop_back();
        state->inUse = true;
        return state;
    }
    void release(BoardState* state) {
        if (state && state->inUse) {
            state->inUse = false;
            freeList.push_back(state);
        }
    }
    int getAvailableCount() const { return static_cast<int>(freeList.size()); }
};

class CheckersBoard {
public:
    std::vector<int> board;
    std::vector<PieceStats> statsBoard;
    BitboardState bitboard;

    uint64_t MASK_EVEN_ROWS = 0;
    uint64_t MASK_ODD_ROWS = 0;

    const int maxEnemies = 2;
    int turnsSinceLastSpawn = 0;
    int turnsSinceLastMove = 0;
    const int SPAWN_COOLDOWN = 6;
    const int MOVE_COOLDOWN = 3;

    CheckersBoard() {
        board.resize(TOTAL_TILES, EMPTY);
        statsBoard.resize(TOTAL_TILES);
        std::srand(static_cast<unsigned int>(std::time(0)));
        initializeMasks();
        initializeBoard();
    }

    void initializeMasks() {
        for (int r = 0; r < BOARD_SIZE; ++r) {
            for (int c = 0; c < BOARD_SIZE / 2; ++c) {
                uint64_t bitIndex = (r * 5) + c;
                if (r % 2 == 0) MASK_EVEN_ROWS |= (1ULL << bitIndex);
                else MASK_ODD_ROWS |= (1ULL << bitIndex);
            }
        }
    }

    int getBitIndex(int idx) const {
        int r = idx / 10;
        int c = idx % 10;
        if ((r + c) % 2 == 0) return -1;
        return (r * 5) + (c / 2);
    }

    void updateBitboard() {
        bitboard.p1 = 0; bitboard.p2 = 0;
        bitboard.p1_kings = 0; bitboard.p2_kings = 0;
        bitboard.obstacles = 0; bitboard.empty = 0;

        for (int i = 0; i < TOTAL_TILES; ++i) {
            int bitIdx = getBitIndex(i);
            if (bitIdx != -1) {
                uint64_t mask = (1ULL << bitIdx);
                if (board[i] == P1) bitboard.p1 |= mask;
                else if (board[i] == P2) bitboard.p2 |= mask;
                else if (board[i] == P1_KING) bitboard.p1_kings |= mask;
                else if (board[i] == P2_KING) bitboard.p2_kings |= mask;
                else if (board[i] == OBSTACLE) bitboard.obstacles |= mask;
                else bitboard.empty |= mask;
            }
        }
    }

    void demoBitboardPower() {
        uint64_t p1_down_left = ((bitboard.p1 & MASK_EVEN_ROWS) << 5) | ((bitboard.p1 & MASK_ODD_ROWS) << 4);
        uint64_t p1_down_right = ((bitboard.p1 & MASK_EVEN_ROWS) << 6) | ((bitboard.p1 & MASK_ODD_ROWS) << 5);
        uint64_t valid_p1_moves = (p1_down_left | p1_down_right) & bitboard.empty;

        // แก้ไข __builtin_popcountll ให้รองรับ MSVC โดยใช้ std::bitset
        int possibleMoves = static_cast<int>(std::bitset<64>(valid_p1_moves).count());
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
        updateBitboard();
    }

    void copyToState(BoardState* state) const {
        state->board = this->board;
        state->statsBoard = this->statsBoard;
        state->bitboard = this->bitboard;
    }

    void recordMove(int fromIdx, int toIdx, int pieceType) {
        if (fromIdx == toIdx) return;
        int rDiff = (toIdx / 10) - (fromIdx / 10);
        int cDiff = (toIdx % 10) - (fromIdx % 10);

        if (pieceType == P1 || pieceType == P1_KING) {
            if (rDiff > 0) statsBoard[fromIdx].forward++;
            else if (rDiff < 0) statsBoard[fromIdx].backward++;
        }
        else if (pieceType == P2 || pieceType == P2_KING) {
            if (rDiff < 0) statsBoard[fromIdx].forward++;
            else if (rDiff > 0) statsBoard[fromIdx].backward++;
        }

        if (cDiff > 0) statsBoard[fromIdx].right++;
        else if (cDiff < 0) statsBoard[fromIdx].left++;

        statsBoard[toIdx] = statsBoard[fromIdx];
        statsBoard[fromIdx] = PieceStats();
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
                if (p == P1 || p == P1_KING) isEnemy = (midPiece == P2 || midPiece == P2_KING || midPiece == OBSTACLE);
                else if (p == P2 || p == P2_KING) isEnemy = (midPiece == P1 || midPiece == P1_KING || midPiece == OBSTACLE);

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
                    if ((r + c) % 2 != 0 && board[i] == EMPTY) targetSlots.push_back(i);
                }
                if (!targetSlots.empty()) {
                    board[targetSlots[std::rand() % targetSlots.size()]] = OBSTACLE;
                    turnsSinceLastSpawn = 0;
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
                        // ปรับแก้ Warning C4244 โดยการแปลง type ให้ตรงกันและหลีกเลี่ยงการใช้ pow ซ้อนทับกัน
                        float dx = static_cast<float>((j / 10) - er);
                        float dy = static_cast<float>((j % 10) - ec);
                        float d = std::sqrt(dx * dx + dy * dy);
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

class ZobristHasher {
public:
    uint64_t pieceKeys[6][50];
    uint64_t turnKey;

    ZobristHasher() {
        std::mt19937_64 rng(12345);
        for (int p = 0; p < 6; ++p) {
            for (int pos = 0; pos < 50; ++pos) {
                pieceKeys[p][pos] = rng();
            }
        }
        turnKey = rng();
    }

    uint64_t computeHash(const CheckersBoard& game, int currentTurn) {
        uint64_t h = 0;
        for (int i = 0; i < TOTAL_TILES; ++i) {
            int bitIdx = game.getBitIndex(i);
            if (bitIdx != -1) {
                int piece = game.board[i];
                if (piece != EMPTY) {
                    h ^= pieceKeys[piece][bitIdx];
                }
            }
        }
        if (currentTurn == P1) h ^= turnKey;
        return h;
    }
};

struct TTEntry {
    int score;
    int depth;
};

class TranspositionTable {
private:
    std::unordered_map<uint64_t, TTEntry> table;

public:
    void store(uint64_t hash, int score, int depth) {
        table[hash] = { score, depth };
    }

    bool lookup(uint64_t hash, TTEntry& entry) {
        auto it = table.find(hash);
        if (it != table.end()) {
            entry = it->second;
            return true;
        }
        return false;
    }
    int size() const { return static_cast<int>(table.size()); }
};

struct AIMoveChoice {
    int fromIndex;
    int toIndex;
    Move captureMove;
    bool isCapture;
};

void makeAITurn(CheckersBoard& game) {
    std::vector<AIMoveChoice> possibleCaptures;
    std::vector<AIMoveChoice> possibleNormalMoves;

    for (int i = 0; i < TOTAL_TILES; i++) {
        int piece = game.board[i];
        if (piece == P2 || piece == P2_KING) {

            std::vector<Move> caps = game.getBestCaptures(i);
            for (auto& cap : caps) {
                possibleCaptures.push_back({ i, cap.toIndex, cap, true });
            }

            if (caps.empty()) {
                int r = i / 10;
                int c = i % 10;
                int dr[] = { -1, -1, 1, 1 };
                int dc[] = { -1, 1, -1, 1 };

                for (int dir = 0; dir < 4; dir++) {
                    if (piece == P2 && dr[dir] == 1) continue;

                    int nr = r + dr[dir];
                    int nc = c + dc[dir];

                    if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
                        int targetIdx = nr * 10 + nc;
                        if (game.board[targetIdx] == EMPTY) {
                            possibleNormalMoves.push_back({ i, targetIdx, Move(), false });
                        }
                    }
                }
            }
        }
    }

    AIMoveChoice selectedMove;
    bool moveFound = false;

    if (!possibleCaptures.empty()) {
        int maxScore = 0;
        for (auto& m : possibleCaptures) {
            if (m.captureMove.score() > maxScore) maxScore = m.captureMove.score();
        }
        std::vector<AIMoveChoice> bestCaptures;
        for (auto& m : possibleCaptures) {
            if (m.captureMove.score() == maxScore) bestCaptures.push_back(m);
        }
        selectedMove = bestCaptures[std::rand() % bestCaptures.size()];
        moveFound = true;
        std::cout << "[AI] Found a capture! Eating " << maxScore << " piece(s)." << std::endl;
    }
    else if (!possibleNormalMoves.empty()) {
        selectedMove = possibleNormalMoves[std::rand() % possibleNormalMoves.size()];
        moveFound = true;
        std::cout << "[AI] Moving forward." << std::endl;
    }

    if (moveFound) {
        int from = selectedMove.fromIndex;
        int to = selectedMove.toIndex;
        int movingPiece = game.board[from];

        if (selectedMove.isCapture) {
            int currentPos = from;
            for (int pathIdx : selectedMove.captureMove.pathIndices) {
                game.recordMove(currentPos, pathIdx, movingPiece);
                currentPos = pathIdx;
            }
            game.board[to] = movingPiece;
            game.board[from] = EMPTY;
            for (int capIdx : selectedMove.captureMove.capturedIndices) {
                game.board[capIdx] = EMPTY;
                game.statsBoard[capIdx] = PieceStats();
            }
        }
        else {
            game.recordMove(from, to, movingPiece);
            game.board[to] = movingPiece;
            game.board[from] = EMPTY;
        }

        // --- ระบบเข้าฮอส (King Promotion) ให้ AI (สีเขียวเดินขึ้นไปแถว 0) ---
        if (movingPiece == P2 && to / BOARD_SIZE == 0) {
            game.board[to] = P2_KING;
            std::cout << "[AI] My piece has been PROMOTED to KING!" << std::endl;
        }
    }
    else {
        std::cout << "[AI] No valid moves left. I surrender!" << std::endl;
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 800), "Checkers - Red's Turn (Player)");
    CheckersBoard game;
    StatePool aiMemoryPool(10000);
    ZobristHasher zobrist;
    TranspositionTable tTable;

    int selectedIndex = -1;
    std::vector<Move> availableCaptures;
    int currentTurn = P1;

    sf::Clock aiClock;
    bool isAIWaiting = false;

    game.demoBitboardPower();

    auto endTurn = [&]() {
        selectedIndex = -1;
        availableCaptures.clear();
        game.spawnEnemy();
        game.moveEnemies();

        game.updateBitboard();

        uint64_t currentHash = zobrist.computeHash(game, currentTurn);
        TTEntry entry;
        if (!tTable.lookup(currentHash, entry)) {
            int simulatedScore = (std::rand() % 100) - 50;
            tTable.store(currentHash, simulatedScore, 1);
        }

        currentTurn = (currentTurn == P1) ? P2 : P1;
        if (currentTurn == P1) window.setTitle("Checkers - Red's Turn (Player)");
        else window.setTitle("Checkers - Green's Turn (AI Thinking...)");
        };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // ล็อกให้คลิกได้เฉพาะตาของ P1 (คนเล่น) เท่านั้น
            if (currentTurn == P1 && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                // เพิ่ม static_cast<int> เพื่อป้องกัน Warning C4244
                int col = static_cast<int>(event.mouseButton.x / TILE_SIZE);
                int row = static_cast<int>(event.mouseButton.y / TILE_SIZE);
                int clickedIndex = row * BOARD_SIZE + col;

                if (col >= 0 && col < BOARD_SIZE && row >= 0 && row < BOARD_SIZE) {

                    if (game.board[clickedIndex] != EMPTY && game.board[clickedIndex] != OBSTACLE) {
                        int p = game.board[clickedIndex];
                        if (p == P1 || p == P1_KING) {
                            selectedIndex = clickedIndex;
                            availableCaptures = game.getBestCaptures(selectedIndex);
                        }
                    }
                    else if (selectedIndex != -1) {
                        bool moved = false;
                        int movingPiece = game.board[selectedIndex];

                        for (auto& m : availableCaptures) {
                            if (m.toIndex == clickedIndex) {
                                int currentPos = selectedIndex;
                                for (int pathIdx : m.pathIndices) {
                                    game.recordMove(currentPos, pathIdx, movingPiece);
                                    currentPos = pathIdx;
                                }

                                game.board[clickedIndex] = game.board[selectedIndex];
                                game.board[selectedIndex] = EMPTY;

                                for (int capIdx : m.capturedIndices) {
                                    game.board[capIdx] = EMPTY;
                                    game.statsBoard[capIdx] = PieceStats();
                                }
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
                                    game.recordMove(selectedIndex, clickedIndex, movingPiece);
                                    game.board[clickedIndex] = game.board[selectedIndex];
                                    game.board[selectedIndex] = EMPTY;
                                    moved = true;
                                }
                            }
                        }

                        if (moved) {
                            // --- ระบบเข้าฮอส (King Promotion) ให้ผู้เล่น P1 (สีแดงเดินลงมาแถว 9) ---
                            if (movingPiece == P1 && clickedIndex / BOARD_SIZE == 9) {
                                game.board[clickedIndex] = P1_KING;
                                std::cout << "[Player] Your piece has been PROMOTED to KING!" << std::endl;
                            }
                            endTurn();
                        }
                        else {
                            selectedIndex = -1;
                            availableCaptures.clear();
                        }
                    }
                }
            }
            // เพิ่มการแจ้งเตือน หากพยายามคลิกตอนเป็นตาของ AI
            else if (currentTurn == P2 && event.type == sf::Event::MouseButtonPressed) {
                std::cout << "[System] Not your turn! AI is thinking..." << std::endl;
            }
        }

        // ==========================================
        // --- ส่วนควบคุม AI ให้ทำงานเมื่อถึงตา P2 ---
        // ==========================================
        if (currentTurn == P2) {
            if (!isAIWaiting) {
                std::cout << "[System] AI is taking control. Waiting 600ms..." << std::endl;
                aiClock.restart();
                isAIWaiting = true;
            }
            else if (aiClock.getElapsedTime().asMilliseconds() >= 600) {
                makeAITurn(game);
                endTurn();
                isAIWaiting = false;
            }
        }

        window.clear();
        for (int i = 0; i < TOTAL_TILES; i++) {
            sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            rect.setPosition((i % 10) * TILE_SIZE, (i / 10) * TILE_SIZE);

            if (((i / 10 + i % 10) % 2 != 0)) rect.setFillColor(sf::Color(139, 69, 19));
            else rect.setFillColor(sf::Color(245, 222, 179));

            if (selectedIndex != -1) {
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

                if (i == selectedIndex) {
                    circle.setOutlineThickness(4);
                    circle.setOutlineColor(sf::Color::Yellow);
                }
                window.draw(circle);
            }
        }
        window.display();
    }
    return 0;
}