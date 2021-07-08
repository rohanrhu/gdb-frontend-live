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
#include <unistd.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <zlib.h>

#include "../lib/jsonic/jsonic.h"

#include "../include/websocket.h"

#include "../include/util.h"
#include "../include/instance.h"

gdbfrontendlive_websocket_t* gdbfrontendlive_websocket_init(int port) {
    gdbfrontendlive_websocket_t* ws = malloc(sizeof(gdbfrontendlive_websocket_t));
    ws->port = port;
    ws->host_address = NULL;
    ws->bind_address = NULL;

    return ws;
}

void gdbfrontendlive_websocket_listen(gdbfrontendlive_websocket_t* ws) {
    sigaction(SIGPIPE, &(struct sigaction){broken_pipe_handler}, NULL);
    
    int server_socket;
    int client_socket;
    int client_addr_len;

    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Socket error");
        exit(1);
    }

    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(ws->port);

    if (ws->bind_address) {
        serv_addr.sin_addr.s_addr = inet_addr(ws->bind_address);
    }

    if (bind(server_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind error.");
        exit(1);
    }

    listen(server_socket, 10);
    client_addr_len = sizeof(cli_addr);

    gdbfrontendlive_websocket_clients_t* clients = NULL;

    ws->clients = clients;

    gdbfrontendlive_verbose("Server is listening from 0.0.0.0:%d\n", ws->port);

    printf("\033[32mOpen gdb-frontend-live-plugin/index.html and share gdb-frontend-live-plugin directory.\033[0m\n");

    pthread_t client_thread;

    char str_addr[INET6_ADDRSTRLEN+1];

    gdbfrontendlive_websocket_clients_t* client = NULL;
    gdbfrontendlive_websocket_clients_t* current_client = NULL;

    for (;;) {
        client_socket = accept(server_socket, (struct sockaddr *) &cli_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept error");
            exit(1);
        }

        inet_ntop(AF_INET, (void*)&cli_addr.sin_addr, str_addr, INET_ADDRSTRLEN);
        
        gdbfrontendlive_verbose("New client: %s\n", str_addr);

        client = gdbfrontendlive_websocket_client_init();
        client->ws = ws;
        client->server_socket = server_socket;
        client->is_host = 0;
        client->instance = NULL;
        client->socket = client_socket;
        client->address = cli_addr.sin_addr.s_addr;

        if (!clients) {
            clients = client;
        } else {
            client->prev = current_client;
            current_client->next = client;
        }

        current_client = client;

        pthread_create(
            &client_thread,
            NULL,
            (void *) &client_handler,
            (void *) client
        );

        pthread_detach(client_thread);
    }
}

static void client_handler(gdbfrontendlive_websocket_clients_t* client) {
    sigaction(SIGPIPE, &(struct sigaction){broken_pipe_handler}, NULL);
    receive_http_packet(client);
}

