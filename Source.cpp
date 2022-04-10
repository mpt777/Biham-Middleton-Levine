#include <SFML/Graphics.hpp>
#ifdef _MSC_VER
#include <windows.h>
#endif  
#include <vector>
#include <fstream>
#include <time.h> 
#include <math.h>       /* floor */

#include <mpi.h>
#include <iostream>
#include <stdio.h>
//

using namespace std;
#define MCW MPI_COMM_WORLD
#include <chrono>
using namespace std::chrono;



#define WIDTH 1000
#define HEIGHT 1000


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



void cleargrid() {
    grid.clear();
    //initgrid();
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

    int XCELLS = 200;
    int YCELLS = 200;
    int FULL = 33;
    if (argc > 1) XCELLS = atoi(argv[1]);
    if (argc > 2) YCELLS = atoi(argv[2]);
    if (argc > 3) FULL = atoi(argv[3]);

    int cellw = WIDTH / XCELLS, cellh = HEIGHT / YCELLS;

    int rank, size;
    int work = 0;
    int x = 0;
    int y = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    MPI_Status status;

    int x_cells = XCELLS / (size - 1);

    int** A;
    int** recv_A;

    int flag;

    int next_in_line = (rank + 1) % size;
    if (next_in_line == 0) next_in_line = 1;

    int prev_in_line = rank - 1;
    if (prev_in_line == 0) prev_in_line = size - 1;

    int token_tag = 7;
    int ask_tag = 2;
    int response_tag = 3;

    int pending_requests = 0;

    cout << rank << " | next " << next_in_line << " | prev " << prev_in_line << endl;

    if (rank == 0) { /* i am a pig */
        A = alloc_2d_int(XCELLS, YCELLS);
        recv_A = alloc_2d_int(x_cells, YCELLS);

        std::cout << std::endl << "Biham–Middleton–Levine" << std::endl << std::endl
            << "Space: Toggle start/stop" << std::endl
            << "C: Clear the grid" << std::endl
            << "Tab: Fast forward" << std::endl;


        auto start = high_resolution_clock::now();

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
            }

            mwindow.clear(bgcolor);

            /*
            if (!stopped) {
                timemanager();
            }
            else {
                gameclock.restart();
            }
            */

            float th = 1.f;

            ////////////////////////////////////////////////
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
            /////////////////////////////////////////////////////
            if (!stopped) {
                MPI_Send(&work, 1, MPI_INT, (rank + 1) % size, token_tag, MCW);

                //cout << "wait" << endl;
                MPI_Recv(&work, 1, MPI_INT, size - 1, token_tag, MCW, &status);
                //cout << "got" << endl;

                for (int i = 1; i < size; i++) {
                    MPI_Status status;
                    //cout << "here for "<< i << endl;
                    MPI_Recv(&(recv_A[0][0]), x_cells * YCELLS, MPI_INT, i, 0, MCW, &status);
                    int sender = status.MPI_SOURCE;
                    for (int x_pos = 0; x_pos < x_cells; x_pos++) {
                        for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                            A[x_pos + ((i - 1) * x_cells)][y_pos] = recv_A[x_pos][y_pos];
                        }
                    }
                }
                //cout << "render" << endl;

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
    }
    else {
        A = alloc_2d_int(x_cells, YCELLS);

        //Radnomly sets each board up
        srand(time(NULL) + (rank));

        for (int xc = 0; xc < x_cells; xc++) {
            for (int yc = 0; yc < YCELLS; yc++) {
                int random = 0;
                int to_fill = rand() % 100 + 0;
                if (to_fill < FULL) {
                    random = rand() % 2 + 1;
                }
                A[xc][yc] = random;
            }
        }

        mwindow.close();
        while (1) {
            int data[3] = { -1,-1,-1 };
            int new_data[3] = { -1,-1,-1 };

            //////////////////////////////////////////////////////////////
            vector<vector<int>> temp(x_cells, vector<int>(YCELLS, 0));

            //Clones the current board into a temp board
            for (int x_pos = 0; x_pos < x_cells; x_pos++) {
                for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                    temp[x_pos][y_pos] = A[x_pos][y_pos];
                }
            }

            /////////////////////////////////////////////////////////////

            for (int x_pos = 0; x_pos < x_cells; x_pos++) {
                for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                    if (temp[x_pos][y_pos] == RED) {
                        if (temp[x_pos][(y_pos + 1) % YCELLS] == EMPTY) {
                            A[x_pos][(y_pos + 1) % YCELLS] = RED;
                            A[x_pos][y_pos] = EMPTY;
                        }
                    }
                }
            }

            /////////////////////////////////////////////////////////////
            //Clones the current board into a temp board
            for (int x_pos = 0; x_pos < x_cells; x_pos++) {
                for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                    temp[x_pos][y_pos] = A[x_pos][y_pos];
                }
            }
            //////////////////////////////////////////////////////////////


            //////////////////////////////
            /*
            for (int x_pos = 0; x_pos < x_cells; x_pos++) {
                for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                    if (temp[x_pos][y_pos] == BLUE) {
                        if (temp[(x_pos+1) % x_cells][(y_pos)] == EMPTY) {
                            A[(x_pos+1)%x_cells][y_pos] = BLUE;
                            A[x_pos][y_pos] = EMPTY;
                        }
                    }
                }
            }
            */

            for (int x_pos = 0; x_pos < x_cells; x_pos++) {
                for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                    if (temp[x_pos][y_pos] == BLUE) {
                        if (x_pos + 1 >= x_cells) {

                            data[0] = BLUE;
                            data[1] = 0;
                            data[2] = y_pos;

                            //cout <<"Send Data Rank " << rank << " color " << data[0] << " x"<< data[1] << " y" << data[2] << endl;
                            MPI_Send(&data, 3, MPI_INT, next_in_line, ask_tag, MCW);
                            pending_requests++;
                        }
                        else if (temp[(x_pos + 1)][y_pos] == EMPTY) {
                            A[(x_pos + 1)][y_pos] = BLUE;
                            A[x_pos][y_pos] = EMPTY;
                        }
                    }
                }
            }

            //Token passing
            while (1) {
                MPI_Iprobe(prev_in_line, ask_tag, MCW, &flag, &status);
                if (flag) {
                    while (flag) {
                        MPI_Recv(&data, 3, MPI_INT, prev_in_line, ask_tag, MCW, MPI_STATUS_IGNORE);
                        int color = data[0];
                        int x = data[1];
                        int y = data[2];
                        //cout << "Recv Data Rank " << rank << " color " << data[0] << " x" << data[1] << " y" << data[2] << endl;

                        //cout << "R " << rank << " To " << prev_in_line << " Send " << endl;
                        int new_color = color;
                        if (temp[x][y] == EMPTY) {
                            //cout << "empty" << endl;
                            A[x][y] = color;
                            new_color = EMPTY;
                        }
                        new_data[0] = new_color;
                        new_data[1] = x_cells - 1;
                        new_data[2] = y;
                        //cout << "2 Recv Data Rank " << rank << " color " << new_data[0] << " x" << new_data[1] << " y" << new_data[2] << endl;
                        MPI_Send(&new_data, 3, MPI_INT, prev_in_line, response_tag, MCW);
                        MPI_Iprobe(prev_in_line, ask_tag, MCW, &flag, &status);
                    }
                }

                MPI_Iprobe(next_in_line, response_tag, MCW, &flag, &status);
                if (flag) {
                    while (flag) {
                        MPI_Recv(&data, 3, MPI_INT, next_in_line, response_tag, MCW, MPI_STATUS_IGNORE);
                        pending_requests--;
                        int color = data[0];
                        int x = data[1];
                        int y = data[2];
                        //cout << "Respond Data || Rank " << rank << " color " << data[0] << " x" << data[1] << " y" << data[2] << endl;
                        A[x][y] = color;
                        MPI_Iprobe(MPI_ANY_SOURCE, response_tag, MCW, &flag, &status);
                    }
                }
                //TOKEN 
                MPI_Iprobe(rank - 1, token_tag, MCW, &flag, &status);
                if (flag and pending_requests == 0) {
                    //cout << "rank " << rank << " recv " << rank - 1 << endl;
                    MPI_Recv(&work, 1, MPI_INT, rank - 1, token_tag, MCW, MPI_STATUS_IGNORE);
                    //cout << "rank |*| " << rank << " send to " << (rank + 1) % size << endl;
                    MPI_Send(&work, 1, MPI_INT, (rank + 1) % size, token_tag, MCW);
                    break;
                }
                //Sleep(1000);
            }
            //cout << "cycle" << endl;
            /////////////////////////////////////////////////////////////
            MPI_Send(&(A[0][0]), x_cells * YCELLS, MPI_INT, 0, 0, MCW);
        }

    }
    MPI_Finalize();
    return 0;
}

