#ifndef _STATUS_H_
#define _STATUS_H_

static const char *code_desc[] = {"Ok", "Invalid input"};

enum ret_code { kOk = 0, kInvalidInput = 1 };

typedef enum ret_code dns_rc_t;
#endif // _STATUS_H_
