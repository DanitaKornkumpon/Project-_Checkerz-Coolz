#include <SFML/Graphics.hpp>
#include "CheckersBoard.h"
#include "GameLogic.h"

const float TILE_SIZE = 80.0f;

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 800), "Checkers 10x10");

    CheckersBoard board;
    int currentPlayer = P1;
    int selectedIndex = -1;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                int col = static_cast<int>(event.mouseButton.x / TILE_SIZE);
                int row = static_cast<int>(event.mouseButton.y / TILE_SIZE);
                int index = row * BOARD_SIZE + col;

                if (board.getPiece(index) == currentPlayer)
                {
                    selectedIndex = index;
                }
                else if (selectedIndex != -1)
                {
                    auto moves = GameLogic::getAllValidMoves(board, currentPlayer);

                    for (auto& m : moves)
                    {
                        if (m.from == selectedIndex && m.to == index)
                        {
                            GameLogic::applyMove(board, m);
                            currentPlayer = (currentPlayer == P1 ? P2 : P1);
                            break;
                        }
                    }

                    selectedIndex = -1;
                }
            }
        }

        window.clear();

        for (int row = 0; row < BOARD_SIZE; row++)
        {
            for (int col = 0; col < BOARD_SIZE; col++)
            {
                int index = row * BOARD_SIZE + col;

                sf::RectangleShape square(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                square.setPosition(col * TILE_SIZE, row * TILE_SIZE);

                if ((row + col) % 2 == 1)
                    square.setFillColor(sf::Color(139, 69, 19));
                else
                    square.setFillColor(sf::Color(245, 222, 179));

                window.draw(square);

                int piece = board.getPiece(index);
                if (piece != EMPTY)
                {
                    sf::CircleShape checker(TILE_SIZE / 2 - 8);
                    checker.setPosition(col * TILE_SIZE + 8,
                        row * TILE_SIZE + 8);

                    if (piece == P1)
                        checker.setFillColor(sf::Color::Red);
                    else if (piece == P2)
                        checker.setFillColor(sf::Color::Green);

                    if (index == selectedIndex)
                    {
                        checker.setOutlineThickness(4);
                        checker.setOutlineColor(sf::Color::Yellow);
                    }

                    window.draw(checker);
                }
            }
        }

        window.display();
    }

    return 0;
}