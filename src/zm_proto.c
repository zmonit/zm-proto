/*  =========================================================================
    zm_proto - Basic messaging for zmon.it

    Codec class for zm_proto.

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

/*
@header
    zm_proto - Basic messaging for zmon.it
@discuss
@end
*/

#ifdef NDEBUG
#undef NDEBUG
#endif

#include "../include/zm_proto.h"

//  Structure of our class

struct _zm_proto_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  zm_proto message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    char device [256];                  //  Device universal unique identifier
    uint64_t time;                      //  Time when message was generated
    uint32_t ttl;                       //  Time to live, after $current time > time - ttl, message is droped
    zhash_t *ext;                       //  Additional extended informations for the message
    size_t ext_bytes;                   //  Size of hash content
    char type [256];                    //  Metric name, e.g.: "temperature", "humidity", "power.load", ...
    char value [256];                   //  Metric value, e.g.: "25.323" or "900".
    char unit [256];                    //  Metric unit, e.g.: "C" or "F" for temperature, "W" or "kW" for realpower etc...
    char rule [256];                    //  Identifier of the rule which triggers this alert.
    byte severity;                      //  Alert is present and critical (value > 0) or resolved (value 0).
    char description [256];             //  Alert description.
};

//  --------------------------------------------------------------------------
//  Network data encoding macros

//  Put a block of octets to the frame
#define PUT_OCTETS(host,size) { \
    memcpy (self->needle, (host), size); \
    self->needle += size; \
}

//  Get a block of octets from the frame
#define GET_OCTETS(host,size) { \
    if (self->needle + size > self->ceiling) { \
        zsys_warning ("zm_proto: GET_OCTETS failed"); \
        goto malformed; \
    } \
    memcpy ((host), self->needle, size); \
    self->needle += size; \
}

//  Put a 1-byte number to the frame
#define PUT_NUMBER1(host) { \
    *(byte *) self->needle = (byte) (host); \
    self->needle++; \
}

//  Put a 2-byte number to the frame
#define PUT_NUMBER2(host) { \
    self->needle [0] = (byte) (((host) >> 8)  & 255); \
    self->needle [1] = (byte) (((host))       & 255); \
    self->needle += 2; \
}

//  Put a 4-byte number to the frame
#define PUT_NUMBER4(host) { \
    self->needle [0] = (byte) (((host) >> 24) & 255); \
    self->needle [1] = (byte) (((host) >> 16) & 255); \
    self->needle [2] = (byte) (((host) >> 8)  & 255); \
    self->needle [3] = (byte) (((host))       & 255); \
    self->needle += 4; \
}

//  Put a 8-byte number to the frame
#define PUT_NUMBER8(host) { \
    self->needle [0] = (byte) (((host) >> 56) & 255); \
    self->needle [1] = (byte) (((host) >> 48) & 255); \
    self->needle [2] = (byte) (((host) >> 40) & 255); \
    self->needle [3] = (byte) (((host) >> 32) & 255); \
    self->needle [4] = (byte) (((host) >> 24) & 255); \
    self->needle [5] = (byte) (((host) >> 16) & 255); \
    self->needle [6] = (byte) (((host) >> 8)  & 255); \
    self->needle [7] = (byte) (((host))       & 255); \
    self->needle += 8; \
}

//  Get a 1-byte number from the frame
#define GET_NUMBER1(host) { \
    if (self->needle + 1 > self->ceiling) { \
        zsys_warning ("zm_proto: GET_NUMBER1 failed"); \
        goto malformed; \
    } \
    (host) = *(byte *) self->needle; \
    self->needle++; \
}

//  Get a 2-byte number from the frame
#define GET_NUMBER2(host) { \
    if (self->needle + 2 > self->ceiling) { \
        zsys_warning ("zm_proto: GET_NUMBER2 failed"); \
        goto malformed; \
    } \
    (host) = ((uint16_t) (self->needle [0]) << 8) \
           +  (uint16_t) (self->needle [1]); \
    self->needle += 2; \
}

