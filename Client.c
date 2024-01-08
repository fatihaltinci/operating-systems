#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>

#define MAX_CLIENTS 10

struct User {
    int userID;
    int phoneNumber;
    char name[50];
    char surname[50];
    SOCKET socket;
};

void ListContacts();
void AddUser();
void DeleteUser();
void SendMessageToUser();
void CheckMessages();

struct User clientContactList[MAX_CLIENTS];
int clientContactCount = 0;
SOCKET clientSocket;

void ListContacts() {
    int i;
    printf("Contact List:\n");
    for (i = 0; i < clientContactCount; i++) {
        printf("UserID: %d, Name: %s, Surname: %s, Phone Number: %d\n",
               clientContactList[i].userID,
               clientContactList[i].name,
               clientContactList[i].surname,
               clientContactList[i].phoneNumber);
    }
}

void AddUser() {
    struct User newUser;
    int i;
    printf("Enter UserID: ");
    scanf("%d", &newUser.userID);

    // Check if the user is already in the contact list
    for (i = 0; i < clientContactCount; i++) {
        if (clientContactList[i].userID == newUser.userID) {
            printf("User is already in the contact list.\n");
            return;
        }
    }

    printf("Enter Name: ");
    fflush(stdin);  // Clear the buffer from previous input
    fgets(newUser.name, sizeof(newUser.name), stdin);
    newUser.name[strcspn(newUser.name, "\n")] = '\0';  // Remove trailing newline

    printf("Enter Surname: ");
    fflush(stdin);  // Clear the buffer from previous input
    fgets(newUser.surname, sizeof(newUser.surname), stdin);
    newUser.surname[strcspn(newUser.surname, "\n")] = '\0';  // Remove trailing newline

    printf("Enter Phone Number: ");
    scanf("%d", &newUser.phoneNumber);

    // Add the new user to the contact list
    clientContactList[clientContactCount] = newUser;
    clientContactCount++;

    send(clientSocket, (char *)&newUser, sizeof(newUser), 0);

    printf("User added to the contact list.\n");
}

void DeleteUser() {
    int userID;
    int i;
    printf("Enter UserID to delete: ");
    scanf("%d", &userID);

    // Find the user in the contact list
    int userIndex = -1;
    for (i = 0; i < clientContactCount; i++) {
        if (clientContactList[i].userID == userID) {
            userIndex = i;
            break;
        }
    }

    // Delete the user from the contact list
    if (userIndex != -1) {
        for (i = userIndex; i < clientContactCount - 1; i++) {
            clientContactList[i] = clientContactList[i + 1];
        }
        clientContactCount--;
        printf("User deleted from the contact list.\n");
    } else {
        printf("User not found in the contact list.\n");
    }
}

void SendMessageToUser() {
    int userID;
    int i;
    printf("Enter UserID to send a message: ");
    scanf("%d", &userID);

    // Find the user in the contact list
    int userIndex = -1;
    for (i = 0; i < clientContactCount; i++) {
        if (clientContactList[i].userID == userID) {
            userIndex = i;
            break;
        }
    }

    // Send a message if the user is in the contact list
    if (userIndex != -1) {
        char message[1024];
        printf("Enter your message: ");
        fflush(stdin);  // Clear the buffer from previous input
        fgets(message, sizeof(message), stdin);  // Read a whole line
        message[strcspn(message, "\n")] = '\0';  // Remove trailing newline

        send(clientSocket, message, strlen(message), 0);
        printf("Message sent to UserID %d.\n", userID);
    } else {
        printf("User not found in the contact list.\n");
    }
}

void CheckMessages() {
    // Receive messages from the server
    char receivedMessage[1024];
    int bytesReceived = recv(clientSocket, receivedMessage, sizeof(receivedMessage) - 1, 0);

    if (bytesReceived > 0) {
        receivedMessage[bytesReceived] = '\0'; // Null-terminate the received message
        printf("Received Message: %s\n", receivedMessage);
    } else if (bytesReceived == 0) {
        printf("Connection closed by the server.\n");
    } else {
        printf("No new messages.\n");
    }
}

int main() {
    WSADATA wsaData;
    struct sockaddr_in serverAddr;
    char message[1024];
    int PORT = 12345;
    char *SERVER_IP = "127.0.0.1";

    // Winsock baslatma
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    // Soket olusturma
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    // Server adres bilgilerini ayarlama
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_port = htons(PORT);

    // Server'a baglanma
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Connection failed\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to server\n");
    struct User tempUserStruct;
    // Kullanici girisi
    printf("Enter your UserID: ");
    scanf("%d", &tempUserStruct.userID);

    printf("Enter Name: ");
    fflush(stdin); 
    fgets(tempUserStruct.name, sizeof(tempUserStruct.name), stdin);
    tempUserStruct.name[strcspn(tempUserStruct.name, "\n")] = '\0';  // Satır sonundaki yeni satır karakterini kaldır

    printf("Enter Surname: ");
    fflush(stdin);  
    fgets(tempUserStruct.surname, sizeof(tempUserStruct.surname), stdin);
    tempUserStruct.surname[strcspn(tempUserStruct.surname, "\n")] = '\0';  // Satır sonundaki yeni satır karakterini kaldır

    printf("Enter Phone Number: ");
    scanf("%d", &tempUserStruct.phoneNumber);

    send(clientSocket, (char *)&tempUserStruct, sizeof(tempUserStruct), 0);

    // Ana menu
    while (1) {
        // Menu
        printf("\nMenu:\n");
        printf("1. List Contacts\n");
        printf("2. Add User\n");
        printf("3. Delete User\n");
        printf("4. Send Message\n");
        printf("5. Check Messages\n");
        printf("6. Exit\n");

        // Kullanici girdisi
        int choice;
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                ListContacts();
                break;
            case 2:
                AddUser();
                break;
            case 3:
                DeleteUser();
                break;
            case 4:
                SendMessageToUser();
                break;
            case 5:
                CheckMessages();
                break;
            case 6:
                // Cleanup and exit
                closesocket(clientSocket);
                WSACleanup();
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}
