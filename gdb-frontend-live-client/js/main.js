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
GDBFrontendLive.components = {};
 
(function () {

GDBFrontendLive.showErrorMessage = function () {
    GDBFrontendLive.components.messageBox.open({text: 'An error occured. Please submit a report on <a href="https://github.com/rohanrhu/gdb-frontend-live/" target="_blank">Github Issues</a>.'});
};
    
$(document).ready(function (event) {
    var $messageBox = MessageBox.new();
    $messageBox.appendTo($('body'));
    GDBFrontendLive.components.messageBox = $messageBox.data().MessageBox;
    
    var $gdbFrontendLive = $('.GDBFrontendLive');
    $gdbFrontendLive.GDBFrontendLive({
        http_protocol: GDBFrontendLive.config.http_protocol,
        http_path: GDBFrontendLive.config.http_path,
        ws_protocol: GDBFrontendLive.config.ws_protocol,
        host: GDBFrontendLive.config.host,
        port: GDBFrontendLive.config.port,
        instanceURLGenerator: GDBFrontendLive.config.instanceURLGenerator,
        gfURLGenerator: GDBFrontendLive.config.gfURLGenerator
    });
    GDBFrontendLive.components.gdbFrontendLive = $gdbFrontendLive.data().GDBFrontendLive;
});

})();
