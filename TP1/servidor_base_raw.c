#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()

#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h> // if_nametoindex(3)
// #include <netpacket/packet.h> // Agregar esta línea

#include <malloc.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>

#define MAX 80
#define PORT 8080
#define SA struct sockaddr

#define BUFFER_SIZE 65536

// /// para cada ioctl, mantenga una estructura ifreq diferente; de lo contrario, puede producirse un error al enviar (sendto)
struct ifreq ifreq_c,ifreq_i,ifreq_ip; 
int sock_raw;
unsigned char *sendbuff;
 
#define DESTMAC0	0xd0
#define DESTMAC1	0x67
#define DESTMAC2	0xe5
#define DESTMAC3	0x12
#define DESTMAC4	0x6f
#define DESTMAC5	0x8f
 
#define ip_destino 127.0.0.1

int total_len = 0,send_len;

void get_eth_index()
{
	memset(&ifreq_i,0,sizeof(ifreq_i));
	strncpy(ifreq_i.ifr_name,"lo",IFNAMSIZ-1);

	if((ioctl(sock_raw,SIOCGIFINDEX,&ifreq_i))<0) {
		printf("error index ioctl ");
        perror("ioctl");
        exit(EXIT_FAILURE);
    }

	printf("index=%d\n",ifreq_i.ifr_ifindex);

}

void get_mac()
{
	memset(&ifreq_c,0,sizeof(ifreq_c));
	strncpy(ifreq_c.ifr_name,"lo",IFNAMSIZ-1);

	if((ioctl(sock_raw,SIOCGIFHWADDR,&ifreq_c))<0)
		printf("error en SIOCGIFHWADDR ioctl ");

	printf("Mac= %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]),(unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]));

	
	printf("iniciando paquete ethernet  ... \n");
	
	struct ethhdr *eth = (struct ethhdr *)(sendbuff);
  	eth->h_source[0] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[0]);
  	eth->h_source[1] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[1]);
   	eth->h_source[2] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[2]);
   	eth->h_source[3] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[3]);
   	eth->h_source[4] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[4]);
   	eth->h_source[5] = (unsigned char)(ifreq_c.ifr_hwaddr.sa_data[5]);

   	eth->h_dest[0]    =  DESTMAC0;
   	eth->h_dest[1]    =  DESTMAC1;
   	eth->h_dest[2]    =  DESTMAC2;
  	eth->h_dest[3]    =  DESTMAC3;
   	eth->h_dest[4]    =  DESTMAC4;
   	eth->h_dest[5]    =  DESTMAC5;

   	eth->h_proto = htons(ETH_P_IP);   //0x800

   	printf("ethernet packaging done.\n");

	total_len+=sizeof(struct ethhdr);


}

void get_data()
{
	sendbuff[total_len++]	=	'R';
	sendbuff[total_len++]	=	'S';
	sendbuff[total_len++]	=	'T';
	sendbuff[total_len++]	=	'A';
	sendbuff[total_len++]	=	'\0';

}

void get_udp()
{
	struct udphdr *uh = (struct udphdr *)(sendbuff + sizeof(struct iphdr) + sizeof(struct ethhdr));

	uh->source	= htons(23451);
	uh->dest	= htons(23452);
	uh->check	= 0;

	total_len+= sizeof(struct udphdr);
	get_data();
	uh->len		= htons((total_len - sizeof(struct iphdr) - sizeof(struct ethhdr)));

}

