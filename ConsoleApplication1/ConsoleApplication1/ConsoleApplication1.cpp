#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>

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
    virtual bool Fits(int x, int y) = 0;
    virtual bool Duplicate(const Information& info) = 0;
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

    bool Fits(int x, int y) override {
        return x - radius >= 0 && x + radius < BOARD_WIDTH && y - radius / FIGURE_SCALE >= 0 && y + radius / FIGURE_SCALE < BOARD_HEIGHT;
    }

    bool Duplicate(const Information& info) override {
        return info.type == "circle" && info.width == radius;
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

    bool Fits(int x, int y) override {
        return x >= 0 && x + side_length < BOARD_WIDTH && y >= 0 && y + side_length / FIGURE_SCALE < BOARD_HEIGHT;
    }

    bool Duplicate(const Information& info) override {
        return info.type == "square" && info.width == side_length;
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

    bool Fits(int x, int y) override {
        return x - height >= 0 && x + height < BOARD_WIDTH && y >= 0 && y + height < BOARD_HEIGHT;
    }

    bool Duplicate(const Information& info) override {
        return info.type == "triangle" && info.width == height;
    }
};

bool PlaceShape(int x, int y, Shape* shape, const std::vector<Information>& shapes_info, const std::string& type, int dim1, int dim2 = 0) {
    if (!shape->Fits(x, y)) {
        std::cout << "Shape doesn't fit on the board.\n";
        return false;
    }

    for (const auto& info : shapes_info) {
        if (info.x == x && info.y == y && info.type == type && info.width == dim1 && info.height == dim2) {
            std::cout << "Shape with the same type and parameters already exists at this location.\n";
            return false;
        }
    }
    return true;
}

void saveToFile(const std::string& filename, const std::vector<Information>& shapes_info) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Could not save the file";
        return;
    }

    for (const auto& info : shapes_info) {
        file << info.id << " " << info.type << " " << info.x << " " << info.y << " " << info.width << " " << info.height << "\n";
    }

    file.close();
}

void loadFromFile(const std::string& filename, Board& board, std::vector<Shape*>& shapes, std::vector<Information>& shapes_info, int& shape_id) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Could not open the file";
        return;
    }

    shapes_info.clear();
    for (auto& shape : shapes) {
        delete shape;
    }
    shapes.clear();
    board.clear();

    int id, x, y, dim1, dim2;
    std::string type;
    while (file >> id >> type >> x >> y >> dim1 >> dim2) {
        if (type == "circle") {
            Circle* circle = new Circle(dim1);
            circle->draw(board, x, y);
            shapes.push_back(circle);
            shapes_info.emplace_back(id, type, x, y, dim1);
        }
        else if (type == "square") {
            Square* square = new Square(dim1);
            square->draw(board, x, y);
            shapes.push_back(square);
            shapes_info.emplace_back(id, type, x, y, dim1, dim2);
        }
        else if (type == "triangle") {
            Triangle* triangle = new Triangle(dim1);
            triangle->draw(board, x, y);
            shapes.push_back(triangle);
            shapes_info.emplace_back(id, type, x, y, dim1);
        }
        shape_id = std::max(shape_id, id + 1);
    }

    file.close();
}

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
            if (PlaceShape(x, y, triangle, shapes_info, "triangle", height)) {
                triangle->draw(board, x, y);
                shapes.push_back(triangle);
                shapes_info.emplace_back(shape_id++, "triangle", x, y, height);
            }
            else {
                delete triangle;
            }
        }
        else if (command == "circle") {
            int x, y, radius;
            std::cout << "Enter the location of the circle, and its radius: ";
            std::cin >> x >> y >> radius;

            Circle* circle = new Circle(radius);
            if (PlaceShape(x, y, circle, shapes_info, "circle", radius)) {
                circle->draw(board, x, y);
                shapes.push_back(circle);
                shapes_info.emplace_back(shape_id++, "circle", x, y, radius);
            }
            else {
                delete circle;
            }
        }
        else if (command == "square") {
            int x, y, side;
            std::cout << "Enter the location of the square, and its side length: ";
            std::cin >> x >> y >> side;

            Square* square = new Square(side);
            if (PlaceShape(x, y, square, shapes_info, "square", side)) {
                square->draw(board, x, y);
                shapes.push_back(square);
                shapes_info.emplace_back(shape_id++, "square", x, y, side);
            }
            else {
                delete square;
            }
        }
        else if (command == "save") {
            std::string filename;
            std::cout << "Enter the filename: ";
            std::cin >> filename;
            saveToFile(filename, shapes_info);
        }
        else if (command == "load") {
            std::string filename;
            std::cout << "Enter the filename: ";
            std::cin >> filename;
            loadFromFile(filename, board, shapes, shapes_info, shape_id);
        }
        else if (command == "clear") {
            board.clear();
        }
        else if (command == "exit") {
            break;
        }
        else {
            std::cout << "Invalid command.\n";
        }
    }

    for (auto& shape : shapes) {
        delete shape;
    }

    return 0;
}
