#include "cheat.h"
#include "ncurses.h"



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

    #define NUM_CHOICES 15
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
        "14. Compare Solutions with Different Grid Sizes",
        "15. Find Difference Between Two Grids" // New menu option
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
                            return 1;
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
                            return 1;
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
                        return 1;
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
                        return 1;
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
                else if(choice == 15) {
                    // Find difference between two grids
                    find_difference_between_grids();
                }
                break;
            default:
                break;
        }
    }

    endwin();
    return 0;
}