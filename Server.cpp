#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <winsock2.h>
#include <vector>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int BUFFER_SIZE = 1024;

class User
{
    //основные поля, с которыми мы будем работать
    string name;
    string login;
    string password;
    vector<string>messages;//сообщений в чате может вводиться много, поэтому для них нужен динамический массив

public:
    //конструктор
    User(const string& name, const string& login, const string& password) : name(name), login(login), password(password)
    {

    }
    //геттер приватного поля - имени
    const string& getName() const
    {
        return name;
    }
    //геттер приватного поля - логина
    const string& getLogin() const
    {
        return login;
    }
    //функция проверки пароля
    bool checkPassword(const string& inputPassword) const
    {
        return password == inputPassword;
    }
    //функция отправки сообщения
    void addMessage(const string& message)
    {
        messages.push_back(message);
    }
    //геттер сообщений из массива
    const vector<string>& getMessages() const
    {
        return messages;
    }
};

class Chat
{
public:
    //динамический массив из пользователей чата
    vector<User> users;
    //функция регистрации пользователя
    //функцией push_back добавляем элементы в динамический массив
    void registerUser(const string& name, const string& login, const string& password)
    {
        users.push_back(User(name, login, password));
        cout << "Пользователь " << name << " успешно зарегистрирован" << endl;
    }
    //функция входа зарегистрированного пользователя
    //создаем объект класса User
    //внутри цикла проверяем условия, что введенное имя логина равно геттеру логина и пароль введен правильно
    User* login(const string& login, const string& password)
    {
        for (User& user : users)
        {
            if (user.getLogin() == login && user.checkPassword(password))
            {
                cout << "Вход выполнен как " << user.getName() << endl;
                return &user;//работаем уже с этим пользователем, а не обнуляемся
            }
        }
        cout << "Неверный логин или пароль." << endl;
        return nullptr;
    }
    //функция отправки сообщениия от пользователя к другому
    void sendMessage(User& sender, User& receiver, const string& message)
    {
        receiver.addMessage("Личное от " + sender.getName() + ": " + message);
    }

    void sendToChat(User& sender, const string& message)
    {
        for (User& user : users)
        {
            if (&user != &sender)
            {
                user.addMessage("Общий чат: " + message);
            }
        }
    }
};

struct Client 
{
    SOCKET socket;
};

vector<Client> clients;
vector<string> messageHistory;

void error(const char* msg) 
{
    cerr << msg << endl;
    exit(1);
}

void broadcastMessage(const string& message, const string& sender) 
{
    messageHistory.push_back(sender + ": " + message);
    for (const Client& client : clients) 
    {
        if (client.username != sender) 
        {
            send(client.socket, (sender + ": " + message).c_str(), message.size() + sender.size() + 2, 0);
        }
    }
}

void clientHandler(Client client) 
{
    char clientMessage[BUFFER_SIZE];
    while (true) 
    {
        int bytesRead = recv(client.socket, clientMessage, BUFFER_SIZE, 0);
        if (bytesRead <= 0) 
        {
            cout << "Klient " << client.username << " otklyuchilsya." << endl;
            closesocket(client.socket);
            break;
        }

        clientMessage[bytesRead] = '\0';
        string message = clientMessage;
        cout << "Klient " << client.username << ": " << message << endl;

        if (message == "exit") 
        {
            cout << "Klient " << client.username << " vushel iz chata." << endl;
            closesocket(client.socket);
            break;
        }

        broadcastMessage(message, client.username);
    }

    // Удаление клиента из списка
    for (auto it = clients.begin(); it != clients.end(); ++it) 
    {
        if (it->socket == client.socket) 
        {
            clients.erase(it);
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        error("Oshibka pri inicializacii Winsock");
    }

    SOCKET serverSocket;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) 
    {
        error("Oshibka pri sozdanii soketa");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // Порт сервера
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        error("Oshibka pri privyazke soketa k portu");
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) 
    {
        error("Oshibka pri proslushivanii porta");
    }

    cout << "Server zapushen. Ozhidanie klientov..." << endl;

    while (true) 
    {
        SOCKET clientSocket;
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);

        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) 
        {
            cerr << "Oshibka pri prinyatii soedineniya s klientom" << endl;
            continue;
        }

        char username[BUFFER_SIZE];
        int bytesRead = recv(clientSocket, username, BUFFER_SIZE, 0);
        if (bytesRead <= 0) 
        {
            cerr << "Oshibka pri chtenii imeni klienta" << endl;
            closesocket(clientSocket);
            continue;
        }
        username[bytesRead] = '\0';

        Client client;
        client.socket = clientSocket;
        client.username = username;

        cout << "Klient " << client.username << " podklyuchilsya." << endl;
        clients.push_back(client);

        thread clientThread(clientHandler, client);
        clientThread.detach(); // Отсоединяем поток клиента
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
