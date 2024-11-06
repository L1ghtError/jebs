#include <arpa/nameser_compat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define PORT 53
#include "dns/dns-parse.h"
int
get_resolve (const char *filepath, char **addr)
{
   FILE *file = NULL;
   if ((file = fopen (filepath, "r")) == NULL) {
      printf ("Err, cannot open %s\n", filepath);
      return -1;
   }
   char line[256];
   while (fgets (line, sizeof (line), file)) {
      if (strncmp (line, "nameserver", 10) == 0) {
         char *ip_addr = strtok (line + 11, " \t\n");
         int addrlen = strlen (ip_addr);
         *addr = (char *) malloc ((addrlen * sizeof (**addr)) + 1);
         strncpy (*addr, ip_addr, addrlen);
         break;
      }
   }
   fclose (file);
   return 0;
}
void
dump_resp (char *data, ssize_t len)
{
   for (int i = 0; i < len; ++i) {
      printf ("%02X ", data[i]);
      if (!(i % 16))
         printf ("\n");
   }
}

int
main ()
{
   char *upstr_addr;
   if (get_resolve ("../resolv.conf", &upstr_addr) == 0) {
      printf ("addr:%s\n", upstr_addr);
   };

   int sockfd, sockup;
   struct sockaddr_in server_addr, client_addr, upstream_addr;
   char buffer[BUFFER_SIZE] = {0};

   // Creating the UDP socket
   if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror ("Socket creation failed");
      exit (EXIT_FAILURE);
   }

   if ((sockup = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
      perror ("Upstream socket creation failed");
      exit (EXIT_FAILURE);
   }

   // Setting up the server address structure
   memset (&server_addr, 0, sizeof (server_addr));
   server_addr.sin_family = AF_INET;         // IPv4
   server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on any available network interface
   server_addr.sin_port = htons (PORT);      // Server port

   // Setting up the server address structure
   memset (&upstream_addr, 0, sizeof (upstream_addr));
   upstream_addr.sin_family = AF_INET;         // IPv4
   upstream_addr.sin_addr.s_addr = INADDR_ANY; // Listen on any available network interface
   if (inet_pton (AF_INET, upstr_addr, &upstream_addr.sin_addr) <= 0) {
      perror ("inte_pton err");
      return -1;
   }
   upstream_addr.sin_port = htons (PORT); // Server port

   // Binding the socket to the specified port and IP
   if (bind (sockfd, (const struct sockaddr *) &server_addr, sizeof (server_addr)) < 0) {
      perror ("Bind failed");
      close (sockfd);
      exit (EXIT_FAILURE);
   }

   printf ("UDP Echo Server is running on port %d\n", PORT);

   while (1) {
      socklen_t len = sizeof (client_addr);
      ssize_t n;

      // Receive a message from a client
      n = recvfrom (sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &len);
      dns_h_t *dht = new_dns_h (buffer, NULL);
      sendto (sockup, buffer, n, 0, (struct sockaddr *) &upstream_addr, len);
      memset (buffer, 0, BUFFER_SIZE);

      n = recvfrom (sockup, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &upstream_addr, &len);
      sendto (sockfd, buffer, n, 0, (struct sockaddr *) &client_addr, len);
      memset (buffer, 0, BUFFER_SIZE);
      // printf("Echoed message back to client\n");
   }

   // Close the socket (optional, as this code runs in an infinite loop)
   close (sockfd);
   return 0;
}
