#include "pacman.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

using std::cerr;

int getRand(int min, int max) {
    return (rand() % (max - min)) + min;
}

// Grid -------------------------------------------------------------

Grid::Grid(int rows, int cols) {
    this->rows = rows;
    this->cols = cols;
    this->spots = new char[rows * cols];
}

Grid::~Grid() {
    delete this->spots;
}

inline char* Grid::at(int row, int col) {
    return &this->spots[row + col * this->cols];
}

inline char* Grid::at(Vec2 pos) {
    return &this->spots[pos.y + pos.x * this->cols];
}

void Grid::readGrid(std::ifstream *file) {
    if (!file->is_open()) {
        cerr << "error when reading file \n";
        return;
    }

    for (int i = 0; i < this->rows * this->cols; i++)
    {
        char aux;
        if (!(*file) >> this -> spots[i] >> aux) {
            cerr << "read less bytes than specified";
            return;
        }
    }

    return;
}

// Direction -------------------------------------------------------------

void Direction::pointUp() {
    this->type = up;
    this->v = {.x = 0, .y = -1};
}

void Direction::pointLeft() {
    this->type = left;
    this->v = {.x = -1, .y = 0};
}

void Direction::pointDown() {
    this->type = down;
    this->v = {.x = 0, .y = 1};
}

void Direction::pointRight() {
    this->type = right;
    this->v = {.x = 1, .y = 0};
}

// Pacman -------------------------------------------------------------

Pacman::Pacman() {
    this->position = {5, 5};
    this->visibility = 1;
}

// Ghost ------------------------------------------------------------------

int Ghost::updateRed(Grid* grid) {
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
                if (next_spot != EMPTY) {
                    this->direction.pointLeft();
                } else {
                    valid_next_position = true;
                }
                break;

            case DirectionType::left:
                if (next_spot != EMPTY) {
                    this->direction.pointDown();
                } else {
                    valid_next_position = true;
                }
                break;

            case DirectionType::down:
                if (next_spot != EMPTY) {
                    this->direction.pointRight();
                } else {
                    valid_next_position = true;
                }
                break;

            case DirectionType::right:
                if (next_spot != EMPTY) {
                    this->direction.pointUp();
                } else {
                    valid_next_position = true;
                }
                break;
        }
    } while (!valid_next_position);

    this->position = next_pos;
    grid->at(this->position.y, this->position.x);

    return 0;
};

int Ghost::updateBlue(Grid* grid) {
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
                if (next_spot != EMPTY) {
                    this->direction.pointRight();
                } else {
                    valid_next_position = true;
                }
                break;

            case DirectionType::right:
                if (next_spot != EMPTY) {
                    this->direction.pointDown();
                } else {
                    valid_next_position = true;
                }
                break;

            case DirectionType::down:
                if (next_spot != EMPTY) {
                    this->direction.pointLeft();
                } else {
                    valid_next_position = true;
                }
                break;

            case DirectionType::left:
                if (next_spot != EMPTY) {
                    this->direction.pointUp();
                } else {
                    valid_next_position = true;
                }
                break;
        }
    } while (!valid_next_position);

    *grid->at(this->position.y, this->position.x) = EMPTY;
    this->position = next_pos;
    *grid->at(this->position.y, this->position.x) = this->type;

    return 0;
};

int Ghost::updateGreen(Grid* grid) {
    if (this->option % 2 == 0) {
        this->updateRed(grid);
    } else {
        this->updateBlue(grid);
    }
    this->option++;

    return 0;
};

int Ghost::updateYellow(Grid* grid) {
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

    *grid->at(this->position.y, this->position.x) = EMPTY;
    this->position = next_pos;
    *grid->at(this->position.y, this->position.x) = this->type;

    return 0;
};


// Game State --------------------------------------------------------------

