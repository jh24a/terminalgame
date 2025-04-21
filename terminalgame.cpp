#include <iostream>
#include "header.h"
#include <chrono>
#include <cstdlib>
#include <thread>
#include <windows.h>
// Handle case where no curses library is found during build
#if defined(NO_CURSES_FOUND)
    // Fallback to direct console output with ANSI escape codes
    #define USE_ANSI_FALLBACK
#else
    // Try multiple possible header locations for cross-platform support
    #if defined(_WIN32)
        // Try PDCurses paths for Windows
        #if __has_include(<pdcurses.h>)
            #include <pdcurses.h>
        #elif __has_include(<curses.h>)
            #include <curses.h>
        #else
            // Fallback to direct console output
            #define USE_ANSI_FALLBACK
        #endif
    #else
        // For Linux/Mac
        #include <ncurses.h>
    #endif
#endif

using namespace std;

int const VIEW_PORT_ROW = 40;
int const VIEW_PORT_COL = 200;

void fill(Object objects[], char array[VIEW_PORT_ROW][VIEW_PORT_COL], int x, int y);
void handle_input(int& x, int& y, bool& quit);
void makeScene(Object objects[10]);
void draw_screen(char array[VIEW_PORT_ROW][VIEW_PORT_COL]);

// ANSI color codes for the fallback mode
#ifdef USE_ANSI_FALLBACK
#define ANSI_RESET      "\033[0m"
#define ANSI_GREEN      "\033[32m"
#define ANSI_RED        "\033[31m"
#define ANSI_CLEAR      "\033[2J\033[H"
#define ANSI_HIDE_CURSOR "\033[?25l"
#define ANSI_SHOW_CURSOR "\033[?25h"
#endif

int main() {
#ifndef USE_ANSI_FALLBACK
    // Initialize ncurses
    initscr();            // Start ncurses mode
    cbreak();             // Disable line buffering
    noecho();             // Don't echo keypresses
    keypad(stdscr, TRUE); // Enable arrow keys
    curs_set(0);          // Hide cursor
    nodelay(stdscr, TRUE);// Non-blocking input
    
    // Check if terminal supports colors
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);  // For player
        init_pair(2, COLOR_RED, COLOR_BLACK);    // For objects
    }
#else
    // ANSI fallback mode setup
    // Hide cursor and clear screen
    std::cout << ANSI_HIDE_CURSOR << ANSI_CLEAR;
    // Set up Windows console for ANSI escape codes
    #ifdef _WIN32
    // Enable virtual terminal processing
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    #endif
#endif
    
    // Game variables
    char array[VIEW_PORT_ROW][VIEW_PORT_COL];
    int x = 12;
    int y = 15;
    bool quit = false;
    
    // Initialize objects
    Object objects[10];
    makeScene(objects);
    
    // Display instructions
    mvprintw(0, 0, "Use WASD or arrow keys to move, Q to quit");
    mvprintw(1, 0, "Press any key to start...");
    refresh();
    getch(); // Wait for a keypress to start
    
    // Game loop
    while (!quit) {
        // Clear the virtual screen for the next frame
        clear();
        
        // Update game state
        fill(objects, array, x, y);
        
        // Draw to the screen buffer
        draw_screen(array);
        
        // Handle keyboard input
        handle_input(x, y, quit);
        
        // Update the physical screen from the buffer (double buffering)
        refresh();
        
        // Control game speed
        this_thread::sleep_for(chrono::milliseconds(10));
    }
    
    // Clean up ncurses
    clear();
    refresh();
    endwin();
    
    // Additional cleanup for Windows console
    #ifdef _WIN32
    // Flush any pending input
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    // Reset the console mode
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode | ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
    #endif
    
    return 0;
}

/************************************/
void handle_input(int& x, int& y, bool& quit) {

    if (GetAsyncKeyState('W') & 0x8000) {
        --y;
    }
    if (GetAsyncKeyState('A') & 0x8000) {
        --x;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        ++y;
    }
    if (GetAsyncKeyState('D') & 0x8000) {
        ++x;
    }
    if (x < 1) x = 1;
    if (x >= VIEW_PORT_COL - 1) x = VIEW_PORT_COL - 2;
    if (y < 1) y = 1;
    if (y >= VIEW_PORT_ROW - 1) y = VIEW_PORT_ROW - 2;
    if (GetAsyncKeyState('Q') & 0x8000) quit = true;
}

/************************************/
void fill(Object objects[], char array[VIEW_PORT_ROW][VIEW_PORT_COL], int x, int y) {
    // Initialize the array with borders and spaces
    for (int i = 0; i < VIEW_PORT_ROW; i++) {
        for (int j = 0; j < VIEW_PORT_COL; j++) {
            if (i == 0 || i == VIEW_PORT_ROW - 1 || j == 0 || j == VIEW_PORT_COL - 1) {
                // Border
                array[i][j] = '#';
            } else {
                // Empty space
                array[i][j] = ' ';
            }
        }
    }
    
    // Place objects
    for (int i = 0; i < 10; i++) {
        if (objects[i].x > 0 && objects[i].x < VIEW_PORT_COL && 
            objects[i].y > 0 && objects[i].y < VIEW_PORT_ROW) {
            array[objects[i].y][objects[i].x] = 'X';
        }
    }
    
    // Place player
    array[y][x] = '@';
}

/***************************************/
void draw_screen(char array[VIEW_PORT_ROW][VIEW_PORT_COL]) {
    for (int i = 0; i < VIEW_PORT_ROW; i++) {
        for (int j = 0; j < VIEW_PORT_COL; j++) {
            // Draw each character with appropriate colors
            if (array[i][j] == '@' && has_colors()) {
                attron(COLOR_PAIR(1));  // Green for player
                mvaddch(i, j, array[i][j]);
                attroff(COLOR_PAIR(1));
            } else if (array[i][j] == 'X' && has_colors()) {
                attron(COLOR_PAIR(2));  // Red for objects
                mvaddch(i, j, array[i][j]);
                attroff(COLOR_PAIR(2));
            } else {
                mvaddch(i, j, array[i][j]);
            }
        }
    }
    
    // Display game information
    mvprintw(VIEW_PORT_ROW + 1, 0, "Use WASD or arrow keys to move, Q to quit");
}

/*************************************/
void makeScene(Object objects[10]) {
    srand(time(0));
    
    for (int i = 0; i < 10; ++i) {
        objects[i].x = 1 + rand() % (VIEW_PORT_COL - 2);
        objects[i].y = 1 + rand() % (VIEW_PORT_ROW - 2);
    }
}
