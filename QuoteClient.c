#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define PORT 1717
#define MAX_BUFFER_SIZE 512

int main(int argc, const char * argv[]) {
    const char *server_ip;
    int total_bytes_received = 0;
    int bytes_received;
    int client_socket;
    struct sockaddr_in server_address;
    struct hostent *server;
    char *quote_buffer = malloc(MAX_BUFFER_SIZE);
    
    if (!quote_buffer) {
        perror("Error allocating memory for quote buffer");
        exit(EXIT_FAILURE);
    }

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <server_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    server_ip = argv[1];
    server = gethostbyname(server_ip);

    if (server == NULL) {
        fprintf(stderr, "Error: No such host\n");
        exit(EXIT_FAILURE);
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset((char *) &server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    memcpy((char *)&server_address.sin_addr.s_addr, (char *)server->h_addr_list[0], server->h_length);
    server_address.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Connection error");
        exit(EXIT_FAILURE);
    }

    while ((bytes_received = recv(client_socket, quote_buffer + total_bytes_received, MAX_BUFFER_SIZE - total_bytes_received, 0)) > 0) {
        total_bytes_received += bytes_received;
        
        if (total_bytes_received >= MAX_BUFFER_SIZE) {
            quote_buffer = realloc(quote_buffer, total_bytes_received + MAX_BUFFER_SIZE);
        
            if (!quote_buffer) {
                perror("Error reallocating memory");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (bytes_received == 0) {
        quote_buffer[total_bytes_received] = '\0';
        printf("%s", quote_buffer);
    } 
    
    else if (bytes_received < 0) {
        perror("Error receiving data");
    }

    close(client_socket);
    free(quote_buffer);
    return 0;
}
