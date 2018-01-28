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
*              Copyright 2000-2004 ARC International(UK) Ltd
*                          All Rights Reserved
*
* File name:    Jtagport.cpp
*
* Author   :    Stephane Bihan and Tom Pennello
*
* Date     :    19/02/99 and following
*
* Description : Jtag parallel control layer file for 95/NT DLL
*
*   This module is a part of the DLL interfacing the SeeCode debugger
*   and the ARCAngel board via the pseudo serial port (using 4 pins of
*   the parallel port) for Jtag module on ARCAngel.
*   This part controls and accesses the port.
*   This DLL implementation works either with Windows95 or WindowsNT
*
*   This forms the lower layer and works with three other ones,
*   the interface layer (api.cpp), the low level interface layer (lowio.cpp)
*   and the protocol layer(jtagproto.cpp).
*
*   The original file "remote.c" has been split in layers to give this
*   one.
*
*   (See "The debug components reference" document, chapter 3.  For
*   information on JTAG.)
*
* History:
*
* Version  Date     Author       Description
*
* 1.0      24/04/95 Fuzzz / Alex Holland / Nick Ross    File created.
* 1.1      14/11/00 Huw Lewis    Improved commenting, and neatness of code
* 1.whatever.  Tom P.  Completely overhauled this code, removing lots
*  of duplicated code; sped it up; prepared for A6 multiarc.
*
*******************************************************************************/


/**************************************************************************
 * NOTE (Huw Lewis 14/11/00):
 ****************************************************************************
 *  If RANDOM errors occur when using JTAG 
 *  (HostLink inefficiency/ (r/w) memory problems),
 *  most likely not a DLL problem.  
 *  These RANDOM problems are specific to Altera ARCAngels,
 *  and should not occur using any other target, including Xilinx ARCAngels !!!
 ****************************************************************************/


/////////////////////////////////////////////
// Multi-ARC support:
// This module operates in one of two modes.
// 1. There is a single ARC in the JTAG chain, with potential
//    foreign JTAGS on its left or its right (but not both).  
//    This single ARC can be a MADI-build ARC.
// 2. There are multiple ARCs in the JTAG chain, with potential
//    foreign JTAGS on its left or its right (but not both).  
//    The individual ARCs may NOT be MADI builds.  The n-th ARC
//    is addressed as the n-th cpu.
// The call to switch_madi tells jtagproto which mode we're in.
////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#undef NDEBUG
#include <assert.h>

#include "port.h"
#include "globals.h"
#include "jtagport.h"
#include "gportio.h"
#include "pioeval.h"    // Lump together the data writes.

#define JTAG_TDO    0x10    /* Jtag Data out */
#define JTAG_BUSY   0x80    /* Jtag Busy */
#define JTAG_RESET  0x08    /* Jtag reset   */
    
#define JTAG_TCK_0  0x00    /* clock signal */
#define JTAG_TCK_1  0x01


/*********************************************************
* Global Variables
**********************************************************/

#define MAX_RETRY 9 // Tim upped to 9.

/* Jtag state machine
   state[n] means you loop to that state on value n (0 or 1)
   Some states are labelled so you can indicate a xition to a label
   w/out drawing it.

test-logic-reset[1] <------------------------------------------------+
0 V                                  |
RTI: run-test-idle[0] -1> SDS:select-dr-scan -1-> select-ir-scan -1->+
               0 V             0 V
                   capture-DR -1-+         capture-IR -1-+
               0 V           |         0 V           |
               shift-DR[0] <-|-+       shift-IR[0] <-|-+
               1 V           | |       1 V           | |
              +-1- exit-DR    <--+ |  +-1- exit-IR    <--+ |
              |    0 V             |  |    0 V             |
              |    pause-DR[0]     |  |    pause-IR[0]     |
              |    1 V             |  |    1 V             |
              |    exit2-DR   -0-->+  |    exit2-IR   -0-->+
              |    1 V                |    1 V
              +--> update-DR -0->RTI  +--> update-IR -0->RTI
               1 V                     1 V
               SDS                     SDS

Any xition out of shift-DR/IR shifts a bit.
TDO is tied to the LSB of the shift register.
TDI gets shift into the register upon clock tick.
 */
#define RUN_TEST_IDLE_STATE     0x1
#define SELECT_DR_SCAN_STATE    0x02

// The ordering was designed to match the two globals
// RUN_TEST_IDLE_STATE and SELECT_DR_SCAN_STATE so we could use them
// as states directly.
#define STATES(Z) \
    Z(null,"null",null,null)                \
    Z(q_run_test_idle,"run-test-idle",q_run_test_idle,q_select_DR_scan) \
    Z(q_select_DR_scan,"select-DR-scan",q_capture_DR,q_select_IR_scan)  \
    Z(q_test_logic_reset,"test-logic-reset",q_run_test_idle,q_test_logic_reset)\
    Z(q_capture_DR,"capture-DR",q_shift_DR,q_exit_DR)       \
    Z(q_shift_DR,"shift_DR",q_shift_DR,q_exit_DR)       \
    Z(q_exit_DR,"exit-DR",q_pause_DR,q_update_DR)       \
    Z(q_pause_DR,"pause-DR",q_pause_DR,q_exit2_DR)      \
    Z(q_exit2_DR,"exit2-DR",q_shift_DR,q_update_DR)     \
    Z(q_update_DR,"update-DR",q_run_test_idle,q_select_DR_scan) \
                                \
    Z(q_select_IR_scan,"select-IR-scan",q_capture_IR,q_test_logic_reset)\
    Z(q_capture_IR,"capture-IR",q_shift_IR,q_exit_IR)       \
    Z(q_shift_IR,"shift_IR",q_shift_IR,q_exit_IR)       \
    Z(q_exit_IR,"exit-IR",q_pause_IR,q_update_IR)       \
    Z(q_pause_IR,"pause-IR",q_pause_IR,q_exit2_IR)      \
    Z(q_exit2_IR,"exit2-IR",q_shift_IR,q_update_IR)     \
    Z(q_update_IR,"update-IR",q_run_test_idle,q_select_DR_scan)     

#define Z(q,s,zero,one) q,
enum TAP_state { STATES(Z) TAP_state_array_size };
#undef Z
    
// C_CNT isn't even used for JTAG.  Why was this originally used in TCK1/TCK0?
#undef C_CNT
#define C_CNT 0

// Clock 1, then 0 for the jtag state machine:
unsigned char TCK1, TCK0;

/* || port pinouts:
    AA3: 
    Parallel   PC io   ARC    Xilinx
    port pin   port    usage  usage
               D C S            (data, control, or status byte)
I   1            C0    TCLK     (active low, so we write 0)
    2          D0             TDI
    3          D1             TCLK
    4          D2         TMS
    5          D3             
    6          D4      
    7          D5
    8          D6      TMS
    9          D7      TDI
    10         
I   11
    12
    13             S4  TDO
I   14
    15                  SS0:SS1 steering bits:
    16       C2    SS0      00 = reset   10 = fpga jtag
I   17       C3    SS1      01 or 11 = ARC jtag
^
Apparently the ones marked with I = inverted for hysterical reasons.


Byte offsets in PC port: Data = 0  status = 1  control = 2.

For xilinx, then, we need to just write the data register.
We assume the data latches; we'll see.

The using_xilinx boolean controls an attempt to use the data byte
for sending TCK1 and TCK0.  We haven't been able to get this to work.
The status register comes back 8f all the time.  It should toggle
between ef and ff (tdo 1 and tdo 0).
 */

