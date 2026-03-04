#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <map>
#include <string>

const int BOARD_SIZE = 10;
const int TOTAL_TILES = 100;
const float TILE_SIZE = 80.0f;

enum PieceType { EMPTY = 0, P1 = 1, P2 = 2, P1_KING = 3, P2_KING = 4, OBSTACLE = 5 };
enum GameState { MENU, PLAYING, GAMEOVER };

// --- โครงสร้างสำหรับเก็บตาเดิน ---
struct Move {
    int toIndex;
    std::vector<int> capturedIndices;
    std::vector<int> pathIndices;
    int score() const { return capturedIndices.size(); }
};

// --- โครงสร้างสำหรับเก็บสถิติการเดินของหมากแต่ละตัว ---
struct PieceStats {
    int id = 0;
    int totalMoves = 0;
    int moveForward = 0;
    int moveBackward = 0;
    int moveLeft = 0;
    int moveRight = 0;
};

class CheckersBoard {
public:
    std::vector<int> board;
    std::vector<int> pieceIdBoard; // กระดานคู่ขนานสำหรับจำหมายเลขหมาก 1-40
    std::map<int, PieceStats> stats; // สมุดจดสถิติ

    const int maxEnemies = 2;
    int turnsSinceLastSpawn = 0;
    int turnsSinceLastMove = 0;
    const int SPAWN_COOLDOWN = 6;
    const int MOVE_COOLDOWN = 3;

    CheckersBoard() {
        board.resize(TOTAL_TILES, EMPTY);
        pieceIdBoard.resize(TOTAL_TILES, 0);
        std::srand(static_cast<unsigned int>(std::time(0)));
        initializeBoard();
    }

    void initializeBoard() {
        // สร้างหมาก P1 (สีแดง) พร้อมให้หมายเลข 1 ถึง 20
        int p1_id = 1;
        for (int i = 0; i < 40; i++) {
            int r = i / 10, c = i % 10;
            if ((r + c) % 2 != 0) {
                board[i] = P1;
                pieceIdBoard[i] = p1_id;
                stats[p1_id].id = p1_id;
                p1_id++;
            }
        }

        // สร้างหมาก P2 (สีเขียว) พร้อมให้หมายเลข 21 ถึง 40
        int p2_id = 21;
        for (int i = 60; i < 100; i++) {
            int r = i / 10, c = i % 10;
            if ((r + c) % 2 != 0) {
                board[i] = P2;
                pieceIdBoard[i] = p2_id;
                stats[p2_id].id = p2_id;
                p2_id++;
            }
        }
    }

    void reset() {
        std::fill(board.begin(), board.end(), EMPTY);
        std::fill(pieceIdBoard.begin(), pieceIdBoard.end(), 0);
        stats.clear();
        turnsSinceLastSpawn = 0;
        turnsSinceLastMove = 0;
        initializeBoard();
    }

    // --- ฟังก์ชันอัปเดตสถิติและย้ายหน่วยความจำหมาก ---
    void executeMoveAndTrackStats(int fromIdx, int toIdx, const std::vector<int>& capturedIndices) {
        int id = pieceIdBoard[fromIdx];

        if (id != 0) {
            int fromR = fromIdx / BOARD_SIZE, fromC = fromIdx % BOARD_SIZE;
            int toR = toIdx / BOARD_SIZE, toC = toIdx % BOARD_SIZE;

            stats[id].totalMoves++;

            // P1 (1-20) เดินลงคือหน้า | P2 (21-40) เดินขึ้นคือหน้า
            bool isP1 = (id >= 1 && id <= 20);

            if (toR > fromR) {
                if (isP1) stats[id].moveForward++; else stats[id].moveBackward++;
            }
            else if (toR < fromR) {
                if (isP1) stats[id].moveBackward++; else stats[id].moveForward++;
            }

            if (toC > fromC) stats[id].moveRight++;
            else if (toC < fromC) stats[id].moveLeft++;

            // แสดงผลลัพธ์ใน Console
            std::cout << "[TRACKING] หมากหมายเลข " << id << " เดินรวม " << stats[id].totalMoves << " ครั้ง -> "
                << "(หน้า: " << stats[id].moveForward << ", หลัง: " << stats[id].moveBackward
                << ", ซ้าย: " << stats[id].moveLeft << ", ขวา: " << stats[id].moveRight << ")\n";
        }

        // อัปเดตตำแหน่ง ID ในกระดานคู่ขนาน
        pieceIdBoard[toIdx] = pieceIdBoard[fromIdx];
        pieceIdBoard[fromIdx] = 0;
        for (int capIdx : capturedIndices) {
            pieceIdBoard[capIdx] = 0; // หมากถูกกิน ลบ ID ทิ้ง
        }
    }

