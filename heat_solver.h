
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <ncurses.h>
#include <time.h>

// Structure to hold the solver parameters and results
typedef struct {
    int n;
    int m;
    double T;
    double h;
    double tau;
    double alpha;
    double **results;
} HeatEquationSolver;

HeatEquationSolver* createHeatEquationSolver(int n, int m, double T);
void freeHeatEquationSolver(HeatEquationSolver* solver);
double boundary_left(double t);
double boundary_right(double t);
double initial_condition(double x);
double source_term(double t);
void calculate_coefficients(HeatEquationSolver* solver, double *a, double *b, double *c, double *d, int j);
// void solve(HeatEquationSolver* solver);
void print_results(HeatEquationSolver* solver);
void solve(HeatEquationSolver* solver, WINDOW* win, clock_t start_time);
void update_progress_bar1(WINDOW *win, float progress, double remaining_time);