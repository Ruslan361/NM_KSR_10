#include "cheat.h"

// Сохранение CSV в формате t, x, v
void save_csv_format1(HeatEquationSolver* solver, WINDOW *progress_win, int comments) {
    FILE* fp = fopen("results_format1.csv", "w");
    if(fp == NULL) {
        fprintf(stderr, "Ошибка открытия файла для записи.\n");
        return;
    }
    if (comments){
        // Запись заголовков с временем и шагом
        fprintf(fp, "# T: %e\n", solver->T);
        fprintf(fp, "# h: %e\n", solver->h);
        fprintf(fp, "# tau: %e\n", solver->tau);
    }
    fprintf(fp, "t,x,v\n");
    for(int j = 0; j <= solver->m; ++j) {
        double t_val = j * solver->tau;
        for(int i = 0; i <= solver->n; ++i) {
            double x_val = i * solver->h;
            fprintf(fp, "%e,%e,%e\n", t_val, x_val, solver->results[j][i]);
        }
        // Обновление индикатора прогресса
        float progress = (float)j / solver->m;
        update_progress_bar(progress_win, progress, 0);
    }
    fclose(fp);
}

// Сохранение CSV в формате с строками t и столбцами x
void save_csv_format2(HeatEquationSolver* solver, WINDOW *progress_win) {
    FILE* fp = fopen("results_format2.csv", "w");
    if(fp == NULL) {
        fprintf(stderr, "Ошибка открытия файла для записи.\n");
        return;
    }
    // Запись заголовков с временем и шагом
    fprintf(fp, "# Время: %e\n", solver->T);
    fprintf(fp, "# Размер шага (h): %e\n", solver->h);
    fprintf(fp, "# Размер шага (tau): %e\n", solver->tau);
    // Запись значений x как столбцов
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
        // Обновление индикатора прогресса
        float progress = (float)j / solver->m;
        update_progress_bar(progress_win, progress, 0);
    }
    fclose(fp);
}

// Функция для обработки ошибки выделения памяти для решателя
void handle_solver_error() {
    WINDOW *error_win = newwin(3, 50, (LINES - 3) / 2, (COLS - 50) / 2);
    mvwprintw(error_win, 1, 1, "Не удалось выделить память для HeatEquationSolver.");
    wrefresh(error_win);
    wgetch(error_win);
}

// Функция для создания нового экземпляра решателя
HeatEquationSolver* initialize_solver(int n, int m, double T) {
    HeatEquationSolver* solver = createHeatEquationSolver(n, m, T);
    if(solver == NULL){
        handle_solver_error();
    }
    return solver;
}

// Обновленная функция start_computation
void start_computation(HeatEquationSolver** solver_ptr, int n, int m, double T) {
    if(*solver_ptr != NULL){
        freeHeatEquationSolver(*solver_ptr);
        *solver_ptr = NULL;
    }
    *solver_ptr = initialize_solver(n, m, T);
    if(*solver_ptr == NULL){
        return;
    }

    // Создание индикатора прогресса
    WINDOW *progress_win = create_progress_bar();

    // Начало решения
    clock_t start_time = clock();
    solve(*solver_ptr, progress_win, start_time);

    // Завершение индикатора прогресса
    finalize_progress_bar(progress_win);

    // Уведомление пользователя о завершении
    WINDOW *success_win = newwin(3, 50, (LINES - 3) / 2, (COLS - 50) / 2);
    mvwprintw(success_win, 1, 1, "Вычисления успешно завершены.");
    wrefresh(success_win);
    wgetch(success_win);
    delwin(success_win);
}

