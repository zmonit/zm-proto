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

    if ((endptr - s) != strlen (s))
        return false;

    *res = ret;
    return true;
}

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
        zsys_info ("zmpub - Helper tool to publish zm-proto messages on a stream");


    const char *stream = zargs_first (args);
    if (!stream) {
        zsys_error ("No stream defined, use one of metric/alert/device");
        exit (EXIT_FAILURE);
    }

    if (!strcaseq (stream, "alert") && !strcaseq (stream, "device") && !strcaseq (stream, "metric")) {
        zsys_error ("Unknown stream name '%s', use alert/device/metric", stream);
        exit (EXIT_FAILURE);
    }

    zm_proto_t *msg = zm_proto_new ();
    char *subject = NULL;
    if (strcaseq (stream, "metric")) {

        if (zargs_arguments (args) != 6) {
            zsys_error ("Not enough arguments for metric command");
            exit (EXIT_FAILURE);
        }

        const char *device = zargs_next (args);
        uint64_t ttl;
        const char *foo = zargs_next (args);
        bool s = s_atou64 (foo, &ttl);
        if (!s) {
            zsys_debug ("Failed to parse %s as a number", foo);
            exit (EXIT_FAILURE);
        }
        const char *type = zargs_next (args);
        const char *value = zargs_next (args);
        const char *unit = zargs_next (args);

        zm_proto_set_id (msg, ZM_PROTO_METRIC);
        zm_proto_set_device (msg, device);

        subject = zsys_sprintf ("%s@%s", type, device);

        zm_proto_set_ttl (msg, ttl);
        zm_proto_set_type (msg, type);
        zm_proto_set_value (msg, value);
        zm_proto_set_unit (msg, unit);
    }
    else
    if (strcaseq (stream, "alert")) {

        if (zargs_arguments (args) != 7) {
            zsys_error ("Not enough arguments for alert command");
            exit (EXIT_FAILURE);
        }

        const char *device = zargs_next (args);
        uint64_t ttl;
        const char *foo = zargs_next (args);
        bool s = s_atou64 (foo, &ttl);
        if (!s) {
            zsys_debug ("Failed to parse %s as a number", foo);
            exit (EXIT_FAILURE);
        }
        const char *rule = zargs_next (args);

        char severity = 0;
        foo = zargs_next (args);
        if (strcaseq (foo, "critical")) {
            severity = 1;
        }

        const char *description = zargs_next (args);

        zm_proto_set_id (msg, ZM_PROTO_ALERT);
        zm_proto_set_device (msg, device);

        subject = zsys_sprintf ("%s@%s", rule, device);

        zm_proto_set_ttl (msg, ttl);
        zm_proto_set_rule (msg, rule);
        zm_proto_set_severity (msg, severity);
        zm_proto_set_description (msg, description);
    }
    else
    if (strcaseq (stream, "device")) {

        if (zargs_arguments (args) != 3) {
            zsys_error ("Not enough arguments for device command");
            exit (EXIT_FAILURE);
        }

        const char *device = zargs_next (args);
        uint64_t ttl;
        const char *foo = zargs_next (args);
        bool s = s_atou64 (foo, &ttl);
        if (!s) {
            zsys_debug ("Failed to parse %s as a number", foo);
            exit (EXIT_FAILURE);
        }

        subject = strdup (device);

        zm_proto_set_id (msg, ZM_PROTO_DEVICE);
        zm_proto_set_device (msg, device);
        zm_proto_set_ttl (msg, ttl);
    }
    // to give background threads the time to send it
    // connect to malamute
    mlm_client_t *client = mlm_client_new ();
    assert (client);

    zuuid_t *uuid = zuuid_new ();
    char *address = zsys_sprintf ("zmpub-%s", zuuid_str_canonical (uuid));
    zuuid_destroy (&uuid);
    int r = mlm_client_connect (client, endpoint, 3000, address);
    assert (r != -1);
    zstr_free (&address);

    zmsg_t *zmsg = zmsg_new ();
    zm_proto_send (msg, zmsg);
    mlm_client_send (client, subject, &zmsg);
    zclock_sleep (500);

    zm_proto_destroy (&msg);
    zstr_free (&subject);
    zm_proto_destroy (&msg);
    mlm_client_destroy (&client);

    zargs_print (args);
    zargs_destroy (&args);
    return 0;
}