    void countPieces(int& p1Count, int& p2Count) {
        p1Count = 0; p2Count = 0;
        for (int piece : board) {
            if (piece == P1 || piece == P1_KING) p1Count++;
            if (piece == P2 || piece == P2_KING) p2Count++;
        }
    }

    // --- ฟังก์ชัน Recursion หาเส้นทางการกิน ---
    void findCaptures(int idx, int p, std::vector<int> currentCaptured, std::vector<int> currentPath, std::vector<Move>& allMoves, std::vector<int>& tempBoard) {
        int r = idx / BOARD_SIZE, c = idx % BOARD_SIZE;
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

    // --- ระบบศัตรูอุปสรรค ---
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

void centerTextInRect(sf::Text& text, const sf::RectangleShape& rect) {
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    text.setPosition(rect.getPosition().x + rect.getSize().x / 2.0f, rect.getPosition().y + rect.getSize().y / 2.0f);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 800), "Checkers Menu");
    CheckersBoard game;
    int selectedIndex = -1;
    std::vector<Move> availableCaptures;
    int currentTurn = P1;

    GameState currentState = MENU;
    int numPlayers = 2;
    sf::Clock aiTimer;

    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        if (!font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
            std::cout << "Error: Could not load font.\n";
        }
    }

    // --- UI เมนู ---
    sf::Texture menuBgTexture;
    bool hasMenuBg = false;
    std::vector<std::string> bgFileNames = {
        "checker.jpg",
        "a08dd583883b147e9996f867f0b4bb7c.jpg", "a08dd583883b147e9996f867f0b4bb7c.png", "menu_bg.jpg", "menu_bg.png"
    };

    for (const auto& fname : bgFileNames) {
        if (menuBgTexture.loadFromFile(fname)) { hasMenuBg = true; break; }
    }

    sf::Sprite menuBgSprite;
    if (hasMenuBg) {
        menuBgSprite.setTexture(menuBgTexture);
        menuBgSprite.setScale(800.0f / menuBgTexture.getSize().x, 800.0f / menuBgTexture.getSize().y);
    }

    sf::Texture texBtn1P, texBtn2P, texBtnStart;
    bool hasTex1P = texBtn1P.loadFromFile("btn_1p.png");
    bool hasTex2P = texBtn2P.loadFromFile("btn_2p.png");
    bool hasTexStart = texBtnStart.loadFromFile("btn_start.png");

    sf::Color woodNormal(160, 104, 63);
    sf::Color woodHover(190, 134, 93);
    sf::Color woodSelected(120, 74, 33);
    sf::Color woodBorder(70, 40, 20);

    sf::Text titleText("CHECKERS", font, 80);
    titleText.setFillColor(sf::Color(255, 220, 150));
    titleText.setOutlineColor(woodBorder);
    titleText.setOutlineThickness(4);
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.left + titleBounds.width / 2.0f, titleBounds.top + titleBounds.height / 2.0f);
    titleText.setPosition(400.0f, 150.0f);

    sf::RectangleShape btn1P(sf::Vector2f(220.0f, 70.0f));
    btn1P.setPosition(150.0f, 350.0f);
    if (hasTex1P) btn1P.setTexture(&texBtn1P);
    else { btn1P.setFillColor(woodNormal); btn1P.setOutlineThickness(4); btn1P.setOutlineColor(woodBorder); }
    sf::Text text1P("1 Player", font, 35);
    text1P.setFillColor(sf::Color::White);
    centerTextInRect(text1P, btn1P);

    sf::RectangleShape btn2P(sf::Vector2f(220.0f, 70.0f));
    btn2P.setPosition(430.0f, 350.0f);
    if (hasTex2P) btn2P.setTexture(&texBtn2P);
    else { btn2P.setFillColor(woodNormal); btn2P.setOutlineThickness(4); btn2P.setOutlineColor(woodBorder); }
    sf::Text text2P("2 Players", font, 35);
    text2P.setFillColor(sf::Color::White);
    centerTextInRect(text2P, btn2P);

    sf::RectangleShape btnStart(sf::Vector2f(280.0f, 85.0f));
    btnStart.setPosition(260.0f, 520.0f);
    if (hasTexStart) btnStart.setTexture(&texBtnStart);
    else { btnStart.setFillColor(sf::Color(100, 180, 80)); btnStart.setOutlineThickness(5); btnStart.setOutlineColor(woodBorder); }
    sf::Text textStart("START", font, 45);
    textStart.setFillColor(sf::Color::White);
    centerTextInRect(textStart, btnStart);

    sf::Texture endgameTexture;
    bool hasEndgameImage = endgameTexture.loadFromFile("endgame.jpg");
    sf::Sprite endgameSprite;
    sf::FloatRect endgameClickArea;
    if (hasEndgameImage) {
        endgameSprite.setTexture(endgameTexture);
        endgameSprite.setScale(800.0f / endgameTexture.getSize().x, 800.0f / endgameTexture.getSize().y);
        endgameClickArea = sf::FloatRect(330.0f, 480.0f, 140.0f, 70.0f);
    }

    auto endTurn = [&]() {
        selectedIndex = -1;
        availableCaptures.clear();

        game.spawnEnemy();
        game.moveEnemies();

        currentTurn = (currentTurn == P1) ? P2 : P1;
        if (numPlayers == 1 && currentTurn == P2) aiTimer.restart();

        int p1Count = 0, p2Count = 0;
        game.countPieces(p1Count, p2Count);

        if (p1Count <= 1 || p2Count <= 1) {
            currentState = GAMEOVER;
            if (p1Count <= 1) window.setTitle("Checkers - Green Wins! (Click to Return to Menu)");
            if (p2Count <= 1) window.setTitle("Checkers - Red Wins! (Click to Return to Menu)");
        }
        else {
            if (currentTurn == P1) window.setTitle("Checkers - Red's Turn (P1)");
            else window.setTitle("Checkers - Green's Turn (P2)");
        }
        };

    auto performAIMove = [&]() {
        std::vector<int> p2Pieces;
        for (int i = 0; i < TOTAL_TILES; i++) {
            if (game.board[i] == P2 || game.board[i] == P2_KING) p2Pieces.push_back(i);
        }

        std::vector<std::pair<int, Move>> allCaptures;
        int maxCapScore = 0;
        for (int idx : p2Pieces) {
            auto caps = game.getBestCaptures(idx);
            for (auto& m : caps) {
                if (m.score() > maxCapScore) {
                    maxCapScore = m.score();
                    allCaptures.clear();
                    allCaptures.push_back({ idx, m });
                }
                else if (m.score() == maxCapScore && maxCapScore > 0) {
                    allCaptures.push_back({ idx, m });
                }
            }
        }

        if (!allCaptures.empty()) {
            int randIdx = std::rand() % allCaptures.size();
            int fromIdx = allCaptures[randIdx].first;
            Move m = allCaptures[randIdx].second;

            game.executeMoveAndTrackStats(fromIdx, m.toIndex, m.capturedIndices);

            game.board[m.toIndex] = game.board[fromIdx];
            game.board[fromIdx] = EMPTY;
            for (int capIdx : m.capturedIndices) game.board[capIdx] = EMPTY;

            if (m.toIndex / 10 == 0) game.board[m.toIndex] = P2_KING;

            endTurn();
            return;
        }

        struct SimpleMove { int from, to; };
        std::vector<SimpleMove> simpleMoves;
        for (int idx : p2Pieces) {
            int r = idx / 10, c = idx % 10;
            int p = game.board[idx];

            int dr[] = { -1, -1, 1, 1 };
            int dc[] = { -1, 1, -1, 1 };
            int dirs = (p == P2_KING) ? 4 : 2;

            for (int i = 0; i < dirs; i++) {
                int nr = r + dr[i], nc = c + dc[i];
                if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
                    int nIdx = nr * 10 + nc;
                    if (game.board[nIdx] == EMPTY) simpleMoves.push_back({ idx, nIdx });
                }
            }
        }

        if (!simpleMoves.empty()) {
            int randIdx = std::rand() % simpleMoves.size();
            SimpleMove sm = simpleMoves[randIdx];

            game.executeMoveAndTrackStats(sm.from, sm.to, {});

            game.board[sm.to] = game.board[sm.from];
            game.board[sm.from] = EMPTY;

            if (sm.to / 10 == 0) game.board[sm.to] = P2_KING;

            endTurn();
        }
        else {
            endTurn();
        }
        };

    while (window.isOpen()) {
        sf::Event event;
        float mouseX = sf::Mouse::getPosition(window).x;
        float mouseY = sf::Mouse::getPosition(window).y;

        if (currentState == MENU) {
            if (!hasTex1P) btn1P.setFillColor(btn1P.getGlobalBounds().contains(mouseX, mouseY) ? woodHover : (numPlayers == 1 ? woodSelected : woodNormal));
            if (!hasTex2P) btn2P.setFillColor(btn2P.getGlobalBounds().contains(mouseX, mouseY) ? woodHover : (numPlayers == 2 ? woodSelected : woodNormal));
            if (!hasTexStart) btnStart.setFillColor(btnStart.getGlobalBounds().contains(mouseX, mouseY) ? sf::Color(120, 200, 100) : sf::Color(100, 180, 80));
        }

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (currentState == MENU) {
                    if (btn1P.getGlobalBounds().contains(mouseX, mouseY)) numPlayers = 1;
                    if (btn2P.getGlobalBounds().contains(mouseX, mouseY)) numPlayers = 2;
                    if (btnStart.getGlobalBounds().contains(mouseX, mouseY)) {
                        currentState = PLAYING;
                        game.reset();
                        currentTurn = P1;
                        window.setTitle("Checkers - Red's Turn (P1)");
                    }
                }
                else if (currentState == GAMEOVER) {
                    bool clickedToRestart = false;
                    if (hasEndgameImage && endgameClickArea.contains(mouseX, mouseY)) clickedToRestart = true;
                    else if (!hasEndgameImage) clickedToRestart = true;

                    if (clickedToRestart) {
                        currentState = MENU;
                        window.setTitle("Checkers Menu");
                    }
                }
                else if (currentState == PLAYING) {
                    if (numPlayers == 1 && currentTurn == P2) continue;

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
                                    game.executeMoveAndTrackStats(selectedIndex, clickedIndex, m.capturedIndices);

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
                                        game.executeMoveAndTrackStats(selectedIndex, clickedIndex, {});

                                        game.board[clickedIndex] = game.board[selectedIndex];
                                        game.board[selectedIndex] = EMPTY;
                                        moved = true;
                                    }
                                }
                            }

                            if (moved) {
                                if (currentTurn == P1 && clickedIndex / 10 == 9) game.board[clickedIndex] = P1_KING;
                                if (currentTurn == P2 && clickedIndex / 10 == 0) game.board[clickedIndex] = P2_KING;
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
        }

        if (currentState == PLAYING && numPlayers == 1 && currentTurn == P2) {
            if (aiTimer.getElapsedTime().asSeconds() > 0.6f) performAIMove();
        }

        window.clear(sf::Color(45, 40, 35));

        if (currentState == MENU) {
            if (hasMenuBg) {
                window.draw(menuBgSprite);
                sf::RectangleShape dimOverlay(sf::Vector2f(800.0f, 800.0f));
                dimOverlay.setFillColor(sf::Color(0, 0, 0, 140));
                window.draw(dimOverlay);
            }
            window.draw(titleText);
            window.draw(btn1P); if (!hasTex1P) window.draw(text1P);
            window.draw(btn2P); if (!hasTex2P) window.draw(text2P);
            window.draw(btnStart); if (!hasTexStart) window.draw(textStart);
        }
        else if (currentState == PLAYING || currentState == GAMEOVER) {
            for (int i = 0; i < TOTAL_TILES; i++) {
                sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                rect.setPosition((i % 10) * TILE_SIZE, (i / 10) * TILE_SIZE);

                if (((i / 10 + i % 10) % 2 != 0)) rect.setFillColor(sf::Color(139, 69, 19));
                else rect.setFillColor(sf::Color(245, 222, 179));

                if (selectedIndex != -1 && currentState == PLAYING) {
                    for (auto& m : availableCaptures) {
                        for (int pathIdx : m.pathIndices) if (i == pathIdx) rect.setFillColor(sf::Color(0, 255, 255, 120));
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

                    if (i == selectedIndex && currentState == PLAYING) {
                        circle.setOutlineThickness(4);
                        circle.setOutlineColor(sf::Color::Yellow);
                    }
                    window.draw(circle);

                    // --- วาดหมายเลข ID กำกับบนหมาก ---
                    int currentID = game.pieceIdBoard[i];
                    if (currentID != 0) {
                        sf::Text idText(std::to_string(currentID), font, 18);
                        idText.setFillColor(sf::Color::White);
                        sf::FloatRect textBounds = idText.getLocalBounds();
                        idText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
                        idText.setPosition((i % 10) * TILE_SIZE + TILE_SIZE / 2.0f, (i / 10) * TILE_SIZE + TILE_SIZE / 2.0f);
                        window.draw(idText);
                    }
                }
            }

            if (currentState == GAMEOVER) {
                sf::RectangleShape overlay(sf::Vector2f(800.0f, 800.0f));
                overlay.setFillColor(sf::Color(0, 0, 0, 200));
                window.draw(overlay);

                if (hasEndgameImage) window.draw(endgameSprite);
                else {
                    sf::Text text;
                    text.setFont(font);
                    text.setString("GAME OVER\nClick center to Return to Menu");
                    text.setCharacterSize(40);
                    text.setFillColor(sf::Color::White);
                    sf::FloatRect textBounds = text.getLocalBounds();
                    text.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
                    text.setPosition(400.0f, 400.0f);
                    window.draw(text);
                }
            }
        }
        window.display();
    }
    return 0;
}