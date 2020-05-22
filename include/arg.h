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

#ifndef __GDBFRONTENDLIVE_ARG_H__
#define __GDBFRONTENDLIVE_ARG_H__

enum {
    GDBFRONTENDLIVE_ARGUMENT_FULL,
    GDBFRONTENDLIVE_ARGUMENT_SHORT,
    GDBFRONTENDLIVE_ARGUMENT_IS_EXPECT_VALUE,
    GDBFRONTENDLIVE_ARGUMENT_FUNCTION
};

#define GDBFRONTENDLIVE_ARG_WITH_VALUE (void*) 1
#define GDBFRONTENDLIVE_ARG_WITHOUT_VALUE (void*) 0

struct gdbfrontendlive_arg {
    char** argv;
    int argc;
    void** arguments;
};
typedef struct gdbfrontendlive_arg gdbfrontendlive_arg_t;

typedef void (*gdbfrontendlive_arg_function_t)(char*);

extern gdbfrontendlive_arg_t* gdbfrontendlive_arg_init(char** argv, int argc, void** arguments);
extern void gdbfrontendlive_arg_handle(gdbfrontendlive_arg_t* arg);

#endif