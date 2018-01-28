
// Author: justin wilde.
// Tom Pennello changed paragraphing to MetaWare coding standards; also
// fixed spelling of "impedance".

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "fpga.h"
#include "gportio.h"
#include "globals.h"

const short PPORT_DATA_OFFSET = 0;
const short PPORT_STAT_OFFSET = 1;
const short PPORT_CTRL_OFFSET = 2;
	
int cpldConfigData ;    // the bit pattern to be sent to the cpld
  

const int CLOCK_SOURCE_HIGH_IMPEDANCE = 0;
const int CLOCK_SOURCE_PLL_MCLK = 1;
const int CLOCK_SOURCE_PLL_VCLK = 2;
const int CLOCK_SOURCE_CRYSTAL = 3;
const int CLOCK_SOURCE_PLL_MCLK_HARVARD = 4;
const int CLOCK_SOURCE_PLL_VCLK_HARVARD = 5;
const int CLOCK_SOURCE_HOST_STROBE = 6;
const int CLOCK_SOURCE_CRYSTAL_DIVIDED = 7;
const int CLOCK_SOURCE_COUNT = 8;
	
const char* CLOCK_SOURCE_STRINGS[] = {
    "High Impedance",
    "PLL MCLK",
    "PLL VCLK",
    "Crystal",
    "High Impedance",
    "High Impedance",
    "Host Strobe",
    "Crystal With Division"
    };
	
const char * GCLOCK3_SOURCE_STRINGS[] = {
    "High Impedance",
    "PLL MCLK",
    "PLL VCLK",
    "Crystal",
    "PLL MCLK  (+ Harvard)",
    "PLL VCLK  (+ Harvard)",
    "Host Strobe",
    "Crystal With Division  (+ Harvard)"
    };
	
const int DefaultGClkSources[] = {
    CLOCK_SOURCE_HIGH_IMPEDANCE,
    CLOCK_SOURCE_CRYSTAL,
    CLOCK_SOURCE_HOST_STROBE,
    CLOCK_SOURCE_CRYSTAL_DIVIDED
    };

#if _MSC_VER
#include <windows.h>
int configureCPLD(short iPPortBase, int iGClkSource0, int iGClkSource1,
	    int iGClkSource2, int iGClkSource3) {
    const int CPLD_CLK_BIT  = 0x01 ;
    const int CPLD_DATA_BIT = 0x02 ;
    const int CPLD_SET_BIT  = 0x04 ;
	
    const int NUM_OF_CPLD_CFG_BITS = 16 ;

    const int C_XOR = 11 ;    
    const int C_STR = 1 ;
    const int C_CNT = 2 ;
    const int C_SS0 = 4 ;
    const int C_SS1 = 8 ;
    const int C_BI  = 32 ;
    int iRetVal = 0;
	
    // initialise the gpio port driver
    // NOTE : we may need to remove this gpioInit/Map call if it causes the
    //        control port to be modified - as this might cause ss0 & ss1 to
    //        go low, trashing the fpga config. Not sure what the init & Map
    //        functions actually do.
	  
    gpioInit();
    gpioMapPort(iPPortBase,4) ;
		
    // snapshot the control port: 
    int iOriginalState = win32Inp((short)(iPPortBase+PPORT_CTRL_OFFSET));
				
    // set the parallel port data to 0, in preparation for sending config data
    win32Outp((short)(iPPortBase+PPORT_DATA_OFFSET), 0) ;
    os->sleep(1);
		
    // Set CPLD into config mode:
		
    // set SS0=1, SS1=1, CNT=1, BIDir=0
    int iControlState = (iOriginalState | C_SS0 | C_SS1 | C_CNT | C_BI) ^ C_BI;
    win32Outp((short)(iPPortBase+PPORT_CTRL_OFFSET), iControlState ^ C_XOR);
    os->sleep(2);
		
    // Ensure STROBE is high
    iControlState = iControlState | C_STR;
    win32Outp((short)(iPPortBase+PPORT_CTRL_OFFSET), iControlState ^ C_XOR);
    os->sleep(2);
		
    // set CPLD into config mode by setting SS0=1, SS1=0, CNT=1
    iControlState = iControlState ^ C_SS1;
    win32Outp((short)(iPPortBase+PPORT_CTRL_OFFSET), iControlState ^ C_XOR);
    os->sleep(2);
		
    // now send the config data stream with set low
    // set clock high and data low
    int temp ;
		
    cpldConfigData = iGClkSource0;
    cpldConfigData += iGClkSource1 << 3;
    cpldConfigData += iGClkSource2 << 6;
    cpldConfigData += iGClkSource3 << 9;
		
    int cfg_data = cpldConfigData ;
    for (int index = 0; index < NUM_OF_CPLD_CFG_BITS; index++) {
	// see if the next cfg bit is 0 or 1
	if ((cfg_data & 0x1) == 0x1)
	    temp = CPLD_DATA_BIT ;
	else
	    temp = 0 ;
			
	// put data bit out to parallel port
	win32Outp((short)(iPPortBase+PPORT_DATA_OFFSET), temp) ;
	//		os->sleep(1);
			
	// and toggle the clock line
	temp |= CPLD_CLK_BIT ;
	win32Outp((short)(iPPortBase+PPORT_DATA_OFFSET), temp) ;
	//		os->sleep(1);
			
	temp &= ~CPLD_CLK_BIT ;
	win32Outp((short)(iPPortBase+PPORT_DATA_OFFSET), temp) ;
	//		os->sleep(1);
			
	cfg_data >>= 1 ;  
	}
		
    // now take the clock and set bits high
    temp = CPLD_CLK_BIT | CPLD_SET_BIT ;
    win32Outp((short)(iPPortBase+PPORT_DATA_OFFSET), temp) ;
    //	os->sleep(1);
		
    // finally take clock low
    temp = CPLD_SET_BIT ;
    win32Outp((short)(iPPortBase+PPORT_DATA_OFFSET), temp) ;
    //	os->sleep(1);
		
    // and put the cpld into arc run mode, ss0=1, ss1= 1, cnt=x
    iControlState = iControlState | C_SS1 | C_BI;//) ^ C_CNT;
    win32Outp((short)(iPPortBase+PPORT_CTRL_OFFSET), iControlState ^ C_XOR);    
    os->sleep(2);
		
    return iRetVal;  
    }

