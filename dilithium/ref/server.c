#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define MAX_PACKET_SIZE 10000


void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in servaddr, cliaddr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Socket creation failed");

    memset(&servaddr, 0, sizeof(servaddr));

    // Configure server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        error("Socket bind failed");

    // Listen
    if (listen(sockfd, 5) < 0)
        error("Socket listen failed");

    printf("Server listening on port %d...\n", PORT);

    clilen = sizeof(cliaddr);

    // Accept connection from client
    if ((newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
        error("Socket accept failed");

    printf("Client connected\n");

    // Receive public key from client
    uint8_t pk[pqcrystals_dilithium5_ref_PUBLICKEYBYTES];
    if (recv(newsockfd, pk, sizeof(pk), 0) < 0) {
        perror("Receive failed");
        return 1;
    }

    printf("Received public key from client\n");

    // Receive message and signature from client
    size_t packet_len;
    if (recv(newsockfd, &packet_len, sizeof(size_t), 0) < 0) {
        perror("Receive failed");
        return 1;
    }

    uint8_t packet[packet_len];
    if (recv(newsockfd, packet, packet_len, 0) < 0) {
        perror("Receive failed");
        return 1;
    }

    // Verify signature
    if (pqcrystals_dilithium5_ref_verify(packet + strlen((char *)packet), pqcrystals_dilithium5_ref_BYTES, packet, strlen((char *)packet), pk) != 0) {
        fprintf(stderr, "Signature verification failed\n");
        return 1;
    }

    printf("Signature verified successfully\n");

    // Extract and print message
    printf("Received Message: %s\n", packet);

    // Close sockets
    close(newsockfd);
    close(sockfd);

    return 0;
}
