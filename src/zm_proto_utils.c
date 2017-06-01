/*  =========================================================================
    zm_proto_utils - Extra helpers for zmon.it messages

    Copyright (c) the Contributors as noted in the AUTHORS file.  This file is part
    of zmon.it, the fast and scalable monitoring system.

    This Source Code Form is subject to the terms of the Mozilla Public License, v.
    2.0. If a copy of the MPL was not distributed with this file, You can obtain
    one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

/*
@header
    zm_proto_utils - Extra helpers for zmon.it messages
@discuss
@end
*/

#include "zm_proto_classes.h"

//  --------------------------------------------------------------------------
//  Get string item from ext attribute

const char *
zm_proto_ext_string (zm_proto_t *self, const char *name, const char *dflt)
{
    assert (self);
    zhash_t *ext = zm_proto_ext (self);
    if (! ext) return dflt;
    const char *value = (const char *) zhash_lookup (ext, name);
    return value ? value : dflt;
}

//  --------------------------------------------------------------------------
//  Set string item in ext attribute

void zm_proto_ext_set_string (zm_proto_t *self, const char *name, const char *value)
{
    assert (self);
    zhash_t *ext = zm_proto_ext (self);
    if (! ext) {
        ext = zhash_new ();
        zhash_autofree (ext);
        zhash_update (ext, name, (void *) value);
        zm_proto_set_ext (self, &ext);
    } else {
        zhash_update (ext, name, (void *) value);
    }
}

//  --------------------------------------------------------------------------
//  Get long int item from ext attribute

long int
zm_proto_ext_int (zm_proto_t *self, const char *name, long int dflt)
{
    const char *value = zm_proto_ext_string (self, name, NULL);
    if (! value) return dflt;
    char *errorptr;
    errno = 0;
    long int result = strtol (value, &errorptr, 10);
    if (value == errorptr) {
        // error: no numbers at all
        return dflt;
    }
    if (errorptr && errorptr[0]) {
        // error: errorptr points to error character
        return dflt;
    }
    return result;
}

//  --------------------------------------------------------------------------
//  Set long int item in ext attribute

void
zm_proto_ext_set_int (zm_proto_t *self, const char *name, long int value)
{
    int size = snprintf(NULL, 0, "%li", value);

    char buffer [size+1];
    snprintf(buffer, size + 1, "%li", value);
    zm_proto_ext_set_string (self, name, buffer);
}

//  --------------------------------------------------------------------------
//  Get double item from ext attribute

double
zm_proto_ext_double (zm_proto_t *self, const char *name, double dflt)
{
    const char *value = zm_proto_ext_string (self, name, NULL);
    if (! value) return dflt;
    char *errorptr;
    errno = 0;
    double result = strtod (value, &errorptr);
    if (value == errorptr) {
        // error: no numbers at all
        return dflt;
    }
    if (errorptr && errorptr[0]) {
        // error: errorptr points to error character
        return dflt;
    }
    return result;
}

//  --------------------------------------------------------------------------
//  Set double item in ext attribute

void
zm_proto_ext_set_double (zm_proto_t *self, const char *name, double value)
{
    int size = snprintf(NULL, 0, "%f", value);

    char buffer [size+1];
    snprintf(buffer, size + 1, "%f", value);
    zm_proto_ext_set_string (self, name, buffer);
}

//  --------------------------------------------------------------------------
//  Converts zmsg to zm_proto, this is for compatibility with zproto v1 codec
zm_proto_t *
zm_proto_decode (zmsg_t **message_p)
{
    if (! message_p || ! *message_p) return NULL;

    zmsg_t *message = *message_p;
    zm_proto_t *self = zm_proto_new ();

    if (zm_proto_recv (self, message) == 0) {
        zmsg_destroy (message_p);
        return self;
    } else {
        zm_proto_destroy (&self);
        return NULL;
    }
}

static zm_proto_t *
s_zm_proto_encode_common (
    const char *device,
    int64_t time,
    int32_t ttl,
    zhash_t *ext
)
{
    zm_proto_t *self = zm_proto_new ();
    assert (self);

    zm_proto_set_device (self, device);
    zm_proto_set_time (self, time);
    zm_proto_set_ttl (self, ttl);
    if (ext) {
        zhash_t *d = zhash_dup (ext);
        zm_proto_set_ext (self, &d);
    }
    return self;
}

//  --------------------------------------------------------------------------
//  v1 codec compatibility function, creates zm_proto_t with metric and encode it to zmsg_t

