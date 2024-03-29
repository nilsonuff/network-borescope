/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/


#ifndef __TC_H
#define __TC_H

typedef struct {
    int      refcount;
    int      beta;
    int      deleted;
    int      missing;
} TC, * PTC;


int tc_init       (PTC ptc, int beta);
int tc_adjust     (PTC ptc, int beta, int shared, int inc);
int tc_make_proper(PTC ptc, int beta);
int tc_deleted    (PTC ptc, int set);

#define TC_INIT(PN, BETA)                   tc_init((PTC) (PN), BETA)
#define TC_ADJUST(PN, BETA, SHARED, INC)    tc_adjust((PTC) (PN), BETA, SHARED, INC)
#define TC_MAKE_PROPER(PN, BETA)            tc_make_proper((PTC) (PN), BETA)
#define TC_DELETED(PN, SET)                 tc_deleted((PTC) (PN), SET)


//***************************************************************************
//
//    API 0
//
//***************************************************************************

typedef struct api0 {
} Api0, *PApi0;

#endif