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
#include <pwd.h>
#include <signal.h>

#include "../include/server.h"

#include "../include/util.h"
#include "../include/arg.h"
#include "../include/instance.h"
#include "../include/websocket.h"

char* gdbfrontendlive_host = NULL;
char* gdbfrontendlive_bind = NULL;

int gdbfrontendlive_ws_port = GDBFRONTENDLIVE_DEFAULT_WS_PORT;

static void sigabrt_handler() {
    printf("SIGABRT received.");
}

int main(int argc, char** argv) {
    printf("GDBFrontendLive %s\n", GDBFRONTENDLIVE_VERSION_STRING);

    sigaction(SIGABRT, &(struct sigaction){sigabrt_handler}, NULL);

    gdbfrontendlive_instance_user_uid = 0;
    gdbfrontendlive_instance_group_gid = 0;

    gdbfrontendlive_arg_t* arg;
    arg = gdbfrontendlive_arg_init(argv, argc, (void**)arguments);
    gdbfrontendlive_arg_handle(arg);

    if (!gdbfrontendlive_instance_user_uid) {
        printf("\033[31m[Error] You must specify instance user. (Example: --instance-user=USERNAME)\033[0m\n");
        exit(1);
    }

    if (!gdbfrontendlive_instance_group_gid) {
        gdbfrontendlive_instance_group_gid = gdbfrontendlive_instance_user_uid;
    }

    if (!gdbfrontendlive_host) {
        gdbfrontendlive_host = malloc(10);
        sprintf(gdbfrontendlive_host, "127.0.0.1");
    }
    
    if (!gdbfrontendlive_bind) {
        gdbfrontendlive_bind = malloc(8);
        sprintf(gdbfrontendlive_host, "0.0.0.0");
    }

    gdbfrontendlive_websocket_t* ws;
    ws = gdbfrontendlive_websocket_init(gdbfrontendlive_ws_port);
    ws->host_address = gdbfrontendlive_host;
    ws->bind_address = gdbfrontendlive_bind;

    gdbfrontendlive_websocket_listen(ws);

    return 0;
}

static void arg_handler_help(char* value) {
    printf("Usage: gdbfrontendlive [options]\n");
    printf("Options:\n");
    printf("\t--help, -h:\t\t\t\tShows this help text.\n");
    printf("\t--version, -v:\t\t\t\tShows GDBFrontendLive and GDBFrontend versions.\n");
    printf("\t--host-address=IP, -H IP:\t\tSpecifies bind address.\n");
    printf("\t--bind-address=IP, -l IP:\t\tSpecifies host address. (Default is 127.0.0.1)\n");
    printf("\t--ws-port=PORT, -wsp PORT:\t\tSpecifies GDBFrontend websocket server's port.\n");
    printf("\t--instance-user=USER, -u USER:\t\tSets the user that runs instances.\n");
    printf("\t--instance-group=GROUP, -g GROUP:\tSets the group that runs instances.\n");
    exit(0);
}

static void arg_handler_version(char* value) {
    printf("GDBFrontend: %s\n", GDBFRONTENDLIVE_GF_VERSION_STRING);
    exit(0);
}

static void arg_handler_host_address(char* addr) {
    gdbfrontendlive_host = addr;
}

static void arg_handler_bind_address(char* addr) {
    gdbfrontendlive_bind = addr;
}

static void arg_handler_ws_port(char* value) {
    gdbfrontendlive_ws_port = atoi(value);
}

static void arg_handler_instance_user(char* username) {
    gdbfrontendlive_verbose("Using user for instances: %s\n", username);

    gdbfrontendlive_instance_user_name = username;

    struct passwd* pwd = getpwnam(username);
    gdbfrontendlive_instance_user_uid = pwd->pw_uid;
}

static void arg_handler_instance_group(char* groupname) {
    gdbfrontendlive_verbose("Using group for instances: %s\n", groupname);

    gdbfrontendlive_instance_group_name = groupname;

    struct group* pwd = getgrnam(groupname);
    gdbfrontendlive_instance_group_gid = pwd->gr_gid;
}