#include <cstdio>
#include <iostream>
#include <string.h>
#include <stdlib.h>     /* strtof */
#include <math.h>
#include "socket_utils.h"

const int _doppler_port_ul = 52003;
const int _doppler_port_dl = 52002;


void sendDoppler(float freq_dl, float freq_ul)
{
    int sock = -1;
    struct sockaddr_in myaddr;
    struct hostent *he;
    uint8_t datagram[128];
    int     datagramLength;
    /* información sobre la dirección del servidor */
    if ((he=gethostbyname("localhost")) != NULL) {
        if ((sock=socket(AF_INET, SOCK_DGRAM, 0))>0) {
            memset(&myaddr, 0, sizeof(myaddr));
            myaddr.sin_family=AF_INET;
            myaddr.sin_addr = *((struct in_addr *)he->h_addr);
            myaddr.sin_port=htons(_doppler_port_ul);
            sprintf((char *) datagram, "F%u\n", (unsigned int) freq_ul);
            datagramLength=strlen((char *) datagram);
            sendto(sock, datagram, strlen((char *) datagram), 0, (struct sockaddr *)&myaddr, sizeof(myaddr));
            close(sock);
        }
    }
    if ((he=gethostbyname("localhost")) != NULL) {
        if ((sock=socket(AF_INET, SOCK_DGRAM, 0))>0) {
            memset(&myaddr, 0, sizeof(myaddr));
            myaddr.sin_family=AF_INET;
            myaddr.sin_addr = *((struct in_addr *)he->h_addr);
            myaddr.sin_port=htons(_doppler_port_dl);
            sprintf((char *) datagram, "F%u\n", (unsigned int)freq_dl);
            datagramLength=strlen((char *) datagram);
            sendto(sock, datagram, strlen((char *) datagram), 0, (struct sockaddr *)&myaddr, sizeof(myaddr));
            close(sock);
        }
    }
}

int main(void)
{
    sendDoppler(437250000, 437250000);
}
