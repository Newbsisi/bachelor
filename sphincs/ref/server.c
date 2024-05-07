// File: server.c
#include "api.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

unsigned char public_key[CRYPTO_PUBLICKEYBYTES]; // This should be initialized correctly

int load_public_key();  // Assuming this function is implemented to load the key

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server, client;
    socklen_t c;

    if (load_public_key() != 0) {
        fprintf(stderr, "Failed to load public key\n");
        return 1;
    }

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    // Bind
    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind failed. Error");
        return 1;
    }

    // Listen
    listen(server_sock, 3);

    // Accept an incoming connection
    printf("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);

    // Accept connection from an incoming client
    client_sock = accept(server_sock, (struct sockaddr *)&client, &c);
    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }
    printf("Connection accepted\n");

    unsigned char client_message[2000];
    unsigned long long message_len;
    int read_size;

    // Receive a message from client
    while ((read_size = recv(client_sock, client_message, 2000, 0)) > 0) {
        // Verify the signature
        if (crypto_sign_open(client_message, &message_len, client_message, read_size, public_key) != 0) {
            printf("Signature verification failed\n");
        } else {
            printf("Received and verified: %s\n", client_message + CRYPTO_BYTES); // Adjust offset for signature
        }
    }

    if (read_size == 0) {
        puts("Client disconnected");
    } else if (read_size == -1) {
        perror("recv failed");
    }

    close(server_sock);
    close(client_sock);
    return 0;
}

int load_public_key() {
    // Placeholder: Load the key from a file or config, or set here directly
    // Example hardcoded key (This is NOT secure or practical for real use):
    memset(public_key, 0xAB, CRYPTO_PUBLICKEYBYTES);
    return 0;
}
