/*
 * gdb-frontend is a easy, flexible and extensionable gui debugger
 *
 * https://github.com/rohanrhu/gdb-frontend
 * https://oguzhaneroglu.com/projects/gdb-frontend/
 *
 * Licensed under GNU/GPLv3
 * Copyright (C) 2019, Oğuzhan Eroğlu (https://oguzhaneroglu.com/) <rohanrhu2@gmail.com>
*/

(function($) {
    var methods = {};

    methods.init = function (parameters) {
        var t_init = this;
        var $elements = $(this);

        if (typeof parameters == 'undefined') {
            parameters = {};
        }

        t_init.parameters = parameters;

        $elements.each(function () {
            var $GDBFrontendLive = $(this);

            $(window).off('GDBFrontendLive');
            $(document).off('GDBFrontendLive');
            $('body').off('GDBFrontendLive')
            
            $GDBFrontendLive.off('.GDBFrontendLive');
            $GDBFrontendLive.find('*').off('.GDBFrontendLive');

            var data = {};
            $GDBFrontendLive.data('GDBFrontendLive', data);
            data.$GDBFrontendLive = $GDBFrontendLive;

            data.$GDBFrontendLive_debugger = $GDBFrontendLive.find('.GDBFrontendLive_debugger');
            data.$GDBFrontendLive_debugger_iframe = data.$GDBFrontendLive_debugger.find('.GDBFrontendLive_debugger_iframe');
            data.$GDBFrontendLive_debugger_loading = data.$GDBFrontendLive_debugger.find('.GDBFrontendLive_debugger_loading');

            data.$GDBFrontendLive_menu = $GDBFrontendLive.find('.GDBFrontendLive_menu');
            data.$GDBFrontendLive_menu_share = data.$GDBFrontendLive_menu.find('.GDBFrontendLive_menu_share');
            data.$GDBFrontendLive_menu_share_input_rI = data.$GDBFrontendLive_menu.find('.GDBFrontendLive_menu_share_input_rI');

            data.http_protocol = t_init.parameters.http_protocol;
            data.http_path = t_init.parameters.http_path;
            data.ws_protocol = t_init.parameters.ws_protocol;
            data.host = t_init.parameters.host;
            data.port = t_init.parameters.port;
            data.instanceURLGenerator = t_init.parameters.instanceURLGenerator ? t_init.parameters.instanceURLGenerator: false;
            data.gfURLGenerator = t_init.parameters.gfURLGenerator ? t_init.parameters.gfURLGenerator: false;
            
            data.socket = false;

            data.instance = false;

            data.connect = function (parameters) {
                if (parameters === undefined) {
                    parameters = {};
                }

                data.socket = new WebSocket(data.ws_protocol+'://'+data.host+':'+data.port.toString());

                data.socket.onopen = function (event) {
                    data.$GDBFrontendLive_debugger_loading.css('display', 'flex');
                    
                    var message;
                    
                    try {
                        var url_parameters = !window.location.hash.length ? {}: JSON.parse(decodeURIComponent(window.location.hash).substr(1));
                    } catch (error) {
                        window.location = data.http_protocol+'://'+data.host+data.http_path;
                    }
                    
                    if (url_parameters.hasOwnProperty('id')) {
                        message = {action: 'instance_get', 'id': url_parameters.id};
                    } else {
                        message = {action: 'instance_create'};
                    }
                    
                    data.socket.send(JSON.stringify(message));
                };
                
                data.socket.onclose = function (event) {
                };
                
                data.socket.onmessage = function (event) {
                    try {
                        var message = JSON.parse(event.data);
                    } catch (error) {
                        GDBFrontendLive.showErrorMessage()
                        return;
                    }

                    if (!message.ok) {
                        GDBFrontendLive.showErrorMessage()
                        return;
                    }

                    if (message.event == 'instance_create_ret') {
                        data.instance = message.instance;

                        var instance_url;

                        if (!data.instanceURLGenerator) {
                            instance_url = data.http_protocol+'://'+data.host+data.http_path+'/#'+JSON.stringify({id: data.instance.id});
                        } else {
                            instance_url = data.instanceURLGenerator({instance: data.instance});
                        }
                        
                        var gf_url;

                        if (!data.gfURLGenerator) {
                            gf_url = 'http://'+data.host+':'+data.instance.http_port+'/terminal/';
                        } else {
                            gf_url = data.gfURLGenerator({instance: data.instance});
                        }
                        
                        data.$GDBFrontendLive_menu_share_input_rI.val(instance_url);
                        data.$GDBFrontendLive_debugger_iframe.attr('src', gf_url);
                        data.$GDBFrontendLive_menu_share.css('display', 'flex');
                        
                        data.$GDBFrontendLive_debugger_loading.hide();
                    } else if (message.event == 'instance_get_ret') {
                        data.instance = message.instance;
                        
                        var instance_url;

                        if (!data.instanceURLGenerator) {
                            instance_url = data.http_protocol+'://'+data.host+data.http_path+'/#'+JSON.stringify({id: data.instance.id});
                        } else {
                            instance_url = data.instanceURLGenerator({instance: data.instance});
                        }
                        
                        var gf_url;

                        if (!data.gfURLGenerator) {
                            gf_url = 'http://'+data.host+':'+data.instance.http_port+'/terminal/';
                        } else {
                            gf_url = data.gfURLGenerator({instance: data.instance});
                        }
                        
                        data.$GDBFrontendLive_menu_share_input_rI.val(instance_url);
                        data.$GDBFrontendLive_debugger_iframe.attr('src', gf_url);
                        data.$GDBFrontendLive_menu_share.css('display', 'flex');

                        data.$GDBFrontendLive_debugger_loading.hide();
                    }
                };
            };
            
            data.init = function (parameters) {
                if (parameters === undefined) {
                    parameters = {};
                }

                data.connect();
            };
            
            data.comply = function (parameters) {
                if (parameters === undefined) {
                    parameters = {};
                }
            };

            data.$GDBFrontendLive_menu_share_input_rI.on('focus.GDBFrontendLive', function (event) {
                data.$GDBFrontendLive_menu_share_input_rI.select();
            });

            $(window).on('resize.GDBFrontendLive', function (event) {
                data.comply({event: event});
            });

            data.init();
        });
    }

    $.fn.GDBFrontendLive = function(method) {
        if (methods[method]) {
            return methods[method].apply(this, Array.prototype.slice.call(arguments, 1));
        } else if (typeof method === 'object' || !method) {
            return methods.init.apply(this, arguments);
        } else {
            $.error('Method '+method+' does not exist on jQuery.GDBFrontendLive');
        }
    };

    $.fn.GDBFrontendLive.kvKey = function (key) {
        return 'GDBFrontendLive:'+key;
    };

    $.fn.GDBFrontendLive.event = function (event) {
        return 'GDBFrontendLive_'+event;
    };

    $.fn.GDBFrontendLive.events = function (events) {
        return events.map(function (e) {
            return $.fn.GDBFrontendLive.event(e);
        }).join(', ');
    };
})(jQuery);
