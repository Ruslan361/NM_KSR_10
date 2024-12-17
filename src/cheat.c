#include "cheat.h"



// Save CSV in format t, x, v
void save_csv_format1(HeatEquationSolver* solver, WINDOW *progress_win, int comments) {
    FILE* fp = fopen("results_format1.csv", "w");
    if(fp == NULL) {
        fprintf(stderr, "Error opening file for writing.\n");
        return;
    }
    if (comments){
        // Write headers with time and step
        fprintf(fp, "# Time: %e\n", solver->T);
        fprintf(fp, "# Step size (h): %e\n", solver->h);
        fprintf(fp, "# Step size (tau): %e\n", solver->tau);
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

// Save CSV in format with t rows and x columns
void save_csv_format2(HeatEquationSolver* solver, WINDOW *progress_win) {
    FILE* fp = fopen("results_format2.csv", "w");
    if(fp == NULL) {
        fprintf(stderr, "Error opening file for writing.\n");
        return;
    }
    // Write headers with time and step
    fprintf(fp, "# Time: %e\n", solver->T);
    fprintf(fp, "# Step size (h): %e\n", solver->h);
    fprintf(fp, "# Step size (tau): %e\n", solver->tau);
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



// Function to handle solver allocation error
void handle_solver_error() {
    WINDOW *error_win = newwin(3, 50, (LINES - 3) / 2, (COLS - 50) / 2);
    mvwprintw(error_win, 1, 1, "Failed to allocate HeatEquationSolver.");
    wrefresh(error_win);
    wgetch(error_win);
}

// Function to create a new solver instance
HeatEquationSolver* initialize_solver(int n, int m, double T) {
    HeatEquationSolver* solver = createHeatEquationSolver(n, m, T);
    if(solver == NULL){
        handle_solver_error();
    }
    return solver;
}

// Updated start_computation function
void start_computation(HeatEquationSolver** solver_ptr, int n, int m, double T) {
    if(*solver_ptr != NULL){
        freeHeatEquationSolver(*solver_ptr);
        *solver_ptr = NULL;
    }
    *solver_ptr = initialize_solver(n, m, T);
    if(*solver_ptr == NULL){
        return;
    }

    // Create progress bar
    WINDOW *progress_win = create_progress_bar();

    // Start solving
    clock_t start_time = clock();
    solve(*solver_ptr, progress_win, start_time);

    // Finalize progress bar
    finalize_progress_bar(progress_win);

    // Notify user of completion
    WINDOW *success_win = newwin(3, 50, (LINES - 3) / 2, (COLS - 50) / 2);
    mvwprintw(success_win, 1, 1, "Computation completed successfully.");
    wrefresh(success_win);
    wgetch(success_win);
    delwin(success_win);
}



void search_data_by_time(HeatEquationSolver* solver, WINDOW *menu_win) {
    clear();
    if (solver == NULL) {
        mvprintw(LINES - 1, 0, "Computation not performed. Press any key to continue.");
        refresh();
        getch();
        return;
    }

    double t_query;
    int N;

    // Input time and number of nearest steps
    echo();
    curs_set(1);
    mvprintw(LINES - 4, 0, "Enter query time (t): ");
    refresh();
    scanw("%lf", &t_query);
    mvprintw(LINES - 3, 0, "Enter number of nearest steps (N): ");
    refresh();
    scanw("%d", &N);
    noecho();
    curs_set(0);

    // Validate N
    if (N <= 0 || N > solver->m + 1) {
        mvprintw(LINES - 1, 0, "Invalid N. Press any key to continue.");
        refresh();
        getch();
        return;
    }

    // Find N nearest time steps
    typedef struct {
        int index;
        double diff;
    } TimeDiff;

    TimeDiff* diffs = malloc((solver->m + 1) * sizeof(TimeDiff));
    for(int j = 0; j <= solver->m; ++j) {
        diffs[j].index = j;
        diffs[j].diff = fabs(j * solver->tau - t_query);
    }

    // Sort by ascending difference
    for(int i = 0; i < solver->m; ++i) {
        for(int j = i + 1; j <= solver->m; ++j) {
            if(diffs[i].diff > diffs[j].diff) {
                TimeDiff temp = diffs[i];
                diffs[i] = diffs[j];
                diffs[j] = temp;
            }
        }
    }

    // Select N nearest
    int* selected_indices = malloc(N * sizeof(int));
    for(int i = 0; i < N; ++i) {
        selected_indices[i] = diffs[i].index;
    }

    // Sort selected by t
    for(int i = 0; i < N - 1; ++i) {
        for(int j = i + 1; j < N; ++j) {
            if(selected_indices[i] > selected_indices[j]) {
                int temp = selected_indices[i];
                selected_indices[i] = selected_indices[j];
                selected_indices[j] = temp;
            }
        }
    }

    // Create a window to display data
    int data_height = LINES - 6;
    int data_width = COLS - 6;
    int data_starty = 3;
    int data_startx = 3;
    WINDOW *data_win = newwin(data_height, data_width, data_starty, data_startx);
    box(data_win, 0, 0);

    // Headers with increased spacing
    mvwprintw(data_win, 1, 2, "t\\x");
    for(int i = 0; i <= solver->n; ++i) {
        mvwprintw(data_win, 1, 15 + i * 20, "%.2e", i * solver->h);
    }

    // Display data
    for(int k = 0; k < N && (k + 2) < data_height; ++k) {
        int j = selected_indices[k];
        mvwprintw(data_win, k + 2, 2, "%.2e", j * solver->tau);
        for(int i = 0; i <= solver->n && (15 + i * 20) < data_width; ++i) {
            mvwprintw(data_win, k + 2, 15 + i * 20, "%.2e", solver->results[j][i]);
        }
    }

    wrefresh(data_win);

    // Navigation and exit
    int ch;
    while((ch = wgetch(data_win)) != 'q' && ch != 'Q') {
        // Navigation handling can be added if needed
    }

    werase(data_win);
    wrefresh(data_win);
    delwin(data_win);

    free(diffs);
    free(selected_indices);
}

void analyze_data(HeatEquationSolver* solver, WINDOW *menu_win) {
    if (solver == NULL) {
        mvprintw(LINES - 1, 0, "Computation not performed. Press any key to continue.");
        refresh();
        getch();
        return;
    }

    double v_max = solver->results[0][0];
    double v_min = solver->results[0][0];
    int i_max = 0, j_max = 0, i_min = 0, j_min = 0;

    for(int j = 0; j <= solver->m; ++j) {
        for(int i = 0; i <= solver->n; ++i) {
            double value = solver->results[j][i];
            if(value > v_max) {
                v_max = value;
                i_max = i;
                j_max = j;
            }
            if(value < v_min) {
                v_min = value;
                i_min = i;
                j_min = j;
            }
        }
    }

    double t_max = j_max * solver->tau;
    double x_max = i_max * solver->h;
    double t_min = j_min * solver->tau;
    double x_min = i_min * solver->h;

    // Create a window to display analysis results
    WINDOW *result_win = newwin(12, 60, (LINES - 12) / 2, (COLS - 60) / 2);
    box(result_win, 0, 0);
    mvwprintw(result_win, 1, 2, "Analysis Results:");
    mvwprintw(result_win, 3, 2, "Maximum value: %.2e at (t=%.2e, x=%.2e)", v_max, t_max, x_max);
    mvwprintw(result_win, 4, 2, "Minimum value: %.2e at (t=%.2e, x=%.2e)", v_min, t_min, x_min);
    mvwprintw(result_win, 6, 2, "Computation parameters:");
    mvwprintw(result_win, 7, 4, "N = %d", solver->n);
    mvwprintw(result_win, 8, 4, "M = %d", solver->m);
    mvwprintw(result_win, 9, 4, "T = %.2e", solver->T);
    mvwprintw(result_win, 10, 4, "tau = %.2e", solver->tau);
    mvwprintw(result_win, 11, 4, "h = %.2e", solver->h);
    wrefresh(result_win);
    wgetch(result_win);
    delwin(result_win);
}

double calculate_max_difference(HeatEquationSolver* solverA, HeatEquationSolver* solverB, int step_j, int step_i) {
    double max_difference = 0.0;
    for(int j = 0; j <= solverA->m && j * step_j <= solverB->m; ++j) {
        for(int i = 0; i <= solverA->n && i * step_i <= solverB->n; ++i) {
            double diff = fabs(solverA->results[j][i] - solverB->results[j * step_j][i * step_i]);
            if(diff > max_difference) {
                max_difference = diff;
            }
        }
    }
    return max_difference;
}

void compare_solutions_with_different_grids() {
    clear();
    double T;
    int grid_size;

    // Input T and grid size
    echo();
    curs_set(1);
    mvprintw(LINES - 6, 0, "Enter total simulation time (T): ");
    refresh();
    scanw("%lf", &T);
    mvprintw(LINES - 5, 0, "Enter base grid size (n): ");
    refresh();
    scanw("%d", &grid_size);
    noecho();
    curs_set(0);

    int grid_size_1 = grid_size;
    int grid_size_2 = grid_size * 10;
    int grid_size_3 = grid_size_2 * 10;

    // Create solvers
    HeatEquationSolver* solver1 = createHeatEquationSolver(grid_size_1, grid_size, T);
    HeatEquationSolver* solver2 = createHeatEquationSolver(grid_size_2, grid_size, T);
    HeatEquationSolver* solver3 = createHeatEquationSolver(grid_size_3, grid_size, T);

    if(solver1 == NULL || solver2 == NULL || solver3 == NULL){
        mvprintw(LINES - 1, 0, "Failed to allocate memory for HeatEquationSolver.");
        refresh();
        getch();
        return;
    }

    // Solve all problems
    WINDOW *progress_win = create_progress_bar();
    clock_t start_time = clock();
    solve(solver1, progress_win, start_time);
    solve(solver2, progress_win, start_time);
    solve(solver3, progress_win, start_time);
    finalize_progress_bar(progress_win);

    // Calculate maximum differences
    int step_12 = grid_size_2 / grid_size_1;
    int step_23 = grid_size_3 / grid_size_2;
    //calculate_max_difference(solver1, solver2, step_i, 1)
    double max_difference_1 = calculate_max_difference(solver1, solver2, 1, 10);
    printf("Max difference 1: %lf\n", max_difference_1);
    double max_difference_2 = calculate_max_difference(solver2, solver3, 1, 10);
    printf("Max difference 2: %lf\n", max_difference_2);
    // Calculate error ratio
    double error_ratio = max_difference_1 / max_difference_2;

    // Display results
    clear();
    mvprintw(2, 2, "Maximum difference between grids %d and %d: %lf", grid_size_1, grid_size_2, max_difference_1);
    mvprintw(3, 2, "Maximum difference between grids %d and %d: %lf", grid_size_2, grid_size_3, max_difference_2);
    mvprintw(5, 2, "Error ratio (E(h)/E(h/10)): %lf", error_ratio);
    mvprintw(7, 2, "If the method has an order of convergence of 2, the error ratio should be approximately 100.");
    mvprintw(9, 2, "Press any key to continue.");
    refresh();
    getch();

    // Free memory
    freeHeatEquationSolver(solver1);
    freeHeatEquationSolver(solver2);
    freeHeatEquationSolver(solver3);
}

void find_difference_between_grids() {
    clear();
    double T;
    int grid_size1, grid_size2;

    // Input T and two grid sizes
    echo();
    curs_set(1);
    mvprintw(LINES - 6, 0, "Enter total simulation time (T): ");
    refresh();
    scanw("%lf", &T);
    mvprintw(LINES - 5, 0, "Enter first grid size (n1): ");
    refresh();
    scanw("%d", &grid_size1);
    mvprintw(LINES - 4, 0, "Enter second grid size (n2): ");
    refresh();
    scanw("%d", &grid_size2);
    noecho();
    curs_set(0);

    // Validate that grid_size2 is a multiple of grid_size1
    if (grid_size2 % grid_size1 != 0) {
        mvprintw(LINES - 3, 0, "Error: n2 must be a multiple of n1. Press any key to continue.");
        refresh();
        getch();
        return;
    }

    // Create solvers
    HeatEquationSolver* solver1 = createHeatEquationSolver(grid_size1, grid_size1, T);
    HeatEquationSolver* solver2 = createHeatEquationSolver(grid_size2, grid_size1, T);

    if(solver1 == NULL || solver2 == NULL){
        mvprintw(LINES - 1, 0, "Failed to allocate memory for HeatEquationSolver.");
        refresh();
        getch();
        return;
    }

    // Solve both problems
    WINDOW *progress_win = create_progress_bar();
    clock_t start_time = clock();
    solve(solver1, progress_win, start_time);
    solve(solver2, progress_win, start_time);
    finalize_progress_bar(progress_win);

    // Calculate maximum difference
    int step_j = grid_size2 / grid_size1;
    int step_i = grid_size2 / grid_size1;
    double max_difference = calculate_max_difference(solver1, solver2, 1, step_i);

    // Display result
    clear();
    mvprintw(2, 2, "Maximum difference between grids %d and %d: %lf", grid_size1, grid_size2, max_difference);
    mvprintw(4, 2, "Press any key to continue.");
    refresh();
    getch();

    // Free memory
    freeHeatEquationSolver(solver1);
    freeHeatEquationSolver(solver2);
}

