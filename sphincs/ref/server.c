#include "api.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

unsigned char public_key[CRYPTO_PUBLICKEYBYTES]; // Initialized properly elsewhere

int load_public_key() {
    // Implementation assumed, ensure this reads or sets the public key correctly
    memset(public_key, 0xAB, CRYPTO_PUBLICKEYBYTES);
    return 0;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server, client;
    socklen_t c;

    if (load_public_key() != 0) {
        fprintf(stderr, "Failed to load public key\n");
        return 1;
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(server_sock, 3);
    printf("Server listening for incoming connections...\n");
    c = sizeof(struct sockaddr_in);

    client_sock = accept(server_sock, (struct sockaddr *)&client, &c);
    if (client_sock < 0) {
        perror("Accept failed");
        return 1;
    }
    printf("Connection accepted\n");

    unsigned char client_message[30000];
    unsigned long long message_len;
    int read_size;

    while ((read_size = recv(client_sock, client_message, sizeof(client_message), 0)) > 0) {
        printf("Data received: %d bytes\n", read_size);
        if (crypto_sign_open(client_message, &message_len, client_message, read_size, public_key) == 0) {
            printf("Signature verification failed\n");
        } else {
            printf("Received and verified message: %.*s\n", (int)message_len, client_message);
        }
    }

    if (read_size == 0) {
        puts("Client disconnected");
    } else if (read_size == -1) {
        perror("Recv failed");
    }

    close(client_sock);
    close(server_sock);
    return 0;
}
