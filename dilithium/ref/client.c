#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Socket creation failed");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        error("Connection to server failed");

    uint8_t pk[pqcrystals_dilithium5_ref_PUBLICKEYBYTES];
    uint8_t sk[pqcrystals_dilithium5_ref_SECRETKEYBYTES];
    if (pqcrystals_dilithium5_ref_keypair(pk, sk) != 0) {
        fprintf(stderr, "Failed to generate key pair\n");
        return 1;
    }

    if (send(sockfd, pk, sizeof(pk), 0) < 0) {
        perror("Send public key failed");
        return 1;
    }
    printf("Public key sent.\n");

    const char *message = "Hello, Raspberry Pi!";
    size_t message_len = strlen(message);

    uint8_t signature[pqcrystals_dilithium5_ref_BYTES];
    size_t signature_len;
    if (pqcrystals_dilithium5_ref_signature(signature, &signature_len, (const uint8_t *)message, message_len, sk) != 0) {
        fprintf(stderr, "Failed to sign message\n");
        return 1;
    }

    size_t packet_len = message_len + signature_len;
    uint8_t *packet = malloc(packet_len);
    if (packet == NULL) {
        fprintf(stderr, "Failed to allocate memory for packet\n");
        return 1;
    }
    memcpy(packet, message, message_len);
    memcpy(packet + message_len, signature, signature_len);

    printf("Sending message and signature\n");
    uint32_t net_packet_len = htonl(packet_len);
    if (send(sockfd, &net_packet_len, sizeof(net_packet_len), 0) < 0) {
        perror("Send packet length failed");
        free(packet);
        return 1;
    }

    if (send(sockfd, packet, packet_len, 0) < 0) {
        perror("Send packet failed");
        free(packet);
        return 1;
    }

    printf("Message sent successfully.\n");

    free(packet);
    close(sockfd);

    return 0;
}
