#ifndef __SMTP_CMD_UTILS_H__
#define __SMTP_CMD_UTILS_H__

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pcre.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "config.h"
#include "sds.h"

#define LOCAL_SIZE (64 + 1)
#define DOMAIN_SIZE (255 + 1)
#define ADDRESS_SIZE (256 + 1)
#define CMD_SIZE (512 + 1)
#define RESP_SIZE (512 + 1)
#define TEXT_LINE (1000 + 1)
#define DATA_SIZE (64 * 1024 + 1)
#define MESSAGE_SIZE (DATA_SIZE + 1024)
#define RCPT_COUNT (100)
#define READ_SIZE (1024 * 1024)
#define DATA_END_STR "\r\n.\r\n"

typedef enum args_error_t {
    ARGS_OK,
    ARGS_ERROR,
    ARGS_PCRE_EXEC,
    ARGS_PCRE_SUBSTRING,
    ARGS_TOO_MUCH
} args_error_t;

typedef enum response_code_t {
    STATUS,                    // 211
    HELP,                      // 214
    READY,                     // 220
    CLOSING,                   // 221
    OK,                        // 250
    USER_NOT_LOCAL_WILL,       // 251
    CANT_VRFY,                 // 252
    START_INPUT,               // 354
    NOT_AVAILABLE,             // 421
    MAILBOX_UNAVAILABLE_MAIL,  // 450
    ABORTED,                   // 451
    SYS_STORAGE,               // 452_1
    TOO_MANY_RECIPIENTS,       // 452_2
    UNABLE_PARAMETERS,         // 455
    SYNTAX,                    // 500_1
    TOO_LONG_LINE,             // 500_2
    SYNTAX_PARAMETERS,         // 501_1
    TOO_LONG_PATH,             // 501_2
    NOT_IMPLEMENTED,           // 502
    BAD_SEQUENCE,              // 503
    PARAMETER_NOT_IMPLEMENTED, // 504
    MAILBOX_UNAVAILABLE,       // 550
    USER_NOT_LOCAL_TRY,        // 551
    STORAGE_EXCEEDED,          // 552_1
    TOO_MUCH_MAIL_DATA,        // 552_2
    MAILBOX_NAME,              // 553
    TRANSACTION_FAILED,        // 554
    MAIL_RCPT_PARAMETERS,      // 555
    RESP_COUNT
} response_code_t;

typedef enum smtp_command_t {
    HELO_CMD,
    EHLO_CMD,
    MAIL_CMD,
    RCPT_CMD,
    DATA_CMD,
    RSET_CMD,
    VRFY_CMD,
    EXPN_CMD,
    HELP_CMD,
    NOOP_CMD,
    QUIT_CMD,
    COMMAND_COUNT,
    ERROR_CMD,
    ERROR_CMD_ARG
} smtp_command_t;

typedef enum smtp_cmd_arg_t {
    DOMAIN_ARG,
    MAIL_ARG,
    CMD_ARG_COUNT
} smtp_cmd_arg_t;

static const char RESP_LIST[RESP_COUNT][70] = {
    "211 System status %s\r\n",
    "214 Help\r\n",
    "220 %s Service ready\r\n",
    "221 %s Service closing transmission channel\r\n",
    "250 OK\r\n",
    "251 User not local; will forward to %s\r\n",
    "252 Cannot VRFY user, but will accept message and attempt delivery\r\n",
    "354 Start mail input; end with <CRLF>.<CRLF>\r\n",
    "421 Service not available, closing transmission channel\r\n",
    "450 Requested mail action not taken: mailbox unavailable\r\n",
    "451 Requested action aborted: error in processing\r\n",
    "452 Requested action not taken: insufficient system storage\r\n",
    "452 Too many recipients\r\n",
    "455 Server unable to accommodate parameters\r\n",
    "500 Syntax error, command unrecognized\r\n",
    "500 Line too long\r\n",
    "501 Syntax error in parameters or arguments\r\n",
    "501 Path too long\r\n",
    "502 Command not implemented\r\n",
    "503 Bad sequence of commands\r\n",
    "504 Command parameter not implemented\r\n",
    "550 Requested action not taken: mailbox unavailable\r\n",
    "551 User not local; please try %s\r\n",
    "552 Requested mail action aborted: exceeded storage allocation\r\n",
    "552 Too much mail data\r\n",
    "553 Requested action not taken: mailbox name not allowed\r\n",
    "554 Transaction failed\r\n",
    "555 MAIL FROM/RCPT TO parameters not recognized or not implemented\r\n"};

