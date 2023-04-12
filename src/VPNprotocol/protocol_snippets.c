// This isnt going to be used, its just for tests
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "encrypt.h"

#define PORT 8080
#define SERVER_ADR "127.0.0.1"

int main(int argc, char *argv[]) {
    // Establish a connection between the client and server
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_ADR, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Authenticate the client
    char* username = "username";
    char* password = "password";
    send(sockfd, username, strlen(username), 0);
    send(sockfd, password, strlen(password), 0);

    // Exchange encryption keys
    // Diffie-Hellman key exchange algorithm can be implemented here

    // Encrypt all traffic
    // AES encryption algorithm can be implemented here

    // Implement data encapsulation and tunneling
    // Add additional headers to each packet to route traffic through the VPN tunnel

    // Implement VPN gateway
    // Forward traffic between local network and VPN network


    return EXIT_SUCCESS;
}
