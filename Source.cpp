#include <SFML/Graphics.hpp>
#include <windows.h>
#include <vector>
#include <fstream>
#include <time.h> 

#include <mpi.h>
#include <iostream>
#include <stdio.h>

using namespace std;
#define MCW MPI_COMM_WORLD
#include <chrono>
using namespace std::chrono;



#define WIDTH 1000
#define HEIGHT 1000

#define XCELLS 200
#define YCELLS 200

int cellw = WIDTH / XCELLS, cellh = HEIGHT / YCELLS;

int EMPTY = 0;
int RED = 1;
int BLUE = 2;

bool stopped = false;
bool step = true;

using namespace std;

std::vector<std::vector<int>> grid;

sf::RenderWindow mwindow(sf::VideoMode(WIDTH, HEIGHT), "Traffic");

float defaultdelay = 1.f;
float fastdelay = 0.0005f;
float delay = defaultdelay;

sf::Clock gameclock;
void timemanager() {
    return;
    float elapsed = gameclock.getElapsedTime().asSeconds();
    //if (elapsed >= delay) {
    
        std::vector<std::vector<int>> temp = grid;

        for (int x_pos = 0; x_pos < XCELLS; x_pos++) {
            for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                if (temp[x_pos][y_pos] == RED) {
                    if (temp[x_pos][(y_pos + 1) % YCELLS] == EMPTY) {
                        grid[x_pos][(y_pos + 1) % YCELLS] = RED;
                        grid[x_pos][y_pos] = EMPTY;
                    }
                }
            }
        }
        temp = grid;
        for (int x_pos = 0; x_pos < XCELLS; x_pos++) {
            for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                if (temp[x_pos][y_pos] == BLUE) {
                    if (temp[(x_pos + 1) % XCELLS][y_pos] == EMPTY) {
                        grid[(x_pos + 1) % XCELLS][y_pos] = BLUE;
                        grid[x_pos][y_pos] = EMPTY;
                    };
                }
            }
        }
        gameclock.restart();
   // }
}

void process_grid() {
    std::vector<std::vector<int>> temp = grid;
    for (int x_pos = 0; x_pos < XCELLS; x_pos++) {
        for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
            if (temp[x_pos][y_pos] == RED) {
                if (temp[x_pos][(y_pos + 1) % YCELLS] == EMPTY) {
                    grid[x_pos][(y_pos + 1) % YCELLS] = RED;
                    grid[x_pos][y_pos] = EMPTY;
                }
            }
        }
    }
    temp = grid;
    for (int x_pos = 0; x_pos < XCELLS; x_pos++) {
        for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
            if (temp[x_pos][y_pos] == BLUE) {
                if (temp[(x_pos + 1) % XCELLS][y_pos] == EMPTY) {
                    grid[(x_pos + 1) % XCELLS][y_pos] = BLUE;
                    grid[x_pos][y_pos] = EMPTY;
                };
            }
        }
    }
}

void initgrid() {
    srand(time(NULL));
    for (int yc = 0; yc < YCELLS; yc++) {
        std::vector<int> ln;
        for (int xc = 0; xc < XCELLS; xc++) {
            int random = rand() % 3 + 0;
            if (random != 0) {
                if ((rand() % 2 + 0)) {
                    random = 0;
                }
            }
            ln.push_back(random);
        }

        grid.push_back(ln);
    }
}

void cleargrid() {
    grid.clear();
    initgrid();
}

void correcttitle() {
    if (stopped) {
        mwindow.setTitle("Biham–Middleton–Levine - Stopped");
    }
    else {
        mwindow.setTitle("Biham–Middleton–Levine");
    }
}

void togglestop() {
    stopped = !stopped;
    if (stopped) {
        mwindow.setTitle("Biham–Middleton–Levine- Stopped");
    }
    else {
        mwindow.setTitle("Biham–Middleton–Levine");
    }
}

void togglecell(int ln, int col) {
    int v = grid[ln][col];
    if (v == 1) {
        grid[ln][col] = 0;
    }
    else {
        grid[ln][col] = 1;
    }
}

int** alloc_2d_int(int rows, int cols) {
    int* data = (int*)malloc(rows * cols * sizeof(int));
    int** array = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++)
        array[i] = &(data[cols * i]);

    return array;
}