static unsigned char tms_tdi_value;
static bool using_xilinx;

static unsigned char TMS_0;
static unsigned char TMS_1;
static unsigned char TDI_0;
static unsigned char TDI_1;

struct TAP_controller {
    // For now we have this just to check that our assumptions in the
    // code as to which state we are in is correct.  In the future
    // we may use this to automatically get to a state.
    struct { TAP_state dest[2]; char *name; } TAP[TAP_state_array_size];
    void setup_FSM() {
        // Inititalize the finite state machine.
    #define Z(q,s,zero,one) \
        TAP[q].dest[0] = zero; \
        TAP[q].dest[1] = one; \
        TAP[q].name = s;
    STATES(Z)
    #undef Z
    }
    void set_tms(int tms) { this->tms_on = tms & TMS_1; }
    char tms_on;
    TAP_state state;
    TAP_state current_state() { return state; }
    void take_transition() { 
        // if (errors < 10) printf("TAP: %s -> ",name(state));
    state = TAP[state].dest[tms_on != 0]; 
        // if (errors < 10) printf("%s\n",name(state));
    }
    void set_idle() { state = q_run_test_idle; }
    TAP_controller() { 
    tms_on = FALSE; 
    set_idle();
    setup_FSM();
    assert(q_run_test_idle == RUN_TEST_IDLE_STATE);
    assert(q_select_DR_scan == SELECT_DR_SCAN_STATE);
    errors = 0;
    }
    const char *name(TAP_state q) { return TAP[q].name; }
    int errors;
    Printf*print;
    void receive_printf(Printf*p) { this->print = p; }
    NOINLINE void check_state(/*TAP_state*/ int q,int line) {
        if (errors > 10) return;
        if (state != q) {
        print->printf("Line %d:TAP controller is in state %s; "
            "expected %s.\n",
            line, name(state),name((TAP_state)q));
        errors++;
        }
    }
    } static tap_controller;

#define check_state(q) check_state(q,__LINE__)
    
/*********************************************************
*  Function: print_jtag (UBYTE bits, char *state)
*
*  print out the contents of the bits (used for debug purposes only !)
**********************************************************/
static void print_jtag (UBYTE bits, char *state) {
    UBYTE temp = bits >> 5;
    unsigned char TMS = (temp & 0x2) >> 1;
    unsigned char TDI = (temp & 0x4) >> 2;
    temp = ((UBYTE) port->inp(
        (unsigned short)(epp_addr + PP_CONTROL_REG_OFFSET)))^C_XOR;
    unsigned char TCK = temp & C_STR;
    temp = ((UBYTE) port->inp(
        (unsigned short)(epp_addr + PP_STATUS_REG_OFFSET)))^S_XOR;
    unsigned char TDO = (temp & JTAG_TDO) != 0;
    printf("TMS %d TDI %d TDO %d TCK %d\n",TMS,TDI,TDO,TCK);
}

static void tick_the_clock(const char *state, int count = 1) {
    for (int i = 0; i < count; i++) {
        // Clock TCK up and down.
        if (!using_xilinx) {
            slow_clock_loop();
            port->outp((unsigned short)(epp_addr + PP_CONTROL_REG_OFFSET), TCK1);
            slow_clock_loop();
            port->outp((unsigned short)(epp_addr + PP_CONTROL_REG_OFFSET), TCK0);
            }
        else {
            // Give data time to stabilize.
            port->outp(epp_addr, TCK0|tms_tdi_value);
            port->outp(epp_addr, TCK1|tms_tdi_value);
            port->outp(epp_addr, TCK0|tms_tdi_value);
            }
        tap_controller.take_transition();
    }
}

enum {SIZE_OUTPUT = 3}; // tms/tdi write; clock 1; clock 0.
#define compute_entries(databytes) ((databytes+1)*SIZE_OUTPUT)

static void out_tms_tdi(int databyte) {
    if (using_xilinx) {
        // Wait 'til later to put the data, when we tick the clock.
        tms_tdi_value = databyte;
    }
    else port->outp(epp_addr, databyte);
        tap_controller.set_tms(databyte);
}

#if _LINUX
static bool allow_pio = FALSE;
#define use_extended_driver TRUE
#else
static bool allow_pio = TRUE;
#endif

struct JTAG_out {
    // Optimizing JTAG.  Don't repeat previous TMS/TDI value if it is
    // the same as before.
    // Three methods of output:
    // 1. Use new_driver's facility to do array write.  This is unnecessary
    //    as method 2 has always been available but the previous authors of this
    //    module never took advantage of it; hence work was done to add
    //    array write in the new driver itself; but this was unnecessary.
    //    (The array write is stored in a GPIO_ARRAY_STRUCT.)
    // 2. Use pio to buffer up the data.
    // 3. Directly write to port.  This is no longer used except perhaps
    //    for Linux.  To enable it, change the code to not use_pio if buf == 0.
    JTAG_out(GPIO_ARRAY_STRUCT *buf) {
        // If buf is 0, we write out immediately; o/wise we buffer
        // and blast all at once within driver.
        this->buf = buf;
        last_tms_tdi = -1;  // Invalid value.
        cnt = 0;
        use_pio = buf == 0 && allow_pio && slow_clock == 0;
        // pio has a 4K static buffer we can fill with commands.
        if (use_pio) pioInitInstr_(pio);
    }
    PioInstrStream pio;
    bool use_pio;
    static unsigned saved;
    void add(int tms_tdi, const char *comment);
    /*NOINLINE*/ virtual void send() {
#if !_LINUX
        if (use_pio) {
            /*
            int size = pioSize_(pio);
            printf("!OK, gpioeval, size %d\n",pioSize_(pio));
            for (int i = 0; i < size; i++) {
                printf("%d:%02x ",
                pio.pioInstrBuf[i].code,pio.pioInstrBuf[i].data);
            }
            printf("\n");
            */
            unsigned char dummy_output[1];
            gpioEval_(&pio,dummy_output,0);
            if (dbg_port) printInFile("[pio]send queued data\n");
        }
        else 
#endif
            if (buf == 0) return;   // we've already sent all.
            else {
                // Need trailing 0?
                buf[cnt].port = 0;
                cnt++;  // Be sure to send terminating 0 port.  O/wise driver loops!
                if (dbg_port) printInFile("[driver]send queued data\n");
                win32OutpArray(&buf[0], sizeof(*buf)*cnt);
            }
    }
    int last_tms_tdi;   // Optimize: remember last tms/tdi value.
    int cnt;    // Count of items in buf.
    GPIO_ARRAY_STRUCT *buf;
};

// Parallel port pin conversions:

#define TDI(val) (val ? TDI_1 : TDI_0)
#define TMS(val) (val ? TMS_1 : TMS_0)

