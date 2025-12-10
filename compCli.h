#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

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

ssize_t read_n_bytes(int fd, char *buffer, size_t n) {
  size_t total_bytes_read = 0;
  while (total_bytes_read < n) {
    ssize_t bytes_read =
        read(fd, buffer + total_bytes_read, n - total_bytes_read);
    // ssize_t bytes_read = read(fd, buffer + total_bytes_read, 1);
    if (bytes_read <= 0) {
      return bytes_read;
    }
    total_bytes_read += bytes_read;
  }
  return total_bytes_read;
}

int leer_entero_seguro(int fd, int len) {
  vector<char> buffer(len + 1, 0);
  if (read_n_bytes(fd, buffer.data(), len) <= 0) {
    return -1;
  }
  try {
    return stoi(buffer.data());
  } catch (...) {
    return -1;
  }
}

void enviarP_matrix(int clientSocketFD, const string &sizeNN) {
  char tipo = 'G';
  string tamano_Size = string(5 - sizeNN.length(), '0') + sizeNN;

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
      } else if (*line == 'b') {
        if (line[1] == ':') {
          std::string msgContent = "";
          if (strlen(line) > 2) {
            msgContent = std::string(line).substr(2);
          }
          std::string nickname = std::string(packageS + 1);
          string sizeNickStr =
              string(4 - to_string(nickname.size()).length(), '0') +
              to_string(nickname.length());
          string sizeMsgStr =
              string(5 - to_string(msgContent.size()).length(), '0') +
              to_string(msgContent.length());
          std::string paquete = "b";
          paquete += nickname;
          paquete += sizeMsgStr;
          paquete += msgContent;

          std::cout << "Paquete armado (" << paquete.length() << " bytes): ["
                    << paquete << "]" << std::endl;
          ssize_t amountWasSent =
              write(socketFD, paquete.c_str(), paquete.length());
          if (amountWasSent < 0) {
            std::cerr << "Error al enviar el broadcast." << std::endl;
            break;
          } else {
            std::cout << "Broadcast enviado con éxito." << std::endl;
          }
        } else {
          cout << "Vuelva a intentar" << endl;
        }
      } else if (*line == 'I') {
        // char *valor_start = line + 2;
        // if (*valor_start == '\0') {
        //   cerr << "Error: Valor vacío" << endl;
        //   return;
        // }
        // string valorRecibido(valor_start);
        char tipo = 'I';
        std::string paquete;
        paquete += tipo; // (1 byte)
        ssize_t bytes_written =
            write(socketFD, paquete.c_str(), paquete.size());
        if (bytes_written < 0) {
          std::cerr << "Error: No se pudo enviar el archivo al cliente."
                    << std::endl;
        } else {
          std::cout << "Archivo enviado correctamente." << std::endl;
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

void processPacket(int client_fd, char tipo) {
  if (tipo == 'L') {
    char message_size_str[6] = {0};
    if (read_n_bytes(client_fd, message_size_str, 5) <= 0) {
      std::cerr << "Error leyendo tamaño del mensaje.\n";
      return;
    }
    int message_size = std::stoi(message_size_str);
    // 5. Leer el mensaje según el tamaño
    std::string message(message_size, 0);
    if (read_n_bytes(client_fd, &message[0], message_size) <= 0) {
      std::cerr << "Error leyendo mensaje.\n";
      return;
    }
    cout << "$ " << tipo << ":";
    cout << message << "\n";
    cout << "$ ";
  } else if (tipo == 'M') { // Escuchando mensaje
    char name_size_str[5] = {0};
    char message_size_str[6] = {0};
    char time_recive[11] = {0};
    // 2. Leer el tamaño del nombre (4 bytes)
    if (read_n_bytes(client_fd, name_size_str, 4) <= 0) {
      std::cerr << "Error leyendo tamaño del nombre.\n";
      return;
    }
    int name_size = std::stoi(name_size_str);
    // 3. Leer el nombre según el tamaño
    std::string name(name_size, 0);
    if (read_n_bytes(client_fd, &name[0], name_size) <= 0) {
      std::cerr << "Error leyendo nombre.\n";
      return;
    }
    // 4. Leer el tamaño del mensaje (5 bytes)
    if (read_n_bytes(client_fd, message_size_str, 5) <= 0) {
      std::cerr << "Error leyendo tamaño del mensaje.\n";
      return;
    }
    int message_size = std::stoi(message_size_str);
    // 5. Leer el mensaje según el tamaño
    std::string message(message_size, 0);
    if (read_n_bytes(client_fd, &message[0], message_size) <= 0) {
      std::cerr << "Error leyendo mensaje.\n";
      return;
    }

    std::string date_recive(11, 0);
    if (read_n_bytes(client_fd, &date_recive[0], 10) <= 0) {
      std::cerr << "Error leyendo mensaje.\n";
      return;
    }
    // Mostrar el contenido recibido
    std::cout << "$ " << tipo << ":";
    std::cout << "Nombre: " << name << "(" << date_recive << "):";
    std::cout << message << "\n";
    std::cout << "$ ";
  } else if (tipo == 'B') { // Escuchando Broadcast
    char name_size_str[5] = {0};
    char message_size_str[6] = {0};
    char time_recive[11] = {0};
    // 2. Leer el tamaño del nombre (4 bytes)
    if (read_n_bytes(client_fd, name_size_str, 4) <= 0) {
      std::cerr << "Error leyendo tamaño del nombre.\n";
      return;
    }
    int name_size = std::stoi(name_size_str);
    // 3. Leer el nombre según el tamaño
    std::string name(name_size, 0);
    if (read_n_bytes(client_fd, &name[0], name_size) <= 0) {
      std::cerr << "Error leyendo nombre.\n";
      return;
    }
    // 4. Leer el tamaño del mensaje (5 bytes)
    if (read_n_bytes(client_fd, message_size_str, 5) <= 0) {
      std::cerr << "Error leyendo tamaño del mensaje.\n";
      return;
    }
    int message_size = std::stoi(message_size_str);
    // 5. Leer el mensaje según el tamaño
    std::string message(message_size, 0);
    if (read_n_bytes(client_fd, &message[0], message_size) <= 0) {
      std::cerr << "Error leyendo mensaje.\n";
      return;
    }

    std::string date_recive(11, 0);
    if (read_n_bytes(client_fd, &date_recive[0], 10) <= 0) {
      std::cerr << "Error leyendo mensaje.\n";
      return;
    }
    // Mostrar el contenido recibido
    std::cout << "$ " << tipo << ":";
    std::cout << "Nombre: " << name << "(" << date_recive << "):";
    std::cout << message << "\n";
    std::cout << "$ ";
  } else if (tipo == 'f') {
    char file_size_str[6] = {0};
    char nickname_size_str[6] = {0};
    char content_size_str[12] = {0};

    if (read_n_bytes(client_fd, file_size_str, 5) <= 0) {
      std::cerr << "Error leyendo el tamaño del nombre del archivo.\n";
      return;
    }
    int file_size = std::stoi(file_size_str);

    std::string filename(file_size, 0);
    if (read_n_bytes(client_fd, &filename[0], file_size) <= 0) {
      std::cerr << "Error leyendo el nombre del archivo.\n";
      return;
    }

    if (read_n_bytes(client_fd, nickname_size_str, 5) <= 0) {
      std::cerr << "Error leyendo el tamaño del nickname destino.\n";
      return;
    }
    int nickname_size = std::stoi(nickname_size_str);

    std::string nickname_dest(nickname_size, 0);
    if (read_n_bytes(client_fd, &nickname_dest[0], nickname_size) <= 0) {
      std::cerr << "Error leyendo el nickname destino.\n";
      return;
    }

    if (read_n_bytes(client_fd, content_size_str, 11) <= 0) {
      std::cerr << "Error leyendo el tamaño del archivo.\n";
      return;
    }
    int content_size = std::stoi(content_size_str);

    std::string file_content(content_size, 0);
    if (read_n_bytes(client_fd, &file_content[0], content_size) <= 0) {
      std::cerr << "Error leyendo el contenido del archivo.\n";
      return;
    }

    std::cout << "F:" << filename << ":" << nickname_dest << "\n";
    /*std::cout << "tamano del archivo" << content_size << " bytes\n";*/
    if (filename[file_size - 3] == 't') {
      std::string new_filename = filename + "_R";
      std::ofstream output_file(new_filename, std::ios::binary);
      if (!output_file) {
        std::cerr << "Error al crear el archivo " << new_filename << ".\n";
        return;
      }
      output_file.write(file_content.c_str(), content_size);
      output_file.close();
      std::cout << "Archivo guardado como: " << new_filename << "\n";
    } else {
      std::string new_filename = filename + ".Hex";
      std::ofstream output_file(new_filename, std::ios::binary);
      if (!output_file) {
        std::cerr << "Error al crear el archivo " << new_filename << ".\n";
        return;
      }
      output_file.write(file_content.c_str(), content_size);
      output_file.close();
      std::cout << "Archivo guardado como: " << new_filename << "\n";
    }
  }
}

void listen_Print(int socketFD) {
  while (true) {
    char type;
    ssize_t res = read_n_bytes(socketFD, &type, 1);
    if (res <= 0) {
      cout << "Cliente desconectado o error." << endl;
      break;
    } else {
      processPacket(socketFD, type);
    }
  }
  close(socketFD);
}

int initClient() {
  int socketFD = socket(AF_INET, SOCK_STREAM, 0);
  int puerto = 45000;
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
  cout << "[+] Conectado al server." << endl;
  std::thread(listen_Print, socketFD).detach();
  functionInputC_S(socketFD);
  close(socketFD);
  return 0;
}
