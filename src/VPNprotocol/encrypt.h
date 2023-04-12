#ifndef ENCRYPT_HEADER_FILE
#define ENCRYPT_HEADER_FILE

#include <sys/unistd.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

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

    printf("Enter your username: ");
    scanf("%s", username);
    printf("Enter your password: ");
    scanf("%s", password);

    // Send username and password to the server
    sprintf(buffer, "%s:%s", username, password);
    int n = write(sockfd, buffer, strlen(buffer));
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



void encrypt_data(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;

    int ciphertext_len, len;

    if (!(ctx = EVP_CIPHER_CTX_new())) {
        perror("Error creating cipher context");
        exit(EXIT_FAILURE);
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1) {
        perror("Error initializing encryption");
        exit(EXIT_FAILURE);
    }

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
        perror("Error encrypting data");
        exit(EXIT_FAILURE);
    }

    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        perror("Error finalizing encryption");
        exit(EXIT_FAILURE);
    }

    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
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


#endif