inline unsigned read_PP() {
    if (using_xilinx) {
        int TCK1 = ((C_SS0 | C_CNT | C_SS1 | JTAG_TCK_1)^C_XOR);
        int TCK0 = ((C_SS0 | C_CNT | C_SS1 | JTAG_TCK_0)^C_XOR);
        // Strobe to latch on to data.
        port->outp(epp_addr + PP_CONTROL_REG_OFFSET,TCK1);
        port->outp(epp_addr + PP_CONTROL_REG_OFFSET,TCK0);
    }
    return (UBYTE)port->inp((unsigned short)(epp_addr + PP_STATUS_REG_OFFSET));
}
inline unsigned convert_PP_byte_to_TDO(unsigned char byte) {
    return (((byte ^S_XOR) & JTAG_TDO) >> 4) & 1;
}

static void transitions(
    const char *tms_values, int tdi_value, const char *final) {
    // Take a string of 1s and 0s (tms_values) and execute transitions
    // with them.
    GPIO_ARRAY_STRUCT outbuf[compute_entries(128)];
    JTAG_out out(use_extended_driver ? outbuf : 0);
    for (const char *p = tms_values; *p; p++)
        out.add(TMS(*p == '1') | tdi_value, *p == 0 ? final : "Unknown");
    out.send();
}

/*********************************************************
* Function: Select_IR_scan
*
* move to the Select_IR_scan JTAG state.
**********************************************************/
static void Select_IR_scan_(JTAG_out &out, char current_state) {
    /*
    There are two states you could be in at this point ,
    [Run-Test/idle], and Select_DR_Scan.
    Therefore to get to the select_IR_scan state requires :
    TMS = 1, TCK, TCK
    TMS = 1, TCK, Respectively.
    */

    tap_controller.check_state(current_state);
    if (dbg_port) printInFile("\nSelect_IR_scan ...");

    transitions(current_state == RUN_TEST_IDLE_STATE ? "11" : "1", TDI_0,
        "Select_IR_scan");
}

unsigned JTAG_out::saved = 0;

struct Real_JTAG_handler : JTAG_handler {
    override bool switch_madi(int cpunum) {
        this->cpunum = cpunum;
        discover_JTAG();
        which_JTAG_am_I();
        return total_arc_JTAGs > 1;
    }
    unsigned total_arc_JTAGs;
    override void Select_Load_Write(
        int start_state, int data_register_type, ULONG data, 
        int end_state) {
        // Does Select_IR_Scan, Load_IR and Write_DR all in one burst.
        JTAG_out out(use_extended_driver ? shared_buf : 0);
        0 && printf("%x:cpunum %d: drt %x data %x\n",this,cpunum,
            data_register_type,data);
        Select_IR_scan_(out,start_state);
        load_IR_(out,data_register_type, SELECT_DR_SCAN_STATE);
        write_DR_(out,data_register_type, data, end_state);
        out.send();
    }
    override void Select_IR_scan(char current_state) {
        JTAG_out out(use_extended_driver ? shared_buf : 0);
        Select_IR_scan_(out,current_state);
        out.send();
    }
    override void load_IR(char data_register_type, char final_state) {
        JTAG_out out(use_extended_driver ? shared_buf : 0);
        load_IR_(out,data_register_type,final_state);
        out.send();
    }
    void load_IR_(JTAG_out &out, char DATA_REGISTER_TYPE, char final_state);
    void write_DR_(
        JTAG_out &out, char DATA_REGISTER_TYPE, ULONG data, char final_state);
    override void write_DR(
        char data_register_type, ULONG data, char final_state) {
        JTAG_out out(use_extended_driver ? shared_buf : 0);
        write_DR_(out,data_register_type,data,final_state);
        out.send();
    }
    bool discover_jtag, trace_discover;
    unsigned cpunum;    // 1, 2, ...
    enum {GPIO_MAX = 1024};
    GPIO_ARRAY_STRUCT *shared_buf;
    void figure_out_tck() {
        if (using_xilinx) {
            // Data values for xilinx clock:
            TCK0 = TMS_0 = TDI_0 = 0;
            TCK1  = 1 << 1;
            TMS_1 = 1 << 2;
            TDI_1 = 1 << 0;
            /*
            // Avoid race condition; invert 0 and 1.  Requires an
            // extra write of 0 before I can read output.
            TCK1 = 0; TCK0 = 1 << 1;
            */
            // xilinx pinout
            // 2           D0             TDI
            // 3           D1             TCLK
            // 4           D2         TMS
            printf("TCK1 %d TCK0 %d MS_1 %d TDI_1 %d\n",TCK1,TCK0,TMS_1,TDI_1);
        }
        else {
            TCK1 = ((C_SS0 | C_CNT | C_SS1 | JTAG_TCK_1)^C_XOR);
            TCK0 = ((C_SS0 | C_CNT | C_SS1 | JTAG_TCK_0)^C_XOR);
            TMS_0 = JTAG_TMS_0;
            TMS_1 = JTAG_TMS_1;
            TDI_0 = JTAG_TDI_0;
            TDI_1 = JTAG_TDI_1;
            }
    }
    override int process_property(const char *key, const char *value) {
        if (strcmp(key,"discover_jtag") == 0) {
            // This has to be done by hand for now.
            // Unless you provide it, we won't deal with multiple jtags.
            discover_jtag = *value == '1';
            // We'll discover the jtag upon reset.
        }
        else if (strcmp(key,"trace_discover") == 0) 
            trace_discover = *value == '1';
        else if (strcmp(key,"xilinx") == 0) {
            using_xilinx = *value == '1';
            figure_out_tck();
        }
        else if (strcmp(key,"cpunum") == 0) {
            // cpunum 1, 2...
            cpunum = atoi(value);
            0 && printf("!ASSIGNED cpunum is %d\n",cpunum);
            0 && printInFile("\ncpunum %d\n",cpunum);
        }
        else if (strcmp(key,"prepare_for_new_program") == 0) {
            discover_JTAG();    // In case we haven't already.
            which_JTAG_am_I();
        }
        else if (strcmp(key,"use_pio") == 0) {
            allow_pio = *value == '1';
        }
        else return 0;
        return 1;
    }
    override void read_DR(
        char DATA_REGISTER_TYPE, ULONG *value, char end_state);
    override int until_not_busy(const char *operation);
    override void set_JTAG_to_idle() {
        if (dbg_port) printInFile("Set JTAG to idle...\n");
        // This goes to idle via test-logic-reset.
        transitions("0111110",TDI_0,"run test idle");
        if (dbg_port) printInFile("END Set JTAG to idle...\n");
    }
    override unsigned read_TDO() {
        // read TDO.
        // right shift port result by 4, so result is at bit 0 instead of bit 4.
        // Result is 1 iff TDO is 1.
        int ans = convert_PP_byte_to_TDO(read_PP());
        0 && printf("tdo is %x\n",ans);
        return ans;
    }

