#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "api.h"

#define PORT 12345
#define MAX_BUFFER_SIZE 1024

void handle_message(const uint8_t *message, size_t message_len) {
    // Verify or sign the message using SPHINCS+ functions
    // Example:
    unsigned char pk[CRYPTO_PUBLICKEYBYTES];
    unsigned char sk[CRYPTO_SECRETKEYBYTES];
    unsigned char sig[CRYPTO_BYTES];

    // Load public and secret keys (you might want to load these elsewhere)
    crypto_sign_seed_keypair(pk, sk, NULL); // Seed is NULL for simplicity, you may use a real seed

    // Verify signature
    if (crypto_sign_verify(message, CRYPTO_BYTES, message + CRYPTO_BYTES, message_len - CRYPTO_BYTES, pk) == 0) {
        printf("Message verified successfully.\n");
    } else {
        printf("Message verification failed.\n");
    }

    // Sign message
    size_t signed_message_len;
    crypto_sign(sig, &signed_message_len, message, message_len, sk);

    // Send signed message
    // send(socket, sig, signed_message_len, 0);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[MAX_BUFFER_SIZE];

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Accept incoming connections
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Receive message from client
        ssize_t bytes_received = recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("Receive failed");
            exit(EXIT_FAILURE);
        }

        printf("Received message from client: %s\n", buffer);

        // Handle the received message
        handle_message((uint8_t *)buffer, bytes_received);

        // Close client socket
        close(client_socket);
    }

    // Close server socket
    close(server_socket);

    return 0;
}
