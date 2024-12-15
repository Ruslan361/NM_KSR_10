#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>
#include "heat_solver.h"
//#define OMP_NUM_THREADS = 1

using namespace std;

// ... (Definitions of structures and functions remain unchanged)

void solve_parallel_improved(HeatEquationSolver* solver) {
    int n = solver->n;
    int m = solver->m;

    // Number of threads (processing units)
    int p = omp_get_max_threads();

    // Adjust block sizes to handle non-divisible n
    std::vector<int> block_sizes(p);
    int base_block_size = n / p;
    int remainder = n % p;

    // Distribute the remainder among the first few threads
    for (int i = 0; i < p; ++i) {
        block_sizes[i] = base_block_size + (i < remainder ? 1 : 0);
    }

    // Prefix sum to get start indices
    std::vector<int> block_starts(p + 1, 0);
    for (int i = 0; i < p; ++i) {
        block_starts[i + 1] = block_starts[i] + block_sizes[i];
    }

    // Time-stepping loop
    for (int j = 1; j <= m; ++j) {
        double t = j * solver->tau;

        // Set up the coefficients
        std::vector<double> a(n + 1), b(n + 1), c(n + 1), d(n + 1), u(n + 1);

        // Boundary conditions
        b[0] = 1.0;
        c[0] = 0.0;
        d[0] = boundary_left(t);

        a[n] = 0.0;
        b[n] = 1.0;
        d[n] = boundary_right(t);
        double aa = -5/(solver->h*solver->h);
        double bb = 1/solver->tau + 5/(solver->h*solver->h);
        // Interior points
        #pragma omp parallel for
        for (int i = 1; i < n; ++i) {
            // a[i] = aa;
            // b[i] = bb;
            // c[i] = aa;
            // d[i] = sin(t- solver->tau/2) + (1/solver->tau + aa)*solver->results[j-1][i] + (5/(2 *solver->h * solver->h))*solver->results[j-1][i-1] + (5/(2 *solver->h * solver->h))*solver->results[j-1][i+1];
            a[i] = -solver->alpha;
            b[i] = 1 + 2 * solver->alpha;
            c[i] = -solver->alpha;
            d[i] = solver->alpha * solver->results[j - 1][i - 1]
                 + (1 - 2 * solver->alpha) * solver->results[j - 1][i]
                 + solver->alpha * solver->results[j - 1][i + 1]
                 + solver->tau * source_term(t - solver->tau / 2);
        }

        // Arrays for the reduced system
        std::vector<double> A(p), B(p), C(p), D(p);

        // Forward elimination within blocks
        #pragma omp parallel
        {
            int thread_id = omp_get_thread_num();

            int start = block_starts[thread_id] + 1;     // +1 to avoid the boundary point at index 0
            int end = block_starts[thread_id + 1];       // Exclusive upper bound
            int local_size = end - start;

            // Vectors for local computation
            std::vector<double> a_local(local_size), b_local(local_size), c_local(local_size), d_local(local_size);

            // Copy data to local vectors
            for (int i = 0; i < local_size; ++i) {
                int idx = start + i;
                a_local[i] = a[idx];
                b_local[i] = b[idx];
                c_local[i] = c[idx];
                d_local[i] = d[idx];
            }

            // Forward elimination
            for (int i = 1; i < local_size; ++i) {
                double m = a_local[i] / b_local[i - 1];
                b_local[i] -= m * c_local[i - 1];
                d_local[i] -= m * d_local[i - 1];
            }

            // Store the compressed coefficients for the reduced system
            A[thread_id] = a_local[0];
            B[thread_id] = b_local[0];
            C[thread_id] = c_local.back();
            D[thread_id] = d_local[0];

            // Store the modified coefficients back to the global arrays
            for (int i = 0; i < local_size; ++i) {
                int idx = start + i;
                a[idx] = a_local[i];
                b[idx] = b_local[i];
                c[idx] = c_local[i];
                d[idx] = d_local[i];
            }
        }

        // Solve the reduced system sequentially
        // Forward elimination for the reduced system
        for (int i = 1; i < p; ++i) {
            double m = A[i] / B[i - 1];
            B[i] -= m * C[i - 1];
            D[i] -= m * D[i - 1];
        }

        // Backward substitution for the reduced system
        std::vector<double> X(p);
        X[p - 1] = D[p - 1] / B[p - 1];
        for (int i = p - 2; i >= 0; --i) {
            X[i] = (D[i] - C[i] * X[i + 1]) / B[i];
        }

        // Backward substitution within blocks
        #pragma omp parallel
        {
            int thread_id = omp_get_thread_num();

            int start = block_starts[thread_id] + 1;     // +1 to avoid the boundary point at index 0
            int end = block_starts[thread_id + 1];       // Exclusive upper bound
            int local_size = end - start;

            // Use the interface values from the reduced system
            std::vector<double> u_local(local_size);

            // Initialize the last value in each block
            if (local_size > 0) {
                int global_idx = end - 1;
                if (thread_id < p - 1) {
                    u_local[local_size - 1] = (d[global_idx] - c[global_idx] * X[thread_id + 1]) / b[global_idx];
                } else {
                    // Last block
                    u_local[local_size - 1] = (d[global_idx]) / b[global_idx];
                }

                // Backward substitution within the block
                for (int i = local_size - 2; i >= 0; --i) {
                    int idx = start + i;
                    u_local[i] = (d[idx] - c[idx] * u_local[i + 1]) / b[idx];
                }

                // Write the local solution back to the global solution vector
                for (int i = 0; i < local_size; ++i) {
                    int idx = start + i;
                    u[idx] = u_local[i];
                }
            }
        }

        // Apply boundary conditions to the solution
        u[0] = d[0];
        u[n] = d[n];

        // Save the solution for this time step
        for (int i = 0; i <= n; ++i) {
            solver->results[j][i] = u[i];
        }
    }
}

int main() {
    int n = 1000;
    int m = 1000;
    double T = 100; // Use M_PI for better precision

    HeatEquationSolver* solver = createHeatEquationSolver(n, m, T);
    if (solver == nullptr) {
        fprintf(stderr, "Failed to allocate memory for solver.\n");
        return 1;
    }

    solve_parallel_improved(solver);

    // Save the results to CSV
    FILE* fp = fopen("results.csv", "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file for writing.\n");
        freeHeatEquationSolver(solver); // Free before exiting
        return 1;
    }

    fprintf(fp, "t,x,v\n");
    for (int j = 0; j <= solver->m; ++j) {
        double t = j * solver->tau;
        for (int i = 0; i <= solver->n; ++i) {
            double x = i * solver->h;
            fprintf(fp, "%f,%f,%f\n", t, x, solver->results[j][i]);
        }
    }
    fclose(fp);

    freeHeatEquationSolver(solver); // Ensure this is called only once

    // Execute Python script (optional error handling)
    int ret = system("conda run -n pyside6 python ./show.py results.csv True True --save_3d_plot");
    if (ret != 0) {
        fprintf(stderr, "Failed to execute show.py\n");
        return ret;
    }

    return 0;
}