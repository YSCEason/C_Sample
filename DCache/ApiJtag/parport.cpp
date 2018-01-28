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
*            Created 2000-2004 and Protected as an Unpublished Work
*                  Under the U.S. Copyright Act of 1976.
*                Copyright 2000-2004 ARC International(UK) Ltd
*                          All Rights Reserved
*
* File name:  parport.cpp
*
* Author   :  Stephane Bihan
*
* Date     :  28/01/99
*
* Description : Parallel control layer file for 95/NT DLL
*
* This module is a part of the DLL interfacing the SeeCode debugger
* and the ARCAngel board via the parallel port. This part controls
* and accesses the port. You will find documentation on how to programm the
* Gportio driver in the MetaWare documentation and the gportio.h and Pioeval.h files
* This DLL implementation works either with Windows95
* or WindowsNT
*
* This forms the lower layer and works with three other ones,
* the interface layer (api.cpp), the low level api (lowio.cpp) and the
* protocol layer (parproto.cpp).
*
* For documentation on how to program the Gportio driver, see Gportio.h file
*
* History:
*
* Version  Date     Author       Description
*
* 0.0      24/04/95 S.Williams  File created.
* 1.0      01/07/95 S.Williams  Alpha release multiple processor ger.
* 1.1      04/07/95 S.Williams  Added this header.
*          13/07/95 S.Williams  Added 'missing' set debug addr. reg. command
*                               in REMOTE_Read_ARC_Register.
* 1.2      24/09/96 Fuzzz       Mods for ARCAngel, reg + mem up/download.
* 2.0      28/01/99 S.Bihan     Split the original file
*          15/10/99 S.Bihan     Improved memory download on NT
*
*******************************************************************************/


/*********************************************************
* Include Files
**********************************************************/
#include <stdio.h>
#include "pioeval.h"
#include "gportio.h"
#include "parport.h"
#include "port.h"

#define AA_TIMEOUT    200 // number of retries on ack/busy lines

/*********************************************************
* Global Variables
**********************************************************/
/*********************************************************
* function to check the status of the op (fail) signal by PC
* to determine whether the write was accepted or not.
* Returns 1 for fail OK, 0 for fail line set.
**********************************************************/
int CheckFail() {

    if (dbg_port) {
        printInFile("\n\t\t------------PORT------------");
        printInFile("\n\t\tCheckFail()");
	}
    if (!port->OK()) return ERR_PORT_ERROR;

    UBYTE ival = (UBYTE) port->inp( (UWORD) (epp_status_reg_addr)) ^ S_XOR;

    if ((ival&S_OP) == 0) return 1;
    else return ERR_CHECK_FAIL;

    }


/*********************************************************
* Until_Not_Busy : Wait until the board's busy line goes low
* Returns 0 if busy was low, -1 if we timed out.
**********************************************************/
int Until_Not_Busy() {
    if (dbg_port) {
        printInFile("\n\t\t------------PORT------------");
        printInFile("\n\t\tUntil_Not_Busy()");
	}

    for (int i = 0; i < AA_TIMEOUT; i++) {
        /* wait for busy to go low */

        /* get status value and fix bits which are inverted */
        UBYTE status_val = port->inp( (UWORD) (epp_status_reg_addr))^S_XOR;

        if ((status_val & S_BUSY) == 0)
            return 1;                  // success!
	}

    return ERR_BUSY; // fail!
    }


/*********************************************************
* ReadStrobe : Receive a single byte from the PC link
*
* cnt,ss0,ss1 are always 1.
*
* Set port into port.input mode.
* Take strobe low,
* Wait for ack to go low - if it's not low already
* read data
* Take strobe high,
*
**********************************************************/
static UBYTE ReadStrobe() {
    if (dbg_port) {
        printInFile("\n\t\t------------PORT------------");
        printInFile("\n\t\tReadStrobe()");
	}

    int i;
    UBYTE status_val,ret_val;
    slow_clock_loop();

    port->outp( (UWORD) (epp_control_reg_addr),     // set strobe high, port = in
        (C_BI | C_SS1 | C_SS0 | C_CNT | C_STR) ^ C_XOR);

    slow_clock_loop();
    port->outp( (UWORD) (epp_control_reg_addr),      // set strobe low, port = in
        (C_BI | C_SS1 | C_SS0 | C_CNT) ^ C_XOR); // this causes the read access to be started

    for (i = 0; i < AA_TIMEOUT; i++) {
        /* wait for ack to go low */
        /* get status value and fix bits which are inverted */
        status_val = port->inp((UWORD) (epp_status_reg_addr))^S_XOR;

        if ((status_val & S_ACK) == 0)
            break;
	}

    if ( i == AA_TIMEOUT )
        printf("\nTimeout (%d) on ReadStrobe() - %08x ",i,status_val);

    ret_val = port->inp(epp_addr);
    slow_clock_loop();

    /* set strobe high, port = in */
    port->outp( (UWORD) (epp_control_reg_addr),
        (C_BI | C_SS1 | C_SS0 | C_CNT | C_STR) ^ C_XOR);

    return((UBYTE)ret_val);
    }