    override void send_JTAG(unsigned tms_and_tdi, const char *new_state);
    override void discover_JTAG();
    void which_JTAG_am_I();
    NOINLINE void send_JTAG_(
        unsigned tms_and_tdi, const char *new_state, int line) {
        if (dbg_port) printInFile("\nsend random stuff, line %d...",line);
        send_JTAG(tms_and_tdi,new_state);
    }
    #define send_JTAG_(a,b) send_JTAG_(a,b,__LINE__)
    Real_JTAG_handler() {
        jtags = 0;
        jtag_count = 0;
        discover_jtag = TRUE;
        using_xilinx = FALSE;
        trace_discover = FALSE;
        cpunum = 0;
        // Used by various folks.
        shared_buf = new GPIO_ARRAY_STRUCT[GPIO_MAX];
        total_arc_JTAGs = 0;
        figure_out_tck();
        errors_loud = FALSE; //TRUE;
        error_count = 0;
    }
    struct Which_JTAG_info {
        unsigned left_ir_bypass_to_my_arc, right_ir_bypass_to_my_arc;
        unsigned left_data_bypass_to_my_arc, right_data_bypass_to_my_arc;
        Which_JTAG_info() { zero_bypass_bits(); computed = FALSE; }
        void zero_bypass_bits() {
            left_ir_bypass_to_my_arc = 0;
            right_ir_bypass_to_my_arc = 0;
            left_data_bypass_to_my_arc = 0;
            right_data_bypass_to_my_arc = 0;
        }
        bool computed;
    };
    enum { MAX_ARCS = 256 };
    // JTAG info for each ARC.  Numbering is 1..max cpus.
    Which_JTAG_info which_JTAG_info[MAX_ARCS+1];
    override ~Real_JTAG_handler() { 
        if (jtags) { delete[] jtags; jtags = 0; }
        delete []shared_buf;
    }
    struct JTAG_info {
        // Here keep track of the each jtag unit in the chain.
        bool is_id;
        unsigned idreg;
        bool is_arc;
        unsigned irlen; // Length of instruction register.
        bool irlen_known;
        unsigned arcnum;    // Which arc is this?  1, 2, 3... 
        JTAG_info() { 
            is_id = is_arc = false; idreg = 0; irlen = 0; 
            irlen_known = FALSE;
            arcnum = 0;
        }
    };
    JTAG_info *jtags;
    int jtag_count;
    NOINLINE void print_jtags() {
        print->printf("There are %d jtag unit(s).  Left-to-right, they are:\n  ",
            jtag_count);
        for (int i = 0; i < jtag_count; i++) {
            JTAG_info &J = jtags[i];
            print->printf("%d:",i+1);
            if (J.is_id) print->printf("id=0x%08x",J.idreg);
            else print->printf("bypass");
            if (J.is_arc) print->printf("(ARC)");
            if (J.irlen_known) print->printf(",irlen=%d",J.irlen);
            print->printf("  ");
        }
        print->printf("\n");
    }
    unsigned read_bits(int howmany);
    Printf* print;
    override void receive_printf(Printf*p) {
        this->print = p;
        tap_controller.receive_printf(p);
    }
    char path[TAP_state_array_size];
    NOINLINE bool depth_first(int depth, int max, int q, int dest) {
        // Get from q to dest with at most max xitions.
        // But avoid reset, which goofs things up.
        if (q == q_test_logic_reset) return false;
        if (q == dest) return true;
        if (depth > max) return false;
        path[depth+1]=0;
        path[depth] = '0';
        work++;
        if (depth_first(depth+1,max, tap_controller.TAP[q].dest[0], dest))
            return true; 
        path[depth] = '1';
        if (depth_first(depth+1,max, tap_controller.TAP[q].dest[1], dest))
            return true; 
        return false;
    }
    NOINLINE bool breadth_first(int q, int dest) {
        // Dumb breadth first by doing depth first with a fixed depth!
        for (int i = 1; i <= 8; i++)
            if (depth_first(0,i,q,dest)) return true;
        return false;
    }
    int work;
    override void go_to(int state) {
        // Find shortest path to state from current state.
        // This doesn't take too long so we leave it this way rather
        // than having the client call us to precompute the path.
        work = 0;
        breadth_first(tap_controller.current_state(),state);
        // printf("!found path %s after %d\n",path,work);
        transitions(path,TDI_0,"state");
    }
    bool errors_loud;
    override void set_error_silence(bool silent) {
        errors_loud = !silent;
    }
    unsigned error_count;
    override unsigned get_error_count() { return error_count; }
};

JTAG_handler *JTAG_handler::make_JTAG_handler() { 
    return new Real_JTAG_handler(); 
}
JTAG_handler::~JTAG_handler() { }

static void add_ones(unsigned char *&p, int count) {
    for (signed i = 0; i < count; i++) *p++ = TDI(1);
}
/*********************************************************
* Function: Load_IR
*
* Shift the bits into the instruction register.
**********************************************************/
void Real_JTAG_handler::load_IR_(
    JTAG_out &out, char DATA_REGISTER_TYPE, char final_state) {
    // Select_IR_scan state => capture_IR => shift_IR.
    out.add(TMS_0 | TDI_0,"Capture_IR");
    out.add(TMS_0 | TDI_0,"Shift_IR");
    unsigned char max_bits[512], *p = max_bits;
    // We want to place some IR value abcd in the ARC IR.
    // We have to precede and follow it by the right number of bits
    // to drive the other JTAGs into bypass mode.  The bypass opcode
    // is all 1s.  So, e.g.:
    //      1111111 abcd 1111
    // The number of bits to the left and right was computed earlier.

    // Add bits to put jtags to my right in bypass mode.
    add_ones(p, which_JTAG_info[cpunum].right_ir_bypass_to_my_arc);

    // Load in the ARC's IR.  
    // data_register_type is the actual value of the IR opcode.
    unsigned dr = DATA_REGISTER_TYPE;
    *p++ = TDI(dr & 1); 
    *p++ = TDI(dr & 2); 
    *p++ = TDI(dr & 4);
    *p++ = TDI(dr & 8);

    // Add bits to put jtags to my left in bypass mode.
    add_ones(p, which_JTAG_info[cpunum].left_ir_bypass_to_my_arc);

    // Now we have all the IR bits to shift in all the IRs in the chain.
    for (unsigned char *q = max_bits; q < p; q++) {
        // TMS is 0 except for the last bit, where we'll go to Exit_IR.
    bool last = q == p-1;
    out.add(TMS(last) | *q, last ? "Exit_IR" : "Shift_IR");
    }

    // Move from the Exit1_IR state to Update_IR.
    out.add(TMS_1 | TDI_0,"Update_IR");

    // "state" parameter selects the final state of the function.
    // Possible final states = Select_DR_scan (OR) Run_test_idle.
    if (final_state == SELECT_DR_SCAN_STATE) {
    // Enter select_DR_scan state.
    out.add(TMS_1 | TDI_0,"Select_DR_scan");
    }
    else out.add(TMS_0 | TDI_0,"Run_test_idle");
    tap_controller.check_state(final_state);
    }

