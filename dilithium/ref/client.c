#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "api.h"

#define PORT 8080
#define SERVER_ADDR "127.0.0.1"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_ADDR, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    // Generate keypair
    uint8_t pk[pqcrystals_dilithium3_PUBLICKEYBYTES];
    uint8_t sk[pqcrystals_dilithium3_SECRETKEYBYTES];
    pqcrystals_dilithium3_ref_keypair(pk, sk);

    // Send public key to the server
    send(sock, pk, pqcrystals_dilithium3_PUBLICKEYBYTES, 0);
    printf("Public key sent to server\n");

    // Sign a message
    uint8_t message[] = "Hello, Server!";
    size_t message_len = strlen((char*)message);
    uint8_t signature[pqcrystals_dilithium3_BYTES];
    size_t signature_len;
    pqcrystals_dilithium3_ref_signature(signature, &signature_len, message, message_len, sk);

    // Send message and signature to the server
    send(sock, message, message_len, 0);
    send(sock, signature, signature_len, 0);

    printf("Message and signature sent to server\n");

    close(sock);
    return 0;
}
