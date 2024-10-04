#include <iostream>
#include <string>
#include <vector>
#include <cmath>

const int BOARD_WIDTH = 80;
const int BOARD_HEIGHT = 80;
const int FIGURE_SCALE = 2.0;

struct Board {
    std::vector<std::vector<char>> grid;
    Board() : grid(BOARD_HEIGHT, std::vector<char>(BOARD_WIDTH, ' ')) {}

    void print() {
        for (auto& row : grid) {
            for (char c : row) {
                std::cout << c;
            }
            std::cout << "\n";
        }
    }

    void clear() {
        grid.assign(BOARD_HEIGHT, std::vector<char>(BOARD_WIDTH, ' '));
    }
};

struct Shape {
    virtual void draw(Board& board, int x, int y) = 0;
    virtual ~Shape() = default;
};

struct Circle : public Shape {
    int radius;

    Circle(int r) : radius(r) {}

    void draw(Board& board, int X, int Y) override {
        if (radius <= 0) return;

        for (int y = -radius; y <= radius; ++y) {
            for (int x = -radius; x <= radius; ++x) {
                float correctY = y * FIGURE_SCALE;
                float distance = sqrt(x * x + correctY * correctY);
                if (std::abs(distance - radius) <= 0.5) {
                    int drawnX = X + x;
                    int drawnY = Y + y;
                    if (drawnX >= 0 && drawnX < BOARD_WIDTH && drawnY >= 0 && drawnY < BOARD_HEIGHT) {
                        board.grid[drawnY][drawnX] = '*';
                    }
                }
            }
        }
    }
};

int main() {
    Board board;
    std::string command;

    while (true) {
        std::cout << "> ";
        std::cin >> command;

        if (command == "draw") {
            board.print();
        }
        else if (command == "circle") {
            int x, y, radius;
            std::cout << "Enter the location of the circle, and its radius: ";
            std::cin >> x >> y >> radius;

            Circle circle(radius);
            circle.draw(board, x, y);
        }
        else if (command == "exit") {
            break;
        }
    }

    return 0;
}