/*********************************************************
* Function: Write_DR
*
* Shift the data into the specified data register
* i.e. Address, data, transaction.
**********************************************************/
void Real_JTAG_handler::write_DR_(
    JTAG_out &out, char DATA_REGISTER_TYPE, ULONG data, char final_state) {

    // From the Select_DR_scan state move through the capture_DR 
    // to the shift_DR.
    out.add(TMS_0 | TDI_0,"Capture_DR");
    out.add(TMS_0 | TDI_0,"Shift_DR");

    // Prepare TDI values here; send them later.
    unsigned char bits[512], *p = bits;

    switch (DATA_REGISTER_TYPE) {

    case ADDRESS_REGISTER :
        // Store the address data to the record.
        //address_register = data;
        // FALLTHROUGH: data_register routine same as 
        // address_register routine(if there was one)
        // pre-loaded instruction_register(IR) will know 
        // where to write data to.
        // break;

    case DATA_REGISTER : {
        for (int i = 0; i < 32; i++) {
        // Prepare the 32 address bits serially.
        *p++ = TDI(data&1);
        data >>= 1;
        }
        } break;

    case TRANSACTION_REGISTER :
        // the variable "data" will contain the
        // four bit communication transaction code.
        // only four bits require writing, bit four is always 0.
        *p++ = TDI(data & 1);
        *p++ = TDI(data & 2);
        *p++ = TDI(data & 4);
        *p++ = TDI(data & 8);
        break;

    case STATUS_REGISTER :
         //This register should not be written to, (READ ONLY !)
        break;

    case ARC_NO_REGISTER :
        // Write the 3 bit arc number, not required yet.
        break;

    default :
        //error: unknown register specified to write to
        break;
    }

    if (p > bits) {
    // Now we have to shift enough bits in to account for bypasses
    // in the JTAGs to the left of me, so my data will arrive
    // where I want it.
    for (unsigned i = 0; 
        i < which_JTAG_info[cpunum].left_data_bypass_to_my_arc; i++) 
        *p++ = TDI(0);
    // Now send all the bits.
    for (unsigned char *q = bits; q < p; q++) {
        bool last = q == p-1;
        out.add(TMS(last) | *q, last ? "Exit_DR" : "Shift_DR");
        }
    }
    else out.add(TMS_1, "Exit_DR");

    // Enter update_DR state.  
    out.add(TMS_1 | TDI_0,"Update_DR");
    // "state" parameter selects the final state of the function
    // Possible states = Select_DR_scan (OR) Run_test_idle
    if (final_state == SELECT_DR_SCAN_STATE) 
    out.add(TMS_1 | TDI_0,"Select_DR_scan");
    else out.add(TMS_0 | TDI_0,"Run_test_idle");
    tap_controller.check_state(final_state);
    }

void JTAG_out::add(int tms_tdi, const char *comment) {
    // Compare the tdi/tms prior value.  If the same,
    // we don't need to write the new value.
    if (optimize_jtag && tms_tdi == last_tms_tdi && !using_xilinx) {
    saved++;
    if (FALSE && saved && saved % (4*4096) == 0)
        printf("Saved %d port writes.\n",saved);
    }
    else {
        if (use_pio) {
        tap_controller.set_tms(tms_tdi);
        if (!using_xilinx) {
        if (dbg_port) 
            printInFile("[pio]queue %x => port %x\n",
            tms_tdi,epp_addr);
        pioConst_(pio,tms_tdi); pioOutp_(pio,epp_addr-epp_addr);
        }
        else {
        if (dbg_port) 
            printInFile("[pio]queue %x, %x => port %x\n",
            tms_tdi|TCK1,tms_tdi|TCK0,epp_addr);
        pioConst_(pio,tms_tdi|TCK1); pioOutp_(pio,epp_addr-epp_addr);
        pioConst_(pio,tms_tdi|TCK0); pioOutp_(pio,epp_addr-epp_addr);
            }
        }
    else if (buf == 0) out_tms_tdi(tms_tdi);
    else {
        tap_controller.set_tms(tms_tdi);
        // Change tms/tdi values.
        if (!using_xilinx) {
        if (dbg_port) 
            printInFile("[driver]queue %x => port %x\n",
            tms_tdi,epp_addr);
        buf[cnt  ].port = epp_addr;
        buf[cnt++].data = tms_tdi;
        }
        else {
        if (dbg_port) 
            printInFile("[driver]queue %x, %x, %x => port %x\n",
            tms_tdi|TCK0,tms_tdi|TCK1,tms_tdi|TCK0,epp_addr);
        buf[cnt  ].port = epp_addr;
        buf[cnt++].data = tms_tdi|TCK0;
        buf[cnt  ].port = epp_addr;
        buf[cnt++].data = tms_tdi|TCK1;
        buf[cnt  ].port = epp_addr;
        buf[cnt++].data = tms_tdi|TCK0;
            }
        }
    last_tms_tdi = tms_tdi;
    }
    if (use_pio) {
    if (!using_xilinx) {
        pioConst_(pio,TCK1); pioOutp_(pio,epp_control_reg_addr-epp_addr);
        pioConst_(pio,TCK0); pioOutp_(pio,epp_control_reg_addr-epp_addr);
        tap_controller.take_transition();
        }
    }
    else if (buf) {
        if (!using_xilinx) {
        // TCK 1, then TCK 0.
        buf[cnt  ].port = epp_control_reg_addr;
        buf[cnt++].data = TCK1;
        buf[cnt  ].port = epp_control_reg_addr;
        buf[cnt++].data = TCK0;
        tap_controller.take_transition();
        }
    }
    else tick_the_clock(comment);
    }

void Real_JTAG_handler::send_JTAG(unsigned tms_and_tdi, const char *new_state) {
    GPIO_ARRAY_STRUCT outbuf[compute_entries(1)];
    JTAG_out out(use_extended_driver ? outbuf : 0);
    out.add(tms_and_tdi,new_state);
    out.send();
    }

unsigned int_from_bits(unsigned char *bits, int howmany) {
    unsigned ans = 0, bit = 1;
    for (int i = 0; i < howmany; i++) {
    if (*bits++) ans |= bit;
    bit <<= 1;
    }
    return ans;
    }

static const int ARC_IR_LEN = 4;    // instr reg len for ARC.
static const int MAX_IR_LEN = 32;   // max instr reg len for non-ARC.