const unsigned short CONTROL_REG_OFFSET = 2;
const unsigned int C_XOR = 0x0b; // XOR value with this to get all bits postive with respect to the signal values.
const unsigned int C_STR = 0x01; // Strobe pin
const unsigned int C_CNT = 0x02;
const unsigned int C_SS0 = 0x04;
const unsigned int C_SS1 = 0x08;
const unsigned int C_BI = 0xA0; // Direction pin

const unsigned short STATUS_REG_OFFSET = 1;
const unsigned int S_XOR = 0x80;  // XOR value with this to get all bits postive with respect to the signal values.
const unsigned int S_SEL = 0x10;  // Select pin   
const unsigned int S_OP = 0x20;  // PaperEnd pin
const unsigned int S_ACK = 0x40;  // Ack pin
const unsigned int S_BUSY = 0x80;  // Busy pin


void readStatusLines(short iPPortBase, 
	bool *pbBusy, bool *pbACK, bool *pbOP, bool *pbSEL) {
    int status = win32Inp(iPPortBase+STATUS_REG_OFFSET) ^ S_XOR;
    if (NULL != pbBusy)  *pbBusy = (0 != (status & S_BUSY));
    if (NULL != pbACK)  *pbACK = (0 != (status & S_ACK));
    if (NULL != pbOP)  *pbOP = (0 != (status & S_OP));
    if (NULL != pbSEL)  *pbSEL = (0 != (status & S_SEL));
    }

int tester() { return 5; }

