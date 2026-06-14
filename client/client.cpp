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
    // 1. Abrir el archivo local usando ifstream en modo binario
    ifstream infile(filename, ios::binary | ios::ate);
    if (!infile) {
        cerr << "[ERROR] No se pudo encontrar o abrir el archivo local: " << filename << endl;
        return;
    }

    // 2. Obtener el tamaño del archivo
    size_t filesize = infile.tellg();
    infile.seekg(0, ios::beg);

    // 3. Enviar el comando: "UPLOAD <filename> <size>"
    string command = "UPLOAD " + filename + " " + to_string(filesize) + "\n";
    sendAll(sock, command.c_str(), command.length());

    // 4. Recibir la respuesta del servidor (esperar un "OK\n")
    char responseBuf[256];
    memset(responseBuf, 0, sizeof(responseBuf));
    recv(sock, responseBuf, sizeof(responseBuf) - 1, 0);
    
    string response(responseBuf);
    if (response.find("OK") == string::npos) {
        cerr << "[ERROR] El servidor rechazó la subida. Respuesta: " << response << endl;
        infile.close();
        return;
    }

    cout << "[INFO] Servidor listo. Enviando archivo de " << filesize << " bytes..." << endl;

    // 5. Enviar el contenido del archivo mediante un bucle
    char dataBuffer[BUFFER_SIZE];
    size_t totalSent = 0;
    while (totalSent < filesize) {
        infile.read(dataBuffer, BUFFER_SIZE);
        size_t bytesRead = infile.gcount();
        
        if (!sendAll(sock, dataBuffer, bytesRead)) {
            cerr << "[ERROR] La conexión se perdió mientras se enviaba el archivo." << endl;
            break;
        }
        totalSent += bytesRead;
    }
    infile.close();

    // 6. Recibir confirmación final
    memset(responseBuf, 0, sizeof(responseBuf));
    recv(sock, responseBuf, sizeof(responseBuf) - 1, 0);
    cout << "[INFO] " << responseBuf; // Debería imprimir UPLOAD_SUCCESS
}





void downloadFile(int sock, const string& filename) {
    // 1. Enviar el comando: "DOWNLOAD <filename>"
    string command = "DOWNLOAD " + filename + "\n";
    sendAll(sock, command.c_str(), command.length());

    // 2. Recibir la respuesta del servidor leyendo caracter por caracter hasta el salto de línea '\n'
    // IMPORTANTE: Se hace así porque el servidor envía "OK <size>\n" y pegado envía los datos binarios.
    string header = "";
    char c;
    while (recv(sock, &c, 1, 0) > 0) {
        header += c;
        if (c == '\n') break;
    }

    stringstream ss(header);
    string status;
    ss >> status;

    // 3. Si es OK, leer el tamaño
    if (status == "OK") {
        size_t filesize;
        ss >> filesize;

        cout << "[INFO] Preparando para recibir " << filesize << " bytes..." << endl;

        // 4. Crear un archivo local usando ofstream en modo binario
        ofstream outfile("descargado_"+filename, ios::binary);
        if (!outfile) {
            cerr << "[ERROR] No se pudo crear el archivo local para guardar la descarga." << endl;
            return;
        }

        // 5. Recibir los datos en un bucle usando recv hasta alcanzar el tamaño indicado
        char dataBuffer[BUFFER_SIZE];
        size_t totalReceived = 0;
        
        while (totalReceived < filesize) {
            size_t toReceive = min((size_t)BUFFER_SIZE, filesize - totalReceived);
            ssize_t received = recv(sock, dataBuffer, toReceive, 0);
            
            if (received <= 0) {
                cerr << "[ERROR] Conexión interrumpida durante la descarga." << endl;
                break;
            }
            
            outfile.write(dataBuffer, received);
            totalReceived += received;
        }
        
        outfile.close();
        if (totalReceived == filesize) {
            cout << "[INFO] Archivo " << filename << " descargado exitosamente." << endl;
        }
        
    } else {
        // Manejar el caso de ERROR (por ejemplo, FILE_NOT_FOUND)
        cout << "[ERROR] El servidor respondió: "   << header;
    }
}




int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <IP_del_servidor>" << endl;
        return 1;
    }

    string serverIp = argv[1];
    int sock = 0;
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 2. Convertir la dirección IP de string a formato binario (ESTO SE QUEDA AFUERA)
    if (inet_pton(AF_INET, serverIp.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr << "[ERROR] Dirección inválida / No soportada" << endl;
        return -1;
    }

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

        // Si elige salir, rompemos el bucle antes de intentar conectar
        if (opcion == 3) {
            cout << "Desconectando..." << endl;
            break;
        }

        // 1. Crear el socket del cliente para esta petición
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            cerr << "[ERROR] Error al crear el socket" << endl;
            continue;
        }

        // 3. Conectar al servidor para esta petición
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            cerr << "[ERROR] Conexión fallida al servidor " << serverIp << " en el puerto " << PORT << endl;
            close(sock);
            continue;
        }

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
            default:
                cout << "Opción inválida." << endl;
                break;
        }

        // Cerrar el socket al terminar la opción elegida 
        close(sock);

    } while (opcion != 3);

    return 0;
}