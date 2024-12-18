#include "heat_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define MAX_FILENAME_LEN 256

// Инициализация настроек ncurses
void initialize_ncurses() {
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(0);

    if (has_colors() == FALSE) {
        endwin();
        printf("Ваш терминал не поддерживает цвета.\n");
        exit(1);
    }
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
}

// Печать меню с текущими параметрами
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
    // Отображение текущих параметров
    mvwprintw(menu_win, y + 1, x, "Текущие параметры:");
    mvwprintw(menu_win, y + 2, x, "Число узлов по оси x n = %d", n);
    mvwprintw(menu_win, y + 3, x, "Число узлов по оси t m = %d", m);
    mvwprintw(menu_win, y + 4, x, "Конечное время T = %.2f", T);
    wrefresh(menu_win);
}

// Создание окна индикатора прогресса
WINDOW* create_progress_bar() {
    int height = 5, width = 70;
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;
    WINDOW *win = newwin(height, width, starty, startx);
    keypad(win, TRUE);
    box(win, 0, 0);
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 2, "Прогресс: [>                         ] 0%% Оставшееся время: Расчет...");
    wattroff(win, COLOR_PAIR(1));
    wrefresh(win);
    return win;
}

// Обновление индикатора прогресса
void update_progress_bar(WINDOW *win, float progress, double remaining_time) {
    int width = 30;
    int pos = (int)(progress * width);
    int percent = (int)(progress * 100);

    // Очистка предыдущего индикатора прогресса
    mvwprintw(win, 1, 2, "                                                                               ");
    
    // Отрисовка индикатора прогресса
    mvwprintw(win, 1, 2, "Прогресс: [");
    for(int i = 0; i < width; ++i) {
        if(i < pos)
            waddch(win, '#');
        else if(i == pos)
            waddch(win, '>');
        else
            waddch(win, ' ');
    }
    waddch(win, ']');

    // Отображение процента и оставшегося времени
    mvwprintw(win, 1, 12 + width + 2, " %3d%% Оставшееся время: %.0f сек", percent, remaining_time);
    wrefresh(win);
}

// Завершение и очистка окна индикатора прогресса
void finalize_progress_bar(WINDOW *win) {
    werase(win);
    wrefresh(win);
    delwin(win);
}

// Функция для печати результатов
void print_results(HeatEquationSolver* solver) {
    for (int j = 0; j <= solver->m; ++j) {
        for (int i = 0; i <= solver->n; ++i) {
            printf("%f ", solver->results[j][i]);
        }
        printf("\n");
    }
}

// Отображение данных в формате 1
void display_data_format1(HeatEquationSolver* solver, WINDOW *menu_win) {
    clear();
    // Вычисление размера окна данных
    int data_height = LINES - 4;
    int data_width = COLS - 4;

    // Создание окна для отображения данных
    int data_starty = 2;
    int data_startx = 2;
    WINDOW *data_win = newwin(data_height, data_width, data_starty, data_startx);
    box(data_win, 0, 0);

    int ch;
    int current_row = 0;
    int max_row = solver->m; // Предполагается, что max_row - это количество строк в данных

    do {
        // Очистка окна
        werase(data_win);
        box(data_win, 0, 0);

        // Печать заголовков
        mvwprintw(data_win, 1, 2, "t\t\tx\t\t\tv");
        mvwprintw(data_win, 2, 2, "-------------------------------------------------------------");

        // Отображение данных, начиная с current_row
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
            // Обработка клавиш влево и вправо, если необходимо
        }
    } while(ch != 27); // Клавиша ESC для выхода

    delwin(data_win);
}

// Отображение данных в формате 2
void display_data_format2(HeatEquationSolver* solver, WINDOW *menu_win) {
    clear();
    if (solver == NULL) {
        mvprintw(LINES - 1, 0, "Вычисления не выполнены. Нажмите любую клавишу для продолжения.");
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

void display_data_with_range(HeatEquationSolver* solver, WINDOW *menu_win) {
    clear();
    double start_time, end_time;
    double start_x, end_x;

    // Проверка, выполнены ли вычисления
    if (solver == NULL) {
        mvprintw(LINES - 1, 0, "Вычисления не выполнены. Нажмите любую клавишу для продолжения.");
        refresh();
        getch();
        return;
    }

    // Ввод диапазона для t и x
    echo();
    curs_set(1);
    mvprintw(LINES - 4, 0, "Введите начальное время (t_start): ");
    refresh();
    scanw("%lf", &start_time);
    mvprintw(LINES - 3, 0, "Введите конечное время (t_end): ");
    refresh();
    scanw("%lf", &end_time);
    mvprintw(LINES - 2, 0, "Введите начальное x (x_start): ");
    refresh();
    scanw("%lf", &start_x);
    mvprintw(LINES - 1, 0, "Введите конечное x (x_end): ");
    refresh();
    scanw("%lf", &end_x);
    noecho();
    curs_set(0);

    // Преобразование времени в индексы
    int start_j = (int)(start_time / solver->tau);
    int end_j = (int)(end_time / solver->tau);
    int start_i = (int)(start_x / solver->h);
    int end_i = (int)(end_x / solver->h);

    // Проверка диапазона
    if (start_j < 0) start_j = 0;
    if (end_j > solver->m) end_j = solver->m;
    if (start_i < 0) start_i = 0;
    if (end_i > solver->n) end_i = solver->n;
    if (start_j > end_j || start_i > end_i) {
        mvprintw(LINES - 1, 0, "Неверный диапазон. Нажмите любую клавишу для продолжения.");
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