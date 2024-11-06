#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_
#include "cstdint"

enum Filter_type {
   ipv4 = 0,
   ipv6 = 1,
   all = 2,
};

enum Match_type {
   contains = 0,
   exact = 1,
};

struct Filter_conf {
   uint8_t *host;
   uint8_t *predefine_addr;
   Filter_type filter_type;
   Match_type match_type;
};

struct Forwarder_conf {
   uint8_t *addr;
   uint16_t port;
};

struct Conf {
   Forwarder_conf upstream;
   int filter_size;
   Filter_conf filters;
};

#endif // _CONFIGURATION_H_