/*  =========================================================================
    zmargs - Argument parsing helper

    Copyright (c) the Contributors as noted in the AUTHORS file.  This file is part
    of zmon.it, the fast and scalable monitoring system.                           
                                                                                   
    This Source Code Form is subject to the terms of the Mozilla Public License, v.
    2.0. If a copy of the MPL was not distributed with this file, You can obtain   
    one at http://mozilla.org/MPL/2.0/.                                            
    =========================================================================
*/

/*
@header
    zmargs - Argument parsing helper
@discuss
@end
*/

#include "zm_proto_classes.h"

//  Structure of our class

static char *ZARG_PARAM_EMPTY = "";
struct _zmargs_t {
    char *progname;
    zlist_t *arguments;
    zhash_t *parameters;
};


//  --------------------------------------------------------------------------
//  Create a new zmargs

zmargs_t *
zmargs_new (int argc, char **argv)
{
    zmargs_t *self = (zmargs_t*) zmalloc (sizeof (zmargs_t));
    assert (self);

    self->progname = argv [0];
    assert (self->progname);
    self->arguments = zlist_new ();
    assert (self->arguments);
    self->parameters = zhash_new ();
    assert (self->parameters);

    if (argc == 1)
        return self;

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


//  --------------------------------------------------------------------------
//  Destroy the zmargs

void
zmargs_destroy (zmargs_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        zmargs_t *self = *self_p;
        zlist_destroy (&self->arguments);
        zhash_destroy (&self->parameters);
        *self_p = NULL;
    }
}

//  --------------------------------------------------------------------------
//  Return the program name (argv[0]) 

const char *
zmargs_progname (zmargs_t *self) {
    assert (self);
    return self->progname;
}

//  --------------------------------------------------------------------------
//  Return the number of command line arguments

size_t
zmargs_arguments (zmargs_t *self) {
    assert (self);
    return zlist_size (self->arguments);
}

//  --------------------------------------------------------------------------
//  Return first command line argument

const char *
zmargs_first (zmargs_t *self) {
    assert (self);
    return (const char*) zlist_first (self->arguments);
}

//  --------------------------------------------------------------------------
//  Return next command line argument

const char *
zmargs_next (zmargs_t *self) {
    assert (self);
    return (const char*) zlist_next (self->arguments);
}

//  --------------------------------------------------------------------------
//  Return first command line parameter value

const char *
zmargs_param_first (zmargs_t *self) {
    assert (self);
    return (const char*) zhash_first (self->parameters);
}

//  --------------------------------------------------------------------------
//  Return next command line parameter value

const char *
zmargs_param_next (zmargs_t *self) {
    assert (self);
    return (const char*) zhash_next (self->parameters);
}

//  --------------------------------------------------------------------------
//  Return current command line parameter name

const char *
zmargs_param_name (zmargs_t *self) {
    assert (self);
    return (const char*) zhash_cursor (self->parameters);
}

//  --------------------------------------------------------------------------
//  Lookup for command line parameter by name(s)

const char *
zmargs_param_lookup (zmargs_t *self, const char *key1, const char *key2) {
    assert (self);
    const char *ret = (const char*) zhash_lookup (self->parameters, key1);
    if (!ret && key2)
        ret = (const char*) zhash_lookup (self->parameters, key2);
    return ret;
}

//  --------------------------------------------------------------------------
//  return if -h /--help parameters are present

bool
zmargs_has_help (zmargs_t *self) {
    return zmargs_param_lookup (self, "--help", "-h");
}

//  --------------------------------------------------------------------------
//  check if argument had a value or not

bool
zmargs_param_empty (const char* arg) {
    return arg && arg == ZARG_PARAM_EMPTY;
}

//  --------------------------------------------------------------------------
//  Print the zmargs instance

void
zmargs_print (zmargs_t *self) {
    assert (self);
    printf ("%s ", self->progname);
    for (const char *pvalue = zmargs_param_first (self);
                    pvalue != NULL;
                    pvalue = zmargs_param_next (self)) {
        const char *pname = zmargs_param_name (self);
        if (pvalue == ZARG_PARAM_EMPTY)
            printf ("%s ", pname);
        else
            printf ("%s %s ", pname, pvalue);
    }
    for (const char *arg = zmargs_first (self);
                     arg != NULL;
                     arg = zmargs_next (self)) {
        printf ("%s ", arg);
    }
    puts ("");
}

//  --------------------------------------------------------------------------
//  Self test of this class

void
zmargs_test (bool verbose)
{
    printf (" * zmargs: ");

    //  @selftest
    //  Simple create/destroy test

    // Note: If your selftest reads SCMed fixture data, please keep it in
    // src/selftest-ro; if your test creates filesystem objects, please
    // do so under src/selftest-rw. They are defined below along with a
    // usecase for the variables (assert) to make compilers happy.
    const char *SELFTEST_DIR_RO = "src/selftest-ro";
    const char *SELFTEST_DIR_RW = "src/selftest-rw";
    assert (SELFTEST_DIR_RO);
    assert (SELFTEST_DIR_RW);
    // Uncomment these to use C++ strings in C++ selftest code:
    //std::string str_SELFTEST_DIR_RO = std::string(SELFTEST_DIR_RO);
    //std::string str_SELFTEST_DIR_RW = std::string(SELFTEST_DIR_RW);
    //assert ( (str_SELFTEST_DIR_RO != "") );
    //assert ( (str_SELFTEST_DIR_RW != "") );
    // NOTE that for "char*" context you need (str_SELFTEST_DIR_RO + "/myfilename").c_str()

    char *argv [] = {"progrname", };
    zmargs_t *self = zmargs_new (1, argv);
    assert (self);
    zmargs_destroy (&self);
    //  @end
    printf ("OK\n");
}
