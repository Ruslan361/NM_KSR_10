#include "heat_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>
#include <locale.h>
#include <string.h>
#include <time.h>

#define MAX_FILENAME_LEN 256

// Function prototypes
void print_menu(WINDOW *menu_win, int highlight, char *choices[], int n_choices, int n, int m, double T);
void initialize_ncurses();
WINDOW* create_progress_bar();
void update_progress_bar(WINDOW *win, float progress, double remaining_time);
void finalize_progress_bar(WINDOW *win);
void save_csv_format1(HeatEquationSolver* solver, WINDOW *progress_win, int comments);
void save_csv_format2(HeatEquationSolver* solver, WINDOW *progress_win);

int main() {
    setlocale(LC_ALL, "");

    int n = 100;
    int m = 100;
    double T = 10.0;

    // Initialize ncurses
    initialize_ncurses();

    // Create menu window
    int width = 70;
    int height = 20;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;
    WINDOW *menu_win = newwin(height, width, starty, startx);
    keypad(menu_win, TRUE);

    #define NUM_CHOICES 7
    char *choices[NUM_CHOICES] = {
        "1. Set parameters n, m, T",
        "2. Draw 3D plot",
        "3. Save to XLS",
        "4. Save to CSV (Format 1: t, x, v)",
        "5. Save to CSV (Format 2: t rows, x columns)",
        "6. Save 3D plot",
        "7. Exit"
    };

    int choice = 0;
    int highlight = 1;

    // Main loop
    while(1) {
        print_menu(menu_win, highlight, choices, NUM_CHOICES, n, m, T);
        int c = wgetch(menu_win);
        switch(c) {
            case KEY_UP:
                if(highlight == 1)
                    highlight = NUM_CHOICES;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if(highlight == NUM_CHOICES)
                    highlight = 1;
                else 
                    ++highlight;
                break;
            case 10: // Enter
                choice = highlight;
                if(choice == 1) {
                    // Set parameters
                    echo();
                    curs_set(1);
                    mvprintw(starty + height, 0, "Enter n: ");
                    clrtoeol();
                    refresh();
                    scanw("%d", &n);
                    mvprintw(starty + height + 1, 0, "Enter m: ");
                    clrtoeol();
                    refresh();
                    scanw("%d", &m);
                    mvprintw(starty + height + 2, 0, "Enter T: ");
                    clrtoeol();
                    refresh();
                    scanw("%lf", &T);
                    noecho();
                    curs_set(0);
                    // Clear input
                    for(int i = 0; i < 3; ++i) {
                        move(starty + height + i, 0);
                        clrtoeol();
                    }
                    refresh();
                }
                else if(choice == 2 || choice == 6) {
                    // Draw or Save 3D plot
                    // Ensure results are saved before plotting
                    // Create progress bar
                    WINDOW *progress_win = create_progress_bar();
                    HeatEquationSolver* solver = createHeatEquationSolver(n, m, T);
                    clock_t start_time = clock();
                    solve(solver, progress_win, start_time);
                    freeHeatEquationSolver(solver);
                    finalize_progress_bar(progress_win);

                    // Save results to CSV in t, x, v format
                    progress_win = create_progress_bar();
                    solver = createHeatEquationSolver(n, m, T);
                    solve(solver, progress_win, start_time);
                    save_csv_format1(solver, progress_win, 0);
                    freeHeatEquationSolver(solver);
                    finalize_progress_bar(progress_win);

                    // Formulate command
                    char command[512];
                    if(choice == 2) {
                        // Draw 3D plot
                        snprintf(command, sizeof(command),
                            "conda run -n pyside6 python ./show.py results_format1.csv False True");
                    } else {
                        // Save 3D plot
                        snprintf(command, sizeof(command),
                            "conda run -n pyside6 python ./show.py results_format1.csv False False --save_3d_plot");
                    }
                    system(command);
                }
                else if(choice == 3) {
                    // Save to XLS (Assuming Excel support)
                    // Implement saving to XLS if required
                }
                else if(choice == 4) {
                    // Save to CSV (Format 1: t, x, v)
                    WINDOW *progress_win = create_progress_bar();
                    // Create and solve the problem
                    HeatEquationSolver* solver = createHeatEquationSolver(n, m, T);
                    clock_t start_time = clock();
                    solve(solver, progress_win, start_time);
                    // Save results
                    save_csv_format1(solver, progress_win, 1);
                    freeHeatEquationSolver(solver);
                    finalize_progress_bar(progress_win);
                }
                else if(choice == 5) {
                    // Save to CSV (Format 2: t rows, x columns)
                    WINDOW *progress_win = create_progress_bar();
                    // Create and solve the problem
                    HeatEquationSolver* solver = createHeatEquationSolver(n, m, T);
                    clock_t start_time = clock();
                    solve(solver, progress_win, start_time);
                    // Save results
                    save_csv_format2(solver, progress_win);
                    freeHeatEquationSolver(solver);
                    finalize_progress_bar(progress_win);
                }
                else if(choice == 7) {
                    // Exit
                    endwin();
                    return 0;
                }
                break;
            default:
                break;
        }
    }

    endwin();
    return 0;
}

