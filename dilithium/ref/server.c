#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "api.h"

#define SERVER_PORT 8080

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
    }

    // Receive signed message from client
    uint8_t signed_message[pqcrystals_dilithium5_ref_BYTES];
    if (recv(client_sockfd, signed_message, sizeof(signed_message), 0) < 0) {
        perror("Receiving signed message failed");
        return 1;
    }

    // Extracting the signature and the message from the received signed message
    uint8_t *signature = signed_message;
    uint8_t *message = signed_message + pqcrystals_dilithium5_ref_BYTES;
    size_t mlen = pqcrystals_dilithium5_ref_BYTES;

    printf("Received signature: ");

    // Verify signature
    if (pqcrystals_dilithium5_ref_verify(signature, pqcrystals_dilithium5_ref_BYTES, message, mlen, pk) == 0) {
        printf("Signature verified. Message: %s\n", message);
    } else {
        printf("Signature verification failed\n");
    }

    close(client_sockfd);
    close(sockfd);
    return 0;
}
