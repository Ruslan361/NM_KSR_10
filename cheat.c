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

// Main function
int main() {
    // setlocale(LC_ALL, "");
    setenv("QT_QPA_PLATFORM", "xcb", 1);
    int n = 100;
    int m = 100;
    double T = 10.0;
    HeatEquationSolver* solver = NULL;

    // Initialize ncurses
    initialize_ncurses();

    // Create menu window
    int width = 80;
    int height = 26;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;
    WINDOW *menu_win = newwin(height, width, starty, startx);
    keypad(menu_win, TRUE);

    #define NUM_CHOICES 14
    char *choices[NUM_CHOICES] = {
        "1. Set parameters n, m, T",
        "2. Start Computation",
        "3. Draw 3D plot",
        "4. Save to XLS",
        "5. Save to CSV (Format 1: t, x, v)",
        "6. Save to CSV (Format 2: t rows, x columns)",
        "7. Save 3D plot (--save_3d_plot)",
        "8. Display Data",
        "9. Display Data (Format 2)",
        "10. Exit",
        "11. Display Data with Range Selection",
        "12. Search Data by Time",
        "13. Analyze Data",
        "14. Compare Solutions with Different Grids"
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
                else if(choice == 2) {
                    // Start Computation
                    start_computation(&solver, n, m, T);
                }
                else if(choice == 3) {
                    // Draw 3D plotf
                    if(solver == NULL){
                        WINDOW *info_win = newwin(5, 50, starty + height + 2, startx + 15);
                        mvwprintw(info_win, 1, 1, "No data available. Please run a simulation first.");
                        wrefresh(info_win);
                        wgetch(info_win);
                        delwin(info_win);
                    } else {
                        WINDOW *progress_win = create_progress_bar();
                        // Formulate command for drawing
                        char command[512];
                        save_csv_format1(solver, progress_win, 0);
                        snprintf(command, sizeof(command),
                                 "conda run -n pyside6 python ./show.py results_format1.csv False True > /dev/null");
                        //clear();

                        int ret = system(command);
                        if (ret == -1) {
                            // Обработка ошибки
                            mvprintw(LINES - 1, 0, "Error executing command. Press any key to continue.");
                            refresh();
                            getch();
                            return;
                        }
                        refresh();
                        //getch();
                        finalize_progress_bar(progress_win);
                        // Notify user
                        WINDOW *success_win = newwin(3, 50, starty + height + 2, startx + 15);
                        mvwprintw(success_win, 1, 1, "3D plot drawn successfully.");
                        wrefresh(success_win);
                        wgetch(success_win);
                        delwin(success_win);
                    }
                }
                else if(choice == 4) {
                    // Save to XLS
                    if(solver == NULL){
                        WINDOW *info_win = newwin(5, 50, starty + height + 2, startx + 15);
                        mvwprintw(info_win, 1, 1, "No data available. Please run a simulation first.");
                        wrefresh(info_win);
                        wgetch(info_win);
                        delwin(info_win);
                    } else {
                        WINDOW *progress_win = create_progress_bar();
                        // Formulate command for saving to XLS
                        char command[512];
                        save_csv_format1(solver, progress_win, 0);
                        snprintf(command, sizeof(command),
                                 "conda run -n pyside6 python ./show.py results_format1.csv True False > /dev/null");
                        int ret = system(command);
                        if (ret == -1) {
                            // Error handling
                            mvprintw(LINES - 1, 0, "Error executing command. Press any key to continue.");
                            refresh();
                            getch();
                            return;
                        }
                        
                        finalize_progress_bar(progress_win);
                        refresh();
                        // Notify user
                        WINDOW *success_win = newwin(3, 50, starty + height + 2, startx + 15);
                        mvwprintw(success_win, 1, 1, "Data saved to XLS successfully.");
                        wrefresh(success_win);
                        wgetch(success_win);
                        delwin(success_win);
                    }
                // Save to CSV (Format 1: t, x, v)
                if(solver == NULL){
                    WINDOW *info_win = newwin(5, 50, starty + height + 2, startx + 15);
                    mvwprintw(info_win, 1, 1, "No data available. Please run a simulation first.");
                    wrefresh(info_win);
                    wgetch(info_win);
                    delwin(info_win);
                } else {
                    WINDOW *progress_win = create_progress_bar();
                    save_csv_format1(solver, progress_win, 0);
                    finalize_progress_bar(progress_win);
                    // Notify user
                    WINDOW *success_win = newwin(3, 50, starty + height + 2, startx + 15);
                    mvwprintw(success_win, 1, 1, "Data saved to CSV Format 1 successfully.");
                    wrefresh(success_win);
                    wgetch(success_win);
                    delwin(success_win);
                }
                }
                else if(choice == 5) {
                    // Save to CSV (Format 1: t, x, v)
                    if(solver == NULL){
                        start_computation(&solver, n, m, T);
                    }
                    WINDOW *progress_win = create_progress_bar();
                    save_csv_format1(solver, progress_win, 1);
                    finalize_progress_bar(progress_win);
                    // Notify user
                    WINDOW *success_win = newwin(3, 50, starty + height + 2, startx + 15);
                    mvwprintw(success_win, 1, 1, "Data saved to CSV Format 1 successfully.");
                    wrefresh(success_win);
                    wgetch(success_win);
                    delwin(success_win);
                }
                else if(choice == 6) {
                    // Save to CSV (Format 2: t rows, x columns)
                    if(solver == NULL){
                        start_computation(&solver, n, m, T);
                    }
                    WINDOW *progress_win = create_progress_bar();
                    save_csv_format2(solver, progress_win);
                    finalize_progress_bar(progress_win);
                    // Notify user
                    WINDOW *success_win = newwin(3, 50, starty + height + 2, startx + 15);
                    mvwprintw(success_win, 1, 1, "Data saved to CSV Format 2 successfully.");
                    wrefresh(success_win);
                    wgetch(success_win);
                    delwin(success_win);
                }
                else if(choice == 7) {
                    // Save 3D plot
                    if(solver == NULL){
                        start_computation(&solver, n, m, T);
                    }
                    //save_csv_format1(solver, NULL, 0);
                    WINDOW *progress_win = create_progress_bar();
                    save_csv_format1(solver, progress_win, 0);
                    finalize_progress_bar(progress_win);
                    // Formulate command for saving 3D plot
                    char command[512];
                    snprintf(command, sizeof(command),
                             "conda run -n pyside6 python ./show.py results_format1.csv False False --save_3d_plot > /dev/null");
                    int ret = system(command);
                    if (ret == -1) {
                        // Обработка ошибки
                        mvprintw(LINES - 1, 0, "Error executing command. Press any key to continue.");
                        refresh();
                        getch();
                        return;
                    }
                    refresh();
                    // Notify user
                    WINDOW *success_win = newwin(3, 50, starty + height + 2, startx + 15);
                    mvwprintw(success_win, 1, 1, "3D plot saved successfully.");
                    wrefresh(success_win);
                    wgetch(success_win);
                    delwin(success_win);
                }
                else if(choice == 8) {
                    // Display Data Format 1
                    if(solver == NULL){
                        WINDOW *info_win = newwin(5, 50, starty + height + 2, startx + 15);
                        mvwprintw(info_win, 1, 1, "No data available. Please run a simulation first.");
                        wrefresh(info_win);
                        wgetch(info_win);
                        delwin(info_win);
                    } else {
                        display_data_format1(solver, menu_win);
                        // Redraw menu after displaying data
                        werase(menu_win);
                        refresh();
                    }
                }
                else if(choice == 9) {
                    // Display Data Format 2
                    if(solver == NULL) {
                        mvprintw(LINES - 1, 0, "Computation not performed. Press any key to continue.");
                        refresh();
                        getch();
                        return;
                    }

                    // Создание окна для отображения данных
                    int data_height = LINES - 6;
                    int data_width = COLS - 6;
                    int data_starty = 3;
                    int data_startx = 3;
                    WINDOW *data_win = newwin(data_height, data_width, data_starty, data_startx);
                    box(data_win, 0, 0);

                    // Заголовки с увеличенными отступами
                    mvwprintw(data_win, 1, 2, "t\\x");
                    for(int i = 0; i <= solver->n; ++i) {
                        mvwprintw(data_win, 1, 15 + i*20, "%.2e", i * solver->h);
                    }

                    // Отображение данных
                    for(int j = 0; j <= solver->m && (j +2) < data_height; ++j) {
                        mvwprintw(data_win, j +2, 2, "%.2e", j * solver->tau);
                        for(int i = 0; i <= solver->n && (15 + i*20) < data_width; ++i) {
                            mvwprintw(data_win, j +2, 15 + i*20, "%.2e", solver->results[j][i]);
                        }
                    }

                    wrefresh(data_win);
                    wgetch(data_win);
                    delwin(data_win);
                    refresh();
                }
                else if(choice == 10) {
                    // Exit
                    if(solver != NULL){
                        freeHeatEquationSolver(solver);
                    }
                    endwin();
                    return 0; // Изменено с return;
                }
                else if(choice == 11) {
                    display_data_with_range(solver, menu_win);
                    refresh();
                }
                else if(choice == 12) {
                    search_data_by_time(solver, menu_win);
                    refresh();
                }
                else if(choice == 13) {
                    analyze_data(solver, menu_win);
                    refresh();
                }
                else if(choice == 14) {
                    compare_solutions_with_different_grids();
                    refresh();
                }
                break;
            default:
                break;
        }
    }

    endwin();
    return 0;
}

