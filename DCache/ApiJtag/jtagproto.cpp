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
*          Created 2000-2004 and Protected as an Unpublished Work
*                  Under the U.S. Copyright Act of 1976.
*             Copyright 2000-2004 ARC International(UK) Ltd
*                          All Rights Reserved
*
* File name:    Jtagproto.cpp
*
* Author   :    Stephane Bihan
*
* Date     :    19/02/99
*
* Description : Jtag protocol layer file for 95/NT DLL
*
*   This module is a part of the DLL interfacing the SeeCode debugger
*   and the ARCAngel board via the pseudo serial port (using 4 pins of
*   the parallel port) for Jtag module on ARCAngel.
*   This module represents the protocol to access the communication port
*   This DLL implementation works either with Windows95 or WindowsNT.
*
*   This forms the 3rd layer of the DLL and works with three other ones,
*   the interface layer (api.cpp), the low level interface layer (lowio.cpp)
*   which are above it. Also 1 layer below it, the port control
*   layer (jtagport.cpp)
*
*   The original file "remote.c" has been split in layers.
*
*   (See "The debug components reference" document, chapter 3.  For
*    information on JTAG.)
*
* History:
*
* Version  Date     Author       Description
*
* 1.0      24/04/95 Fuzzz / Alex Holland / Nick Ross    File created.
* 1.1      14/11/00 Huw Lewis    Improved commenting, and neatness of code
* 1.2      25/05/01 Huw Lewis    Added an implmentation for ResetState() (see ResetState())
*******************************************************************************/

/********************************************************************************************
 * NOTE (Huw Lewis 14/11/00):
 ****************************************************************************
 *  If "RANDOM" errors occur when using JTAG (HostLink inefficiency/ (r/w) memory problems),
 *  most likely not a DLL problem.  These RANDOM problems are specific to Altera ARCAngels,
 *  and should not occur using any other target, including Xilinx ARCAngels !!!
 *******************************************************************************************/

/*********************************************************
* Include Files
**********************************************************/
#include <stdio.h>
#include <stdlib.h>  // TBH
#include <string.h>

#include "gportio.h"
#include "globals.h"
#include "proto.h"
#include "jtagport.h"
#include "port.h"


/*********************************************************
* Global Variables
**********************************************************/

/* JTAG transaction codes */
#define MEM_WRITE           0x00000000 /* 0000 */
#define ARC_CORE_WRITE      0x00000001 /* 0001 */
#define ARC_AUX_WRITE       0x00000002 /* 0010 */
#define ARC_MADI_WRITE      0x00000007 /* 0111 */

#define MEM_READ            0x00000004 /* 0100 */
#define ARC_CORE_READ       0x00000005 /* 0101 */
#define ARC_AUX_READ        0x00000006 /* 0110 */
#define ARC_MADI_READ       0x00000008 /* 1000 */


int jtag_reset_always = 0;  // TBH  13 JUN 2002 for reset of JTAG on every Until_not_busy.
int jtag_retry_base_number = 1;  // TBH  27 JUN 2002 for setable number of loops.
int optimize_jtag = 1;  // Don't send TDI/TMS if the same as last time.

/*********************************************************
* HIGH LEVEL FUNCTIONS
**********************************************************/
struct Jtag_protocol: Protocol {
    override const char *id() {
        return "JTAG Port"; 
    // Note: api.cpp looks for string "JTAG" to make MADI decisions.
    }
    Printf *printf;
    override void receive_printf(Printf*p) {
        this->printf = p;
    jtag->receive_printf(p);
    }

    UWORD epp_addr;

    override int get_port() {
        return epp_addr;
    }

    Jtag_protocol() {
        // Initial setting for debugging proto.
        #if DEBUG_PROTO
            dbg_proto = 1;
        #else
            dbg_proto = 0;
        #endif
    jtag = JTAG_handler::make_JTAG_handler();
    addr_increment_needed = false;
    a7_addr_increment = false;  // HW has been fixed.
    read_ident_reg = false;
    read_slow = false;
    this_is_a7 = false;
    successful_aux_reads = 0;
    }
    JTAG_handler *jtag;

