#include "api.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 1234
#define BUFFER_SIZE 4096
#define MESSAGEBYTES 14

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addr_size;
    char buffer[BUFFER_SIZE];
    uint8_t publicKey[pqcrystals_dilithium5_ref_PUBLICKEYBYTES];
    uint8_t secretKey[pqcrystals_dilithium5_ref_SECRETKEYBYTES];
    uint8_t signedMessage[MESSAGEBYTES + pqcrystals_dilithium5_ref_BYTES];
    size_t signedMessageLen;

    // Generate keys
    pqcrystals_dilithium5_ref_keypair(publicKey, secretKey);

    // Create socket
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Bind socket
    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    // Listen for connections
    listen(serverSocket, 1);

    // Accept a connection
    addr_size = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addr_size);

    // Send public key to client
    send(clientSocket, publicKey, pqcrystals_dilithium5_ref_PUBLICKEYBYTES, 0);
    printf("Public key sent to client\n");

    // Receive signed message from client
    recv(clientSocket, buffer, sizeof(buffer), 0);
    memcpy(signedMessage, buffer, sizeof(signedMessage));
    printf("Signed message received from client\n");
    printf("Size of signed message: %lu\n", sizeof(signedMessage));

    // Verify signed message
    if (pqcrystals_dilithium5_ref_open((uint8_t*)buffer, &signedMessageLen, signedMessage, sizeof(signedMessage), publicKey) != 0) {
        printf("Signature verification failed\n");
    } else {
        printf("Received message: %s\n", buffer);
    }

    return 0;
}