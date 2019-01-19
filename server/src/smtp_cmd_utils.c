#include "smtp_cmd_utils.h"

static int re_inited = 0;
static pcre *LIST_RE[COMMAND_COUNT];

void init_re(void) {
    if (re_inited) {
        return;
    }
    const char *pcre_error;
    int pcre_error_offset;
    for (int i = 0; i < COMMAND_COUNT; i++) {
        LIST_RE[i] = pcre_compile(CMD_LIST_RE[i], PCRE_DUPNAMES, &pcre_error,
                                  &pcre_error_offset, NULL);
    }
    re_inited = 1;
}

void free_re(void) {
    if (!re_inited) {
        return;
    }
    for (int i = 0; i < COMMAND_COUNT; i++) {
        pcre_free(LIST_RE[i]);
    }
    re_inited = 0;
}

int substring_re(char *str, int cmd_code, const char *group_name, char *substr,
                 size_t substr_size) {
    int substr_vec[30];
    pcre *re = LIST_RE[cmd_code];
    int exec_ret = pcre_exec(re, NULL, str, strlen(str), 0, 0, substr_vec, 30);
    if (exec_ret < 0) {
        return ARGS_PCRE_EXEC;
    }
    if (exec_ret == 0) {
        exec_ret = 10;
    }
    if (substr != NULL) {
        const char *tmp_substr;
        int rc = pcre_get_named_substring(re, str, substr_vec, exec_ret,
                                          group_name, &tmp_substr);
        if (rc < 0) {
            return ARGS_PCRE_SUBSTRING;
        }
        snprintf(substr, substr_size, "%s", tmp_substr);
        pcre_free_substring(tmp_substr);
    }
    return ARGS_OK;
}

int is_host_ok(const char *host, const char *ip_str) {
    struct addrinfo hints, *res;
    char addrstr[INET6_ADDRSTRLEN];
    void *ptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;
    if (getaddrinfo(host, NULL, &hints, &res) != 0) {
        return FALSE;
    }
    while (res) {
        if (res->ai_family == AF_INET) {
            ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        } else {
            ptr = &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
        }
        inet_ntop(res->ai_family, ptr, addrstr, INET6_ADDRSTRLEN);
        if (!strcmp(addrstr, ip_str)) {
            freeaddrinfo(res);
            return TRUE;
        }
        struct addrinfo *tmp_res = res;
        res = res->ai_next;
        freeaddrinfo(tmp_res);
    }
    freeaddrinfo(res);
    return FALSE;
}

int create_dir(char *dir_name) {
    struct stat st;
    if (stat(dir_name, &st) == -1) {
        if (mkdir(dir_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
            return ERROR_RET;
        }
    }
    return SUCCESS_RET;
}

int create_maildir(void) {
    if (create_dir(MAILDIR) == ERROR_RET) {
        return ERROR_RET;
    }
    if (create_dir(MAILDIR_TMP) == ERROR_RET) {
        return ERROR_RET;
    }
    if (create_dir(MAILDIR_NEW) == ERROR_RET) {
        return ERROR_RET;
    }
    if (create_dir(MAILDIR_CUR) == ERROR_RET) {
        return ERROR_RET;
    }
    return SUCCESS_RET;
}

int write_message(char *message) {
    if (create_maildir() == ERROR_RET) {
        return ERROR_RET;
    }
    int sleep_count = 0;
    int end_cycle = 0;
    char tmp_file[FILE_PATH_SIZE];
    char file_name[FILE_PATH_SIZE];
    do {
        unsigned long tm = time(NULL);
        snprintf(file_name, FILE_PATH_SIZE, "%lu.%d.%s.%ld", tm, getpid(), HOST,
                 random());
        snprintf(tmp_file, FILE_PATH_SIZE, "%s/%s", MAILDIR_TMP, file_name);
        struct stat st;
        end_cycle = stat(tmp_file, &st) && errno == ENOENT;
        if (!end_cycle) {
            sleep(2);
            sleep_count++;
        }
        if (sleep_count > 10) {
            return ERROR_RET;
        }
    } while (!end_cycle);
    FILE *f = fopen(tmp_file, "w");
    if (!f) {
        return ERROR_RET;
    }
    if (fprintf(f, "%s", message) < 0 || fclose(f) == EOF) {
        return ERROR_RET;
    }
    char new_file[FILE_PATH_SIZE];
    snprintf(new_file, FILE_PATH_SIZE, "%s/%s", MAILDIR_NEW, file_name);
    if (link(tmp_file, new_file) == -1) {
        return ERROR_RET;
    }
    return SUCCESS_RET;
}
