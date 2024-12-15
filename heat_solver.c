#include "heat_solver.h"
void update_progress_bar1(WINDOW *win, float progress, double remaining_time);



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
    // double aa = -5/(solver->h*solver->h);
    // double bb = 1/solver->tau + 5/(solver->h*solver->h);
    
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

// // // Function to solve the heat equation
// // void solve(HeatEquationSolver* solver) {
// //     for (int i = 0; i <= solver->n; ++i) {
// //         double x = i * solver->h;
// //         solver->results[0][i] = initial_condition(x);
// //     }

// //     for (int j = 1; j <= solver->m; ++j) {
// //         double a[solver->n + 1], b[solver->n + 1], c[solver->n + 1], d[solver->n + 1];
// //         double u[solver->n + 1];

// //         calculate_coefficients(solver, a, b, c, d, j);

// //         for (int i = 1; i <= solver->n; ++i) {
// //             double w = a[i] / b[i - 1];
// //             b[i] -= w * c[i - 1];
// //             d[i] -= w * d[i - 1];
// //         }

// //         u[solver->n] = d[solver->n] / b[solver->n];
// //         for (int i = solver->n - 1; i >= 0; --i) {
// //             u[i] = (d[i] - c[i] * u[i + 1]) / b[i];
// //         }

// //         for(int i = 0; i <= solver->n; ++i) {
// //             solver->results[j][i] = u[i];
// //         }
// //     }
// // }

// // // Function to print the results
// // void print_results(HeatEquationSolver* solver) {
// //     for (int j = 0; j <= solver->m; ++j) {
// //         for (int i = 0; i <= solver->n; ++i) {
// //             printf("%f ", solver->results[j][i]);
// //         }
// //         printf("\n");
// //     }
// // }

// #include "heat_solver.h"
// #include <ncurses.h>
// #include <time.h>

// // // Function to create and initialize a HeatEquationSolver
// // HeatEquationSolver* createHeatEquationSolver(int n, int m, double T) {
// //     HeatEquationSolver* solver = (HeatEquationSolver*)malloc(sizeof(HeatEquationSolver));
// //     if (solver == NULL) {
// //         fprintf(stderr, "Memory allocation failed for HeatEquationSolver.\n");
// //         exit(1);
// //     }

// //     solver->n = n;
// //     solver->m = m;
// //     solver->T = T;
// //     solver->h = 1.0 / n;
// //     solver->tau = T / m;

// //     // Allocate memory for results
// //     solver->results = (double **)malloc((m + 1) * sizeof(double *));
// //     for(int j = 0; j <= m; ++j) {
// //         solver->results[j] = (double *)malloc((n + 1) * sizeof(double));
// //     }

// //     return solver;
// // }

// // // Function to free the allocated memory
// // void freeHeatEquationSolver(HeatEquationSolver* solver) {
// //     for(int j = 0; j <= solver->m; ++j) {
// //         free(solver->results[j]);
// //     }
// //     free(solver->results);
// //     free(solver);
// // }

// // // Boundary condition functions
// // double boundary_left(double t) { return 0.0; } // Example
// // double boundary_right(double t) { return 0.0; } // Example

// // // Initial condition function
// // double initial_condition(double x) { return sin(M_PI * x); } // Example

// // // Source term function
// // double source_term(double t) { return 0.0; } // Example

// // // Function to calculate coefficients
// // void calculate_coefficients(HeatEquationSolver* solver, double *a, double *b, double *c, double *d, int j) {
// //     double r = solver->alpha * solver->tau / (solver->h * solver->h);
// //     for(int i = 1; i < solver->n; ++i) {
// //         a[i] = -r;
// //         b[i] = 1 + 2 * r;
// //         c[i] = -r;
// //         d[i] = solver->results[j-1][i] + solver->tau * source_term(j * solver->tau);
// //     }
// //     // Boundary conditions
// //     b[0] = 1.0;
// //     d[0] = boundary_left(j * solver->tau);
// //     b[solver->n] = 1.0;
// //     d[solver->n] = boundary_right(j * solver->tau);
// // }

// // Function to solve the heat equation
// void solve(HeatEquationSolver* solver, WINDOW* win, clock_t start_time) {
//     // Initialize the first row with initial condition
//     for (int i = 0; i <= solver->n; ++i) {
//         double x = i * solver->h;
//         solver->results[0][i] = initial_condition(x);
//     }

