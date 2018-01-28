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
* File name:  parproto.cpp
*
* Author   :  Stephane Bihan
*
* Date     :  28/01/99
*
* Description : Parallel protocol layer file for 95/NT DLL
*
* This module is a part of the DLL interfacing the SeeCode debugger
* and the ARCAngel board via the parallel port.
* This module represents the protocol to access the communication port
* This DLL implementation works either with Windows95 or WindowsNT.
*
* This forms the middle layer and works with three other layers,
* the interface layer (api.cpp), the low level api (lowio.cpp) and the
* port control layer (parport.cpp).
*
* The parallel port protocol is described in the "ARC Interface Manual"
* and in the "ARC Development Board (ARCAngel)"
*
* History:
*
* Version  Date     Author       Description
*
* 0.0      24/04/95 S.Williams  File created.
* 1.0      01/07/95 S.Williams  Alpha release multiple processor ger.
* 1.2      24/09/96 Fuzzz       Mods for ARCAngel, reg + mem up/download.
* 2.0      28/01/99 S.Bihan     Split the original file
* 2.0      09/09/99 S.Bihan     Added Write/Read MADI registers
*          15/10/99 S.Bihan     Memory transfert improvment on NT
* 2.1      11/11/00 H.Lewis     Fixed bug in ResetBoard()
* 2.2      25/05/01 H.Lewis     Fixed ResetState() implementation (see ResetState() method)
*******************************************************************************/


/*********************************************************
* Include Files
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "gportio.h"
#include "globals.h"
#include "proto.h"
#include "parport.h"
#include "port.h"
#include <string.h>

/* access mode definition */
#define CR_MEM_R  0x00000000  // memory read access mode
#define CR_CORE_R 0x01000000  // core register read access mode
#define CR_AUX_R  0x02000000  // aux register read access mode
#define CR_MADI_R 0x03000000  // madi register read access mode
#define CR_MEM_W  0x00000000  // memory access mode
#define CR_CORE_W 0x01000000  // core register access mode
#define CR_AUX_W  0x02000000  // aux register access mode
#define CR_MADI_W 0x03000000  // madi register access mode

#define TEST_NUMBER     0x12345678 // to test the communication link
#define TIME_OUT  100              //number of tries allowed for reset_state(), before problem detected

extern void PCSend32(ULONG value, int control);
extern void PCGet32(ULONG *value);

#if _LINUX
#define use_extended_driver FALSE
#define use_buffered_write TRUE
#define Win_NT_Host FALSE
#else 
#define use_buffered_write Win_NT_Host
#endif

/*********************************************************
* Global Variables
**********************************************************/
struct Par_protocol: Protocol {
    Printf *print;
    override void receive_printf(Printf*p) {
        this->print = p;
	}
    override const char *id() { return "Parallel Port"; }

    UWORD epp_addr;

    override int get_port() { return epp_addr; }

    Par_protocol() {
        // Initial setting for debugging proto.
        #if DEBUG_PROTO
            dbg_proto = 1;
        #else
            dbg_proto = 0;
        #endif

        epp_addr = 0;
	}

    bool dbg_proto;
    static bool PORT_SETUP;

    override ~Par_protocol() {
        /* shutdown the port driver */
        if (PORT_SETUP)
            port->shutdown(), PORT_SETUP = FALSE;
	}


    /*********************************************************
    * Function : Read_ARC_Register
    *
    * Read specified arc register on slave device.
    * The read result is contained in *value.
    * return 0 if fail
    **********************************************************/
    override int Read_ARC_Register(
	    UBYTE reg_type, ULONG reg_no, ULONG *value ) {
        unsigned reg_spec;

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nRead_ARC_Register(%d,%d)",reg_type,reg_no);
	    }

        /* First - Set up PC port address to point to core or aux register */
        if ( reg_type == CORE_REGISTER )
	    reg_spec = (ULONG) CR_CORE_R + reg_no;
        else if (reg_no >= 0x01000000)
	    return ERR_REG_OUT_OF_RANGE;
	else
	    // reg_type = aux register
	    reg_spec = (ULONG) CR_AUX_R + reg_no;

