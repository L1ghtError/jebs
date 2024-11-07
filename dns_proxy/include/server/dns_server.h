#ifndef _DNS_SERVER_H_
#define _DNS_SERVER_H_

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "netdb.h"

#include "configuration/configuration.h"
#include "utils/status.h"

#ifdef __linux__
#define DNS_SOCK int
#endif

struct dns_server {
   struct sockaddr_storage s_storage;
   struct sockaddr_storage u_storage;
   struct addrinfo s_hints;
   struct addrinfo u_hints;
   char s_host[INET6_ADDRSTRLEN];
   char u_host[INET6_ADDRSTRLEN];
   DNS_SOCK self_sockfd;
   DNS_SOCK upstream_sockfd;
   uint16_t s_port;
   uint16_t u_port;
};
typedef struct dns_server dns_server_t;

dns_server_t *
init_dns_server (const dns_conf_t *conf, dns_rc_t *rc);

void
destroy_dns_server (dns_server_t *server);

dns_rc_t
run_dns_server (const dns_server_t *server);


const uint8_t *
validate_dns_conf (const dns_conf_t *conf, dns_rc_t *rc);

#endif // _DNS_SERVER_H_