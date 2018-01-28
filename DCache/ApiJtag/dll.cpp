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
* File name:          dll.cpp
* 
* Author:             Tom Pennello
*
* Creation Date:      15 0ct 2000
*
* Description:
*
*   This file has been created so the api.cpp file is not relient on a particular operating 
*   system.  It's responsibility is manipulating a dll (i.e loading), (if using unix, 
*   this file will need changing)  
*   
* History:
*
*   Version    Date        Author           Description
*   
*   1.0        15/10/00    Tom Pennello     File created. 
* 
*******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "dll.h" 

DLL_load::~DLL_load() {}    // Microsoft bug.

#if _MSC_VER

#include <windows.h>
struct Windows_DLL_load : DLL_load {
    Windows_DLL_load() {}
    HINSTANCE cookie;

    
   /*********************************************************
    * int load_DLL(const char *name)  
    *
    * This function is used as a check to see if a DLL exists
    * returns 1 if DLL can be loaded, 0 if it can't 
    **********************************************************/
    override int load_DLL(const char *name) { 
        cookie = LoadLibrary(name);
        
        // I think W98 provides the .dll extension for you, but not NT 4.
        //if (dll doesn't exist) + (no *.dll extension on file)
        if (cookie == 0 && strstr(name,".dll") == 0 && strstr(name,".DLL")==0) {
            char temp[1024];
            sprintf(temp,"%s.dll",name);      //add .dll extension to file
            0 && printf("Trying to load %s\n",temp);
            cookie = LoadLibrary(temp);       //try loading it again
	    }

        //if no DLL exists -> error !
        if (cookie == 0) {
            printf("DLL load of %s failed:",name);
            int i = GetLastError();
            char msgbuf[512]; *msgbuf = 0;
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,0,i,0,
            msgbuf, sizeof msgbuf, 0);
            printf(" NT/W9x error code %d:\n\t%s\n",i,msgbuf);
	    }

        //return 1 if dll exists, 0 otherwise
        return cookie != 0;
	}
    
    override ~Windows_DLL_load() { 
	// destructor - removes loaded DLL from mem.
        if (cookie) FreeLibrary(cookie); 
	cookie = 0;
	}


    /*********************************************************
    * void* get_proc_address(const char *procname) 
    *
    * Returns an address to a function with name (procname),
    * within a DLL.
    **********************************************************/
    override void* get_proc_address(const char *procname) {
        return GetProcAddress(cookie,procname);
	}
    };

DLL_load* DLL_load::new_DLL_load() { 
    return new Windows_DLL_load(); 
    }

#elif _LINUX || _SOL

#include <dlfcn.h>
struct UNIX_DLL_load : DLL_load {
    UNIX_DLL_load() {}
    void * cookie;
    override int load_DLL(const char *name) {
	#ifndef RTLD_GLOBAL
	// This not defined on sol/sparc.
	#define RTLD_GLOBAL 0
	#endif
	#ifndef RTLD_PARENT
	// Not defined on LINUX
	#define RTLD_PARENT 0
	#endif
	const char *old_name = name;
	cookie = dlopen(name,RTLD_PARENT|RTLD_GLOBAL|RTLD_LAZY); 
        if (cookie == 0 && strstr(name,".so") == 0) {
            char temp[1024];
            sprintf(temp,"%s.so",name);      //add .dll extension to file
            0 && printf("Trying to load %s\n",temp);
            cookie = dlopen(name,RTLD_PARENT|RTLD_GLOBAL|RTLD_LAZY); 
	    }

	return cookie != 0;
	}
    override ~UNIX_DLL_load() {
        // Unload the DLL.
	if (cookie) dlclose(cookie); 
	cookie = 0;
	}
    override void* get_proc_address(const char *procname) {
	return (void*) dlsym(cookie,procname);
	}
    };

DLL_load* DLL_load::new_DLL_load() { 
    return new UNIX_DLL_load(); 
    }

#endif
