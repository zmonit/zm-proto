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

static char *ZARG_PARAM_EMPTY = "";

struct _zargs_t {
    char *progname;
    zlist_t *arguments;
    zhash_t *parameters;
};

typedef struct _zargs_t zargs_t;

zargs_t *
zargs_new (int argc, char **argv) {
    zargs_t *self = (zargs_t*) zmalloc (sizeof (zargs_t));
    assert (self);

    self->progname = argv [0];
    assert (self->progname);
    self->arguments = zlist_new ();
    assert (self->arguments);
    self->parameters = zhash_new ();
    assert (self->parameters);

    int idx = 1;
    while (argv [idx]) {
        if (argv [idx][0] != '-')
            zlist_append (self->arguments, argv [idx]);
        else {
            if (argv [idx+1] && argv [idx+1][0] != '-') {
                zhash_insert (self->parameters, argv [idx], argv [idx+1]);
                idx ++;
            }
            else {
                zhash_insert (self->parameters, argv [idx], ZARG_PARAM_EMPTY);
            }
        }
        idx ++;
    }

    return self;
}

void
zargs_destroy (zargs_t **self_p) {
    assert (self_p);
    if (*self_p) {
        zargs_t *self = *self_p;
        zlist_destroy (&self->arguments);
        zhash_destroy (&self->parameters);
        *self_p = NULL;
    }
}

const char *
zargs_progname (zargs_t *self) {
    assert (self);
    return self->progname;
}

size_t
zargs_arguments (zargs_t *self) {
    assert (self);
    return zlist_size (self->arguments);
}

const char *
zargs_first (zargs_t *self) {
    assert (self);
    return (const char*) zlist_first (self->arguments);
}

const char *
zargs_next (zargs_t *self) {
    assert (self);
    return (const char*) zlist_next (self->arguments);
}

const char *
zargs_param_first (zargs_t *self) {
    assert (self);
    return (const char*) zhash_first (self->parameters);
}

const char *
zargs_param_next (zargs_t *self) {
    assert (self);
    return (const char*) zhash_next (self->parameters);
}

const char *
zargs_param_name (zargs_t *self) {
    assert (self);
    return (const char*) zhash_cursor (self->parameters);
}

const char *
zargs_param_lookup (zargs_t *self, const char *key1, const char *key2) {
    assert (self);
    const char *ret = (const char*) zhash_lookup (self->parameters, key1);
    if (!ret && key2)
        ret = (const char*) zhash_lookup (self->parameters, key2);
    return ret;
}

size_t
zargs_size (zargs_t *self) {
    assert (self);
    return zlist_size (self->arguments);
}

bool
zargs_has_help (zargs_t *self) {
    return zargs_param_lookup (self, "--help", "-h");
}

void
zargs_print (zargs_t *self) {
    assert (self);
    printf ("%s ", self->progname);
    for (const char *pvalue = zargs_param_first (self);
                    pvalue != NULL;
                    pvalue = zargs_param_next (self)) {
        const char *pname = zargs_param_name (self);
        if (pvalue == ZARG_PARAM_EMPTY)
            printf ("%s ", pname);
        else
            printf ("%s %s ", pname, pvalue);
    }
    for (const char *arg = zargs_first (self);
                     arg != NULL;
                     arg = zargs_next (self)) {
        printf ("%s ", arg);
    }
    puts ("");
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
    if (zargs_param_lookup (args, "--help", "-h"))
        verbose = true;

    const char *foo = zargs_param_lookup (args, "--endpoint", "-e");
    if (foo && foo == ZARG_PARAM_EMPTY) {
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

        if (zargs_size (args) != 6) {
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

        if (zargs_size (args) != 7) {
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

        if (zargs_size (args) != 3) {
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