// Initialize ncurses settings
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

// Print the menu with current parameters
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

// Create a progress bar window
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

// Update the progress bar
void update_progress_bar(WINDOW *win, float progress, double remaining_time) {
    int width = 30;
    int pos = (int)(progress * width);
    int percent = (int)(progress * 100);

    // Clear previous progress bar
    mvwprintw(win, 1, 2, "                                                                               ");
    
    // Draw progress bar
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

    // Display percentage and estimated time
    mvwprintw(win, 1, 12 + width + 2, " %3d%% Estimated time: %.0f sec", percent, remaining_time);
    wrefresh(win);
}

// Finalize and clear the progress bar window
void finalize_progress_bar(WINDOW *win) {
    werase(win);
    wrefresh(win);
    delwin(win);
}

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

// Display data in Format 1
void display_data_format1(HeatEquationSolver* solver, WINDOW *menu_win) {
    clear();
    // Calculate data window size
    int data_height = LINES - 4;
    int data_width = COLS - 4;

    // Create a window to display data
    int data_starty = 2;
    int data_startx = 2;
    WINDOW *data_win = newwin(data_height, data_width, data_starty, data_startx);
    box(data_win, 0, 0);

    int ch;
    int current_row = 0;
    int max_row = solver->m; // Assuming max_row is the number of rows in the data

    do {
        // Clear the window
        werase(data_win);
        box(data_win, 0, 0);

        // Print headers
        mvwprintw(data_win, 1, 2, "t\t\tx\t\t\tv");
        mvwprintw(data_win, 2, 2, "-------------------------------------------------------------");

        // Display data starting from current_row
        int display_row = 3;
        for(int j = current_row; j <= solver->m && display_row < data_height - 1; ++j) {
            for(int i = 0; i <= solver->n && display_row < data_height - 1; ++i) {
                wattron(data_win, A_BOLD);
                mvwprintw(data_win, display_row, 2, "%.2lf\t%.2lf\t%.2lf", j * solver->tau, i * solver->h, solver->results[j][i]);
                wattroff(data_win, A_BOLD);
                display_row++;
            }
        }

        wrefresh(data_win);
        ch = wgetch(data_win);
        switch(ch) {
            case KEY_UP:
                if(current_row > 0) current_row--;
                break;
            case KEY_DOWN:
                if(current_row < max_row) current_row++;
                break;
            // Handle left and right if needed
        }
    } while(ch != 27); // ESC key to exit

    delwin(data_win);
}

