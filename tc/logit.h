/*
Copyright 2017, 2018, 2019 Nilson Luís Damasceno
Distributed under the GNU General Public License, Version 3.
Last update : October 10th, 2019.
*/

#ifndef LOGIT_H
#define LOGIT_H

#define __LOGIT_NO

#ifdef __LOGIT        
#define LOGIT_DECL(X) X
#else
#define LOGIT_DECL(X) 
#endif

#ifdef __LOGIT      
#define LOGIT(M) logit(M)
#else
#define LOGIT(M) 
#endif

#ifdef __LOGIT      
#define LOGIT_1(M,A) logit(M,A)
#else
#define LOGIT_1(M,A) 
#endif

#ifdef __LOGIT      
#define LOGIT_2(M,A,B) logit(M,A,B)
#else
#define LOGIT_2(M,A,B) 
#endif

#ifdef __LOGIT      
#define LOGIT_3(M,A,B,C) logit(M,A,B,C)
#else
#define LOGIT_3(M,A,B,C) 
#endif

#ifdef __LOGIT      
#define LOGIT_4(M,A,B,C,D) logit(M,A,B,C,D)
#else
#define LOGIT_4(M,A,B,C,D) 
#endif

#ifdef __LOGIT      
#define LOGIT_5(M,A,B,C,D,E) logit(M,A,B,C,D,E)
#else
#define LOGIT_5(M,A,B,C,D,E) 
#endif

#ifdef __LOGIT      
#define LOGIT_6(M,A,B,C,D,E,F) logit(M,A,B,C,D,E,F)
#else
#define LOGIT_6(M,A,B,C,D,E,F) 
#endif

void logit_set_filename(char *fname);

void logit(char *msg,...);


#endif /* LOGIT_H */