int isFPGAcfg(short iPPortBase) {
    bool bConfigured = false;

    // get the current state of the control register
    int origCTRL = win32Inp(iPPortBase+CONTROL_REG_OFFSET) ^ C_XOR;

    // If SS0 is low, bring this high first (to protect against reset)
    int newCTRL;
    if (C_SS0 != (origCTRL & C_SS0)) {
	newCTRL = (origCTRL|C_SS0);
		
	// Output new control state
	win32Outp(iPPortBase+CONTROL_REG_OFFSET, newCTRL^C_XOR);
	os->sleep(1);
	}

    // Ensure that SS0 is high, and SS1 and CNT are low
    newCTRL = (origCTRL|C_SS0) & 0xF5;	// 11110101

    // Output new control state (along with bidirectional input enabled)
    win32Outp(iPPortBase+CONTROL_REG_OFFSET, (newCTRL|C_BI)^C_XOR);
    os->sleep(1);

    // Read the OP input
    bool bOP = false;
    readStatusLines(iPPortBase, NULL, NULL, &bOP, NULL);
    bConfigured = bOP;

    // If SS1 was originally high then bring high now (to protect against reset)
    if (C_SS1 == (origCTRL & C_SS1)) {
	newCTRL = (origCTRL|C_SS1);
		
	// Output new control state (Gray code transition)
	win32Outp(iPPortBase+CONTROL_REG_OFFSET, newCTRL^C_XOR);
	os->sleep(1);
	}

    // restore the control register
    win32Outp(iPPortBase+CONTROL_REG_OFFSET, origCTRL^C_XOR);
    os->sleep(1);

    // OP === FPGA Configured
    return bConfigured ? 1 : 0;
    }


// define constants for max and min output frequencies for the Pll chip
const double MIN_MCLK = 0.406 ;
const double MAX_MCLK = 120.0 ;
const double MIN_VCLK = 0.508 ;
const double MAX_VCLK = 120.0 ;

// define constants for max and min VCO frequencies for the Pll chip
const double MIN_M_VCO_FREQ = 52.0 ;
const double MAX_M_VCO_FREQ = 120.0 ;
const double MIN_V_VCO_FREQ = 65.0 ;
const double MAX_V_VCO_FREQ = 165.0 ;

// define the contraints on the divide ratios P,Q, and D
const int    MIN_P =   4 ;
const int    MAX_P = 130 ;
const int    MIN_Q =   3 ;
const int    MAX_Q = 129 ;
const int    MAX_D =   7 ;

// define the contraints on ref_clk/Q
const double MIN_REFCLK_OVER_Q = 0.2 ;
const double MAX_REFCLK_OVER_Q = 1.0 ;

const int    MREG_ADDRESS = 3 ;

// define the freq boundaries between the VCO preset (index) values
double vco_preset_boundaries[] = {50.0, 51.0, 53.2, 58.5, 60.7, 64.4, 66.8,
                                 73.5, 75.6, 80.9, 83.2, 91.5, 100.0, 120.0} ;

// member fields
double ref_clk = 14.31818 ;      // input to Pll - in MHz
double m_clk = 25.0;                   // desired memory clock output freq
double v_clk = 25.0;                   // desired video clock output freq

int    v_clk_setupreg_no = 0;    // which of Reg0 ...Reg2 is used to set the VClock freq

  
int CalcCtrlWord(double freq, double min_vco_freq, double max_vco_freq);
bool write_pll_reg(short iPPortBase, int addr, int ctrl_word);


/*------------------------------------------------------------------------
 * Execute : Inherited from Arcangel3Test, this method does the actual
 *           programming of the PLL.
 *----------------------------------------------------------------------*/
extern "C" 
#if _MSC_VER
    __declspec(dllexport) 
#endif
int setPLL(short iPPortBase, double MClk_freq, double VClk_freq,
	double *Actual_MClk_freq, double *Actual_VClk_freq) {
    //boolean bConnected = hostIF.isConnected();
    //if (bConnected)  hostIF.disconnect();

    // Get the cobalt setting for the active parallel port address
    //iPPortBase = hostIF.getPortAddress();

    // first need to work out the control words for the frequencies set
    int m_ctrl_word = CalcCtrlWord (MClk_freq, MIN_M_VCO_FREQ, MAX_M_VCO_FREQ) ;
    int v_ctrl_word = CalcCtrlWord (VClk_freq, MIN_V_VCO_FREQ, MAX_V_VCO_FREQ) ;

    // set up the pll chip. We program the MREG, the REG0/1/2 - whichever
    // is selected to control VCLK
    bool ok = write_pll_reg (iPPortBase, MREG_ADDRESS, m_ctrl_word) ;
    ok        &= write_pll_reg (iPPortBase, v_clk_setupreg_no, v_ctrl_word) ;

    if (!ok)  throw "PllProgramming failed: n SetPll";

    return ok ? 0 : -1;

    } // void execute(Board _hostIF, ResultMonitor monitor)


