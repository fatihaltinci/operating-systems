#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>
#include <string.h>

#define MAX_CLIENTS 10

// User struct definition
struct User {
    int userID;
    int phoneNumber;
    char name[50];
    char surname[50];
    SOCKET socket;
};

void HandleClient(void *clientSocket);
void RemoveUserFromServer(int userID);
void AddUserToServer(int userID, SOCKET socket);
void BroadcastMessageToOtherUsers(int senderID, const char *message);

struct User userList[MAX_CLIENTS]; // User list
int userCount = 0; // Number of users

void RemoveUserFromServer(int userID) {
    int userIndex = -1;
    int i;
    for (i = 0; i < userCount; i++) {
        if (userList[i].userID == userID) {
            userIndex = i;
            break;
        }
    }

    if (userIndex != -1) {
        for (i = userIndex; i < userCount - 1; i++) {
            userList[i] = userList[i + 1];
        }
        userCount--;
    }
}

void AddUserToServer(int userID, SOCKET socket) {
    int i;
    if (userCount < MAX_CLIENTS) {
        userList[userCount].userID = userID;
        userList[userCount].socket = socket;
        printf("New user added to the server: UserID %d\n", userID);
        userCount++;
    } else {
        printf("User list is full. Cannot add more users.\n");
    }

    printf("UserList on Server:\n");
    for (i = 0; i < userCount; i++) {
        printf("UserID: %d\n", userList[i].userID);
    }
}

void BroadcastMessageToOtherUsers(int senderID, const char *message) {
    int i;
    for (i = 0; i < userCount; i++) {
        if (userList[i].userID != senderID) {
            // Construct the final message format
            char finalMessage[1024];
            fflush(stdin);  // Clear the buffer from previous input
            sprintf(finalMessage, "Client %d -> %s", senderID, message);

            send(userList[i].socket, finalMessage, strlen(finalMessage), 0);
        }
    }
}

void HandleClient(void *clientSocket) {
    SOCKET socket = (SOCKET)clientSocket;
    char buffer[1024];
    int bytesReceived;
    int i;
    // Login handling
    struct User newUser;
    recv(socket, (char *)&newUser, sizeof(newUser), 0);
    printf("Client logged in: UserID %d\n", newUser.userID);
    int userIndex = -1;
    for (i = 0; i < userCount; i++) {
        if (userList[i].userID == newUser.userID) {
            userIndex = i;
            break;
        }
    }

    if (userIndex == -1) {
        // If user does not exist, add to the user list
        AddUserToServer(newUser.userID, socket);
    }

    // Main message handling loop
    while (1) {
        // Receive messages from the client
        bytesReceived = recv(socket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            // If the client disconnects, remove the user from the list
            RemoveUserFromServer(newUser.userID);
            printf("Client disconnected: UserID %d\n", newUser.userID);
            break;
        }

        // Process the received message
        buffer[bytesReceived] = '\0';

        // Forward the message to all other users
        BroadcastMessageToOtherUsers(newUser.userID, buffer);
    }

    // Client end socket
    closesocket(socket);
    _endthread();
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    int threadID;
    int PORT = 12345;

    // Winsock ba�latma
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    // Soket olu�turma
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\n");
        return 1;
    }

    // Server adres bilgilerini ayarlama
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Soketi ba�lama
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Socket bind failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Dinleme ba�latma
    if (listen(serverSocket, MAX_CLIENTS) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);
    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed\n");
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        printf("Client connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        _beginthread(HandleClient, 0, (void *)clientSocket);
    }

    // Temizleme i�lemleri
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}