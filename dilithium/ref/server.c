#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "api.h"

#define SERVER_PORT 8080
#define CRYPTO_BYTES 2420

int main() {
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Server address
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        return 1;
    }

    // Listen for connections
    if (listen(sockfd, 5) < 0) {
        perror("Listening failed");
        return 1;
    }

    printf("Server listening on port %d\n", SERVER_PORT);

    // Accept connections
    socklen_t client_len = sizeof(client_addr);
    int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (client_sockfd < 0) {
        perror("Accepting connection failed");
        return 1;
    }

    printf("Client connected\n");

    // Receive public key from client
    uint8_t pk[pqcrystals_dilithium5_ref_PUBLICKEYBYTES];
    if (recv(client_sockfd, pk, pqcrystals_dilithium5_ref_PUBLICKEYBYTES, 0) < 0) {
        perror("Receiving public key failed");
        return 1;
    } else {
        printf("Received public key\n");
    }

    size_t siglen;
    // Receive signature length from client
    if (recv(client_sockfd, &siglen, 4627, 0) < 0) {
        perror("Receiving signature length failed");
        return 1;
    } else {
        printf("Received signature length\n");
    }

    uint8_t sig[pqcrystals_dilithium5_ref_BYTES];
    // Receive signature from client
    if (recv(client_sockfd, sig, siglen, 0) < 0) {
        perror("Receiving signature failed");
        return 1;
    }
    printf("Received signature: ");

    // Receive message length from client
    size_t mlen;
    size_t mlen_net;
    if (recv(client_sockfd, &mlen_net, sizeof(mlen_net), 0) < 0) {
        perror("Receiving message length failed");
        return 1;
    }
    mlen = ntohl(mlen_net); // Convert from network byte order to host byte order

    // Receive message from client
    char *message = malloc(mlen + 1);
    if (message == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    if (recv(client_sockfd, message, mlen, 0) < 0) {
        perror("Receiving message failed");
        free(message);
        return 1;
    }
    message[mlen] = '\0'; // Null-terminate the received string

    // Verify signed message
    if(pqcrystals_dilithium5_ref_verify(sig, siglen, (const uint8_t *)message, mlen, pk) != 0) {
        printf("Signature verification failed\n");
    } else {
        printf("Signature verified\n");
    }

    free(message); // Don't forget to free the allocated memory

    close(client_sockfd);
    close(sockfd);
    return 0;
}