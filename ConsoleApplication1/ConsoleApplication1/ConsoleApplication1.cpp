#include <iostream>
#include <string>
#include <vector>

const int BOARD_WIDTH = 80;
const int BOARD_HEIGHT = 25;

struct Board {
    std::vector<std::vector<char>> grid;
    Board() : grid(25, std::vector<char>(80, ' ')) {}

    void print() {
        for (auto& row : grid) {
            for (char c : row) {
                std::cout << c;
            }
            std::cout << "\n";
        }
    }

    void clear() {
        grid.assign(25, std::vector<char>(80, ' '));
    }
};

struct Shape {
    virtual void draw(Board& board, int x, int y) = 0;
    virtual ~Shape() = default;
};

struct Triangle : public Shape {
    int height;

    Triangle(int h) : height(h) {}

    void draw(Board& board, int x, int y) override {
        if (height <= 0) return;
        for (int i = 0; i < height; ++i) {
            int left = x - i;
            int right = x + i;
            int posY = y + i;

            if (posY < 25) {
                if (left >= 0 && left < 80)
                    board.grid[posY][left] = '*';
                if (right >= 0 && right < 80 && left != right)
                    board.grid[posY][right] = '*';
            }
        }
        for (int j = 0; j < 2 * height - 1; ++j) {
            int baseX = x - height + 1 + j;
            int baseY = y + height - 1;
            if (baseX >= 0 && baseX < 80 && baseY < 25)
                board.grid[baseY][baseX] = '*';
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
        else if (command == "triangle") {
            int x, y, height;
            std::cout << "Enter the location of the triangle, and its height";
            std::cin >> x >> y >> height;

            Triangle triangle(height);
            triangle.draw(board, x, y);
        }
        else if (command == "clear") {
            board.clear();
        }
        else if (command == "exit") {
            break;
        }
    }

    return 0;
}