void Real_JTAG_handler::discover_JTAG() {
    if (!discover_jtag) return;
    if (jtags) return;  // Already done.
    if (dbg_port) printInFile("\ndiscover jtag (I am CPU %d)\n",cpunum);
    // Called only after reset board.  Each JTAG must have either
    // bypass or idcode in their IRs.
    // Idle all the JTAGs.
    set_JTAG_to_idle();
    // Capture all the bypass or ident registers.  First move
    // through capture-DR to get them and go to shift-DR
    // so we can read the results out.
    // According to the JTAG spec:
    // o a bypass register must be 1 bit and when captured always captures 0
    // o the ident register, if present, must be 32 bits.
    // o the bottom bit of the ident register is always a 1
    // o the next 7 bits, the vendor code, can never be 7f (illegal code).
    // You can use these facts to determine the number of jtag units
    // present and to determine which have ident regs and which not.
    // We shift in a bad id (0xff) and wait to see it come out.
    // What precedes it will be the ident and bypass registers.
    // Capture the registers (bypass or idcode).
    transitions("100",0,"shift_DR");
    // Now we in the mode to capture the result.
    // Shift in the 32-bit value 0000_00ff.  This is an invalid pattern
    // for the idcode register.  If it comes out untouched we know all
    // the jtags are in bypass mode.
    unsigned bad_id = 0xff;
    unsigned shift_in = bad_id, shift_out = 0;
    // Read out a maximum of bits before we give up.
    #define DAVE 0
    #if !DAVE
    static const int MAX = 1024;
    #else
    static const int MAX = 128;
    #endif
    unsigned cnt = MAX;
    unsigned char results[MAX], *pb = results;
    while (shift_out != bad_id && cnt > 0) {
    // See what's on the other side.
    // I'm in shift DR state.  Read bottom bit.
    unsigned result = read_TDO();
    // printf("get %d\n",result);
    cnt--;
    *pb++ = result;
    shift_out >>= 1; shift_out |= (result ? 0x80000000 : 0);
    // Push in my bit.
        send_JTAG(TMS(0) | TDI(shift_in&1), "shift_DR");
    shift_in >>= 1;
    // printf("shift out is %x\n",shift_out);
        }
    #if DAVE
    if (trace_discover) {
    print->printf("JTAG chain analysis: input 0x%x, "
        "output bit stream was:\n\t",bad_id);
    for (unsigned char *p = results; p < pb; p++) {
        if (p - results && ((p-results) %4 == 0)) print->printf(" ");
        print->printf("%c",*p ? '1' : '0');
        }
    print->printf("\n");
    fflush(stdout);
    }
    #endif
    if (cnt == 0) {
        // MAX bits is too many.  Something's wrong.  Perhaps
    // the ARC is not blasted!
        print->printf("Automatic JTAG discovery: "
        "TAP controller(s) seem to be inoperative.\n");
    if (dbg_port) printInFile(
        "Automatic JTAG discovery: "
        "TAP controller(s) seem to be inoperative.\n");
    discover_jtag=FALSE;
    set_JTAG_to_idle();
    return;
    }
    if (trace_discover) {
    print->printf("JTAG chain analysis: input 0x%x, "
        "output bit stream was:\n\t",bad_id);
    for (unsigned char *p = results; p < pb; p++) {
        if (p - results && ((p-results) %4 == 0)) print->printf(" ");
        print->printf("%c",*p ? '1' : '0');
        }
    print->printf("\n");
    fflush(stdout);
    }
    // Analyze the results.
    static const int MAX_JTAGS = 256;
    {
    /* Recent ARC ID register changes.
    The IDCODE register will appear as JTAG register 0xC, and will be
    selected upon initialization or upon running the TAP controller
    through Test-Logic-Reset.  It consists of the following fields, from
    most significant through least significant:

    IDCODE[31 : 28] JTAG_VERSION    = 0001 // Version of the JTAG circuitry
    IDCODE[27 : 18] ARCNUM      = // ARC number within design
        (varies, 10 bits of zero in the version you'll see) 
    IDCODE[17 : 12] ARC_TYPE    = 00_0010       
        // Major core type, 2 is ARC 600, although you're getting A5s
    IDCODE[11 : 1]  ARC_JEDEC_CODE  = 0100_101_1000 
        // Code assigned to ARC International by JEDEC, 
        // encoded as per IEEE 1149.1-2001; 0x58 in group 5 (don't ask)
    IDCODE[0]           = 1 // To distinguish from bypass reg

    This code is loaded into the shift register in Capture-DR, when the
    instruction register points to IDCODE. Of those fields, the
    least-significant two are fixed (the JEDEC code and the 1 in the
    LSB). I made up the rest, and you're welcome to suggest definitions
    or sizes you'd prefer.
    */

    static const unsigned ARC_IDENT = (0x258 << 1) | 1,
        ARC_IDENT_MASK = (0x3ff << 1) | 1;
    // As of 3/25/04 the ARC can supply an identity register.
    JTAG_info local_jtags[MAX_JTAGS], jtags_temp[MAX_JTAGS];
    int jtag_ix = 0;
    for (unsigned char *p = results; p < pb; p++) {
        JTAG_info &J = jtags_temp[jtag_ix];
        if (*p == 0) {
        J.is_id = false;
      IS_ARC: ;
        J.is_arc = true;
        J.irlen = ARC_IR_LEN;
        J.irlen_known = TRUE;
        // Assume all bypass JTAGs are ARCs and all others aren't.
        // Later we can assume otherwise.
        jtag_ix++;
        }
    else {
        // Non-bypass.  Consume the bits.
        J.is_id = true;
        unsigned id = int_from_bits(p,32);
        p += (32-1);    // p++ does the 1.
        J.idreg = id;
        // printf("id is %x\n",id);
        if (id == bad_id) ;
        else {
        if ((id & ARC_IDENT_MASK) == ARC_IDENT) goto IS_ARC;
        jtag_ix++;
        if (trace_discover)
            print->printf("JTAG %d: non-bypass identity register "
                "= %x\n",id);
        }
        J.is_arc = false;
        }
        }
    // They are right-to-left.  Swap them.
    int i;
    for (i = 0; i < jtag_ix; i++) local_jtags[i] = jtags_temp[jtag_ix-1-i];
    this->jtags = new JTAG_info[jtag_count = jtag_ix];
    memcpy(this->jtags,local_jtags,sizeof(*jtags)*jtag_count);
    // Now that the jtags are in the right order, number the ARCs accordingly.
    int arc = 0;
    for (    i = 0; i < jtag_count; i++) {
        JTAG_info &J = jtags[i];
    if (J.is_arc) J.arcnum = ++arc;
        }
    }
    if (trace_discover) print_jtags();
    // We can discover the irlen of the sum of non-ARCs if the jtag
    // chain is one of the two forms:
    //  non-arc* arc*
    //  arc* non-arc*
    // If neither we give up.
    // We need to know the length of the IR reg for non-ARCs so we 
    // can set IR to bypass.
    // The problem with a chain such as
    //  non-arc* arc* non-arc*
    // is that if the total IR len is 80 and the ARCs account for 20,
    // how much is to the left or right of the arc*?  We can't know.
    int non_arcs = 0, arcs = 0, left_non_arcs = 0, right_non_arcs = 0;
    for (int i = 0; i < jtag_count; i++) {
        JTAG_info &J = jtags[i];
        if (J.is_arc) arcs++; 
    else {
        non_arcs++;
        if (arcs) right_non_arcs++;
        else left_non_arcs++;
        }
    }
    total_arc_JTAGs = arcs;
    if (non_arcs) {
        if (gverbose) 
        print->printf(
            "discover_JTAG: There are %d ARCs and %d non-ARC JTAGss.\n",
        arcs,non_arcs);
        if (left_non_arcs && right_non_arcs) {
        printf("There are non-ARCs scattered in the chain amongst ARCs.\n");
        printf("I cannot figure out how to set them in bypass mode.\n");
        printf("Communication cannot work.\n");
        printf("Use option [to be defined] to tell me about them.\n");
        exit(1);
        }
    // Figure out the irlen by shifting 1 and seeing when it comes out.
    // The ARCS have 4.
    set_JTAG_to_idle();
    transitions("1100",0,"shift_IR");
    // First fill all IRs with 0.
    // Then shift in a 1 and see when it comes out.
    int arc_irbits = ARC_IR_LEN*arcs;
    int max_ir_len_sum = MAX_IR_LEN*non_arcs + arc_irbits;
    out_tms_tdi(TMS(0) | TDI(0));
    tick_the_clock("shift_IR",max_ir_len_sum);
    // OK, now everyone has 0s in the IR.  Find out how long by
    // putting in a 1 and seeing when it comes out.
    out_tms_tdi(TMS(0) | TDI(1));   // Set 1 input.
    int shifts = 0;
        if (gverbose) 
        print->printf("Discovering length of IRs for non-ARCs...\n");
    for (unsigned shift_out = 0; ;) {
        // TDO is the LSB bit of the rightmost entity's IR.
        shift_out = read_TDO();
        // printf("shifts=%d MSB of shift register is %d\n",shifts,shift_out);
        if (shift_out == 1) break;  // Done.
        shifts++;
        0 && printf("get %d\n",shift_out);
        // Push in my bit, get another.
        // printf("Shifting again\n");
        tick_the_clock("shift_IR");
        }
    int irbits = shifts;
    trace_discover && printf("%d bits is total IR len of all jtags.\n",irbits);
    // Now put the bits in just one of the others.  It doesn't
    // matter which.
    JTAG_info &which = left_non_arcs 
        ? this->jtags[0] : this->jtags[jtag_count-1];
    which.irlen = irbits - arc_irbits;
    which.irlen_known = TRUE;
        if (gverbose) print_jtags();
        }
    set_JTAG_to_idle();
    which_JTAG_am_I();
    }

