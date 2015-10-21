#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <err.h>

char response_ok[] = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
char response_err[] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";

void usage(char *basename) {
    printf("Usage: %s [--port <PORT: 1-65535>]\n", basename);
}

int main(int argc, char *argv[])
{
    int port = 4567, i = 1;
    
    while (i < argc) {
        if (strncmp(argv[i], "--help", sizeof("--help")) == 0) {
            usage(argv[0]);
            exit(0);
        } 
        
        if (strncmp(argv[i], "--port", sizeof("--port")) == 0) {
            if (++i >= argc) {
                usage(argv[0]);
                exit(1);
            }

            port = atoi(argv[i]);
            if (port < 1 || port > 65535) {
                usage(argv[0]);
                exit(1);
            }

            i++; continue;
        }

        usage(argv[0]);
        exit(1);
    }

    int one = 1, client_fd;
    struct sockaddr_in svr_addr, cli_addr;
    socklen_t sin_len = sizeof(cli_addr);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        err(1, "Can't open socket");

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) == -1) {
        close(sock);
        err(1, "Can't bind");
    }

    listen(sock, 5);
    while (1) {
        client_fd = accept(sock, (struct sockaddr *) &cli_addr, &sin_len);

        if (client_fd == -1) {
            perror("Can't accept");
            continue;
        }

        fprintf(stderr, "Reloading nginx...\n");
        if (system("nginx -s reload") == 0)
            write(client_fd, response_ok, sizeof(response_ok) - 1); /*-1:'\0'*/
        else 
            write(client_fd, response_err, sizeof(response_err) - 1); 

        close(client_fd);
    }
}
