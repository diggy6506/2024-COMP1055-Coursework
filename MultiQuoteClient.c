#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUF_SIZE 512
#define PORT 1818

int read_line(int sock, char *buffer, int max_size) {
    int total = 0;
    int n;
    char c;

    while (total < max_size - 1) {
        n = recv(sock, &c, 1, 0);
        if (n <= 0) {
            return total;
        }
        buffer[total++] = c;
        if (c == '\n') {
            break;
        }
    }
    buffer[total] = '\0';
    return total;
}

void fetch_quotes(const char *host, int num_quotes) {
    int sock;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUF_SIZE];
    int i;

    server = gethostbyname(host);
    if (!server) {
        fprintf(stderr, "Error: No such host\n");
        exit(1);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Error: Unable to open socket\n");
        exit(1);
    }

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Error: Unable to connect\n");
        exit(1);
    }

    memset(buffer, 0, BUF_SIZE);
    read_line(sock, buffer, BUF_SIZE);
    printf("%s", buffer);

    for (i = 1; i < num_quotes; i++) {
    
        printf("\n");
        
        send(sock, "ANOTHER\r\n", 9, 0);
        
        memset(buffer, 0, BUF_SIZE);
        read_line(sock, buffer, BUF_SIZE);
        printf("%s", buffer);
    }

    send(sock, "CLOSE\r\n", 7, 0);
    
    memset(buffer, 0, BUF_SIZE);
    read_line(sock, buffer, BUF_SIZE);
    
    close(sock);
}

int main(int argc, const char *args[]) {
    const char *host;
    int num_quotes;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <number_of_quotes>\n", args[0]);
        return 1;
    }

    host = args[1];
    num_quotes = atoi(args[2]);

    if (num_quotes <= 0) {
        fprintf(stderr, "Error: Invalid number of quotes\n");
        return 1;
    }

    fetch_quotes(host, num_quotes);
    return 0;
}