//  Get a 4-byte number from the frame
#define GET_NUMBER4(host) { \
    if (self->needle + 4 > self->ceiling) { \
        zsys_warning ("zm_proto: GET_NUMBER4 failed"); \
        goto malformed; \
    } \
    (host) = ((uint32_t) (self->needle [0]) << 24) \
           + ((uint32_t) (self->needle [1]) << 16) \
           + ((uint32_t) (self->needle [2]) << 8) \
           +  (uint32_t) (self->needle [3]); \
    self->needle += 4; \
}

//  Get a 8-byte number from the frame
#define GET_NUMBER8(host) { \
    if (self->needle + 8 > self->ceiling) { \
        zsys_warning ("zm_proto: GET_NUMBER8 failed"); \
        goto malformed; \
    } \
    (host) = ((uint64_t) (self->needle [0]) << 56) \
           + ((uint64_t) (self->needle [1]) << 48) \
           + ((uint64_t) (self->needle [2]) << 40) \
           + ((uint64_t) (self->needle [3]) << 32) \
           + ((uint64_t) (self->needle [4]) << 24) \
           + ((uint64_t) (self->needle [5]) << 16) \
           + ((uint64_t) (self->needle [6]) << 8) \
           +  (uint64_t) (self->needle [7]); \
    self->needle += 8; \
}

//  Put a string to the frame
#define PUT_STRING(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER1 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a string from the frame
#define GET_STRING(host) { \
    size_t string_size; \
    GET_NUMBER1 (string_size); \
    if (self->needle + string_size > (self->ceiling)) { \
        zsys_warning ("zm_proto: GET_STRING failed"); \
        goto malformed; \
    } \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}

//  Put a long string to the frame
#define PUT_LONGSTR(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER4 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a long string from the frame
#define GET_LONGSTR(host) { \
    size_t string_size; \
    GET_NUMBER4 (string_size); \
    if (self->needle + string_size > (self->ceiling)) { \
        zsys_warning ("zm_proto: GET_LONGSTR failed"); \
        goto malformed; \
    } \
    free ((host)); \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}


//  --------------------------------------------------------------------------
//  Create a new zm_proto

zm_proto_t *
zm_proto_new (void)
{
    zm_proto_t *self = (zm_proto_t *) zmalloc (sizeof (zm_proto_t));
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the zm_proto

void
zm_proto_destroy (zm_proto_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        zm_proto_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        zhash_destroy (&self->ext);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Create a deep copy of a zm_proto instance

zm_proto_t *
zm_proto_dup (zm_proto_t *other)
{
    assert (other);
    zm_proto_t *copy = zm_proto_new ();

    // Copy the routing and message id
    zm_proto_set_routing_id (copy, zframe_dup (zm_proto_routing_id (other)));
    zm_proto_set_id (copy, zm_proto_id (other));

    // Copy the rest of the fields
    zm_proto_set_device (copy, zm_proto_device (other));
    zm_proto_set_time (copy, zm_proto_time (other));
    zm_proto_set_ttl (copy, zm_proto_ttl (other));
    {
        zhash_t *dup_hash = zhash_dup (zm_proto_ext (other));
        zm_proto_set_ext (copy, &dup_hash);
    }
    zm_proto_set_type (copy, zm_proto_type (other));
    zm_proto_set_value (copy, zm_proto_value (other));
    zm_proto_set_unit (copy, zm_proto_unit (other));
    zm_proto_set_rule (copy, zm_proto_rule (other));
    zm_proto_set_severity (copy, zm_proto_severity (other));
    zm_proto_set_description (copy, zm_proto_description (other));

    return copy;
}

//  --------------------------------------------------------------------------
//  Deserialize a zm_proto from the specified message, popping
//  as many frames as needed. Returns 0 if OK, -1 if the recv was interrupted,
//  or -2 if the message is malformed.
int
zm_proto_recv (zm_proto_t *self, zmsg_t *input)
{
    assert (input);
    int rc = 0;


    zframe_t *frame = zmsg_pop (input);
    if (!frame) {
        zsys_warning ("zm_proto: missing frames in message");
        rc = -1;                //  Interrupted
        goto malformed;
    }
    //  Get and check protocol signature
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);

    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 1)) {
        zsys_warning ("zm_proto: invalid signature");
        rc = -2;                //  Malformed
        goto malformed;
    }
    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case ZM_PROTO_METRIC:
            GET_STRING (self->device);
            GET_NUMBER8 (self->time);
            GET_NUMBER4 (self->ttl);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                zhash_destroy (&self->ext);
                self->ext = zhash_new ();
                zhash_autofree (self->ext);
                while (hash_size--) {
                    char key [256];
                    char *value = NULL;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->ext, key, value);
                    free (value);
                }
            }
            GET_STRING (self->type);
            GET_STRING (self->value);
            GET_STRING (self->unit);
            break;

        case ZM_PROTO_ALERT:
            GET_STRING (self->device);
            GET_NUMBER8 (self->time);
            GET_NUMBER4 (self->ttl);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                zhash_destroy (&self->ext);
                self->ext = zhash_new ();
                zhash_autofree (self->ext);
                while (hash_size--) {
                    char key [256];
                    char *value = NULL;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->ext, key, value);
                    free (value);
                }
            }
            GET_STRING (self->rule);
            GET_NUMBER1 (self->severity);
            GET_STRING (self->description);
            break;

        case ZM_PROTO_DEVICE:
            GET_STRING (self->device);
            GET_NUMBER8 (self->time);
            GET_NUMBER4 (self->ttl);
            {
                size_t hash_size;
                GET_NUMBER4 (hash_size);
                zhash_destroy (&self->ext);
                self->ext = zhash_new ();
                zhash_autofree (self->ext);
                while (hash_size--) {
                    char key [256];
                    char *value = NULL;
                    GET_STRING (key);
                    GET_LONGSTR (value);
                    zhash_insert (self->ext, key, value);
                    free (value);
                }
            }
            break;

        default:
            zsys_warning ("zm_proto: bad message ID");
            rc = -2;            //  Malformed
            goto malformed;
    }
    zframe_destroy (&frame);
    //  Successful return
    return rc;

    //  Error returns
    malformed:
        zframe_destroy (&frame);
        return rc;              //  Invalid message
}


