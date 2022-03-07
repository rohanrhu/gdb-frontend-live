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
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <grp.h>
#include <fcntl.h>

#include "../include/instance.h"

#include "../include/util.h"
#include "../include/uniqid.h"

char* gdbfrontendlive_instance_user_name = NULL;
uid_t gdbfrontendlive_instance_user_uid = 0;

char* gdbfrontendlive_instance_group_name = NULL;
gid_t gdbfrontendlive_instance_group_gid = 0;

gdbfrontendlive_instance_t* instances = NULL;
gdbfrontendlive_instance_t* instances_current = NULL;

static void sigabrt_handler() {
    printf("SIGABRT received.");
}

extern gdbfrontendlive_instance_t* gdbfrontendlive_instance_create() {
    gdbfrontendlive_instance_t* instance = malloc(sizeof(gdbfrontendlive_instance_t));
    instance->is_running = 0;
    instance->id = gdbfrontendlive_uniqid();
    instance->client = NULL;
    instance->workdir = NULL;
    instance->host_address = NULL;
    instance->bind_address = NULL;

    gdbfrontendlive_verbose("Instance created #%s\n", instance->id);
    
    return instance;
}

extern void gdbfrontendlive_instance_run(gdbfrontendlive_instance_t* instance) {
    signal(SIGCHLD, proc_exit_handler);

    char plugins_dir[200];
    realpath("gdb-frontend-plugins", plugins_dir);
    
    char workdir_base[100];
    char workdir[200];
    
    if (gdbfrontendlive_instance_user_uid) {
        sprintf(workdir_base, "/home/%s/gflive_instances", gdbfrontendlive_instance_user_name);
    } else {
        sprintf(workdir_base, "/root/gflive_instances");
    }

    sprintf(workdir, "%s/%s", workdir_base, instance->id);

    DIR* base = opendir(workdir_base);
    if (base) {
        closedir(base);
    } else if (errno == ENOENT) {
        mkdir(workdir_base, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP | S_IXUSR);
        chown(workdir_base, gdbfrontendlive_instance_user_uid, gdbfrontendlive_instance_group_gid);
    }

    mkdir(workdir, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP | S_IXUSR);
    chown(workdir, gdbfrontendlive_instance_user_uid, gdbfrontendlive_instance_group_gid);

    instance->workdir = malloc(strlen(workdir)+1);
    strcpy(instance->workdir, workdir);

    char mf[50];
    sprintf(mf, "%s/main.c", instance->workdir);

    char cp_templ[300];
    sprintf(cp_templ, "cp template/main.c %s", mf);

    system(cp_templ);
    chown(mf, gdbfrontendlive_instance_user_uid, gdbfrontendlive_instance_group_gid);
    chmod(mf, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP);

    instance->gfproc = malloc(sizeof(gdbfrontendlive_instance_gfproc_t));
    instance->gfproc->pid = 0;
    
    uid_t euid, ruid;
    
    char gf_executable[PATH_MAX+1];
    realpath("gdb-frontend/gdbfrontend", gf_executable);

    char* tmp = "/tmp/gdbfrontend-mmap-";
    int tmp_len = sizeof("/tmp/gdbfrontend-mmap-")-1;
    int id_len = strlen(instance->id);
    int len = tmp_len+id_len;
    char* mmap_path = malloc(len+1);
    mmap_path[len] = '\0';
    memcpy(mmap_path, tmp, tmp_len);
    memcpy(mmap_path+tmp_len, instance->id, id_len);

    int pagesize = getpagesize();

    int fd = open(mmap_path, O_CREAT | O_TRUNC | O_RDWR);
    chown(mmap_path, gdbfrontendlive_instance_user_uid, gdbfrontendlive_instance_group_gid);
    chmod(mmap_path, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP);

    for (int i=0; i < pagesize; i++) {
        lseek(fd, i, SEEK_CUR);
        write(fd, "\0", 1);
    }

    void* map = mmap(NULL, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_FILE | MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap() error.\n");
    }

    instance->prev = NULL;
    instance->next = NULL;

    if (!instances) {
        instances = instance;
    } else {
        instance->prev = instances_current;
        instances_current->next = instance;
    }
    
    instances_current = instance;

    pid_t ppid = getpid();
    instance->gfproc->pid = fork();

    if (instance->gfproc->pid == 0) {
        sigaction(SIGABRT, &(struct sigaction){sigabrt_handler}, NULL);
        
        if (instance->client) {
            close(instance->client->socket);
            close(instance->client->server_socket);
        }
        
        gdbfrontendlive_verbose("Instance[%s] Running GDBFrontend..\n", instance->id);

        setpgid(0, getpid());

        gid_t groups[] = {gdbfrontendlive_instance_group_gid};
        setgroups(sizeof(groups), groups);

        setgid(gdbfrontendlive_instance_group_gid);
        setuid(gdbfrontendlive_instance_user_uid);
        
        gdbfrontendlive_verbose("Running exectuable: %s\n", gf_executable);

        char* args[] = {
            gf_executable,
            "-l", (!instance->bind_address) ? "0.0.0.0": instance->bind_address,
            "-H", (!instance->host_address) ? "127.0.0.1": instance->host_address,
            "-p", "0",
            "-t", instance->id,
            "-w", instance->workdir,
            "-P", plugins_dir,
            "-D",
            NULL
        };

        execvp(gf_executable, args);

        printf("exec() error: %s\n", strerror(errno));
        exit(1);
    }
    
    gdbfrontendlive_verbose("GDBFrontend is spawned as PID: %d\n", instance->gfproc->pid);

    u_int16_t port = 0;

    while (!port) {
        port = ((int*)map)[0];
    }

    instance->http_port = port;

    gdbfrontendlive_verbose("HTTP Port (%u) is using by instance (%s)\n", instance->http_port, instance->id);

    munmap(map, pagesize);
    close(fd);

    instance->is_running = 1;
}

