/*  =========================================================================
    zm_proto - Basic messaging for zmon.it

    Codec header for zm_proto.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: zm_proto.xml, or
     * The code generation script that built this file: zproto_codec_c
    ************************************************************************
    Copyright (c) the Contributors as noted in the AUTHORS file.  This file is part
    of zmon.it, the fast and scalable monitoring system.                           
                                                                                   
    This Source Code Form is subject to the terms of the Mozilla Public License, v.
    2.0. If a copy of the MPL was not distributed with this file, You can obtain   
    one at http://mozilla.org/MPL/2.0/.                                            
    =========================================================================
*/

#ifndef ZM_PROTO_H_INCLUDED
#define ZM_PROTO_H_INCLUDED

/*  These are the zm_proto messages:

    METRIC - 
        device              uuid        Device universal unique identifier
        time                number 8    Time when message was generated
        ttl                 number 4    Time to live, after $current time > time - ttl, message is droped
        ext                 hash        Additional extended informations for the message
        type                string      Metric name, e.g.: "temperature", "humidity", "power.load", ...
        value               string      Metric value, e.g.: "25.323" or "900".
        unit                string      Metric unit, e.g.: "C" or "F" for temperature, "W" or "kW" for realpower etc...

    ALERT - 
        device              uuid        Device universal unique identifier
        time                number 8    Time when message was generated
        ttl                 number 4    Time to live, after $current time > time - ttl, message is droped
        ext                 hash        Additional extended informations for the message
        rule                string      Identifier of the rule which triggers this alert.
        state               number 1    Alert is active (value 1) or resolved (value 0).
        severity            number 1    Alert is critical (value 1) or not (value 0).
        description         string      Alert description.

    DEVICE - 
        device              uuid        Device universal unique identifier
        time                number 8    Time when message was generated
        ttl                 number 4    Time to live, after $current time > time - ttl, message is droped
        ext                 hash        Additional extended informations for the message
*/


#define ZM_PROTO_METRIC                     1
#define ZM_PROTO_ALERT                      2
#define ZM_PROTO_DEVICE                     3

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef ZM_PROTO_T_DEFINED
typedef struct _zm_proto_t zm_proto_t;
#define ZM_PROTO_T_DEFINED
#endif

//  @interface
//  Create a new empty zm_proto
zm_proto_t *
    zm_proto_new (void);

//  Destroy a zm_proto instance
void
    zm_proto_destroy (zm_proto_t **self_p);

//  Create a deep copy of a zm_proto instance
zm_proto_t *
    zm_proto_dup (zm_proto_t *other);

//  Receive a zm_proto from the socket. Returns 0 if OK, -1 if
//  the read was interrupted, or -2 if the message is malformed.
//  Blocks if there is no message waiting.
int
    zm_proto_recv (zm_proto_t *self, zsock_t *input);

//  Send the zm_proto to the output socket, does not destroy it
int
    zm_proto_send (zm_proto_t *self, zsock_t *output);

#if defined (MLM_VERSION)
//  --------------------------------------------------------------------------
//  Publish the zm_proto to malamute broker. Does not destroy it. Returns 0 if
//  OK, else -1.

int
zm_proto_msend (zm_proto_t *self, mlm_client_t *client, const char* subject);
#endif

//  Print contents of message to stdout
void
    zm_proto_print (zm_proto_t *self);

//  Get/set the message routing id
zframe_t *
    zm_proto_routing_id (zm_proto_t *self);
void
    zm_proto_set_routing_id (zm_proto_t *self, zframe_t *routing_id);

//  Get the zm_proto id and printable command
int
    zm_proto_id (zm_proto_t *self);
void
    zm_proto_set_id (zm_proto_t *self, int id);
const char *
    zm_proto_command (zm_proto_t *self);

//  Get/set the device field
zuuid_t *
    zm_proto_device (zm_proto_t *self);
void
    zm_proto_set_device (zm_proto_t *self, zuuid_t *uuid);
//  Get the device field and transfer ownership to caller
zuuid_t *
    zm_proto_get_device (zm_proto_t *self);

//  Get/set the time field
uint64_t
    zm_proto_time (zm_proto_t *self);
void
    zm_proto_set_time (zm_proto_t *self, uint64_t time);

//  Get/set the ttl field
uint32_t
    zm_proto_ttl (zm_proto_t *self);
void
    zm_proto_set_ttl (zm_proto_t *self, uint32_t ttl);

//  Get a copy of the ext field
zhash_t *
    zm_proto_ext (zm_proto_t *self);
//  Get the ext field and transfer ownership to caller
zhash_t *
    zm_proto_get_ext (zm_proto_t *self);
//  Set the ext field, transferring ownership from caller
void
    zm_proto_set_ext (zm_proto_t *self, zhash_t **hash_p);

//  Get/set the type field
const char *
    zm_proto_type (zm_proto_t *self);
void
    zm_proto_set_type (zm_proto_t *self, const char *value);

//  Get/set the value field
const char *
    zm_proto_value (zm_proto_t *self);
void
    zm_proto_set_value (zm_proto_t *self, const char *value);

//  Get/set the unit field
const char *
    zm_proto_unit (zm_proto_t *self);
void
    zm_proto_set_unit (zm_proto_t *self, const char *value);

//  Get/set the rule field
const char *
    zm_proto_rule (zm_proto_t *self);
void
    zm_proto_set_rule (zm_proto_t *self, const char *value);

//  Get/set the state field
byte
    zm_proto_state (zm_proto_t *self);
void
    zm_proto_set_state (zm_proto_t *self, byte state);

//  Get/set the severity field
byte
    zm_proto_severity (zm_proto_t *self);
void
    zm_proto_set_severity (zm_proto_t *self, byte severity);

//  Get/set the description field
const char *
    zm_proto_description (zm_proto_t *self);
void
    zm_proto_set_description (zm_proto_t *self, const char *value);

//  Self test of this class
void
    zm_proto_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define zm_proto_dump       zm_proto_print

#ifdef __cplusplus
}
#endif

#endif
