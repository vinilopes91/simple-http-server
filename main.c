#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

void send_file(int client, int fp)
{
    char buf[1024];
    int nr;

    do
    {
        bzero(buf, 1024);
        nr = read(fp, buf, 1024);
        if (nr > 0)
        {
            send(client, buf, nr, 0);
        }
    } while (nr > 0);
    close(fp);
    close(client);

    printf("DEBUG: Fim send_file\n");
}

void send_headers(int client)
{
    char buf[1024];

    strcpy(buf, "HTTP/1.1 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    printf("DEBUG: Fim headers\n");
}

int main()
{
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int received_characters = 0;

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8001);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    listen(server_socket, 4096);

    int client_socket;

    while (1)
    {
        char client_message[BUFFER_SIZE];
        client_socket = accept(server_socket, NULL, NULL);
        received_characters = recv(client_socket, client_message, BUFFER_SIZE, 0);
        if (received_characters < 1)
        {
            printf("Falha ao ler a requisição\n");
            break;
        }

        printf("%s", client_message);

        int fp = open("google.png", O_RDONLY);

        send_headers(client_socket);
        send_file(client_socket, fp);
    }
    close(server_socket);

    return 0;
}
