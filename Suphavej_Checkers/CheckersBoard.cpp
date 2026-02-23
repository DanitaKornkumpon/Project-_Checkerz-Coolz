#include "CheckersBoard.h"

// Constructor
CheckersBoard::CheckersBoard()
{
    board.resize(TOTAL_TILES, EMPTY);
    initialize();
}

// Initialize board with 20 pieces per player
void CheckersBoard::initialize()
{
    // Top 4 rows = Player 1
    for (int i = 0; i < 40; i++)
    {
        int row = i / BOARD_SIZE;
        int col = i % BOARD_SIZE;

        if ((row + col) % 2 == 1)
            board[i] = P1;
    }

    // Bottom 4 rows = Player 2
    for (int i = 60; i < 100; i++)
    {
        int row = i / BOARD_SIZE;
        int col = i % BOARD_SIZE;

        if ((row + col) % 2 == 1)
            board[i] = P2;
    }
}

// Get piece at index
int CheckersBoard::getPiece(int index) const
{
    return board[index];
}

// Set piece at index
void CheckersBoard::setPiece(int index, int piece)
{
    board[index] = piece;
}