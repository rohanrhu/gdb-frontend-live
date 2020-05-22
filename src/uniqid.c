/*
 * GDBFrontendLive is a server that creates sharable GDBFrontend instances and provides online IDEs.
 *
 * https://github.com/rohanrhu/gdb-frontend-live
 * https://oguzhaneroglu.com/projects/gdb-frontend-live/
 *
 * Licensed under GNU/GPLv3
 * Copyright (C) 2020, Oğuzhan Eroğlu (https://oguzhaneroglu.com/) <rohanrhu2@gmail.com>
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/sha.h>
#include <uuid/uuid.h>
#include <string.h>

#include "../include/uniqid.h"

extern char* gdbfrontendlive_uniqid() {
    uuid_t uuid;
    char* uuid_str = malloc(UUID_STR_LEN);
    
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuid_str);
    uuid_unparse(uuid, uuid_str);

    return uuid_str;
}