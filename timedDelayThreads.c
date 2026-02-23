//modified by lynneth tigse pena n01424240
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define sleep(x) Sleep(1000 * (x))
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
//for mac added theses lines to work
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#define closesocket(s) close(s)
#endif

#define PORT 8080

typedef struct {
    SOCKET client_socket;
    int client_id;
} client_args_t;

void* handle_client(void* arg) {
    client_args_t* args = (client_args_t*)arg;
    SOCKET client_socket = args->client_socket;
    int client_id = args->client_id;
    free(args);

    const char *message =
    "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello Client!";

    printf("[Server] Handing client %d...\n", client_id);
    printf("[Server] Processing request for 5 seconds...\n");

    sleep(5);

    send(client_socket, message, (int)strlen(message), 0);
    printf("[Server] Response sent to client %d. Closing connection\n", client_id);
    closesocket(client_socket);
    return NULL;
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_count = 0;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);

    printf("Server listening on port %d...\n", PORT);
    printf("NOTE: This server is MULTITHREADED. It can handle multipule client at a time!\n\n");

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket != INVALID_SOCKET){
            client_count++;

            client_args_t* args = malloc(sizeof(client_args_t));
            if(!args){
                closesocket(client_socket);
                continue;
            }
            args->client_socket = client_socket;
            args->client_id = client_count;

            pthread_t tid;
            if (pthread_create(&tid, NULL, handle_client, args) != 0){
                free(args);
                closesocket(client_socket);
                continue;
            }
            pthread_detach(tid);
        }
    }

    closesocket(server_socket);   
#ifdef _WIN32
    WSACleanup();
#else
    close(server_socket);
#endif
    return 0;
}
