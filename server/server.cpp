#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 4096

using namespace std;

// Función para enviar todos los bytes asegurando que se envían completos
bool sendAll(int socket, const char* buffer, size_t length) {
    size_t totalSent = 0;
    while (totalSent < length) {
        ssize_t bytesSent = send(socket, buffer + totalSent, length - totalSent, 0);
        if (bytesSent <= 0) return false;
        totalSent += bytesSent;
    }
    return true;
}

// Manejar la conexión de un cliente
void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];
    
    // 1. Leer el comando inicial (UPLOAD filename tamaño o DOWNLOAD filename)
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }

    string commandStr(buffer);
    stringstream ss(commandStr);
    string command;
    ss >> command;

    if (command == "UPLOAD") {
        string filename;
        size_t filesize;
        ss >> filename >> filesize;

        cout << "[INFO] Petición de subida: " << filename << " (" << filesize << " bytes)" << endl;

        // Responder OK para que el cliente comience a enviar los datos
        string response = "OK\n";
        send(clientSocket, response.c_str(), response.length(), 0);

        // Guardar el archivo localmente en el servidor
        ofstream outfile("recibido_" + filename, ios::binary);
        if (!outfile) {
            cerr << "[ERROR] No se pudo crear el archivo " << filename << endl;
            close(clientSocket);
            return;
        }

        size_t totalReceived = 0;
        char dataBuffer[BUFFER_SIZE];
        while (totalReceived < filesize) {
            size_t toReceive = min((size_t)BUFFER_SIZE, filesize - totalReceived);
            ssize_t received = recv(clientSocket, dataBuffer, toReceive, 0);
            if (received <= 0) break;
            outfile.write(dataBuffer, received);
            totalReceived += received;
        }
        
        outfile.close();
        
        if (totalReceived == filesize) {
            cout << "[INFO] Archivo " << filename << " recibido con éxito." << endl;
            string successMsg = "UPLOAD_SUCCESS\n";
            send(clientSocket, successMsg.c_str(), successMsg.length(), 0);
        } else {
            cerr << "[ERROR] La transferencia se interrumpió." << endl;
        }

    } else if (command == "DOWNLOAD") {
        string filename;
        ss >> filename;
        
        cout << "[INFO] Petición de descarga: " << filename << endl;
        
        ifstream infile(filename, ios::binary | ios::ate);
        if (!infile) {
            // Archivo no existe en el servidor
            string response = "ERROR FILE_NOT_FOUND\n";
            send(clientSocket, response.c_str(), response.length(), 0);
            cerr << "[ERROR] El archivo solicitado no existe." << endl;
        } else {
            // Obtener el tamaño del archivo
            size_t filesize = infile.tellg();
            infile.seekg(0, ios::beg);
            
            // Enviar OK y el tamaño
            string response = "OK " + to_string(filesize) + "\n";
            send(clientSocket, response.c_str(), response.length(), 0);
            
            // Esperar un pequeño instante o asumir que el cliente ya está listo para recibir (el TCP stream asegura el orden)
            // Enviar los datos del archivo
            char dataBuffer[BUFFER_SIZE];
            size_t totalSent = 0;

            while (totalSent < filesize) {
                infile.read(dataBuffer, BUFFER_SIZE);
                size_t bytesReadFromFile = infile.gcount();

                if (bytesReadFromFile == 0) {
                    cerr << "[ERROR] Fin de archivo inesperado o archivo corrupto." << endl;
                    break; // Esto rompe el bucle infinito
                }


                if (!sendAll(clientSocket, dataBuffer, bytesReadFromFile)) {
                    cerr << "[ERROR] Se cortó la conexión enviando el archivo." << endl;
                    break;
                }
                totalSent += bytesReadFromFile;
            }
            infile.close();
            
            cout << "[INFO] Archivo " << filename << " enviado al cliente." << endl;
        }
    } else {
        cout << "[ERROR] Comando no reconocido: " << command << endl;
    }

    close(clientSocket);
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // 1. Crear socket (IPv4, TCP)
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "[ERROR] No se pudo crear el socket" << endl;
        return 1;
    }

    // Permitir reutilizar el puerto
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. Configurar la dirección del servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Escuchar en todas las interfaces de red
    serverAddr.sin_port = htons(PORT);

    // 3. Vincular el socket (Bind)
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "[ERROR] Fallo en el bind en el puerto " << PORT << endl;
        close(serverSocket);
        return 1;
    }

    // 4. Poner a escuchar el socket (Listen)
    if (listen(serverSocket, 5) < 0) {
        cerr << "[ERROR] Fallo en listen" << endl;
        close(serverSocket);
        return 1;
    }

    cout << "========================================" << endl;
    cout << "     Servidor de Archivos Iniciado      " << endl;
    cout << "========================================" << endl;
    cout << "Escuchando conexiones en el puerto " << PORT << "..." << endl;

    // 5. Bucle infinito para aceptar y atender clientes
    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            cerr << "[ERROR] Fallo al aceptar el cliente" << endl;
            continue;
        }

        cout << "\n[INFO] Nuevo cliente conectado." << endl;
        // En una implementación avanzada, aquí crearíamos un hilo (thread)
        // para atender múltiples clientes simultáneamente.
        handleClient(clientSocket);
    }

    close(serverSocket);
    return 0;
}
