/*  =========================================================================
    zmpub - Helper tool to publish zm-proto messages on a stream

    Copyright (c) the Contributors as noted in the AUTHORS file.  This file is part
    of zmon.it, the fast and scalable monitoring system.                           
                                                                                   
    This Source Code Form is subject to the terms of the Mozilla Public License, v.
    2.0. If a copy of the MPL was not distributed with this file, You can obtain   
    one at http://mozilla.org/MPL/2.0/.                                            
    =========================================================================
*/

/*
@header
    zmpub - Helper tool to publish zm-proto messages on a stream
@discuss
@end
*/

#include "zm_proto_classes.h"

static bool
s_atou64 (const char* s, uint64_t *res) {

    assert (s);

    char *endptr = NULL;
    uint64_t ret = (uint64_t) strtoul (s, &endptr, 10);

    if (!endptr) {
        *res = ret;
        return true;
    }
    return false;
}

int main (int argc, char *argv [])
{
    bool verbose = false;
    int argn;
    char *endpoint = "ipc://@/malamute";
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("zmpub [options] stream [arguments ...]");
            puts ("  --endpoint / -e        malamute endpoint (defaults to ipc://@/malamute)");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --help / -h            this information");
            puts ("");
            puts ("                metric device ttl type value unit");
            return 0;
        }
        else
        if (streq (argv [argn], "--verbose")
        ||  streq (argv [argn], "-v"))
            verbose = true;
        else
        if (streq (argv [argn], "--endpoint")
        ||  streq (argv [argn], "-e")) {
            if (argn + 1 == argc) {
                zsys_error ("value after --endpoint / -e expected", "");
                exit (EXIT_FAILURE);
            }
            endpoint = argv [argn+1];
            argn ++;
        }
        else {
            printf ("Unknown option: %s\n", argv [argn]);
            return 1;
        }
    }

    if (!argv [argn]) {
        zsys_error ("No stream defined, use one of metric/alert/device");
        exit (EXIT_FAILURE);
    }

    char *stream = argv [argn];
    if (streq (stream, "metric")) {
        // FIXME: this check expects --ext arguments to be placed before, this can be fixed by
        //        reordering of argv like getopt does
        if (argc - argn != 5) {
            zsys_error ("metric expect 5 arguments exactly device ttl type value unit");
            exit (EXIT_FAILURE);
        }
    }
    else {
        zsys_error ("No code for stream %s", stream);
        exit (EXIT_FAILURE);
    }

    //  Insert main code here
    if (verbose)
        zsys_info ("zmpub - Helper tool to publish zm-proto messages on a stream");

    // connect to malamute
    mlm_client_t *client = mlm_client_new ();
    assert (client);

    zuuid_t *uuid = zuuid_new ();
    char *address = zsys_sprintf ("zmpub-%s", zuuid_str_canonical (uuid));
    zuuid_destroy (&uuid);
    int r = mlm_client_connect (client, endpoint, 3000, address);
    zstr_free (&address);

    zm_proto_t *msg = zm_proto_new ();
    if (streq (stream, "metric")) {
        char *device = argv [argn+1];
        uint64_t ttl;
        bool s = s_atou64 (argv [argn+2], &ttl);
        if (!s) {
            zsys_debug ("Failed to parse %s as a number", argv [argn+2]);
            exit (EXIT_FAILURE);
        }
        char *type = argv [argn+3];
        char *value = argv [argn+4];
        char *unit = argv [argn+5];

        zm_proto_set_id (msg, ZM_PROTO_METRIC);
        zuuid_t *uuid = zuuid_new ();
        r = zuuid_set_str (uuid, device);
        if (r == -1) {
            zsys_error ("given device %s is not valid uuid!", device);
            zuuid_destroy (&uuid);
            exit (EXIT_FAILURE);
        }
        zm_proto_set_device (msg, uuid);

        char *subject = zsys_sprintf ("%s@%s", type, device);

        zuuid_destroy (&uuid);
        zm_proto_set_ttl (msg, ttl);
        zm_proto_set_type (msg, type);
        zm_proto_set_value (msg, value);
        zm_proto_set_unit (msg, unit);

        zm_proto_msend (msg, client, subject);
        zstr_free (&subject);

    }
    // to give background threads the time to send it
    zclock_sleep (2000);

    zm_proto_destroy (&msg);
    mlm_client_destroy (&client);
    return 0;
}
