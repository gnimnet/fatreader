/*
*	This file has read & write functions declared
*	those functions are in rwfun.c
*	By Ming		2009-1
*/

#ifndef _RWFUN_H_
#define _RWFUN_H_

#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>       
#include <sys/types.h>
#include <unistd.h>

#include "mytype.h"

#define OFF_T DWORD

int initRW(const char *pathname);
int seekreadRW(OFF_T sector,OFF_T offset,void *buf,int count);
int seekwriteRW(OFF_T sector,OFF_T offset,void *buf,int count);
void setSectorSize(WORD Size);

#endif//end of rwfun.h
