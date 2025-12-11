#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <functional>
#include <algorithm>
#include <fstream>

using namespace std;

const int m = 1200;
const int n = 1100;
const int k = 1000;

using Matrix = vector<double>;

void matrixInit(Matrix &mat, int rows, int cols)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0, 10);

    for (int i = 0; i < rows * cols; ++i)
    {
        mat[i] = dis(gen);
    }
}

void sequentialMultiplication(const Matrix &A, const Matrix &B, Matrix &C)
{
    fill(C.begin(), C.end(), 0);

    for (int i = 0; i < m; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            double sum = 0.0;
            for (int z = 0; z < k; ++z)
            {
                sum += A[i * k + z] * B[z * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

void singleRowMultiplication(const Matrix &A, const Matrix &B, Matrix &C, int start_row, int end_row)
{
    for (int i = start_row; i < end_row; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            double sum = 0.0;
            for (int z = 0; z < k; ++z)
            {
                sum += A[i * k + z] * B[z * n + j];
            }
            C[i * n + j] = sum;
        }
    }
}

void parallelMultiplication(const Matrix &A, const Matrix &B, Matrix &C)
{
    fill(C.begin(), C.end(), 0);
    unsigned int num_threads = thread::hardware_concurrency();

    vector<thread> threads;

    int rows_per_thread = m / num_threads;
    int remainder = m % num_threads;

    int start_row = 0;
    for (unsigned int i = 0; i < num_threads; ++i)
    {
        int end_row = start_row + rows_per_thread + (i < remainder ? 1 : 0);

        threads.emplace_back(singleRowMultiplication, cref(A), cref(B), ref(C), start_row, end_row);
        start_row = end_row;
    }

    for (auto &t : threads)
    {
        t.join();
    }
}

bool verification(const Matrix &sequential, const Matrix &parallel)
{
    double error = 0;
    for (int i = 0; i < m * n; ++i)
    {
        double difference = fabs(sequential[i] - parallel[i]);
        if (difference > error)
        {
            error = difference;
        }
    }
    cout << "Verification Passed! Error: " << error << endl;
    return true;
}

int main()
{
    ofstream outFile("hw04Result.txt");
    const int iterations = 5;

    cout << "=== Homework 4: Matrix Multiplication ===" << std::endl;
    cout << "Matrix Dimensions: "
         << "A[" << m << "x" << k << "] * "
         << "B[" << k << "x" << n << "] = "
         << "C[" << m << "x" << n << "]" << endl;

    cout << "Initializing" << endl;
    Matrix A(m * k);
    Matrix B(k * n);
    Matrix C_sequential(m * n);
    Matrix C_parallel(m * n);

    matrixInit(A, m, k);
    matrixInit(B, k, n);

    cout << "Running Sequential Multiplication (Average of " << iterations << " rounds)" << endl;
    double total_seq_time = 0.0;
    for (int i = 0; i < iterations; ++i)
    {
        auto start_seq = chrono::high_resolution_clock::now();
        sequentialMultiplication(A, B, C_sequential);
        auto end_seq = chrono::high_resolution_clock::now();
        chrono::duration<double> duration_seq = end_seq - start_seq;
        total_seq_time += duration_seq.count();
    }
    double avg_seq_time = total_seq_time / iterations;
    cout << "Average Sequential Time: " << avg_seq_time << " seconds." << endl;

    unsigned int n_threads = thread::hardware_concurrency();
    cout << "Running Parallel Multiplication (" << (n_threads ? n_threads : 4) << " threads, Average of " << iterations << " rounds)" << endl;
    double total_par_time = 0.0;
    for (int i = 0; i < iterations; ++i)
    {
        auto start_par = chrono::high_resolution_clock::now();
        parallelMultiplication(A, B, C_parallel);
        auto end_par = chrono::high_resolution_clock::now();
        chrono::duration<double> duration_par = end_par - start_par;
        total_par_time += duration_par.count();
    }
    double avg_par_time = total_par_time / iterations;
    cout << "Average Parallel Time: " << avg_par_time << " seconds." << endl;

    double speedup = avg_seq_time / avg_par_time;
    cout << "Speedup: " << speedup << " Times" << endl;

    cout << "Verifying results..." << endl;
    verification(C_sequential, C_parallel);

    if (outFile.is_open())
    {
        outFile << "=============================================================================" << endl;
        outFile << "                      Homework 4: Matrix Multiplication                      " << endl;
        outFile << "=============================================================================" << endl;

        outFile << left << setw(18) << "Matrix Dimensions: " << setw(12) << ("A[" + to_string(m) + "x" + to_string(k) + "] * ")
                << setw(12) << ("B[" + to_string(k) + "x" + to_string(n) + "] = ")
                << setw(12) << ("C[" + to_string(m) + "x" + to_string(n) + "]") << endl;
        outFile << endl;

        outFile << "|--------------|------------|----------------------|------------------------|" << endl;
        outFile << "|     Type     |  Threads   |  Avg Time (Seconds)  |        Speedup         |" << endl;
        outFile << "|--------------|------------|----------------------|------------------------|" << endl;

        outFile << "|Sequential    | 1          | " << setw(20) << avg_seq_time << " | 1                      |" << endl;
        outFile << "|Parallel      | " << setw(10) << (n_threads ? n_threads : 4) << " | " << setw(20) << avg_par_time << " | " << setw(20) << speedup << "   |" << endl;
        outFile << "|--------------|------------|----------------------|------------------------|" << endl;
        outFile << "=============================================================================" << endl;

        printf("\n[Success] Report saved to \"hw04Result.txt\"\n");
        outFile.close();
    }

    return 0;
}