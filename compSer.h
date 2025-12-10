#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

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

bool sendAll(int sock, const void *data, size_t len) {
  const char *ptr = static_cast<const char *>(data);
  size_t totalSent = 0;
  while (totalSent < len) {
    // ssize_t sent = send(sock, ptr + totalSent, len - totalSent, 0);
    ssize_t written = write(sock, ptr + totalWritten, len - totalWritten);
    if (written <= 0)
      return false; // Error o desconexión
    totalSent += written;
  }
  return true;
}

// ----------------------------------------------------------------------
// 2. Función Principal
// ----------------------------------------------------------------------
void enviarfilesAt() {
  // A. OBTENER EL PRIMER CLIENTE
  if (maPair.empty()) {
    std::cerr << "[!] No hay clientes conectados." << std::endl;
    return;
  }
  // Obtenemos el socket del primer elemento del mapa
  int clientSocket = maPair.begin()->second;
  std::string clientID = maPair.begin()->first;
  std::cout << "[*] Iniciando transferencia a: " << clientID << std::endl;
  // B. ABRIR ARCHIVO (>4GB)
  std::string filename = "./binData/R.bin"; // Tu archivo de doubles
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    std::cerr << "[!] Error abriendo el archivo." << std::endl;
    return;
  }
  // tellg() devuelve streampos, que es 64 bits en sistemas modernos
  uint64_t fileSize = file.tellg();
  file.seekg(0, std::ios::beg); // Volver al inicio para leer datos
  std::cout << "[*] Tamaño del archivo: " << fileSize << " bytes" << std::endl;
  // C. CONSTRUIR EL HEADER USANDO STD::STRING
  // Protocolo: <Tipo:1><LenNombre:2><SizeArchivo:8><Nombre:Variable>
  std::string cabecera;
  cabecera += 'R'; // 'D' de Data/Double
  // 2. Longitud del nombre (2 bytes)
  uint16_t nameLen = static_cast<uint16_t>(filename.length());
  uint16_t nameLenNet = htons(nameLen); // Endianness de red
  cabecera.append(reinterpret_cast<const char *>(&nameLenNet),
                  sizeof(nameLenNet));

  cabecera.append(reinterpret_cast<const char *>(&fileSize), sizeof(fileSize));
  cabecera += filename;
  if (!sendAll(clientSocket, cabecera.data(), cabecera.size())) {
    std::cerr << "[!] Error enviando cabecera." << std::endl;
    return;
  }
  const int CHUNK_SIZE = 65536; // 64 KB
  std::vector<char> buffer(CHUNK_SIZE);
  uint64_t totalEnviado = 0;
  while (file) {
    file.read(buffer.data(), CHUNK_SIZE);
    std::streamsize bytesLeidos = file.gcount(); // Cuánto leímos realmente
    if (bytesLeidos > 0) {
      if (!sendAll(clientSocket, buffer.data(), bytesLeidos)) {
        std::cerr << "[!] Error enviando chunk. Abortando." << std::endl;
        break;
      }
      totalEnviado += bytesLeidos;
    } else {
      break; // Fin del archivo
    }
    if (totalEnviado % (500 * 1024 * 1024) == 0) { // Cada 500MB
      std::cout << "--> Enviado: " << totalEnviado / (1024 * 1024) << " MB"
                << std::endl;
    }
  }
  file.close();
  std::cout << "[*] Transferencia completada. Total: " << totalEnviado
            << " bytes." << std::endl;
}

