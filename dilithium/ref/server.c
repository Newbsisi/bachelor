#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // For network communication
#include "api.h" // Include your API header file

#define MAX_MESSAGE_SIZE 1024
#define SERVER_PORT 12345 // Port number the server will listen on

// Function to verify the signature of the received message
int verify_signature(const uint8_t *signed_message, size_t signed_message_len, const uint8_t *public_key) {
    size_t message_len = signed_message_len - pqcrystals_dilithium5_ref_BYTES;
    const uint8_t *message = signed_message + pqcrystals_dilithium5_ref_BYTES;

    // Verify the signature
    int verification_result = pqcrystals_dilithium5_ref_verify(signed_message, signed_message_len, message, message_len, public_key);
    return verification_result;
}

int main() {
    int sockfd, client_sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    ssize_t bytes_received;
    uint8_t received_message[MAX_MESSAGE_SIZE];

    // Initialize public key
    uint8_t public_key[pqcrystals_dilithium5_ref_PUBLICKEYBYTES];
    // You need to initialize it somehow, possibly by reading it from a file or generating it

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address, bind socket to port, listen for incoming connections, etc., as before...

    printf("Server listening on port %d...\n", SERVER_PORT);

    while (1) {
        // Accept incoming connection
        socklen_t client_addr_len = sizeof(client_addr);

        client_addr_len = sizeof(client_addr);
        if ((client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
            perror("accept");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

        // Receive message from client
        bytes_received = recv(client_sockfd, received_message, MAX_MESSAGE_SIZE, 0);
        if (bytes_received == -1) {
            perror("recv");
            close(client_sockfd);
            continue;
        }

        // Verify the signature of the received message
        int verification_result = verify_signature(received_message, bytes_received, public_key);
        if (verification_result != 0) {
            printf("Signature verification failed.\n");
            close(client_sockfd);
            continue;
        }

        // Signature verified, extract the message and process it
        size_t message_len = bytes_received - pqcrystals_dilithium5_ref_BYTES;
        printf("Received message from client: %.*s\n", (int)message_len, (char *)(received_message + pqcrystals_dilithium5_ref_BYTES));

        // Process the message as needed...

        // Close the client socket
        close(client_sockfd);
    }

    // Close the server socket (not reached in this simple example)
    close(sockfd);

    return 0;
}
