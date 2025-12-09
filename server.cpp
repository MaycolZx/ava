#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <random>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

using namespace std;

map<std::string, int> maPair;

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

void funtionFillCeros(char *mainBuffer, char *bufferConcat, int n_BfName,
                      int clientSocketFD) {
  /*char buffTMP[strlen(mainBuffer) + 1];*/
  std::string nameLength = to_string(strlen(mainBuffer));
  char lengthStr[n_BfName], buffUnion[n_BfName];
  if (n_BfName == to_string(strlen(mainBuffer)).size()) {
    for (int i = 0; i < n_BfName; i++) {
      lengthStr[i] = nameLength[i];
    }
    lengthStr[strlen(mainBuffer)] = '\0';
  } else {
    snprintf(lengthStr, sizeof(lengthStr), "%zu", strlen(mainBuffer));
  }
  int numeroRellanar = n_BfName - strlen(lengthStr);
  if (numeroRellanar > 0) {
    sprintf(buffUnion, "%0*d%s", numeroRellanar, 0, lengthStr);
  } else {
    for (int i = 0; i < n_BfName; i++) {
      buffUnion[i] = lengthStr[i];
    }
    buffUnion[n_BfName] = '\0';
  }
  int sizeT_send = n_BfName + 1 + strlen(mainBuffer);
  char packageS[sizeT_send];
  strcpy(packageS, bufferConcat);
  strcat(packageS, buffUnion);
  strcat(packageS, mainBuffer);
  packageS[sizeT_send] = '\0';
  /*cpyPackS = packageS;*/
  /*cout << "PACKAGES: " << cpyPackS << endl;*/
  write(clientSocketFD, packageS, strlen(packageS));
}

void enviarPaqueteMensaje(int clientSocketFD, const std::string &nombre,
                          const std::string &mensaje) {
  char tipo = 'M';
  std::string tamano_nombre = std::to_string(nombre.length());
  tamano_nombre = std::string(4 - tamano_nombre.length(), '0') + tamano_nombre;
  std::string tamano_mensaje = std::to_string(mensaje.length());
  tamano_mensaje =
      std::string(5 - tamano_mensaje.length(), '0') + tamano_mensaje;
  time_t now = time(0);
  struct tm *now_tm = localtime(&now);
  char time_str[11];
  strftime(time_str, sizeof(time_str), "%y%m%d%H%M", now_tm);

  std::string paquete;
  paquete += tipo;           // Tipo (1 byte)
  paquete += tamano_nombre;  // Tamaño del nombre (4 bytes)
  paquete += nombre;         // Nombre
  paquete += tamano_mensaje; // Tamaño del mensaje (5 bytes)
  paquete += mensaje;        // Mensaje
  paquete += time_str;       // Tiempo (10 bytes)

  for (std::map<std::string, int>::iterator it = maPair.begin();
       it != maPair.end(); ++it) {
    if (it->first == nombre) {
      write(it->second, paquete.c_str(), paquete.length());
    }
  }
}

void enviarPaqueteATodos(int clientSocketFD, const std::string &nombre,
                         const std::string &mensaje) {
  char tipo = 'B';
  std::string tamano_nombre = std::to_string(nombre.length());
  tamano_nombre = std::string(4 - tamano_nombre.length(), '0') + tamano_nombre;
  std::string tamano_mensaje = std::to_string(mensaje.length());
  tamano_mensaje =
      std::string(5 - tamano_mensaje.length(), '0') + tamano_mensaje;
  time_t now = time(0);
  struct tm *now_tm = localtime(&now);
  char time_str[11];
  strftime(time_str, sizeof(time_str), "%y%m%d%H%M", now_tm);

  std::string paquete;
  paquete += tipo;           // (1 byte)
  paquete += tamano_nombre;  // (4 bytes)
  paquete += nombre;         // Nombre
  paquete += tamano_mensaje; // (5 bytes)
  paquete += mensaje;        // Mensaje
  paquete += time_str;       // (10 bytes)

  for (std::map<std::string, int>::iterator it = maPair.begin();
       it != maPair.end(); ++it) {
    if (it->second != clientSocketFD) {
      write(it->second, paquete.c_str(), paquete.length());
    }
  }
}

