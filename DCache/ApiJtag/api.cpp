// FIX_TIM and TBH and ASK_TIM
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
*         Created 2000-2004 and Protected as an Unpublished Work
*                  Under the U.S. Copyright Act of 1976.
*             Copyright 2000-2004 ARC International(UK) Ltd
*                          All Rights Reserved
*
* File name:  api.cpp
*
* Author   :  Stephane Bihan, Tom Pennello, and others
*
* Date     :  28/01/99 and following
*
* Description :
*
* This module is a part of the DLL interfacing the SeeCode debugger
* and the ARCAngel board via the parallel port. This implementation
* contains all the routines called by the debugger.
*
* This forms the higher layer api and works with three other layers, the low level
* api layer(lowio.cpp),the protocol layer (parproto.cpp / jtagproto.cpp) and the 
* port control layer (parport.cpp / jtagport.cpp).  It can also be used in a seperate
* DLL, containing only target independant content, being linked to no other DLL layers.
* (see "Writing a target debug DLL for the Metaware SeeCode Debugger" for more information)
*
  You can refer to the "ARC Programmers Reference Manual" and the "Debug
  Components Ref." documents concerning the register description, the MADI
  block for multiARC system and the actionpoint mechanism.
 
  History:
 
  Version   Date        Author        Description
 
  1.0       28/01/99    S.Bihan       extract from the original dll
  2.0       28/06/99    S.Bihan       add MADI and Actionpoint facilities
            13/09/99    S.Bihan       Instruction Step is used by default
                                      if ARC version is greater or equal to 7
            28/09/99    S.Bihan       Memory write improvment for NT (write blocks of data).
            10/11/99    S.Bihan       Cycle-step the ARC if -on=cycle_step
  2.1       23/03/00    S.Bihan       Uncommented out a part of the step function (was a mistake)
  2.2       03/04/00    S.Bihan       Fix bug when multiarc and ARCAngel1
  2.3          09/00    Tom Pennello  C++ conversion.  
                                      Support for separate low-level 
                                      ARC access DLL. (see "COMMENTS FROM TOM")
 
  2.4          10/00    Tom Pennello  api.cc now no longer depends on Windows
                                      or on any hardware.
 
  2.5       31/10/00    Huw Lewis     (see "HUW's CHANGES")
 
  2.6       22/11/00    Tom Pennello  Fixed bug in set_mem_size().
  2.7       19/12/00    Huw Lewis     Set up "gverbose" property, so dll's stdout
                                      can be disabled (dll can now be made quiet)
 
  2.8       17/01/01    Huw Lewis     Update to step(), where ED (action point enable) 
                                      bit in debug reg was getting cleared, whilst 
                                      writing step bit(s).
                                      ED bit doesn't get cleared now.
 
  2.9       20/02/01    Huw Lewis     Error messages are not disabled regardless of the 
                                      gverbose property.
 
  3.0       25/05/01    Huw Lewis     
  	Implemented a jtag ResetState() method, fixed the parallel port
	ResetState() method (see jtagproto.cpp and parproto.cpp for
	details). In lowio.cpp where prepare_for_new_program() is called
	where a program is not required to be downloaded; the ResetState
	method is now called, to ensure par/jtag module is in the
	correct state.  Jtag was crashing when -off=download and board
	had been freshly re-blasted, this change prevents this.
 
  3.1       06/07/01    Huw Lewis     
   	As a -chipinit file would be called before
	prepare_for_new_program(), the commands in the init file could
	be evaluated on a board in an unstable state.  "prop
	reset_board" can now be optionally used as the first command in
	the file to avoid this. (Addition made to process_properties()).
 
  3.2       11/07/01    Huw Lewis     
   	In fix v3.0, fixed par port reset_state() function, but in rare
	circumstances if the reset_state func fails to clear the busy
	line the func would be left in an infinite loop.  Time out used
	for elegant exit, as well as ResetBoard() called, to force busy
	line to clear. Also ResetState function terminates debugging
	session if error found. No point continuing !

  3.3       27/07/01    Huw Lewis     
  	For JTAG should allow for addressing 32 bit aux regs.  Before
	this fix only 31 bit addressing could be achieved.  This was
	because "int" was used to represent a register number in
	"api.cpp", "lowio.cpp" and "arcint.h".  Now unsigned long is
	used, therefore because unsigned, no need for sign bit for bit
	32, i.e. 32 bits can be used.  Doesn't really matter for
	parrallel port as it only allows 24 bit reg addressing anyway.
 
  3.4       17/08/01    Huw Lewis     
  	This version now supports ARCompact(> arc_version 10), analyses
	the arc version number to see what processor type used.  If
	ARCompact, the DLL behaves differently in that core regs 61..63
	are activated.  Because lowio.cpp needed to know the arc_version
	as well as api.cpp; to activate these regs appropriately,
	low->prepare_for_new_program() now gets the version number.
	low->read_reg() and low->write_reg() can then analyse the
	version number to alter the core_reg range. Also for step(),
	there are some differences, if arc_version >= 10 then cycle
	stepping is simplified where a simple hardware cycle step is
	performed (debugger controls intelligence here, i.e, extra step
	for Limm), see "step()" method.
  
  3.5		07/09/2001	Mark Farr	  Amended run() function for ARCCompact (v10+). Added HALT bit constant for 
 	ARCCompact.
 
  3.6		01/12/2001  Mark Farr     Fixed MADI Jtag bug (MADI registers not set). Fixed multiarc reset bug.
 
*******************************************************************************/
const char *VERSION = "v4.36";

// 4.36: Disable action point logic for A6 and above: causes annoying
// 	 aux reg write error on A7.
// 4.35: Support remote blasting via SCIT (mblast property to blast from memory)
// 4.34: a7_addr_increment defaults to OFF for A7 now that the JTAG 
//	 auto-increments addresses.
// 4.33: Error message changes in jtag: return error to debugger on memory
//       read failure, rather than saying everything's fine!
// 4.32: Don't permanently affect user's input upon big-endian write!
// 4.31: Improve non-existent aux reg handling for A7.
// 4.30: fix crash when using lowdll=.
// 4.29: detect data-invariant big endian and set up byte reversal for it.
// 4.28: dc_flush, reset_delay moved to lowio, slow_clock, sleep(200ms) after
//       SS1 taken low.
// 4.27: check for icache presence in lowio before ivic-ing.
// 4.26: action point range pairing.
// 4.25: Improve jtag behavior upon failure.  For memory read/write, ensure
// 	 we've read the identity register so we know it's an A7.
// 4.24: preserve UB bit in debug register for A7.  Optimized data write
// 	 for jtag (don't reload data reg if value is the same).
// 4.23: adjust for a7 not incrementing the address on block mem I/O.
// 4.22: reset the ARC upon first download, even if not cpunum==1.
//       This happened with CMPD where [1]=ISS and [2]=arc.
// 4.21: support for Linux GPIO driver.  No changes to windows version.
// 4.20: allow more messages to go to GUI console.
// 4.19: Respect target vs host endian for data reads/writes.
//       Conditioned on a toggle due to older, not-quite-correct big-endian 
//	 ARCs.  Toggle to be on by default eventually.
// 4.18: First successful linux hardware download to an AA3.
// 4.17: changes in preparation for Linux.
// 4.16: add invalidate_icache toggle to lowio.cpp.
// 4.15: chained jtag actually works for multiple ARCs!
// 4.14: chained jtag actually works!
// 4.13: more jtag overhaul and chained jtag detect.
// 4.12: optimize jtag comms.
// 4.11: fixed problem where ResetBoard was being called when port
// was specified, even before a blast had occurred!
// 4.1: added Justin Wilde's 4.0 changes.
// Version 4.0: Justin Wilde, Added PLL and clock routing options
// 3.83: add Tim's changes for jtag stability.
// 3.81: added reset_jtag property.
// Version 3.8: radical changes in interface to board, allowing use
// of advanced || port driver with lots of ARC stuff built-in to the
// driver for increased speed.
// Version 3.7: made use of new ARC interface to provide actionpoint
// accuracy.  3.71: removed at_breakpoint_cookie; could return cookie
// from at_breakpoint.

// COMMENTS FROM TOM.
// Advantages of C++:
// 1. You can put variables close to their use, rather than be forced
//    to have them at the top.
// 2. static const int variables are back-substituted as constants.
//    This removes #ifs, which are clutter and problematic.
// 3. No more p-> necessary.
// 4. You can take advantage of default function bodies in new versions
//    of arcint.h.
// 5. It'll be easier to move to a layered DLL approach where 
//    one can use yet another DLL  to do the raw IO.  
//    This allows a customer to write a dumb DLL to read/write reg/mem,
//    and for the code in this file to handle the complicated parts 
//    (stepping, etc).
// 6. No forward references needed for functions.

// Other changes:
// 1. Some paragraphing was cleaned up.
// 2. }s were indented one level.  This conforms to MetaWare's style,
//    but I realize others may dislike it, so I'll change back if
//    you wish.
// 3. Removed extraneous "typedef" keyword.
// 4. Fixed bug in remove_reg_watchpoint and remove_mem_watchpoint
//    where this line appeared:
//     for (int i = 0; i < i< number_of_AP; i++){
//    It was corrected to:
//     for (int i = 0;     i< number_of_AP; i++){
// 5. Fixed inconsistent declarations of port setup/shutdown functions.
// 6. Added log_stdout property so I could send log to stdout.
// 7. Added dbg_proto, dbg_port, and dbg_int properties 
//    so I could turn off and on debugging dynamically.
// 8. Got rid of Sleep(0) which was causing poor performance.
//    This is an old bug that has crept back in again.
// 9. Made the jtag paragraphing styles unified relative to the win
//    paragrahing styles.
// END

