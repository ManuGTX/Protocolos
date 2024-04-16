#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 8080
#define SA struct sockaddr

void func(int sockfd, struct sockaddr_in servaddr)
{
    char buff[MAX];
    int n, lenMsg;
    int len = sizeof(servaddr);

    // Loop para el chat
    for (;;) {
        bzero(buff, MAX);

        printf("Ingrese texto : ");
        n = 0;
        // Copio el mensaje del cliente en el buffer
        while ((buff[n++] = getchar()) != '\n');

        // y envío el buffer al servidor
        if (sendto(sockfd, buff, MAX, 0, (SA*)&servaddr, len) < 0) {
            printf("Error en envío de mensaje, saliendo...\n");
            break;
        };
   
        // si el mensaje dice "SALIR" salgo y cierro conexión
        if (strncmp("SALIR", buff, 4) == 0) {
            printf("Salgo del chat...\n");
            break;
        }
        bzero(buff, MAX);

        printf("Esperando respuesta...\n");
        // Leo el mesaje del servidor y lo copio en un buffer
        lenMsg = recvfrom(sockfd, buff, MAX, 0, NULL, NULL);

        if (lenMsg < 0) {
            printf("Error en recepción de mensaje, saliendo...\n");
            break;
        };

        // Muestro el buffer con los datos del servidor
        buff[lenMsg] = '\0';
        printf("Del servidor: %s\n", buff);
    }
}
 
int main()
{
    int sockfd;
    struct sockaddr_in servaddr;
 
    // socket: creo socket y lo verifico
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
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
 
    // Bind del nuevo socket a la dirección IP y lo verifico
    // if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
    //     printf("Falla socket bind ...\n");
    //     exit(0);
    // }
    // else {
    //     printf("Se hizo el socket bind..\n");
    
        // Función para el chat
        func(sockfd, servaddr);
    
        //Cierro el socket
        close(sockfd);
    
}