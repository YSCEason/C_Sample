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
* File name:          dll.h
* 
* Author:             Tom Pennello
*
* Creation Date:      15 Oct 2000
*
* Description:
*
*   This header file contains a DLL_load struct definition. 
* 
*
*  History:
*
*   Version    Date         Author           Description
*
*   1.0        15/10/00     Tom Pennello     File created. 
*
*******************************************************************************/

struct DLL_load {
    virtual ~DLL_load() = 0;
    // Returns non-zero if the load succeeded.
    virtual int load_DLL(const char *name) = 0;
    // Returns the address of the procedure.
    virtual void* get_proc_address(const char *procname) = 0;
    static DLL_load* new_DLL_load();
    };

