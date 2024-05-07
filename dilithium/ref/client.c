#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAXLINE 1024

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[MAXLINE];

    // Initialize socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Socket creation failed");

    memset(&servaddr, 0, sizeof(servaddr));

    // Set server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change to the IP address of the target Raspberry Pi
    servaddr.sin_port = htons(PORT);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        error("Connection to server failed");

    // Generate key pair
    uint8_t pk[pqcrystals_dilithium5_ref_PUBLICKEYBYTES];
    uint8_t sk[pqcrystals_dilithium5_ref_SECRETKEYBYTES];
    pqcrystals_dilithium5_ref_keypair(pk, sk);

    // Example message
    const char *message = "Hello, Raspberry Pi!";

    // Sign message
    uint8_t signature[pqcrystals_dilithium5_ref_BYTES];
    size_t signature_len;
    pqcrystals_dilithium5_ref_signature(signature, &signature_len, (const uint8_t *)message, strlen(message), sk);

    // Construct packet to send (message + signature)
    size_t packet_len = strlen(message) + signature_len;
    uint8_t packet[packet_len];
    memcpy(packet, message, strlen(message));
    memcpy(packet + strlen(message), signature, signature_len);

    // Send packet
    send(sockfd, packet, packet_len, 0);
    printf("Message sent successfully.\n");

    // Close socket
    close(sockfd);

    return 0;
}