//*********************************
//  HUW's CHANGES (31/10/00)(v2.5)
//*********************************
// 1. Changed code layout from Metaware style to (in my opinion) a more readable style, ARC's Style
// 2. Additions to comments   
// 3. Fixed bug in step(), where stepping a LIMM instruction in a loop was causing problems.(see step())
// 4. Added debug info to parport.cpp
// 5. Fixed bug where dbg_int could only be selected in api.cpp (not in lowio.cpp)
// 6. Changed dynamic debug options so when dbg_int is selected, dbg_proto is not chosen as well
//    This could lead to confusion, when using options as to what is selected using dbg_int.
//    I'll change it back, if anyone wants me to. 
// 7. Changed DLL names from JPAR -> JTAG, since JPAR is a bit too similar to PAR.
// 8. For parallel port, bug fixed in ResetBoard() method (see parproto.cpp)
// 9. Improved jtag comments/ improved neatness of code
//10. Removed parproto.h from source, since referenced in port.cpp (a target independant source file)
//*********************************

/*********************************************************
* Include Files
**********************************************************/
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

#define OEM_USE_OF_DEBUGGER_HEADER_FILES  1
#include "arcint.h"
#include "globals.h"
#include "low.h"
#include "dll.h"

/*********************************************************
* Defines
**********************************************************/
#define UNINITIALISED -1
#define SUCCESS 1
#define FAIL 0
#define MEM_SIZE 512*1024                   // 512K by default (with SRAM)
                                            // 16M with DRAM
#define MAX_AP 8                            // max number of actionpoints

//#define DEBUG_INTERFACE 1
static const UBYTE   LIMM = 0x3e;                  // Operand value for Long immediate data
static const UBYTE   LIMM_MASK = 0x3f;             // Operand mask for Long immediate data
static const ULONG   OPCODE = 0x0000001f;          // Mask for accessing the opcode
static const ULONG   PC_ADDRESS_MASK = 0x00ffffff; // Mask for getting the address
                                            // from the STATUS register

/* register definitions. Below is the convention adopted
  XX_M is for Mask
  XX_V is for bit position Value */

/* Status Register */

static const ULONG   HALT_BIT_V = 25;
static const ULONG   ZERO_HALT_BIT = 0xfdffffff;   // Preserve all the bits except the Halt bit

//ARC Compact 
static const ULONG   AC_ZERO_HALT_BIT = 0xfffffffe;// Preserve all the bits except the Halt bit


/* Debug register */
static const ULONG   ASR_V = 3;                    // Actionpoint Status Register
static const ULONG   ASR_M = 0x000007f8;
static const ULONG   IS_V = 11;                    // Instruction Step bit
static const ULONG   IS_M = 0x00000800;
static const ULONG   SS_V = 0;                     // single step bit
static const ULONG   SS_M = 0x00000001;
static const ULONG   AH_V = 2;                     // Actionpoint Halt bit
static const ULONG   AH_M = 0x00000004;
static const ULONG   FH_V = 0x00000001;            // Force Halt bit
static const ULONG   FH_M = 0x00000002;

static const ULONG   ED_M = (1<<24);            //Action Point enable bit
static const ULONG   UB_M = (1<<28);            //brk in user mode enable bit

/* Actionpoint Build Configuration Register field definition */
static const ULONG   NAP_V = 8;                    // Number of actionpoints
static const ULONG   NAP_M = 0x00000f00;


/* MADI Build Configuration Register definition */
static const ULONG   MAN_V = 8;                    //number of ARCs
static const ULONG   MAN_M = 0x0000ff00;


/* ACR (Actionpoint Comparison Register) */
static const ULONG   AN_V  = 12;                   // actionpoint number to program
// static const ULONG   AN_M  = 0x00007000;	// Unused.
      /* ap comparison mode value */
static const ULONG   AC_EQ = 0x00000000;           // equal to ap data value
static const ULONG   AC_GT = 0x00000040;           // greater than ap data value
static const ULONG   AC_LT = 0x00000080;           // less than ap data value
static const ULONG   AC_NE = 0x000000c0;           // not equal toap data value
      /* type of data comparison */
static const ULONG   AT_MWA = 0x00000004;          // break on write to specific mem addr (from core only)
      /* mode */
static const ULONG   AP_ENA = 0x00000001;          // enable ap
static const ULONG   AP_DIS = 0x00000000;          // disable ap

      /* pre-programmed actionpoint modes (ACR register)
                                              PA  AC  AT    AM */
static const ULONG   BKP_MODE = 0x001;             // 0  00  0000  01
static const ULONG   CORE_REG_MODE = 0x00d;        // 0  00  0011  01
static const ULONG   AUX_REG_MODE = 0x015;         // 0  00  0101  01
static const ULONG   MEM_MODE = 0x005;             // 0  00  0001  01


/* MADI register */
static const ULONG   SINGLE_CAST_MODE = 0x000;
static const ULONG   MULTI_CAST_MODE =  0x100;

#if _MSC_VER
extern "C" void __stdcall Sleep(long);
#endif

/*********************************************************
* Global Type Definitions
**********************************************************/
/* used to store bkp information in the cookie provided by the debugger */
struct bkpinfo_struct 
{
    int actionpoint_number;
    ULONG address;
};


/* used to store information on programmed actionpoints */
/* actiopoint type */
typedef enum aptype 
{
    UNUSED= 0, WATCH_MEM, WATCH_REG, BKP
} APTYPE;


/* general actionpoint information strucure definition */
struct APinfo {
    APTYPE type;      	// actionpoint type
    union {
	ULONG address; 	// programmed address for mem watchpoint
	ULONG reg;   	// reg number for reg watchpoint
	} data;      
    ULONG interface_address;	// given to us by interface
    int paired_ap;    // associe another ap in case of 8 bytes memory watchpoint
    bkpinfo_struct bkp_info;
    void *cookie;	// cookie shared with debugger.
    };

/*********************************************************
* Global Variables
**********************************************************/
static bool RESET = FALSE;   
	// Reset the board only once.
	// bug fixed: in multiarc if detach and load a new program in the
	// first ARC, the board (thus the 2nd ARC) was reset as well.

struct API_ARC: ARC {
    API_ARC(ARC *low) {
        callback = 0;
        memsize = MEM_SIZE;
        number_of_AP = UNINITIALISED;
        arc_version = UNINITIALISED;
        arcnum = 0;
	arcnum_needs_checking = FALSE;
        CYCLE_STEP = FALSE;
        num_ARC_connected++;
        this->low = low;
        low_dll_loader = 0;

        #if DEBUG_INTERFACE
            dbg = 1;
        #else
            dbg = 0;
        #endif
	// The new_bp_cookie was added in when ARC_FEATURE_at_breakpoint_cookie
	// was implemented.  Later we figured out that new_bp_bookie isn't
	// necessary.  However, as new_bp_cookie makes such things as
	// RADAR much easier to deal with, we leave in the implementation,
	// and someday may turn it on by default when RADAR is revised.
	new_bp_cookie = FALSE;
	port_setup = FALSE;
	host_and_target_endian_are_the_same = TRUE;
	respect_endian = FALSE;	// Set TRUE later on.
	endian_checked = FALSE;
	action_point_ranges = TRUE;
	dc_flush = TRUE;
	slow_halt = 0;
	}
    bool dc_flush;
    bool endian_checked;
    int port_setup;
    static unsigned get_integer(const char *value) {
        return strtoul(value,0,0);
	}
    NOINLINE int force_reset() {
        if (low == 0) return 0;	// Can't.
	print_startup_message();
	RESET = FALSE; // Force reset 
	// lowio.cpp is forced to reset board
	int ret_val = low->prepare_for_new_program(TRUE);  
	if (!ret_val) return ret_val;
	RESET = TRUE;
	return 1;
	}

    bool host_and_target_endian_are_the_same;
    bool respect_endian;
    bool action_point_ranges;
    bool host_is_little() {
	// Return TRUE if we are on little-endian machine.
	// We know this for some compile-hosts:
	#if _I386 || _MSC_VER
	if (1) return TRUE;
	else  
	#endif
	{
	union { char c; short i; } u;
	u.i = 0;
	u.c = 1;
	return u.i == 1;
	}
	}

    static ARC *low;                  // contains the low level dll, if there is one
    DLL_load *low_dll_loader;         // used to hold a dll object 
    static ARC *low_dll_arc;          // low level dll is loaded into this variable (later transferred to "low")
    int dbg;                          // bool value which states 1->debug interface on, 0-> off
    static int number_of_arcs;        // number of ARCs in a multiARC system
    static int current_arc;           // indicates which ARC points the multiplexor
                                      // in a multiARC system.
    bool CYCLE_STEP;                  // flag to use the cycle-step mechanism insted of
                                      // instruction-step mechanism.
                                      // Specified to the debugger command line with -on=cycle-step
    static bool IS_CMPD;              // flag indicating if the debugger is the CMPD
    static int num_ARC_connected;     // number of ARCs currently connected
    static bool printed_start_message;// This bool value used to check if 'intro printout' has 
                                      // occured yet, so not printed out twice with multiarc.
                                      // false = not printed yet, true = don't print it again
    int memsize;                      // Size in bytes of memory on card, unused in this implementation
    char *mem;                        // Unused in this implementation
    int arcnum;                       // arc number in a multiARC system (only used in a multiARC system
    bool arcnum_needs_checking;
    int arc_version;                  // ARC version

    /* the AP numbering starts at 1 till 8 */
    UBYTE AP_used;                // 8 bits, represent a vector of used AP
    int number_of_AP;             // number of actionpoints built in an ARC

    APinfo ap_table[MAX_AP]; // records information on the actionpoints in use
    bool new_bp_cookie;

