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
*            Created 2000 and Protected as an Unpublished Work
*                  Under the U.S. Copyright Act of 1976.
*                Copyright 2000 ARC International(UK) Ltd
*                          All Rights Reserved
*
*
* File name:          lowarc.cpp
* 
* Author:             Tom Pennello
*
* Creation Date:      15 Oct 2000
*
* Description:
*
*       This file is used to include a get_ARC_interface to a DLL 
*       which may only contain a low level API (will only have get_low_ARC).
*       This will allow the low level API to now link to an API DLL.
* 
*
*  History:
*
*   Version    Date         Author           Description
*
*   1.0        15/10/00     Tom Pennello     File created. 
*
*******************************************************************************/


// #include "arcint.h"

struct ARC;

#include "low.h"

extern "C" __declspec(dllexport) ARC *get_ARC_interface() 
{
    return get_low_ARC();
}
