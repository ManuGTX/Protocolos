#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

void func(int sockfd)
{
    char buff[MAX];
    int n, connfd, lenMsg;
    struct sockaddr_in cli;
    int len = sizeof(cli);

    // Loop para el chat
    for (;;) {
        bzero(buff, MAX);

        // Leo el mesaje del cliente y lo copio en un buffer
        lenMsg = recvfrom(sockfd, buff, MAX, 0, (SA*)&cli, &len);

        if (lenMsg < 0) {
            printf("Error en recepción de mensaje, saliendo...\n");
            break;
        };

        // Muestro el buffer con los datos del cliente
        buff[lenMsg] = '\0';
        printf("Del cliente: %s\t: ", buff);
        bzero(buff, MAX);

        n = 0;
        // Copio el mensaje del servidor en el buffer
        while ((buff[n++] = getchar()) != '\n');
   
        // y envío el buffer al cliente
        if (sendto(sockfd, buff, MAX, 0, (SA*)&cli, len) < 0) {
            printf("Error en envío de respuesta, saliendo...\n");
            break;
        };
   
        // si el mensaje dice "SALIR" salgo y cierro conexión
        if (strncmp("SALIR", buff, 4) == 0) {
            printf("Salgo del servidor...\n");
            break;
        }
    }
}
   
// Función principal
int main()
{
    int sockfd;
    struct sockaddr_in servaddr;
   
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        printf("Falla la creación del socket...\n");
        exit(0);
    }
    else
        printf("Socket creado...\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // asigno IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    // Bind del nuevo socket a la dirección IP y lo verifico
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("Falla socket bind ...\n");
        exit(0);
    }
    else {
        printf("Se hace el socket bind ..\n");
        // Funcion para el chat entre el cliente y el servidor
    
        func(sockfd);
    
        // Cierro el socket al terminar el chat
        close(sockfd);
    }
}