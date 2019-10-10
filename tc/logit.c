/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "logit.h"
 
static FILE *flog = NULL;
static char filename[1024];

void logit_set_filename(char *fname) {
    strcpy(filename,fname);
    if (flog) {
        fclose(flog);
        flog = fopen(filename,"wt");
    }
}


void logit(char *msg,...) {
    va_list args;

    va_start(args, msg);
    if (!flog) {
        flog = fopen(filename,"wt");
    }
    
    if (msg) {
        vfprintf(flog,msg,args);
        fprintf(flog,"\n");
        fflush(flog);
    }
    va_end(args);

}