//     for (int j = 1; j <= solver->m; ++j) {
//         double a[solver->n + 1], b[solver->n + 1], c[solver->n + 1], d[solver->n + 1];
//         double u[solver->n + 1];

//         calculate_coefficients(solver, a, b, c, d, j);

//         // Forward sweep
//         for (int i = 1; i <= solver->n; ++i) {
//             double w = a[i] / b[i - 1];
//             b[i] -= w * c[i - 1];
//             d[i] -= w * d[i - 1];
//         }

//         // Back substitution
//         u[solver->n] = d[solver->n] / b[solver->n];
//         for (int i = solver->n - 1; i >= 0; --i) {
//             u[i] = (d[i] - c[i] * u[i + 1]) / b[i];
//         }

//         for(int i = 0; i <= solver->n; ++i) {
//             solver->results[j][i] = u[i];
//         }

//         // Update progress bar
//         float progress = (float)j / solver->m;
//         int percent = (int)(progress * 100);
//         clock_t current_time = clock();
//         double elapsed = (double)(current_time - start_time) / CLOCKS_PER_SEC;
//         double estimated_total = elapsed / progress;
//         double remaining = estimated_total - elapsed;

//         update_progress_bar1(win, progress, remaining);
//     }
// }

// // // Function to print the results (optional)
// // void print_results(HeatEquationSolver* solver) {
// //     for(int j = 0; j <= solver->m; ++j) {
// //         for(int i = 0; i <= solver->n; ++i) {
// //             printf("%lf ", solver->results[j][i]);
// //         }
// //         printf("\n");
// //     }
// // }


// #include "heat_solver.h"
#include <ncurses.h>
#include <time.h>

// // Function to create and initialize a HeatEquationSolver
// HeatEquationSolver* createHeatEquationSolver(int n, int m, double T) {
//     HeatEquationSolver* solver = (HeatEquationSolver*)malloc(sizeof(HeatEquationSolver));
//     if (solver == NULL) {
//         fprintf(stderr, "Memory allocation failed for HeatEquationSolver.\n");
//         exit(1);
//     }

//     solver->n = n;
//     solver->m = m;
//     solver->T = T;
//     solver->h = 1.0 / n;
//     solver->tau = T / m;

//     // Allocate memory for results
//     solver->results = (double **)malloc((m + 1) * sizeof(double *));
//     for(int j = 0; j <= m; ++j) {
//         solver->results[j] = (double *)malloc((n + 1) * sizeof(double));
//     }

//     return solver;
// }

// // Function to free the allocated memory
// void freeHeatEquationSolver(HeatEquationSolver* solver) {
//     for(int j = 0; j <= solver->m; ++j) {
//         free(solver->results[j]);
//     }
//     free(solver->results);
//     free(solver);
// }


// Function to solve the heat equation
void solve(HeatEquationSolver* solver, WINDOW* win, clock_t start_time) {
    // Initialize the first row with initial condition
    for (int i = 0; i <= solver->n; ++i) {
        double x = i * solver->h;
        solver->results[0][i] = initial_condition(x);
    }

    for (int j = 1; j <= solver->m; ++j) {
        double a[solver->n + 1], b[solver->n + 1], c[solver->n + 1], d[solver->n + 1];
        double u[solver->n + 1];

        calculate_coefficients(solver, a, b, c, d, j);

        // Forward sweep
        for (int i = 1; i <= solver->n; ++i) {
            double w = a[i] / b[i - 1];
            b[i] -= w * c[i - 1];
            d[i] -= w * d[i - 1];
        }

        // Back substitution
        u[solver->n] = d[solver->n] / b[solver->n];
        for (int i = solver->n - 1; i >= 0; --i) {
            u[i] = (d[i] - c[i] * u[i + 1]) / b[i];
        }

        for(int i = 0; i <= solver->n; ++i) {
            solver->results[j][i] = u[i];
        }

        // Update progress bar
        float progress = (float)j / solver->m;
        int percent = (int)(progress * 100);
        clock_t current_time = clock();
        double elapsed = (double)(current_time - start_time) / CLOCKS_PER_SEC;
        double estimated_total = elapsed / progress;
        double remaining = estimated_total - elapsed;

        update_progress_bar1(win, progress, remaining);
    }
}

// Function to print the results (optional)
void print_results(HeatEquationSolver* solver) {
    for(int j = 0; j <= solver->m; ++j) {
        for(int i = 0; i <= solver->n; ++i) {
            printf("%lf ", solver->results[j][i]);
        }
        printf("\n");
    }
}