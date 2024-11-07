#ifndef _STRING_TOOLS_H_
#define _STRING_TOOLS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "status.h"
#include "ctype.h"
int
str_i_cmp (const char *s1, const char *s2)
{
   while (*s1 && *s2) {
      int diff = tolower ((unsigned char) *s1) - tolower ((unsigned char) *s2);
      if (diff != 0) {
         return diff;
      }
      ++s1;
      ++s2;
   }
   return (unsigned char) *s1 - (unsigned char) *s2;
}


#endif // _STRING_TOOLS_H_