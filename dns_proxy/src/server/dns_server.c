#include "server/dns_server.h"
#include "dns/dns-parse.h"
#include "stdlib.h"
#include "string.h"
#include <errno.h>

dns_rc_t
init_dns_addrinfo (struct addrinfo *ainfo, const char *host, uint16_t port, struct sockaddr_storage *storage)
{
   memset (storage, 0, sizeof (*storage)); // Ensure zero-initialization
   memset (ainfo, 0, sizeof (*ainfo));     // Ensure zero-initialization

   struct sockaddr_in *sa = (struct sockaddr_in *) storage;
   struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *) storage;
   sa->sin_port = port;
   ainfo->ai_family = AF_INET;
   ainfo->ai_socktype = SOCK_DGRAM;
   ainfo->ai_protocol = IPPROTO_UDP;
   ainfo->ai_addr = (struct sockaddr *) sa;
   ainfo->ai_addrlen = sizeof (*sa);

   sa->sin_family = AF_INET;
   if (inet_pton (AF_INET, host, &sa->sin_addr) == 1) {
      return kOk;
   }
   sa6->sin6_port = port;
   ainfo->ai_family = AF_INET6;
   ainfo->ai_protocol = IPPROTO_UDP;
   ainfo->ai_addr = (struct sockaddr *) sa6;
   ainfo->ai_addrlen = sizeof (*sa6);

   sa6->sin6_family = AF_INET6;
   if (inet_pton (AF_INET6, host, &sa6->sin6_addr) == 1) {
      return kOk;
   }

   return kDataMalformed;
}

DNS_SOCK
bind_dns_socket (const struct addrinfo *ainfo, struct sockaddr_storage *storage)
{
   DNS_SOCK sockfd = -1;
   if ((sockfd = socket (ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol)) == -1) {
      close (sockfd);
      return -1;
   }
   int so_reuseaddr = 1;
   if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof (int)) == -1) {
      close (sockfd);
      return -1;
   }

   if (bind (sockfd, ainfo->ai_addr, ainfo->ai_addrlen) == -1) {
      close (sockfd);
      return -1;
   }
   return sockfd;
}

DNS_SOCK
connect_dns_socket (const struct addrinfo *ainfo, struct sockaddr_storage *storage)
{
   DNS_SOCK sockfd = -1;
   if ((sockfd = socket (ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol)) == -1) {
      close (sockfd);
      return -1;
   }
   int so_reuseaddr = 1;
   if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof (int)) == -1) {
      close (sockfd);
      return -1;
   }
   if (connect (sockfd, ainfo->ai_addr, ainfo->ai_addrlen) == -1) {
      close (sockfd);
      return -1;
   }
}

dns_server_t *
init_dns_server (const dns_conf_t *conf, dns_rc_t *rc)
{
   dns_rc_t *lrc = rc;
   if (rc == NULL) {
      dns_rc_t trc;
      lrc = &trc;
   }
   *lrc = kOk;

   if (conf == NULL) {
      *lrc = kInvalidInput;
      return NULL;
   }

   validate_dns_conf (conf, lrc);
   if (*lrc != kOk) {
      return NULL;
   }
   size_t addrlen = strlen (conf->self.addr);
   dns_server_t *server = (dns_server_t *) calloc (1, sizeof (*server));
   strncpy (server->s_host, conf->self.addr, addrlen);
   server->s_port = conf->self.port;

   addrlen = strlen (conf->upstream.addr);
   strncpy (server->u_host, conf->upstream.addr, addrlen);
   server->u_port = conf->upstream.port;


   *lrc = init_dns_addrinfo (&server->s_hints, server->s_host, server->s_port, &server->s_storage);
   if (*lrc != kOk) {
      destroy_dns_server (server);
      return NULL;
   }
   server->self_sockfd = bind_dns_socket (&server->s_hints, &server->s_storage);
   if (server->self_sockfd == -1) {
      *lrc = kAborted;
      destroy_dns_server (server);
      return NULL;
   }

   *lrc = init_dns_addrinfo (&server->u_hints, server->u_host, server->u_port, &server->u_storage);
   if (*lrc != kOk) {
      destroy_dns_server (server);
      return NULL;
   }
   server->upstream_sockfd = connect_dns_socket (&server->u_hints, &server->u_storage);
   if (server->upstream_sockfd == -1) {
      *lrc = kAborted;
      destroy_dns_server (server);
      return NULL;
   }
   return server;
}

