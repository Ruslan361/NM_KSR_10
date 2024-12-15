// #include "heat_solver.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <math.h>
// #include <ncurses.h>
// #include <locale.h>
// #include <string.h>

// // Function to display the menu and parameters
// void print_menu(WINDOW *menu_win, int highlight, char *choices[], int n_choices, int n, int m, double T, int save_3d_plot, int save_excel) {
//     int x, y, i;
//     x = 2;
//     y = 2;
//     box(menu_win, 0, 0);
//     for (i = 0; i < n_choices; ++i) {
//         if (highlight == i + 1) {
//             wattron(menu_win, A_REVERSE);
//             mvwprintw(menu_win, y, x, "%s", choices[i]);
//             wattroff(menu_win, A_REVERSE);
//         } else {
//             mvwprintw(menu_win, y, x, "%s", choices[i]);
//         }
//         ++y;
//     }

//     // Separator
//     mvwprintw(menu_win, y + 1, x, "-------------------------");

//     // Display current parameters
//     mvwprintw(menu_win, y + 2, x, "Current Parameters:");
//     mvwprintw(menu_win, y + 3, x, "n = %d", n);
//     mvwprintw(menu_win, y + 4, x, "m = %d", m);
//     mvwprintw(menu_win, y + 5, x, "T = %.2f", T);
//     mvwprintw(menu_win, y + 6, x, "Save 3D Plot = %s", save_3d_plot ? "Yes" : "No");
//     mvwprintw(menu_win, y + 7, x, "Save Excel = %s", save_excel ? "Yes" : "No");
//     wrefresh(menu_win);
// }

// int main() {
//     // Set locale to support English characters
//     setlocale(LC_ALL, "en_US.UTF-8");

//     int n = 100;
//     int m = 100;
//     double T = 10.0;
//     int save_3d_plot = 0;
//     int save_excel = 0;

//     // Initialize ncurses
//     initscr();
//     clear();
//     noecho();
//     cbreak();

    

//     // Optional: Check for color support
//     if (has_colors() == FALSE) {
//         endwin();
//         printf("Your terminal does not support colors.\n");
//         exit(1);
//     }
//     start_color();

//     // Define menu window size and position
//     int width = 60;    // Increased width to accommodate status text
//     int height = 15;
//     int startx = (COLS - width) / 2;
//     int starty = (LINES - height) / 2;
//     WINDOW *menu_win = newwin(height, width, starty, startx);
//     keypad(menu_win, TRUE);

//     // Define dynamic menu choices with status
//     #define NUM_CHOICES 7
//     char choices_text[NUM_CHOICES][100] = {
//         "1. Set n (default 100)",
//         "2. Set m (default 100)",
//         "3. Set T (default 10)",
//         "4. Enable/Disable Save 3D Plot (Disabled)",
//         "5. Enable/Disable Save Excel (Disabled)",
//         "6. Run Solver",
//         "7. Exit"
//     };
//     char *choices[NUM_CHOICES] = {
//         choices_text[0],
//         choices_text[1],
//         choices_text[2],
//         choices_text[3],
//         choices_text[4],
//         choices_text[5],
//         choices_text[6]
//     };

//     int choice = 0;
//     int highlight = 1;

//     while(1) {
//         print_menu(menu_win, highlight, choices, NUM_CHOICES, n, m, T, save_3d_plot, save_excel);
//         int c = wgetch(menu_win);
//         switch(c) {
//             case KEY_UP:
//                 if(highlight == 1)
//                     highlight = NUM_CHOICES;
//                 else
//                     --highlight;
//                 break;
//             case KEY_DOWN:
//                 if(highlight == NUM_CHOICES)
//                     highlight = 1;
//                 else 
//                     ++highlight;
//                 break;
//             case 10: // Enter key
//                 choice = highlight;
//                 if(choice == 1) {
//                     echo();
//                     curs_set(1);
//                     mvprintw(starty + height, 0, "Enter n: ");
//                     scanw("%d", &n);
//                     noecho();
//                     curs_set(0);
//                 }
//                 else if(choice == 2) {
//                     echo();
//                     curs_set(1);
//                     mvprintw(starty + height, 0, "Enter m: ");
//                     scanw("%d", &m);
//                     noecho();
//                     curs_set(0);
//                 }
//                 else if(choice == 3) {
//                     echo();
//                     curs_set(1);
//                     mvprintw(starty + height, 0, "Enter T: ");
//                     scanw("%lf", &T);
//                     noecho();
//                     curs_set(0);
//                 }
//                 else if(choice == 4) {
//                     //save_3d_plot = !save_3d_plot;
//                     save_3d_plot = save_3d_plot ? 0 : 1,
//                     // Update the menu choice text with the current status
//                     snprintf(choices_text[3], sizeof(choices_text[3]), "4. Enable/Disable Save 3D Plot (%s)", save_3d_plot ? "Enabled" : "Disabled");
//                 }
//                 else if(choice == 5) {
//                     //save_excel = !save_excel;
//                     save_excel = save_excel ? 0 : 1,
//                     // Update the menu choice text with the current status
//                     snprintf(choices_text[4], sizeof(choices_text[4]), "5. Enable/Disable Save Excel (%s)", save_excel ? "Enabled" : "Disabled");
//                 }
//                 else if(choice == 6) {
//                     // Close ncurses
//                     endwin();

