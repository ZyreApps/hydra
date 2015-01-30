/*  =========================================================================
    hydrad - command-line service

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of the Hydra Project

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/
/*
@header
    Hydrad is a test program.
@discuss
@end
*/

#include "hydra_classes.h"

#define PRODUCT         "Hydra service/0.1.0"
#define COPYRIGHT       "Copyright (c) 2014-15 the Contributors"
#define NOWARRANTY \
"This Software is provided under the MPLv2 License on an \"as is\" basis,\n" \
"without warranty of any kind, either expressed, implied, or statutory.\n"

int main (int argc, char *argv [])
{
    puts (PRODUCT);
    puts (COPYRIGHT);
    puts (NOWARRANTY);

    int argn = 1;
    if (argn < argc && streq (argv [argn], "-h")) {
        puts ("syntax: hydrad [-v] [-z] [ [-t] [-i] directory ]");
        puts (" -- defaults to .hydra in current directory");
        puts (" -v = run Hydra protocol in verbose mode");
        puts (" -z = run Zyre discovery in verbose mode");
        puts (" -t = create some test posts");
        puts (" -i = run over ipc:// without networking");
        exit (0);
    }
    bool verbose = false;
    bool zverbose = false;
    bool testmode = false;
    bool localhost = false;
    char *directory = ".hydra";
    while (argn < argc && *argv [argn] == '-') {
        if (streq (argv [argn], "-v"))
            verbose = true;
        else
        if (streq (argv [argn], "-z"))
            zverbose = true;
        else
        if (streq (argv [argn], "-t"))
            testmode = true;
        else
        if (streq (argv [argn], "-i"))
            localhost = true;
        else {
            puts ("Invalid option, run hydrad -h to see options");
            exit (0);
        }
        argn++;
    }
    if (argn < argc)
        directory = argv [argn];
    
    hydra_t *hydra = hydra_new (directory);
    if (!hydra)
        exit (0);
    
    if (verbose)
        hydra_set_animate (hydra);
    if (zverbose)
        hydra_set_verbose (hydra);
    if (localhost)
        hydra_set_local_ipc (hydra);
    
    hydra_start (hydra);
    while (!zsys_interrupted)
        sleep (1);
    hydra_destroy (&hydra);
    
    return 0;
}