static void receive_http_packet(gdbfrontendlive_websocket_clients_t* client) {
    char str_addr[INET6_ADDRSTRLEN+1];
    inet_ntop(AF_INET, (void*)&client->address, str_addr, INET_ADDRSTRLEN);
    
    ssize_t result;

    int is_cr = 0;
    int is_lf = 0;

    char header_buff[HTTP_HEADER_BUFF_SIZE+1];
    char prop_buff[HTTP_PROP_BUFF_SIZE+1];
    char val_buff[HTTP_PROP_BUFF_SIZE+1];

    int header_buff_i = 0;
    int prop_buff_i = 0;
    int val_buff_i = 0;

    enum GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE
    parser_state = GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_METHOD;

    char byte;

    RECEIVE_PACKET:

    result = recv(client->socket, &byte, 1, MSG_WAITALL);

    if (!result) {
        gdbfrontendlive_verbose("Client disconnected: %s\n", str_addr);
        return;
    }

    if (parser_state == GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_METHOD) {
        if (!is_cr) {
            if (byte == '\r') {
                is_cr = 1;
            }
        } else {
            if (byte == '\n') {
                parser_state = GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_PROP;
                goto RECEIVE_PACKET;
            } else {
                close(client->socket);
                return;
            }
        }
        
        *(header_buff+(header_buff_i++)) = byte;

        if (!is_lf) {
            goto RECEIVE_PACKET;
        }

        is_lf = 0;

        const char* expected_header = "GET / HTTP/1.1";

        if (strncmp(header_buff, expected_header, sizeof("GET / HTTP/1.1")-1) == 0) {
            goto RECEIVE_PACKET;
        } else {
            close(client->socket);
            return;
        }

        goto RECEIVE_PACKET;
    } else if (parser_state == GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_PROP) {
        if (byte == '\r') {
            parser_state = GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_END;
        } if (byte != ':') {
            *(prop_buff+(prop_buff_i++)) = byte;
        } else {
            parser_state = GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_SPACE;
            *(prop_buff+(prop_buff_i)) = '\0';
            prop_buff_i = 0;
        }
    } else if (parser_state == GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_SPACE) {
        if (byte != ' ') {
            close(client->socket);
            return;
        } else {
            parser_state = GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_VAL;
        }
    } else if (parser_state == GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_VAL) {
        if (byte == '\r') {
            parser_state = GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_CR;
            *(val_buff+(val_buff_i)) = '\0';
            val_buff_i = 0;
        } else {
            *(val_buff+(val_buff_i++)) = byte;
        }
    } else if (parser_state == GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_CR) {
        if (byte == '\n') {
            parser_state = GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_PROP;

            if (strcmp(prop_buff, "Sec-WebSocket-Key") == 0) {
                int len = strlen(val_buff);
                client->ws_key = malloc(len+1);
                strcpy(client->ws_key, val_buff);
            }
        } else {
            close(client->socket);
            return;
        }
    } else if (parser_state == GDBFRONTEND_WEBSOCKET_HTTP_HEADER_PARSER_STATE_END) {
        if (byte == '\n') {
            char response_buf[500];

            char concated[100];
            unsigned char hash[SHA_DIGEST_LENGTH+1];
            char* accept;

            sprintf(concated, "%s%s", client->ws_key, GDBFRONTENDLIVE_WEBSOCKET_GUID);

            hash[SHA_DIGEST_LENGTH] = '\0';

            SHA1(concated, strlen(concated), hash);
            
            accept = gdbfrontendlive_util_base64_encode(hash);
            
            sprintf(response_buf, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\nCompression: None\r\n\r\n", accept);

            send(client->socket, response_buf, strlen(response_buf), 0);
            
            goto RECEIVE_PACKET_WS;
        }
    }

    goto RECEIVE_PACKET;

    RECEIVE_PACKET_WS:

    receive_ws_packet(client);
}

static void receive_ws_packet(gdbfrontendlive_websocket_clients_t* client) {
    char str_addr[INET6_ADDRSTRLEN+1];
    inet_ntop(AF_INET, (void*)&client->address, str_addr, INET_ADDRSTRLEN);
    
    ssize_t result;

    uint16_t header0_16;
    uint64_t plen;
    uint16_t plen16;
    uint16_t plen64;
    uint8_t opcode;
    uint8_t is_masked;
    unsigned char mkey[4];

    char* req = NULL;
    char* res = NULL;

    RECEIVE_FRAME:

    result = recv(client->socket, &header0_16, 2, MSG_WAITALL);
    if (!result) {
        client_disconnected(client);
        return;
    }

    opcode = ((uint8_t)header0_16) & 0b00001111;

    is_masked = (*(((uint8_t*)(&header0_16))+1)) & -128;
    plen = (*(((uint8_t*)(&header0_16))+1)) & 127;
    
    if (plen == 126) {
        result = recv(client->socket, &plen16, 2, MSG_WAITALL);
        if (!result) {
            client_disconnected(client);
            return;
        }

        plen = ntohs(plen16);
    } else if (plen == 127) {
        result = recv(client->socket, &plen64, 8, MSG_WAITALL);
        if (!result) {
            client_disconnected(client);
            return;
        }

        plen = ntohs(plen64);
    }

    if (is_masked) {
        result = recv(client->socket, mkey, 4, MSG_WAITALL);
        if (!result) {
            client_disconnected(client);
            return;
        }
    }

    req = malloc(plen+1);
    req[plen] = '\0';

    result = recv(client->socket, req, plen, MSG_WAITALL);
    if (!result) {
        client_disconnected(client);
        return;
    }

    if (is_masked) {
        for (int i=0; i < plen; i++) {
            req[i] = req[i] ^ mkey[i%4];
        }
    }

    gdbfrontendlive_verbose("Raw Message: %s\n", req);

    jsonic_node_t* root = jsonic_get_root(req);
    jsonic_node_t* action = jsonic_object_get(req, root, "action");
    
    if (action->type == JSONIC_NONE) {
        goto RECEIVE_FRAME;
    }

    if (strcmp(action->val, "instance_create") == 0) {
        client->is_host = 1;
        client->instance = gdbfrontendlive_instance_create();
        client->instance->host_address = client->ws->host_address;
        client->instance->bind_address = client->ws->bind_address;
        client->instance->client = client;

        gdbfrontendlive_instance_run(client->instance);

        res = malloc(500);
        
        sprintf(
            res,
            "{"
                "\"ok\": true,"
                "\"event\": \"instance_create_ret\","
                "\"instance\": {"
                    "\"http_port\": %u,"
                    "\"id\": \"%s\""
                "}"
            "}",
            client->instance->http_port,
            client->instance->id
        );

        send_ws_packet(client, res);
    } else if (strcmp(action->val, "instance_get") == 0) {
        jsonic_node_t* id = jsonic_object_get(req, root, "id");

        if (id->type == JSONIC_NONE) {
            goto RECEIVE_FRAME;
        }

        client->instance = gdbfrontendlive_instance_getby_id(id->val);

        res = malloc(500);

        if (client->instance) {
            sprintf(
                res,
                "{"
                    "\"ok\": true,"
                    "\"event\": \"instance_get_ret\","
                    "\"instance\": {"
                        "\"http_port\": %u,"
                        "\"id\": \"%s\""
                    "}"
                "}",
                client->instance->http_port,
                client->instance->id
            );
        } else {
            sprintf(
                res,
                "{"
                    "\"ok\": true,"
                    "\"instance\": false"
                "}"
            );
        }

        send_ws_packet(client, res);
    }
    
    free(req);
    free(res);

    goto RECEIVE_FRAME;
}

static void client_disconnected(gdbfrontendlive_websocket_clients_t* client) {
    char str_addr[INET6_ADDRSTRLEN+1];
    inet_ntop(AF_INET, (void*)&client->address, str_addr, INET_ADDRSTRLEN);
    
    gdbfrontendlive_verbose("Client disconnected: %s\n", str_addr);
    gdbfrontendlive_websocket_client_free(client);
}

static void send_ws_packet(gdbfrontendlive_websocket_clients_t* client, char* message) {
    unsigned int message_len = strlen(message);

    if (message_len < 126) {
        gdbfrontendlive_websocket_packet_frame_len8_t message_frame;
        message_frame.fin_rsv_opcode = 0b10000001;
        message_frame.mlen8 = message_len;

        send(client->socket, &message_frame, sizeof(gdbfrontendlive_websocket_packet_frame_len8_t), 0);
    } else if (message_len < (1 << 16)) {
        gdbfrontendlive_websocket_packet_frame_len16_t message_frame;
        message_frame.fin_rsv_opcode = 0b10000001;
        message_frame.mlen8 = 126;
        message_frame.len16 = message_len;

        send(client->socket, &message_frame, sizeof(gdbfrontendlive_websocket_packet_frame_len16_t), 0);
    } else {
        gdbfrontendlive_websocket_packet_frame_len64_t message_frame;
        message_frame.fin_rsv_opcode = 0b10000001;
        message_frame.mlen8 = 127;
        message_frame.len64 = message_len;

        send(client->socket, &message_frame, sizeof(gdbfrontendlive_websocket_packet_frame_len64_t), 0);
    }
    
    send(client->socket, message, strlen(message), 0);
}

extern gdbfrontendlive_websocket_clients_t* gdbfrontendlive_websocket_client_init() {
    gdbfrontendlive_websocket_clients_t* client = malloc(sizeof(gdbfrontendlive_websocket_clients_t));
    client->socket = 0;
    client->ws_key = NULL;
    client->address = 0;
    client->next = NULL;
    client->prev = NULL;
    
    return client;
}

extern void gdbfrontendlive_websocket_client_free(gdbfrontendlive_websocket_clients_t* client) {
    if (client->next) {
        client->next->prev = client->prev;
    }
    
    if (client->prev) {
        client->prev->next = client->next;
    }

    if (client->is_host && client->instance) {
        gdbfrontendlive_instance_destroy(client->instance);
    }

    if (client->ws_key != NULL) {
        free(client->ws_key);
    }

    free(client);
}

void gdbfrontendlive_websocket_free(gdbfrontendlive_websocket_t* ws) {
    free(ws);
}

void broken_pipe_handler(int sig) {
    gdbfrontendlive_verbose("Broken pipe.\n");
}