#ifndef ENCRYPT_HEADER_FILE
#define ENCRYPT_HEADER_FILE


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <openssl/evp.h>


int generate_key(unsigned char key[]) {
    // Define the message to encrypt
    unsigned char message[] = "Hello, world!";
    int message_len = strlen((const char*)message);

    // Define the key and IV
    unsigned char iv[] = "myinitialvector1";

    // Allocate memory for the encrypted message
    void *encrypted_message = malloc(message_len + EVP_MAX_BLOCK_LENGTH);
    memset(encrypted_message, 0, message_len + EVP_MAX_BLOCK_LENGTH);

    // Create and initialize the encryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv);

    // Encrypt the message
    int encrypted_len = 0;
    EVP_EncryptUpdate(ctx, (unsigned char *)encrypted_message, &encrypted_len, message, message_len);
    int final_len = 0;
    EVP_EncryptFinal_ex(ctx, (unsigned char *)encrypted_message + encrypted_len, &final_len);

    // Print the encrypted message in hexadecimal format
    unsigned char * msgToRet = (unsigned char *)encrypted_message;
    printf("Encrypted message: ");
    for (int i = 0; i < encrypted_len + final_len; i++) {
        printf("%02x", msgToRet[i]);
    }
    printf("\n");

    // Free the memory allocated for the encrypted message and the encryption context
    free(encrypted_message);
    EVP_CIPHER_CTX_free(ctx);
}


// Function to compute the modular exponentiation (base^exp) mod mod
unsigned long long modexp(unsigned long long base, unsigned long long exp, unsigned long long mod)
{
    unsigned long long result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}


// Function to compute the shared secret key
unsigned long long diffie_hellman(unsigned long long p, unsigned long long g, unsigned long long a, unsigned long long b)
{
    unsigned long long A = modexp(g, a, p);
    unsigned long long B = modexp(g, b, p);
    unsigned long long secret = modexp(B, a, p);
    return secret;
}

#endif
