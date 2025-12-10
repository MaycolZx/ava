#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

// Librerías de sistema Linux (POSIX)
#include <fcntl.h>    // Para open
#include <sys/mman.h> // Para mmap, munmap
#include <sys/stat.h> // Constantes de modo archivo
#include <unistd.h>   // Para close, ftruncate, lseek

// Configuracion
const int M = 10; // Filas (pequeño para probar, cámbialo a 1000+ luego)
const int N = 10; // Columnas

// ==========================================
// 1. UTILIDADES DE MAPEO DE MEMORIA (Linux)
// ==========================================

// Estructura para manejar el recurso mapeado
struct MappedMatrix {
  double *data;
  size_t size_bytes;
  int fd;
  MappedMatrix(const char *filename, int rows, int cols, bool create_new) {
    size_bytes = (size_t)rows * cols * sizeof(double);
    // Abrir archivo: Lectura/Escritura, Crear si no existe
    // O_RDWR: Lectura y Escritura
    // O_CREAT: Crear si no existe
    // 0666: Permisos de lectura/escritura para el usuario
    fd = open(filename, O_RDWR | O_CREAT, 0666);

    if (fd == -1) {
      std::cerr << "Error al abrir el archivo: " << filename << std::endl;
      exit(1);
    }
    if (create_new) {
      if (ftruncate(fd, size_bytes) == -1) {
        std::cerr << "Error al redimensionar archivo" << std::endl;
        exit(1);
      }
    }
    void *ptr =
        mmap(NULL, size_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (ptr == MAP_FAILED) {
      std::cerr << "Error en mmap" << std::endl;
      exit(1);
    }

    data = static_cast<double *>(ptr);
  }

  ~MappedMatrix() {
    if (data != MAP_FAILED) {
      munmap(data, size_bytes);
    }
    if (fd != -1) {
      close(fd);
    }
  }
};

// ==========================================
// 2. ALGORITMO QR HOUSEHOLDER (Versión Plana 1D)
// ==========================================

// Acceso rápido estilo matriz: A[i][j] -> data[i * stride + j]
inline double &access(double *data, int row, int col, int stride) {
  return data[row * stride + col];
}

double getNorm(const std::vector<double> &v) {
  double sum = 0.0;
  for (double x : v)
    sum += x * x;
  return std::sqrt(sum);
}

void qr_decomposition_mmap(MappedMatrix &A_map, MappedMatrix &Q_map,
                           MappedMatrix &R_map, int m, int n) {
  double *A = A_map.data;
  double *Q = Q_map.data;
  double *R = R_map.data;

  // 1. Copiar A en R (R empieza siendo A)
  // std::copy es eficiente, pero con mmap grandes puede tardar por I/O
  for (size_t i = 0; i < (size_t)m * n; ++i) {
    R[i] = A[i];
  }

  // 2. Inicializar Q como Identidad
  // Llenar con ceros
  for (size_t i = 0; i < (size_t)m * m; ++i)
    Q[i] = 0.0;
  // Poner 1s en la diagonal
  for (int i = 0; i < m; ++i)
    access(Q, i, i, m) = 1.0;

  // --- Inicio del Algoritmo ---
  for (int k = 0; k < std::min(m - 1, n); k++) {

    // Paso A: Extraer columna k de la submatriz de R
    std::vector<double> x(m - k);
    for (int i = k; i < m; i++) {
      x[i - k] = access(R, i, k, n);
    }

    double normX = getNorm(x);
    if (normX < 1e-12)
      continue; // Columna vacía o casi cero

    double alpha = (x[0] > 0) ? -normX : normX;

    // u = x - alpha * e1
    std::vector<double> u = x;
    u[0] -= alpha;

    double normU = getNorm(u);
    if (normU < 1e-12)
      continue;

    // v = u / ||u||
    std::vector<double> v(m - k);
    for (size_t i = 0; i < u.size(); i++)
      v[i] = u[i] / normU;

    // Paso B: Actualizar R (PARALELIZABLE)
    // #pragma omp parallel for schedule(static)
    for (int j = k; j < n; j++) {
      double dot = 0.0;
      // v^T * R_columna_j
      for (int i = k; i < m; i++) {
        dot += v[i - k] * access(R, i, j, n);
      }
      // Resta
      for (int i = k; i < m; i++) {
        access(R, i, j, n) -= 2.0 * v[i - k] * dot;
      }
    }

    // Paso C: Actualizar Q (PARALELIZABLE)
    // #pragma omp parallel for schedule(static)
    for (int i = 0; i < m; i++) {
      double dot = 0.0;
      // Fila_Q_i * v
      for (int l = k; l < m; l++) {
        dot += access(Q, i, l, m) * v[l - k];
      }
      // Resta
      for (int l = k; l < m; l++) {
        access(Q, i, l, m) -= 2.0 * dot * v[l - k];
      }
    }
  }
}

// ==========================================
// 3. MAIN Y GENERACIÓN DE DATOS
// ==========================================
int initDQR() {
  std::cout << "--- QR en Linux con Archivos Binarios (mmap) ---\n";

  // Nombres de archivo
  const char *fileA = "./binData/A.bin";
  const char *fileQ = "./binData/Q.bin";
  const char *fileR = "./binData/R.bin";

  // 1. Generar A.bin con datos aleatorios para probar
  {
    std::cout << "Generando " << fileA << " (" << M << "x" << N << ")..."
              << std::endl;
    MappedMatrix A(fileA, M, N, true); // true = crear/sobrescribir
    for (int i = 0; i < M; i++) {
      for (int j = 0; j < N; j++) {
        // Datos de prueba simples
        access(A.data, i, j, N) = (double)(i + j + 1);
      }
    }
    // El destructor de MappedMatrix cierra y guarda A.bin aquí
  }

  // 2. Mapear los archivos (A lectura, Q y R nuevos)
  std::cout << "Mapeando memoria..." << std::endl;
  MappedMatrix A(fileA, M, N, false); // false = abrir existente
  MappedMatrix Q(fileQ, M, M, true);  // Crear Q (MxM)
  MappedMatrix R(fileR, M, N, true);  // Crear R (MxN)

  // 3. Ejecutar QR
  std::cout << "Calculando QR..." << std::endl;
  qr_decomposition_mmap(A, Q, R, M, N);

  // 4. Mostrar resultados (solo si la matriz es pequeña)
  if (M <= 10) {
    std::cout << "\nMatriz R (Resultante en disco):\n";
    for (int i = 0; i < M; i++) {
      for (int j = 0; j < N; j++) {
        double val = access(R.data, i, j, N);
        if (std::abs(val) < 1e-10)
          val = 0.0;
        std::cout << std::setw(8) << std::fixed << std::setprecision(2) << val
                  << " ";
      }
      std::cout << "\n";
    }
  }

  std::cout << "\nProceso terminado. Datos guardados en Q.bin y R.bin"
            << std::endl;

  return 0;
}