    enum {NO_FREE_AP=-1};
    int get_free_actionpoint();
    void get_free_actionpoint_pair(int &ap1, int  &ap2);
    void remove_actionpoint(int ap_number);
    int switch_madi();
    unsigned long identity_register;
    int get_arc_version_number();
    void print_startup_message();

    
    /*********************************************************
    * int version()
    *
    * Interface version:
    * The version starts out at 1. Later versions will append functions
    * to the end of this specification, but not delete them. Two enumerated
    * values defined in arcint.h which can be used to specify the version
    * number are as follows
    * ARCINT_BASE_VERSION = 1, ARCINT_BREAK_WATCHPOINT_VERSION = 2
    **********************************************************/
    override int MS_CDECL version() {
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] version(): %d", arcnum, ARCINT_TRACE_VERSION);
	    }
    
        return ARCINT_TRACE_VERSION;     
	}


    /*********************************************************
    * char  *id()
    *
    * Returns board ID: implementor, manufacturer, whatever, etc. Used
    * in printing messages:
    **********************************************************/
    override const char *MS_CDECL id() {
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] id()",arcnum);
	    }
    
        return "ARC API";
	}
    
    
    /*********************************************************
    * void destroy()
    *
    * Destroys the object. Equivalent to a C++ destructor. After calling
    * this, the client will never use the object again:
    **********************************************************/
    override void MS_CDECL destroy() { delete this; }

    ~API_ARC() {
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] destroy()", arcnum);
	    }

        /* shutdown the port only if it is the last destroy call */
        if (num_ARC_connected == 1) {
            /* unitialise all the global variable in case the dll has not
            been closed and the debugger invoked again */
            number_of_arcs = UNINITIALISED;
            
            // LOG_CREATED = FALSE;
            // Destroy the interface before you destroy the DLL loader.
            // When you destroy the loader you remove the code from memory!
            
            if (low) low->destroy();
            
            if (low_dll_loader) delete low_dll_loader;
	    }

        num_ARC_connected --;
	}

    
    /*********************************************************
    * int is_simulator()
    *
    * Returns 1 if the implementation is a simulator. If it is, a
    * debugger may choose to repeatedly step rather than run, so that it is
    * possible to halt the execution of a program by tapping the keyboard.
    * The debugger would check for the keyboard tap every so often during
    * instruction stepping.
    **********************************************************/
    override int MS_CDECL is_simulator() {
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] is_simulator()",arcnum);
	    }
    
        return 0; // The DLL is to access hardware not a simulator.
	}


    /*********************************************************
    * int step()
    *
    * Effects a single-step of the machine; for the hardware, initiates
    * the single step; for the simulator, returns when the single-step
    * is completed. Returns 1 if succeeded, 0 if failed.
    * The implementation of this function is quite complex due to the fact
    * that not all instruction can complete in one clock cycle.
    * Instructions with long immediate data must wait for that data to be
    * fetched which takes longer than the single cycle that setting the ss
    * (single step) bit in the debug register affords.
    * To get round this the step function must first decide whether an
    * instruction has Limm (long immediate) data or not. Each type of
    * instruction code associated with it (see page 6.1 of Arc PRM for
    * details). This code is used to determine whether or not the
    * instruction can have Limm data, the number of operands it has, and
    * which operands can indicate Limm data following. If the instruction
    * can support Limm data then a check is performed to see if any
    * follows. If it does then the appropriate number of single cycle steps
    * are performed until then data is fetched and the instruction
    * executed.
    *
    * Use the Instruction Step mechanism if the ARC version is 7
    * or above unless the user desires to use the previous method
    **********************************************************/
    override int MS_CDECL step() {
        ULONG   lpCount;  // Value of Loop count register when function is called
        ULONG   lpC;      // Current value of Loop count register during single
                          // stepping of the arc. Used to determine when an
                          // in instruction has completed */
        ULONG   startPC;  // Value of Program counter when function is called */
        ULONG   cPC;      // Current value of Program counter during single stepping
                          //  of the arc. Also used to determine when an instruction
                          //  has completed */
        ULONG   lpStart;
        ULONG   lpEnd;
        ULONG   instruction;     // The current instruction */
        UBYTE   fieldB=0;        // The B and C operand fields found in certain */
        UBYTE   fieldC=0;        // instructions */
        bool    limmInstr=FALSE; // Indicates whether the instruction contains Limm data */
        ULONG   SingleStep;      // Value written to the debug register when single stepping
        ULONG   debugReg;        // Used to store the value of the debug register
        if (dbg) 
        {
            printInFile("\n-------------");
            printInFile("\n[%d] step()",arcnum);
        }

        //get current debug register value
        read_reg(reg_DEBUG, &debugReg);

        //get current value of ED(action point enable) bit and UB
	//UB (brk allowed in user mode (A7)) bits.
        unsigned keep_bits = debugReg & (ED_M|UB_M);
        
        //if arc verison is 10 or above and cycle stepping is needed, then simply cycle step the arc
        //NOTE: Debugger will deal with the extra intelligence, i.e extra step if Limm
        if (arc_version >= 10 && CYCLE_STEP) {
            //Single step the arc
            SingleStep = SS_M | keep_bits;
            write_reg(reg_DEBUG, SingleStep);
            
            return SUCCESS;
	    }
        
        //if ARC version greater or equal than 7, use the instruction step mechanism
        //unless the user wants to use the old mechanism.
        //NOTE: At this point, if user has requested to use the old mechanism
        //then arc version will be below 10 !
        if ((arc_version >= 7) && !(CYCLE_STEP)) {
            //Instruction step the arc
            SingleStep = (SS_M | IS_M | keep_bits);
            write_reg(reg_DEBUG, SingleStep);

            return SUCCESS;
	    }
        

        /* Cycle step mechanism for ARC version less than 7 */
        read_reg(reg_STATUS,&startPC);  // Likewise for Status register

        startPC = startPC & PC_ADDRESS_MASK; // Only interested in address
        read_memory(startPC*4, (char *)&instruction, 4); // Get a copy of the current instruction

        /* Switch on opcode */
        switch( (instruction >> 27) & OPCODE ) {

            /* Can't have LIMM data on these instructions */
            case 4:   // Bcc instructions
            case 5:   // BLcc instructions
            case 6:   // LPcc instructions
            break;

            /* One operand only. Operand always in b */
            case 0:
            case 3:   // Single operand intructions (Type 3)
            //***************************************************************
            //    insert special case for operand 2 in here
            //***************************************************************
            case 7:   // Jcc instructions

            // Get the individual operand
            fieldB = (char)( instruction >> 15) & LIMM_MASK;
        
            if ( fieldB == LIMM )
               limmInstr = TRUE;
           
            break;

            default:
            // case 1:   //LD (type 1 instruction) and LR instructions
            // case 2:   //ST/SR instructions
            // case 8:

            /* Get the individual operands */
            fieldB = (char)( instruction >> 15) & LIMM_MASK;
            fieldC = (char)( instruction >> 9) & LIMM_MASK;

            /* Check for Limm data */
            if ( (fieldB == LIMM) || (fieldC == LIMM))
                limmInstr = TRUE;
        }

        // get address of start of loop (value won't be used if not inside a loop)
        read_reg(reg_LP_START, &lpStart);

        // get address of end of loop (value won't be used if not inside a loop)
        read_reg(reg_LP_END, &lpEnd);

        // Get start value of Loop count register
        read_reg(reg_LP_COUNT,&lpCount);

        while (TRUE) {
            /* Single step the arc */
            SingleStep = SS_M | keep_bits;
            write_reg(reg_DEBUG, SingleStep);

            /* Get current value of loop counter */
            read_reg(reg_LP_COUNT, &lpC );

            /* Get current pc value */
            read_reg(reg_STATUS, &cPC );

            cPC = cPC & PC_ADDRESS_MASK; // Only interested in address
        
            if (limmInstr) {
                if ( (cPC != startPC) && (cPC != startPC + 1) )
                    break;
		}
            else {
                // has simm instruction completed (1 cycle) ?
                if (cPC != startPC)
                    break;
		}
        
            if (cPC >= lpStart && cPC <= lpEnd) {   // if currently in a loop
                // If a previous instruction has loaded a different
                // value into the lp_count register, there is a chance
                // that if this instruction is still in the pipeline
                // then it will update the register during the stepping
                // of the current instruction.  In this case the
                // instruction will be assumed to have completed, and
                // the instruction step will fail.  Need to check if
                // inside a loop, if so, condition below can be checked.
                // No problem using shimm or limm data.
                
                if (lpCount != lpC)  // if loop counter changes
                    break;  // shimm or limm instruction must have finished !
        
                /* Here is the test case illustrating the above problem:
                From Mark_Farr/ARC@arccores.com Mon Jul 31 09:38:56 2000
                When using the standard DLL you distribute I get
                funny behaviour on the 'loop_here' statement below.
                Register 25 gets incremented by 2 per isi, as does
                the LPC register, when they should only be
                incremented by 1.

                _start::
                     mov %r25, 0
                     mov %LP_COUNT, 10
                     mov %r0, loop_here>>2
                     add %r1, %r0, 1
                     sr  %r0, [%LP_START]
                     sr  %r1, [%LP_END]
                     nop
                     nop
                loop_here:     add  %r25, %r25, 1
                     nop
                     nop
                     nop
                */
		}
	    }
    
        return SUCCESS;
	}


    /*********************************************************
    * int run()
    *
    * Starts the CPU running.  Returns 1 if succeeded, 0 if failed.
    * If this is a simulator, the debugger will not use the run() method,
    * but instead repeatedly step, so that the execution can be interrupted
    * by, say, pressing a key.
    * The debugger polls the status register waiting for the halt bit to be
    * set in the status register indicating execution has stopped. There is
    * no need to wait for it to halt
    **********************************************************/
    override int MS_CDECL run() {
        ULONG run;
        
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] run()",arcnum);
	    }
    
	if (arc_version >= 10){ //Its ARCCompact			
	    /* Just set the arc running and return */
	    read_reg(reg_AC_STATUS32, &run);  // Read ARC Compact status register
	    run = run & AC_ZERO_HALT_BIT;     // Preserve all bits 
	    				      // except ARC Compact halt bit
	    write_reg(reg_AC_STATUS32, run);  // Set arc going and return
					      // control to the debugger
	    }
	else {			
	    /* Just set the arc running and return */
	    read_reg(reg_STATUS, &run);  // Read status register
	    run = run & ZERO_HALT_BIT;   // Preserve all bits except halt bit
	    write_reg(reg_STATUS, run);  // Set arc going and return
					 // control to the debugger
	    }
        return SUCCESS;
	}


    unsigned swap_long(unsigned x)  {
	return ((x & 0xff) << 24) |
	       ((x & 0xff00) << 8) |
	       ((x & 0xff0000) >> 8) |
	       ((x & 0xff000000) >> 24);
	}
    void reverse_words(unsigned adr, void *buf, unsigned amount) {
        unsigned *ibuf = (unsigned*)buf;
	for (unsigned i = 0; i < amount/4; i++) ibuf[i] = swap_long(ibuf[i]);
        }
    // We assume that low->write and low->read memory will package
    // up the memory in 4-byte chunks, and that each 4-byte chunk must
    // be a value consistent with the way the ARC target would load
    // that value from ARC memory.  Thus, for example, if what is to
    // be written in memory is the byte sequence
    //		aa bb cc dd 01 02 03 04
    // and the ARC target is little endian, we presume that the 
    // registers in a little-endian ARC target that controls writing 
    // into memory should expect the two 32-bit values
    //		ddccbbaa    04030201
    // because when those two values are stored from a LE ARC's register,
    // memory is correct.
    // We thus make the assumption that low->read and low->write
    // (a) traffic in 4-byte chunks to the ARC
    // (b) assume that someone else has done the work for them so
    //     that they can just load the values on the host machine and
    //     know that the values are correct for the ARC target.
    // So, we swap the integers in memory here before we call low->.
    // Note that if you implement a low-> that does byte-stream access
    // to the ARC memory, our logic here is wrong.

    /*********************************************************
    * int read_memory(unsigned long adr, void *buf, unsigned long amount)
    *
    * For the memory requests, you may assume that requests come in
    * multiples of 4 bytes and aligned to 0 mod 4 boundaries.  Failure is
    * assumed if the return result does not equal amount.
    * Reads memory and returns # bytes read:
    **********************************************************/
    override int MS_CDECL read_memory(unsigned long adr, void *buf,
                unsigned long amount, int context = 0) {	

        if ((adr & 3) | (amount & 3)) {
            // This function fails unless the amounts are appropriately
            // aligned.
            return 0;
	    }   
    
        check_endian();
        memset(buf,0,amount);
        switch_madi();
			    
        if (dbg) {
            printInFile("\n[%d] read_memory() Start address = 0x%+08x, "
            "amount = %d bytes \n",
            arcnum, adr,amount);
	    }

        int ret = low->read_memory(adr,buf,amount,context);
    	if (!host_and_target_endian_are_the_same)
            reverse_words(adr,buf,amount);
        return ret;
	}


    /*********************************************************
    * int write_memory(unsigned long adr, void *buf,
    *                       unsigned long amount)
    *
    * Writes memory and returns # bytes written:
    **********************************************************/
    override int MS_CDECL write_memory(unsigned long adr, void *buf,
                unsigned long amount, int context = 0) {

        if ((adr & 3) | (amount & 3)) {
            // This function fails unless the amounts are appropriately
            // aligned.
            return 0;
	    }   
	check_endian();
        switch_madi();
		
        if (dbg) {
            printInFile("\n[%d] write_memory() Start address = 0x%+08x, amount = %d bytes ",arcnum, adr,amount);
	    }

    	if (!host_and_target_endian_are_the_same)
	    reverse_words(adr,buf,amount);
        int ret = low->write_memory(adr,buf,amount,context);
	// Restore the input!  Don't leave it in reversed state!
    	if (!host_and_target_endian_are_the_same)
	    reverse_words(adr,buf,amount);
	return ret;
	}

    
    /*********************************************************
    * int read_reg(ARC_REG_TYPE r, unsigned long *value)
    *
    * Reads a register and returns 1 if succeeded, 0 otherwise
    * This function is used for reading both core and auxilliary
    * registers.
    * Registers 0 to 63 are core registers. Registers 64 and above
    * map to auxilliary registers from 0 upwards.
    * To get the correct register number for an aux register
    * 64 must be subtracted from the parameter r.
    **********************************************************/
    override int MS_CDECL read_reg(ARC_REG_TYPE r, unsigned long *value) {    
	switch_madi();

        if (dbg) {
            printInFile("\n[%d] read_reg %d:",arcnum,r);
	    }

        return low->read_reg(r,value);
	}

    enum { 
	DCACHE_FLUSH_REG = 0x4B,
	MEMSUBSYS_REG = 0x67 // memsubsys tells endian of target.
	};
    NOINLINE void check_endian() {
	if (endian_checked) return;
        unsigned long membcr;
	read_reg(MEMSUBSYS_REG + AUX_BASE,&membcr);
	if (membcr & 4) {
	    xprintf("Data-invariant big-endian target detected.\n");
	    // We know the host is X86.
	    host_and_target_endian_are_the_same = FALSE;
	    }
	endian_checked = TRUE;
        }

    unsigned slow_halt;
    /*********************************************************
    * int write_reg(ARC_REG_TYPE r, unsigned long value)
    *
    * Writes a register and returns 1 if succeeded, 0 otherwise:
    **********************************************************/
    override int MS_CDECL write_reg(ARC_REG_TYPE r, unsigned long value) {    
	switch_madi();    
	if (dc_flush == FALSE && r == AUX_BASE + DCACHE_FLUSH_REG) {
	    if (dbg) 
		printInFile("\nSkip writing to data cache flush register.");
	    return SUCCESS;
	    }

        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] write_reg: register r%d <- 0x%+08x",arcnum, r,value);
	    }        

	#if _MSC_VER	// No sleep on linux.
        if (slow_halt && r == reg_DEBUG && (value & FH_M) == FH_M) {
	    // This was solely to test a flaw in SeeCode where stop
	    // could be called nestedly (in a different thread) when stop
	    // takes too long.  Such cannot be allowed.
	    printf("Sleep in stop...\n");
	    Sleep(slow_halt);
	    }
	#endif
        return low->write_reg(r,value);
	}
    
    /***********************************************************
    * bool supports_banks() 
    * 
    * a check function that returns a value if r/w banked regs are 
    * supported(true) or not (false)
    ************************************************************/
    bool supports_banks() {
        return low && low->version() >= ARCINT_TRACE_VERSION &&
               (low->supports_feature() & ARC_FEATURE_banked_reg);
	}


    /*********************************************************
    * int read_banked_reg(int bank, ARC_REG_TYPE r, unsigned long *value) 
    *
    * Reads to a register in a certain register bank, for example,
    * to read from MADI cntr reg, specify "bank = 2" and "r = 0".
    * Other current banks include "CORE BANK = 0", "AUX BANK = 1"
    * All registers begin at 0 for each bank, See arcint.h for
    * more details !!!
    **********************************************************/ 
    override int MS_CDECL read_banked_reg(
	    int bank, ARC_REG_TYPE r, unsigned long *value) {
        //Don't switch_madi if bank is MADI, madi registers 
	// should be shared between ARCs.
        if (bank != reg_bank_madi) {
            switch_madi();
	    }
		
	if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] read banked reg: bank %d register r%d <- 0x%+08x",arcnum, bank, r,value);
	    }
        
        //can't access function unless, supports_banks expicity states it is supported 
        if (supports_banks()) {
            return low->read_banked_reg(bank,r,value);
	    }
        else {
            switch(bank) {
                case reg_bank_core: 
                    return read_reg(r,value);
                case reg_bank_aux: 
                    return read_reg(AUX_BASE+r,value);
                default: 
                    return 0;   // MADI or otherwise: fail.
		}
	    }
	}
    
    
    /*********************************************************
    * int write_banked_reg(int bank, ARC_REG_TYPE r, unsigned long *value) 
    *
    * Writes to a register in a certain register bank, for example,
    * to write to MADI cntr reg, specify "bank = 2" and "r = 0".
    * Other current banks include "CORE BANK = 0", "AUX BANK = 1"
    * All registers begin at 0 for each bank, See arcint.h for
    * more details !!!
    **********************************************************/ 
    override int MS_CDECL write_banked_reg(
    	int bank, ARC_REG_TYPE r, unsigned long *value) {
        //Don't switch_madi if bank is MADI, madi registers 
	// should be shared between ARCs
        if (bank != reg_bank_madi) {
            switch_madi();
	    }

	if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] write banked reg: bank %d register r%d <- 0x%+08x",arcnum, bank, r,value);
	    }
        
        //can't access function unless, supports_banks expicity states it is supported 
        if (supports_banks()) {
            return low->write_banked_reg(bank,r,value);
	    }
        else {
            switch(bank) {
                case reg_bank_core: 
                    return write_reg(r,*value);
                case reg_bank_aux: 
                    return write_reg(AUX_BASE+r,*value);
                default: 
                    return 0;   // MADI or otherwise: fail.
		}
	    }
	}


    /*********************************************************
    * unsigned memory_size()
    *
    * Returns the memory size of the implementation.
    *      unsigned memory_size()
    **********************************************************/
    override unsigned MS_CDECL memory_size() {
	if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] memory_size()",arcnum);
	    }

        return memsize;
	}


    /*********************************************************
    * int set_memory_size(unsigned S)
    *
    * Sets the memory size of the implementation.
    * Used for simulator implementation only
    **********************************************************/
    override int MS_CDECL set_memory_size(unsigned S) {
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] set_memory_size() 0x%x",arcnum, S);
	    }
          
        memsize = S;
    
        return SUCCESS;
	}


    //----------------------------REMOVED-----------------------------------
    #if 0//  No need to implement these.  Default bodies given in new arcint.h
    
    /*********************************************************
    * void *additional_information()
    *
    * Returns something that the client can use to communicate with
    * the server, as indicated by additional_possibilities() return value.
    * Typically, in C++, this will be object calls which are made
    * to do the communication.
    **********************************************************/
    override void *MS_CDECL additional_information() {
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] additional_information()", arcnum);
	    }
    
        return FAIL; // Not concerned with clients and servers
	}


    /*********************************************************
    * const char * additional_possibilities()
    *
    * A string that might identify data that could be passed in to the
    * additional_information function. This is a way for client and server
    * to share information without tying it down in an interface.
    * Currently MetaWare uses it to detect that the debugger is talking
    * to MetaWare's simulator or ARC hardware interface, and does something
    * special for those two interfaces.
    * Do not depend on either of these functions being called by the client
    **********************************************************/
    override const char * MS_CDECL additional_possibilities() {
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] additional_possibilities()", arcnum);
	    }
    
        return FAIL;
	}
    
    #endif
    //----------------------------END OF REMOVE SECTION-------------------

    

    NOINLINE void xprintf(const char *format, ...) {
	va_list ap;  va_start(ap,format);
	if (callback) callback->vprintf(format,ap);
	else vprintf(format,ap);
	}
    /**********************************************************
    * void MS_CDECL receive_callback(ARC_callback*cb) 
    *
    * By using the ARC_callback object, allows various facilities
    * to be used from debugger.  For example, one feature is 
    * printf (use with: callback->printf(...)).  This printf
    * allows the trace output to be printed using the debugger's
    * printf() method, meaning output will be interweaved with 
    * debuggers output, if using a GUI, displayed in output window.
    * See arcint.h for other usage details !!!
    ************************************************************/
    ARC_callback *callback;
    void MS_CDECL receive_callback(ARC_callback*cb) {
	if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] receive_callback()", arcnum);
	    }
        callback = cb;
	check_low_callback();
	}
    NOINLINE void check_low_callback() {
        if (callback && low && low->version() >= ARCINT_TRACE_VERSION)
	    low->receive_callback(callback);
        }


    /*********************************************************
    * int prepare_for_new_program(int willdownload)
    *
    * Prepare for a new program to be downloaded. This has the
    * responsibility of testing the communcations link. But it doesn't
    * flush the pipe or execute anything ambitious on the processor.
    * However, *do* remove any hardware watchpoint or breakpoint settings.
    *
    **********************************************************/
    override int MS_CDECL prepare_for_new_program(int willdownload) {
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] prepare_for_new_program()", arcnum);
	    }
        //Print startup message once (function handles multiple calls) 
        print_startup_message();        

	#if ASK_TIM
        // 23 Jul 2002 added for case where port is not passed at right time
	if (low && !port_setup)  {
	    int ret_val = low->process_property("port","0x378");
	    if (!ret_val) {
		xprintf("Fail to setup port; returning.\n");
		return ret_val;
		}
	    port_setup= TRUE;
	    }
	#endif

        // Initialise corresponding variables

	if (arcnum == 0) {
	    // No CMPD; we weren't told which ARC we were via cpunum.
	    // So we must be a single ARC.
	    arcnum = 1;
	    low && low->process_property("cpunum","1");
	    }

	if (low) {
	    // Reset the board and test the comm link if the 
	    // debugger is not grabbing a running process.
	    // This is done when accessing the first ARC 
	    // to avoid repetition on multiARC system.
	    // We can't depend on arcnum == 1.  arcnum might be 2
	    // in a CMPD environment where the first ARC is a simulator
	    // and the second arc is hardware.
	    // Must reset whenever we talk to the board, even if this
	    // isn't the first ARC!
	    if (/*arcnum==1 &&*/ !RESET) {
		// Don't prepare for non-#1 ARC.
		// For the low-level, this can cause IO initialization
		// of the comm protocols, which is not what we want.
		// We acknowledge that we can't attach to
		// a running multi-ARC.  The reset will scramble it.
		int ret_val = low->prepare_for_new_program(willdownload);
		if (!ret_val) return ret_val;
		RESET = TRUE;
		}
	    else {
		// Here we inform low that we are preparing;
		// we've used a property because we've attached reset
		// semantics to low->prepare_for_new_program, and we're
		// not wanting to call it for the other ARCs.
		low->process_property("prepare_for_new_program",
		    willdownload ? "1" : "0");
		}
	    }

	#define MAX_ARCS 64 // If MADI read fails.

	0 && printf("prepare.  cpunum %d nc %d\n",
		arcnum, arcnum_needs_checking);
	if (arcnum_needs_checking) {
	    // If we were assigned an arcnum, compute number of arcs,
	    // and evaluate our arcnum to be sure it's valid.

	    arcnum_needs_checking = FALSE;
	    if (number_of_arcs == UNINITIALISED) {
		// MADI initialisation: read the number of ARCs.
		IS_CMPD = TRUE;

		/* Read the ARC version number first */
		if (arc_version == UNINITIALISED) {
		    write_reg (reg_DEBUG, FH_M);
		    arc_version = get_arc_version_number();
		    }

		if (arc_version >= 7) {
		    ULONG nb_arcs; read_reg(reg_MADI_build, &nb_arcs);
		    unsigned long ident; read_reg(reg_IDENTITY, &ident);
		    if (ident == nb_arcs) {
			// We have a valid ident reg and it matches
			// madi.  Thus, there is no MADI.
			// However, this may be a chained JTAG implementation,
			// so allow it to proceed.
			number_of_arcs = strstr(low->id(),"JTAG")
			    ? MAX_ARCS
			    : 1;
			}
		    else number_of_arcs = (nb_arcs & MAN_M) >> MAN_V;
            
		    if (number_of_arcs == 0) {
			// number_of_arcs = 1; // single ARC system
			//!Tim: -arc7 always has 1 for madi register?
			if (dbg) 
			    xprintf("\nProcess property cpunum "
				"setting number arcs to %d\n "
				"Fail to read MADI build number arcs\n", 
				MAX_ARCS);
			number_of_arcs = MAX_ARCS; 
			// This was added because some version of the 
			// debugger.
			}
		    else /* Multi ARC system */
			current_arc = arcnum; // set current ARC in multiplexor
		    }
		else {
		    // number_of_arcs = 1;
		    if (dbg)  
			xprintf("\nProcess property cpunum "
			    "setting number arcs to %d; "
			    "ARC version too small, or "
			    "DEBUG register read failure.\n", MAX_ARCS);
		    number_of_arcs = MAX_ARCS; 
		    }
		}

	    if (arcnum > number_of_arcs) {
		xprintf("cpunum %d exceeds number_of_arcs %d.\n", 
			arcnum, number_of_arcs);
		// The process will be killed by the debugger.
		return 0;
		}
	    }

        // get the ARC version number if it has not been done yet.
        // The board ID may have been read in cpunum process_property for CMPD

        write_reg(reg_DEBUG, FH_M); // Force Halt the ARC
	       
        if (arc_version == UNINITIALISED)
            arc_version = get_arc_version_number();

        if (arc_version >= 7 && arc_version < 0x21) {  
	    // only for ARC version 7 and above, but not for A6 and later,
	    // as action points for A6 and later are radically different
	    // from A5 and before, and are handled by the debugger, not
	    // by this DLL.
	    /* actionpoints initialisation */
	    /* determine the number of actionpoints in hardware */
            /* ARC version 7 and above may have actionpoints */
            ULONG nb_ap = 0; read_reg(reg_AP_build, &nb_ap);

            if (nb_ap != 0) {
                /* there are actionpoints: how many? */
                switch((nb_ap & NAP_M) >> NAP_V) {
                    case 0: number_of_AP = 2; break;
                    case 1: number_of_AP = 4; break;
                    case 2: number_of_AP = 8; break;
		    }
		}
            else 
                number_of_AP = 0;
	    }

        /* Remove all the watchpoints/breakpoints  */
        AP_used = 0x00; // set all the actionpoints unused

        for (int i=0; i<number_of_AP; i++) {
            ULONG ap = (i << AN_V) | AP_DIS; // set the ap no and disable it
            write_reg(reg_ACR, ap );
            ap_table[i].type = UNUSED; // update the actionpoint table
	    }

        return SUCCESS;
	}


    NOINLINE void more_help() {
	char *help_string = 
	    "    -off=new_driver\t-- use old gportio.sys driver\n"
	    "    prop reset_delay=milliseconds\t-- delay after reset called\n"
	    "    prop reset_board\t-- call reset in interface DLL\n"
	    "    prop dbg_int=1\t-- log interface level calls to logfile.txt"
		    "\n\t\t\t(=0 to turn off)\n"
	    "    prop dbg_proto=1\t-- log protocol (JTAG/Parallel) "
		    "calls to logfile.txt \n\t\t\t(=0 to turn off)\n"
	    "    prop dbg_port=1\t-- log IO port calls to logfile.txt"
		    "\n\t\t\t(=0 to turn off)\n"
	    "    prop log_stdout=1\t-- send log to stdout instead of logfile.txt"
		    "\n\t\t\t(=0 to turn off)\n"
	    "    -off=invalidate_icache -- skip invalidating icache"
		    " on memory writes\n"
	    "    -on=respect_endian\t-- reverse mem reads/writes for big-endian"
		    " ARC target\n"
	    "    -off=action_point_ranges\t-- disable < and > action point pairing"
	            "\n\t(which gives you watchpoints over an address range)\n"
	    "    -off=dc_flush\t-- do not allow write to data cache flush register\n"
	    "    -on=trace_discover\t-- trace JTAG chain discovery\n"
	    "    prop slow_clock=number\t-- slow clock with busy loops\n"
	    "        (Use such values as 50-500 for parallel port\n"
	    "         and 1-5 for jtag port.)\n"
	    "For JTAG targets only:\n"
	    "    prop reset_jtag\t--reset JTAG state machine\n"
	    "    prop loop_test\t--run JTAG bypass mode loop-back test\n"
	    "    prop reset_always\t--reset JTAG state after every transaction\n"
	    "    prop retry_jtag=number\t-- 0=forever\n"
	    "For ARCAngel3 targets only:\n"
	    "    -DLLprop=gclk=<clock source>\t-- Same as gclk3 (below)\n"
	    "    -DLLprop=gclk0=<clock source>\t-- Sets clock source for gclk0\n"
	    "    -DLLprop=gclk1=<clock source>\t-- Sets clock source for gclk1\n"
	    "    -DLLprop=gclk2=<clock source>\t-- Sets clock source for gclk2\n"
	    "    -DLLprop=gclk3=<clock source>\t-- Sets clock source for gclk3\n"
	    "    -DLLprop=harvard=1\t-- Activate phase-shifted clock "
		    "for harvard builds\n"
	    "    <clock source> is of the form:\n"
	    "        crystal   -- use the physical crystal\n"
	    "        dips      -- use the use the crystal divided\n"
	    "                     by the DIP switch divisors\n"
	    "        highimp   -- set the clock source to high impedance\n"
	    "        host      -- use the STR singla of the host interface\n"
	    ;
	xprintf("%s",help_string);
	}

    /*********************************************************
    * int process_property(const char *property, const char *value)
    *
    * Process a property setting.  This is used to communicate arbitrary
    * values to the hardware.
    *
    * If you "own" the property, return 1 (whether or not processing the
    * property was successful); if not, return 0.
    **********************************************************/
    override int MS_CDECL process_property(
    	const char *property, const char *value) {
	#define LDB 0
	LDB && printf("!process prop %s=%s\n",property,value);
        
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] process_property() %s = %s ",arcnum, property ,value );
	    }
	/**************************
	 Please do NOT access the hardware in this routine, unless
	 you know we have already executed prepare_for_new_prgoram,
	 in which we do initialization and potential reset.
	 MADI code was removed from here and placed in prepare_... .
	 The debugger will not request hardware access until it
	 calls prepare_... .  Untimely hardware access here may fail
	 as the board may not be initialized.
	 *************************/

        // First, properties that belong to the API layer and that should
        // never go to the low-level layer.

	if (strcmp(property,"is_simulator") == 0) 
	    return 4;	// Indicates value of is_simulator doesn't change.
        if (!os->stricmp(property,"lowdll")) {
            if (low_dll_arc) {
                low = low_dll_arc;
                return 1;
		}

            low_dll_loader = DLL_load::new_DLL_load(); //low_dll_loader = DLL_Load object (see dll.cpp)
            int OK = low_dll_loader->load_DLL(value); // load dll
            
            if (OK == 0) {
                printf("Error: cannot load low-level IO DLL %s.\n", value);
                goto HORRIBLE_FAILURE;
		}

            {
            //get pointer to get_ARC_interface function
            const char *pname = "get_ARC_interface";
            void *p = low_dll_loader->get_proc_address(pname);
        
            //if no pointer exists -> error !, can't be correct DLL
            if (p == 0) {
                printf("Error: %s does not export %s.\n",value,pname);
                goto HORRIBLE_FAILURE;
		}

            //low_dll_arc = get_ARC_interface ()  (from the low-api dll)
            low_dll_arc = ((ARC* (*)())p)();
        
            //if no *ARC is returned,  error!
            if (low_dll_arc == 0) {
                printf("Error: %s::%s returns 0!\n",value,pname);
                goto HORRIBLE_FAILURE;
		}
        
            low = low_dll_arc;    //addition of low ARC is made
        
            return 1;
	    }
      
	    HORRIBLE_FAILURE:
	    printf("Cannot function without a correct low-level API.\n");
	    exit(1);
	    }

	if (os->stricmp(property,"arc_dll_help") == 0) {
	    more_help();
	    return 1;
	    }

	if (!os->stricmp(property,"timeout") ||
	    !os->stricmp(property,"icnts") ||
	    !os->stricmp(property,"killeds") ||
	    !os->stricmp(property,"delay_killeds") 
	    ) {
	    // TBH Tired of the debugger passing ISS properties and reporting.
	    return 1;  
	    }

	if (!os->stricmp(property,"num_arcs")) {  // TBH
	    // Forcibly set # of ARCs in case of other problems. Added by Tim H.
            int temp = get_integer(value);

	    if (temp >= 1 && temp < MAX_ARCS)
		number_of_arcs = temp;
	    else {
		xprintf("Illegal value for num_arcs:%s\n", value);
		return 0;
		}

	    xprintf("Force number of arcs to %d.\n", number_of_arcs);
	    IS_CMPD=1;
	    return 1;
	    }
    
        if (!os->stricmp(property,"cpunum")) {
	    // assign the arc number to a real ARC.
	    // This property is only passed when debugging a multiARC system.
        
	    arcnum = get_integer(value);
	    // We check the arcnum later when we prepare.
	    // We avoid for now reading registers until we've prepared.
	    arcnum_needs_checking = TRUE;

	    // Tell the low-level guy which he is.
	    // This usually doesn't matter, but for JTAG it does.
	    low->process_property(property,value);
            return SUCCESS;
	    }
    
        if (!os->stricmp(property,"cycle_step")) {
	    // The user wants to use the old step mechanism even if 
	    // ARC version is 7 or above.
            sscanf(value,"%d",&CYCLE_STEP);
            return SUCCESS;
	    }

	bool value_is_1 = *value == '1';

        if (!os->stricmp(property,"gverbose")) {
            gverbose = value_is_1;
	    low && low->process_property("gverbose", value);
	    return SUCCESS;
	    }

        if (!os->stricmp(property,"dbg_int")) {
            dbg = value_is_1;
            //set debug_int in low level api
            low && low->process_property("dbg_int", value);
            return SUCCESS;
	    }

        if (!os->stricmp(property,"log_stdout")) {
            log_to_stdout = value_is_1;
            return SUCCESS;
	    }
        if (!os->stricmp(property,"flush_stdout")) {
	    fflush(stdout);	// Helps to post tracing to a file.
            return SUCCESS;
	    }
        if (!os->stricmp(property,"respect_endian")) {
	    respect_endian = value_is_1;
            return SUCCESS;
	    }
        if (!os->stricmp(property,"endian")) {
	    // The debugger is telling us the endianness of the target.
	    // The value is either LE or BE.
	    if (respect_endian)
		host_and_target_endian_are_the_same = 
		    (strcmp(value,"LE") == 0) == host_is_little();
	    0 && printf("HTES %d\n",host_and_target_endian_are_the_same);
            return SUCCESS;
	    }
        if (!os->stricmp(property,"port")) {
	    // TBH added 23 JUL 2002 
	    port_setup=TRUE;
	    // Continue to do lowio port setup
	    }

	if (!os->stricmp(property,"new_bp_cookie")) {
	    new_bp_cookie = value_is_1;
            return SUCCESS;
	    }

	if (!os->stricmp(property,"dc_flush")) {
	    dc_flush = value_is_1;
            return SUCCESS;
	    }

	if (!os->stricmp(property,"action_point_ranges")) {
	    action_point_ranges = value_is_1;
            return SUCCESS;
	    }

	if (!os->stricmp(property,"slow_halt")) {
	    slow_halt = get_integer(value);
            return SUCCESS;
	    }

        if (!os->stricmp(property,"reset_board"))   {  
	    // Added for -chipinit case v3.1 notes; however, not
	    // generally need now as chipinits don't execute until
	    // prepare_for_new_program has been called.
            if (arcnum==1 || !IS_CMPD) {
		// Don't prepare for non-#1 ARC.
		// For the low-level, this can cause IO initialization
		// of the comm protocols, which is not what we want.
                if (low) {
		    int r = force_reset();
		    if (!r) return r;
		    }
		}
            
            return SUCCESS;
	    }
        
        // Now can follow properties that are OK for the low-level layer.
        // In particular, debugging the protocols belong there.
        // So now we can send a property to the lower-level layer (if there
        // is one).
        // Put this here, so that the low DLL doesn't get the lowdll
        // or cpunum property.  Those must go to the API layer.
        int i = low && low->process_property(property,value);
    
        #if TRACING
	// Prolog, epilog tracing for debugging.  MetaWare High C only.
	extern "C" char TRACE_DBG;
    
	if (!os->stricmp(property,"trace")) {
	    TRACE_DBG = value_is_1;
	    return SUCCESS;
	    }
        #endif

	LDB && printf("!done with PP\n");
        return i;
	}


    /*********************************************************
    * From here onwards are the extended functions of version 2 of the
    * interface.
    **********************************************************/


    /*********************************************************
    * int set_reg_watchpoint(ARC_REG_TYPE r, int length)
    *
    * Set a register watchpoint. This stops the processor whenever
    * the register changes.
    * r is the register number, length is byte unit.
    * Attempt to use an actionpoint.
    * Returns 1 if succeeded, otherwise 0.
    **********************************************************/
    override int MS_CDECL set_reg_watchpoint(ARC_REG_TYPE r, int length) {

        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] set watchpoint on r%d", arcnum, r);
	    }

        if ( number_of_AP > 0) {
            /* try to get a free actionpoint number */
	    int free_actionpoint = get_free_actionpoint();

            if (free_actionpoint != -1) {
		ULONG ap_reg, ap_value;
                /* an actionpoint is available */
                /* update the ap table */
                ap_table[free_actionpoint].type = WATCH_REG;
                ap_table[free_actionpoint].data.reg= r;

                /* set the ap registers and ap mode */
                if (r >= 0 && r<AUX_BASE) {
                    /* for core reg watchpoint */
                    ap_reg = r;
                    ap_value = (free_actionpoint << AN_V) | CORE_REG_MODE;
		    }
                else {
                    /* for aux reg watchpoint */
                    ap_reg = r - AUX_BASE;
                    ap_value = (free_actionpoint << AN_V) | AUX_REG_MODE;
		    }

                /* program the hw actionpoint */
                write_reg(reg_ADCR, ap_reg);
                write_reg(reg_ACR, ap_value);

                return SUCCESS;
		}
	    }
    
        return FAIL;
	}


    /*********************************************************
    * int remove_reg_watchpoint(ARC_REG_TYPE r, int length)
    *
    * Removes a register watchpoint.  If we have such a watchpoint set, it
    * removes it, and returns 1; otherwise 0.
    **********************************************************/
    override int MS_CDECL remove_reg_watchpoint(ARC_REG_TYPE r, int length) {
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] remove watchpoint on r%d", arcnum, r);
	    }

        if (number_of_AP > 0) {
            // determine which actionpoint has been selected for this watchpoint
            // NB: the debugger refuses to set 2 watchpoints on the same reg
            for (int i = 0; i< number_of_AP; i++) {
                if ( (ap_table[i].type == WATCH_REG) && 
			(ap_table[i].data.reg == r)) {
                    /* this is the right actionpoint */
                    remove_actionpoint(i);
                    return SUCCESS;
		    }
		}
	    }
    
        return FAIL; // no actionpoints matching this watchreg
	}

    /*********************************************************
    * int set_mem_watchpoint(unsigned long addr, int length)
    *
    * Sets/removes a memory watchpoint. Similar to reg watchpoints.
    * notice that these memory watchpoints are set
    * on write to specific memory address, from core only
    **********************************************************/
    override int MS_CDECL set_mem_watchpoint(unsigned long addr, int length) {
        ULONG ap_addr, ap_pgm;

        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] set mem watchpoint from 0x%+08x to 0x%+08x", arcnum, addr, addr + length);
	    }

        /* the current actionpoint mechanism only allows to track
        32-bit words, word aligned.
        This will be improved in the future.
        We accept to set 2 mem actionpoints if the length is 8 bytes.
        We reject all the other requests: non word aligned addresses and
        non multiples of 4 bytes length */
        if ( (addr%4) != 0) return FAIL;
    
        if (length %4 != 0) return FAIL;

        if ( number_of_AP > 0) {
            if (length == 4) {
                /* we program an ap that will halt the ARC if the
		32-bit value at the 'addr' address has changed */
		int free_actionpoint = get_free_actionpoint();

                if (free_actionpoint != -1) {
		    APinfo &A = ap_table[free_actionpoint];
                    /* update the ap table */
                    A.type = WATCH_MEM;
                    A.data.address = A.interface_address = addr;

                    /* set the memory address to watch and the ap mode */
                    ap_addr = addr;
                    ap_pgm = 
		    	(free_actionpoint << AN_V) 
		    	|  AC_EQ | AT_MWA | AP_ENA;

                    /* program the hw actionpoint */
                    write_reg(reg_ADCR, ap_addr);
                    write_reg(reg_ACR, ap_pgm);
            
                    return SUCCESS;
		    }
		}
	    else if (length == 8) {
		// length = two words.
                // program 2 actionpoints
                // the first one will be triggered if the first word changes
                // the second one will be triggered if the secnd word changes

                // try to get two actionpoints.  They don't need to be pairable.
		int ap1 = get_free_actionpoint(), ap2 = get_free_actionpoint();
		if (ap1 == NO_FREE_AP) return FAIL;
		else if (ap2 == NO_FREE_AP) {
		    remove_actionpoint(ap1);
		    return FAIL;
		    }

		APinfo &AP1 = ap_table[ap1], &AP2 = ap_table[ap2];

                /* pgm the first ap */
                AP1.type = WATCH_MEM;
                AP1.data.address = ap_addr = addr;

                // this memory watchpoint uses 2 ap, records the scnd one used 
                ap_table[ap1].paired_ap = ap2;

                // set the mem addr to watch and ap mode 
                ap_pgm = ( ap1 << AN_V) |  AC_EQ | AT_MWA | AP_ENA;

                /* program the hw actionpoint */
                write_reg(reg_ADCR, ap_addr);
                write_reg(reg_ACR, ap_pgm);

                /* pgm the second ap */
                AP2.type = WATCH_MEM;
                AP2.data.address = ap_addr = addr + 4;
                AP2.paired_ap = ap1;

                // set the memory address to watch (the next word) and ap mode 
                ap_pgm = ( ap2 << AN_V) |  AC_EQ | AT_MWA | AP_ENA;

                /* program the hw actionpoint */
                write_reg(reg_ADCR, ap_addr);
                write_reg(reg_ACR, ap_pgm);
        
                return SUCCESS;
		}
            else if (action_point_ranges && 
		    addr > 0 && (addr+length) < 0xffffffff) {
		// Use paired action points: > for the first, < for the second.
                // get two actionpoints */
                int ap1,ap2; 
		get_free_actionpoint_pair(ap1,ap2);
                if (ap1 == NO_FREE_AP) return FAIL;

		APinfo &AP1 = ap_table[ap1], &AP2 = ap_table[ap2];

                AP1.type = WATCH_MEM;
		// Comparison is >, hence -1:
                AP1.data.address = ap_addr = addr-1;	
		AP1.interface_address = addr;
                AP1.paired_ap = ap2;

                ap_pgm = (ap1 << AN_V) | AC_GT | AT_MWA | AP_ENA;

                /* program the hw actionpoint */
                write_reg(reg_ADCR, ap_addr);
                write_reg(reg_ACR, ap_pgm);

                AP2.type = WATCH_MEM;
                AP2.data.address = ap_addr = addr+length;
                AP2.paired_ap = ap1;

                ap_pgm = (ap2 << AN_V) | AC_LT | AT_MWA | AP_ENA |
			(1 << 8);	// Specify that this is paired w/prior.

                /* program the hw actionpoint */
                write_reg(reg_ADCR, ap_addr);
                write_reg(reg_ACR, ap_pgm);
                return SUCCESS;
		}
	    }

        return FAIL; // in any other case (arc_version <7, no AP available...)
	}

    override int MS_CDECL remove_mem_watchpoint(
	    unsigned long addr, int length) {
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] remove mem watchpoint at 0x%+08x to 0x%+08x", arcnum, addr, addr+length);
	    }

        if ( number_of_AP > 0) {
            /* determine which actionpoint has been programmed for this watchpoint */
            for (int i = 0; i< number_of_AP; i++) {
	        APinfo &A = ap_table[i];
                if ((AP_used & (1<<i)) && 
			A.type == WATCH_MEM && A.interface_address == addr) {
                    /* find out if this watchmem used two actionpoints */
                    if (A.paired_ap != NO_FREE_AP && length > 4)
                        // remove the associated actionpoint.
                        remove_actionpoint(A.paired_ap);
            
                    remove_actionpoint(i);
            
                    return SUCCESS;
		    }
		}
	    }

        return FAIL;
	}

    /*********************************************************
    * int stopped_at_watchpoint()
    *
    * Tells whether we stopped at a watchpoint.  1 if we did.
    * The reason for this function is that we have no processor-defined
    * way to determine this, and the debugger would like to know we
    * stopped at a watchpoint.  However, we might be able to remove the
    * requirement for this function in the future.
    **********************************************************/
    override int MS_CDECL stopped_at_watchpoint() {
        ULONG debug_value;
        ULONG halt_addr;
        int AP_number;
        int status = FAIL;

        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] stopped_at_watchpoint()", arcnum);
	    }

        if ( number_of_AP > 0) {
            /* read the debug register */
            read_reg(reg_DEBUG, &debug_value);

            if ( ((debug_value & AH_M)>> AH_V) == 1) {
                /* an ap has been triggered */

                /* determine what addr caused the halt */
                read_reg(reg_APCR,&halt_addr);

                /* check the status of each actionpoints */
                debug_value = (debug_value & ASR_M) >> ASR_V;

                AP_number = 0;
        
                while (AP_number < number_of_AP) {  // for each ap
                    if ((debug_value >>AP_number)& 0x00000001) {
                        /* this actionpoint has been triggered */
                        /* check if it is a watchpoint (reg/mem)*/

                        if ( ap_table[AP_number].type == WATCH_REG) 
                            status = SUCCESS;
                        else if (ap_table[AP_number].type == WATCH_MEM) 
			    status = SUCCESS;
			}
            
                    AP_number ++;
		    }
        
                return status;
		}
	    }

        return FAIL;
	}


    /*********************************************************
    * int stopped_at_exception()
    *
    * Exceptions
    * Returns non-zero (an exception number) if the processor stopped due
    * to some exception (otherwise unobtainable from processor status regs).
    * For example, the simulator stops on bad instruction or bad memory ref.
    * This is experimental; return 0 for now.
    **********************************************************/
    override int MS_CDECL stopped_at_exception() {
        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] stopped_at_exception()", arcnum);
	    }

        return FAIL;
	}


    /*********************************************************
    * int set_breakpoint(unsigned addr, void *cookie)
    *
    * Set a breakpoint at the given address; returns 1 if succeeded, else 0
    * We provide you a "cookie_buf" where you can deposit information that
    * we'll hand back to you on a remove_breakpoint. The cookie buf is of
    * length breakpoint_cookie_len that you tell us. If you return 0,
    * the debugger will attempt to set a breakpoint by using the standard
    * jump to "flag 1; nop; nop; nop".
    **********************************************************/
    override int MS_CDECL set_breakpoint(unsigned addr, void *cookie) {
        int free_actionpoint;
        unsigned long bkp;

        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] set_breakpoint() 0x%x",arcnum ,addr);
	    }

        /* check if this ARC has got ap */
        if ( number_of_AP > 0) {
            /* if we've still got unused ap, then retrieve its number and mark it as used */
            free_actionpoint = get_free_actionpoint();

            if (free_actionpoint != -1) {
		APinfo &api = ap_table[free_actionpoint];
		bkpinfo_struct &bkp_info = api.bkp_info;

                /* we set an actionpoint on the PC */
                /* write the data value for comparison */
                write_reg(reg_ADCR, addr);
                
                /* set the actionpoint mode */
                bkp = (free_actionpoint << AN_V) | BKP_MODE;

                /* pgrogram tthe hw actionpoint */
                write_reg(reg_ACR, bkp);

                /* record the bkp information in the cookie */
                bkp_info.actionpoint_number = free_actionpoint;
                bkp_info.address = addr;
        
                /* copy in the cookie mem space. A local variable is used and
                the cookie get a copy of it because the cookie addr may not be aligned */
		if (new_bp_cookie) {
		    // If new format, we own the cookie memory, 
		    // not the debugger.  We return to the debugger 
		    // pointer to our cookie.
		    api.cookie = *(void**)cookie = &bkp_info;
		    }
		else {
		    memcpy(cookie, &bkp_info, sizeof(bkpinfo_struct));
		    api.cookie = cookie;
		    }

                /* update ap table */
                api.type = BKP;
                api.data.address = addr;
        
                return SUCCESS;
		}
	    }

        return FAIL;
	}


    /*********************************************************
    * int at_breakpoint()
    *
    * Tell whether the reason we have stopped is because we are at a
    * breakpoint. Return 1 iff we are at a breakpoint.  The PC is assumed
    * to point to the location of the broken instruction.  This is relevant
    * only for breakpoints set via set_breakpoint above.
    **********************************************************/
    override int MS_CDECL at_breakpoint() {

        ULONG debug_value;
        int AP_number;
        // int status = FAIL;
        ULONG halt_addr;
	int found_ap_number = UNINITIALISED;

        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] at_breakpoint()", arcnum);
	    }

        if (number_of_AP > 0) {
            /* test the HALT BIT */
            read_reg(reg_DEBUG, &debug_value);
        
            if ( ((debug_value & AH_M)>> AH_V) == 1) {
                /* determine what addr caused the halt */
                read_reg(reg_APCR,&halt_addr);

                /* determine which actionpoints have been triggered */
                debug_value = (debug_value & ASR_M) >> ASR_V;
                AP_number = 0;

                while (AP_number < number_of_AP) {  // for each ap
                    if (debug_value & 0x00000001) {
                        /* check if it is a bkp and check if the recorded address matches
                        note that there is no delay on PC actionpoint */
                        if (((ap_table[AP_number]).type == BKP) && 
			    ((ap_table[AP_number]).data.address == halt_addr)) {
                            // status = SUCCESS;
			    found_ap_number = AP_number;
			    }
			}
            
                    debug_value >>= 1;
                    AP_number ++;
		    }
		}
	    }

        // Because we support ARC_FEATURE_at_breakpoint_cookie:
	// If at_breakpoint() is true, return the cookie associated with
	// this breakpoint so the debugger can determine the actual breakpoint
	// hit.  This is needed for ARCompact where action points slide past
	// a breakpoint location, and the PC address alone is not sufficient
	// for the debugger to determine which breakpoint has been hit.
	// For example, if you have action points at A, A+2, and A+4,
	// and the processor stops due to the action point at A, the program
	// counter could be at A, A+2, or A+4, depending upon characteristics
	// of the program.  So, return a cookie here that allows the debugger
	// to correlate the found action point with a code address.

	return found_ap_number != UNINITIALISED
	    ? (int)ap_table[found_ap_number].cookie
	    : 0;

	// OLD METHOD:
        // return status;
	}


    /*********************************************************
    * int remove_breakpoint(unsigned addr, void *cookie)
    *
    * Remove a breakpoint at the given address that was previously set
    * by set_breakpoint. Returns 1 if succeeded, 0 otherwise:
    **********************************************************/
    override int MS_CDECL remove_breakpoint(unsigned addr, void *cookie) {

        int ap_number;

        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] remove_breakpoint at 0x%x",arcnum, addr);
	    }

        /* check ARC version and and if this ARC's got ap */
        if (number_of_AP > 0) {
            /* get the cookie back into the local variable because the cookie may be unaligned */
            bkpinfo_struct bkp_info;
	    bkpinfo_struct *bpi = &bkp_info;

	    if (new_bp_cookie) bpi = (bkpinfo_struct*)cookie;
	    else memcpy(&bkp_info, cookie, sizeof(struct bkpinfo_struct));

            /* get the ap number */
            ap_number = bpi->actionpoint_number;

            /* check if the addresses are the same and if we effectively recorded a bkp */
            if ((ap_table[ap_number].type == BKP) && 
	    	(ap_table[ap_number].data.address == addr)) {
                remove_actionpoint(ap_number);
                return SUCCESS;
		}
	    }
    
        return FAIL;
	}


    /*********************************************************
    * int retrieve_breakpoint_code(unsigned address,
    *     char *dest, unsigned len, void *cookie_buf)
    *
    * If your algorithm involved saving and overwriting the code at the
    * address, this function writes the saved code at dest for a maximum
    * length of len. This is typically used to be able to present a
    * disassembler the original code for disassembly.
    **********************************************************/
    override int MS_CDECL retrieve_breakpoint_code(unsigned address,
          char *dest, unsigned len, void *cookie_buf) {
        /* We don't overwrite the code, so no need to do something here. */

        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] retrieve_breakpoint_code()", arcnum);
	    }

        return SUCCESS;
	}


    /*********************************************************
    * int breakpoint_cookie_len()
    *
    * Specify the size of the cookie_buf that we need to provide you for
    * breakpoint information storage.  The buffer is not guaranteed to be
    * aligned.
    **********************************************************/
    override int MS_CDECL breakpoint_cookie_len() {

        if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] breakpoint_cookie_len()", arcnum);
	    }

        return (sizeof(struct bkpinfo_struct));
	}

    
    /*********************************************************
    * int supports_feature() 
    *
    * Returns OR of features required for support from version 5
    * See arcint.h for more details !!!
    * 
    **********************************************************/
    override int MS_CDECL supports_feature() {
	if (dbg) {
            printInFile("\n-------------");
            printInFile("\n[%d] supports_feature()", arcnum);
	    }

        //currently support only 1 feature => read/write banked regs
        return ARC_FEATURE_banked_reg | ARC_FEATURE_at_breakpoint_cookie |
	       (new_bp_cookie ? ARC_FEATURE_new_bp_cookie : 0);
	}
    };

