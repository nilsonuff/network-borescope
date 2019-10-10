/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/

#ifndef __CLASS_PREFIX
#define __CLASS_PREFIX

union {
    int     i;
    double  d;
    void   *p;
} UCommonClassParam;

#define   N_COMMON_CLASS_PARAMS  4
#define   COMMOM_CLASS_PARAMS    UCommonClassParam class_params[N_COMMON_CLASS_PARAMS]

#endif