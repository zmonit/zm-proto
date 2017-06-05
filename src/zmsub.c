/*  =========================================================================
    zmsub - Helper tool to subscribe to zm-proto messages from a stream

    Copyright (c) the Contributors as noted in the AUTHORS file.  This file is part
    of zmon.it, the fast and scalable monitoring system.                           
                                                                                   
    This Source Code Form is subject to the terms of the Mozilla Public License, v.
    2.0. If a copy of the MPL was not distributed with this file, You can obtain   
    one at http://mozilla.org/MPL/2.0/.                                            
    =========================================================================
*/

/*
@header
    zmsub - Helper tool to subscribe to zm-proto messages from a stream
@discuss
@end
*/

#include "zm_proto_classes.h"

static bool
strcaseq (const char *s1, const char *s2) {
    return ! strcasecmp (s1, s2);
}

int main (int argc, char *argv [])
{
    zsys_init ();
    bool verbose = false;
    const char *endpoint = "ipc://@/malamute";

    zargs_t *args = zargs_new (argc, argv);

    if (zargs_has_help (args)) {
        puts ("zmpub [options] stream [arguments ...]");
        puts ("  --endpoint / -e        malamute endpoint (defaults to ipc://@/malamute)");
        puts ("  --verbose / -v         verbose test output");
        puts ("  --help / -h            this information");
        puts ("");
        puts ("                alert device ttl rule severity description");
        puts ("                device device ttl");
        puts ("                metric device ttl type value unit");
        return 0;
    }
    if (zargs_has_help (args))
        verbose = true;

    const char *foo = zargs_param_lookupx (args, "--endpoint", "-e", NULL);
    if (zargs_param_empty (foo)) {
        zsys_error ("endpoint value mandatory");
        exit (EXIT_FAILURE);
    }

    //  Insert main code here
    if (verbose)
        zsys_info ("zmsub - Helper tool to subscribe to zm-proto messages from a stream");

    const char *stream = zargs_first (args);
    if (!stream) {
        zsys_error ("No stream defined, use one of metric/alert/device");
        exit (EXIT_FAILURE);
    }

    const char *mlmstream = NULL;
    if (strcaseq (stream, "alert"))
        mlmstream = ZM_PROTO_ALERT_STREAM;
    else
    if (strcaseq (stream, "metric"))
        mlmstream = ZM_PROTO_METRIC_STREAM;
    else
    if (strcaseq (stream, "device"))
        mlmstream = ZM_PROTO_DEVICE_STREAM;
    else {
        zsys_error ("Unknown stream name '%s', use alert/device/metric", stream);
        exit (EXIT_FAILURE);
    }

    mlm_client_t *client = mlm_client_new ();
    assert (client);

    zuuid_t *uuid = zuuid_new ();
    char *address = zsys_sprintf ("zmsub-%s", zuuid_str_canonical (uuid));
    zuuid_destroy (&uuid);
    int r = mlm_client_connect (client, endpoint, 3000, address);
    assert (r != -1);
    zstr_free (&address);
    r = mlm_client_set_consumer (client, mlmstream, ".*");
    assert (r != -1);

    zm_proto_t *msg = zm_proto_new ();
    while (!zsys_interrupted) {
	zmsg_t *zmsg = mlm_client_recv (client);
        zm_proto_recv (msg, zmsg);
	zm_proto_print (msg);
	zmsg_destroy (&zmsg);
    }
    zm_proto_destroy (&msg);
    mlm_client_destroy (&client);
    zargs_destroy (&args);

    return 0;
}
