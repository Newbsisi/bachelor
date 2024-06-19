#include "api.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    int sock;
    struct sockaddr_in server;
    unsigned char message[20000];
    unsigned char signed_message[20000 + CRYPTO_PUBLICKEYBYTES];
    unsigned long long signed_message_len;

    unsigned char public_key[CRYPTO_PUBLICKEYBYTES];
    unsigned char secret_key[CRYPTO_SECRETKEYBYTES];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("192.168.137.55"); // IP address of the server
    //server.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // localhost
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);

    // Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    // Generate key pair
    if (crypto_sign_keypair(public_key, secret_key) != 0) {
        fprintf(stderr, "Failed to generate key pair\n");
        return 1;
    }

    // Send the public key first to the server
    if (send(sock, public_key, CRYPTO_PUBLICKEYBYTES, 0) < 0) {
        perror("Failed to send public key");
        return 1;
    }
    puts("Public key sent to server");

    // Prepare message
    strcpy((char *)message, "Contrary to popular belief, Lorem Ipsum is not simply random text. It has roots in a piece of classical Latin literature from 45 BC, making it over 2000 years old. Richard McClintock, a Latin professor at Hampden-Sydney College in Virginia, looked up one of the more obscure Latin words, consectetur, from a Lorem Ipsum passage, and going through the cites of the word in classical literature, discovered the undoubtable source. Lorem Ipsum comes from sections 1.10.32 and 1.10.33 of de Finibus Bonorum et Malorum (The Extremes of Good and Evil) by Cicero, written in 45 BC. This book is a treatise on the theory of ethics, very popular during the Renaissance. The first line of Lorem Ipsum, Lorem ipsum dolor sit amet. comes from a line in section 1.10.32.");

    // Sign the message
    if (crypto_sign(signed_message, &signed_message_len, message, strlen((char *)message), secret_key) != 0) {
        fprintf(stderr, "Failed to sign message\n");
        return 1;
    }
    printf("Size of signed message: %lu\n", signed_message_len);

    // Send the signed message
    if (send(sock, signed_message, signed_message_len, 0) < 0) {
        perror("Send failed");
        return 1;
    }

    //Display signature length
    int signature = crypto_sign_bytes();
    printf("Signature length %d\n", signature);

    //Display public key size
    int publicKeySize = crypto_sign_publickeybytes();
    printf("Public key size %d\n", publicKeySize);

    puts("Signed data sent\n");
    close(sock);
    return 0;
}
