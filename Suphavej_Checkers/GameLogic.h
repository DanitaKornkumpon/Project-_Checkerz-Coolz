#pragma once
#include <vector>
#include "CheckersBoard.h"

struct Move
{
    int from;
    int to;
    int captured = -1;
};

struct MoveStats
{
    int forward = 0;
    int backward = 0;
    int left = 0;
    int right = 0;
};

class GameLogic
{
public:
    static std::vector<Move> getAllValidMoves(const CheckersBoard& board, int player);
    static void applyMove(CheckersBoard& board, const Move& move);

    static void printStats();

private:
    static MoveStats stats;
    static void trackMoveDirection(int from, int to);
};