// void enviarfilesAt(int senderSocketFD) {
//   const char *filename = "archivo_grande.bin";
//   std::ifstream file(filename, std::ios::binary | std::ios::ate);
//   if (!file.is_open()) {
//     std::cerr << "Error al abrir el archivo" << std::endl;
//     return;
//   }
//   // 1. OBTENER TAMAÑO DEL ARCHIVO
//   uint64_t fileSize = file.tellg();
//   file.seekg(0, std::ios::beg);
//   // 2. PREPARAR LA CABECERA (HEADER)
//   // Protocolo: <Tipo(1)><LongitudNombre(2)><TamañoArchivo(8)><Nombre(...)>
//   uint16_t nameLen =
//       htons(strlen(filename)); // htons para orden de red (2 bytes)
//   uint64_t sizeNet = 0;
//   // Conversión manual a Big Endian para 64 bits (o usar htobe64 en
//   Linux/BSD)
//   // Para simplificar aquí asumiremos que cliente y servidor tienen el mismo
//   // Endianness Si es producción, debes asegurar el Endianness.
//   sizeNet = fileSize;
//
//   // Construimos el buffer de la cabecera
//   std::vector<char> header;
//   header.push_back('F'); // Tipo 'F'ile
//
//   // Insertar longitud del nombre (2 bytes crudos)
//   char *lenPtr = reinterpret_cast<char *>(&nameLen);
//   header.insert(header.end(), lenPtr, lenPtr + sizeof(nameLen));
//
//   // Insertar tamaño del archivo (8 bytes crudos)
//   char *sizePtr = reinterpret_cast<char *>(&sizeNet);
//   header.insert(header.end(), sizePtr, sizePtr + sizeof(sizeNet));
//   // Insertar el nombre del archivo
//   for (int i = 0; i < strlen(filename); i++)
//     header.push_back(filename[i]);
//   // 3. ENVIAR CABECERA A TODOS (Broadcast Header)
//   for (auto const &[key, clientSocket] : maPair) {
//     if (clientSocket != senderSocketFD) {
//       send(clientSocket, header.data(), header.size(), 0);
//     }
//   }
//   const int BUFFER_SIZE = 65536; // 64 KB
//   char buffer[BUFFER_SIZE];
//   while (file.read(buffer, BUFFER_SIZE) || file.gcount() > 0) {
//     int bytesLeidos =
//         file.gcount(); // Cuánto se leyó realmente (útil para la última
//         parte)
//     // Iteramos sobre todos los clientes para enviar ESTE pedacito
//     for (auto const &[key, clientSocket] : maPair) {
//       if (clientSocket != senderSocketFD) {
//         // Importante: send puede no enviar todo de una vez, en producción
//         // deberías hacer un bucle while hasta enviar 'bytesLeidos'.
//         ssize_t sent = send(clientSocket, buffer, bytesLeidos, 0);
//         if (sent == -1) {
//           // Manejar error (cliente desconectado, etc.)
//         }
//       }
//     }
//   }
//   file.close();
//   std::cout << "Archivo enviado a todos los clientes." << std::endl;
// }

bool processPacket(int clientSocketFD, char tipo) {
  if (tipo == 'n') { // user log
    char name_size_str[5] = {0};
    if (read_n_bytes(clientSocketFD, name_size_str, 4) <= 0) {
      std::cerr << "Error leyendo tamaño del nombre.\n";
      return false;
    }
    int name_size = std::stoi(name_size_str);
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
    return true;
  } else if (tipo == 'm') { // Mensaje privado
    char name_size_str[5] = {0};
    char message_size_str[6] = {0};
    if (read_n_bytes(clientSocketFD, name_size_str, 4) <= 0) {
      std::cerr << "Error leyendo tamaño del nombre.\n";
      return false;
    }
    int name_size = std::stoi(name_size_str);
    std::string name(name_size, 0);
    if (read_n_bytes(clientSocketFD, &name[0], name_size) <= 0) {
      std::cerr << "Error leyendo nombre.\n";
      return false;
    }
    if (read_n_bytes(clientSocketFD, message_size_str, 5) <= 0) {
      std::cerr << "Error leyendo tamaño del mensaje.\n";
      return false;
    }
    int message_size = std::stoi(message_size_str);
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
      return false;
    }
    enviarPaqueteMensaje(clientSocketFD, name, message);
    return true;
  } else if (tipo == 'b') { // BROADCAST
    // char name_size_str[5] = {0};
    // char message_size_str[6] = {0};
    // // leer tamaño del nombre (4 bytes)
    // if (read_n_bytes(clientSocketFD, name_size_str, 4) <= 0) {
    //   std::cerr << "Error leyendo tamaño del nombre.\n";
    //   return false;
    // }
    // int name_size = std::stoi(name_size_str);
    // // leer el nombre según el tamaño
    // std::string name(name_size, 0);
    // if (read_n_bytes(clientSocketFD, &name[0], name_size) <= 0) {
    //   std::cerr << "Error leyendo nombre.\n";
    //   return false;
    // }
    // leer el tamaño del mensaje (5 bytes)
    // if (read_n_bytes(clientSocketFD, message_size_str, 5) <= 0) {
    //   std::cerr << "Error leyendo tamaño del mensaje.\n";
    //   return false;
    // }
    // int message_size = std::stoi(message_size_str);
    // // leer el mensaje según el tamaño
    // std::string message(message_size, 0);
    // if (read_n_bytes(clientSocketFD, &message[0], message_size) <= 0) {
    //   std::cerr << "Error leyendo mensaje.\n";
    //   return false;
    // }
    // std::cout << "Nombre: " << name << "\n";
    // std::cout << "Mensaje: " << message << "\n";
    // enviarPaqueteATodos(clientSocketFD, name, message);
    // return true;
    /////////////////////////
    int name_size = leer_entero_seguro(clientSocketFD, 4);
    if (name_size < 0)
      return false;
    string name(name_size, '\0');
    if (read_n_bytes(clientSocketFD, &name[0], name_size) <= 0)
      return false;
    int message_size = leer_entero_seguro(clientSocketFD, 5);
    if (message_size < 0)
      return false;
    string message(message_size, '\0');
    if (read_n_bytes(clientSocketFD, &message[0], message_size) <= 0)
      return false;
    cout << "Nombre: " << name << "\n";
    cout << "Mensaje: " << message << "\n";
    enviarPaqueteATodos(clientSocketFD, name, message);
    return true;
  } else if (tipo == 'O') {
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
    return true;
  } else if (tipo == 'G') {
    int content_size = leer_entero_seguro(clientSocketFD, 5);
    if (content_size < 0)
      return false;
    cout << "Tamano a generar: " << content_size << endl;
    string a = "./binData/datA.bin";
    genBin(a, content_size);
    leerBin(a);
    return true;
  } else if (tipo == 'L') {
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
    return true;
  } else if (tipo == 'I') {
    // int name_size = leer_entero_seguro(clientSocketFD, 4);
    // if (name_size < 0)
    // return false;
    // string name(name_size, '\0');
    // if (read_n_bytes(clientSocketFD, &name[0], name_size) <= 0)
    // return false;
    // int message_size = leer_entero_seguro(clientSocketFD, 5);
    // if (message_size < 0)
    //   return false;
    // string message(message_size, '\0');
    // if (read_n_bytes(clientSocketFD, &message[0], message_size) <= 0)
    //   return false;
    // cout << "Nombre: " << name << "\n";
    // cout << "Mensaje: " << message << "\n";
    // enviarPaqueteATodos(clientSocketFD, name, message);
    enviarfilesAt();
    return true;
  }
  std::cerr << "Paquete no reconocido: " << tipo << endl;
  return false;
}