static const char CMD_LIST_NAME[COMMAND_COUNT][11] = {
    "helo ", "ehlo ", "mail from:", "rcpt to:", "data", "rset",
    "vrfy ", "expn ", "help",       "noop",     "quit"};

static const char ARG_LIST[CMD_ARG_COUNT][256] = {"domain", "mailbox"};

static const char CMD_LIST_RE[COMMAND_COUNT][1100] = {
    "(?:^)(?i:helo\\x20(?<domain>[0-9a-z](?:[\\x2d0-9a-z]*[0-9a-z])?(?:\\x2e[0-"
    "9a-z](?:[\\x2d0-9a-z]*[0-9a-z])?)*)\\r\\n)(?:$)",
    "(?:^)(?i:ehlo\\x20(?:(?<domain>[0-9a-z](?:[0-9a-z\\x2d]*[0-9a-z])?(?:"
    "\\x2e[0-9a-z](?:[0-9a-z\\x2d]*[0-9a-z])?)*)|\\x5b(?:\\d{1,3}(?:\\x2e\\d{1,"
    "3}){3}|ipv6:(?:[0-9a-f]{1,4}(?::[0-9a-f]{1,4}){7}|(?:[0-9a-f]{1,4}(?::[0-"
    "9a-f]{1,4}){0,5})?::(?:[0-9a-f]{1,4}(?::[0-9a-f]{1,4}){0,5})?|[0-9a-f]{1,"
    "4}(?::[0-9a-f]{1,4}){5}:\\d{1,3}(?:\\x2e\\d{1,3}){3}|(?:[0-9a-f]{1,4}(?::["
    "0-9a-f]{1,4}){0,3})?::(?:[0-9a-f]{1,4}(?::[0-9a-f]{1,4}){0,3}:)?\\d{1,3}(?"
    ":\\x2e\\d{1,3}){3})|[0-9a-z\\x2d]*[0-9a-z]:[!-@-~\\x5e]+)\\x5d)\\r\\n)(?:$"
    ")",
    "(?:^)(?i:mail\\x20from:(?:<(?:@[0-9a-z](?:[\\x2d0-9a-z]*[0-9a-z])?(?:"
    "\\x2e[0-9a-z](?:[\\x2d0-9a-z]*[0-9a-z])?)*(?:,@[0-9a-z](?:[\\x2d0-9a-z]*["
    "0-9a-z])?(?:\\x2e[0-9a-z](?:[\\x2d0-9a-z]*[0-9a-z])?)*)*:)?(?<mailbox>(?:["
    "!\\x23-'\\x2a\\x2b\\x2d/-9=\\x3f\\x5e-~]+(?:\\x2e[!\\x23-'\\x2a\\x2b\\x2d/"
    "-9=\\x3f\\x5e-~]+)*|\"(?:[\\x20!\\x23-@\\x5b\\x5d-~]|\\x5c[\\x20-@\\x5b-~]"
    ")*\")@(?:[0-9a-z](?:[\\x2d0-9a-z]*[0-9a-z])?(?:\\x2e[0-9a-z](?:[\\x2d0-9a-"
    "z]*[0-9a-z])?)*|\\x5b(?:\\d{1,3}(?:\\x2e\\d{1,3}){3}|ipv6:(?:[0-9a-f]{1,4}"
    "(?::[0-9a-f]{1,4}){7}|(?:[0-9a-f]{1,4}(?::[0-9a-f]{1,4}){0,5})?::(?:[0-9a-"
    "f]{1,4}(?::[0-9a-f]{1,4}){0,5})?|[0-9a-f]{1,4}(?::[0-9a-f]{1,4}){5}:\\d{1,"
    "3}(?:\\x2e\\d{1,3}){3}|(?:[0-9a-f]{1,4}(?::[0-9a-f]{1,4}){0,3})?::(?:[0-"
    "9a-f]{1,4}(?::[0-9a-f]{1,4}){0,3}:)?\\d{1,3}(?:\\x2e\\d{1,3}){3})|[\\x2d0-"
    "9a-z]*[0-9a-z]:[!-@\\x5e-~]+)\\x5d))>|<(?<mailbox>)>)(?:\\x20[0-9a-z]["
    "\\x2d0-9a-z]*(?:=[!-<>-@\\x5b-~]+)?(?:\\x20[0-9a-z][\\x2d0-9a-z]*(?:=[!-<>"
    "-@\\x5b-~]+)?)*)?\\r\\n)(?:$)",
    "(?:^)(?i:rcpt\\x20to:(?:<(?-i:Postmaster)@[0-9a-z](?:[\\x2d-9a-z]*[0-9a-z]"
    ")?(?:\\x2e[0-9a-z](?:[\\x2d0-9a-z]*[0-9a-z])?)*>|<(?-i:Postmaster)>|<(?:@["
    "0-9a-z](?:[\\x2d0-9a-z]*[0-9a-z])?(?:\\x2e[0-9a-z](?:[\\x2d0-9a-z]*[0-9a-"
    "z])?)*(?:,@[0-9a-z](?:[\\x2d0-9a-z]*[0-9a-z])?(?:\\x2e[0-9a-z](?:[\\x2d0-"
    "9a-z]*[0-9a-z])?)*)*:)?(?<mailbox>(?:[!\\x23-'\\x2a\\x2b\\x2d/"
    "-9=\\x3f\\x5e-~]+(?:\\x2e[!\\x23-'\\x2a\\x2b\\x2d/"
    "-9=\\x3f\\x5e-~]+)*|\"(?:[\\x20!\\x23-@\\x5b\\x5d-~]|\\x5c[\\x20-@\\x5b-~]"
    ")*\")@(?:[0-9a-z](?:[\\x2d0-9a-z]*[0-9a-z])?(?:\\x2e[0-9a-z](?:[\\x2d0-9a-"
    "z]*[0-9a-z])?)*|\\x5b(?:\\d{1,3}(?:\\x2e\\d{1,3}){3}|ipv6:(?:[0-9a-f]{1,4}"
    "(?::[0-9a-f]{1,4}){7}|(?:[0-9a-f]{1,4}(?::[0-9a-f]{1,4}){0,5})?::(?:[0-9a-"
    "f]{1,4}(?::[0-9a-f]{1,4}){0,5})?|[0-9a-f]{1,4}(?::[0-9a-f]{1,4}){5}:\\d{1,"
    "3}(?:\\x2e\\d{1,3}){3}|(?:[0-9a-f]{1,4}(?::[0-9a-f]{1,4}){0,3})?::(?:[0-"
    "9a-f]{1,4}(?::[0-9a-f]{1,4}){0,3}:)?\\d{1,3}(?:\\x2e\\d{1,3}){3})|[\\x2d0-"
    "9a-z]*[0-9a-z]:[!-@\\x5e-~]+)\\x5d))>)(?:\\x20[0-9a-z][\\x2d0-9a-z]*(?:=[!"
    "-<>-@\\x5b-~]+)?(?:\\x20[0-9a-z][\\x2d0-9a-z]*(?:=[!-<>-@\\x5b-~]+)?)*)?"
    "\\r\\n)(?:$)",
    "(?:^)(?i:data\\r\\n)(?:$)", "(?:^)(?i:rset\\r\\n)(?:$)",
    "(?:^)(?i:vrfy\\x20(?:[!\\x23-'\\x2a\\x2b\\x2d/"
    "-9=\\x3f\\x5e-~]+|\"(?:[\\x20!\\x23-@\\x5b\\x5d-~]|\\x5c[\\x20-@\\x5b-~])*"
    "\")\\r\\n)(?:$)",
    "(?:^)(?i:expn\\x20(?:[!\\x23-'\\x2a\\x2b\\x2d/"
    "-9=\\x3f\\x5e-~]+|\"(?:[\\x20!\\x23-@\\x5b\\x5d-~]|\\x5c[\\x20-@\\x5b-~])*"
    "\")\\r\\n)(?:$)",
    "(?:^)(?i:help(?:\\x20(?:[!\\x23-'\\x2a\\x2b\\x2d/"
    "-9=\\x3f\\x5e-~]+|\"(?:[\\x20!\\x23-@\\x5b\\x5d-~]|\\x5c[\\x20-@\\x5b-~])*"
    "\"))?\\r\\n)(?:$)",
    "(?:^)(?i:noop(?:\\x20(?:[!\\x23-'\\x2a\\x2b\\x2d/"
    "-9=\\x3f\\x5e-~]+|\"(?:[\\x20!\\x23-@\\x5b\\x5d-~]|\\x5c[\\x20-@\\x5b-~])*"
    "\"))?\\r\\n)(?:$)",
    "(?:^)(?i:quit\\r\\n)(?:$)"};

void init_re(void);
void free_re(void);
int substring_re(char *str, int cmd_code, const char *group_name, char *substr,
                 size_t substr_size);
int is_host_ok(const char *host, const char *ip_str);
int write_message(char *message);

#endif // __SMTP_CMD_UTILS_H__
