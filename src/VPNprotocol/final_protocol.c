// TODO: add NAT traversal
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>
#include <arpa/inet.h>

#include "encrypt.h"

#define PORT 8080
#define BUFFER_SIZE 16


void handle_error(const char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}


int main() {
    struct sockaddr_in server_addr, client_addr;
    
    // create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    
    // initialize server address structure
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // bind socket to server address
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
    
    // listen for incoming connections
    listen(sockfd, 5);
    printf("Server started and listening on port %d\n", PORT);
    
    int newsockfd;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    while (1) {
        // accept incoming connection
        client_len = sizeof(client_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (newsockfd < 0) {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }
        
        printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));
        
        while (1) {
            // receive message from client
            bzero(buffer, BUFFER_SIZE);
            ssize_t bytes_received = recv(newsockfd, buffer, BUFFER_SIZE, 0);
            if (bytes_received < 0) {
                perror("Error receiving message");
                exit(EXIT_FAILURE);
            } else if (bytes_received == 0) {
                printf("Client disconnected\n");
                close(newsockfd);
                break;
            }
            
            printf("Received message from client (%s): %s", inet_ntoa(client_addr.sin_addr), buffer);
            
            // send message back to client
            ssize_t bytes_sent = send(newsockfd, buffer, strlen(buffer), 0);
            if (bytes_sent < 0) {
                perror("Error sending message");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    return 0;
}
