#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define PORT 8080

void *handle_client(void *arg) {
    int socketfd = *(int *)arg;
    char buffer[1024] = {0};

    int valread = read(socketfd, buffer, 1024);
    if (valread < 0) {
        perror("read error");
        close(socketfd);
        pthread_exit(NULL);
    }


    printf("Received message: %s\n", buffer);
    char *response = "Test";
    
    long sendMsg = send(socketfd, response, strlen(response), 0);
    if (sendMsg < 0) {
        perror("send error");
    }

    close(socketfd);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd == 0) {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverfd, SOMAXCONN) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);


    while (1) {
        int new_socket = accept(serverfd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            if (errno == EINTR)
                continue;
            else {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }
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
