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

#ifndef __GDBFRONTENDLIVE_WEBSOCKET_H__
#define __GDBFRONTENDLIVE_WEBSOCKET_H__

#include <bits/stdint-uintn.h>

#include "../include/util.h"
#include "../include/instance.h"

#define gdbfrontendlive_websocket_packet_frame_header \
    uint8_t fin_rsv_opcode; \
    uint8_t mlen8;
;

typedef struct gdbfrontendlive_websocket_packet_frame_len8 gdbfrontendlive_websocket_packet_frame_len8_t;
struct gdbfrontendlive_websocket_packet_frame_len8 {
    gdbfrontendlive_websocket_packet_frame_header
};

typedef struct gdbfrontendlive_websocket_packet_frame_len16 gdbfrontendlive_websocket_packet_frame_len16_t;
struct gdbfrontendlive_websocket_packet_frame_len16 {
    gdbfrontendlive_websocket_packet_frame_header
    uint16_t len16;
};

typedef struct gdbfrontendlive_websocket_packet_frame_len64 gdbfrontendlive_websocket_packet_frame_len64_t;
struct gdbfrontendlive_websocket_packet_frame_len64 {
    gdbfrontendlive_websocket_packet_frame_header
    uint64_t len64;
};

#define GDBFRONTENDLIVE_WEBSOCKET_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define HTTP_HEADER_BUFF_SIZE 700
#define HTTP_PROP_BUFF_SIZE 40
#define HTTP_VAL_BUFF_SIZE  50

enum GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE {
    GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_METHOD = 1,
    GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_PROP,
    GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_SPACE,
    GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_VAL,
    GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_CR,
    GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_END
};

enum GDBFRONTEND_WEBSOCKET_RESPONSE {
    GDBFRONTEND_WEBSOCKET_RESPONSE_INSTANCE_PORT = 1
};

typedef struct gdbfrontendlive_websocket gdbfrontendlive_websocket_t;
typedef struct gdbfrontendlive_websocket_clients gdbfrontendlive_websocket_clients_t;

struct gdbfrontendlive_websocket {
    int port;
    char* host_address;
    char* bind_address;
    gdbfrontendlive_websocket_clients_t* clients;
};

typedef struct gdbfrontendlive_instance gdbfrontendlive_instance_t;

struct gdbfrontendlive_websocket_clients {
    gdbfrontendlive_websocket_t* ws;
    int socket;
    int server_socket;
    int address;
    char* ws_key;
    int is_host;
    gdbfrontendlive_instance_t* instance;
    gdbfrontendlive_websocket_clients_t* prev;
    gdbfrontendlive_websocket_clients_t* next;
};

gdbfrontendlive_websocket_t* gdbfrontendlive_websocket_init(int port);
void gdbfrontendlive_websocket_listen(gdbfrontendlive_websocket_t* ws);
void gdbfrontendlive_websocket_free(gdbfrontendlive_websocket_t* ws);

extern gdbfrontendlive_websocket_clients_t* gdbfrontendlive_websocket_client_init();
extern void gdbfrontendlive_websocket_client_free(gdbfrontendlive_websocket_clients_t* client);

static void client_handler(gdbfrontendlive_websocket_clients_t* client);
static void receive_http_packet(gdbfrontendlive_websocket_clients_t* client);
static void receive_ws_packet(gdbfrontendlive_websocket_clients_t* client);
static void send_ws_packet(gdbfrontendlive_websocket_clients_t* client, char* message);
static void client_disconnected(gdbfrontendlive_websocket_clients_t* client);

void broken_pipe_handler(int sig);

#endif