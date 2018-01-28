#ifndef _GLOBALS_H
#define _GLOBALS_H
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
*
* File name:  globals.h
* 
* Author   :  Tom Pennello
*
* Date     :  17/09/00
*
* Description : Global function/variable definitions, used thoughout DLLs
*
*  History:
*
*   Version    Date         Author           Description
*
*   1.0        17/09/00     Tom Pennello     File created. 
*
*******************************************************************************/


/* Function prototypes */
extern int printInFile(const char *format, ... );

/* Type definitions */
// Unfortunately Microsoft defines these in windows.h so we have to
// be careful.
typedef unsigned long ULONG;
typedef unsigned short UWORD;
#if _MSC_VER  // TBH 8 SEP 2005 this caused an error with Visual Studio
#ifndef FALSE
#define FALSE false
#endif
#ifndef TRUE
#define TRUE true
#endif
#else
#endif
#define UBYTE char 


#if !__HIGHC__
#define override    // MSC doesn't support override, so make blank.
#else
#define override _Override
pragma offwarn(127,553);
enum bool {false,true};
static const bool FALSE = false;
static const bool TRUE = true;
pragma cpp_level(2);
#endif
#define NOINLINE virtual

extern bool Win_NT_Host;
extern bool log_to_stdout;
extern bool gverbose;

struct OS {
    // Isolate some OS-specific stuff here.
    virtual void sleep(unsigned milliseconds) = 0;
    virtual int stricmp(const char *a, const char *b) = 0;
    virtual int strnicmp(const char *a, const char *b, int len) = 0;
    };
extern OS *os;

struct Printf {
    virtual void printf(const char *format, ...) = 0;
    };

#define PRINTLINE printf("line %d file %s\n",__LINE__,__FILE__);
#if _LINUX
// Avoid any conflict with engine, which has a class of the same name.
#define DLL_load OTHER_DLL_load
#define UNIX_DLL_load OTHER_UNIX_DLL_load
#endif
#endif

