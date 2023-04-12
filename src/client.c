#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define SERVER_ADDR "127.0.0.1" // Server IP address

int main() {
    char message[1024];
    int sock;
    struct sockaddr_in server;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Initialize server information
    memset(&server, '0', sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Send message to server
    strcpy(message, "Hello from client");
    send(sock, message, strlen(message), 0);

    // Receive response from server
    memset(message, 0, sizeof(message));
    recv(sock, message, 1024, 0);

    printf("Server message: %s\n", message);

    // Close socket
    close(sock);

    return 0;
}