void Real_JTAG_handler::which_JTAG_am_I() {
    // Based on my cpunum, figure out 
    // (a) data bypass bits to the right and left of me.
    // (b) IR bypass bits to the right and left of me.
    if (cpunum == 0) cpunum = 1;
    if (jtags == 0) {
        // OK, there are no multiple jtags, so just use slot 1, 
    // where the bits are initialized to 0 already.
    return;
    }
    if (0 && trace_discover) {
        printf("Which JTAG am I?  I am ARC CPU %d.\n",cpunum);
    print_jtags();
    }
    Which_JTAG_info &which = which_JTAG_info[cpunum];
    if (which.computed) return;
    which.zero_bypass_bits();
    which.computed = TRUE; 
    if (total_arc_JTAGs == 1 && cpunum > 1) {
        // Either this is single ARC with MADI, or single non-MADI
    // ARC with a mistake.  In any case we don't have an automated
    // mechanism to detect which.
    int t = cpunum; cpunum = 1; which_JTAG_am_I();
    cpunum = t; which_JTAG_info[cpunum] = which_JTAG_info[1];
    return;
        }
    bool found = FALSE;
    for (int i = 0; i < jtag_count; i++) {
        JTAG_info &J = jtags[i];
    // printf("looking at unit %d, arcnum %d\n",i,J.arcnum);
    if (J.arcnum == cpunum) found = TRUE;
    else if (!found) {
        which.left_data_bypass_to_my_arc++; // bypass reg bits.
        which.left_ir_bypass_to_my_arc += J.irlen;
        }
    else if (found) {
        which.right_data_bypass_to_my_arc++;    // bypass reg bits.
        which.right_ir_bypass_to_my_arc += J.irlen;
        }
    }
    if (trace_discover) {
    if (which.left_ir_bypass_to_my_arc) {
        print->printf("ARC #%d: to my left: %d IR bits, %d data bits.\n",
            cpunum,which.left_ir_bypass_to_my_arc, 
            which.left_data_bypass_to_my_arc);
        }
    if (which.right_ir_bypass_to_my_arc) {
        print->printf("ARC #%d: to my right: %d IR bits, %d data bits.\n",
            cpunum,which.right_ir_bypass_to_my_arc, 
            which.right_data_bypass_to_my_arc);
        }
    }
    }

unsigned Real_JTAG_handler::read_bits(int howmany) {
    if (howmany == 0) return 0;
    // We are xitioning to Shift_DR state; read a word of N bits out.
    unsigned ans = 0;
    unsigned bit = 1;
    bool done = FALSE;
    #define USE_PIO 1
    #if USE_PIO && !_LINUX
    if (slow_clock == 0) {
    // Batch up the reads in the driver.  Static buffer supports 4096 items.
    pioInitInstr();
    // Max # of bits I can read.
    unsigned char buffer[32];
    // Set TMS and TDI for the duration.  These don't change.
    pioConst(TMS_0 | TDI_0); pioOutp(0);
    int i;
    for (i = 0; i < howmany; i++) {
        // Clock 1, then 0.
        pioConst(TCK1); pioOutp(epp_control_reg_addr-epp_addr);
        pioConst(TCK0); pioOutp(epp_control_reg_addr-epp_addr);
        pioInp(PP_STATUS_REG_OFFSET);   // Read byte from port.
        }
    int bufsize = sizeof(buffer);
    gpioEval(buffer,&bufsize);
    // printf("bufsize back is %d\n",bufsize);
    // Now convert the buffer values to bits.
    for (    i = 0; i < howmany; i++, bit <<= 1) {
        if (convert_PP_byte_to_TDO(buffer[i])) ans = ans | bit;
        }
    done = TRUE;
    }
    #endif
    if (!done) {
    for (int i = 0; i < howmany; i++, bit <<= 1) {
        // Shift data out of jtag
        send_JTAG(TMS_0 | TDI_0,"Shift_DR");
        // Read result.
        if (read_TDO()) ans = ans | bit;
        }
    }
    return ans;
    }

/*********************************************************
* Function: Read_DR
*
* Shift data out of the specified data register to the pointer.
**********************************************************/
void Real_JTAG_handler::read_DR(
    char DATA_REGISTER_TYPE, ULONG *value, char final_state) {

    send_JTAG_(TMS_0 | TDI_0,"Capture_DR");

    // First read out junk from the ARCs to my right.

    switch (DATA_REGISTER_TYPE) {
        case ADDRESS_REGISTER:
        case TRANSACTION_REGISTER :
            //This register should be written to, not read from !
            break;

        case DATA_REGISTER: {    // Read out a 32 bit data value.
            if (dbg_port) printInFile("\nread_DR(DATA_REGISTER) ...");
        unsigned junk = 
        read_bits(which_JTAG_info[cpunum].right_data_bypass_to_my_arc);
        *value = read_bits(32);
            } break;

        case STATUS_REGISTER: {
            // Read out all four bits from the status register
            //and store the OR result of 4 bits, in "value".
            if (dbg_port) printInFile("\nread_DR(STATUS) ...");
        unsigned junk = 
        read_bits(which_JTAG_info[cpunum].right_data_bypass_to_my_arc);
            *value = read_bits(4);
            } break;

        case ARC_NO_REGISTER:
            // Read out the 3 bit arc number, not required yet.
            break;

        default :
            //unknown register to read
            if (dbg_port)
                printInFile("\n[ERROR] read_DR(?unknown register?) ...");
            break;
    }

    // Return to the Select_DR_Scan State.
    send_JTAG_(TMS_1 | TDI_0,"Exit_DR");
    send_JTAG_(TMS_1 | TDI_0,"Update_DR");

    //"state" parameter selects the final state of the function
    //Possible states = Select_DR_scan (OR) Run_test_idle
    if (final_state == SELECT_DR_SCAN_STATE) {
        send_JTAG_(TMS_1 | TDI_0,"Select_DR_scan");
        // Select_DR_scan.
    }
    else
        send_JTAG_(TMS_0 | TDI_0,"Run_test_idle");
    tap_controller.check_state(final_state);
    }

