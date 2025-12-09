#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <random>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using namespace std;

ssize_t read_n_bytes(int fd, char *buffer, size_t n) {
  ssize_t total_bytes_read = 0;
  while (total_bytes_read < n) {
    ssize_t bytes_read = read(fd, buffer + total_bytes_read, 1);
    if (bytes_read <= 0) {
      return bytes_read;
    }
    total_bytes_read += bytes_read;
  }
  return total_bytes_read;
}

// void enviarPaqueteFile(int clientSocketFD, const std::string &fl_name,
// const std::string &nickname,
// const std::string &file_path) {
// char tipo = 'f';
//
// std::string tamano_flname = std::to_string(fl_name.length());
// tamano_flname = std::string(5 - tamano_flname.length(), '0') +
// tamano_flname;
//
// std::string tamano_nickname = std::to_string(nickname.length());
// tamano_nickname =
//     std::string(5 - tamano_nickname.length(), '0') + tamano_nickname;
//
// std::ifstream file(file_path, std::ios::binary);
// if (!file.is_open()) {
//   std::cerr << "Error: No se puede abrir el archivo " << file_path
//             << std::endl;
//   return;
// }
//
// std::ostringstream oss;
// oss << file.rdbuf();
// std::string file_content = oss.str();
// file.close();
//
// std::string tamano_file = std::to_string(file_content.size());
// tamano_file = std::string(11 - tamano_file.length(), '0') + tamano_file;
//
// std::string paquete;
// paquete += tipo;            // Tipo (1 byte)
// paquete += tamano_flname;   // Tamaño del nombre del archivo (5 bytes)
// paquete += fl_name;         // Nombre del archivo
// paquete += tamano_nickname; // Tamaño del nickname (5 bytes)
// paquete += nickname;        // Nickname
// paquete += tamano_file;     // Tamaño del archivo (11 bytes)
// paquete += file_content;    // Contenido del archivo
//
// ssize_t bytes_written =
//     write(clientSocketFD, paquete.c_str(), paquete.size());
// if (bytes_written < 0) {
//   std::cerr << "Error: No se pudo enviar el archivo al cliente." <<
//   std::endl;
// } else {
//   std::cout << "Archivo enviado correctamente." << std::endl;
// }
// }

void enviarP_matrix(int clientSocketFD, const string &sizeNN) {
  char tipo = 'G';
  std::string tamano_Size = std::to_string(sizeNN.length());
  tamano_Size = std::string(5 - tamano_Size.length(), '0') + sizeNN;

  std::string paquete;
  paquete += tipo;        // (1 byte)
  paquete += tamano_Size; // (5 bytes)
  cout << paquete << endl;

  ssize_t bytes_written =
      write(clientSocketFD, paquete.c_str(), paquete.size());
  if (bytes_written < 0) {
    std::cerr << "Error: No se pudo enviar el archivo al cliente." << std::endl;
  } else {
    std::cout << "Archivo enviado correctamente." << std::endl;
  }
}