int main(int argc, char** argv) {

    sf::Color gridcolor(200, 200, 200);
    sf::Color cellcolor(0, 0, 0);
    sf::Color redcolor(255, 0, 0);
    sf::Color bluecolor(0, 0, 255);
    sf::Color bgcolor(255, 255, 255);


    int rank, size;
    int work = 0;
    int x = 0;
    int y = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    MPI_Status status;

    togglestop();
    cleargrid();

    int** A;

    A = alloc_2d_int(XCELLS, YCELLS);

    //grid[XCELLS][YCELLS];

    if (rank == 0) { /* i am a pig */

        std::cout << std::endl << "Biham–Middleton–Levine" << std::endl << std::endl
            << "Space: Toggle start/stop" << std::endl
            << "C: Clear the grid" << std::endl
            << "Tab: Fast forward" << std::endl;


        auto start = high_resolution_clock::now();


        srand(time(NULL));
        for (int yc = 0; yc < YCELLS; yc++) {
            for (int xc = 0; xc < XCELLS; xc++) {
                int random = rand() % 3 + 0;
                if (random != 0) {
                    if ((rand() % 2 + 0)) {
                        random = 0;
                    }
                }
                A[xc][yc] = random;
            }
        }

        for (int i = 1; i < size; ++i) {
            MPI_Send(&(A[0][0]), XCELLS * YCELLS, MPI_INT, i, 0, MCW);
        }

        //while
        while (mwindow.isOpen()) {
            sf::Event event;
            while (mwindow.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    mwindow.close();
                }
                else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Space) { // toggle stop / start
                        togglestop();
                    }
                    else if (event.key.code == sf::Keyboard::C) { // clear
                        cleargrid();
                    }
                    else if (event.key.code == sf::Keyboard::Tab) { // fast forward

                    }
                }
                else if (event.type == sf::Event::KeyReleased) {
                    if (event.key.code == sf::Keyboard::Tab) { // restore default speed
                        delay = defaultdelay;
                        correcttitle();
                    }
                }
                else if (event.type == sf::Event::MouseButtonReleased) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        int col = floor(event.mouseButton.x / cellw);
                        int ln = floor(event.mouseButton.y / cellh);

                        gameclock.restart();
                        togglecell(ln, col);

                        // std::cout << "Line: " << ln << ", Column: " << col << std::endl;
                    }
                }
            }

            mwindow.clear(bgcolor);

            if (!stopped) {
                timemanager();
            }
            else {
                gameclock.restart();
            }

            float th = 1.f;

            /* Draw the grid */
            for (int xc = 0; xc <= XCELLS; xc++) {
                int c = cellw * xc;

                sf::RectangleShape line;
                line.setPosition(c, 0);
                line.setSize(sf::Vector2f(th, HEIGHT));

                line.setFillColor(gridcolor);
                line.setOutlineThickness(0);

                mwindow.draw(line);
            }

            for (int yc = 0; yc <= YCELLS; yc++) {
                int c = cellh * yc;

                sf::RectangleShape line;
                line.setPosition(0, c);
                line.setSize(sf::Vector2f(WIDTH, th));

                line.setFillColor(gridcolor);
                line.setOutlineThickness(0);

                mwindow.draw(line);
            }
            if (!stopped) {
                MPI_Send(&work, 1, MPI_INT, 1, 1, MCW);
                MPI_Recv(&(A[0][0]), XCELLS * YCELLS, MPI_INT, 1, MPI_ANY_TAG, MCW, MPI_STATUS_IGNORE);
                //MPI_Recv(&work, XCELLS * YCELLS, MPI_INT, 1, MPI_ANY_TAG, MCW, MPI_STATUS_IGNORE);
            }
            /* Draw the cells */
            for (int x_pos = 0; x_pos < XCELLS; x_pos++) {
                for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                    int col = A[x_pos][y_pos];
                    if (col != EMPTY) {
                        sf::RectangleShape cell;
                        cell.setPosition(y_pos * cellw + th, x_pos * cellh + th);
                        cell.setSize(sf::Vector2f(cellw - th, cellh - th));

                        cell.setOutlineThickness(0);

                        if (col == BLUE) {
                            cell.setFillColor(bluecolor);
                        }
                        else if (col == RED) {
                            cell.setFillColor(redcolor);
                        }
                        mwindow.draw(cell);
                    }
                }
            }

            mwindow.display();
        }


        //cout << "All Done!" << endl;

    }
    else {  
        /* i am a horse  */
        mwindow.close();
        MPI_Recv(&(A[0][0]), XCELLS * YCELLS, MPI_INT, 0, MPI_ANY_TAG, MCW, MPI_STATUS_IGNORE);

        
        while (1){
            MPI_Recv(&work, 1, MPI_INT, 0, MPI_ANY_TAG, MCW, MPI_STATUS_IGNORE);

            //////////////////////////////////////////////////////////////
            int temp[XCELLS][YCELLS];
            for (int x_pos = 0; x_pos < XCELLS; x_pos++) {
                for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                    temp[x_pos][y_pos] = A[x_pos][y_pos];
                }
            }

            for (int x_pos = 0; x_pos < XCELLS; x_pos++) {
                for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                    if (temp[x_pos][y_pos] == RED) {
                        if (temp[x_pos][(y_pos + 1) % YCELLS] == EMPTY) {
                            A[x_pos][(y_pos + 1) % YCELLS] = RED;
                            A[x_pos][y_pos] = EMPTY;
                        }
                    }
                }
            }

            for (int x_pos = 0; x_pos < XCELLS; x_pos++) {
                for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                    temp[x_pos][y_pos] = A[x_pos][y_pos];
                }
            }

            for (int x_pos = 0; x_pos < XCELLS; x_pos++) {
                for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                    if (temp[x_pos][y_pos] == BLUE) {
                        if (temp[(x_pos + 1) % XCELLS][y_pos] == EMPTY) {
                            A[(x_pos + 1) % XCELLS][y_pos] = BLUE;
                            A[x_pos][y_pos] = EMPTY;
                        }
                    }
                }
            }
            /////////////////////////////////////////////////////////////
            MPI_Send(&(A[0][0]), XCELLS* YCELLS, MPI_INT, 0, 0, MCW);
            //MPI_Send(&grid[0][0], 1, MPI_INT, 0, 0, MCW);
        }

    }
    MPI_Finalize();
    return 0;
}