extern void gdbfrontendlive_instance_destroy(gdbfrontendlive_instance_t* instance) {
    gdbfrontendlive_verbose("Removing instance #%s\n", instance->id);

    if (instance->is_running && instance->gfproc->pid) {
        killpg(getpgid(instance->gfproc->pid), SIGKILL);
        kill(instance->gfproc->pid, SIGKILL);
    }

    pid_t pid = fork();

    if (pid == 0) {
        gid_t groups[] = {gdbfrontendlive_instance_group_gid};
        setgroups(sizeof(groups), groups);

        setgid(gdbfrontendlive_instance_group_gid);
        setuid(gdbfrontendlive_instance_user_uid);

        char command[300];
        sprintf(command, "tmux kill-session -t %s", instance->id);
        system(command);

        exit(0);
    }

    waitpid(pid, NULL, 0);

    if (instances == instance) {
        instances = NULL;
    }

    if (instance->prev) {
        instance->prev->next = instance->next;
    }

    if (instance->next) {
        instance->next->prev = instance->prev;
    }

    if (instance->workdir) {
        char rmrf[200];
        sprintf(rmrf, "rm -rf %s", instance->workdir);
        
        system(rmrf);
        
        if (instance->workdir) {
            free(instance->workdir);
            instance->workdir = NULL;
        }
    }

    if (instance->gfproc) {
        free(instance->gfproc);
        instance->gfproc = NULL;
    }
    
    free(instance);
}

extern gdbfrontendlive_instance_t* gdbfrontendlive_instance_getby_id(char* id) {
    gdbfrontendlive_instance_t* instance = instances;

    if (!instance) {
        return NULL;
    }

    loop:

    if (strcmp(instance->id, id) == 0) {
        return instance;
    }

    if (instance->next) {
        instance = instance->next;
        goto loop;
    }

    return NULL;
}

static void proc_exit_handler(int status) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}