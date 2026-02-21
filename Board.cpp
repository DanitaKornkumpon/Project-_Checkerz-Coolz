#include <iostream>
#include <vector>
#include <cstdlib> // สำหรับ rand()
#include <ctime>   // สำหรับ time()

// กำหนดขนาดกระดาน 10x10
const int BOARD_SIZE = 10;
const int TOTAL_TILES = 100;

// ประเภทของสิ่งที่อยู่บนกระดาน
enum PieceType {
    EMPTY = 0,
    P1 = 1,          // หมากผู้เล่น 1 (O)
    P2 = 2,          // หมากผู้เล่น 2 (X)
    P1_KING = 3,     // ฮอสผู้เล่น 1
    P2_KING = 4,     // ฮอสผู้เล่น 2
    OBSTACLE = 5,    // ศัตรูที่เป็นกลาง (#)
    WALL = 99        // ขอบกระดาน
};

struct PieceInfo {
    int id;
    int movesForward;
    int movesBackward;
    int turnLeft;
    int turnRight;
};

class CheckersBoard {
private:
    std::vector<int> board;
    std::vector<PieceInfo> p1Stats;
    std::vector<PieceInfo> p2Stats;

public:
    CheckersBoard() {
        // กำหนดขนาดกระดานและเคลียร์ค่าเริ่มต้น
        board.resize(TOTAL_TILES, EMPTY);
        // เริ่มต้นระบบสุ่ม
        std::srand(static_cast<unsigned int>(std::time(0)));
        initializeBoard();
    }

    void initializeBoard() {
        // วางหมาก P1 (4 แถวบน - ช่องสีเข้ม)
        for (int i = 0; i < 40; i++) {
            int row = i / 10;
            int col = i % 10;
            // สูตรเช็คช่องสีเข้ม (Dark square)
            if ((row % 2 == 0 && col % 2 == 1) || (row % 2 == 1 && col % 2 == 0)) {
                board[i] = P1;
            }
        }

        // วางหมาก P2 (4 แถวล่าง - ช่องสีเข้ม)
        for (int i = 60; i < 100; i++) {
            int row = i / 10;
            int col = i % 10;
            if ((row % 2 == 0 && col % 2 == 1) || (row % 2 == 1 && col % 2 == 0)) {
                board[i] = P2;
            }
        }
    }

    // ฟังก์ชันนี้แหละครับที่เคยหายไป!
    void spawnRandomObstacle() {
        // สุ่มโอกาสเกิด 20%
        if (std::rand() % 100 < 20) {
            int randomIdx;
            int attempts = 0;
            do {
                randomIdx = std::rand() % TOTAL_TILES;
                attempts++;
                // ป้องกัน Infinite loop กรณีบอร์ดเต็ม
                if (attempts > 200) return;

                // เช็คว่าเป็นช่องสีเข้มและว่างอยู่ไหม
                int row = randomIdx / 10;
                int col = randomIdx % 10;
                bool isDarkSquare = (row % 2 == 0 && col % 2 == 1) || (row % 2 == 1 && col % 2 == 0);

                if (isDarkSquare && board[randomIdx] == EMPTY) {
                    board[randomIdx] = OBSTACLE;
                    std::cout << "\n[ALERT] A neutral enemy appeared at index " << randomIdx << "!\n";
                    break;
                }
            } while (true);
        }
    }

    void printBoard() {
        std::cout << "   0 1 2 3 4 5 6 7 8 9" << std::endl;
        std::cout << "  --------------------" << std::endl;
        for (int i = 0; i < BOARD_SIZE; ++i) {
            std::cout << i << "| ";
            for (int j = 0; j < BOARD_SIZE; ++j) {
                int index = i * BOARD_SIZE + j;
                char symbol = '.'; // ช่องว่างสีขาว

                // เช็คช่องสีเข้ม
                if ((i % 2 == 0 && j % 2 == 1) || (i % 2 == 1 && j % 2 == 0)) {
                    symbol = '_'; // ช่องว่างสีดำ
                }

                if (board[index] == P1) symbol = 'O';
                else if (board[index] == P2) symbol = 'X';
                else if (board[index] == OBSTACLE) symbol = '#'; // ศัตรู

                std::cout << symbol << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "  --------------------" << std::endl;
    }
};

int main() {
    CheckersBoard game;
    char input;

    // จำลอง Game Loop ง่ายๆ
    while (true) {
        // เคลียร์หน้าจอ (ใช้ได้เฉพาะ Windows)
        system("cls");

        std::cout << "=== Checkers 10x10 with Obstacles ===\n";
        std::cout << "O = Player 1 | X = Player 2 | # = Enemy\n\n";

        game.printBoard();
        game.spawnRandomObstacle(); // สุ่มเกิดอุปสรรค

        std::cout << "\nPress Enter to simulate next turn (or 'q' to quit)...";
        input = std::cin.get();
        if (input == 'q') break;
    }

    return 0;
}
