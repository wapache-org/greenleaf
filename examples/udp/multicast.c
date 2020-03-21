/* 

multicast.c

The following program sends or receives multicast packets. 
If invoked with one argument, it sends a packet containing the current time to an arbitrarily chosen multicast group and UDP port. 
If invoked with no arguments, it receives and prints these packets. 
Start it as a sender on just one host and as a receiver on all the other hosts
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define EXAMPLE_PORT 6239
#define EXAMPLE_GROUP "239.0.0.1"
#define EXAMPLE_LOCAL "10.150.90.66"

int main(int argc, char* argv[])
{
    printf("role: %s\n", argc>1 ? "send":"receive");

   struct sockaddr_in addr;
   int addrlen, sock, cnt;
   struct ip_mreq mreq;
   char message[50];

   /* set up socket */
   sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) {
     perror("socket");
     exit(1);
   }
   bzero((char *)&addr, sizeof(addr));

   addr.sin_family = AF_INET;
   addr.sin_port = htons(EXAMPLE_PORT);
   addrlen = sizeof(addr);

   if (argc > 1) {
      /* send */

      addr.sin_addr.s_addr = inet_addr(EXAMPLE_GROUP);

      while (1) {
        time_t t = time(0);
        sprintf(message, "time is %-24.24s", ctime(&t));
        printf("sending: %s\n", message);
        cnt = sendto(sock, message, sizeof(message), 0, (struct sockaddr *) &addr, addrlen);
        if (cnt < 0) {
            perror("sendto");
            exit(1);
        }
        sleep(5);
      }
   } else {
      /* receive */

    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //    addr.sin_addr.s_addr = inet_addr(EXAMPLE_LOCAL); // RODO 为什么不能指定绑定到哪个IP?一旦指定就接收不到数据了

      if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {        
         perror("bind");
	    exit(1);
      }

      mreq.imr_multiaddr.s_addr = inet_addr(EXAMPLE_GROUP);         
      mreq.imr_interface.s_addr = htonl(INADDR_ANY);            
    //   mreq.imr_interface.s_addr = inet_addr(EXAMPLE_LOCAL); // 指定用哪个IP加入组
      if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt mreq");
        exit(1);
      }

      struct sockaddr_in sender_addr;
      int sender_addr_length;
      while (1) {
        cnt = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *) &sender_addr, &sender_addr_length);
        if (cnt < 0) {
            perror("recvfrom");
            exit(1);
        } else if (cnt == 0) {
            break;
        }
        printf("%s: message = \"%s\"\n", inet_ntoa(sender_addr.sin_addr), message);
      }
    }

    return 0;
}

