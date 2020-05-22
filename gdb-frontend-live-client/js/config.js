/*
 * Client for GDBFrontendLive.
 *
 * https://github.com/rohanrhu/gdb-frontend-live-client
 * https://oguzhaneroglu.com/projects/gdb-frontend-live-client/
 *
 * Licensed under GNU/GPLv3
 * Copyright (C) 2020, Oğuzhan Eroğlu (https://oguzhaneroglu.com/) <rohanrhu2@gmail.com>
 *
 */

GDBFrontendLive;
 
GDBFrontendLive.config = {};
GDBFrontendLive.config.http_protocol = 'http';
GDBFrontendLive.config.http_path = '/gdb-frontend-live-client';
GDBFrontendLive.config.ws_protocol = 'ws';
GDBFrontendLive.config.host = '192.168.1.102';
GDBFrontendLive.config.port = 4551;

GDBFrontendLive.config.instanceURLGenerator = function (parameters) {
    return GDBFrontendLive.config.http_protocol+'://'+GDBFrontendLive.config.host+GDBFrontendLive.config.http_path+'/#'+JSON.stringify({id: parameters.instance.id});
};

GDBFrontendLive.config.gfURLGenerator = function (parameters) {
    return 'http://'+GDBFrontendLive.config.host+':'+parameters.instance.http_port+'/terminal/';
};