#include "GameLogic.h"
#include <cmath>
#include <iostream>

MoveStats GameLogic::stats;

// ----------------------------------------
// Track movement direction
// ----------------------------------------
void GameLogic::trackMoveDirection(int from, int to)
{
    int fromRow = from / BOARD_SIZE;
    int fromCol = from % BOARD_SIZE;
    int toRow = to / BOARD_SIZE;
    int toCol = to % BOARD_SIZE;

    int rowDiff = toRow - fromRow;
    int colDiff = toCol - fromCol;

    if (rowDiff > 0)
        stats.forward++;
    else if (rowDiff < 0)
        stats.backward++;

    if (colDiff > 0)
        stats.right++;
    else if (colDiff < 0)
        stats.left++;
}

// ----------------------------------------
// Get all valid moves for player
// ----------------------------------------
std::vector<Move> GameLogic::getAllValidMoves(const CheckersBoard& board, int player)
{
    std::vector<Move> moves;

    for (int i = 0; i < TOTAL_TILES; i++)
    {
        if (board.getPiece(i) != player)
            continue;

        int row = i / BOARD_SIZE;
        int col = i % BOARD_SIZE;

        int direction = (player == P1) ? 1 : -1;

        // Normal moves
        for (int dc : {-1, 1})
        {
            int newRow = row + direction;
            int newCol = col + dc;

            if (newRow >= 0 && newRow < BOARD_SIZE &&
                newCol >= 0 && newCol < BOARD_SIZE)
            {
                int newIndex = newRow * BOARD_SIZE + newCol;

                if (board.getPiece(newIndex) == EMPTY)
                {
                    moves.push_back({ i, newIndex, -1 });
                }
            }
        }

        // Capture moves
        for (int dc : {-2, 2})
        {
            int newRow = row + 2 * direction;
            int newCol = col + dc;

            if (newRow >= 0 && newRow < BOARD_SIZE &&
                newCol >= 0 && newCol < BOARD_SIZE)
            {
                int midRow = row + direction;
                int midCol = col + dc / 2;

                int midIndex = midRow * BOARD_SIZE + midCol;
                int newIndex = newRow * BOARD_SIZE + newCol;

                int midPiece = board.getPiece(midIndex);

                if (board.getPiece(newIndex) == EMPTY &&
                    midPiece != EMPTY &&
                    midPiece != player)
                {
                    moves.push_back({ i, newIndex, midIndex });
                }
            }
        }
    }

    return moves;
}

// ----------------------------------------
// Apply move
// ----------------------------------------
void GameLogic::applyMove(CheckersBoard& board, const Move& move)
{
    int movingPiece = board.getPiece(move.from);

    // Track direction BEFORE changing board
    trackMoveDirection(move.from, move.to);

    board.setPiece(move.to, movingPiece);
    board.setPiece(move.from, EMPTY);

    if (move.captured != -1)
        board.setPiece(move.captured, EMPTY);

    printStats();
}

// ----------------------------------------
void GameLogic::printStats()
{
    std::cout << "\n==== MOVE STATS ====\n";
    std::cout << "Forward  : " << stats.forward << "\n";
    std::cout << "Backward : " << stats.backward << "\n";
    std::cout << "Left     : " << stats.left << "\n";
    std::cout << "Right    : " << stats.right << "\n";
    std::cout << "====================\n";
}