// Verify proper implemenation of function signatures.
// If you have intended to override a function but goofed, you'll
// get an error message from the compiler.  Microsoft C++ doesn't support
// MetaWare High C++'s "override" so we resort to this fancy macro.
#define IMPNAME API_ARC
//CJ-- CHECK_OVERRIDES(ARC)

//STATICS
//********************************************

ARC *API_ARC::low_dll_arc = 0;               // hold the low level dll 
ARC *API_ARC::low = 0;                       

bool API_ARC::IS_CMPD = FALSE;               // if false not using madi, if true is using madi
bool API_ARC::printed_start_message = FALSE;
int API_ARC::num_ARC_connected = 0;          // number of ARCs currently connected
int API_ARC::number_of_arcs = UNINITIALISED; // number of ARCs in a multiARC system
int API_ARC::current_arc = UNINITIALISED;    // indicates which ARC points the multiplexor
//*********************************************

extern "C" 
#if _MSC_VER
    __declspec(dllexport) 
#endif

ARC *get_ARC_interface() {
    #if _LINUX
    static bool first_time = TRUE;
    if (first_time) {
	extern "C" __mw_cpp_init();
	__mw_cpp_init();
	}
    first_time = FALSE;
    #endif

    // See if a low-level ARC is builtin.
    // If not, we'll require a lowdll= specification.
    if (API_ARC::low == 0)
        API_ARC::low = get_low_ARC();

    ARC *p = new API_ARC(API_ARC::low);
    
    // If you are getting duplicate static data on NT, that's a bug on NT,
    // a workaround for which is provided for by SeeCode version 3j or later.
    // This printf should always print the same address; if it's different,
    // you have the problem, and this DLL won't work right in a MADI context.
    // printf("get_ARC &api=%x\n",&API_ARC::low);

    #if DEBUG_INTERFACE
    printInFile("\n-------------");
    printInFile("\nget_ARC_interface() ARC structure @ 0x%x\n",p);
    #endif

    return p;
    }

