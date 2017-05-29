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
//  Self test of this class

void
zm_proto_utils_test (bool verbose)
{
    printf (" * zm_proto_utils: ");

    //  @selftest
    //  Simple create/destroy test

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

    zm_proto_destroy (&self);
    //  @end
    printf ("OK\n");
}
