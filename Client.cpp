#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int BUFFER_SIZE = 1024;

void error(const char *msg) {
    cerr << msg << endl;
    exit(1);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error("Ошибка при инициализации Winsock");
    }

    SOCKET clientSocket;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        error("Ошибка при создании сокета");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // Порт сервера
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP сервера

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        error("Ошибка при подключении к серверу");
    }

    char username[100];
    cout << "Введите ваше имя: ";
    cin.getline(username, 100);

    send(clientSocket, username, strlen(username), 0);

    while (true) {
        char message[BUFFER_SIZE];
        cout << "Введите сообщение: ";
        cin.getline(message, BUFFER_SIZE);

        send(clientSocket, message, strlen(message), 0);

        if (strcmp(message, "exit") == 0) {
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