void
destroy_dns_server (dns_server_t *server)
{
   free (server);
   close (server->self_sockfd);
   close (server->upstream_sockfd);
}
#define BUFFER_SIZE 1024
dns_rc_t
run_dns_server (const dns_server_t *server)
{
   char buffer[BUFFER_SIZE] = {0};
   while (1) {
      struct sockaddr_in client_addr;
      socklen_t c_len = sizeof (client_addr);
      socklen_t u_len = server->u_hints.ai_addrlen;
      ssize_t n;

      // Receive a message from a client
      n = recvfrom (server->self_sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &c_len);
      dns_h_t *dht = new_dns_h (buffer, NULL);
      sendto (server->upstream_sockfd, buffer, n, 0, server->u_hints.ai_addr, server->u_hints.ai_addrlen);
      memset (buffer, 0, BUFFER_SIZE);

      n = recvfrom (server->upstream_sockfd, buffer, BUFFER_SIZE, 0, server->u_hints.ai_addr, &u_len);
      sendto (server->self_sockfd, buffer, n, 0, (struct sockaddr *) &client_addr, &c_len);
      memset (buffer, 0, BUFFER_SIZE);
   }
   return kOk;
}

const uint8_t *
validate_dns_conf (const dns_conf_t *conf, dns_rc_t *rc)
{
   dns_rc_t *lrc = rc;
   if (rc == NULL) {
      dns_rc_t trc;
      lrc = &trc;
   }
   *lrc = kOk;

   if (conf == NULL) {
      *lrc = kInvalidInput;
      static const uint8_t *err = "provided config is NULL";
      return err;
   }


   if (conf->self.addr == NULL) {
      *lrc = kDataMalformed;
      static const uint8_t *err = "server address is not provided";
      return err;
   }

   struct sockaddr_in sa;
   struct sockaddr_in6 sa6;
   if (inet_pton (AF_INET, conf->self.addr, &(sa.sin_addr)) != 1) {
      if (inet_pton (AF_INET6, conf->self.addr, &(sa6.sin6_addr)) != 1) {
         *lrc = kDataMalformed;
         static const uint8_t *err = "provided server address is invalid, it should be valid ipv4 or ipv6 address";
         return err;
      }
   }
   if (conf->self.port == 0) {
      *lrc = kDataMalformed;
      static const uint8_t *err = "provided server port address is 0, it should be greater than 0";
      return err;
   }


   if (conf->upstream.addr == NULL) {
      *lrc = kDataMalformed;
      static const uint8_t *err = "upstream address is not provided";
      return err;
   }

   if (inet_pton (AF_INET, conf->upstream.addr, &(sa.sin_addr)) != 1) {
      if (inet_pton (AF_INET6, conf->upstream.addr, &(sa6.sin6_addr)) != 1) {
         *lrc = kDataMalformed;
         static const uint8_t *err = "provided upstream address is invalid, it should be valid ipv4 or ipv6 address";
         return err;
      }
   }
   if (conf->upstream.port == 0) {
      *lrc = kDataMalformed;
      static const uint8_t *err = "provided upstream port address is 0, it should be greater than 0";
      return err;
   }
   for (int i = 0; i < conf->filter_size; ++i) {
      if (conf->filters[i].host == NULL) {
         *lrc = kDataMalformed;
         static const uint8_t *err = "one of the filters \"host\" is not provided";
         return err;
      }

      if (conf->filters[i].action_type == DNS_AT_REDIRECT) {
         if (conf->filters[i].redirect_addr == NULL) {
            *lrc = kDataMalformed;
            static const uint8_t *err = "selected action is \"redirect\" but \"redirect_addr\" is not provided";
            return err;
         }

         if (inet_pton (AF_INET, conf->filters[i].redirect_addr, &(sa.sin_addr)) != 1) {
            if (inet_pton (AF_INET6, conf->filters[i].redirect_addr, &(sa6.sin6_addr)) != 1) {
               *lrc = kDataMalformed;
               static const uint8_t *err =
                  "provided \"redirect_addr\" is invalid, it should be valid ipv4 or ipv6 address";
               return err;
            }
         }
      }
   }
   return NULL;
}