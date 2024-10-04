#include <iostream>
#include <string>
#include <vector>
#include <cmath>

const int BOARD_WIDTH = 80;
const int BOARD_HEIGHT = 80;
const int FIGURE_SCALE = 2;

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

struct Information {
    int id;
    std::string type;
    int x, y;
    int width, height;

    Information(int id, std::string type, int x, int y, int dim1, int dim2 = 0)
        : id(id), type(type), x(x), y(y), width(dim1), height(dim2) {}
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

struct Square : public Shape {
    int side_length;

    Square(int side) : side_length(side) {}

    void draw(Board& board, int X, int Y) override {
        if (side_length <= 0) return;

        for (int y = 0; y < side_length; ++y) {
            float correctY = y / FIGURE_SCALE;

            for (int x = 0; x < side_length; ++x) {
                if (y == 0 || y == side_length - 1 || x == 0 || x == side_length - 1) {
                    int drawnX = X + x;
                    int drawnY = Y + static_cast<int>(correctY);
                    if (drawnX >= 0 && drawnX < BOARD_WIDTH && drawnY >= 0 && drawnY < BOARD_HEIGHT) {
                        board.grid[drawnY][drawnX] = '*';
                    }
                }
            }
        }
    }
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

            if (posY < BOARD_HEIGHT) {
                if (left >= 0 && left < BOARD_WIDTH)
                    board.grid[posY][left] = '*';
                if (right >= 0 && right < BOARD_WIDTH && left != right)
                    board.grid[posY][right] = '*';
            }
        }
        for (int j = 0; j < 2 * height - 1; ++j) {
            int baseX = x - height + 1 + j;
            int baseY = y + height - 1;
            if (baseX >= 0 && baseX < BOARD_WIDTH && baseY < BOARD_HEIGHT)
                board.grid[baseY][baseX] = '*';
        }
    }
};

int main() {
    Board board;
    std::string command;
    std::vector<Information> shapes_info;
    std::vector<Shape*> shapes;
    int shape_id = 1;

    while (true) {
        std::cout << "> ";
        std::cin >> command;

        if (command == "draw") {
            board.print();
        }
        else if (command == "triangle") {
            int x, y, height;
            std::cout << "Enter the location of the triangle, and its height: ";
            std::cin >> x >> y >> height;

            Triangle* triangle = new Triangle(height);
            triangle->draw(board, x, y);
            shapes.push_back(triangle);
            shapes_info.emplace_back(shape_id++, "triangle", x, y, height);
        }
        else if (command == "circle") {
            int x, y, radius;
            std::cout << "Enter the location of the circle, and its radius: ";
            std::cin >> x >> y >> radius;

            Circle* circle = new Circle(radius);
            circle->draw(board, x, y);
            shapes.push_back(circle);
            shapes_info.emplace_back(shape_id++, "circle", x, y, radius);
        }
        else if (command == "square") {
            int x, y, side_length;
            std::cout << "Enter the location of the square, and its side length: ";
            std::cin >> x >> y >> side_length;

            Square* square = new Square(side_length);
            square->draw(board, x, y);
            shapes.push_back(square);
            shapes_info.emplace_back(shape_id++, "square", x, y, side_length, side_length);
        }
        else if (command == "shapes") {
            std::cout << "circle coordinates radius\n";
            std::cout << "square coordinates side size\n";
            std::cout << "triangle coordinates height\n";
        }
        else if (command == "list") {
            for (const auto& info : shapes_info) {
                if (info.type == "circle") {
                    std::cout << "> " << info.id << " " << info.type << " radius: " << info.width << "\n";
                    std::cout << "coordinates: (" << info.x << ", " << info.y << ")\n";
                }
                else if (info.type == "square") {
                    std::cout << "> " << info.id << " " << info.type << " width: " << info.width << " height: " << info.height << "\n";
                    std::cout << "coordinates: (" << info.x << ", " << info.y << ")\n";
                }
                else if (info.type == "triangle") {
                    std::cout << "> " << info.id << " " << info.type << " height: " << info.width << "\n";
                    std::cout << "coordinates: (" << info.x << ", " << info.y << ")\n";
                }
            }
        }
        else if (command == "exit") {
            break;
        }
    }
    for (auto& shape : shapes) {
        delete shape;
    }

    return 0;
}
