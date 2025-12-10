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
int varU = 0;
bool sendError(int clientSocketFD, char valorTMP) {

  if (valorTMP == 'b') { // BROADCAST
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

int main() {
  ... while (true) {
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
}
