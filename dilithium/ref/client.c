#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "api.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main() {
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return 1;
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    uint8_t pk[pqcrystals_dilithium5_ref_PUBLICKEYBYTES];
    uint8_t sk[pqcrystals_dilithium5_ref_SECRETKEYBYTES];
    if (pqcrystals_dilithium5_ref_keypair(pk, sk) != 0) {
        fprintf(stderr, "Key pair generation failed\n");
        return 1;
    }

    // Example message to sign
    const char *message = "Hello, server!";
    size_t mlen = strlen(message);

    // Sign the message
    uint8_t sig[pqcrystals_dilithium5_ref_BYTES];
    size_t siglen;
    if (pqcrystals_dilithium5_ref_signature(sig, &siglen, (const uint8_t *)message, mlen, sk) != 0) {
        fprintf(stderr, "Signing failed\n");
        return 1;
    }

    // Send public key to server
    if (send(sockfd, pk, pqcrystals_dilithium5_ref_PUBLICKEYBYTES, 0) < 0) {
        perror("Sending public key failed");
        return 1;
    }

    // Send signed message to server
    if (send(sockfd, sig, siglen, 0) < 0) {
        perror("Sending signed message failed");
        return 1;
    }

    close(sockfd);
    return 0;
}
