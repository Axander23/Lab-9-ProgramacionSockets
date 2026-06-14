# Laboratorio 9 - Programación de Sockets

Este repositorio contiene la implementación del proyecto de la asignatura sobre intercambio de archivos a través de Sockets en C++ (entorno Linux POSIX).

## Estructura

- `server/server.cpp`: Código del Servidor (Parte 1).
- `client/client.cpp`: Esqueleto inicial del Código del Cliente (Parte 2).
- `Makefile`: Archivo de configuración para compilar ambos proyectos.

## Instrucciones de Compilación (Linux / WSL)

1. Abrir una terminal en la raíz de este proyecto.
2. Ejecutar `make` para compilar todo. Se generarán los ejecutables `server/server` y `client/client`.

## Ejecución

1. En una terminal, iniciar el servidor:
   ```bash
   cd server
   ./server
   ```
2. En otra terminal, iniciar el cliente:
   ```bash
   cd client
   ./client 127.0.0.1
   ```

## Protocolo de Comunicación (Para el Compañero)

El servidor y cliente deben comunicarse en texto plano para los comandos, seguido del flujo binario de los archivos.

### 1. Subir un Archivo (UPLOAD)
- **Cliente:** Envía una cadena con el formato: `UPLOAD nombre_archivo tamaño_en_bytes` (Ej: `UPLOAD prueba.txt 1024`). Se pueden enviar espacios y rellenar si sobra espacio en el buffer.
- **Servidor:** Responde con una cadena `OK\n`.
- **Cliente:** Al recibir el `OK`, envía todo el contenido binario del archivo por el socket (`send`).
- **Servidor:** Recibe los datos y finalmente responde con `UPLOAD_SUCCESS\n`.

### 2. Descargar un Archivo (DOWNLOAD)
- **Cliente:** Envía una cadena con el formato: `DOWNLOAD nombre_archivo` (Ej: `DOWNLOAD prueba.txt`).
- **Servidor:**
  - Si el archivo existe, responde `OK tamaño_en_bytes\n` (Ej: `OK 1024\n`).
  - Si no existe, responde `ERROR FILE_NOT_FOUND\n`.
- **Servidor:** Si existía, a continuación envía todo el contenido binario por el socket.
- **Cliente:** Recibe el archivo usando `recv` en un bucle hasta alcanzar el tamaño indicado por el servidor.
