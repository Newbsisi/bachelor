#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "api.h" // Include the API header file

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                   &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Receive public key from client
    uint8_t public_key[pqcrystals_dilithium3_PUBLICKEYBYTES];
    read(new_socket, public_key, pqcrystals_dilithium3_PUBLICKEYBYTES);
    printf("Received public key from client\n");

    // Receive message from client
    char buffer[3293] = {0}; // Assuming maximum message length is 1024
    read(new_socket , buffer, 3293);
    printf("Received message from client: %s\n", buffer);
    printf("Message length: %ld\n", strlen(buffer));

    // Receive signature from client
    uint8_t signature[pqcrystals_dilithium3_BYTES];
    read(new_socket , signature, pqcrystals_dilithium3_BYTES);
    printf("Received signature from client\n");

    // Open the signed message
    uint8_t opened_message[1024]; // Assuming message length is not greater than 1024
    size_t opened_message_len = 1024; // Initialize the length to the maximum size
    int opening_result = pqcrystals_dilithium3_ref_open(opened_message, &opened_message_len,
                                                        buffer, strlen(buffer),
                                                        public_key);
    if (opening_result == 0) {
        printf("Message opening successful! Opened message: %s\n", opened_message);
    } else {
        printf("Message opening failed!\n");
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
