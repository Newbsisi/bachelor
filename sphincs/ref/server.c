#include "api.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

unsigned char public_key[CRYPTO_PUBLICKEYBYTES];

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server, client;
    socklen_t c;

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

        while ((current_read_size = recv(client_sock, client_message + total_read_size, sizeof(client_message) - total_read_size, 0)) > 0) {
            total_read_size += current_read_size;
            // Optionally add a check here to see if you've received a full message based on your protocol
        }

        if (total_read_size > 0) {
            printf("Data received: %d bytes\n", total_read_size);

            // Verify the signature and extract the message
            if (crypto_sign_open(client_message, &message_len, client_message, total_read_size, public_key) != 0) {
                printf("Signature verification failed\n");
            } else {
                printf("Received and verified message: %.*s\n", (int)message_len, client_message);
            }
        }

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
