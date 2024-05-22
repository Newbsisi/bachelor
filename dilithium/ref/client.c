#include "api.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 1234
#define BUFFER_SIZE 4595
#define MESSAGEBYTES 14
#define CRYPTO_BYTES 2420

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    uint8_t publicKey[pqcrystals_dilithium5_ref_PUBLICKEYBYTES];
    uint8_t secretKey[pqcrystals_dilithium5_ref_SECRETKEYBYTES];
    uint8_t signedMessage[MESSAGEBYTES + CRYPTO_BYTES];
    size_t signedMessageLen;

    // Generate keys
    pqcrystals_dilithium5_ref_keypair(publicKey, secretKey);

    // Create socket
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to server
    connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    // Send public key to server
    send(clientSocket, publicKey, pqcrystals_dilithium5_ref_PUBLICKEYBYTES, 0);
    printf("Public key sent to server\n");

    const char* message = "Hello, server!";
    size_t message_len = strlen(message);
    // Sign a message
    pqcrystals_dilithium5_ref(signedMessage, &signedMessageLen, (uint8_t*)message, message_len, secretKey);

    if(pqcrystals_dilithium5_ref_verify((uint8_t*)buffer, &signedMessageLen, signedMessage, sizeof(signedMessage), publicKey) != 0) {
        printf("Signature verification failed\n");
    } else {
        printf("Received message: %s\n", buffer);
    }
    
    // Send signed message to server
    send(clientSocket, signedMessage, signedMessageLen, 0);

    return 0;
}