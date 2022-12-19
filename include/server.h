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

#ifndef __GDBFRONTENDLIVE_SERVER_H__
#define __GDBFRONTENDLIVE_SERVER_H__

#include "../include/util.h"
#include "../include/instance.h"
#include "../include/arg.h"

#define GDBFRONTENDLIVE_DEFAULT_WS_PORT 4551

#define GDBFRONTENDLIVE_VERSION {0, 2, 0, "beta", '\0'}
#define GDBFRONTENDLIVE_VERSION_STRING "v0.2.0-beta"

#define GDBFRONTENDLIVE_GF_VERSION {0, 11, 4, "git", '\0'}
#define GDBFRONTENDLIVE_GF_VERSION_STRING "v0.11.4-git"

char* gdbfrontendlive_host;
char* gdbfrontendlive_bind;

static void arg_handler_help(char* value);
static void arg_handler_version(char* value);
static void arg_handler_ws_port(char* value);
static void arg_handler_instance_user(char* username);
static void arg_handler_instance_group(char* groupname);
static void arg_handler_host_address(char* addr);
static void arg_handler_bind_address(char* addr);

static void* arguments[] = {
    "--help", "-h", GDBFRONTENDLIVE_ARG_WITHOUT_VALUE, arg_handler_help,
    "--version", "-v", GDBFRONTENDLIVE_ARG_WITHOUT_VALUE, arg_handler_version,
    "--host-address", "-H", GDBFRONTENDLIVE_ARG_WITH_VALUE, arg_handler_host_address,
    "--bind-address", "-l", GDBFRONTENDLIVE_ARG_WITH_VALUE, arg_handler_bind_address,
    "--ws-port", "-wsp", GDBFRONTENDLIVE_ARG_WITH_VALUE, arg_handler_ws_port,
    "--instance-user", "-u", GDBFRONTENDLIVE_ARG_WITH_VALUE, arg_handler_instance_user,
    "--instance-group", "-g", GDBFRONTENDLIVE_ARG_WITH_VALUE, arg_handler_instance_group,
    NULL
};

#endif