#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>
#include "heat_solver.h"
// Structure to hold the solver parameters and results
// typedef struct {
//     int n;
//     int m;
//     double T;
//     double h;
//     double tau;
//     double alpha;
//     double **results;
// } HeatEquationSolver;

// Function to create and initialize a HeatEquationSolver
HeatEquationSolver* createHeatEquationSolver(int n, int m, double T) {
    HeatEquationSolver* solver = (HeatEquationSolver*)malloc(sizeof(HeatEquationSolver));
    if (solver == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1);
    }

    solver->n = n;
    solver->m = m;
    solver->T = T;
    solver->h = 1.0 / n;
    solver->tau = T / m;
    solver->alpha = 5 * solver->tau / (2 * solver->h * solver->h);
    //5 * solver->tau / (solver->h * solver->h)  
    solver->results = (double**)malloc((m + 1) * sizeof(double*));
    if (solver->results == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(1);
    }
    for (int j = 0; j <= m; ++j) {
        solver->results[j] = (double*)malloc((n + 1) * sizeof(double));
        if (solver->results[j] == NULL) {
            fprintf(stderr, "Memory allocation failed!\n");
            exit(1);
        }
        for (int i = 0; i <= n; ++i) {
            solver->results[j][i] = 0.0;
        }
    }

    return solver;
}

// Function to free the allocated memory
void freeHeatEquationSolver(HeatEquationSolver* solver) {
    for (int j = 0; j <= solver->m; ++j) {
        free(solver->results[j]);
    }
    free(solver->results);
    free(solver);
}

// Boundary condition functions
double boundary_left(double t) {
    return cos(t);
}

double boundary_right(double t) {
    return sin(4 * t);
}

// Initial condition function
double initial_condition(double x) {
    return 1 - x * x;
}

// Source term function
double source_term(double t) {
    return sin(t);
}

// Function to calculate coefficients
void calculate_coefficients(HeatEquationSolver* solver, double *a, double *b, double *c, double *d, int j) {
    double t = j * solver->tau;

    b[0] = 1.0;
    d[0] = boundary_left(t);

    b[solver->n] = 1.0;
    d[solver->n] = boundary_right(t);

    for (int i = 1; i < solver->n; ++i) {
        a[i] = -solver->alpha;
        b[i] = 1 + 2 * solver->alpha;
        c[i] = -solver->alpha;
        d[i] = solver->alpha * solver->results[j - 1][i - 1] +
               (1 - 2 * solver->alpha) * solver->results[j - 1][i] +
               solver->alpha * solver->results[j - 1][i + 1] +
               solver->tau * source_term(t - solver->tau / 2);
    }
}

// Function to solve the heat equation
void solve(HeatEquationSolver* solver, WINDOW* win, clock_t start_time) {
    for (int i = 0; i <= solver->n; ++i) {
        double x = i * solver->h;
        solver->results[0][i] = initial_condition(x);
    }

    for (int j = 1; j <= solver->m; ++j) {
        double a[solver->n + 1], b[solver->n + 1], c[solver->n + 1], d[solver->n + 1];
        double u[solver->n + 1];

        calculate_coefficients(solver, a, b, c, d, j);

        for (int i = 1; i <= solver->n; ++i) {
            double w = a[i] / b[i - 1];
            b[i] -= w * c[i - 1];
            d[i] -= w * d[i - 1];
        }

        u[solver->n] = d[solver->n] / b[solver->n];
        for (int i = solver->n - 1; i >= 0; --i) {
            u[i] = (d[i] - c[i] * u[i + 1]) / b[i];
        }

        for (int i = 0; i <= solver->n; ++i) {
            solver->results[j][i] = u[i];
        }

        // Update progress bar
        float progress = (float)j / solver->m;
        double remaining_time = (solver->m - j) * solver->tau;
        update_progress_bar(win, progress, remaining_time);
    }
}

// Function to print the results
void print_results(HeatEquationSolver* solver) {
    for (int j = 0; j <= solver->m; ++j) {
        for (int i = 0; i <= solver->n; ++i) {
            printf("%f ", solver->results[j][i]);
        }
        printf("\n");
    }
}

// int main() {
//     int n = 100;
//     int m = 100;
//     double T = 2.0*3.14;


//     HeatEquationSolver* solver = createHeatEquationSolver(n, m, T);
//     solve(solver);

//     // Save the results to CSV
//     FILE* fp = fopen("results.csv", "w");
//     if (fp == NULL) {
//         fprintf(stderr, "Failed to open file for writing.\n");
//         return 1;
//     }

//     // for (int j = 0; j <= solver->m; ++j) {
//     //     for (int i = 0; i <= solver->n; ++i) {
//     //         fprintf(fp, "%f", solver->results[j][i]);
//     //         if (i < solver->n) {
//     //             fprintf(fp, ",");
//     //         }
//     //     }
//     //     fprintf(fp, "\n");
//     // }

//     // fclose(fp);
//     fprintf(fp, "t,x,v\n");
//     for (int j = 0; j <= solver->m; ++j) {
//         double t = j * solver->tau;
//         for (int i = 0; i <= solver->n; ++i) {
//             double x = i * solver->h;
//             fprintf(fp, "%f,%f,%f\n", t, x, solver->results[j][i]);
//         }
//     }
//     fclose(fp);
//     freeHeatEquationSolver(solver);
//     return system("conda run -n NM_1 python3 ./show.py");
//     return 0;
// }