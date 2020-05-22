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

#ifndef __GDBFRONTENDLIVE_UTIL_H__
#define __GDBFRONTENDLIVE_UTIL_H__

extern void gdbfrontendlive_verbose(char* text, ...);
extern void gdbfrontendlive_set_verbose(int _is_verbose);

extern char* gdbfrontendlive_util_base64_encode(char* str);
extern char* gdbfrontendlive_util_base64_decode(char* str);

#endif