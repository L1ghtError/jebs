#include "stdlib.h"
#include "string.h"
#include <netinet/in.h>
#include <stdio.h>

#include "dns/dns-parse.h"
#include "dns/dns-protocol.h"

dns_rc_t
process_qname (uint8_t *dst, const uint8_t *src, int length)
{
   int segment_length = *src;
   if (segment_length == 0 || dst == NULL || src == NULL) {
      return kInvalidInput;
   }
   uint8_t *d = dst;
   for (const uint8_t *c = src + 1; *c != 0 || (c - src) >= length; ++c) {
      if (segment_length > 0) {
         if ((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') || (*c >= '0' && *c <= '9') || (*c == '-')) {
            *d = *c;
            ++d;
         } else {
            return kInvalidInput;
         }
         --segment_length;
      } else if (segment_length == 0) {
         segment_length = *c;
         *d = '.';
         ++d;
      }
   }
   if (d - dst < length) {
      d = 0;
   }
   return kOk;
}

dns_h_t *
new_dns_h (const uint8_t *req, dns_rc_t *rc)
{
   dns_rc_t *lrc = rc;
   if (rc == NULL) {
      dns_rc_t trc;
      lrc = &trc;
   }
   *lrc = kOk;

   if (req == NULL) {
      *lrc = kInvalidInput;
      return NULL;
   }
   dns_header_t *hdr_ptr = (dns_header_t *) req;
   dns_h_t *dns = (dns_h_t *) malloc (sizeof (*dns));
   dns->header.id = ntohs (hdr_ptr->id);
   dns->header.hb3 = hdr_ptr->hb3;
   dns->header.hb4 = hdr_ptr->hb4;
   dns->header.qdcount = ntohs (hdr_ptr->qdcount);
   dns->header.ancount = ntohs (hdr_ptr->ancount);
   dns->header.nscount = ntohs (hdr_ptr->nscount);
   dns->header.arcount = ntohs (hdr_ptr->arcount);

   dns->qrs = (dns_qrr_t *) malloc (dns->header.qdcount * sizeof (*dns->qrs));
   const uint8_t *cur_rr = req + sizeof (*hdr_ptr);
   for (int i = 0; i < dns->header.qdcount; i++) {
      // Read and offset position of cur_rr to type field
      process_qname ((char *) dns->qrs[i].name, (char *) cur_rr, RR_NAME_MAX);
      size_t len = strlen ((char *) dns->qrs[i].name) + 1;
      cur_rr += len;
      GETSHORT (dns->qrs[i].type, cur_rr);
      GETSHORT (dns->qrs[i].clas, cur_rr);
   }
   return dns;
};
