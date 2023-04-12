#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <arpa/inet.h>

#include <openssl/aes.h>
#include <openssl/evp.h>

#define PORT 8080
#define BUFFER_SIZE 16


void handle_error(const char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}


// Encrypt message using AES-256 in CBC mode with a random IV
void encrypt_message(const char* plaintext, const char* key, char* ciphertext) {
    // Generate a random IV
    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, AES_BLOCK_SIZE);

    // Set up the encryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (const unsigned char*)key, iv);

    // Encrypt the message using AES in CBC mode
    int ciphertext_len;
    EVP_EncryptUpdate(ctx, (unsigned char*)ciphertext, &ciphertext_len, (const unsigned char*)plaintext, strlen(plaintext));
    int final_len;
    EVP_EncryptFinal_ex(ctx, (unsigned char*)ciphertext + ciphertext_len, &final_len);

    // Append the IV to the ciphertext
    memcpy(ciphertext + ciphertext_len + final_len, iv, AES_BLOCK_SIZE);
    ciphertext[ciphertext_len + final_len + AES_BLOCK_SIZE] = '\0';

    // Clean up
    EVP_CIPHER_CTX_free(ctx);
}


// Decrypt message using AES-256 in CBC mode
void decrypt_message(const char* ciphertext, const char* key, char* plaintext) {
    // Get the IV from the end of the ciphertext
    unsigned char iv[AES_BLOCK_SIZE];
    memcpy(iv, ciphertext + strlen(ciphertext) - AES_BLOCK_SIZE, AES_BLOCK_SIZE);

    // Set up the decryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (const unsigned char*)key, iv);

    // Decrypt the message using AES in CBC mode
    int plaintext_len;
    EVP_DecryptUpdate(ctx, (unsigned char*)plaintext, &plaintext_len, (const unsigned char*)ciphertext, strlen(ciphertext) - AES_BLOCK_SIZE);
    int final_len;
    EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext + plaintext_len, &final_len);
    plaintext[plaintext_len + final_len] = '\0';

    // Clean up
    EVP_CIPHER_CTX_free(ctx);
}


int authenticate(int sockfd) {
    char username[64], password[64], buffer[1024];
    int n;

    printf("Enter your username: ");
    scanf("%s", username);
    printf("Enter your password: ");
    scanf("%s", password);

    // Send username and password to the server
    sprintf(buffer, "%s:%s", username, password);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        perror("Error sending authentication details");
        return EXIT_FAILURE;
    }

    // Wait for response from the server
    memset(buffer, 0, sizeof(buffer));
    n = read(sockfd, buffer, sizeof(buffer));
    if (n < 0) {
        perror("Error reading server response");
        return EXIT_FAILURE;
    }

    if (strcmp(buffer, "OK") == 0) {
        printf("Authentication successful.\n");
        return 0;
    } else {
        printf("Authentication failed.\n");
        return EXIT_FAILURE;
    }
}


int key_exchange(int sockfd) {
    char buffer[1024];
    int n;

    // Generate a random key
    srand(time(NULL));
    int key = rand() % 10000;

    // Send the key to the server
    sprintf(buffer, "%d", key);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        perror("Error sending encryption key");
        return EXIT_FAILURE;
    }

    // Wait for response from the server
    memset(buffer, 0, sizeof(buffer));
    n = read(sockfd, buffer, sizeof(buffer));
    if (n < 0) {
        perror("Error reading server response");
        return EXIT_FAILURE;
    }

    if (strcmp(buffer, "OK") == 0) {
        printf("Key exchange successful.\n");
        return key;
    } else {
        printf("Key exchange failed.\n");
        return EXIT_FAILURE;
    }
}


void encrypt_data(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;

    int ciphertext_len, len;

    if (!(ctx = EVP_CIPHER_CTX_new())) {
        perror("Error creating cipher context");
        exit(1);
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1) {
        perror("Error initializing encryption");
        exit(1);
    }

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
        perror("Error encrypting data");
        exit(1);
    }

    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        perror("Error finalizing encryption");
        exit(1);
    }

    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
}


int main() {
    int sockfd, newsockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    
    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(1);
    }
    
    // initialize server address structure
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // bind socket to server address
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }
    
    // listen for incoming connections
    listen(sockfd, 5);
    printf("Server started and listening on port %d\n", PORT);
    
    while (1) {
        // accept incoming connection
        client_len = sizeof(client_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (newsockfd < 0) {
            perror("Error accepting connection");
            exit(1);
        }
        
        printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));
        
        while (1) {
            // receive message from client
            bzero(buffer, BUFFER_SIZE);
            ssize_t bytes_received = recv(newsockfd, buffer, BUFFER_SIZE, 0);
            if (bytes_received < 0) {
                perror("Error receiving message");
                exit(1);
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
                exit(1);
            }
        }
    }
    
    return 0;
}