//                     // Create and solve the problem
//                     HeatEquationSolver* solver = createHeatEquationSolver(n, m, T);
//                     solve(solver);

//                     // Save results to CSV
//                     FILE* fp = fopen("results.csv", "w");
//                     if(fp == NULL) {
//                         fprintf(stderr, "Failed to open file for writing.\n");
//                         freeHeatEquationSolver(solver);
//                         exit(1);
//                     }

//                     fprintf(fp, "t,x,v\n");
//                     for(int j = 0; j <= solver->m; ++j) {
//                         double t_val = j * solver->tau;
//                         for(int i = 0; i <= solver->n; ++i) {
//                             double x_val = i * solver->h;
//                             fprintf(fp, "%e,%e,%e\n", t_val, x_val, solver->results[j][i]);
//                         }
//                     }
//                     fclose(fp);
//                     freeHeatEquationSolver(solver);

//                     // Formulate command to run show.py with current parameters
//                     char command[512];
//                     snprintf(command, sizeof(command),
//                              "conda run -n pyside6 python ./show.py results.csv %s %s --save_3d_plot",
//                              save_excel ? "True" : "False",
//                              save_3d_plot ? "True" : "False");
//                     printf("Executing command: %s\n", command);

//                     // Execute the command
//                     int ret = system(command);
//                     (void)ret; // Explicitly ignore the return value

//                     // Reinitialize ncurses to continue
//                     initscr();
//                     clear();
//                     noecho();
//                     cbreak();
//                     menu_win = newwin(height, width, starty, startx);
//                     keypad(menu_win, TRUE);
//                 }
//                 else if(choice == 7) {
//                     endwin();
//                     return 0;
//                 }
//                 break;
//             default:
//                 break;
//         }
//     }

//     endwin();
//     return 0;
// }


#include "heat_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>
#include <locale.h>
#include <string.h>
#include <time.h>

// // Function prototypes
// void print_menu(WINDOW *menu_win, int highlight, char *choices[], int n_choices, int n, int m, double T, int save_3d_plot, int save_excel);
// void initialize_ncurses();
// WINDOW* create_progress_bar();
// void update_progress_bar(WINDOW *win, float progress, double remaining_time);
// void finalize_progress_bar(WINDOW *win);