unsigned short checksum(unsigned short* buff, int _16bitword)
{
	unsigned long sum;
	for(sum=0;_16bitword>0;_16bitword--)
		sum+=htons(*(buff)++);
	do
	{
		sum = ((sum >> 16) + (sum & 0xFFFF));
	}
	while(sum & 0xFFFF0000);

	return (~sum);
}
 
 
void get_ip()
{
	memset(&ifreq_ip,0,sizeof(ifreq_ip));
	strncpy(ifreq_ip.ifr_name,"lo",IFNAMSIZ-1);
  	 if(ioctl(sock_raw,SIOCGIFADDR,&ifreq_ip)<0)
 	 {
		printf("error en SIOCGIFADDR \n");
	 }
	
	printf("%s\n",inet_ntoa((((struct sockaddr_in*)&(ifreq_ip.ifr_addr))->sin_addr)));

    /****** OR
	int i;
	for(i=0;i<14;i++)
	printf("%d\n",(unsigned char)ifreq_ip.ifr_addr.sa_data[i]); ******/

	struct iphdr *iph = (struct iphdr*)(sendbuff + sizeof(struct ethhdr));
	iph->ihl	= 5;
	iph->version	= 4;
	iph->tos	= 16;
	iph->id		= htons(10201);
	iph->ttl	= 64;
	iph->protocol	= 17;
	iph->saddr	= inet_addr(inet_ntoa((((struct sockaddr_in *)&(ifreq_ip.ifr_addr))->sin_addr)));
	iph->daddr	= inet_addr("ip_destino"); 
	total_len += sizeof(struct iphdr); 
	get_udp();

	iph->tot_len	= htons(total_len - sizeof(struct ethhdr));
	iph->check	= htons(checksum((unsigned short*)(sendbuff + sizeof(struct ethhdr)), (sizeof(struct iphdr)/2)));

}

