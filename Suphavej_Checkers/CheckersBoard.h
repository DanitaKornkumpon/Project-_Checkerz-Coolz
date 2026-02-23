#pragma once
#include <vector>

const int BOARD_SIZE = 10;
const int TOTAL_TILES = 100;

enum PieceType
{
    EMPTY = 0,
    P1 = 1,
    P2 = 2
};

class CheckersBoard
{
private:
    std::vector<int> board;

public:
    CheckersBoard();          // declaration only
    void initialize();        // declaration only
    int getPiece(int index) const;
    void setPiece(int index, int piece);
};