// Display data in Format 2
void display_data_format2(HeatEquationSolver* solver, WINDOW *menu_win) {
    clear();
    if (solver == NULL) {
        mvprintw(LINES - 1, 0, "Computation not performed. Press any key to continue.");
        refresh();
        getch();
        return;
    }

    // Создание окна для отображения данных
    int data_height = LINES - 6;
    int data_width = COLS - 6;
    int data_starty = 3;
    int data_startx = 3;
    WINDOW *data_win = newwin(data_height, data_width, data_starty, data_startx);
    box(data_win, 0, 0);

    // Заголовки с увеличенными отступами
    mvwprintw(data_win, 1, 2, "t\\x");
    for(int i = 0; i <= solver->n; ++i) {
        mvwprintw(data_win, 1, 15 + i*20, "%.e", i * solver->h);
    }

    // Отображение данных
    for(int j = 0; j <= solver->m && (j +2) < data_height; ++j) {
        mvwprintw(data_win, j +2, 2, "%.2e", j * solver->tau);
        for(int i = 0; i <= solver->n && (15 + i*20) < data_width; ++i) {
            mvwprintw(data_win, j +2, 15 + i*20, "%.2e", solver->results[j][i]);
        }
    }

    wrefresh(data_win);
    wgetch(data_win);
    delwin(data_win);
    clear();
}

