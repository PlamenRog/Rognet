#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/evp.h>

#define PORT 8080
#define BLOCK_SIZE 16

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
  int sockfd, n;
  struct sockaddr_in serv_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("Error opening socket");
    return -1;
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("Error connecting to server");
    return -1;
  }

  // Perform authentication
  if (authenticate(sockfd) != 0) {
    close(sockfd);
    return -1;
  }

  // Perform key exchange
  int key = key_exchange(sockfd);
  if (key < 0) {
    close(sockfd);
    return -1;
  }

  // Read data from user
  char plaintext[1024];
  printf("Enter plaintext: ");
  scanf("%s", plaintext);

  // Encrypt data
  unsigned char iv[BLOCK_SIZE], ciphertext[1024];
  memset(iv, 0, BLOCK_SIZE);
  encrypt_data((unsigned char *)plaintext, strlen(plaintext), (unsigned char *)&key, iv, ciphertext);

  // Send encrypted data to server
  n = write(sockfd, ciphertext, strlen((char *)ciphertext));
  if (n < 0) {
    perror("Error sending encrypted data");
    close(sockfd);
    return -1;
  }

  printf("Data sent successfully.\n");

  close(sockfd);

  return 0;
}

