#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "api.h"

#define SERVER_PORT 8080
#define CRYPTO_BYTES 2420

int main() {
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(SERVER_PORT); 

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        return 1;
    }

    // Generate key pair
    uint8_t pk[pqcrystals_dilithium5_ref_PUBLICKEYBYTES];
    uint8_t sk[pqcrystals_dilithium5_ref_SECRETKEYBYTES];
    if (pqcrystals_dilithium5_ref_keypair(pk, sk) != 0) {
        fprintf(stderr, "Key pair generation failed\n");
        return 1;
    }

    // Send public key to server
    if (send(sockfd, pk, pqcrystals_dilithium5_ref_PUBLICKEYBYTES, 0) < 0) {
        perror("Sending public key failed");
        return 1;
    }

    // Message
    const char *message = "Hello, server!";
    size_t mlen = strlen(message);

    // Generate Signature
    uint8_t sig[pqcrystals_dilithium5_ref_BYTES];
    size_t siglen;
    if (pqcrystals_dilithium5_ref_signature(sig, &siglen, (const uint8_t *)message, mlen, sk) != 0) {
        fprintf(stderr, "Signing failed\n");
        return 1;
    }

    // Verify signature
    if(pqcrystals_dilithium5_ref_verify(sig, siglen, (const uint8_t *)message, mlen, pk) != 0) {
        printf("Signature verification failed\n");
    } else {
        printf("Signature verified\n");
    }

    printf("Signature length is %d\n", siglen);

    // Send signature length to server
    ssize_t send_result = send(sockfd, &siglen, sizeof(siglen), 0);
    if (send_result < 0) {
        perror("Sending signature length failed");
        printf("siglen: %lu\n", siglen);
        return 1;
    }
    printf("Signature length sent, send returned: %zd\n", send_result);
    
    // Send signature to server
    if (send(sockfd, sig, pqcrystals_dilithium5_ref_BYTES, 0) < 0) {
        perror("Sending signature failed");
        return 1;
    }

    printf("Signature sent\n");

    // Send message length to server
    size_t mlen_net = htonl(mlen); // Convert to network byte order
    if (send(sockfd, &mlen_net, sizeof(mlen_net), 0) < 0) {
        perror("Sending message length failed");
        return 1;
    }

    printf("Message length sent\n");

    // Send message to server
    if (send(sockfd, message, mlen, 0) < 0) {
        perror("Sending message failed");
        return 1;
    }

    printf("Message sent\n");

    close(sockfd);
    return 0;
}