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
*               Copyright 2000-2004 ARC International(UK) Ltd
*                          All Rights Reserved
*
* File name:    Jtagport.h       
* 
* Author   :    Stephane Bihan
*
* Date     :    19/02/99
*
* Description : Jtag port control layer header file for 95/NT DLL
*
*******************************************************************************/

/* Parallel port control register for jtag. */
#define S_XOR       0x80    /* XOR value with this to get all bits postive */
                            /* with respect to the signal values. */
#define C_STR       0x01    /* Control register */
#define C_XOR       0x0b
#define C_CNT       0x02	// Not used for JTAG data.

// SS0 and SS1 are steering logic for AA2 and higher.
// Steers signals to xilinx jtag or ARC jtag.
#define C_SS0       0x04	
#define C_SS1       0x08	


/* The JTAG instruction register codes */
#define STATUS_REGISTER      0x8 /* 1000 */
#define TRANSACTION_REGISTER 0x9 /* 1001 */
#define ADDRESS_REGISTER     0xA /* 1010 */
#define DATA_REGISTER        0xB /* 1011 */ 
#define ARC_NO_REGISTER      0xC /* 1100 */
#define BYPASS_REGISTER      0xF /* 1111 */  // TBH 6 Jun 02

extern int jtag_reset_always; // TBH 13 Jun 02
extern int jtag_retry_base_number; // TBH 27 Jun 02
extern int optimize_jtag; 

/* PP Jtag signal definitions */
#define JTAG_TMS_1  0x40    /* Jtag Test Mode Select */
#define JTAG_TMS_0  0x00    

#define JTAG_TDI_1  0x80    /*Jtag input */
#define JTAG_TDI_0  0x00

/* Parallel port registers */
#define PP_STATUS_REG_OFFSET  1 /* status register */
#define PP_CONTROL_REG_OFFSET 2

/* state definitions */
#define RUN_TEST_IDLE_STATE     0x1
#define SELECT_DR_SCAN_STATE    0x02

struct JTAG_handler {
    // Thou shalt not access the port directly if thou wishes to access JTAG.
    // All port commands must be implemented in jtagport.cpp.
    virtual void Select_IR_scan(char state) = 0;
    virtual void load_IR(char DATA_REGISTER_TYPE, char STATE) = 0;
    virtual void read_DR(char DATA_REGISTER_TYPE, ULONG *value, char STATE) = 0;
    virtual void write_DR(char DATA_REGISTER_TYPE, ULONG data, char STATE) = 0;
    virtual int until_not_busy(const char *operation) = 0;
    virtual void Select_Load_Write(
	int start_state, int DATA_REGISTER_TYPE, ULONG data, int end_state) = 0;
    virtual void set_JTAG_to_idle() = 0;
    virtual unsigned read_TDO() = 0;
    virtual void send_JTAG(unsigned tms_and_tdi, const char *new_state) = 0;
    virtual void discover_JTAG() = 0;
    virtual int process_property(const char *key, const char *value) = 0;
    // Returns true if JTAG handles the ARC switching itself (i.e.,
    // no need to write MADI).
    virtual bool switch_madi(int cpunum) = 0;	
    virtual ~JTAG_handler() = 0;
    static JTAG_handler *make_JTAG_handler();
    virtual void receive_printf(Printf*p) = 0;
    virtual void go_to(int state) = 0;
    virtual void set_error_silence(bool silent) = 0;
    virtual unsigned get_error_count() = 0;
    };