    bool addr_increment_needed;
    bool a7_addr_increment;
    bool read_ident_reg;
    bool dbg_proto;
    bool read_slow;
    bool this_is_a7;
    unsigned successful_aux_reads;
    static bool PORT_SETUP;

    override ~Jtag_protocol() {
        /* shutdown the port driver */
        if (PORT_SETUP)
            port->shutdown(), PORT_SETUP = false;
    delete jtag;
    }


    const char *reg_type_string(UBYTE reg_type) {
        switch(reg_type) {
        case CORE_REGISTER: return "core";
        case AUX_REGISTER: return "aux";
        case MADI_REGISTER: return "madi";
        default: return "???";
        }
    }

    /*********************************************************
    * Function : Read_ARC_Register
    *
    * Read specified arc register on slave device.
    * The read result is contained in *value.
    * return 0 if fail
    **********************************************************/
    override int Read_ARC_Register
        (UBYTE reg_type, ULONG reg_no, ULONG *value ) {

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nRead_ARC_Register(%d,%d,%08x)",reg_type,reg_no,value);
        }

        // First - Set up JTag port address to point to core or aux register

        //Move to Select_IR_scan state (current state = RUN_TEST_IDLE_STATE)
    jtag->Select_Load_Write(RUN_TEST_IDLE_STATE,ADDRESS_REGISTER, 
        reg_no, SELECT_DR_SCAN_STATE);

