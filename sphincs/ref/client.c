// File: client.c
#include "api.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    int sock;
    struct sockaddr_in server;
    unsigned char message[1024];
    unsigned char signed_message[1200]; // Buffer size may need adjustment based on signature size
    unsigned long long signed_message_len;

    unsigned char public_key[CRYPTO_PUBLICKEYBYTES];
    unsigned char secret_key[CRYPTO_SECRETKEYBYTES];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("192.168.1.2"); // IP address of the server
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    // Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    // Generate key pair
    if (crypto_sign_keypair(public_key, secret_key) != 0) {
        fprintf(stderr, "Failed to generate key pair\n");
        return 1;
    }

    // Prepare message
    strcpy((char *)message, "Hello from Raspberry Pi!");

    // Sign the message
    if (crypto_sign(signed_message, &signed_message_len, message, strlen((char *)message), secret_key) != 0) {
        fprintf(stderr, "Failed to sign message\n");
        return 1;
    }

    // Send the message
    if (send(sock, signed_message, signed_message_len, 0) < 0) {
        puts("Send failed");
        return 1;
    }

    puts("Data Sent\n");
    close(sock);
    return 0;
}
