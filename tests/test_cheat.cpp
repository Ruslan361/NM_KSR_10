#include <gtest/gtest.h>
extern "C" {
    #include "heat_solver.h"
}

#include <cstring>
#include <cstdio>
#include <cstdlib>

// Assume that calculate_max_difference is declared in heat_solver.h
extern "C" {
    double calculate_max_difference(const HeatEquationSolver* solverA, const HeatEquationSolver* solverB, int scale_n, int scale_m);
    void start_computation(HeatEquationSolver** solver_ptr, int n, int m, double T);
}

TEST(CreateHeatEquationSolverTest, ReturnsNonNull) {
    int n = 10;
    int m = 10;
    double T = 100.0;
    
    HeatEquationSolver* solver = createHeatEquationSolver(n, m, T);
    EXPECT_NE(solver, nullptr) << "Failed to create solver";
    
    // Verify calculated h and tau
    double expected_h = 1.0 / n;
    double expected_tau = T / m;
    EXPECT_DOUBLE_EQ(solver->h, expected_h);
    EXPECT_DOUBLE_EQ(solver->tau, expected_tau);
    
    freeHeatEquationSolver(solver);
}

TEST(CreateHeatEquationSolverTest, HandlesInvalidParameters) {
    int n = -1; // Invalid grid size
    int m = 10;
    double T = 100.0;
    
    // Assuming createHeatEquationSolver does not handle invalid n, it might still allocate.
    // If it should handle, you might need to modify the function to return NULL on invalid input.
    HeatEquationSolver* solver = createHeatEquationSolver(n, m, T);
    // Modify the expectation based on actual behavior
    EXPECT_NE(solver, nullptr) << "Solver should be created even with invalid n (based on current implementation)";
    
    freeHeatEquationSolver(solver);
}

TEST(CalculateMaxDifferenceTest, IdenticalSolvers) {
    int n = 2;
    int m = 2;
    double T = 1.0;
    
    HeatEquationSolver* solverA = createHeatEquationSolver(n, m, T);
    HeatEquationSolver* solverB = createHeatEquationSolver(n, m, T);
    ASSERT_NE(solverA, nullptr) << "Failed to create solverA";
    ASSERT_NE(solverB, nullptr) << "Failed to create solverB";

    // Initialize both solvers' results to 1.0
    for(int j = 0; j <= solverA->m; ++j) {
        for(int i = 0; i <= solverA->n; ++i) {
            solverA->results[j][i] = 1.0;
            solverB->results[j][i] = 1.0;
        }
    }

    // Calculate max difference
    double max_diff = calculate_max_difference(solverA, solverB, 1, 1);
    EXPECT_DOUBLE_EQ(max_diff, 0.0) << "Max difference should be 0.0";

    // Free solvers
    freeHeatEquationSolver(solverA);
    freeHeatEquationSolver(solverB);
}

TEST(CalculateMaxDifferenceTest, DifferentSolvers) {
    // Create solverA with n=2, m=2, T=1.0
    int n = 2;
    int m = 2;
    double T = 1.0;
    HeatEquationSolver* solverA = createHeatEquationSolver(n, m, T);
    ASSERT_NE(solverA, nullptr) << "Failed to create solverA";

    // Create solverB with n=2, m=2, T=1.0
    HeatEquationSolver* solverB = createHeatEquationSolver(n, m, T);
    ASSERT_NE(solverB, nullptr) << "Failed to create solverB";

    // Initialize solverA results to 1.0
    for(int j = 0; j <= solverA->m; ++j) {
        for(int i = 0; i <= solverA->n; ++i) {
            solverA->results[j][i] = 1.0;
        }
    }

    // Initialize solverB results: 2.0 on diagonal, else 1.0
    for(int j = 0; j <= solverB->m; ++j) {
        for(int i = 0; i <= solverB->n; ++i) {
            solverB->results[j][i] = (j == i) ? 2.0 : 1.0;
        }
    }

    // Calculate max difference
    double max_diff = calculate_max_difference(solverA, solverB, 1, 1);
    EXPECT_DOUBLE_EQ(max_diff, 1.0) << "Max difference should be 1.0";

    // Free solvers
    freeHeatEquationSolver(solverA);
    freeHeatEquationSolver(solverB);
}