void procesar_paquete(unsigned char *buffer, int size) {
    printf("Paquete recibido - Longitud: %d bytes\n", size);

    // Encabezado Ethernet
    struct ethhdr *encabezado_eth = (struct ethhdr *)buffer;
    printf("Encabezado Ethernet\n");
    printf("  Dirección MAC de origen: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
           encabezado_eth->h_source[0], encabezado_eth->h_source[1], encabezado_eth->h_source[2],
           encabezado_eth->h_source[3], encabezado_eth->h_source[4], encabezado_eth->h_source[5]);
    printf("  Dirección MAC de destino: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
           encabezado_eth->h_dest[0], encabezado_eth->h_dest[1], encabezado_eth->h_dest[2],
           encabezado_eth->h_dest[3], encabezado_eth->h_dest[4], encabezado_eth->h_dest[5]);
    printf("  Tipo de protocolo: 0x%.4X\n", ntohs(encabezado_eth->h_proto));

    // Encabezado IP
    struct iphdr *encabezado_ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    printf("Encabezado IP\n");
    printf("  Versión IP: %d\n", encabezado_ip->version);
    printf("  Longitud del encabezado IP: %d bytes\n", encabezado_ip->ihl * 4);
    printf("  Tipo de servicio: %d\n", encabezado_ip->tos);
    printf("  Longitud total: %d bytes\n", ntohs(encabezado_ip->tot_len));
    printf("  Identificación: %d\n", ntohs(encabezado_ip->id));
    printf("  Fragmentación: Flags: %d, Offset: %d\n", (encabezado_ip->frag_off & 0x1FFF), (encabezado_ip->frag_off & 0xE000) >> 13);
    printf("  Tiempo de vida: %d\n", encabezado_ip->ttl);
    printf("  Protocolo: %d\n", encabezado_ip->protocol);
    printf("  Suma de control: 0x%.4X\n", ntohs(encabezado_ip->check));
    printf("  Dirección IP de origen: %s\n", inet_ntoa(*(struct in_addr *)&encabezado_ip->saddr));
    printf("  Dirección IP de destino: %s\n", inet_ntoa(*(struct in_addr *)&encabezado_ip->daddr));

    // Encabezado TCP
    if (encabezado_ip->protocol == IPPROTO_TCP) {
        struct tcphdr *encabezado_tcp = (struct tcphdr *)(buffer + sizeof(struct ethhdr) + encabezado_ip->ihl * 4);
        printf("Encabezado TCP\n");
        printf("  Puerto de origen: %d\n", ntohs(encabezado_tcp->source));
        printf("  Puerto de destino: %d\n", ntohs(encabezado_tcp->dest));
        printf("  Número de secuencia: %u\n", ntohl(encabezado_tcp->seq));
        printf("  Número de acuse de recibo: %u\n", ntohl(encabezado_tcp->ack_seq));
        printf("  Longitud de encabezado TCP: %d bytes\n", encabezado_tcp->doff * 4);
        printf("  Flags: ");
        if (encabezado_tcp->syn) printf("SYN ");
        if (encabezado_tcp->ack) printf("ACK ");
        if (encabezado_tcp->fin) printf("FIN ");
        if (encabezado_tcp->rst) printf("RST ");
        if (encabezado_tcp->psh) printf("PSH ");
        if (encabezado_tcp->urg) printf("URG ");
        printf("\n");
        printf("  Tamaño de ventana: %d\n", ntohs(encabezado_tcp->window));
        printf("  Suma de control: 0x%.4X\n", ntohs(encabezado_tcp->check));
        printf("  Puntero de urgencia: %d\n", ntohs(encabezado_tcp->urg_ptr));
    }

    // Encabezado UDP
    if (encabezado_ip->protocol == IPPROTO_UDP) {
        struct udphdr *encabezado_udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + encabezado_ip->ihl * 4);
        printf("Encabezado UDP\n");
        printf("  Puerto de origen: %d\n", ntohs(encabezado_udp->source));
        printf("  Puerto de destino: %d\n", ntohs(encabezado_udp->dest));
        printf("  Longitud total: %d bytes\n", ntohs(encabezado_udp->len));
        printf("  Suma de control: 0x%.4X\n", ntohs(encabezado_udp->check));
    }

    printf("Datos:\n");
    int i;
    for (i = sizeof(struct ethhdr) + encabezado_ip->ihl * 4; i < size; ++i) {
        printf("%.2X ", buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}
   
// Función para el chat entre el cliente y el servidor
void func(int sockfd)
{
    unsigned char buffer[BUFFER_SIZE];
    int n, connfd, lenMsg, bytes_recibidos;
    struct sockaddr_in cli;
    int len = sizeof(cli);

    // aumentar si los datos son más grandes --> AA  BB  CC  DD  EE
	sendbuff = (unsigned char*)malloc(64); 
    memset(sendbuff,0,64);
    
    // interfaz
    get_eth_index(); 
    get_mac();
    get_ip();

	struct sockaddr_ll sadr_ll;
	sadr_ll.sll_ifindex = if_nametoindex("lo");
	sadr_ll.sll_halen   = ETH_ALEN;
	sadr_ll.sll_addr[0]  = DESTMAC0;
	sadr_ll.sll_addr[1]  = DESTMAC1;
	sadr_ll.sll_addr[2]  = DESTMAC2;
	sadr_ll.sll_addr[3]  = DESTMAC3;
	sadr_ll.sll_addr[4]  = DESTMAC4;
	sadr_ll.sll_addr[5]  = DESTMAC5;

    // Loop para el chat
    for (;;) {
        bzero(buffer, BUFFER_SIZE);
        bytes_recibidos = -1;
        send_len = -1;

        printf("Esperando mensaje... \n");
        bytes_recibidos = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_recibidos < 0) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        printf("Del cliente: ");
        procesar_paquete(buffer, bytes_recibidos);

        bzero(buffer, BUFFER_SIZE);

        // y envío el buffer al cliente
        send_len = sendto(sockfd,sendbuff,64,0,(const struct sockaddr*)&sadr_ll,sizeof(struct sockaddr_ll));
		if(send_len < 0)
		{
			printf("error enviando....sendlen=%d....errno=%d\n",send_len,errno);
			exit(EXIT_FAILURE);
		}
    }
}
   
// Función principal
int main()
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
   
    unsigned char buffer[BUFFER_SIZE];

    // Crear un socket raw
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Socket creado...\n");
    }
    bzero(&servaddr, sizeof(servaddr));
   
    // Configurar para recibir desde localhost (loopback)
    struct sockaddr_ll addr; // Corregir aquí
    socklen_t addr_len = sizeof(addr); // Corregir aquí
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = if_nametoindex("lo");
   
    // Enlazar el socket a la interfaz loopback
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Se hace el socket bind ..\n");

        // Funcion para el chat entre el cliente y el servidor
        func(sockfd);
    }
   
    // Cierro el socket al terminar el chat
    close(sockfd);
}