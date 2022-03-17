#include <SFML/Graphics.hpp>
#include <windows.h>
#include <vector>
#include <iostream>
#include <iostream>
#include <fstream>
#include <time.h> 



#define WIDTH 700
#define HEIGHT 700

#define XCELLS 350
#define YCELLS 350

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

void initgrid() {
//    srand(time(NULL));
    for (int yc = 0; yc < YCELLS; yc++) {
        std::vector<int> ln;
        for (int xc = 0; xc < XCELLS; xc++) {
            int random = rand() % 3 + 0;
            if (random != 0) {
                if ((rand() % 3 + 0)) {
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

int main(int argc, char** argv) {
    std::cout << std::endl << "Biham–Middleton–Levine" << std::endl << std::endl
        << "Space: Toggle start/stop" << std::endl
        << "C: Clear the grid" << std::endl
        << "Tab: Fast forward" << std::endl;

    sf::Color gridcolor(200, 200, 200);
    sf::Color cellcolor(0, 0, 0);
    sf::Color redcolor(255, 0, 0);
    sf::Color bluecolor(0, 0, 255);
    sf::Color bgcolor(255, 255, 255);

    togglestop();
    cleargrid();

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

        /* Draw the cells */
        for (int x_pos = 0; x_pos < XCELLS; x_pos++) {
            for (int y_pos = 0; y_pos < YCELLS; y_pos++) {
                int col = grid[x_pos][y_pos];
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