// Function to start computation
void start_computation(HeatEquationSolver** solver_ptr, int n, int m, double T) {
    // Free existing solver if any
    if(*solver_ptr != NULL){
        freeHeatEquationSolver(*solver_ptr);
        *solver_ptr = NULL;
    }

    // Create a new solver instance
    *solver_ptr = createHeatEquationSolver(n, m, T);
    if(*solver_ptr == NULL){
        WINDOW *error_win = newwin(3, 50, (LINES - 3) / 2, (COLS - 50) / 2);
        mvwprintw(error_win, 1, 1, "Failed to allocate HeatEquationSolver.");
        wrefresh(error_win);
        wgetch(error_win);
        delwin(error_win);
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

void display_data_with_range(HeatEquationSolver* solver, WINDOW *menu_win) {
    clear();
    double start_time, end_time;
    double start_x, end_x;

    // Check if computation is done
    if (solver == NULL) {
        mvprintw(LINES - 1, 0, "Computation not performed. Press any key to continue.");
        refresh();
        getch();
        return;
    }

    // Input range for t and x
    echo();
    curs_set(1);
    mvprintw(LINES - 4, 0, "Enter start time (t_start): ");
    refresh();
    scanw("%lf", &start_time);
    mvprintw(LINES - 3, 0, "Enter end time (t_end): ");
    refresh();
    scanw("%lf", &end_time);
    mvprintw(LINES - 2, 0, "Enter start x (x_start): ");
    refresh();
    scanw("%lf", &start_x);
    mvprintw(LINES - 1, 0, "Enter end x (x_end): ");
    refresh();
    scanw("%lf", &end_x);
    noecho();
    curs_set(0);

    // Convert time to indices
    int start_j = (int)(start_time / solver->tau);
    int end_j = (int)(end_time / solver->tau);
    int start_i = (int)(start_x / solver->h);
    int end_i = (int)(end_x / solver->h);

    // Validate range
    if (start_j < 0) start_j = 0;
    if (end_j > solver->m) end_j = solver->m;
    if (start_i < 0) start_i = 0;
    if (end_i > solver->n) end_i = solver->n;
    if (start_j > end_j || start_i > end_i) {
        mvprintw(LINES - 1, 0, "Invalid range. Press any key to continue.");
        refresh();
        getch();
        return;
    }

    // Создание окна для отображения данных
    int data_height = LINES - 6;
    int data_width = COLS - 4;
    int data_starty = 2;
    int data_startx = 2;
    WINDOW *data_win = newwin(data_height, data_width, data_starty, data_startx);
    box(data_win, 0, 0);

    // Заголовки
    mvwprintw(data_win, 1, 2, "t\\x");
    int col_width = 15;
    int max_cols = (data_width - 10) / col_width; // Максимальное количество столбцов, которые помещаются в окно
    int num_cols = end_i - start_i + 1;
    int display_cols = num_cols > max_cols ? max_cols : num_cols;

    for(int i = 0; i < display_cols; ++i) {
        mvwprintw(data_win, 1, 12 + i * col_width, "%.2e", (start_i + i) * solver->h);
    }

    // Отображение данных
    for(int j = start_j, row = 2; j <= end_j && row < data_height - 1; ++j, ++row) {
        mvwprintw(data_win, row, 2, "%.2e", j * solver->tau);
        for(int i = 0; i < display_cols; ++i) {
            mvwprintw(data_win, row, 12 + i * col_width, "%.2e", solver->results[j][start_i + i]);
        }
    }

    wrefresh(data_win);

    // Навигация и выход
    int ch;
    while((ch = wgetch(data_win)) != 'q' && ch != 'Q') {
        // Можно добавить обработку навигации, если необходимо
    }

    werase(data_win);
    wrefresh(data_win);
    delwin(data_win);
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
    HeatEquationSolver* solver1 = createHeatEquationSolver(grid_size_1, grid_size_1, T);
    HeatEquationSolver* solver2 = createHeatEquationSolver(grid_size_2, grid_size_2, T);
    HeatEquationSolver* solver3 = createHeatEquationSolver(grid_size_3, grid_size_3, T);

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
    double max_difference_1 = 0.0;
    double max_difference_2 = 0.0;

    // Difference between solver1 and solver2
    for(int j = 0; j <= solver1->m; ++j) {
        for(int i = 0; i <= solver1->n; ++i) {
            double diff = fabs(solver1->results[j][i] - solver2->results[j * step_12][i * step_12]);
            if(diff > max_difference_1) {
                max_difference_1 = diff;
            }
        }
    }

    // Difference between solver2 and solver3
    for(int j = 0; j <= solver2->m; ++j) {
        for(int i = 0; i <= solver2->n; ++i) {
            double diff = fabs(solver2->results[j][i] - solver3->results[j * step_23][i * step_23]);
            if(diff > max_difference_2) {
                max_difference_2 = diff;
            }
        }
    }

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