int varU = 0;
bool sendError(int clientSocketFD, char valorTMP) {
  /*cout << "buffer: " << buffer << endl;*/
  // nickname
  if (valorTMP == 'n') {
    char name_size_str[5] = {0};
    // leer tamaño del nombre (4 bytes)
    if (read_n_bytes(clientSocketFD, name_size_str, 4) <= 0) {
      std::cerr << "Error leyendo tamaño del nombre.\n";
      return false;
    }
    int name_size = std::stoi(name_size_str);
    // leer el nombre según el tamaño
    std::string name(name_size, 0);
    if (read_n_bytes(clientSocketFD, &name[0], name_size) <= 0) {
      std::cerr << "Error leyendo nombre.\n";
      return false;
    }
    /*cout << "sizeN: " << name_size_str << "nombre: " << name << endl;*/
    maPair.insert(std::pair<std::string, int>(name, clientSocketFD));

    ofstream userFile("users.json");
    /*std::map<std::string, int>::iterator it_Prend = maPair.end()--; //
     * T_fix*/
    if (userFile.is_open()) {
      std::string json_data = "{";
      for (std::map<std::string, int>::iterator it = maPair.begin();
           it != maPair.end(); ++it) {
        json_data += '"';
        json_data += it->first;
        json_data += '"';
        /*if (it != it_Prend) {*/
        json_data += ",";
        /*}*/
      }
      json_data += "}";
      userFile << json_data;
      userFile.close();
      std::cout << "Nombre " << name << " almacenado en users.json"
                << std::endl;
    } else {
      std::cerr << "No se pudo abrir el archivo para escribir." << std::endl;
    }
    varU = 0;
    return true;
  } else if (valorTMP == 'm') { // Mensaje privado
    char name_size_str[5] = {0};
    char message_size_str[6] = {0};
    // leer tamaño del nombre (4 bytes)
    if (read_n_bytes(clientSocketFD, name_size_str, 4) <= 0) {
      std::cerr << "Error leyendo tamaño del nombre.\n";
      return false;
    }
    int name_size = std::stoi(name_size_str);
    // leer el nombre según el tamaño
    std::string name(name_size, 0);
    if (read_n_bytes(clientSocketFD, &name[0], name_size) <= 0) {
      std::cerr << "Error leyendo nombre.\n";
      return false;
    }
    // leer el tamaño del mensaje (5 bytes)
    if (read_n_bytes(clientSocketFD, message_size_str, 5) <= 0) {
      std::cerr << "Error leyendo tamaño del mensaje.\n";
      return false;
    }
    int message_size = std::stoi(message_size_str);
    // leer el mensaje según el tamaño
    std::string message(message_size, 0);
    if (read_n_bytes(clientSocketFD, &message[0], message_size) <= 0) {
      std::cerr << "Error leyendo mensaje.\n";
      return false;
    }
    std::cout << "Nombre: " << name << "\n";
    std::cout << "Mensaje: " << message << "\n";
    bool checkUsr = false;
    for (std::map<std::string, int>::iterator it = maPair.begin();
         it != maPair.end(); ++it) {
      if (it->first == name) {
        checkUsr = true;
      }
    }
    if (!checkUsr) {
      std::cout << "Error " << name << " esta offline" << endl;
      varU = 0;
      return true; // false
    }

    enviarPaqueteMensaje(clientSocketFD, name, message);
    varU = 0;
    return true;
  } else if (valorTMP == 'b') { // BROADCAST
    char name_size_str[5] = {0};
    char message_size_str[6] = {0};
    // leer tamaño del nombre (4 bytes)
    if (read_n_bytes(clientSocketFD, name_size_str, 4) <= 0) {
      std::cerr << "Error leyendo tamaño del nombre.\n";
      return false;
    }
    int name_size = std::stoi(name_size_str);
    // leer el nombre según el tamaño
    std::string name(name_size, 0);
    if (read_n_bytes(clientSocketFD, &name[0], name_size) <= 0) {
      std::cerr << "Error leyendo nombre.\n";
      return false;
    }
    // leer el tamaño del mensaje (5 bytes)
    if (read_n_bytes(clientSocketFD, message_size_str, 5) <= 0) {
      std::cerr << "Error leyendo tamaño del mensaje.\n";
      return false;
    }
    int message_size = std::stoi(message_size_str);
    // leer el mensaje según el tamaño
    std::string message(message_size, 0);
    if (read_n_bytes(clientSocketFD, &message[0], message_size) <= 0) {
      std::cerr << "Error leyendo mensaje.\n";
      return false;
    }
    std::cout << "Nombre: " << name << "\n";
    std::cout << "Mensaje: " << message << "\n";
    enviarPaqueteATodos(clientSocketFD, name, message);
    varU = 0;
    return true;
  } else if (valorTMP == 'O') {
    std::string vtmp;
    for (std::map<std::string, int>::iterator it = maPair.begin();
         it != maPair.end();) {
      if (it->second == clientSocketFD) {
        vtmp = it->first;
        it = maPair.erase(it);
      } else {
        ++it;
      }
    }
    ofstream userFile("users.json");
    /*std::map<std::string, int>::iterator it_Prend = maPair.end()--; //
     * T_fix*/
    if (userFile.is_open()) {
      std::string json_data = "{";
      for (std::map<std::string, int>::iterator it = maPair.begin();
           it != maPair.end(); ++it) {
        json_data += '"';
        json_data += it->first;
        json_data += '"';
        /*if (it != it_Prend) {*/
        json_data += ",";
        /*}*/
      }
      json_data += "}";
      userFile << json_data;
      userFile.close();
      std::cout << "Nombre " << vtmp << " removido de users.json" << std::endl;
    } else {
      std::cerr << "No se pudo abrir el archivo para escribir." << std::endl;
    }

    varU = 0;
    return true;
  } else if (valorTMP == 'G') {
    char file_size_str[6] = {0};
    if (read_n_bytes(clientSocketFD, file_size_str, 6) <= 0) {
      std::cerr << "Error leyendo el tamaño del nombre del archivo.\n";
      return false;
    }
    int content_size = std::stoi(file_size_str);
    cout << "valor: " << content_size << endl;
    string a = "./binData/datA.bin";
    genBin(a, content_size);
    leerBin(a);
    varU = 0;
    return true;
  } else if (valorTMP == 'L') {
    int nBytes = 5;
    char command_List[6] = {'L'};

    /*std::map<std::string, int>::iterator it_Prend = maPair.end()--; //
     * T_fix*/
    std::string json_data = "{";
    for (std::map<std::string, int>::iterator it = maPair.begin();
         it != maPair.end(); ++it) {
      json_data += '"';
      json_data += it->first;
      json_data += '"';
      /*if (it != it_Prend) {*/
      json_data += ",";
      /*}*/
    }
    json_data += "}";

    char msg_Ex[json_data.size()];
    strcpy(msg_Ex, json_data.c_str());
    funtionFillCeros(msg_Ex, command_List, nBytes, clientSocketFD);

    varU = 0;
    return true;
  } else {
    std::cerr << "Paquete no reconocido: " << valorTMP << ".\n";
    return false;
  }
  return false;
}

