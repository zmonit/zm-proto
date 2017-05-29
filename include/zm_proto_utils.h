/*  =========================================================================
    zm_proto_utils - Extra helpers for zmon.it messages

    Copyright (c) the Contributors as noted in the AUTHORS file.  This file is part
    of zmon.it, the fast and scalable monitoring system.

    This Source Code Form is subject to the terms of the Mozilla Public License, v.
    2.0. If a copy of the MPL was not distributed with this file, You can obtain
    one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

#ifndef ZM_PROTO_UTILS_H_INCLUDED
#define ZM_PROTO_UTILS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Get string item from ext attribute
ZM_PROTO_EXPORT const char *
    zm_proto_ext_string (zm_proto_t *self, const char *name, const char *dflt);

//  Set string item in ext attribute
ZM_PROTO_EXPORT void
    zm_proto_ext_set_string (zm_proto_t *self, const char *name, const char *value);

//  Get long int item from ext attribute
ZM_PROTO_EXPORT long int
    zm_proto_ext_int (zm_proto_t *self, const char *name, long int dflt);

//  Set long int item in ext attribute
ZM_PROTO_EXPORT void
    zm_proto_ext_set_int (zm_proto_t *self, const char *name, long int value);

//  Get double item from ext attribute
ZM_PROTO_EXPORT double
    zm_proto_ext_double (zm_proto_t *self, const char *name, double dflt);

//  Set double item in ext attribute
ZM_PROTO_EXPORT void
    zm_proto_ext_set_double (zm_proto_t *self, const char *name, double value);

//  Self test of this class
ZM_PROTO_EXPORT void
    zm_proto_utils_test (bool verbose);

//  @end

#ifdef __cplusplus
}
#endif

#endif
