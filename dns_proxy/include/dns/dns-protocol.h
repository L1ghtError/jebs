#ifndef _DNS_PROTOCOL_
#define _DNS_PROTOCOL_
#include <stdint.h>

#define NAMESERVER_PORT 53

#define RR_NAME_MAX 255 /* max length for rr name */

#pragma pack(push, 1)
struct dns_header {
   uint16_t id;
   uint8_t hb3, hb4;
   uint16_t qdcount, ancount, nscount, arcount;
};
typedef struct dns_header dns_header_t;
#pragma pack(pop)
#define HB3_QR 0x80 /* Query */
#define HB3_OPCODE 0x78
#define HB3_AA 0x04 /* Authoritative Answer */
#define HB3_TC 0x02 /* TrunCated */
#define HB3_RD 0x01 /* Recursion Desired */

#define HB4_RA 0x80 /* Recursion Available */
#define HB4_AD 0x20 /* Authenticated Data */
#define HB4_CD 0x10 /* Checking Disabled */
#define HB4_RCODE 0x0f

#define OPCODE(x) (((x)->hb3 & HB3_OPCODE) >> 3)
#define SET_OPCODE(x, code) (x)->hb3 = ((x)->hb3 & ~HB3_OPCODE) | code

#define RCODE(x) ((x)->hb4 & HB4_RCODE)
#define SET_RCODE(x, code) (x)->hb4 = ((x)->hb4 & ~HB4_RCODE) | code

#define GETSHORT(s, cp)                                       \
   {                                                          \
      unsigned char *t_cp = (unsigned char *) (cp);           \
      (s) = ((uint16_t) t_cp[0] << 8) | ((uint16_t) t_cp[1]); \
      (cp) += 2;                                              \
   }

#define GETLONG(l, cp)                                                                                               \
   {                                                                                                                 \
      unsigned char *t_cp = (unsigned char *) (cp);                                                                  \
      (l) =                                                                                                          \
         ((uint32_t) t_cp[0] << 24) | ((uint32_t) t_cp[1] << 16) | ((uint32_t) t_cp[2] << 8) | ((uint32_t) t_cp[3]); \
      (cp) += 4;                                                                                                     \
   }

#pragma pack(push, 1)
struct dns_qrr {
   uint8_t name[RR_NAME_MAX];
   uint16_t type;
   uint16_t clas;
};
#pragma pack(pop)
typedef struct dns_qrr dns_qrr_t;

#pragma pack(push, 1)
struct dns_arr {
   uint8_t name[RR_NAME_MAX];
   uint16_t type;
   uint16_t clas;
   uint32_t ttl;
   uint16_t rdlength;
   uint8_t *rdata;
};
#pragma pack(pop)
typedef struct dns_arr dns_arr_t;

#endif // _DNS_PROTOCOL_