GameState::GameState(const char* mapFile) {
    std::ifstream file(mapFile);

    if (!file.is_open())
    {
        cerr << "error when reading file with name: " << mapFile << "\n";
        return;
    }
    // Assumo que arquivo csv está formatado corretamente e contem todos dados que diz ter
    int rows, cols, i;
    char throwaway;
    Vec2 genPosition;


    // Discard ','
    file >> rows >> throwaway >> cols;
    // Read Indicator
    this -> grid = new Grid(rows, cols);
    this -> grid -> readGrid(&file);

    file >> throwaway;

    if (throwaway == 'n') {
        // Generate Ghosts
        while (i < 4)
        {
            genPosition.x = getRand(0, cols);
            genPosition.y = getRand(0, rows);
            throwaway = getRand(0, 4);

            if (*this -> grid -> at(genPosition) != EMPTY)
                continue;
            
            this -> ghost[i].position = genPosition;
            switch ((DirectionType) throwaway) {
                case left:
                    this -> ghost[i].direction.pointLeft();
                    break;
                case right:
                    this -> ghost[i].direction.pointRight();
                    break;
                case down:
                    this -> ghost[i].direction.pointDown();
                    break;
                case up:
                    this -> ghost[i].direction.pointUp();
                    break;        
            }
            i++;
        }
        *this -> grid -> at(this -> ghost[0].position) = RED;
        *this -> grid -> at(this -> ghost[1].position) = BLUE;
        *this -> grid -> at(this -> ghost[2].position) = GREEN;
        *this -> grid -> at(this -> ghost[3].position) = YELLOW;
        this -> ghost[0].type = RED;
        this -> ghost[1].type = BLUE;
        this -> ghost[2].type = GREEN;
        this -> ghost[3].type = YELLOW;

        // Generate Player
        do
        {
            genPosition.x = getRand(0, cols);
            genPosition.y = getRand(0, rows);
        } while (*this -> grid -> at(genPosition) != EMPTY);
        
        this -> pacman.position = genPosition;

        // Generate Files
        i = 0;
        while (i < 6)
        {
            genPosition.x = getRand(0, cols);
            genPosition.y = getRand(0, rows);

            if (*(this -> grid -> at(genPosition)) != EMPTY)
                continue;
            
            // Generates '1', '2', etc in order
            *(this -> grid -> at(genPosition)) = (char) ('1' + i);
            
            i++;
        }
    }
    else
    {
        file >> throwaway;
        for (int i = 0; i < 4; i++)
        {
            file >> this -> ghost[i].position.x >> throwaway;
            file >> this -> ghost[i].position.y >> throwaway;
            file >> throwaway;
        
            switch ((DirectionType) throwaway) {
                case left:
                    this -> ghost[i].direction.pointLeft();
                    break;
                case right:
                    this -> ghost[i].direction.pointRight();
                    break;
                case down:
                    this -> ghost[i].direction.pointDown();
                    break;
                case up:
                    this -> ghost[i].direction.pointUp();
                    break;        
            }
            file >> throwaway;
        }
        this -> ghost[0].type = RED;
        this -> ghost[1].type = BLUE;
        this -> ghost[2].type = GREEN;
        this -> ghost[3].type = YELLOW;

        file >> this -> pacman.position.x >> throwaway;
        file >> this -> pacman.position.y >> throwaway;
    }
    // Max visibility determined by gridsize
    if (this -> grid -> cols > this -> grid -> rows)
        this -> maxVisibility = this -> grid -> cols / 2;
    else
        this -> maxVisibility = this -> grid -> rows / 2;
    
    this -> round = 0;
    this -> remaining_pellets = 6;
}

GameState::~GameState() {
    this -> grid -> ~Grid();
}

void GameState::updateGameState(DirectionType directionPacman) {
    // Return Win Game 
    if (this -> remaining_pellets == 0)
        return;
    
    int foundFile;
    Vec2 nextPos = this->pacman.position;
    switch (directionPacman)
    {
        case up:
            nextPos.x--;
            break;
        case down:
            nextPos.x++;
            break;
        case left:
            nextPos.y--;
            break;
        case right:
            nextPos.y++;
            break;
        default:
            return;
    }

    char nextSquare = *(this -> grid -> at(nextPos));
    
    // Game Over
    if (nextSquare == RED || nextSquare == BLUE || nextSquare == GREEN || nextSquare == YELLOW)
        return;
    
    switch (nextSquare)
    {
        case WALL:
            // Decide if invalid input moves ghost or if only re-ask for input
            foundFile = -1;
            break;
        case FILE1:
            foundFile = 1;
            this -> remaining_pellets--;
            break;
        case FILE2:
            foundFile = 2;
            this -> remaining_pellets--;
            break;
        case FILE3:
            foundFile = 3;
            this -> remaining_pellets--;
            break;
        case FILE4:
            foundFile = 4;
            this -> remaining_pellets--;
            break;
        case FILE5:
            foundFile = 5;
            this -> remaining_pellets--;
            break;
        case FILE6:
            foundFile = 6;
            this -> remaining_pellets--;
            break;
        default:
            foundFile = 0;
            break;
    }

    if (foundFile != -1)
    {
        *this -> grid -> at(this -> pacman.position) = EMPTY;
        this -> pacman.position = nextPos;
        *this -> grid -> at(nextPos) = PACMAN;
    }

    for (int i = 0; i < 4; i++)
        // Ghost move kill player
        if(this->ghost[i].update(&this->ghost[i], this -> grid) == 0)
            return;

    // Return Next Square, if it is equal to a file value initiate transfer
    this -> round++;
    if (this -> round % 5 == 0 )
        if (this -> pacman.visibility < this -> maxVisibility)
            this -> pacman.visibility++;

    return;
}