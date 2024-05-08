#define _POSIX_C_SOURCE 200809L
#include "api.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h> // Include for high-precision timing


unsigned char public_key[CRYPTO_PUBLICKEYBYTES];

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server, client;
    socklen_t c;

    struct timespec start, end; // Use timespec structure for nanosecond precision
    double cpu_time_used;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        fprintf(stderr, "Could not create socket\n");
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

    while (1) {
        c = sizeof(struct sockaddr_in);
        client_sock = accept(server_sock, (struct sockaddr *)&client, &c);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        if (recv(client_sock, public_key, CRYPTO_PUBLICKEYBYTES, MSG_WAITALL) != CRYPTO_PUBLICKEYBYTES) {
            perror("Failed to receive full public key");
            close(client_sock);
            continue;
        }
        printf("Public key received\n");

        unsigned char client_message[20000];
        unsigned long long message_len;
        int total_read_size = 0, current_read_size;

        clock_gettime(CLOCK_MONOTONIC, &start); // Start high-precision timer
        while ((current_read_size = recv(client_sock, client_message + total_read_size, sizeof(client_message) - total_read_size, 0)) > 0) {
            total_read_size += current_read_size;
        }

        if (total_read_size > 0) {
            printf("Data received: %d bytes\n", total_read_size);
            if (crypto_sign_open(client_message, &message_len, client_message, total_read_size, public_key) != 0) {
                printf("Signature verification failed\n");
            } else {
                printf("Received and verified message: %.*s\n", (int)message_len, client_message);
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &end); // End high-precision timer

        cpu_time_used = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0; // Calculate in seconds
        printf("Processing time: %.12f seconds\n", cpu_time_used); // Print with nanosecond precision

        if (current_read_size == 0) {
            puts("Client disconnected");
        } else if (current_read_size == -1) {
            perror("Receive failed");
        }

        close(client_sock);
    }

    close(server_sock);
    return 0;
}