/*********************************************************
* Function: Until_not_busy
*
* Keep reading the Status register until not busy + not transaction
* error.  Return 1 if JTAG is ready for a new transaction.
**********************************************************/
int Real_JTAG_handler::until_not_busy(const char *operation) {

    //Control variables
    int retry = TRUE;
    int jtag_failed = FALSE;

    //JTAG status variables
    unsigned long status_val;
    unsigned char READY, PC, STALLED, FAILED;

    //Debug message
    if (dbg_port) printInFile("\nuntil_not_busy(%s) ...",operation);

    if (jtag_reset_always) {
       // ResetState();  // TBH  13 June 02 reset state every access prop reset_always (=0 turns off)
        set_JTAG_to_idle();
    }

    // Select access to the status register.  
    Select_IR_scan(RUN_TEST_IDLE_STATE);
    // Puts rest of JTAGs in bypass.
    load_IR(STATUS_REGISTER, SELECT_DR_SCAN_STATE);

    unsigned loop_count = 0;

    //Loop until ARC is ready or until we give up waiting
    unsigned attempts = MAX_RETRY+jtag_retry_base_number;
    while (retry &&
        (jtag_retry_base_number == 0 /* no timeout -- keep retrying */
        || loop_count <= attempts)) {

        //Reset JTAG failure state
        jtag_failed = FALSE;

        if (dbg_port) printInFile("\nCheck the status bits");
#if 0
        static int sleeper = 0;
        sleeper++;
        if (sleeper % 32 == 0) printf("!sleeping\n"); 
        os->sleep(10);
#endif

        //read status register
        read_DR(STATUS_REGISTER, &status_val, SELECT_DR_SCAN_STATE);
        // printf("!status register is %x\n",status_val);

        //Get the values of the status register fields
        STALLED = (unsigned char)(status_val & 0x1);
        FAILED = (unsigned char)((status_val & 0x2) >> 1);
        READY = (unsigned char)((status_val & 0x4) >> 2);
        PC = (unsigned char)((status_val & 0x8) >> 3);

        // All is well if it's not failed, not stalled and is ready
        if (!FAILED & !STALLED & READY)
            retry = FALSE;
        else {
            //Debug and user messages required
            if (FAILED) {
                if (dbg_port) printInFile("\nJTAG Failed");

                jtag_failed = TRUE;

                // printf("\nJTAG FAILURE !!!");
            }
            if (STALLED) if (dbg_port) printInFile("\nJTAG stalled");
            if (!READY) if (dbg_port) printInFile("\nJTAG not ready");
        }

        //Do we need to retry?
        loop_count++;

        // TBH 7 Jun 2002 reset only after 3 fails
        if ((FAILED && loop_count > 3) ||
            ((STALLED || !READY) && loop_count >= MAX_RETRY-1)) {
            // TBH much better to try a reset to run_test_idle than fail
            set_JTAG_to_idle();

            // Select access to the status register
            Select_IR_scan(RUN_TEST_IDLE_STATE);          
            load_IR(STATUS_REGISTER, SELECT_DR_SCAN_STATE);
        }
    }

    //Retrying as we left...so we have an error
    if (retry) {
        // Retried enough.....its an error
		// CJ--
        //if (errors_loud) { 
        //    print->printf(jtag_failed
        //    ? "JTAG failed after %d attempts; attempted operation was %s.\n"
        //    : "JTAG remains stalled after %d attempts; attempted operation was %s.\n"
        //    , attempts, operation);
        //    print->printf("  (JTAG status register is %x.)\n",status_val);
        //}
        error_count++;
        set_JTAG_to_idle();
        return ERR_BUSY;
    }

    // Now in the Select_DR_Scan State.
    return 1;
}


/*
Multiple JTAGs, multiple DLLs, multiple ARCs.

The A6 and subsequent chips chain the JTAGs together: TDI of one is wired
to TDO of the next.

Assume, e.g., two ARCs.  We would have

    TDI -> jtag1 -> TDO = TDI -> jtag2 -> TDO
        ARC1              ARC2

Thus when you want to talk to ARC2, you have to put ARC1 in bypass
mode, and vice-versa.  In bypass mode TDI and TDO are connected to a
single-bit shift reigster when you are shift-DR state.
Thus you have to deal with the consequences of the extra bit for all the
ARCs that are being ignored.

For example, suppose you want to write memory at A with value V in ARC2.
For ARC2, this involves:
o setting IR := address_register
o putting in the address bits
o setting IR := data register
o putting in the data bits
o setting IR := transaction register
o putting in the code for memory write
o moving to state run-test-idle, which commits the transaction.

Changing the JTAG state unfortunately changes ALL the jtag states.
Thus when you load the IR, you load it for ALL jtags.  As the IR is
4 bits wide you must always shift in 4*N bits of IR where there are N
ARCs.  All but one of the ARCs must be set in bypass mode.

So, to communicate with ARC2 one loads the two IRs with:

    ARC1    ARC2
    1111    1010 (addr)
       (send address)
    1111    1011 (data)
       (send data)
    1111    1001 (transaction)

This takes twice the bits!

To send the address, you need to send

    ARC1    ARC2
    0       addr bits

Similar for data.

On the reading side, it's similar, although for ARC2 you can skip
the extra data bit shift for ARC1, as you don't need the result.

For loading the IR:
o if there are N arcs to the "left" of you, end with N*1111 extra bits
o if there are N arcs to the "right" of you, begin with N*1111 bits

For loading the DRs:
o if there are N arcs to the "left" of you, end with N*1 extra bits
o if there are N arcs to the "right" of you, you can ignore them,
  as you don't care about their output.

For reading the DRs:
o if there are N arcs to the "left" of you, you can ignore them.
o if there are N arcs to the "right" of you,
  shift out an extra N*1 bypass register bits.

You can figure out whether you have chained jtag by:
o right after chip reset, all jtags are in bypass mode, and the value
  of the bypass register is 0.
o shift a 1 bit through the data register and see how long it takes
  to appear.  If it appears after N+1 clocks, you have N chainged jtags.
  I.e.:
    set TDI 1; clock 1/0
        -- now TDO contains the previous contents of the
           bypass register, and the bypass register contains 1
    clock 1/0
        -- If a single ARC, TDO is now 1.


Q:
o how do I determine # of ARCs?  Only by reading jtag?
  How will I do it for || port access, then?
  Does the MADI build register still exist, but other MADIs not?
o What is the ident register so I can detect whether this is a6?

 */
