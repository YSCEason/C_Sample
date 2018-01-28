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
* File name:  proto.h         
* 
* Author   :  Tom Pennello
*
* Date     :  15/10/00
*
* Description : Common attributes from JTAG + PAR protocol layers.
*
*******************************************************************************/
#include "globals.h"

// Protocol class.  This could be implemented with yet another ARC
// DLL, a simpler one that the customer could provide.
struct Protocol {
    virtual int Read_ARC_Register( UBYTE reg_type, ULONG reg_no, ULONG *value ) = 0;
    virtual int Read_MADI_Register(ULONG reg_no, ULONG *value ) = 0;
    virtual int Write_ARC_Register(UBYTE reg_type, ULONG reg_no, ULONG new_value ) = 0;
    virtual int Write_MADI_Register(ULONG reg_no, ULONG new_value ) = 0;
    virtual int MemWrite( char *Source, ULONG Dest, ULONG Count) = 0;
    virtual int MemRead( char *Dest, ULONG Source, ULONG Count) = 0;
    virtual void ResetBoard() = 0;
    virtual int Test_Comms_Link() = 0;
    virtual int MemWrite32(char *Source, ULONG Dest) = 0;
    virtual int MemRead32(char *Dest, ULONG Source) = 0;
    virtual void ResetState() = 0;
    virtual int process_property(const char *key, const char *value) = 0;
    virtual const char *id() = 0;
    virtual int get_port() = 0; // Solely for blast.
    virtual void receive_printf(Printf*p) = 0;
    virtual ~Protocol() {}
    };

/* ARC register constants used by all prototypes. */
#define CORE_REGISTER 1
#define AUX_REGISTER  2
#define MADI_REGISTER 3

// Function that gets the protocol, dynamically.
// You must link in this function.
extern Protocol *get_protocol();