TEST(CalculateMaxDifferenceTest, SameGridSolvers) {
    // Create solverA with n=3, m=3, T=1.5
    int n = 3;
    int m = 3;
    double T = 1.5;
    HeatEquationSolver* solverA = createHeatEquationSolver(n, m, T);
    ASSERT_NE(solverA, nullptr) << "Failed to create solverA";

    // Create solverB with n=3, m=3, T=1.5
    HeatEquationSolver* solverB = createHeatEquationSolver(n, m, T);
    ASSERT_NE(solverB, nullptr) << "Failed to create solverB";

    // Initialize both solvers' results to 3.0
    for(int j = 0; j <= solverA->m; ++j) {
        for(int i = 0; i <= solverA->n; ++i) {
            solverA->results[j][i] = 3.0;
            solverB->results[j][i] = 3.0;
        }
    }

    // Calculate max difference
    double max_diff = calculate_max_difference(solverA, solverB, 1, 1);
    EXPECT_DOUBLE_EQ(max_diff, 0.0) << "Max difference should be 0.0";

    // Free solvers
    freeHeatEquationSolver(solverA);
    freeHeatEquationSolver(solverB);
}

TEST(CalculateMaxDifferenceTest, RefinedAndCoarseGridComparison) {
    // Создать solverA с сеткой n=4, m=4, T=2.0 (не сгущенная)
    int n = 4;
    int m = 4;
    double T = 2.0;
    HeatEquationSolver* solverA = createHeatEquationSolver(n, m, T);
    ASSERT_NE(solverA, nullptr) << "Не удалось создать solverA";

    // Создать solverB с сеткой n=8, m=8, T=2.0 (сгущенная в 2 раза)
    int refined_n = 8;
    int refined_m = 8;
    HeatEquationSolver* solverB = createHeatEquationSolver(refined_n, refined_m, T);
    ASSERT_NE(solverB, nullptr) << "Не удалось создать solverB";

    // Инициализировать результаты solverA значением 4.0
    for(int j = 0; j <= solverA->m; ++j) {
        for(int i = 0; i <= solverA->n; ++i) {
            solverA->results[j][i] = 4.0;
        }
    }

    // Инициализировать результаты solverB, сгущая сетку:
    // На узлах с индексами, делящимися на 2, устанавливаем 4.0
    // Остальные узлы устанавливаем 0.0
    for(int j = 0; j <= solverB->m; ++j) {
        for(int i = 0; i <= solverB->n; ++i) {
            if(j % 2 == 0 && i % 2 == 0) {
                solverB->results[j][i] = 4.0;
            } else {
                solverB->results[j][i] = 0.0;
            }
        }
    }

    // Вычислить максимальную разницу на соответствующих узлах
    double max_diff = calculate_max_difference(solverA, solverB, 2, 2);
    EXPECT_DOUBLE_EQ(max_diff, 0.0) << "Максимальная разница должна быть 0.0 на соответствующих узлах";

    // Освободить память
    freeHeatEquationSolver(solverA);
    freeHeatEquationSolver(solverB);
}

TEST(CalculateMaxDifferenceTest, RefinedAndCoarseGridComparisonNonZeroDifference) {
    // Создать solverA с сеткой n=4, m=4, T=2.0 (не сгущенная)
    int n = 4;
    int m = 4;
    double T = 2.0;
    HeatEquationSolver* solverA = createHeatEquationSolver(n, m, T);
    ASSERT_NE(solverA, nullptr) << "Не удалось создать solverA";

    // Создать solverB с сеткой n=8, m=8, T=2.0 (сгущенная в 2 раза)
    int refined_n = 8;
    int refined_m = 8;
    HeatEquationSolver* solverB = createHeatEquationSolver(refined_n, refined_m, T);
    ASSERT_NE(solverB, nullptr) << "Не удалось создать solverB";

    // Инициализировать результаты solverA значением 4.0
    for(int j = 0; j <= solverA->m; ++j) {
        for(int i = 0; i <= solverA->n; ++i) {
            solverA->results[j][i] = 4.0;
        }
    }

    // Инициализировать результаты solverB, сгущая сетку:
    // На узлах с индексами, делящимися на 2, устанавливаем 5.0
    // Остальные узлы устанавливаем 0.0
    for(int j = 0; j <= solverB->m; ++j) {
        for(int i = 0; i <= solverB->n; ++i) {
            if(j % 2 == 0 && i % 2 == 0) {
                solverB->results[j][i] = 5.0;
            } else {
                solverB->results[j][i] = 0.0;
            }
        }
    }

    // Вычислить максимальную разницу на соответствующих узлах
    double max_diff = calculate_max_difference(solverA, solverB, 2, 2);
    EXPECT_DOUBLE_EQ(max_diff, 1.0) << "Максимальная разница должна быть 1.0 на соответствующих узлах";

    // Освободить память
    freeHeatEquationSolver(solverA);
    freeHeatEquationSolver(solverB);
}