// Function to display the menu and parameters
void print_menu(WINDOW *menu_win, int highlight, char *choices[], int n_choices, int n, int m, double T, int save_3d_plot, int save_excel) {
    int x, y, i;
    x = 2;
    y = 2;
    box(menu_win, 0, 0);
    for (i = 0; i < n_choices; ++i) {
        if (highlight == i + 1) {
            wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, y, x, "%s", choices[i]);
            wattroff(menu_win, A_REVERSE);
        } else {
            mvwprintw(menu_win, y, x, "%s", choices[i]);
        }
        ++y;
    }

    // Separator
    mvwprintw(menu_win, y + 1, x, "-------------------------");

    // Display current parameters
    mvwprintw(menu_win, y + 2, x, "Current Parameters:");
    mvwprintw(menu_win, y + 3, x, "n = %d", n);
    mvwprintw(menu_win, y + 4, x, "m = %d", m);
    mvwprintw(menu_win, y + 5, x, "T = %.2f", T);
    mvwprintw(menu_win, y + 6, x, "Save 3D Plot = %s", save_3d_plot ? "Yes" : "No");
    mvwprintw(menu_win, y + 7, x, "Save Excel = %s", save_excel ? "Yes" : "No");
    wrefresh(menu_win);
}
WINDOW* create_progress_bar1() {
    int height = 5, width = 70, starty = (LINES - 5) / 2, startx = (COLS - 70) / 2;
    WINDOW *win = newwin(height, width, starty, startx);
    box(win, 0, 0);
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 2, "Progress: [>                         ] 0%% Estimated time: Calculating...");
    wattroff(win, COLOR_PAIR(1));
    wrefresh(win);
    return win;
}
int main() {
    // Set locale to support English characters
    setlocale(LC_ALL, "en_US.UTF-8");

    int n = 100;
    int m = 100;
    double T = 10.0;
    int save_3d_plot = 0;
    int save_excel = 0;

    // Initialize ncurses
    initialize_ncurses1();

    // Define menu window size and position
    int width = 60;    // Increased width to accommodate status text
    int height = 15;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;
    WINDOW *menu_win = newwin(height, width, starty, startx);
    keypad(menu_win, TRUE);

    // Define dynamic menu choices with status
    #define NUM_CHOICES 7
    char choices_text[NUM_CHOICES][100] = {
        "1. Set n (default 100)",
        "2. Set m (default 100)",
        "3. Set T (default 10)",
        "4. Enable/Disable Save 3D Plot (Disabled)",
        "5. Enable/Disable Save Excel (Disabled)",
        "6. Run Solver",
        "7. Exit"
    };
    char *choices[NUM_CHOICES] = {
        choices_text[0],
        choices_text[1],
        choices_text[2],
        choices_text[3],
        choices_text[4],
        choices_text[5],
        choices_text[6]
    };

    int choice = 0;
    int highlight = 1;

    while(1) {
        print_menu(menu_win, highlight, choices, NUM_CHOICES, n, m, T, save_3d_plot, save_excel);
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
            case 10: // Enter key
                choice = highlight;
                if(choice == 1) {
                    echo();
                    curs_set(1);
                    mvprintw(starty + height, 0, "Enter n: ");
                    clrtoeol();
                    refresh();
                    scanw("%d", &n);
                    noecho();
                    curs_set(0);
                    // Clear the input prompt
                    move(starty + height, 0);
                    clrtoeol();
                    refresh();
                }
                else if(choice == 2) {
                    echo();
                    curs_set(1);
                    mvprintw(starty + height, 0, "Enter m: ");
                    clrtoeol();
                    refresh();
                    scanw("%d", &m);
                    noecho();
                    curs_set(0);
                    // Clear the input prompt
                    move(starty + height, 0);
                    clrtoeol();
                    refresh();
                }
                else if(choice == 3) {
                    echo();
                    curs_set(1);
                    mvprintw(starty + height, 0, "Enter T: ");
                    clrtoeol();
                    refresh();
                    scanw("%lf", &T);
                    noecho();
                    curs_set(0);
                    // Clear the input prompt
                    move(starty + height, 0);
                    clrtoeol();
                    refresh();
                }
                else if(choice == 4) {
                    // Toggle save_3d_plot
                    save_3d_plot = save_3d_plot ? 0 : 1;
                    // Update the menu choice text with the current status
                    snprintf(choices_text[3], sizeof(choices_text[3]), "4. Enable/Disable Save 3D Plot (%s)", save_3d_plot ? "Enabled" : "Disabled");
                }
                else if(choice == 5) {
                    // Toggle save_excel
                    save_excel = save_excel ? 0 : 1;
                    // Update the menu choice text with the current status
                    snprintf(choices_text[4], sizeof(choices_text[4]), "5. Enable/Disable Save Excel (%s)", save_excel ? "Enabled" : "Disabled");
                }
                else if(choice == 6) {
                    // Create progress bar window
                    WINDOW *progress_win = create_progress_bar1();

                    // Create and solve the problem
                    HeatEquationSolver* solver = createHeatEquationSolver(n, m, T);
                    clock_t start_time = clock();
                    solve(solver, progress_win, start_time);

                    // Close progress bar automatically after solving
                    delwin(progress_win);

                    // Save results to CSV
                    FILE* fp = fopen("results.csv", "w");
                    if(fp == NULL) {
                        fprintf(stderr, "Failed to open file for writing.\n");
                        freeHeatEquationSolver(solver);
                        continue;
                    }

                    fprintf(fp, "t,x,v\n");
                    for(int j = 0; j <= solver->m; ++j) {
                        double t_val = j * solver->tau;
                        for(int i = 0; i <= solver->n; ++i) {
                            double x_val = i * solver->h;
                            fprintf(fp, "%e,%e,%e\n", t_val, x_val, solver->results[j][i]);
                        }
                    }
                    fclose(fp);
                    freeHeatEquationSolver(solver);

                    // Formulate command to run show.py with current parameters
                    char command[512];
                    snprintf(command, sizeof(command),
                             "conda run -n pyside6 python ./show.py results.csv %s %s --save_3d_plot",
                             save_excel ? "True" : "False",
                             save_3d_plot ? "True" : "False");
                    mvprintw(starty + height + 2, 0, "Executing command: %s", command);
                    clrtoeol();
                    refresh();

                    // Execute the command
                    int ret = system(command);
                    (void)ret; // Explicitly ignore the return value

                    // Clear the execution message
                    move(starty + height + 2, 0);
                    clrtoeol();
                    refresh();

                    // Reinitialize ncurses to continue
                    // No need to reinitialize; continue using existing ncurses session
                }
                else if(choice == 7) {
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

void initialize_ncurses1() {
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);

    // Optional: Check for color support
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support colors.\n");
        exit(1);
    }
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
}



void update_progress_bar1(WINDOW *win, float progress, double remaining_time) {
    int width = 30; // Width inside the brackets
    int pos = (int)(progress * width);
    int percent = (int)(progress * 100);

    // Draw progress bar
    mvwprintw(win, 1, 12, "[");
    for(int i = 0; i < width; ++i) {
        if(i < pos)
            waddch(win, '#');
        else if(i == pos)
            waddch(win, '>');
        else
            waddch(win, ' ');
    }
    waddch(win, ']');

    // Print percentage and estimated time
    mvwprintw(win, 1, 12 + width + 2, " %3d%% Estimated time: %.0f sec", percent, remaining_time);
    wrefresh(win);
}

void finalize_progress_bar(WINDOW *win) {
    // Automatically close the progress bar without waiting for user input
    delwin(win);
}