/** private method to calc the control word to produce the requested frequecy.*/
int CalcCtrlWord (double freq, double min_vco_freq, double max_vco_freq) {
/**************************************************************************
* the Pll consists of a VCO and 3 counters that divide by p,q and 2^d. The
* VCO runs at 2*RefClk**p/q. This is divided by 2^d to give the Pll output.
* All frequencies are in MHz
* There are several contrains on the various values.
* 4 <= p <= 130
* 3 <= q <= 129
* 0 <= d <= 7
* 0.2 < ref_clk / q <= 1.0
*
* This method is a bit of a palava - very procedural. Basically it uses
* trial and error to find the best values for d,p and q, within the contraints
   ***************************************************************************/
	
    int index, p,q,d ;               // pll parameters (see ICD2061A Data sheet)
    int trial_p, trial_q ;           // temp vars for p & q values that we are trying out
    double vco_freq = 0 ;            // freq at which the VCO will run
    double temp = freq ;             // temp value
	
    // find a value of d which gives a VCO frequency that is within limits
    d = 0 ;                          // VCO output is divided by 2^d
    while  ( (temp < min_vco_freq) && (d < MAX_D)) {
	temp *= 2 ;
	d++ ;
	}
	
    // check that we have found a suitable value for d
    if ( (temp < min_vco_freq) || (temp > max_vco_freq))
	throw "Bad Pll frequency: in SetPll";
    else
	vco_freq = temp ;
	
    // calc the ratio needed for p/q, to get vco_freq from ref_clk
    double p_over_q = vco_freq / (2.0 * ref_clk) ;
	
    // now use some brute force to find the best values for p & q
    double min_delta = 1.0 ;          // smallest error so far
    p = 0 ; q = 0 ;                   // init p & q, keeps compiler happy
    // calc range of values allowed for q ;
    int first_q = __max((int)(ref_clk/MAX_REFCLK_OVER_Q + 0.999999), MIN_Q) ;
    int last_q  = __min((int)(ref_clk/MIN_REFCLK_OVER_Q), MAX_Q) ;
    for (trial_q = first_q; trial_q <= last_q; trial_q++) {
	// calculate the value of p needed with this q value
	temp    = p_over_q * (double)trial_q ;
	trial_p = (int) (temp + 0.5) ;
		
	// range check the required p value
	if (trial_p < MIN_P) continue ;
	if (trial_p > MAX_P) break ;
		
	// see how much error is caused by p being an integer
	double delta = fabs (1.0 - ((double)trial_p/temp)) ;
	// if this is the most accurate so far, then store it
	if (delta < min_delta) {
	    min_delta = delta ;
	    p = trial_p ;
	    q = trial_q ;
	    }
	// if it is exact then quit
	if (min_delta == 0.0)
	    break ;
	}
	
    // have sorted out values for p,q & d - now form them into a control word
    // first, look up the value for Index (VCO preset)
    for (index = 0; index < 14; index++) {
	if (vco_preset_boundaries[index] > vco_freq)
	    break ;
	}
    // make sure we have found a suitable value for I
    if ((index == 0) || (index >= 14)) return 0 ;
    index-- ;
	
    int ctrlword = (index & 0xf) ;
    ctrlword = (ctrlword << 7) | ((p-3) & 0x7f) ;
    ctrlword = (ctrlword << 3) | (d & 0x7) ;
    ctrlword = (ctrlword << 7) | ((q-2)  & 0x7f) ;
	
    return ctrlword ;
    } // int CalcCtrlWord (double freq, double min_vco_freq, double max_vco_freq) throws Arcangel3TestError