void handle_client(int socketFD) {
  while (true) {
    char type;
    ssize_t res = read_n_bytes(socketFD, &type, 1);
    if (res <= 0) {
      cout << "Cliente desconectado o error." << endl;
      break;
    } else {
      processPacket(socketFD, type);
    }
    // if (!processPacket(socketFD, type)) {
    //   cerr << "Error procesando paquete. Cerrando conexión." << endl;
    //   break;
    // }
  }
  close(socketFD);
}

int initServer() {
  int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
  int puerto = 45000;
  if (serverSocketFD == -1) {
    perror("No se pudo crear el socket");
    return 1;
  }
  // int opt = 1;
  // if (setsockopt(serverSocketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))
  // < 0) {
  // perror("Error en setsockopt");
  // return 1;
  // }
  struct sockaddr_in serverAddres;
  memset(&serverAddres, 0, sizeof(struct sockaddr_in));

  serverAddres.sin_family = AF_INET;
  serverAddres.sin_port = htons(puerto);
  serverAddres.sin_addr.s_addr = INADDR_ANY;
  // address.sin_addr.s_addr = inet_addr("127.0.0.1");
  //
  if (bind(serverSocketFD, (const struct sockaddr *)&serverAddres,
           sizeof(struct sockaddr_in)) == -1) {
    perror("Error en bind");
    close(serverSocketFD);
    return 1;
  }

  if (listen(serverSocketFD, 10) == -1) {
    perror("Error en listen");
    close(serverSocketFD);
    return 1;
  }
  cout << "Servidor escuchando en el puerto " << puerto << "..." << endl;

  while (true) {
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    int clientSocketFD =
        accept(serverSocketFD, (struct sockaddr *)&clientAddress, &clientLen);
    if (clientSocketFD < 0) {
      perror("Error al aceptar cliente (ignorando)");
      continue;
    }
    cout << "[+] Cliente conectado desde IP: "
         << inet_ntoa(clientAddress.sin_addr)
         << " Puerto: " << ntohs(clientAddress.sin_port) << endl;
    std::thread(handle_client, clientSocketFD).detach();
  }
  close(serverSocketFD);
  return 0;
}
