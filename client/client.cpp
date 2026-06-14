#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

using namespace std;

// Función sugerida para enviar todos los datos (útil para el compañero)
bool sendAll(int socket, const char* buffer, size_t length) {
    size_t totalSent = 0;
    while (totalSent < length) {
        ssize_t bytesSent = send(socket, buffer + totalSent, length - totalSent, 0);
        if (bytesSent <= 0) return false;
        totalSent += bytesSent;
    }
    return true;
}

void uploadFile(int sock, const string& filename) {
    // TAREA PARA EL COMPAÑERO:
    // 1. Abrir el archivo local usando ifstream
    // 2. Obtener el tamaño del archivo
    // 3. Enviar el comando: "UPLOAD <filename> <size>"
    // 4. Recibir la respuesta del servidor (esperar un "OK\n")
    // 5. Enviar el contenido del archivo mediante un bucle
    // 6. Recibir confirmación final
    cout << "[TODO] Implementar subida de archivo" << endl;
}

void downloadFile(int sock, const string& filename) {
    // TAREA PARA EL COMPAÑERO:
    // 1. Enviar el comando: "DOWNLOAD <filename>"
    // 2. Recibir la respuesta del servidor (puede ser "OK <size>" o "ERROR")
    // 3. Si es OK, leer el tamaño
    // 4. Crear un archivo local usando ofstream
    // 5. Recibir los datos en un bucle usando recv hasta alcanzar el tamaño indicado
    cout << "[TODO] Implementar descarga de archivo" << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <IP_del_servidor>" << endl;
        return 1;
    }

    string serverIp = argv[1];
    int sock = 0;
    struct sockaddr_in serv_addr;

    // 1. Crear el socket del cliente
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "[ERROR] Error al crear el socket" << endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 2. Convertir la dirección IP de string a formato binario
    if (inet_pton(AF_INET, serverIp.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr << "[ERROR] Dirección inválida / No soportada" << endl;
        return -1;
    }

    // 3. Conectar al servidor
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "[ERROR] Conexión fallida al servidor " << serverIp << " en el puerto " << PORT << endl;
        return -1;
    }

    cout << "Conectado exitosamente al servidor!" << endl;

    // Menú interactivo básico para el cliente
    int opcion;
    string filename;
    
    do {
        cout << "\n=== MENÚ DEL CLIENTE ===" << endl;
        cout << "1. Subir archivo (Upload)" << endl;
        cout << "2. Descargar archivo (Download)" << endl;
        cout << "3. Salir" << endl;
        cout << "Elija una opción: ";
        cin >> opcion;

        switch (opcion) {
            case 1:
                cout << "Ingrese el nombre del archivo a subir: ";
                cin >> filename;
                uploadFile(sock, filename);
                break;
            case 2:
                cout << "Ingrese el nombre del archivo a descargar: ";
                cin >> filename;
                downloadFile(sock, filename);
                break;
            case 3:
                cout << "Desconectando..." << endl;
                break;
            default:
                cout << "Opción inválida." << endl;
                break;
        }
    } while (opcion != 3);

    // Cerrar el socket al salir
    close(sock);
    return 0;
}