void search_data_by_time(HeatEquationSolver* solver, WINDOW *menu_win) {
    clear();
    if (solver == NULL) {
        mvprintw(LINES - 1, 0, "Вычисления не выполнены. Нажмите любую клавишу для продолжения.");
        refresh();
        getch();
        return;
    }

    double t_query;
    int N;

    // Ввод времени и количества ближайших шагов
    echo();
    curs_set(1);
    mvprintw(LINES - 4, 0, "Введите время запроса (t): ");
    refresh();
    scanw("%lf", &t_query);
    mvprintw(LINES - 3, 0, "Введите количество ближайших шагов (N): ");
    refresh();
    scanw("%d", &N);
    noecho();
    curs_set(0);

    // Проверка N
    if (N <= 0 || N > solver->m + 1) {
        mvprintw(LINES - 1, 0, "Неверное значение N. Нажмите любую клавишу для продолжения.");
        refresh();
        getch();
        return;
    }

    // Поиск N ближайших временных шагов
    typedef struct {
        int index;
        double diff;
    } TimeDiff;

    TimeDiff* diffs = malloc((solver->m + 1) * sizeof(TimeDiff));
    for(int j = 0; j <= solver->m; ++j) {
        diffs[j].index = j;
        diffs[j].diff = fabs(j * solver->tau - t_query);
    }

    // Сортировка по возрастанию разницы
    for(int i = 0; i < solver->m; ++i) {
        for(int j = i + 1; j <= solver->m; ++j) {
            if(diffs[i].diff > diffs[j].diff) {
                TimeDiff temp = diffs[i];
                diffs[i] = diffs[j];
                diffs[j] = temp;
            }
        }
    }

    // Выбор N ближайших
    int* selected_indices = malloc(N * sizeof(int));
    for(int i = 0; i < N; ++i) {
        selected_indices[i] = diffs[i].index;
    }

    // Сортировка выбранных по t
    for(int i = 0; i < N - 1; ++i) {
        for(int j = i + 1; j < N; ++j) {
            if(selected_indices[i] > selected_indices[j]) {
                int temp = selected_indices[i];
                selected_indices[i] = selected_indices[j];
                selected_indices[j] = temp;
            }
        }
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
        mvwprintw(data_win, 1, 15 + i * 20, "%.2e", i * solver->h);
    }

    // Отображение данных
    for(int k = 0; k < N && (k + 2) < data_height; ++k) {
        int j = selected_indices[k];
        mvwprintw(data_win, k + 2, 2, "%.2e", j * solver->tau);
        for(int i = 0; i <= solver->n && (15 + i * 20) < data_width; ++i) {
            mvwprintw(data_win, k + 2, 15 + i * 20, "%.2e", solver->results[j][i]);
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

    free(diffs);
    free(selected_indices);
}

void analyze_data(HeatEquationSolver* solver, WINDOW *menu_win) {
    if (solver == NULL) {
        mvprintw(LINES - 1, 0, "Вычисления не выполнены. Нажмите любую клавишу для продолжения.");
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

    // Создание окна для отображения результатов анализа
    WINDOW *result_win = newwin(12, 60, (LINES - 12) / 2, (COLS - 60) / 2);
    box(result_win, 0, 0);
    mvwprintw(result_win, 1, 2, "Результаты анализа:");
    mvwprintw(result_win, 3, 2, "Максимальное значение: %.2e при (t=%.2e, x=%.2e)", v_max, t_max, x_max);
    mvwprintw(result_win, 4, 2, "Минимальное значение: %.2e при (t=%.2e, x=%.2e)", v_min, t_min, x_min);
    mvwprintw(result_win, 6, 2, "Параметры вычислений:");
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

    // Ввод T и размера сетки
    echo();
    curs_set(1);
    mvprintw(LINES - 6, 0, "Введите общее время симуляции (T): ");
    refresh();
    scanw("%lf", &T);
    mvprintw(LINES - 5, 0, "Введите базовый размер сетки (n): ");
    refresh();
    scanw("%d", &grid_size);
    noecho();
    curs_set(0);

    int grid_size_1 = grid_size;
    int grid_size_2 = grid_size * 10;
    int grid_size_3 = grid_size_2 * 10;

    // Создание решателей
    HeatEquationSolver* solver1 = createHeatEquationSolver(grid_size_1, grid_size, T);
    HeatEquationSolver* solver2 = createHeatEquationSolver(grid_size_2, grid_size, T);
    HeatEquationSolver* solver3 = createHeatEquationSolver(grid_size_3, grid_size, T);

    if(solver1 == NULL || solver2 == NULL || solver3 == NULL){
        mvprintw(LINES - 1, 0, "Не удалось выделить память для HeatEquationSolver.");
        refresh();
        getch();
        return;
    }

    // Решение всех задач
    WINDOW *progress_win = create_progress_bar();
    clock_t start_time = clock();
    solve(solver1, progress_win, start_time);
    solve(solver2, progress_win, start_time);
    solve(solver3, progress_win, start_time);
    finalize_progress_bar(progress_win);

    // Вычисление максимальных разниц
    int step_12 = grid_size_2 / grid_size_1;
    int step_23 = grid_size_3 / grid_size_2;
    double max_difference_1 = calculate_max_difference(solver1, solver2, 1, 10);
    printf("Максимальная разница 1: %lf\n", max_difference_1);
    double max_difference_2 = calculate_max_difference(solver2, solver3, 1, 10);
    printf("Максимальная разница 2: %lf\n", max_difference_2);
    // Вычисление отношения ошибок
    double error_ratio = max_difference_1 / max_difference_2;

    // Отображение результатов
    clear();
    mvprintw(2, 2, "Максимальная разница между сетками %d и %d: %lf", grid_size_1, grid_size_2, max_difference_1);
    mvprintw(3, 2, "Максимальная разница между сетками %d и %d: %lf", grid_size_2, grid_size_3, max_difference_2);
    mvprintw(5, 2, "Отношение ошибок (E(h)/E(h/10)): %lf", error_ratio);
    mvprintw(7, 2, "Если метод имеет порядок сходимости 2, отношение ошибок должно быть примерно 100.");
    mvprintw(9, 2, "Нажмите любую клавишу для продолжения.");
    refresh();
    getch();

    // Освобождение памяти
    freeHeatEquationSolver(solver1);
    freeHeatEquationSolver(solver2);
    freeHeatEquationSolver(solver3);
}

void find_difference_between_grids() {
    clear();
    double T;
    int grid_size1, grid_size2;

    // Ввод T и двух размеров сетки
    echo();
    curs_set(1);
    mvprintw(LINES - 6, 0, "Введите общее время симуляции (T): ");
    refresh();
    scanw("%lf", &T);
    mvprintw(LINES - 5, 0, "Введите первый размер сетки (n1): ");
    refresh();
    scanw("%d", &grid_size1);
    mvprintw(LINES - 4, 0, "Введите второй размер сетки (n2): ");
    refresh();
    scanw("%d", &grid_size2);
    noecho();
    curs_set(0);

    // Проверка, что grid_size2 является кратным grid_size1
    if (grid_size2 % grid_size1 != 0) {
        mvprintw(LINES - 3, 0, "Ошибка: n2 должен быть кратным n1. Нажмите любую клавишу для продолжения.");
        refresh();
        getch();
        return;
    }

    // Создание решателей
    HeatEquationSolver* solver1 = createHeatEquationSolver(grid_size1, grid_size1, T);
    HeatEquationSolver* solver2 = createHeatEquationSolver(grid_size2, grid_size1, T);

    if(solver1 == NULL || solver2 == NULL){
        mvprintw(LINES - 1, 0, "Не удалось выделить память для HeatEquationSolver.");
        refresh();
        getch();
        return;
    }

    // Решение обеих задач
    WINDOW *progress_win = create_progress_bar();
    clock_t start_time = clock();
    solve(solver1, progress_win, start_time);
    solve(solver2, progress_win, start_time);
    finalize_progress_bar(progress_win);

    // Вычисление максимальной разницы
    int step_j = grid_size2 / grid_size1;
    int step_i = grid_size2 / grid_size1;
    double max_difference = calculate_max_difference(solver1, solver2, 1, step_i);

    // Отображение результата
    clear();
    mvprintw(2, 2, "Максимальная разница между сетками %d и %d: %lf", grid_size1, grid_size2, max_difference);
    mvprintw(4, 2, "Нажмите любую клавишу для продолжения.");
    refresh();
    getch();

    // Освобождение памяти
    freeHeatEquationSolver(solver1);
    freeHeatEquationSolver(solver2);
}