        //Move back to Select_IR_scan state (current state = SELECT_DR_SCAN_STATE)
        if ( reg_type == CORE_REGISTER ) 
        jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,TRANSACTION_REGISTER, 
            ARC_CORE_READ, RUN_TEST_IDLE_STATE);
        else if (reg_type == AUX_REGISTER) 
        jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,TRANSACTION_REGISTER, 
            ARC_AUX_READ, RUN_TEST_IDLE_STATE);
    // We should be in RUN_TEST_IDLE_STATE.
    if (read_slow) os->sleep(1);

    bool a7_error_silence = this_is_a7;
    unsigned err_cnt = jtag->get_error_count();
    jtag->set_error_silence(a7_error_silence);
        // Wait for all the data to be in the JTAG
        if (jtag->until_not_busy("read_register")!=1) {
			if (a7_error_silence && jtag->get_error_count() > err_cnt &&
			successful_aux_reads) {
				// printf->printf("Probably this aux reg doesn't exist.\n");
			*value = 0xdeadbeef;
			return ERR_BUSY;//CJ++change from 0 to ERR_BUSY // Failure.  See what SeeCode does.
			}
			//else //CJ--
				//CJ-- printf->printf("  read  of %s register 0x%x failed.\n", reg_type_string(reg_type),reg_no);
            return ERR_BUSY;
        }
    jtag->set_error_silence(false);

    *value = read_data_register();
    invalidate_transaction_register();
    if (reg_type == AUX_REGISTER) successful_aux_reads++;
    if (reg_type == AUX_REGISTER && reg_no == 4 /* IDENTITY */) {
        read_ident_reg = true;
        this_is_a7 = (*value&0xff) >= 0x31;
        if (this_is_a7 && !addr_increment_needed && a7_addr_increment) {
            if (0) {
        ::printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            ::printf("*** !TEMPORARY! ARC700 NEEDS ADDRESS "
            "INCREMENT ON BLOCK MEMORY READ/WRITE.\n");
        ::printf("*** To disable this, specify -off=a7_addr_increment.\n");
        ::printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
        }
        addr_increment_needed = true;
        }
        }
    0 && ::printf("!just read arc register 0x%x type %d, result 0x%x\n",reg_no,
        reg_type,*value);
    // if (reg_no == 10 && reg_type == AUX_REGISTER) { char *p = 0; *p = 0; }
        return 1;
    }

    NOINLINE unsigned read_data_register() {
        // Read back four bytes.
        // Put in select_IR_scan_state; 
    // current state should be SELECT_DR_SCAN_STATE from
        // previous call to inside until_not_busy().
        ULONG value;
    jtag->Select_IR_scan(SELECT_DR_SCAN_STATE);
        jtag->load_IR(DATA_REGISTER, SELECT_DR_SCAN_STATE);
        jtag->read_DR(DATA_REGISTER, &value, SELECT_DR_SCAN_STATE);
    return value;
    }

    NOINLINE void invalidate_transaction_register() {
        // Invalidate the transaction register.  Otherwise, next time
    // we load the IR with the xaction register and attempt to
    // load it with a value, the previous command will be executed!
    // 3 is an invalid command.
    jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,TRANSACTION_REGISTER, 
        0x00000003, RUN_TEST_IDLE_STATE);
    }

    /*********************************************************
    * Function : Read_MADI_Register
    *
    * Read specified MADI register on slave device.
    * The read result is contained in *value.
    * return 0 if fail.
    * The MADI lock must not be seen by the debugger.
    **********************************************************/
    override int Read_MADI_Register(ULONG reg_no, ULONG *value ) {

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nRead_MADI_Register(madi,%d,%08x)",reg_no,value);
        }

        //First - Set up JTag port address to point to core or aux register

        //Ensure in Select_IR_scan state (current state = RUN_TEST_IDLE_STATE)
    jtag->Select_Load_Write(RUN_TEST_IDLE_STATE,ADDRESS_REGISTER, 
        reg_no, SELECT_DR_SCAN_STATE);

    jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,TRANSACTION_REGISTER, 
        ARC_MADI_READ, RUN_TEST_IDLE_STATE);

        // Wait for the data to be in the JTAG
        if (jtag->until_not_busy("read_MADI_register")!=1)
            return ERR_BUSY;

    *value = read_data_register();
    invalidate_transaction_register();
        return 1;
    }


    /*********************************************************
    * Function : Write_ARC_Register
    *
    * Write specified arc register on slave device.
    **********************************************************/
    override int Write_ARC_Register
        (UBYTE reg_type, ULONG reg_no, ULONG new_value ) {

    // char *p = 0; *p = 0;
if (0) {ULONG pc; Read_ARC_Register(AUX_REGISTER, 6, &pc);}
    0 && ::printf("!write arc register 0x%x\n",reg_no);
        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nWrite_ARC_Register(%d,%d,%08x) \n",reg_type,reg_no,new_value);
        }


        //First - Set up PC port address to point to core or aux register
    jtag->Select_Load_Write(RUN_TEST_IDLE_STATE,ADDRESS_REGISTER, 
        reg_no, SELECT_DR_SCAN_STATE);

        //Now, send four bytes to the port
    jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,DATA_REGISTER, 
        new_value, SELECT_DR_SCAN_STATE);

        //execute write

        if ( reg_type == CORE_REGISTER ) {
        jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,TRANSACTION_REGISTER, 
        ARC_CORE_WRITE, RUN_TEST_IDLE_STATE);
        }
        else if (reg_type == AUX_REGISTER) {
        jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,TRANSACTION_REGISTER, 
        ARC_AUX_WRITE, RUN_TEST_IDLE_STATE);
        }

        if (jtag->until_not_busy("write_register")!=1) {
        //CJ-- printf->printf("  write of %s register 0x%x failed.\n", reg_type_string(reg_type),reg_no);
            return ERR_BUSY;
        }

    invalidate_transaction_register();

        return 1;
    }


    /*********************************************************
    * Function : Write_MADI_Register
    *
    * Write specified MADI register on slave device.
    **********************************************************/
    override int Write_MADI_Register(ULONG reg_no, ULONG new_value ) {

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nWrite_MADI_Register(madi,%d,%08x) \n",reg_no,new_value);
        }

    // If multiple jtag in chain, this must be non-MADI version
    // of ARC.
    #define reg_MCR 0   // MADI control register.
    bool multi_jtag = 
        // If we are updating "which ARC", tell jtag to do it
        // directly.
        reg_no == reg_MCR && jtag->switch_madi((new_value & 0xff) + 1);

    if (multi_jtag) return 1;
        //First - Set up PC port address to point to core or aux register
    jtag->Select_Load_Write(RUN_TEST_IDLE_STATE,ADDRESS_REGISTER, 
        reg_no, SELECT_DR_SCAN_STATE);

        //Now, send four bytes to the port
    jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,DATA_REGISTER, 
        new_value, SELECT_DR_SCAN_STATE);

        //execute write
    jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,TRANSACTION_REGISTER, 
        ARC_MADI_WRITE, RUN_TEST_IDLE_STATE);

        if (jtag->until_not_busy("write_MADI_register")!=1)
            return ERR_BUSY;

    invalidate_transaction_register();

        return 1;
    }


    NOINLINE void read_identity_register() {
        unsigned long value;
    Read_ARC_Register(AUX_REGISTER, 4, &value);
        }

    /*********************************************************
    * Function : MemWrite
    *
    * Write block of memory / register space on remote device.
    **********************************************************/
    override int MemWrite( char *Source, ULONG Dest, ULONG Count) {

        int ret_val = 1;
        unsigned long i;

        char Value[4] ;

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nMemWrite %08x to %08x\n",*Source,Dest);
        }
    if (!read_ident_reg) read_identity_register();

        if (Count >= 4) {

            // (Quick version uses the NEW JTAG autoincrementing address)
        // First - Set up PC port address to point to memory
        jtag->Select_Load_Write(RUN_TEST_IDLE_STATE,ADDRESS_REGISTER,
            Dest, SELECT_DR_SCAN_STATE);

            // Set the transaction register to memory write,
        // then enter Select DR Scan.
        jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,TRANSACTION_REGISTER,
            MEM_WRITE, SELECT_DR_SCAN_STATE);

        unsigned prev; bool wrote_data = false;
            while (Count > 0) {
        // Now, send four bytes to the port.
        // Optimize memwrite by not rewriting the data if it's the same.
        unsigned towrite = *(ULONG *)Source;
        if (1 || towrite != prev || !wrote_data) {
            jtag->Select_Load_Write(SELECT_DR_SCAN_STATE, DATA_REGISTER,
            prev = towrite, RUN_TEST_IDLE_STATE);
            wrote_data = true;
            }
        else jtag->go_to(RUN_TEST_IDLE_STATE);  // Force transaction.

                if (jtag->until_not_busy("memory_write")!=1) {
            //printf->printf("Memory write at 0x%x for 0x%x failed.\n", Dest,Count);
                    return ERR_BUSY;
            }
        // We are in select_dr_scan_state.

                Count -= 4;
                Source += 4;
        if (addr_increment_needed) {
            Dest += 4;  //auto increments except for first a7.
            jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,
                ADDRESS_REGISTER, Dest, SELECT_DR_SCAN_STATE);
            }
        }
        }
        else {
            // We are trying to write less than 32 bits, so we must read
            // back the current 32 bit value from the remote machine and
            // merge it with the new bits.

            ret_val = MemRead32(Value, Dest);

            for( i = 0 ; i < Count ; i++ )
                Value[i] = *Source++;

            ret_val |= MemWrite32(Value, Dest);
        }

    invalidate_transaction_register();

        return ret_val;
    }


    /*********************************************************
    * Function : MemRead
    *
    * Read block of memory / register space from remote device.
    **********************************************************/
    override int MemRead( char *Dest, ULONG Source, ULONG Count) {

        int ret_val = 1;
        unsigned long i;
        int temp = 0;
        char Value[4] ;

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nMemRead from %08x\n",Source);
        }
        if (!read_ident_reg) read_identity_register();

        if ( Count >= 4 ) {
            // First - Set up JTAG port address to point to memory
            jtag->Select_Load_Write(RUN_TEST_IDLE_STATE,ADDRESS_REGISTER,
                Source, SELECT_DR_SCAN_STATE);

            // Set the transaction register to memory read, then execute.
            jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,TRANSACTION_REGISTER,
                MEM_READ,RUN_TEST_IDLE_STATE);
            if (read_slow) os->sleep(1);

            if (jtag->until_not_busy("memory_read")!=1) {
                //printf->printf("Memory read at 0x%x for 0x%x failed.\n", Source,Count);
                return ERR_BUSY;
            }

            int expected_state = SELECT_DR_SCAN_STATE;
            while( Count > 4 ) {

                // Next, read back four bytes from the port
                jtag->Select_IR_scan(expected_state);
                jtag->load_IR(DATA_REGISTER, SELECT_DR_SCAN_STATE);
                // This reads the four bytes and by virtue of going to
                // run-test-idle executes the read of the next four bytes
                // (the addr reg was incremented after previous read).
                jtag->read_DR(DATA_REGISTER, (ULONG *)Dest, 
                    RUN_TEST_IDLE_STATE);

                if (!use_extended_driver) {
                    if (jtag->until_not_busy("memory_read")!=1)
                        return ERR_BUSY;
                    // until_not_busy leaves us in select_DR.
                    expected_state = SELECT_DR_SCAN_STATE;
                }
                else expected_state = RUN_TEST_IDLE_STATE;

                Count  -= 4 ;
                Dest   += 4 ;
                if (Count && addr_increment_needed) {
                    Source += 4 ;  //automatically incremented except for a7.
                    // Load address and execute the read.
                    jtag->Select_Load_Write(expected_state,ADDRESS_REGISTER,
                        Source, RUN_TEST_IDLE_STATE);
                    expected_state = RUN_TEST_IDLE_STATE;
                    if (read_slow) os->sleep(1);
                }
            }

            // Next, read back last 4 bytes from the port
            jtag->Select_IR_scan(expected_state);
            jtag->load_IR(DATA_REGISTER, SELECT_DR_SCAN_STATE);
            jtag->read_DR(DATA_REGISTER, (ULONG *)Dest, SELECT_DR_SCAN_STATE);
  
            invalidate_transaction_register();
        }
        else {
            ret_val = MemRead32(Value, Source);

            for( i = 0 ; i < Count ; i++ )
                *(Dest++) = Value[i] ;

            Count = 0 ; // Done ...
            invalidate_transaction_register();
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
    override int MemWrite32(char *Source, ULONG Dest) {
        if (dbg_proto) {
            printInFile("\n************PROTO************");
            printInFile("\n write four bytes (%08x) to memory", *Source);
        }

        // First - Set up PC port address to point to memory
    jtag->Select_Load_Write(RUN_TEST_IDLE_STATE,ADDRESS_REGISTER, 
        Dest, SELECT_DR_SCAN_STATE);

        // Now, send four bytes to the port
    jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,DATA_REGISTER, 
        *((ULONG *)Source), SELECT_DR_SCAN_STATE);

        // Set the transaction register to memory write, then execute.
    jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,TRANSACTION_REGISTER, 
        MEM_WRITE, RUN_TEST_IDLE_STATE);

        if (jtag->until_not_busy("memwrite32")!=1) return ERR_BUSY;

        return 1;
    }


    /*********************************************************
    * MemRead32() - Read four bytes from memory
    * return 1 iff succeded.
    **********************************************************/
    override int MemRead32(char *Dest, ULONG Source) {

        if (dbg_proto) {
            printInFile("\n************PROTO************");
            printInFile("\n read four bytes from memory");
        }

        // First - Set up JTAG port address to point to memory
        jtag->Select_Load_Write(RUN_TEST_IDLE_STATE,ADDRESS_REGISTER, 
            Source, SELECT_DR_SCAN_STATE);

        // Set the transaction register to memory read, then execute.
        jtag->Select_Load_Write(SELECT_DR_SCAN_STATE,TRANSACTION_REGISTER, 
            MEM_READ, RUN_TEST_IDLE_STATE);
        if (read_slow) os->sleep(1);

        if (jtag->until_not_busy("memread32")!=1)
            return ERR_BUSY;

        *Dest = read_data_register();
        return 1;
    }


    /*********************************************************
    * ResetBoard()
    *
    * Performs a system reset on the board
    * And Resets the state of the jtag module (see ResetState()).
    **********************************************************/
    override void ResetBoard() {
        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nResetBoard() \n");
        }

        port->outp((unsigned short)(epp_addr + PP_CONTROL_REG_OFFSET), (C_SS1 | C_SS0 | C_CNT)^C_XOR);
        port->outp((unsigned short)(epp_addr + PP_CONTROL_REG_OFFSET), (C_SS0 | C_CNT)^C_XOR);
    os->sleep(200); // TBH 18 JUN 2003  needed by slower simulations
        port->outp((unsigned short)(epp_addr + PP_CONTROL_REG_OFFSET), (C_SS1 | C_SS0 | C_CNT)^C_XOR);

        //reset state machine
        ResetState();
    }


    /*********************************************************
    * Function : Test_Comms_Link()
    *
    * Initialize and test communication links
    * return 1 if ok.
    **********************************************************/
    override int Test_Comms_Link() {

        ULONG test = 0;
        ULONG r0 = 0; // save its value for the comm tests
        int return_val;
    port->helpful_messages();

        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nTest_Comms_Link() \n");
        }

        /* the communcation link test consits just in writing
        and reading back the value of r0 */
        //CJ-- if (gverbose) printf->printf("%s:Testing port 0x%x...",id(),epp_addr);

        /* save the contents of r0 first */
        return_val = Read_ARC_Register(CORE_REGISTER,0, &r0);

        if (return_val!=1)
            return return_val;

        return_val=Write_ARC_Register(CORE_REGISTER,0,0x12345678);

        if (return_val!=1)
            return return_val;

        return_val=Read_ARC_Register(CORE_REGISTER,0, &test);

        if (return_val!=1)
            return return_val;

        /* restore r0 */
        Write_ARC_Register(CORE_REGISTER,0,r0);

        if ( test==0x12345678 ) {
            //CJ--if (gverbose) printf->printf("OK\n");
            return 1;    //success
        }
        else {
            return ERR_COMM;   //failure
        }
    }

    /*********************************************************
    * Function : ResetState()
    *
    * Function to reset the state of the JTAG module.
    * To get the JTAG in a resetted state,
    * Drive TMS high and five clock pulses
    **********************************************************/
    void ResetState() {
        if (dbg_proto) {
            printInFile("\n\n************PROTO************");
            printInFile("\nResetState() \n");
        }
    jtag->discover_JTAG();  // If not already discovered.
    jtag->set_JTAG_to_idle();
    }

    /*********************************************************
    * Function : int LoopTest(void)    
    *
    * Loop back test JTAG port
    **********************************************************/

    #define COMPARE_LIMIT 10
    #define COMPARE_OFFSET 25

    int LoopTest(int quiet)  {
    char *tests[] = { "101100111000111100001111100000111111010101010",
        "101101110111101111101111101111101111010101010" };  
    int failed = 0;
    char result_buffer[64];
    char teststr[64];
    unsigned long status_val;
    int offset;
    char *cp;

    printf->printf("JTAG loopback test.\n");
    if (!quiet) printf->printf("Resetting State.\n");

    ResetState();

    printf->printf("Parallel status register reads:\n");
  
    //put in select_IR_scan_state, 
    // current state should be SELECT_DR_SCAN_STATE from
    //inside until_not_busy()
    jtag->Select_IR_scan(SELECT_DR_SCAN_STATE);
    jtag->load_IR(BYPASS_REGISTER, SELECT_DR_SCAN_STATE);

    for (int j = 0; j < 2; j++) {
        strcpy (teststr, (const char *) tests[j]);
        int len = strlen(teststr);
        int i;
        for (i = 0; i < len; i++) {
        if (teststr[i] == '1')
            jtag->send_JTAG(JTAG_TMS_0 | JTAG_TDI_1,"Unknown");
        else
            jtag->send_JTAG(JTAG_TMS_0 | JTAG_TDI_0,"Shift_DR");
        result_buffer[i] = '0' + jtag->read_TDO();
        // if (!quiet) printf->printf("0x%02x ", pp_status);
        }
        result_buffer[i] = '\0';
        if (!quiet) {
        printf->printf("\n");
        printf->printf("TDI (pp pin#9  [data reg   7]) = %s\n",teststr);
        printf->printf("TDO (pp pin#13 [status reg 4]) = %s\n",result_buffer);
        }
        if (!strcmp(result_buffer,teststr)) {
        if (!quiet) printf->printf("Perfect Match!!!\n");
        }
        else {
        teststr[strlen(teststr)-COMPARE_LIMIT] = '\0';
        if (!(cp = strstr(result_buffer,teststr+COMPARE_OFFSET),cp)) {
            if (!quiet) printf->printf("Failed - no match!!!\n");
            failed = 1;
            }
        else {
            offset=(char *) result_buffer - 
                (char *)(cp + COMPARE_OFFSET);
            offset = offset - COMPARE_OFFSET;
            if (!quiet) 
                printf->printf("Partial match at offset %d \n",offset);
            }
        }
        }

    if (!quiet) {
        printf->printf("Resetting State Prior to checking JTAG Status register\n");
        ResetState();

        jtag->Select_IR_scan(RUN_TEST_IDLE_STATE);
        jtag->load_IR(STATUS_REGISTER, SELECT_DR_SCAN_STATE);

        //read status register
        status_val = 0xffffffff;
        jtag->read_DR(STATUS_REGISTER, &status_val, SELECT_DR_SCAN_STATE);
        printf->printf("Status register value (read failure is 0xffffffff) "
            "0x%08x\n", status_val);

        if ((unsigned char)(status_val & 0x1) ) printf->printf("Stalled On - ");
        if ((unsigned char)(status_val & 0x2) ) printf->printf("Failed On - ");
        if ((unsigned char)(status_val & 0x4) ) printf->printf("Ready On - ");
        if ((unsigned char)(status_val & 0x8) ) printf->printf("PC On");
        printf->printf("\n");

        printf->printf("Finished Loop Test  --- Resetting State\n");
        }  // not quiet then check status register

    ResetState();

    return failed;

    }

    /*********************************************************
    * Function : process_property(const char *property, const char *value)
    *
    * Proccess any JTAG port or protocol level properties.
    **********************************************************/
    override int process_property(const char *property, const char *value) {
    bool value_is_1 = *value == '1';
        /* set up the port driver */
        if (!os->stricmp(property,"port")) {
            if (PORT_SETUP == false) {
                PORT_SETUP = true;
                epp_addr = (UWORD)strtoul(value,0,0);
                // printf->printf("Setup port driver, port address = 0x%x.\n", epp_addr);
                int return_val;

                if ((return_val = port->setup(epp_addr, 4))!=1) {
                    printf->printf("\n can not set up the port driver!\n");
                    return return_val;
            }

                // we reset the jtag port state machine
        // This is wrong.  If the board isn't blasted, this fails.
        // We have to do this later.
        // The api does it when we prepare_for_new_program.
        // Furthermore, other properties that may control the JTAG
        // unit are not yet seen as properties and hence not processed!
                // ResetState();
        }
        }
    else if (!os->stricmp(property,"reset_jtag")) {
        // APIJTAG.dll add property reset_jtag to 
        // jtagproto to allow reset of jtag state only when 
        // -off=download used and user just wants to halt
        // without board reset but JTAG has gone back to scan chain mode.
        // This is from Tim Holt of MW tech support.
        ResetState();
        }
        else if (!os->stricmp(property,"retry_jtag")) {
        // TBH not in 3.8 - added for ac/arcjprobe
        jtag_retry_base_number=abs(atoi(value));
            }
        else if (!os->stricmp(property,"loop_test")) {
        // TBH not 6 Jun 2002
        LoopTest(0);
            }
       else if (!os->stricmp(property,"reset_always")) {
        // TBH 13 Jun 2002 not in 3.8 - added for ac/arcjprobe
        jtag_reset_always = (!value || *value != '0');
            }
        else if (!os->stricmp(property,"dbg_proto")) dbg_proto = value_is_1;
    else if (!os->stricmp(property,"dbg_port")) dbg_port = value_is_1;
    else if (!os->stricmp(property,"optimize_jtag")) 
        optimize_jtag = value_is_1;
    else if (!os->stricmp(property,"new_driver")) {
        // Allow this DLL to shut off use of the new driver's facilities.
        use_extended_driver = value_is_1 &&
            driver_major_version >= 2;
        }
    else if (!os->stricmp(property,"a7_addr_increment")) {
        a7_addr_increment = value_is_1;
        }
    else if (!os->stricmp(property,"read_slow")) {
        // Was used as an experiment.  Not useful.
        read_slow = value_is_1;
        }
    else return jtag->process_property(property,value);
    // Successful above; return 1.
    return 1;
    }

    };

bool Jtag_protocol::PORT_SETUP = false;

Protocol *get_protocol() {
    return new Jtag_protocol();
    }