zmsg_t *
zm_proto_encode_metric (
    const char *device,
    int64_t time,
    int32_t ttl,
    zhash_t *ext,
    const char *type,
    const char *value,
    const char *units
)
{
    if (!device || !type || !value) return NULL;

    zm_proto_t *self = s_zm_proto_encode_common (device, time, ttl, ext);
    zm_proto_set_id (self, ZM_PROTO_METRIC);
    zm_proto_set_type (self, type);
    zm_proto_set_value (self, value);
    if (units) {
        zm_proto_set_unit (self, units);
    } else {
        zm_proto_set_unit (self, "");
    }

    zmsg_t *output = zmsg_new ();
    assert (output);
    if (zm_proto_send (self, output) == 0) {
        zm_proto_destroy (&self);
        return output;
    } else {
        zm_proto_destroy (&self);
        zmsg_destroy (&output);
        return NULL;
    }
}

//  --------------------------------------------------------------------------
//  v1 codec compatibility function, creates zm_proto_t with device and encode it to zmsg_t

zmsg_t *
zm_proto_encode_device (
    const char *device,
    int64_t time,
    int32_t ttl,
    zhash_t *ext
)
{
    if (!device) return NULL;

    zm_proto_t *self = s_zm_proto_encode_common (device, time, ttl, ext);
    zm_proto_set_id (self, ZM_PROTO_DEVICE);

    zmsg_t *output = zmsg_new ();
    assert (output);
    if (zm_proto_send (self, output) == 0) {
        zm_proto_destroy (&self);
        return output;
    } else {
        zm_proto_destroy (&self);
        zmsg_destroy (&output);
        return NULL;
    }
}

//  --------------------------------------------------------------------------
//  v1 codec compatibility function, creates zm_proto_t with device and encode it to zmsg_t

zmsg_t *
zm_proto_encode_alert (
    const char *device,
    int64_t time,
    int32_t ttl,
    zhash_t *ext,
    const char *rule,
    char severity,
    const char *description
)
{
    if (!device || !rule) return NULL;

    zm_proto_t *self = s_zm_proto_encode_common (device, time, ttl, ext);
    zm_proto_set_id (self, ZM_PROTO_ALERT);
    zm_proto_set_rule (self, rule);
    zm_proto_set_severity (self, severity);
    zm_proto_set_description (self, description);

    zmsg_t *output = zmsg_new ();
    assert (output);
    if (zm_proto_send (self, output) == 0) {
        zm_proto_destroy (&self);
        return output;
    } else {
        zm_proto_destroy (&self);
        zmsg_destroy (&output);
        return NULL;
    }
}

//  --------------------------------------------------------------------------
//  Self test of this class

void
zm_proto_utils_test (bool verbose)
{
    printf (" * zm_proto_utils: ");

    //  @selftest
    {
        // create/destroy & set/get ext test
        zm_proto_t *self = zm_proto_new ();
        zm_proto_set_id (self, ZM_PROTO_METRIC);
        assert (self);

        assert (streq (zm_proto_ext_string (self, "something", "value"), "value"));

        zm_proto_ext_set_string (self, "something", "nothing");
        assert (streq (zm_proto_ext_string (self, "something", "value"), "nothing"));

        zm_proto_ext_set_int (self, "num", -45);
        assert (zm_proto_ext_int (self, "num", 0) == -45);

        zm_proto_ext_set_double (self, "pi", 3.14159);
        assert (zm_proto_ext_double (self, "pi", 0) == 3.14159);

        zm_proto_ext_set_string (self, "errnum", "3invalid");
        assert (zm_proto_ext_int (self, "errnum", -1) == -1);
        assert (zm_proto_ext_double (self, "errnum", -1.0) == -1.0);

        zm_proto_print (self);
        zm_proto_destroy (&self);
    }
    {
        // Test encode/decode
        zhash_t *ext = zhash_new();
        zhash_autofree (ext);
        zhash_insert (ext, "item", (void *)"value");

        zmsg_t *msg = zm_proto_encode_metric ("mydevice", 10, 20, ext, "temperature", "20.3", "C");
        assert (msg);

        zm_proto_t *self = zm_proto_decode (&msg);
        assert (self);
        assert (streq (zm_proto_device (self),"mydevice"));
        assert (streq (zm_proto_ext_string (self, "item", ""),"value"));

        zm_proto_destroy (&self);
        zmsg_destroy (&msg);
        zhash_destroy (&ext);
    }
    //  @end
    printf ("OK\n");
}
