#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define MAX_PACKET_SIZE 20000000

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Socket creation failed");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        error("Socket bind failed");

    if (listen(sockfd, 5) < 0)
        error("Socket listen failed");

    printf("Server listening on port %d...\n", PORT);
    clilen = sizeof(cliaddr);

    if ((newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
        error("Socket accept failed");
    printf("Client connected\n");

    uint8_t pk[pqcrystals_dilithium5_ref_PUBLICKEYBYTES];
    if (recv(newsockfd, pk, sizeof(pk), 0) < 0) {
        perror("Receive public key failed");
        return 1;
    }
    printf("Public key received.\n");

    uint32_t packet_len;
    if (recv(newsockfd, &packet_len, sizeof(packet_len), 0) < 0) {
        perror("Receive packet length failed");
        return 1;
    }
    packet_len = ntohl(packet_len);
    printf("Packet length received: %u\n", packet_len);

    if (packet_len == 0 || packet_len > MAX_PACKET_SIZE) {
        fprintf(stderr, "Invalid packet length: %u\n", packet_len);
        return 1;
    }

    uint8_t *packet = malloc(packet_len);
    if (packet == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    if (recv(newsockfd, packet, packet_len, 0) < 0) {
        perror("Failed to receive packet");
        free(packet);
        return 1;
    }

    size_t mlen = packet_len - pqcrystals_dilithium5_ref_BYTES;
    size_t siglen = pqcrystals_dilithium5_ref_BYTES;
    const uint8_t *sig = packet + mlen;

    printf("Attempting to verify signature...\n");
    if (pqcrystals_dilithium5_ref_verify(sig, siglen, packet, mlen, pk) != 0) {
        fprintf(stderr, "Signature verification failed\n");
        free(packet);
        return 1;
    }

    printf("Signature verified successfully\n");
    printf("Received message: %.*s\n", (int)mlen, packet);

    close(newsockfd);
    close(sockfd);
    free(packet);
    return 0;
}