        if (use_extended_driver) {
	    // printf("Using extended driver to read register %d!\n",reg_no);
	    int i = win32ArcRegRead(reg_spec, value);
	    if (dbg_proto) printInFile(" %x", *value);
	    return i;
	    }

        if (dbg_proto)
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto)
            printInFile("\nsend control register (cnt=0, ss0=0)");

	PCSend32( reg_spec, 0);   // cnt = 0

        /* Next, read back four bytes from the port */
        if (dbg_proto)
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;     // wait for busy line to go low

        if (dbg_proto)
            printInFile("\nread 4 bytes (cnt=1, ss0=1) :");

        PCGet32(value);

        if (dbg_proto)
            printInFile(" %x", *value);

        if (dbg_proto)
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto)
            printInFile("\ncheck for illegal access...");

        return CheckFail();
	}


    /*********************************************************
    * Function : Read_MADI_Register
    *
    * Read specified MADI register on slave device.
    * The read result is contained in *value.
    * return 0 if fail
    **********************************************************/
    int Read_MADI_Register(ULONG reg_no, ULONG *value ) {

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nRead_MADI_Register(%d,%d)",reg_no);
	    }

        /* First - Set up PC port address to point to core or aux register */
        if (dbg_proto)
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto)
            printInFile("\nsend control register (cnt=0, ss0=0)");

        PCSend32( (ULONG) CR_MADI_R + reg_no, 0);

        /* Next, read back four bytes from the port */
        if (dbg_proto)
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;     // wait for busy line to go low

        if (dbg_proto)
            printInFile("\nread 4 bytes (cnt=1, ss0=1) :");

        PCGet32(value);

        if (dbg_proto)
            printInFile(" %x", *value);

        if (dbg_proto)
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto) {
            printInFile("\ncheck for illegal access...");
	    }

        return CheckFail();
	}


    /*********************************************************
    * Function : Write_ARC_Register
    *
    * Write specified arc register on slave device.
    **********************************************************/
    int Write_ARC_Register(UBYTE reg_type, ULONG reg_no, ULONG new_value ) {
        unsigned reg_spec;

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nWrite_ARC_Register(%d,%d,%08x) \n",reg_type,reg_no,new_value);
	    }

        /* First - Set up PC port address to point to core or aux register */
        if ( reg_type == CORE_REGISTER )
	    reg_spec = (ULONG) CR_CORE_W + reg_no;
        else if (reg_no >= 0x01000000)
	    return ERR_REG_OUT_OF_RANGE;
	else
	    // reg_type = aux register
	    reg_spec = (ULONG) CR_AUX_W + reg_no;

        if (use_extended_driver) {
	    // printf("Using extended driver to write register %d!\n",reg_no);
	    return win32ArcRegWrite(reg_spec, new_value);
	    }

        if (dbg_proto) 
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto)
            printInFile("\nsend control register (cnt=0, ss0=0)");

	PCSend32( reg_spec, 0);   // cnt = 0

        /* Now, send four bytes to the port */
        if (dbg_proto)
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto)
            printInFile("\nsend 4 bytes (cnt=1, ss0=0) : 0x%x", new_value);

        PCSend32(new_value,1);                  // cnt = 1

        if (dbg_proto)
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto)
            printInFile("\ncheck for illegal access...");

        return CheckFail();
	}


    /*********************************************************
    * Function : Write_MADI_Register
    *
    * Write specified MADI register on slave device.
    **********************************************************/
    int Write_MADI_Register(ULONG reg_no, ULONG new_value ) {

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nWrite_MADI_Register(%d,%08x) \n",reg_no,new_value);
	    }

        /* First - Set up PC port address to point to core or aux register */
        if (dbg_proto)
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;  // wait for busy line to go low

        if (dbg_proto)
            printInFile("\nsend control register (cnt=0, ss0=0)");

        PCSend32( (ULONG) CR_MADI_W + reg_no, 0);

        /* Now, send four bytes to the port */
        if (dbg_proto)
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto)
            printInFile("\nsend 4 bytes (cnt=1, ss0=0) : 0x%x", new_value);

        PCSend32(new_value,1);      // cnt = 1

        if (dbg_proto)
            printInFile("\nensure busy line low...");

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto)
            printInFile("\ncheck for illegal access...");

        return CheckFail();
	}


    /*********************************************************
    * write_data - Write 32-bit word
    * Only on NT.
    **********************************************************/
    NOINLINE void write_data (ULONG addr, ULONG value) {
        int i;
        UBYTE val;

        /* write control and address first
        (cnt = 0, cnt & add reg access), (ss0 = 0: write) */

        for (i=0; i<4; i++) {
            val = (UBYTE) (((CR_MEM_W + addr) >> (i*8)) & 0xff);
            WriteByte(val, 0);
	    }

        /* write data (cnt=1: data access) (SS0=0: write data) */
        for (i=0; i<4; i++) {
            val = (UBYTE) ((value >> (i*8)) & 0xff);
            WriteByte(val, 1);
	    }
	}

    /*********************************************************
    * Function : MemWrite
    *
    * Write block of memory / register space on remote device.
    * Address and length must be multiples of 4.
    **********************************************************/
    int MemWrite( char *Source, ULONG Dest, ULONG Count) {

        int ret_val = 1;

        int eachWriteSize = 0;
        int written = 0;
        int *ibuf;
        int chunk = 4;

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nMemWrite %08x to %08x for %x\n",*Source,Dest,Count);
	    }

        if (use_extended_driver) {
            return win32ArcMemWrite((unsigned char *)Source, Dest, Count);
	    }

        if (use_buffered_write) {
            // We write memory block by directly programming the driver
            // because NT is too slow.
	    #if !_LINUX
            if (eachWriteSize == 0) {
                pioInitInstr_(stream);
                write_data(0,0);
                eachWriteSize = pioSize_(stream);
		}
            pioInitInstr_(stream);
	    #else 
	    initialize_write_data();
	    #endif

            ibuf = (int *) Source;

            while (Count >= 4) {
                write_data(Dest, *ibuf);

		#if !_LINUX
                if ((pioSize_(stream) + eachWriteSize) >= MAX_PIO_INSTR_BUF) {
                    gpioEval_(&stream, 0, 0); // execute the buffered write;
                    if (CheckFail() != 1) return 0;
                    pioInitInstr_(stream);
		    }
		#else
		if (check_write_data(FALSE))
		    if (CheckFail() != 1) return 0;
		#endif

                ibuf ++;
                Count -= chunk;
                Dest +=chunk;
                written += chunk;
		}

	    #if !_LINUX
            if (pioSize_(stream)) {
                gpioEval_(&stream, 0, 0);
                if (CheckFail() != 1) return 0;
		}
	    #else
	    if (check_write_data(TRUE) && CheckFail() != 1) return 0;
	    #endif

            return 1;
	    }

        while (Count > 0) {
            if ( Count >= 4 ) {
                ret_val |= MemWrite32(Source, Dest);
                Count -= 4 ;
                Source += 4 ;
                Dest += 4 ;
		}
	    }

        return ret_val;
	}


    /*********************************************************
    * Function : MemRead
    *
    * Write block of memory / register space on remote device.
    **********************************************************/
    int MemRead( char *Dest, ULONG Source, ULONG Count) {

        int ret_val = 1;

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nMemRead from %08x\n",Source);
	    }

        if (use_extended_driver) {
            return win32ArcMemRead((unsigned char *)Dest,
               Source, Count);
	    }

        while( Count > 0 ) {
            if ( Count >= 4 ) {
                ret_val |= MemRead32(Dest, Source);
                Count  -= 4 ;
                Dest   += 4 ;
                Source += 4 ;
		}
	    }

        return ret_val;
	}


    /*********************************************************
    * BASIC PROTOCOLS
    **********************************************************/

    /*********************************************************
    * MemWrite32() - Write four bytes into memory
    * return 1 iff succeded.
    **********************************************************/
    int MemWrite32(char *Source, ULONG Dest) {

        if (dbg_proto) {
            printInFile("\n************PROTO************");
            printInFile("\n write four bytes (%08x) to memory", *Source);
	    }

	if (use_extended_driver)
	    return win32ArcMemWrite((unsigned char *)Source, Dest, 4);

        /* First - Set up PC port address to point to memory */
        if (dbg_proto) {
            printInFile("\nensure busy line low...");
	    }

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        /* set the type of access into the address/control register */
        if (dbg_proto) {
            printInFile("\nsend control register (cnt=0, ss0=0)");
	    }

        PCSend32(CR_MEM_W + Dest, 0);         // cnt = 0

        /* Next, write the four bytes to the port */
        if (dbg_proto) {
            printInFile("\nensure busy line low...");
	    }

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto) {
            printInFile("\nsend 4 bytes (cnt=1, ss0=0) : 0x%x", *Source);
	    }

        PCSend32( *((ULONG *)Source), 1);

        if (dbg_proto) {
            printInFile("\nensure busy line low...");
	    }

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto) {
            printInFile("\ncheck for illegal access...");
	    }

        return CheckFail();
	}


    /*********************************************************
    * MemRead32() - Read four bytes from memory
    * return 1 iff succeded.
    **********************************************************/
    int MemRead32(char *Dest, ULONG Source) {

        if (dbg_proto) {
            printInFile("\n************PROTO************");
            printInFile("\n read four bytes from memory");
	    }

	if (use_extended_driver)
            return win32ArcMemRead((unsigned char *)Dest, Source, 4);

        /* First - Set up PC port address to point to memory */
        if (dbg_proto) {
            printInFile("\nensure busy line low...");
	    }

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        /* set the type of access into the address/control register */
        if (dbg_proto) {
            printInFile("\nsend control register (cnt=0, ss0=0)");
	    }

        PCSend32(CR_MEM_R + Source, 0);         // cnt = 0

        /* Next, read back four bytes from the port */
        if (dbg_proto) {
            printInFile("\nensure busy line low...");
	    }

        if (Until_Not_Busy()!=1)
            return(ERR_BUSY);   // wait for busy line to go low

        if (dbg_proto) {
            printInFile("\nread 4 bytes (cnt=1, ss0=1) :");
	    }

        PCGet32((ULONG *) Dest);


        if (dbg_proto) {
            printInFile(" %x", *Dest);
	    }

        if (dbg_proto) {
            printInFile("\nensure busy line low...");
	    }

        if (Until_Not_Busy()!=1)
            return ERR_BUSY;    // wait for busy line to go low

        if (dbg_proto) {
            printInFile("\ncheck for illegal access...");
	    }

        return CheckFail();
	}


    /*********************************************************
    * ResetBoard()
    *
    * Take the ss1 line low for one second,
    * Then bring it high again
    **********************************************************/
    override void ResetBoard() {
	dbg_proto && printf("Reset board being called!\n");
        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nResetBoard() \n");
	    }

        //SOLUTION TO PREVIOUS PROBLEM !!!!
        //The line below has been added, to ensure the reset command (line 2) is executed
        //with the board in the correct state.
        //When a write reg/mem is the last command issued (current or
        //previous debug session) before a reset, the C_SS1 | C_CNT | C_STR lines would
        //have been set to true, C_SS0 will have been set to false (C_SS0 line
        //indicates a write is taking place).
        //Attempting to reset in this state was causing the angel's config to become unstable
        //after the reset -- bug responsible for unexplained losses of board's config, after a reset!!
        //When ss0 and ss1 are both set low, signals are directed to 'serial download pins' instead
        //of 'parallel data lines', this may be related to past problem.
        port->outp((UWORD) (epp_control_reg_addr), (C_SS1 | C_SS0 | C_CNT | C_STR)^C_XOR);

        //Reset the board by taking SS1 line low
        port->outp((UWORD) (epp_control_reg_addr), (C_SS0 | C_CNT | C_STR)^C_XOR);
	os->sleep(200); // TBH 18 JUN 2003  needed by slower simulations

        //Take the SS1 line high again (put board back into normal state)
        port->outp((UWORD) (epp_control_reg_addr), (C_SS1 | C_SS0 | C_CNT | C_STR)^C_XOR);

	}


    /*********************************************************
    * Initialize and test communication links
    **********************************************************/
    int Test_Comms_Link() {
        int return_val;
        ULONG test;
        ULONG reg_value;
	port->helpful_messages();

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nTest_Comms_Link() \n");
	    }

        if (gverbose) print->printf("%s:Testing port 0x%x...",id(),epp_addr);
        /* save the contents of r0 first */
        return_val = Read_ARC_Register(CORE_REGISTER,0, &reg_value);

        if (return_val!=1)
            return return_val;

        return_val=Write_ARC_Register(CORE_REGISTER,0,TEST_NUMBER);

        if (return_val!=1)
            return return_val;

        return_val=Read_ARC_Register(CORE_REGISTER,0, &test);

        if (return_val!=1)
            return return_val;

        /* restore r0 */

        Write_ARC_Register(CORE_REGISTER,0,reg_value);

        port->outp((UWORD) (epp_control_reg_addr),
		(C_SS1 | C_SS0 | C_CNT | C_STR)^C_XOR);

        if ( test==TEST_NUMBER ) {
            if (gverbose) print->printf("OK\n");

            return 1;    //success
	    }
        else
            return ERR_COMM;   //failure
	}


    /*********************************************************
    * Reset the machine state
    **********************************************************/
    // This method was not implemented correctly as the C_SS1 AND C_SS0
    // pins were set low, meaning that the port signals would point to
    // the serial download pins ready for a new configuration to be
    // downloaded (see commented code in ResetState()).  The point of
    // the method was to repeatably set the strobe high, therefore
    // attempting to finish off any previously unfinished transactions.
    // In addition the C_SS1, C_SS0 and C_CNT lines are set high to
    // correct this method (along with C_STR). Also continuous toggling
    // of the strobe line was not being achieved, the strobe is now
    // continuously set low and high (see correct implementation).

    NOINLINE void ResetState() {
	dbg_proto && printf("Reset state being called!\n");
        //INCORRECT RESET_STATE IMPLEMENTATION !!!
        //while (Until_Not_Busy()!=1)
        //{
            //port->outp((UWORD) (epp_control_reg_addr), C_STR^C_XOR);
        //}

        //number of times resetstate is attempted
        int resetAttempts = 0;

        //CORRECT IMPLEMENTATION
        while(Until_Not_Busy() != 1) {
            printf("Board still busy - attempting to Reset State !\n");

            if(resetAttempts > TIME_OUT) {
                //ERROR !
                printf("Error : Could not ResetState(), board trapped in busy state\n");

                //tidy up state by resetting the board before terminating debug session.
                ResetBoard();

                //terminate
                exit(1);
		}

            //set strobe low
            port->outp((UWORD) (epp_control_reg_addr), (C_CNT | C_SS0 | C_SS1)^C_XOR);

            //set strobe high
            port->outp((UWORD) (epp_control_reg_addr), (C_CNT | C_SS0 | C_SS1 | C_STR)^C_XOR);

            //increment number of attempts
            resetAttempts ++;
	    }

        //SUCCESS (if here) !
	}


    override int process_property(const char *property, const char *value) {
        /* set up the port driver */
        if (!os->stricmp(property,"port")) {
            if (PORT_SETUP == FALSE) {
                PORT_SETUP = TRUE;
                epp_addr = strtoul(value,0,0);
                // printf("Setup port driver, port address = 0x%x.\n", epp_addr);
                int return_val;

                if ((return_val = port->setup(epp_addr, 4))!=1) {
                    printf("\n*** Can't setup the port! *** \n");
                    return return_val;
		    }

                // we reset the parallel port state machine

		// This is wrong.  If the board isn't blasted, this fails.
		// We have to do this later.
		// The api does it when we prepare_for_new_program.
                // ResetState();
		}

	    }
        else if (!os->stricmp(property,"dbg_proto")) {
	    dbg_proto = *value == '1';
	    }
	else if (!os->stricmp(property,"dbg_port")) {
	    dbg_port = *value == '1';
	    }
	else if (!os->stricmp(property,"new_driver")) {
	    // Allow this DLL to shut off use of the new driver's facilities.
	    #if !_LINUX
	    use_extended_driver = *value == '1' &&
	    	driver_major_version >= 2;
	    #endif
	    }
	else return 0;
	return 1;
	}
    };



bool Par_protocol::PORT_SETUP = FALSE;

Protocol *get_protocol() { return new Par_protocol(); }