//********************************************************
// INTERNAL FUNCTIONS
//********************************************************


/*********************************************************
* int get_free_actionpoint(ARC *p)
*
* Return the first unused actionpoint number. (in the range of 0 .. 7)
* set the corresponding bit in the AP_used vector to 1
* return -1 if no AP available
**********************************************************/
int API_ARC::get_free_actionpoint() {

    for (int i=0; i< number_of_AP; i++) {
        if ((AP_used & (1<<i) ) == 0) {
            // AP i is free.  Mark it as used.
            AP_used |= 1<<i;
            return i;
	    }
	}

    return NO_FREE_AP;
    }

void API_ARC::get_free_actionpoint_pair(int &ap1, int  &ap2) {
    // Find two adjacent APs.  If we can't, ap1 == NO_FREE_AP.
    ap1 = NO_FREE_AP;
    for (int i=0; i< number_of_AP-1; i++) {
        if ((AP_used & (3<<i) ) == 0) {
            // i,i+1 are free.  Mark as used.
            AP_used |= 3<<i;
	    ap1 = i; ap2 = i+1;
            return;
	    }
	}
    }

/*********************************************************
* void remove_actionpoint(ARC* p, int ap_number)
*
* Set ap_number bit of the actionpoint vector to 0
*( ap_number is assumed to be between 0 and 7.
* Disable the relative ap
**********************************************************/
void API_ARC::remove_actionpoint(int ap_number) {

    /* update AP_used vector  */
    AP_used &= ~(1<<ap_number);

    /* disable the hardware */
    /* set the ap number and mode (disable) */
    ULONG dis_ap = (ap_number << AN_V) | AP_DIS;
    /* disable the hw ap */
    write_reg(reg_ACR, dis_ap);

    /* update the ap table */
    ap_table[ap_number].type = UNUSED;
    ap_table[ap_number].paired_ap = -1;
    }


