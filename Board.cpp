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
    P1 = 1,          // หมากผู้เล่น 1
    P2 = 2,          // หมากผู้เล่น 2
    P1_KING = 3,     // ฮอสผู้เล่น 1 (เดินไกลได้)
    P2_KING = 4,     // ฮอสผู้เล่น 2 (เดินไกลได้)
    OBSTACLE = 5,    // ศัตรูที่เป็นกลาง (Neutral Enemy)
    WALL = 99        // ขอบกระดาน (เพื่อกันการเดินทะลุ - เทคนิค Mailbox)
};

struct PieceInfo {
    int id;             // ID เฉพาะของหมากตัวนั้น (0-19)
    int movesForward;   // นับจำนวนครั้งที่เดินหน้า
    int movesBackward;  // นับจำนวนครั้งที่เดินถอยหลัง
    int turnLeft;       // นับจำนวนครั้งที่เลี้ยวซ้าย
    int turnRight;      // นับจำนวนครั้งที่เลี้ยวขวา
    // ข้อมูลนี้ใช้สำหรับ "ตรวจสอบหมากทั้ง 20 ตัว" ตามที่คุณต้องการ
};

class CheckersBoard {
private:
    // ใช้ vector แทน array เพื่อความยืดหยุ่น แต่ทำงานเหมือน 1D Array
    std::vector<int> board;
    std::vector<PieceInfo> p1Stats; // เก็บสถิติหมาก P1 20 ตัว
    std::vector<PieceInfo> p2Stats; // เก็บสถิติหมาก P2 20 ตัว

public:
    CheckersBoard() {
        // กำหนดขนาดกระดานและเคลียร์ค่า
        board.resize(TOTAL_TILES, EMPTY);
        initializeBoard();
        srand(time(0)); // เริ่มต้นระบบสุ่มสำหรับอุปสรรค
    }

    void initializeBoard() {
        // การวางหมาก 10x10 ปกติหมากจะวางบนช่องสีเข้มเท่านั้น (50 ช่อง)
        // แต่เพื่อความง่ายในโค้ดตัวอย่างนี้ เราจะวางแบบแถวเรียงไปก่อน

        // ผู้เล่น 1 (วาง 4 แถวบน)
        for (int i = 0; i < 40; i++) {
            if ((i / 10) % 2 == 0 ? i % 2 == 1 : i % 2 == 0) { // สูตรช่องสีเข้ม
                board[i] = P1;
                // Init Stat สำหรับหมากแต่ละตัวตรงนี้
            }
        }

        // ผู้เล่น 2 (วาง 4 แถวล่าง)
        for (int i = 60; i < 100; i++) {
            if ((i / 10) % 2 == 0 ? i % 2 == 1 : i % 2 == 0) {
                board[i] = P2;
            }
        }
    }

    void printBoard() {
        std::cout << "  0 1 2 3 4 5 6 7 8 9" << std::endl;
        for (int i = 0; i < BOARD_SIZE; ++i) {
            std::cout << i << " ";
            for (int j = 0; j < BOARD_SIZE; ++j) {
                int index = i * BOARD_SIZE + j;
                char symbol = '.';
                if (board[index] == P1) symbol = 'O';
                else if (board[index] == P2) symbol = 'X';
                else if (board[index] == OBSTACLE) symbol = '#'; // สัญลักษณ์ศัตรู
                std::cout << symbol << " ";
            }
            std::cout << std::endl;
        }
    }

    // ฟังก์ชันเข้าถึงกระดาน (getter)
    int getPiece(int row, int col) {
        if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) return WALL;
        return board[row * BOARD_SIZE + col];
    }

    // ... ฟังก์ชันอื่นๆ ...
};