void functionInputC_S(int socketFD) {
  // N -> Nickname{
  int numeroRellanar;
  char bufferName[5] = {'n'};
  char *subL = NULL;
  size_t subLSize = 0;
  cout << "Ingrese su nickname: ";
  getline(&subL, &subLSize, stdin);
  subL[strlen(subL) - 1] = '\0';
  std::string nameLength = to_string(strlen(subL));
  int n_BfName = 4;
  char lengthStr[n_BfName], buffUnion[n_BfName];
  if (n_BfName == to_string(strlen(subL)).size()) {
    for (int i = 0; i < n_BfName; i++) {
      lengthStr[i] = nameLength[i];
    }
    lengthStr[strlen(subL)] = '\0';
  } else {
    snprintf(lengthStr, sizeof(lengthStr), "%zu", strlen(subL));
  }
  numeroRellanar = n_BfName - strlen(lengthStr);
  if (numeroRellanar > 0) {
    sprintf(buffUnion, "%0*d%s", numeroRellanar, 0, lengthStr);
  } else {
    for (int i = 0; i < n_BfName; i++) {
      buffUnion[i] = lengthStr[i];
    }
    buffUnion[n_BfName] = '\0';
  }
  strcat(bufferName, buffUnion);
  int sizeT_send = 5 + strlen(subL);
  char packageS[sizeT_send];
  strcpy(packageS, bufferName);
  strcat(packageS, subL);
  packageS[sizeT_send] = '\0';
  cout << packageS << endl;

  ssize_t validName = write(socketFD, packageS, strlen(packageS));

  cout << "Vamos alla y terminemos con esto:\n";
  cout << "Ingrese :\n";
  cout << "chau\t\t\tPara logout\n";
  cout << "Quien esta\t\tPara ver la lista de usuarios conectados\n";
  cout << "msg:<name>:<msg>\tPara enviar un mensaje dirigido\n";
  cout << "G:<size nxn>\t\tPara generar la matriz simetrica nxn\n";
  cout << "F:<nombreDestino>:<filename>\t\tPara enviar un archivo\n";

  char *line = NULL;
  size_t lineSize = 0;

  while (true) {
    char buffer[3];
    cout << "$ ";
    ssize_t charCount = getline(&line, &lineSize, stdin);
    line[charCount - 1] = 0;
    if (charCount > 0) {
      if (strcmp(line, "chau") == 0) {
        cout << line << endl;
        char commandSpec[2] = {};
        commandSpec[0] = 'O';
        commandSpec[1] = '\0';
        snprintf(buffer, sizeof(buffer), "%s", commandSpec);
      } else if (strcmp(line, "Quien esta") == 0) {
        /*line[strlen(line)] = '\0';*/
        cout << line << endl;
        char commandSpec[2] = {};
        commandSpec[0] = 'L';
        commandSpec[1] = '\0';
        snprintf(buffer, sizeof(buffer), "%s", commandSpec);
      } else if (*line == 'G') {
        if (line[1] == ':') {
          char *valor_start = line + 2;
          if (*valor_start == '\0') {
            cerr << "Error: Valor vacío" << endl;
            return;
          }
          string valorRecibido(valor_start);
          cout << "Valor recibido: " << valorRecibido << endl;
          enviarP_matrix(socketFD, valorRecibido);
        }
      } else if (*line == 'f') {
        char *subChar02 = nullptr;
        char *subChar03 = nullptr;
        if (line[1] == ':') {
          char *file_start = strchr(line + 1, ':');
          /*cout << "DBG: flStart    " << file_start << endl;*/
          if (file_start == nullptr) {
            std::cerr << "Formato incorrecto: falta ':' después del comando"
                      << std::endl;
            return;
          }
          char *nickname_start = strchr(file_start + 1, ':');
          /*cout << "DBG: nickname_start    " << nickname_start << endl;*/
          if (nickname_start == nullptr) {
            std::cerr
                << "Formato incorrecto: falta ':' entre filename y nickname"
                << std::endl;
            return;
          }
          // Extraer el nombre del archivo
          int file_name_length = nickname_start - (file_start + 1);
          subChar02 = new char[file_name_length + 1];
          strncpy(subChar02, file_start + 1, file_name_length);
          subChar02[file_name_length] = '\0';

          // Extraer el nickname
          int nickname_length = strlen(nickname_start + 1);
          subChar03 = new char[nickname_length + 1];
          strncpy(subChar03, nickname_start + 1, nickname_length);
          subChar03[nickname_length] = '\0';
          std::string filename(subChar02);
          std::string nickname(subChar03);

          // enviarPaqueteFile(socketFD, filename, nickname, filename);

          delete[] subChar02;
          delete[] subChar03;
          continue;
        } else {
          std::cerr << "Formato incorrecto: falta ':' después del comando 'f'"
                    << std::endl;
        }
      }

      ssize_t amountWasSent = write(socketFD, buffer, strlen(buffer));
      if (amountWasSent == -1) {
        perror("Error al enviar");
        break;
      }
      if (strcmp(line, "chau") == 0) {
        break;
      }
    }
  }
  cout << "Hasta luego :)" << endl;
}

int varU = 0;
void listenAndPrint(int socketFD) {
  while (true) {
    char type;
    if (read_n_bytes(socketFD, &type, 1) <= 0) {
      std::cerr << "Error leyendo tipo de mensaje.\n";
      return;
    } else {
      varU++;
      if (varU == 1) {
        std::cout << "Tipo: " << type << "\n";
        // viewContentS_C(socketFD, type);
      }
    }
  }
  close(socketFD);
}

void recibir_mensajes(int socket) {
  char buffer[1024];
  while (true) {
    memset(buffer, 0, 1024);
    int bytesReceived = recv(socket, buffer, 1024, 0);
    if (bytesReceived <= 0) {
      cout << "\n[!] El servidor se ha desconectado." << endl;
      close(socket);
      exit(0);
    }
    cout << "\nServidor: " << buffer << endl;
    cout << "Tú: " << flush;
  }
}

void enviar_mensajes(int socket) {
  string mensaje;
  while (true) {
    cout << "Tú: " << flush;
    getline(cin, mensaje);

    if (mensaje == "exit") {
      close(socket);
      exit(0);
    }

    send(socket, mensaje.c_str(), mensaje.size(), 0);
  }
}

void leerBin(string &a, int indeX) {
  ifstream rBin(a, ios::binary);
  double tmpV;
  rBin.seekg(indeX * sizeof(double));
  rBin.read(reinterpret_cast<char *>(&tmpV), sizeof(double));
}

int main() {
  int socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD == -1) {
    perror("cannot create socket");
    return 1;
  }
  struct sockaddr_in address;
  memset(&address, 0, sizeof(struct sockaddr_in));
  address.sin_family = AF_INET;
  address.sin_port = htons(45000);
  address.sin_addr.s_addr = INADDR_ANY;
  // address.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(socketFD, (struct sockaddr *)&address,
              sizeof(struct sockaddr_in)) == -1) {
    perror("Error al conectar");
    close(socketFD);
    return 1;
  }
  cout << "[+] Conectado al servidor." << endl;

  std::thread(listenAndPrint, socketFD).detach();
  functionInputC_S(socketFD);

  close(socketFD);

  // thread t_recv(recibir_mensajes, clientSocket);
  // thread t_send(enviar_mensajes, clientSocket);
  // t_recv.join();
  // t_send.join();

  return 0;
}