/*********************************************************
* int switch_madi(ARC* p)
*
* Switch the MADI block to point to the ARC number arcnum
* return 1 if ok.
* return 0 if there was a pb when accessing madi reg.
**********************************************************/
int API_ARC::switch_madi() {

    0 && printf("switch madi? cmpd %d CA %d an %d\n",IS_CMPD,current_arc,arcnum);
    if (IS_CMPD) {
        /* there is a madi block */
        if (current_arc != arcnum) {	
	    0 && printf("%x:Switch: from(current_arc) %d to(arcnum) %d\n",
	    	this,current_arc,arcnum);

            ULONG madi_ctrl;
            //we have to switch
            madi_ctrl = (arcnum-1) | SINGLE_CAST_MODE;
            current_arc = arcnum;
            
            return low->write_banked_reg(reg_bank_madi, reg_MCR, &madi_ctrl);
	    }
        
        return SUCCESS; // don't need to swictch
	}
    else return 1; // we return 1 even if there is no madi
    }



/*********************************************************
* int get_arc_version_number() 
* 
* Return the ARC version number
**********************************************************/
int API_ARC::get_arc_version_number() {
    identity_register = 0;
    read_reg(reg_IDENTITY, &identity_register);
    return identity_register & 0x000000ff;
    }

/*********************************************************
* int print_startup_message() 
* 
* Prints out the DLL start-up message 
**********************************************************/
void API_ARC::print_startup_message() {
    // print start-up message once - toggle the bool
    if (printed_start_message) return;
            
    const char *my_id = API_ARC::low ? API_ARC::low->id() : "API";
            
    //printf("****************************************************\n");
    //printf("*** DLL under development - NOT FOR DISTRIBUTION ***\n");
    //printf("****************************************************\n");

    if (gverbose) {
	//CJ-- xprintf("*** ARC %s DLL %s[%s] (C) 1999-2004 ARC International. \n", my_id, VERSION, __DATE__);
	//CJ-- xprintf("*** Say `prop arc_dll_help' for internal help.\n");

	}
    printed_start_message = TRUE; //Toggle boolean so the messsage wont appear again.
    }

