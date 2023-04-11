#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080

void *handle_client(void *arg) {
    int socket_fd = *(int *)arg;
    char buffer[1024] = {0};

    int valread = read(socket_fd, buffer, 1024);
    
    printf("Received message: %s\n", buffer);
    char *response = "Test";
    send(socket_fd, response, strlen(response), 0);
    
    close(socket_fd);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("New client connected: %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));


        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)&new_socket) < 0) {
            perror("Thread creation error");
            exit(EXIT_FAILURE);
        }

        pthread_detach(thread_id);
    }

    return EXIT_SUCCESS;
}
