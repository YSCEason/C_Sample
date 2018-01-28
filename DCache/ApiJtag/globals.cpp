/*******************************************************************************
*
*                          AA      RRRRRRRR    CCCCCCC
*                         AAAA     RRRRRRRRR  CCCCCCCCC
*                        AAAAAA    RRR    RR  CCC      
*                       AAA  AAA   RRRRRRRRR  CC
*                      AAA    AAA  RRR RRR    CCC      
*                     AAA      AAA RRR  RRRR  CCCCCCCCC
*                    AAA        AAARRR   RRRR  CCCCCCC
*
*
*                       Confidential Information
*            Limited Distribution to Authorized Persons Only
*            Created 2000-2003 and Protected as an Unpublished Work
*                  Under the U.S. Copyright Act of 1976.
*                Copyright 2000-2003 ARC International(UK) Ltd
*                          All Rights Reserved
*
*
* File name:          globals.cpp
* 
* Author:             Tom Pennello
*
* Creation Date:      15 Oct 00
*
* Description:
*
*   This file is used to hold global data and functions used within the DLL.  The data in this
*   file are required in more than 1 layer of the DLL.
*   
* History:
*  Version   Date        Author        Description
*  
*  1.0       15/10/00    Tom Pennello  File created.
*
*******************************************************************************/

#include "globals.h"
#include <stdio.h>
#include <stdarg.h>
/*********************************************************
* Opens file logout.txt, prints character string, closes file
**********************************************************/

static bool LOG_CREATED = FALSE;
bool log_to_stdout = FALSE; // Normally, log to file.
bool gverbose = TRUE;  //Normally allow DLL to print to standard output (if false, it will not)

int printInFile( const char *format, ... ) {
    va_list ap;
    va_start(ap,format);
    
    if (log_to_stdout) 
        return vprintf(format,ap);
    else {
        static FILE *debugfile = 0;
        
        if (LOG_CREATED == FALSE) {
            /* erase the contents of the logfile if it already exists */
            debugfile = fopen("logfile.txt", "w");
            LOG_CREATED = TRUE;
	    }
        else 
            debugfile = fopen("logfile.txt", "a");
    
        int cnt = vfprintf(debugfile, format, ap);
        fclose(debugfile);
    
        return cnt;
	}
    }

#if _MSC_VER
#include <windows.h>
// extern "C" void __stdcall Sleep(long);

struct NT_OS : OS {
    // Isolate some OS-specific stuff here.
    override void sleep(unsigned milliseconds) { Sleep(milliseconds); }
    override int stricmp(const char *a, const char *b) {
        return ::stricmp(a,b); 
	}
    override int strnicmp(const char *a, const char *b, int len) {
        return ::strnicmp(a,b,len); 
	}
    };

OS *os = new NT_OS();
#else

#include <ctype.h>
struct Linux_OS : OS {
    // Isolate some OS-specific stuff here.
    override void sleep(unsigned milliseconds) { 
	/*
	typedef unsigned usecond_t;
	extern "C" void usleep(usecond_t);
	*/
	usleep(milliseconds*1000); 
	}
    // Remove this when our Linux library has stricmp.
    override int stricmp(const char *s1, const char *s2) {
	while(1){
	    int a = *s1++; int b = *s2++;
	    if (a!=b){
		if (islower(a)) a = toupper(a);
		if (islower(b)) b= toupper(b);
		if (a != b) return a-b;
		}
	    else
		if (a==0) return 0;
	    }
	}
    override int strnicmp(const char *s1, const char *s2, int len) {
	int cnt = 0;

	while (cnt++ < len){
	    int a = *s1++; int b = *s2++;
	    if (a!=b){
		if (islower(a)) a = toupper(a);
		if (islower(b)) b= _toupper(b);
		if (a != b) return a-b;
		}
	    else
		if (a==0) return 0;
	    }

	/* if we read n chars without exiting, the strings are (i)equal up to n */
	return(0);
	}
    };

OS *os = new Linux_OS();
#endif
