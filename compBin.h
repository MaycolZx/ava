#include <fstream>
#include <iostream>
#include <random>
using namespace std;

void leerBin(string &a) {
  cout << "LEERBIN" << endl;
  ifstream rBin(a, ios::binary);
  double tmpV;
  while (rBin.read(reinterpret_cast<char *>(&tmpV), sizeof(double))) {
    cout << tmpV << " ";
  }
  // rBin.seekg(indeX * sizeof(double));
  // rBin.read(reinterpret_cast<char *>(&tmpV), sizeof(double));
}

void genBin(string &a, int sizeNN) {
  cout << "GENBIN" << endl;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> distrib_rango(0.0, 100.0);
  ofstream bin_data(a, ios::binary);
  if (bin_data) {
    cout << "Entramos al binario" << endl;
    for (int i = 0; i < sizeNN; i++) {
      for (int j = 0; j < sizeNN; j++) {
        double valorT = distrib_rango(gen);
        cout << valorT << " ";
        bin_data.write(reinterpret_cast<const char *>(&valorT), sizeof(double));
      }
      cout << endl;
    }
  }
  bin_data.close();
}
