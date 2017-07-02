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

/*XXX: does not work!
#define zm_proto_ext_set_int(self, name, value) \
    zm_proto_ext_set_number (self, name, (uint64_t) value)
#define zm_proto_ext_int(self, name, dflt) \
    (int) zm_proto_ext_number (self, name, (uint64_t) dflt)
*/

void
    zm_proto_utils_test (bool verbose);

//  @end

#ifdef __cplusplus
}
#endif

#endif
