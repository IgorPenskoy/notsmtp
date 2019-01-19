#ifndef __SMTP_CMD_H__
#define __SMTP_CMD_H__

#include "client.h"
#include "sds.h"

int smtp_cmd_handle(client_t *client, sds cmd, int cmd_code, sds *resp);

#endif // __SMTP_CMD_H__
