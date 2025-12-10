#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

using Matrix = std::vector<std::vector<double>>;
using Vector = std::vector<double>;

void printMatrix(const Matrix &M, const std::string &name) {
  std::cout << name << ":\n";
  for (const auto &row : M) {
    for (double val : row) {
      if (std::abs(val) < 1e-10)
        val = 0.0;
      std::cout << std::setw(10) << std::fixed << std::setprecision(4) << val
                << " ";
    }
    std::cout << "\n";
  }
  std::cout << "\n";
}

double getNorm(const Vector &v) {
  double sum = 0.0;
  for (double x : v)
    sum += x * x;
  return std::sqrt(sum);
}

double dotProduct(const Vector &a, const Vector &b) {
  double sum = 0.0;
  for (size_t i = 0; i < a.size(); ++i)
    sum += a[i] * b[i];
  return sum;
}

void qr_decomposition(const Matrix &A, Matrix &Q, Matrix &R) {
  int m = A.size();    // Filas
  int n = A[0].size(); // Columnas

  Q.assign(m, Vector(m, 0.0));
  for (int i = 0; i < m; i++)
    Q[i][i] = 1.0;
  R = A;

  for (int k = 0; k < std::min(m - 1, n); k++) {
    // --- Paso 1:
    Vector x(m - k);
    for (int i = k; i < m; i++) {
      x[i - k] = R[i][k];
    }
    // --- Paso 2: Calcular la norma y el vector v ---
    double normX = getNorm(x);
    if (normX < 1e-10)
      continue;
    double alpha = (x[0] > 0) ? -normX : normX;

    Vector u = x;
    u[0] -= alpha; // Restamos alpha al primer elemento
    double normU = getNorm(u);

    if (normU < 1e-10)
      continue;
    Vector v(m - k);
    for (size_t i = 0; i < u.size(); i++) {
      v[i] = u[i] / normU;
    }
    // --- Paso 3: Aplicar la transformación a R ---
    // R_new = (I - 2vvT) * R_old  =>  R - 2 * v * (vT * R)
    // Aplicamos solo a la submatriz desde (k, k)
    for (int j = k; j < n; j++) { // Iterar sobre columnas de R
      printMatrix(R, "MATRIX R(DBG): ");
      double dot = 0.0;
      // v^T * R_columna_j
      for (int i = k; i < m; i++) {
        dot += v[i - k] * R[i][j];
      }
      // Restar 2 * v * dot
      for (int i = k; i < m; i++) {
        R[i][j] -= 2.0 * v[i - k] * dot;
      }
    }
    printMatrix(R, "---> MATRIX R(DBG): ");
    // --- Paso 4: Actualizar Q ---
    // Q_new = Q_old * (I - 2vvT)  =>  Q - 2 * (Q * v) * vT
    for (int i = 0; i < m; i++) {
      double dot = 0.0;
      // Fila_i_Q * v
      for (int j = k; j < m; j++) {
        dot += Q[i][j] * v[j - k];
      }
      // Restar
      for (int j = k; j < m; j++) {
        Q[i][j] -= 2.0 * dot * v[j - k];
      }
    }
  }
}

void verif(const Matrix &Q, const Matrix &R) {
  // Verificación: Q * R debería ser A
  Matrix Check(3, Vector(3, 0.0));
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        Check[i][j] += Q[i][k] * R[k][j];
      }
    }
  }
  printMatrix(Check, "Verificacion (Q * R)");
}

void create_dummy_bin(const char *filename, int size) {
  std::ofstream fs(filename, std::ios::binary);
  double zero = 0.0;
  for (int i = 0; i < size; i++)
    fs.write(reinterpret_cast<char *>(&zero), sizeof(double));
}

int initDQR() {
  // Matrix A = {{12, -51, 4}, {6, 167, -68}, {-4, 24, -41}};
  // Matrix Q, R;
  // qr_decomposition(A, Q, R);
  // cout << "--- Resultado de la Descomposicion QR (Householder) ---\n\n";
  // printMatrix(A, "Matriz A (Original)");
  // printMatrix(Q, "Matriz Q (Ortogonal)");
  // printMatrix(R, "Matriz R (Triangular Superior)");
  // verif(Q,R);
  int m = 1000;
  int n = 1000;
  size_t total_elems = m * n;
  size_t size_bytes = total_elems * sizeof(double);
  create_dummy_bin("Q.bin", m * m); // Q es m x m
  create_dummy_bin("R.bin", m * n); // R es m x n

  double *ptrA = map_file("A.bin", READ_ONLY);
  double *ptrQ = map_file("Q.bin", READ_WRITE);
  double *ptrR = map_file("R.bin", READ_WRITE);

  return 0;
}
