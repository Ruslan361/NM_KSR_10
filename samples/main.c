#include "cheat.h"
#include "ncurses.h"
#include <locale.h>

// Главная функция
int main() {
    setlocale(LC_ALL, "");
    int n = 100;
    int m = 100;
    double T = 10.0;
    HeatEquationSolver* solver = NULL;

    // Инициализация ncurses
    initialize_ncurses();

    // Создание окна меню
    int width = 80;
    int height = 26;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;
    WINDOW *menu_win = newwin(height, width, starty, startx);
    keypad(menu_win, TRUE);

    #define NUM_CHOICES 15
    char *choices[NUM_CHOICES] = {
        "1. Установить параметры n, m, T",
        "2. Начать вычисления",
        "3. Нарисовать 3D график",
        "4. Сохранить в XLS",
        "5. Сохранить в CSV (Формат 1: t, x, v)",
        "6. Сохранить в CSV (Формат 2: строки t, столбцы x)",
        "7. Сохранить 3D график (--save_3d_plot)",
        "8. Отобразить данные",
        "9. Отобразить данные (Формат 2)",
        "10. Выход",
        "11. Отобразить данные с выбором диапазона",
        "12. Поиск данных по времени",
        "13. Анализ данных",
        "14. Сравнить решения с разными размерами сетки",
        "15. Найти разницу между двумя сетками" // Новый пункт меню
    };

    int choice = 0;
    int highlight = 1;

    // Главный цикл
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
                    // Установить параметры
                    echo();
                    curs_set(1);
                    mvprintw(starty + height, 0, "Введите n: ");
                    clrtoeol();
                    refresh();
                    scanw("%d", &n);
                    mvprintw(starty + height + 1, 0, "Введите m: ");
                    clrtoeol();
                    refresh();
                    scanw("%d", &m);
                    mvprintw(starty + height + 2, 0, "Введите T: ");
                    clrtoeol();
                    refresh();
                    scanw("%lf", &T);
                    noecho();
                    curs_set(0);
                    // Очистить ввод
                    for(int i = 0; i < 3; ++i) {
                        move(starty + height + i, 0);
                        clrtoeol();
                    }
                    refresh();
                }
                else if(choice == 2) {
                    // Начать вычисления
                    start_computation(&solver, n, m, T);
                }
                else if(choice == 3) {
                    // Нарисовать 3D график
                    if(solver == NULL){
                        WINDOW *info_win = newwin(5, 50, starty + height + 2, startx + 15);
                        mvwprintw(info_win, 1, 1, "Нет данных. Пожалуйста, сначала выполните симуляцию.");
                        wrefresh(info_win);
                        wgetch(info_win);
                        delwin(info_win);
                    } else {
                        WINDOW *progress_win = create_progress_bar();
                        // Формирование команды для рисования
                        char command[512];
                        save_csv_format1(solver, progress_win, 0);
                        snprintf(command, sizeof(command),
                                 "./venv/bin/python ./show.py results_format1.csv False True > /dev/null");

                        int ret = system(command);
                        if (ret == -1) {
                            // Обработка ошибки
                            mvprintw(LINES - 1, 0, "Ошибка выполнения команды. Нажмите любую клавишу для продолжения.");
                            refresh();
                            getch();
                            return 1;
                        }
                        refresh();
                        finalize_progress_bar(progress_win);
                        // Уведомление пользователя
                        WINDOW *success_win = newwin(3, 50, starty + height + 2, startx + 15);
                        mvwprintw(success_win, 1, 1, "3D график успешно нарисован.");
                        wrefresh(success_win);
                        wgetch(success_win);
                        delwin(success_win);
                    }
                }
                else if(choice == 4) {
                    // Сохранить в XLS
                    if(solver == NULL){
                        WINDOW *info_win = newwin(5, 50, starty + height + 2, startx + 15);
                        mvwprintw(info_win, 1, 1, "Нет данных. Пожалуйста, сначала выполните симуляцию.");
                        wrefresh(info_win);
                        wgetch(info_win);
                        delwin(info_win);
                    } else {
                        WINDOW *progress_win = create_progress_bar();
                        // Формирование команды для сохранения в XLS
                        char command[512];
                        save_csv_format1(solver, progress_win, 0);
                        snprintf(command, sizeof(command),
                                 "./venv/bin/python ./show.py results_format1.csv True False > /dev/null");
                        int ret = system(command);
                        if (ret == -1) {
                            // Обработка ошибки
                            mvprintw(LINES - 1, 0, "Ошибка выполнения команды. Нажмите любую клавишу для продолжения.");
                            refresh();
                            getch();
                            return 1;
                        }
                        
                        finalize_progress_bar(progress_win);
                        refresh();
                        // Уведомление пользователя
                        WINDOW *success_win = newwin(3, 50, starty + height + 2, startx + 15);
                        mvwprintw(success_win, 1, 1, "Данные успешно сохранены в XLS.");
                        wrefresh(success_win);
                        wgetch(success_win);
                        delwin(success_win);
                    }
                }
                else if(choice == 5) {
                    // Сохранить в CSV (Формат 1: t, x, v)
                    if(solver == NULL){
                        start_computation(&solver, n, m, T);
                    }
                    WINDOW *progress_win = create_progress_bar();
                    save_csv_format1(solver, progress_win, 1);
                    finalize_progress_bar(progress_win);
                    // Уведомление пользователя
                    WINDOW *success_win = newwin(3, 50, starty + height + 2, startx + 15);
                    mvwprintw(success_win, 1, 1, "Данные успешно сохранены в CSV Формат 1.");
                    wrefresh(success_win);
                    wgetch(success_win);
                    delwin(success_win);
                }
                else if(choice == 6) {
                    // Сохранить в CSV (Формат 2: строки t, столбцы x)
                    if(solver == NULL){
                        start_computation(&solver, n, m, T);
                    }
                    WINDOW *progress_win = create_progress_bar();
                    save_csv_format2(solver, progress_win);
                    finalize_progress_bar(progress_win);
                    // Уведомление пользователя
                    WINDOW *success_win = newwin(3, 50, starty + height + 2, startx + 15);
                    mvwprintw(success_win, 1, 1, "Данные успешно сохранены в CSV Формат 2.");
                    wrefresh(success_win);
                    wgetch(success_win);
                    delwin(success_win);
                }
                else if(choice == 7) {
                    // Сохранить 3D график
                    if(solver == NULL){
                        start_computation(&solver, n, m, T);
                    }
                    WINDOW *progress_win = create_progress_bar();
                    save_csv_format1(solver, progress_win, 0);
                    finalize_progress_bar(progress_win);
                    // Формирование команды для сохранения 3D графика
                    char command[512];
                    snprintf(command, sizeof(command),
                             "./venv/bin/python show.py --save_3d_plot --output_dir=plots results_format1.csv False False --save_3d_plot");
                    int ret = system(command);
                    if (ret == -1) {
                        // Обработка ошибки
                        mvprintw(LINES - 1, 0, "Ошибка выполнения команды. Нажмите любую клавишу для продолжения.");
                        refresh();
                        getch();
                        return 1;
                    }
                    refresh();
                    // Уведомление пользователя
                    WINDOW *success_win = newwin(3, 50, starty + height + 2, startx + 15);
                    mvwprintw(success_win, 1, 1, "3D график успешно сохранен.");
                    wrefresh(success_win);
                    wgetch(success_win);
                    delwin(success_win);
                }
                else if(choice == 8) {
                    // Отобразить данные Формат 1
                    if(solver == NULL){
                        WINDOW *info_win = newwin(5, 50, starty + height + 2, startx + 15);
                        mvwprintw(info_win, 1, 1, "Нет данных. Пожалуйста, сначала выполните симуляцию.");
                        wrefresh(info_win);
                        wgetch(info_win);
                        delwin(info_win);
                    } else {
                        display_data_format1(solver, menu_win);
                        // Перерисовка меню после отображения данных
                        werase(menu_win);
                        refresh();
                    }
                }
                else if(choice == 9) {
                    // Отобразить данные Формат 2
                    if(solver == NULL) {
                        mvprintw(LINES - 1, 0, "Вычисления не выполнены. Нажмите любую клавишу для продолжения.");
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
                    // Выход
                    if(solver != NULL){
                        freeHeatEquationSolver(solver);
                    }
                    endwin();
                    return 0;
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
                    // Найти разницу между двумя сетками
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