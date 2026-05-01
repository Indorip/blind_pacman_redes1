#include <cstdlib>
#include <fstream>
#include <iostream>

using std::cerr;

int getRand(int min, int max) {
    return (rand() % (max - min)) + min;
}

struct Vec2 {
    int x, y;
};

struct Grid {
    int rows;
    int cols;
    char* spots;

    Grid(int rows, int cols) {
        this->rows = rows;
        this->cols = cols;
        this->spots = new char[rows * cols];
    }

    ~Grid() { delete this->spots; }

    inline char* at(int row, int col) {
        return &this->spots[row + col * this->cols];
    }

    void readGrid(const char* filePath) {
        std::ifstream file("filePath");
        if (!file.is_open()) {
            cerr << "error when reading file with name: " << filePath << "\n";
            return;
        }

        file.read(this->spots, this->rows * this->cols);
        if (file.gcount() < this->rows * this->cols) {
            cerr << "read less bytes than specified";
            return;
        }
    }
};
typedef struct Grid Grid;

enum DirectionType {
    up = 0,
    left = 1,
    down = 2,
    right = 3,
};
typedef enum DirectionType DirectionType;

struct Direction {
    DirectionType type;
    Vec2 v;

    void pointUp() {
        this->type = up;
        this->v = {.x = 0, .y = -1};
    }

    void pointLeft() {
        this->type = left;
        this->v = {.x = -1, .y = 0};
    }

    void pointDown() {
        this->type = down;
        this->v = {.x = 0, .y = 1};
    }

    void pointRight() {
        this->type = right;
        this->v = {.x = 1, .y = 0};
    }
};
typedef struct Direction Direction;

struct Pacman {
    Vec2 position;
    int visibility;

    Pacman() {
        this->position = {5, 5};
        this->visibility = 1;
    }
};
typedef struct Pacman Pacman;

struct Ghost {
    char type;  // 'R', 'G', 'B', 'Y'
    Vec2 position;
    int option;  // used when green decides if uses updateLeft or updateRight
    Direction direction;
    void (*update)(struct Ghost* ghost, Grid* grid);

    void updateRed(Grid* grid) {
        bool valid_next_position = false;
        Vec2 next_pos;
        do {
            next_pos = {
                .x = this->position.x + this->direction.v.x,
                .y = this->position.y + this->direction.v.y,
            };
            char next_spot = grid->spots[next_pos.x + next_pos.y * grid->cols];

            switch (this->direction.type) {
                case DirectionType::up:
                    if (next_spot != '0') {
                        this->direction.pointLeft();
                    } else {
                        valid_next_position = true;
                    }
                    break;

                case DirectionType::left:
                    if (next_spot != '0') {
                        this->direction.pointDown();
                    } else {
                        valid_next_position = true;
                    }
                    break;

                case DirectionType::down:
                    if (next_spot != '0') {
                        this->direction.pointRight();
                    } else {
                        valid_next_position = true;
                    }
                    break;

                case DirectionType::right:
                    if (next_spot != '0') {
                        this->direction.pointUp();
                    } else {
                        valid_next_position = true;
                    }
                    break;
            }
        } while (!valid_next_position);

        this->position = next_pos;
        grid->at(this->position.y, this->position.x);
    };

    void updateBlue(Grid* grid) {
        bool valid_next_position = false;
        Vec2 next_pos;
        do {
            next_pos = {
                .x = this->position.x + this->direction.v.x,
                .y = this->position.y + this->direction.v.y,
            };
            char next_spot = grid->spots[next_pos.x + next_pos.y * grid->cols];

            switch (this->direction.type) {
                case DirectionType::up:
                    if (next_spot != '0') {
                        this->direction.pointRight();
                    } else {
                        valid_next_position = true;
                    }
                    break;

                case DirectionType::right:
                    if (next_spot != '0') {
                        this->direction.pointDown();
                    } else {
                        valid_next_position = true;
                    }
                    break;

                case DirectionType::down:
                    if (next_spot != '0') {
                        this->direction.pointLeft();
                    } else {
                        valid_next_position = true;
                    }
                    break;

                case DirectionType::left:
                    if (next_spot != '0') {
                        this->direction.pointUp();
                    } else {
                        valid_next_position = true;
                    }
                    break;
            }
        } while (!valid_next_position);

        *grid->at(this->position.y, this->position.x) = '0';
        this->position = next_pos;
        *grid->at(this->position.y, this->position.x) = this->type;
    };

    void updateGreen(Grid* grid) {
        if (this->option % 2 == 0) {
            this->updateRed(grid);
        } else {
            this->updateBlue(grid);
        }
        this->option++;
    };

    void updateYellow(Grid* grid) {
        bool valid_next_position = false;
        Vec2 next_pos;
        do {
            int choice = rand() % 4;  // up, left, down, right
            switch (choice) {
                case up:
                    this->direction.pointUp();
                    break;
                case left:
                    this->direction.pointLeft();
                    break;
                case down:
                    this->direction.pointDown();
                    break;
                case right:
                    this->direction.pointRight();
                    break;
            }
            next_pos = {
                .x = this->position.x + this->direction.v.x,
                .y = this->position.y + this->direction.v.y,
            };
            char next_spot = grid->spots[next_pos.x + next_pos.y * grid->cols];

            if (next_spot != '#') {
                valid_next_position = true;
            }

        } while (!valid_next_position);

        *grid->at(this->position.y, this->position.x) = '0';
        this->position = next_pos;
        *grid->at(this->position.y, this->position.x) = this->type;
    };
};
typedef struct Ghost Ghost;
