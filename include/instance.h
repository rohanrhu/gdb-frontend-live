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

#ifndef __GDBFRONTENDLIVE_INSTANCE_H__
#define __GDBFRONTENDLIVE_INSTANCE_H__

#include <stdio.h>
#include <sys/types.h>
#include <grp.h>

#include "../include/util.h"
#include "../include/websocket.h"

extern char* gdbfrontendlive_instance_user_name;
extern uid_t gdbfrontendlive_instance_user_uid;

extern char* gdbfrontendlive_instance_group_name;
extern uid_t gdbfrontendlive_instance_group_gid;

typedef struct gdbfrontendlive_instance_gfproc gdbfrontendlive_instance_gfproc_t;
struct gdbfrontendlive_instance_gfproc {
    pid_t pid;
};

typedef struct gdbfrontendlive_websocket_clients gdbfrontendlive_websocket_clients_t;

typedef struct gdbfrontendlive_instance gdbfrontendlive_instance_t;
struct gdbfrontendlive_instance {
    char* id;
    char* workdir;
    int is_running;
    char* host_address;
    char* bind_address;
    gdbfrontendlive_instance_gfproc_t* gfproc;
    u_int16_t http_port;
    gdbfrontendlive_websocket_clients_t* client;
    gdbfrontendlive_instance_t* prev;
    gdbfrontendlive_instance_t* next;
};

gdbfrontendlive_instance_t* instances;
gdbfrontendlive_instance_t* instances_current;

extern gdbfrontendlive_instance_t* gdbfrontendlive_instance_create();
extern void gdbfrontendlive_instance_run(gdbfrontendlive_instance_t* instance);
extern void gdbfrontendlive_instance_destroy(gdbfrontendlive_instance_t* instance);
extern gdbfrontendlive_instance_t* gdbfrontendlive_instance_getby_id(char* id);

static void proc_exit_handler(int status);

#endif