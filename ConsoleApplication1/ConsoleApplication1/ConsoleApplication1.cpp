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
    char color;

    Information(int id, std::string type, int x, int y, int dim1, int dim2 = 0, char color = '*')
        : id(id), type(type), x(x), y(y), width(dim1), height(dim2), color(color) {}
};

struct Shape {
    virtual void draw(Board& board, int x, int y, char color) = 0;
    virtual ~Shape() = default;
    virtual bool Fits(int x, int y) = 0;
    virtual bool Duplicate(const Information& info) = 0;
};

struct Circle : public Shape {
    int radius;

    Circle(int r) : radius(r) {}

    void draw(Board& board, int X, int Y, char color) override {
        if (radius <= 0) return;

        for (int y = -radius; y <= radius; ++y) {
            for (int x = -radius; x <= radius; ++x) {
                float correctY = y * FIGURE_SCALE;
                float distance = sqrt(x * x + correctY * correctY);
                if (std::abs(distance - radius) <= 0.5) {
                    int drawnX = X + x;
                    int drawnY = Y + y;
                    if (drawnX >= 0 && drawnX < BOARD_WIDTH && drawnY >= 0 && drawnY < BOARD_HEIGHT) {
                        board.grid[drawnY][drawnX] = color;
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

    void draw(Board& board, int X, int Y, char color) override {
        if (side_length <= 0) return;

        for (int y = 0; y < side_length; ++y) {
            float correctY = y / FIGURE_SCALE;

            for (int x = 0; x < side_length; ++x) {
                if (y == 0 || y == side_length - 1 || x == 0 || x == side_length - 1) {
                    int drawnX = X + x;
                    int drawnY = Y + static_cast<int>(correctY);
                    if (drawnX >= 0 && drawnX < BOARD_WIDTH && drawnY >= 0 && drawnY < BOARD_HEIGHT) {
                        board.grid[drawnY][drawnX] = color;
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

    void draw(Board& board, int x, int y, char color) override {
        if (height <= 0) return;
        for (int i = 0; i < height; ++i) {
            int left = x - i;
            int right = x + i;
            int posY = y + i;

            if (posY < BOARD_HEIGHT) {
                if (left >= 0 && left < BOARD_WIDTH)
                    board.grid[posY][left] = color;
                if (right >= 0 && right < BOARD_WIDTH && left != right)
                    board.grid[posY][right] = color;
            }
        }
        for (int j = 0; j < 2 * height - 1; ++j) {
            int baseX = x - height + 1 + j;
            int baseY = y + height - 1;
            if (baseX >= 0 && baseX < BOARD_WIDTH && baseY < BOARD_HEIGHT)
                board.grid[baseY][baseX] = color;
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
        file << info.id << " " << info.type << " " << info.x << " " << info.y << " " << info.width << " " << info.height << " " << info.color << "\n";
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
    char color;
    std::string type;
    while (file >> id >> type >> x >> y >> dim1 >> dim2 >> color) {
        if (type == "circle") {
            Circle* circle = new Circle(dim1);
            circle->draw(board, x, y, color);
            shapes.push_back(circle);
            shapes_info.emplace_back(id, type, x, y, dim1, dim2, color);
        }
        else if (type == "square") {
            Square* square = new Square(dim1);
            square->draw(board, x, y, color);
            shapes.push_back(square);
            shapes_info.emplace_back(id, type, x, y, dim1, dim2, color);
        }
        else if (type == "triangle") {
            Triangle* triangle = new Triangle(dim1);
            triangle->draw(board, x, y, color);
            shapes.push_back(triangle);
            shapes_info.emplace_back(id, type, x, y, dim1, dim2, color);
        }
        shape_id = std::max(shape_id, id + 1);
    }

    file.close();
}

char Color(const std::string& color) {
    if (color == "red") return 'R';
    if (color == "blue") return 'B';
    if (color == "green") return 'G';
    std::cerr << "This color is absent\n";
    return '*';
}

void Edit(Board& board, std::vector<Shape*>& shapes, std::vector<Information>& shapes_info) {
    int id;
    std::cout << "Enter the ID of the shape you want to edit: ";
    std::cin >> id;

    auto info_loop = std::find_if(shapes_info.begin(), shapes_info.end(), [id](const Information& info) {
        return info.id == id;
        });

    if (info_loop == shapes_info.end()) {
        std::cout << "This shape was not found";
        return;
    }

    Information& info = *info_loop;

    std::cout << "1. Type of the figure: " << info.type << "\n";
    std::cout << "2. X coordinate: " << info.x << "\n";
    std::cout << "3. Y coordinate: " << info.y << "\n";
    std::cout << "4. Width of a figure " << info.width << "\n";
    if (info.type != "circle") {
        std::cout << "5. Height of the figure: " << info.height << "\n";
    }
    std::cout << "6. Color of the figure: " << info.color << "\n";

    int property;
    std::cout << "Which property do you want to edit? ";
    std::cin >> property;

    switch (property) {
    case 2: {
        int updatedX;
        std::cout << "Enter new X coordinate for a figure: ";
        std::cin >> updatedX;

        if (updatedX >= 0 && updatedX + info.width < BOARD_WIDTH && (info.type != "circle" || info.height == 0)) {
            info.x = updatedX;
        }
        else {
            std::cout << "Wrong coordinate\n";
        }
        break;
    }
    case 3: {
        int updatedY;
        std::cout << "Enter new Y coordinate for a figure: ";
        std::cin >> updatedY;

        if (updatedY >= 0 && updatedY + info.height < BOARD_HEIGHT) {
            info.y = updatedY;
        }
        else {
            std::cout << "Wrong coordinate\n";
        }
        break;
    }
    case 4: {
        int updatedDim;
        std::cout << "Enter new width of a figure: ";
        std::cin >> updatedDim;

        if (updatedDim > 0) {
            info.width = updatedDim;
        }
        else {
            std::cout << "Wrong height\n";
        }
        break;
    }
    case 5: {
        if (info.type != "circle") {
            int updatedHeight;
            std::cout << "Enter new height of a figure ";
            std::cin >> updatedHeight;

            if (updatedHeight > 0) {
                info.height = updatedHeight;
            }
            else {
                std::cout << "Invalid Height\n";
            }
        }
        else {
            std::cout << "Circles don't have height\n";
        }
        break;
    }
    case 6: {
        std::string updatedColor;
        std::cout << "Enter new color (red, blue, green): ";
        std::cin >> updatedColor;

        char newCharColor = Color(updatedColor);
        if (newCharColor != '*') {
            info.color = newCharColor;
        }
        else {
            std::cout << "Unsupported color\n";
        }
        break;
    }
    default:
        std::cout << "You did not choose a correct property\n";
        return;
    }

    board.clear();
    for (size_t i = 0; i < shapes.size(); ++i) {
        shapes[i]->draw(board, shapes_info[i].x, shapes_info[i].y, shapes_info[i].color);
    }

    std::cout << "This shape was updated";
}

void Move(Board& board, std::vector<Shape*>& shapes, std::vector<Information>& shapes_info) {
    int id, updatedX, updatedY;
    std::cout << "Enter the ID of the shape you want to move: ";
    std::cin >> id;

    auto it_info = std::find_if(shapes_info.begin(), shapes_info.end(), [id](const Information& info) {
        return info.id == id;
        });

    if (it_info == shapes_info.end()) {
        std::cout << "This shape was not found \n";
        return;
    }

    Information& info = *it_info;

    std::cout << "Enter new coordinates for the shape: ";
    std::cin >> updatedX >> updatedY;

    bool canMove = false;
    if (info.type == "circle") {
        Circle temp_circle(info.width);
        canMove = temp_circle.Fits(updatedX, updatedY);
    }
    else if (info.type == "square") {
        Square temp_square(info.width);
        canMove = temp_square.Fits(updatedX, updatedY);
    }
    else if (info.type == "triangle") {
        Triangle temp_triangle(info.width);
        canMove = temp_triangle.Fits(updatedX, updatedY);
    }

    if (!canMove) {
        std::cout << "You cannot place this shape outside of the board \n";
        return;
    }

    for (const auto& other_info : shapes_info) {
        if (other_info.id != id && other_info.x == updatedX && other_info.y == updatedY) {
            std::cout << "Another shape is already placed here \n";
            return;
        }
    }

    info.x = updatedX;
    info.y = updatedY;

    board.clear();
    for (size_t i = 0; i < shapes.size(); ++i) {
        shapes[i]->draw(board, shapes_info[i].x, shapes_info[i].y, shapes_info[i].color);
    }
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
            std::string color;
            std::cout << "Enter the location of the triangle, its height, and its color: ";
            std::cin >> x >> y >> height >> color;
            Shape* triangle = new Triangle(height);
            if (PlaceShape(x, y, triangle, shapes_info, "triangle", height)) {
                triangle->draw(board, x, y, Color(color));
                shapes_info.emplace_back(shape_id++, "triangle", x, y, height, 0, Color(color));
                shapes.push_back(triangle);
            }
            else {
                delete triangle;
            }
        }
        else if (command == "circle") {
            int x, y, radius;
            std::string color;
            std::cout << "Enter the location of the circle, its radius, and its color: ";
            std::cin >> x >> y >> radius >> color;
            Shape* circle = new Circle(radius);
            if (PlaceShape(x, y, circle, shapes_info, "circle", radius)) {
                circle->draw(board, x, y, Color(color));
                shapes_info.emplace_back(shape_id++, "circle", x, y, radius, 0, Color(color));
                shapes.push_back(circle);
            }
            else {
                delete circle;
            }
        }
        else if (command == "square") {
            int x, y, side;
            std::string color;
            std::cout << "Enter the location of the square, its side length, and its color: ";
            std::cin >> x >> y >> side >> color;
            Shape* square = new Square(side);
            if (PlaceShape(x, y, square, shapes_info, "square", side)) {
                square->draw(board, x, y, Color(color));
                shapes_info.emplace_back(shape_id++, "square", x, y, side, 0, Color(color));
                shapes.push_back(square);
            }
            else {
                delete square;
            }
        }
        else if (command == "remove") {
            int id;
            std::cout << "Enter the ID of the shape to remove: ";
            std::cin >> id;

            auto it_info = std::find_if(shapes_info.begin(), shapes_info.end(), [id](const Information& info) {
                return info.id == id;
                });

            if (it_info != shapes_info.end()) {
                int index = std::distance(shapes_info.begin(), it_info);
                delete shapes[index];
                shapes.erase(shapes.begin() + index);
                shapes_info.erase(it_info);

                board.clear();
                for (size_t i = 0; i < shapes.size(); ++i) {
                    shapes[i]->draw(board, shapes_info[i].x, shapes_info[i].y, shapes_info[i].color);
                }
                std::cout << "Shape removed.\n";
            }
            else {
                std::cout << "No shape with ID " << id << " found.\n";
            }
        }
        else if (command == "paint") {
            int id;
            std::string color;
            std::cout << "Enter shape's ID and the new color: ";
            std::cin >> id >> color;

            char new_color = Color(color);
            bool found = false;

            for (auto& info : shapes_info) {
                if (info.id == id) {
                    found = true;
                    info.color = new_color;
                    board.clear();
                    for (size_t i = 0; i < shapes.size(); ++i) {
                        shapes[i]->draw(board, shapes_info[i].x, shapes_info[i].y, shapes_info[i].color);
                    }
                    break;
                }
            }

            if (!found) {
                std::cout << "Shape with ID " << id << " not found.\n";
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
            for (auto& shape : shapes) {
                delete shape;
            }
            shapes.clear();
            shapes_info.clear();
        }
        else if (command == "undo") {
            if (!shapes.empty()) {
                board.clear();
                delete shapes.back();
                shapes.pop_back();
                shapes_info.pop_back();

                for (size_t i = 0; i < shapes.size(); ++i) {
                    const auto& info = shapes_info[i];
                    Shape* shape = shapes[i];
                    shape->draw(board, info.x, info.y, Color("green"));
                }
            }
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
        else if (command == "shapes") {
            std::cout << "circle coordinates radius\n";
            std::cout << "square coordinates side size\n";
            std::cout << "triangle coordinates height\n";
        }
        else if (command == "select") {
            int id;
            std::cout << "Enter ID of the figure you want to check: ";
            std::cin >> id;

            bool found = false;

            for (const auto& info : shapes_info) {
                if (info.id == id) {
                    found = true;
                    std::cout << info.type << " " << info.x << " " << info.y << " " << info.width << " ";
                    if (info.type != "circle") {
                        std::cout << info.height;
                    }
                    std::cout << info.color;
                    break;
                }
            }

            if (!found) {
                std::cout << "Could not find this figure";
            }
        }
        else if (command == "edit") {
            Edit(board, shapes, shapes_info);
        }
        else if (command == "move") {
            Move(board, shapes, shapes_info);
        }
        else if (command == "exit") {
            break;
        }
    }

    for (auto shape : shapes) {
        delete shape;
    }

    return 0;
}