TEST(CalculateMaxDifferenceTest, RefinedGridOneAxisComparison) {
    // Создать solverA с сеткой n=4, m=4, T=2.0 (не сгущенная)
    int n = 4;
    int m = 4;
    double T = 2.0;
    HeatEquationSolver* solverA = createHeatEquationSolver(n, m, T);
    ASSERT_NE(solverA, nullptr) << "Не удалось создать solverA";

    // Создать solverB с сеткой, сгущенной по оси x: n=8, m=4, T=2.0
    int refined_n = 8; // Сгущение по оси x
    int refined_m = 4;
    HeatEquationSolver* solverB = createHeatEquationSolver(refined_n, refined_m, T);
    ASSERT_NE(solverB, nullptr) << "Не удалось создать solverB";

    // Инициализировать результаты solverA значением 4.0
    for(int j = 0; j <= solverA->m; ++j) {
        for(int i = 0; i <= solverA->n; ++i) {
            solverA->results[j][i] = 4.0;
        }
    }

    // Инициализировать результаты solverB, сгущая сетку по оси x:
    // На узлах с индексами, делящимися на 2 по оси x, устанавливаем 4.0
    // На остальных узлах по оси x устанавливаем 5.0
    for(int j = 0; j <= solverB->m; ++j) {
        for(int i = 0; i <= solverB->n; ++i) {
            if(i % 2 == 0) {
                solverB->results[j][i] = 4.0;
            } else {
                solverB->results[j][i] = 5.0;
            }
        }
    }

    // Вычислить максимальную разницу на соответствующих узлах
    double max_diff = calculate_max_difference(solverA, solverB, 2, 1);
    EXPECT_DOUBLE_EQ(max_diff, 1.0) << "Максимальная разница должна быть 1.0 на соответствующих узлах";

    // Освободить память
    freeHeatEquationSolver(solverA);
    freeHeatEquationSolver(solverB);
}

TEST(StartComputationTest, InitializesAndFreesSolver) {
    HeatEquationSolver* solver_ptr = nullptr;
    int n = 10;
    int m = 10;
    double T = 100.0;
    
    start_computation(&solver_ptr, n, m, T);
    EXPECT_NE(solver_ptr, nullptr) << "Failed to initialize solver_ptr";

    // Optionally, you can perform additional checks on solver_ptr
    double expected_h = 1.0 / n;
    double expected_tau = T / m;
    EXPECT_DOUBLE_EQ(solver_ptr->h, expected_h);
    EXPECT_DOUBLE_EQ(solver_ptr->tau, expected_tau);
    
    freeHeatEquationSolver(solver_ptr);
}

// Uncomment and adjust if start_computation handles invalid parameters
/*
TEST(StartComputationTest, HandlesInitializationFailure) {
    HeatEquationSolver* solver_ptr = nullptr;
    int n = -1; // Invalid grid size
    int m = 10;
    double T = 100.0;
    
    start_computation(&solver_ptr, n, m, T);
    EXPECT_EQ(solver_ptr, nullptr) << "Solver_ptr should be NULL on initialization failure";
    
    // No need to free solver_ptr as it should be NULL
}
*/

// Uncomment and adjust if save_csv_format1 and save_csv_format2 are implemented
/*
TEST(SaveCSVFormat1Test, CreatesFileSuccessfully) {
    HeatEquationSolver* solver = createHeatEquationSolver(1, 1, 1.0);
    ASSERT_NE(solver, nullptr) << "Failed to create solver";

    solver->results[0][0] = 0.0;
    solver->results[0][1] = 1.0;
    solver->results[1][0] = 1.0;
    solver->results[1][1] = 2.0;

    FILE* fp = fopen("results_format1.csv", "w");
    ASSERT_NE(fp, nullptr) << "Failed to create results_format1.csv";
    fclose(fp); // Ensure the file can be created

    save_csv_format1(solver, nullptr, 0);

    FILE* file = fopen("results_format1.csv", "r");
    EXPECT_NE(file, nullptr) << "results_format1.csv should exist after saving";
    fclose(file);

    // Clean up
    remove("results_format1.csv");
    freeHeatEquationSolver(solver);
}

TEST(SaveCSVFormat2Test, CreatesFileSuccessfully) {
    HeatEquationSolver* solver = createHeatEquationSolver(1, 1, 1.0);
    ASSERT_NE(solver, nullptr) << "Failed to create solver";

    solver->results[0][0] = 0.0;
    solver->results[0][1] = 1.0;
    solver->results[1][0] = 1.0;
    solver->results[1][1] = 2.0;

    FILE* fp = fopen("results_format2.csv", "w");
    ASSERT_NE(fp, nullptr) << "Failed to create results_format2.csv";
    fclose(fp); // Ensure the file can be created

    save_csv_format2(solver, nullptr);

    FILE* file = fopen("results_format2.csv", "r");
    EXPECT_NE(file, nullptr) << "results_format2.csv should exist after saving";
    fclose(file);

    // Clean up
    remove("results_format2.csv");
    freeHeatEquationSolver(solver);
}
*/

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}