void initialize_ncurses() {
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);

    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support colors.\n");
        exit(1);
    }
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
}

void print_menu(WINDOW *menu_win, int highlight, char *choices[], int n_choices, int n, int m, double T) {
    int x = 2, y = 2;
    box(menu_win, 0, 0);
    for(int i = 0; i < n_choices; ++i) {
        if(highlight == i + 1) {
            wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, y, x, "%s", choices[i]);
            wattroff(menu_win, A_REVERSE);
        } else {
            mvwprintw(menu_win, y, x, "%s", choices[i]);
        }
        ++y;
    }
    // Display current parameters
    mvwprintw(menu_win, y + 1, x, "Current parameters:");
    mvwprintw(menu_win, y + 2, x, "n = %d", n);
    mvwprintw(menu_win, y + 3, x, "m = %d", m);
    mvwprintw(menu_win, y + 4, x, "T = %.2f", T);
    wrefresh(menu_win);
}

WINDOW* create_progress_bar() {
    int height = 5, width = 70;
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;
    WINDOW *win = newwin(height, width, starty, startx);
    keypad(win, TRUE);
    box(win, 0, 0);
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 2, "Progress: [>                         ] 0%% Estimated time: Calculating...");
    wattroff(win, COLOR_PAIR(1));
    wrefresh(win);
    return win;
}

void update_progress_bar(WINDOW *win, float progress, double remaining_time) {
    int width = 30;
    int pos = (int)(progress * width);
    int percent = (int)(progress * 100);

    // Clear line before updating
    mvwprintw(win, 1, 2, "                                                                               ");
    mvwprintw(win, 1, 2, "Progress: [");
    for(int i = 0; i < width; ++i) {
        if(i < pos)
            waddch(win, '#');
        else if(i == pos)
            waddch(win, '>');
        else
            waddch(win, ' ');
    }
    waddch(win, ']');

    mvwprintw(win, 1, 12 + width + 2, " %3d%% Estimated time: %.0f sec", percent, remaining_time);
    wrefresh(win);
}

void finalize_progress_bar(WINDOW *win) {
    werase(win);
    wrefresh(win);
    delwin(win);
}

void save_csv_format1(HeatEquationSolver* solver, WINDOW *progress_win, int comments) {
    FILE* fp = fopen("results_format1.csv", "w");
    if(fp == NULL) {
        fprintf(stderr, "Error opening file for writing.\n");
        return;
    }
    if (comments){
    // Write headers with time and step
        fprintf(fp, "# Time: %lf\n", solver->T);
        fprintf(fp, "# Step size (h): %lf\n", solver->h);
        fprintf(fp, "# Step size (tau): %lf\n", solver->tau);
    }
    fprintf(fp, "t,x,v\n");
    for(int j = 0; j <= solver->m; ++j) {
        double t_val = j * solver->tau;
        for(int i = 0; i <= solver->n; ++i) {
            double x_val = i * solver->h;
            fprintf(fp, "%e,%e,%e\n", t_val, x_val, solver->results[j][i]);
        }
        // Update progress bar
        float progress = (float)j / solver->m;
        update_progress_bar(progress_win, progress, 0);
    }
    fclose(fp);
}

void save_csv_format2(HeatEquationSolver* solver, WINDOW *progress_win) {
    FILE* fp = fopen("results_format2.csv", "w");
    if(fp == NULL) {
        fprintf(stderr, "Error opening file for writing.\n");
        return;
    }
    // Write headers with time and step
    fprintf(fp, "# Time: %lf\n", solver->T);
    fprintf(fp, "# Step size (h): %lf\n", solver->h);
    fprintf(fp, "# Step size (tau): %lf\n", solver->tau);
    // Write x values as columns
    fprintf(fp, "t");
    for(int i = 0; i <= solver->n; ++i) {
        double x_val = i * solver->h;
        fprintf(fp, ",%e", x_val);
    }
    fprintf(fp, "\n");
    for(int j = 0; j <= solver->m; ++j) {
        double t_val = j * solver->tau;
        fprintf(fp, "%e", t_val);
        for(int i = 0; i <= solver->n; ++i) {
            fprintf(fp, ",%e", solver->results[j][i]);
        }
        fprintf(fp, "\n");
        // Update progress bar
        float progress = (float)j / solver->m;
        update_progress_bar(progress_win, progress, 0);
    }
    fclose(fp);
}