#include <arpa/nameser_compat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>


#include "configuration/configuration.h"
#include "server/dns_server.h"
#include "utils/network_tools.h"
//#include "utils/network_tools.h"
int
main ()
{
   dns_rc_t ret = kOk;
   dns_conf_t *conf = new_dns_conf_from_json ("/home/lighterror/Documents/InangoTasks/dns_proxy/config.json", &ret);
   if (ret != kOk) {
      printf ("Err, new_dns_conf_from_json %s\n", code_desc[ret]);
      return -(ret);
   }
   const uint8_t *c_err_msg = validate_dns_conf (conf, &ret);
   if (ret != kOk) {
      printf ("Err, %s\n", c_err_msg);
      return -(ret);
   }

   dns_server_t *server = init_dns_server (conf, &ret);
   if (ret != kOk) {
      printf ("Err, cannot configure dns server %s\n", code_desc[ret]);
      return -(ret);
   }
   char host_ip[INET6_ADDRSTRLEN] = {0};
   get_sockaddr_ip (&server->s_storage, host_ip, sizeof (host_ip));
   printf ("listening on %s:%d\n", server->s_host, server->s_port);
   ret = run_dns_server (server);
   if (ret != kOk) {
      printf ("Err, cannot run dns server %s\n", code_desc[ret]);
      return -(ret);
   }

   return 0;
}