/*********************************************************
* WriteStrobe : Send a single byte down the PC link
*
* cnt is set according to the value in 'control'.
* ss0 = 0 (for write)
* ss1 = 1 (no reset)
*
* Set port into port.output mode.
* Take strobe low, set cnt, ss0, ss1 to the appropriate values.
* Take strobe high, leaving cnt=control, ss0=0
**********************************************************/
static void WriteStrobe(UBYTE value, UBYTE control ) {
    if (dbg_port) {
        printInFile("\n\t\t------------PORT------------");
        printInFile("\n\t\tWriteStrobe(0x%x, 0x%x)",value,control);
	}


    UBYTE oval = C_SS1 | (control*C_CNT);
    unsigned char outbuf[6];
    int length=0;


#if _LINUX
    #define STROBE_VALUES(byte,control) \
        /* set strobe high, port = out */     \
        {epp_control_reg_addr,((oval | C_STR) ^ C_XOR) }, \
        /* set strobe low, port = out */     \
        {epp_control_reg_addr,(oval ^ C_XOR)}, \
        {epp_addr,byte}, \
        {epp_addr,byte},  /* conservatively allow extra time */ \
	/* set strobe high, port = out    */ \
        {epp_control_reg_addr, \
            (C_SS1 | (control*C_CNT) | C_STR) ^ C_XOR },  \

    // Also this has been back-substituted
    GPIO_ARRAY_STRUCT A[] = {
	STROBE_VALUES(value,control)
	{0,0,0}
        };
    win32OutpArray(A,sizeof(A));
#else
    if (Win_NT_Host) {
        /* init nt port driver instruction stream */
        pioInitInstr();
        /* add port writes to insruction buffer */
        outbuf[length]=((oval | C_STR) ^ C_XOR);
        pioConst((UBYTE)((oval | C_STR) ^ C_XOR));
        pioOutp(CONTROL_REG_OFFSET);
        length++;

        outbuf[length]=(oval ^ C_XOR);
        pioConst((UBYTE)(oval ^ C_XOR) );
        pioOutp(CONTROL_REG_OFFSET);
        length++;

        outbuf[length]= value;
        pioConst(value);
        pioOutp(0);
        length++;

        outbuf[length]=((C_SS1 | (control*C_CNT) | C_STR) ^ C_XOR);
        pioConst((UBYTE)((C_SS1 | (control*C_CNT) | C_STR) ^ C_XOR));
        pioOutp(CONTROL_REG_OFFSET);
        length++;

        /* execute outport instrution stream */
        gpioEval(outbuf,&length);
	}
    else  { // 95/98
        /* set strobe high, port = out */
	slow_clock_loop();
        port->outp((UWORD) (epp_control_reg_addr),((oval | C_STR) ^ C_XOR));

        /* set strobe low, port = out */
	slow_clock_loop();
        port->outp((UWORD) (epp_control_reg_addr),(oval ^ C_XOR));

	slow_clock_loop();
        port->outp( epp_addr,value);

        port->outp(epp_addr,value);  // conservatively allow extra time

        port->outp((UWORD) (epp_control_reg_addr),
            (C_SS1 | (control*C_CNT) | C_STR) ^ C_XOR); // set strobe high, port = out
	}
#endif

    }


#if _LINUX
static const int GSIZE=2048;
static GPIO_ARRAY_STRUCT output[GSIZE];
static int output_cnt = 0;
void initialize_write_data() { output_cnt = 0; }
static void add_gpio(int offset, int value) {
    output[output_cnt].port = epp_addr + offset;
    output[output_cnt].data = value;
    output_cnt++;
    }