//  --------------------------------------------------------------------------
//  Serialize and append the zm_proto to the specified message
int
zm_proto_send (zm_proto_t *self, zmsg_t *output)
{
    assert (self);
    assert (output);
    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case ZM_PROTO_METRIC:
            frame_size += 1 + strlen (self->device);
            frame_size += 8;            //  time
            frame_size += 4;            //  ttl
            frame_size += 4;            //  Size is 4 octets
            if (self->ext) {
                self->ext_bytes = 0;
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    self->ext_bytes += 1 + strlen (zhash_cursor (self->ext));
                    self->ext_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            frame_size += self->ext_bytes;
            frame_size += 1 + strlen (self->type);
            frame_size += 1 + strlen (self->value);
            frame_size += 1 + strlen (self->unit);
            break;
        case ZM_PROTO_ALERT:
            frame_size += 1 + strlen (self->device);
            frame_size += 8;            //  time
            frame_size += 4;            //  ttl
            frame_size += 4;            //  Size is 4 octets
            if (self->ext) {
                self->ext_bytes = 0;
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    self->ext_bytes += 1 + strlen (zhash_cursor (self->ext));
                    self->ext_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            frame_size += self->ext_bytes;
            frame_size += 1 + strlen (self->rule);
            frame_size += 1;            //  severity
            frame_size += 1 + strlen (self->description);
            break;
        case ZM_PROTO_DEVICE:
            frame_size += 1 + strlen (self->device);
            frame_size += 8;            //  time
            frame_size += 4;            //  ttl
            frame_size += 4;            //  Size is 4 octets
            if (self->ext) {
                self->ext_bytes = 0;
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    self->ext_bytes += 1 + strlen (zhash_cursor (self->ext));
                    self->ext_bytes += 4 + strlen (item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            frame_size += self->ext_bytes;
            break;
    }
    //  Now serialize message into the frame
    zframe_t *frame = zframe_new (NULL, frame_size);
    self->needle = zframe_data (frame);
    PUT_NUMBER2 (0xAAA0 | 1);
    PUT_NUMBER1 (self->id);

    switch (self->id) {
        case ZM_PROTO_METRIC:
            PUT_STRING (self->device);
            PUT_NUMBER8 (self->time);
            PUT_NUMBER4 (self->ttl);
            if (self->ext) {
                PUT_NUMBER4 (zhash_size (self->ext));
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    PUT_STRING (zhash_cursor (self->ext));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty hash
            PUT_STRING (self->type);
            PUT_STRING (self->value);
            PUT_STRING (self->unit);
            break;

        case ZM_PROTO_ALERT:
            PUT_STRING (self->device);
            PUT_NUMBER8 (self->time);
            PUT_NUMBER4 (self->ttl);
            if (self->ext) {
                PUT_NUMBER4 (zhash_size (self->ext));
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    PUT_STRING (zhash_cursor (self->ext));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty hash
            PUT_STRING (self->rule);
            PUT_NUMBER1 (self->severity);
            PUT_STRING (self->description);
            break;

        case ZM_PROTO_DEVICE:
            PUT_STRING (self->device);
            PUT_NUMBER8 (self->time);
            PUT_NUMBER4 (self->ttl);
            if (self->ext) {
                PUT_NUMBER4 (zhash_size (self->ext));
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    PUT_STRING (zhash_cursor (self->ext));
                    PUT_LONGSTR (item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty hash
            break;

    }
    //  Now store the frame data
    zmsg_append (output, &frame);

    return 0;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
zm_proto_print (zm_proto_t *self)
{
    assert (self);
    switch (self->id) {
        case ZM_PROTO_METRIC:
            zsys_debug ("ZM_PROTO_METRIC:");
            zsys_debug ("    device='%s'", self->device);
            zsys_debug ("    time=%ld", (long) self->time);
            zsys_debug ("    ttl=%ld", (long) self->ttl);
            zsys_debug ("    ext=");
            if (self->ext) {
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->ext), item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    type='%s'", self->type);
            zsys_debug ("    value='%s'", self->value);
            zsys_debug ("    unit='%s'", self->unit);
            break;

        case ZM_PROTO_ALERT:
            zsys_debug ("ZM_PROTO_ALERT:");
            zsys_debug ("    device='%s'", self->device);
            zsys_debug ("    time=%ld", (long) self->time);
            zsys_debug ("    ttl=%ld", (long) self->ttl);
            zsys_debug ("    ext=");
            if (self->ext) {
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->ext), item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            else
                zsys_debug ("(NULL)");
            zsys_debug ("    rule='%s'", self->rule);
            zsys_debug ("    severity=%ld", (long) self->severity);
            zsys_debug ("    description='%s'", self->description);
            break;

        case ZM_PROTO_DEVICE:
            zsys_debug ("ZM_PROTO_DEVICE:");
            zsys_debug ("    device='%s'", self->device);
            zsys_debug ("    time=%ld", (long) self->time);
            zsys_debug ("    ttl=%ld", (long) self->ttl);
            zsys_debug ("    ext=");
            if (self->ext) {
                char *item = (char *) zhash_first (self->ext);
                while (item) {
                    zsys_debug ("        %s=%s", zhash_cursor (self->ext), item);
                    item = (char *) zhash_next (self->ext);
                }
            }
            else
                zsys_debug ("(NULL)");
            break;

    }
}


//  --------------------------------------------------------------------------
//  Get/set the message routing_id

zframe_t *
zm_proto_routing_id (zm_proto_t *self)
{
    assert (self);
    return self->routing_id;
}

void
zm_proto_set_routing_id (zm_proto_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the zm_proto id

int
zm_proto_id (zm_proto_t *self)
{
    assert (self);
    return self->id;
}

void
zm_proto_set_id (zm_proto_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
zm_proto_command (zm_proto_t *self)
{
    assert (self);
    switch (self->id) {
        case ZM_PROTO_METRIC:
            return ("METRIC");
            break;
        case ZM_PROTO_ALERT:
            return ("ALERT");
            break;
        case ZM_PROTO_DEVICE:
            return ("DEVICE");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the device field

const char *
zm_proto_device (zm_proto_t *self)
{
    assert (self);
    return self->device;
}

void
zm_proto_set_device (zm_proto_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->device)
        return;
    strncpy (self->device, value, 255);
    self->device [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the time field

uint64_t
zm_proto_time (zm_proto_t *self)
{
    assert (self);
    return self->time;
}

void
zm_proto_set_time (zm_proto_t *self, uint64_t time)
{
    assert (self);
    self->time = time;
}


//  --------------------------------------------------------------------------
//  Get/set the ttl field

uint32_t
zm_proto_ttl (zm_proto_t *self)
{
    assert (self);
    return self->ttl;
}

void
zm_proto_set_ttl (zm_proto_t *self, uint32_t ttl)
{
    assert (self);
    self->ttl = ttl;
}


//  --------------------------------------------------------------------------
//  Get the ext field without transferring ownership

zhash_t *
zm_proto_ext (zm_proto_t *self)
{
    assert (self);
    return self->ext;
}

//  Get the ext field and transfer ownership to caller

zhash_t *
zm_proto_get_ext (zm_proto_t *self)
{
    zhash_t *ext = self->ext;
    self->ext = NULL;
    return ext;
}

//  Set the ext field, transferring ownership from caller

void
zm_proto_set_ext (zm_proto_t *self, zhash_t **ext_p)
{
    assert (self);
    assert (ext_p);
    zhash_destroy (&self->ext);
    self->ext = *ext_p;
    *ext_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get/set the type field

const char *
zm_proto_type (zm_proto_t *self)
{
    assert (self);
    return self->type;
}

void
zm_proto_set_type (zm_proto_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->type)
        return;
    strncpy (self->type, value, 255);
    self->type [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the value field

const char *
zm_proto_value (zm_proto_t *self)
{
    assert (self);
    return self->value;
}

void
zm_proto_set_value (zm_proto_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->value)
        return;
    strncpy (self->value, value, 255);
    self->value [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the unit field

const char *
zm_proto_unit (zm_proto_t *self)
{
    assert (self);
    return self->unit;
}

void
zm_proto_set_unit (zm_proto_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->unit)
        return;
    strncpy (self->unit, value, 255);
    self->unit [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the rule field

const char *
zm_proto_rule (zm_proto_t *self)
{
    assert (self);
    return self->rule;
}

void
zm_proto_set_rule (zm_proto_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->rule)
        return;
    strncpy (self->rule, value, 255);
    self->rule [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the severity field

byte
zm_proto_severity (zm_proto_t *self)
{
    assert (self);
    return self->severity;
}

void
zm_proto_set_severity (zm_proto_t *self, byte severity)
{
    assert (self);
    self->severity = severity;
}


//  --------------------------------------------------------------------------
//  Get/set the description field

const char *
zm_proto_description (zm_proto_t *self)
{
    assert (self);
    return self->description;
}

void
zm_proto_set_description (zm_proto_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->description)
        return;
    strncpy (self->description, value, 255);
    self->description [255] = 0;
}



//  --------------------------------------------------------------------------
//  Selftest

void
zm_proto_test (bool verbose)
{
    printf (" * zm_proto: ");

    if (verbose)
        printf ("\n");

    //  @selftest
    //  Simple create/destroy test
    zm_proto_t *self = zm_proto_new ();
    assert (self);
    zm_proto_destroy (&self);
    zmsg_t *output = zmsg_new ();
    assert (output);

    zmsg_t *input = zmsg_new ();
    assert (input);


    //  Encode/send/decode and verify each message type
    int instance;
    self = zm_proto_new ();
    zm_proto_set_id (self, ZM_PROTO_METRIC);

    zm_proto_set_device (self, "Life is short but Now lasts for ever");
    zm_proto_set_time (self, 123);
    zm_proto_set_ttl (self, 123);
    zhash_t *metric_ext = zhash_new ();
    zhash_insert (metric_ext, "Name", "Brutus");
    zm_proto_set_ext (self, &metric_ext);
    zm_proto_set_type (self, "Life is short but Now lasts for ever");
    zm_proto_set_value (self, "Life is short but Now lasts for ever");
    zm_proto_set_unit (self, "Life is short but Now lasts for ever");
    zmsg_destroy (&output);
    output = zmsg_new ();
    assert (output);
    //  Send twice
    zm_proto_send (self, output);
    zm_proto_send (self, output);

    zmsg_destroy (&input);
    input = zmsg_dup (output);
    assert (input);
    for (instance = 0; instance < 2; instance++) {
        zm_proto_recv (self, input);
        assert (zm_proto_routing_id (self) == NULL);
        assert (streq (zm_proto_device (self), "Life is short but Now lasts for ever"));
        assert (zm_proto_time (self) == 123);
        assert (zm_proto_ttl (self) == 123);
        zhash_t *ext = zm_proto_get_ext (self);
        assert (zhash_size (ext) == 1);
        assert (streq ((char *) zhash_first (ext), "Brutus"));
        assert (streq ((char *) zhash_cursor (ext), "Name"));
        zhash_destroy (&ext);
        if (instance == 1)
            zhash_destroy (&metric_ext);
        assert (streq (zm_proto_type (self), "Life is short but Now lasts for ever"));
        assert (streq (zm_proto_value (self), "Life is short but Now lasts for ever"));
        assert (streq (zm_proto_unit (self), "Life is short but Now lasts for ever"));
    }
    zm_proto_set_id (self, ZM_PROTO_ALERT);

    zm_proto_set_device (self, "Life is short but Now lasts for ever");
    zm_proto_set_time (self, 123);
    zm_proto_set_ttl (self, 123);
    zhash_t *alert_ext = zhash_new ();
    zhash_insert (alert_ext, "Name", "Brutus");
    zm_proto_set_ext (self, &alert_ext);
    zm_proto_set_rule (self, "Life is short but Now lasts for ever");
    zm_proto_set_severity (self, 123);
    zm_proto_set_description (self, "Life is short but Now lasts for ever");
    zmsg_destroy (&output);
    output = zmsg_new ();
    assert (output);
    //  Send twice
    zm_proto_send (self, output);
    zm_proto_send (self, output);

    zmsg_destroy (&input);
    input = zmsg_dup (output);
    assert (input);
    for (instance = 0; instance < 2; instance++) {
        zm_proto_recv (self, input);
        assert (zm_proto_routing_id (self) == NULL);
        assert (streq (zm_proto_device (self), "Life is short but Now lasts for ever"));
        assert (zm_proto_time (self) == 123);
        assert (zm_proto_ttl (self) == 123);
        zhash_t *ext = zm_proto_get_ext (self);
        assert (zhash_size (ext) == 1);
        assert (streq ((char *) zhash_first (ext), "Brutus"));
        assert (streq ((char *) zhash_cursor (ext), "Name"));
        zhash_destroy (&ext);
        if (instance == 1)
            zhash_destroy (&alert_ext);
        assert (streq (zm_proto_rule (self), "Life is short but Now lasts for ever"));
        assert (zm_proto_severity (self) == 123);
        assert (streq (zm_proto_description (self), "Life is short but Now lasts for ever"));
    }
    zm_proto_set_id (self, ZM_PROTO_DEVICE);

    zm_proto_set_device (self, "Life is short but Now lasts for ever");
    zm_proto_set_time (self, 123);
    zm_proto_set_ttl (self, 123);
    zhash_t *device_ext = zhash_new ();
    zhash_insert (device_ext, "Name", "Brutus");
    zm_proto_set_ext (self, &device_ext);
    zmsg_destroy (&output);
    output = zmsg_new ();
    assert (output);
    //  Send twice
    zm_proto_send (self, output);
    zm_proto_send (self, output);

    zmsg_destroy (&input);
    input = zmsg_dup (output);
    assert (input);
    for (instance = 0; instance < 2; instance++) {
        zm_proto_recv (self, input);
        assert (zm_proto_routing_id (self) == NULL);
        assert (streq (zm_proto_device (self), "Life is short but Now lasts for ever"));
        assert (zm_proto_time (self) == 123);
        assert (zm_proto_ttl (self) == 123);
        zhash_t *ext = zm_proto_get_ext (self);
        assert (zhash_size (ext) == 1);
        assert (streq ((char *) zhash_first (ext), "Brutus"));
        assert (streq ((char *) zhash_cursor (ext), "Name"));
        zhash_destroy (&ext);
        if (instance == 1)
            zhash_destroy (&device_ext);
    }

    zm_proto_destroy (&self);
    zmsg_destroy (&input);
    zmsg_destroy (&output);
    //  @end

    printf ("OK\n");
}