bool write_pll_reg(short iPPortBase, int addr, int ctrl_word) {
    const int C_XOR = 11 ;
    const int C_STR = 1 ;
    const int C_CNT = 2 ;
    const int C_SS0 = 4 ;
    const int C_SS1 = 8 ;
    const int C_BI  = 32 ;

    const int PLL_CLK_BIT  = 0x08 ;
    const int PLL_DATA_BIT = 0x10 ;

    const int s0s1_final_state[] = {0x0, 0x1, 0x2} ;
    int manchester_bitstream[64];
	
    // add the address in the msb's of the ctrl_word
    ctrl_word = (((addr & 0x7) << 21) | (ctrl_word & 0x1fffff)) ;
	
    // create a bit stream at twice the data rate that incorporates the
    // pseudo manchester encoding for the data and also the unlock sequence
    int i ;
    for (i=0; i<11; i++)  manchester_bitstream[i] = 1;
	
    // start bit
    manchester_bitstream[11] = 0 ;
    manchester_bitstream[12] = 0 ;
    manchester_bitstream[13] = 0 ;
	
    i = 14 ;
    for (int j=0; j<24; j++) {
	if ((ctrl_word & 0x1) == 0) {
	    manchester_bitstream[i++] = 1 ;
	    manchester_bitstream[i++] = 0 ;
	    }
	else {
	    manchester_bitstream[i++] = 0 ;
	    manchester_bitstream[i++] = 1 ;
	}
	ctrl_word >>= 1;
	}
	
    // stop bit
    manchester_bitstream[i++] = 1 ;
    manchester_bitstream[i++] = 1 ;
	
    // initialise the gpio port driver
    gpioInit() ;
    gpioMapPort (iPPortBase,4) ;
	
    // snapshot the control port state
    int iOriginalState = win32Inp((short)(iPPortBase+PPORT_CTRL_OFFSET));
	
    // set the parallel port data to 0, in preparation for sending config data
    win32Outp((short)(iPPortBase+PPORT_DATA_OFFSET), 0) ;
    os->sleep(2);
	
    // Set CPLD into config mode:
	
    // set SS0=1, SS1=1, CNT=1, BIDir=0:
    int iControlState = (iOriginalState | C_SS0 | C_SS1 | C_CNT | C_BI) ^ C_BI;
    win32Outp((short)(iPPortBase+PPORT_CTRL_OFFSET), iControlState ^ C_XOR);
    os->sleep(2);
	
    // Ensure STROBE is high
    iControlState = iControlState | C_STR;
    win32Outp((short)(iPPortBase+PPORT_CTRL_OFFSET), iControlState ^ C_XOR);
    os->sleep(2);
	
    // set CPLD into config mode by setting SS0=1, SS1=0, CNT=1
    iControlState = iControlState ^ C_SS1;
    win32Outp((short)(iPPortBase+PPORT_CTRL_OFFSET), iControlState ^ C_XOR);
    os->sleep(2);
	
    // now send the double rate data stream
    // set clock high and data low
    int clk = PLL_CLK_BIT ;
    int data = clk & (~PLL_DATA_BIT) ;
    win32Outp((short)(iPPortBase+PPORT_DATA_OFFSET), data) ;
    //		board.delay(1);
	
    for (i = 0; i < 64; i++) {
	// put the next manchester code bit out
	if (manchester_bitstream[i] == 1)
	    data = data | PLL_DATA_BIT ;
	else
	    data = data & (~PLL_DATA_BIT) ;
	win32Outp((short)(iPPortBase+PPORT_DATA_OFFSET), data) ;
	//			board.delay(1);
		
	// toggle the clock bit
	data = data ^ PLL_CLK_BIT ;
	win32Outp((short)(iPPortBase+PPORT_DATA_OFFSET), data) ;
	//			board.delay(1);
	}
	
    // set data/clock (alias s1/s0) to select the programmed divisor register
    // for the video clock
    win32Outp((short)(iPPortBase+PPORT_DATA_OFFSET), 
    	s0s1_final_state[v_clk_setupreg_no]) ;
    //		board.delay(1);
	
    // set CPLD into ARC-Run Host-Read mode
    iControlState = iControlState | C_SS1 | C_BI;
    win32Outp((short)(iPPortBase+PPORT_CTRL_OFFSET), iControlState ^ C_XOR);
    os->sleep(2);
    //		JARCHWIF.win32Outp((short)(iPPortBase+PPORT_CTRL_OFFSET), ((C_STR | C_CNT | C_SS0 | C_SS1) ^ C_XOR)) ;
	
    return true ;
    }
#else
int setPLL(short iPPortBase, double MClk_freq, double VClk_freq,
	double *Actual_MClk_freq, double *Actual_VClk_freq) { return false; }
int configureCPLD(short iPPortBase, int iGClkSource0, int iGClkSource1,
	    int iGClkSource2, int iGClkSource3) { return false; }
int isFPGAcfg(short iPPortBase) { return false; }
#endif