int check_write_data(int flush) {
    // Return 1 if we wrote something, 0 otherwise.
    if (output_cnt > GSIZE - 50 || (flush && output_cnt)) {
	add_gpio(0,0);
	0 && printf("!flushing, size %d\n",output_cnt);
	win32OutpArray(output,output_cnt*sizeof(*output));
	output_cnt = 0;
	return 1;
	}
    return 0;
    }

void WriteByte(UBYTE value, UBYTE control) {
    if (dbg_port) {
        printInFile("\n\t\t------------PORT------------");
        printInFile("\n\t\tWriteByte(0x%x, 0x%x)",value,control);
	}

    UBYTE oval = C_SS1 | (control*C_CNT);

    /* set strobe high */
    add_gpio(CONTROL_REG_OFFSET,((oval | C_STR) ^ C_XOR));

    /* set strobe low (write access start) */
    add_gpio(CONTROL_REG_OFFSET, (oval ^ C_XOR) );

    /* write data byte */
    add_gpio(0, value);

    /* set strobe high */
    add_gpio(CONTROL_REG_OFFSET,(C_SS1 | (control*C_CNT) | C_STR) ^ C_XOR);
    }
#else
/*********************************************************
* WriteByte - Write a byte by programming the driver
* dynamically. Used for memory download on NT
**********************************************************/
PioInstrStream stream;  // Defining occurrence here.

void WriteByte(UBYTE value, UBYTE control) {
    if (dbg_port) {
        printInFile("\n\t\t------------PORT------------");
        printInFile("\n\t\tWriteByte(0x%x, 0x%x)",value,control);
	}

    UBYTE oval = C_SS1 | (control*C_CNT);

    /* set strobe high */
    pioConst_(stream, ((oval | C_STR) ^ C_XOR));
    pioOutp_(stream, CONTROL_REG_OFFSET);

    /* set strobe low (write access start) */
    pioConst_(stream, (oval ^ C_XOR) );
    pioOutp_(stream, CONTROL_REG_OFFSET);

    /* write data byte */
    pioConst_(stream, value);
    pioOutp_(stream, 0);

    /* set strobe high */
    pioConst_(stream, (C_SS1 | (control*C_CNT) | C_STR) ^ C_XOR);
    pioOutp_(stream, CONTROL_REG_OFFSET);

    }
#endif


/*********************************************************
* PCSend32 - Send four bytes down the link
* after the address/control register has been set up
**********************************************************/
void PCSend32(ULONG value, int control) {
    if (dbg_port) {
        printInFile("\n\t\t------------PORT------------");
        printInFile("\n\t\tPCSend32(0x%x, %d)",value,control);
	}

    // Horrors!  Sleep(0) takes a LONG time in Windows, even if you
    // are sleeping 0.  It makes for poor performance of this DLL.
    // Sleep(0);
    // Actually I think Sleep(0) on Windows means "relinquish my current time slice".
    #if _LINUX
    UBYTE oval = C_SS1 | (control*C_CNT);
    GPIO_ARRAY_STRUCT A[] = {
	STROBE_VALUES(value   & 0xff, control)
	STROBE_VALUES(value>>8  & 0xff, control)
	STROBE_VALUES(value>>16 & 0xff, control)
	STROBE_VALUES(value>>24 & 0xff, control)
	0,0,0};
    win32OutpArray(A,sizeof(A));
    #else
    WriteStrobe( (UBYTE) (value   & 0xff), (UBYTE) control);
    WriteStrobe( (UBYTE) (value>>8  & 0xff), (UBYTE) control);
    WriteStrobe( (UBYTE) (value>>16 & 0xff), (UBYTE) control);
    WriteStrobe( (UBYTE) (value>>24 & 0xff), (UBYTE) control);
    #endif
    }


/*********************************************************
* PCGet32 - Get four bytes from the link
**********************************************************/
void PCGet32(ULONG *value) {
    if (dbg_port) {
        printInFile("\n\t\t------------PORT------------");
        printInFile("\n\t\tPCGet32(0x%x)",value);
	}

    *value =  0;
    // Sleep(0);
    *value |= (ReadStrobe() & 0xff);
    *value |= (ReadStrobe() & 0xff) << 8;
    *value |= (ReadStrobe() & 0xff) << 16;
    *value |= (ReadStrobe() & 0xff) << 24;
    }