void sendDataS_C(int socketFD) { // SocketDeCliente
  while (true) {
    char type;
    if (read_n_bytes(socketFD, &type, 1) <= 0) {
      std::cerr << "Error leyendo tipo de mensaje.\n";
      return;
    } else {
      varU++;
      if (varU == 1) {
        std::cout << "Tipo: " << type << "\n";
        sendError(socketFD, type);
      }
    }
  }
  close(socketFD);
}

// Función para escuchar mensajes del cliente
void recibir_mensajes(int clientSocket) {
  char buffer[1024];
  while (true) {
    memset(buffer, 0, 1024);
    int bytesReceived = recv(clientSocket, buffer, 1024, 0);
    if (bytesReceived <= 0) {
      cout << "\n[!] El cliente se ha desconectado." << endl;
      close(clientSocket);
      exit(0); // Cerramos el programa si el cliente se va
    }
    cout << "\nCliente: " << buffer << endl;
    cout << "Tú: " << flush; // Volver a mostrar el prompt
  }
}

// Función para enviar mensajes al cliente
void enviar_mensajes(int clientSocket) {
  string mensaje;
  while (true) {
    cout << "Tú: " << flush;
    getline(cin, mensaje);
    if (mensaje == "exit") {
      close(clientSocket);
      exit(0);
    }
    send(clientSocket, mensaje.c_str(), mensaje.size(), 0);
  }
}

int main() {
  int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocketFD == -1) {
    perror("cannot create socket");
    return 1;
  }

  struct sockaddr_in serverAddres;
  memset(&serverAddres, 0, sizeof(struct sockaddr_in));

  serverAddres.sin_family = AF_INET;
  serverAddres.sin_port = htons(45000);
  serverAddres.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocketFD, (const struct sockaddr *)&serverAddres,
           sizeof(struct sockaddr_in)) == -1) {
    perror("error bind failed");
    close(serverSocketFD);
    return 1;
  }

  if (listen(serverSocketFD, 10) == -1) {
    perror("error listen failed");
    close(serverSocketFD);
    return 1;
  }

  while (true) {
    struct sockaddr_in clientAddress;
    int clientSocketFD = accept(serverSocketFD, NULL, NULL);
    if (clientSocketFD < 0) {
      perror("error accept failed");
      close(serverSocketFD);
      return 1;
    }

    cout << "[+] Cliente conectado." << endl;
    std::thread(sendDataS_C, clientSocketFD).detach();
  }

  shutdown(serverSocketFD, SHUT_RDWR);

  // thread t_recv(recibir_mensajes, clientSocket);
  // thread t_send(enviar_mensajes, clientSocket);
  // t_recv.join();
  // t_send.join();
  // close(serverSocket);
  return 0;
}
