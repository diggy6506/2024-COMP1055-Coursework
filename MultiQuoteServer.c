#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_SIZE 512
#define PORT 1818
#define MAX_QUOTES 100
#define MAX_QUOTE_LENGTH 512

int load_quotes(const char *filename, char quotes[MAX_QUOTES][MAX_QUOTE_LENGTH], int *num_quotes) {
    FILE *file;
    char line[MAX_QUOTE_LENGTH];
    *num_quotes = 0;
    
    file = fopen(filename, "r");
    if (!file) {
        return -1;
    }
    
    while (fgets(line, sizeof(line), file) && *num_quotes < MAX_QUOTES) {
        strcpy(quotes[*num_quotes], line);
        (*num_quotes)++;
    }
    
    fclose(file);
    return 0;
}

int read_line(int sock, char *buffer, int max_size) {
    int total = 0;
    int n;
    char c;

    while (total < max_size - 1) {
        n = recv(sock, &c, 1, 0);
        if (n <= 0) {
            return -1;
        }
        
        buffer[total++] = c;
        
        if (total >= 2 && 
            buffer[total-2] == '\r' && 
            buffer[total-1] == '\n') {
            break;
        }
    }
    
    buffer[total] = '\0';
    return total;
}

void handle_client(int client_sock, char quotes[MAX_QUOTES][MAX_QUOTE_LENGTH], int num_quotes) {
    char buffer[BUF_SIZE];
    int quote_index = 0;
    int done = 0;
    
    send(client_sock, quotes[quote_index], strlen(quotes[quote_index]), 0);
    quote_index = (quote_index + 1) % num_quotes;
    
    while (!done) {
        memset(buffer, 0, BUF_SIZE);
        if (read_line(client_sock, buffer, BUF_SIZE) <= 0) {
            break;
        }
        
        buffer[strlen(buffer)-2] = '\0';
        
        if (strcmp(buffer, "ANOTHER") == 0) {
            send(client_sock, quotes[quote_index], strlen(quotes[quote_index]), 0);
            quote_index = (quote_index + 1) % num_quotes;
        }
        else if (strcmp(buffer, "CLOSE") == 0) {
            send(client_sock, "BYE\r\n", 5, 0);
            done = 1;
        }
        else {
            send(client_sock, "ERROR\r\n", 7, 0);
        }
    }
    
    close(client_sock);
}

int main(int argc, char *argv[]) {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char quotes[MAX_QUOTES][MAX_QUOTE_LENGTH];
    int num_quotes = 0;
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <quotes_file>\n", argv[0]);
        return 1;
    }
    
    if (load_quotes(argv[1], quotes, &num_quotes) < 0) {
        fprintf(stderr, "Error: Unable to open quotes file\n");
        return 1;
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        fprintf(stderr, "Error: Unable to create socket\n");
        return 1;
    }
    
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Error: Unable to bind socket\n");
        return 1;
    }
    
    listen(server_sock, 5);
    
    while (1) {
        client_len = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_sock < 0) {
            fprintf(stderr, "Error: Unable to accept connection\n");
            continue;
        }
        
        handle_client(client_sock, quotes, num_quotes);
    }
    
    return 0;
}
