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

//  Converts zmsg to zm_proto, this is for compatibility with zproto v1 codec
ZM_PROTO_EXPORT zm_proto_t *
    zm_proto_decode (zmsg_t **message_p);

//  v1 codec compatibility function, creates zm_proto_t with metric and encode it to zmsg_t
ZM_PROTO_EXPORT zmsg_t *
    zm_proto_encode_metric_v1 (
        const char *device,
        int64_t time,
        int32_t ttl,
        zhash_t *ext,
        const char *type,
        const char *value,
        const char *units
    );

//  v1 codec compatibility function, creates zm_proto_t with device and encode it to zmsg_t
ZM_PROTO_EXPORT zmsg_t *
    zm_proto_encode_device_v1 (
        const char *device,
        int64_t time,
        int32_t ttl,
        zhash_t *ext
    );


//  v1 codec compatibility function, creates zm_proto_t with alert and encode it to zmsg_t
ZM_PROTO_EXPORT zmsg_t *
    zm_proto_encode_alert_v1 (
        const char *device,
        int64_t time,
        int32_t ttl,
        zhash_t *ext,
        const char *rule,
        char severity,
        const char *description
    );

//  encode metric
ZM_PROTO_EXPORT void
    zm_proto_encode_metric (
        zm_proto_t *self,
        const char *device,
        int64_t time,
        int32_t ttl,
        zhash_t *ext,
        const char *type,
        const char *value,
        const char *units
    );

//  encode device
ZM_PROTO_EXPORT void
    zm_proto_encode_device (
        zm_proto_t *self,
        const char *device,
        int64_t time,
        int32_t ttl,
        zhash_t *ext
    );

//  encode device
ZM_PROTO_EXPORT void
    zm_proto_encode_alert (
        zm_proto_t *self,
        const char *device,
        int64_t time,
        int32_t ttl,
        zhash_t *ext,
        const char *rule,
        char severity,
        const char *description
    );

//  Encode OK message
ZM_PROTO_EXPORT void
    zm_proto_encode_ok (zm_proto_t *self);

//  Encode ERROR message
ZM_PROTO_EXPORT void
    zm_proto_encode_error (zm_proto_t *self, uint32_t code, const char *description);

//  Self test of this class
ZM_PROTO_EXPORT void
    zm_proto_utils_test (bool verbose);

//  Send STREAM DELIVER zm_proto_t message via mlm_client
ZM_PROTO_EXPORT int
    zm_proto_send_mlm (zm_proto_t *self, mlm_client_t *client, const char *subject);

//  Send MAILBOX DELIVER zm_proto_t message via mlm_client
ZM_PROTO_EXPORT int
    zm_proto_sendto (zm_proto_t *self, mlm_client_t *client, const char *address, const char *subject);

//  Receive zm_proto_t from mlm_client, return -1 and do not touch zm_proto_t
//  if zm_proto_t was NOT delivered
ZM_PROTO_EXPORT int
    zm_proto_recv_mlm (zm_proto_t *self, mlm_client_t *client);

//  @end

#ifdef __cplusplus
}
#endif

#endif
