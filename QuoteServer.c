#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT 1717
#define MAX_QUOTE_LENGTH 512

void send_quote_to_client(int client_socket, const char *quote) {
    char quote_buffer[MAX_QUOTE_LENGTH];
   
    sprintf(quote_buffer, "%s\r\n", quote);
    send(client_socket, quote_buffer, strlen(quote_buffer), 0);
}

int main(int argc, const char * argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length = sizeof(client_address);
    const char *quote_file_path;
    FILE *quote_file;
    char current_quote[MAX_QUOTE_LENGTH];
    int random_quote_index, total_quotes = 0;
    int i;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <quote_file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    quote_file_path = argv[1];
    quote_file = fopen(quote_file_path, "r");

    if (!quote_file) {
        perror("Could not open quote file");
        exit(EXIT_FAILURE);
    }

    while (fgets(current_quote, sizeof(current_quote), quote_file)) {
        total_quotes++;
    }
    rewind(quote_file);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(server_socket, 5);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        random_quote_index = rand() % total_quotes;
        for (i = 0; i <= random_quote_index; i++) {
            fgets(current_quote, sizeof(current_quote), quote_file);
        }

        send_quote_to_client(client_socket, current_quote);
        close(client_socket);
    }
    return 0;
}
