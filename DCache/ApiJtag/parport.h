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
*            Created 1999 and Protected as an Unpublished Work
*                  Under the U.S. Copyright Act of 1976.
*                      Copyright 1999 ARC Cores Ltd
*                          All Rights Reserved
*
* File name:  parport.h       
* 
* Author   :  Stephane Bihan
*
* Date     :  28/01/99
*
* Description : Parallel port control layer header file for 95/NT DLL
*
*******************************************************************************/
#include "globals.h"

/* NT/95 parallel port register codes */
#define STATUS_REG_OFFSET  1
#define CONTROL_REG_OFFSET 2

/* parallel port pin names */
/* Control register */
#define C_XOR   0x0b  // XOR value with this to get all bits postive
                      // with respect to the signal values.
#define C_STR   0x01  // Strobe pin
#define C_CNT   0x02  
#define C_SS0   0x04
#define C_SS1   0x08
#define C_BI    0x20  // Direction pin

/* Status register */
#define S_XOR   0x80  // XOR value with this to get all bits postive
                      // with respect to the signal values.
#define S_SEL   0x10  // Select pin   
#define S_OP    0x20  // PaperEnd pin
#define S_ACK   0x40  // Ack pin
#define S_BUSY  0x80  // Busy pin


/*********************************************************
*    F U N C T I O N    P R O T O T Y P E S
**********************************************************/
int CheckFail();
int Until_Not_Busy();
void WriteByte(UBYTE value, UBYTE control );
#if _LINUX
void initialize_write_data();
int check_write_data(int flush);
#else
extern PioInstrStream stream;
#endif

