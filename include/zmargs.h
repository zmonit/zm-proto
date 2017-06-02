/*  =========================================================================
    zmargs - Argument parsing helper

    Copyright (c) the Contributors as noted in the AUTHORS file.  This file is part
    of zmon.it, the fast and scalable monitoring system.                           
                                                                                   
    This Source Code Form is subject to the terms of the Mozilla Public License, v.
    2.0. If a copy of the MPL was not distributed with this file, You can obtain   
    one at http://mozilla.org/MPL/2.0/.                                            
    =========================================================================
*/

#ifndef ZARGS_H_INCLUDED
#define ZARGS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Create a new zmargs
ZM_PROTO_EXPORT zmargs_t *
    zmargs_new (int argc, char **argv);

//  Destroy the zmargs
ZM_PROTO_EXPORT void
    zmargs_destroy (zmargs_t **self_p);

//  Return the program name (argv[0]) 
ZM_PROTO_EXPORT const char *
    zmargs_progname (zmargs_t *self);

//  Return the number of command line arguments
ZM_PROTO_EXPORT size_t
    zmargs_arguments (zmargs_t *self);

//  Return first command line argument
ZM_PROTO_EXPORT const char *
    zmargs_first (zmargs_t *self);

//  Return next command line argument
ZM_PROTO_EXPORT const char *
    zmargs_next (zmargs_t *self);

//  Return first command line parameter value
ZM_PROTO_EXPORT const char *
    zmargs_param_first (zmargs_t *self);

//  Return next command line parameter value
ZM_PROTO_EXPORT const char *
    zmargs_param_next (zmargs_t *self);

//  Return current command line parameter name
ZM_PROTO_EXPORT const char *
    zmargs_param_name (zmargs_t *self);

//  Lookup for command line parameter by name(s)
ZM_PROTO_EXPORT const char *
    zmargs_param_lookup (zmargs_t *self, const char *key1, const char *key2);

//  return if -h /--help parameters are present
ZM_PROTO_EXPORT bool
    zmargs_has_help (zmargs_t *self);

//  check if argument had a value or not
ZM_PROTO_EXPORT bool
    zmargs_param_empty (const char* arg);

//  Print the zmargs instance
ZM_PROTO_EXPORT void
    zmargs_print (zmargs_t *self);

//  Self test of this class
ZM_PROTO_EXPORT void
    zmargs_test (bool verbose);

//  @end

#ifdef __cplusplus
}
#endif

#endif
