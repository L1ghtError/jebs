#ifndef _NETWORK_TOOLS_H_
#define _NETWORK_TOOLS_H_

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include "stddef.h"

void *
get_in_addr (const struct sockaddr_storage *sa)
{
   if (sa != NULL) {
      if (sa->ss_family == AF_INET) {
         return &((struct sockaddr_in *) sa)->sin_addr;
      } else {
         return &((struct sockaddr_in6 *) sa)->sin6_addr;
      }
   }
   return NULL;
}

void
get_sockaddr_ip (const struct sockaddr_storage *sa, char *s, int buflen)
{
   inet_ntop (sa->ss_family, get_in_addr (sa), s, buflen);
}

#endif // _NETWORK_TOOLS_H_