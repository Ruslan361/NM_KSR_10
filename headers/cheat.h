#ifndef CHEAT
#define CHEAT
#include "heat_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>
// #include <locale.h>
#include <string.h>
#include <time.h>
#include <unistd.h> // Добавьте эту строку

#define MAX_FILENAME_LEN 256
// Function prototypes
void print_menu(WINDOW *menu_win, int highlight, char *choices[], int n_choices, int n, int m, double T);
void initialize_ncurses();
WINDOW* create_progress_bar();
void update_progress_bar(WINDOW *win, float progress, double remaining_time);
void finalize_progress_bar(WINDOW *win);
void save_csv_format1(HeatEquationSolver* solver, WINDOW *progress_win, int comments);
void save_csv_format2(HeatEquationSolver* solver, WINDOW *progress_win);
void display_data(WINDOW *menu_win, HeatEquationSolver* solver, int starty, int startx);
void start_computation(HeatEquationSolver** solver_ptr, int n, int m, double T);
void display_data_format1(HeatEquationSolver* solver, WINDOW *data_win);
void display_data_format2(HeatEquationSolver* solver, WINDOW *data_win);
void display_data_with_range(HeatEquationSolver* solver, WINDOW *menu_win);
void search_data_by_time(HeatEquationSolver* solver, WINDOW *menu_win);
void analyze_data(HeatEquationSolver* solver, WINDOW *menu_win);
void compare_solutions_with_different_grids();
double calculate_max_difference(HeatEquationSolver* solverA, HeatEquationSolver* solverB, int step, int stepj);
void find_difference_between_grids(); // New function prototype
HeatEquationSolver* initialize_solver(int n, int m, double T);
#endif