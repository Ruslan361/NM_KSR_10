#ifndef NCURSES_ELEMENTS_H
#define NCURSES_ELEMENTS_H

#include "heat_solver.h"
#include <ncurses.h>

// Initialize ncurses settings
void initialize_ncurses();

// Print the menu with current parameters
void print_menu(WINDOW *menu_win, int highlight, char *choices[], int n_choices, int n, int m, double T);

// Create a progress bar window
WINDOW* create_progress_bar();

// Update the progress bar
void update_progress_bar(WINDOW *win, float progress, double remaining_time);

// Finalize and clear the progress bar window
void finalize_progress_bar(WINDOW *win);

// Function to print the results
void print_results(HeatEquationSolver* solver);

// Display data in Format 1
void display_data_format1(HeatEquationSolver* solver, WINDOW *menu_win);

// Display data in Format 2
void display_data_format2(HeatEquationSolver* solver, WINDOW *menu_win);

// Display data with range
void display_data_with_range(HeatEquationSolver* solver, WINDOW *menu_win);

#endif // NCURSES_ELEMENTS_H