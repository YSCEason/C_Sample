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
*
* File name:          lowio.cpp
* 
* Author:             Tom Pennello
*
* Creation Date:      15 Oct 2000
*
* Description:
*
*  This file is linked to API and low level layers of the DLL, contains
*  API's low level implementations, so API layer can be free of target
*  dependent references.  In the event of the debugger requesting to
*  access a low level api function (e.g write_reg()) then the debugger
*  will acknowledge the request and then use the low level api layer to
*  satisfy it.      
*
*  History:
*
*   Version    Date         Author           Description
*
*   1.0        15/10/00     Tom Pennello     File created. 
*   1.1        25/05/01     Huw Lewis        Added ResetState call to prepare_for_new_program()
*   ?.?	       summer '02   Justin Wilde     CPLD config changes
*******************************************************************************/



/*********************************************************
* Include Files
**********************************************************/
#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 
#include <time.h> 
#include <math.h>
#include "gportio.h"
#include "low.h"
#include "proto.h"
#include "globals.h"
#define  OEM_USE_OF_DEBUGGER_HEADER_FILES  1
#include "arcint.h"
#include "dll.h"
#include "port.h"
#include "aa3blast.h"

#include "fpga.h"

/*********************************************************
* Defines
**********************************************************/
#define failure() (printf("FAIL! line %d in %s\n",__LINE__,__FILE__), exit(1))

#define UNINITIALISED -1

//#define DEBUG_INTERFACE 1
//////////////////////////////////////////////////////////
// THIS FILE USES THE METAWARE PARAGRAPHING CONVENTION. //
//////////////////////////////////////////////////////////

struct Low_ARC : ARC {
    Low_ARC(Protocol *protocol) : my_printf(this) {
        this->protocol = protocol;
	protocol->receive_printf(&my_printf);
        blast_loader = 0;
        strcpy(blastdll, "newblast.dll"); //dll used to download an rbf file
        strcpy(aa3blastdll, "aa3blast.dll"); //dll used to download an xbf file

        arc_version = UNINITIALISED;

        #if DEBUG_INTERFACE
            dbg = 1;
        #else
            dbg = 0;
        #endif
	resetClockConfig();
	invalidate_icache = TRUE;
	icache_present = TRUE;
	callback = 0;
	reset_delay_msec = 0;
	}

    bool icache_present;
    int arc_version;                  // ARC version

    int get_arc_version_number();
    
    int iPLLUsage;
    double PLLClocks[2];
    int clockSource[4];
    int clockSourceSet[4];
    int bHarvard;
    char invalidate_icache;
    unsigned reset_delay_msec;

    /*********************************************************
    * Function : version() 
    *
    * Returns the version of the ARC interface used (see arcint.h)
    **********************************************************/
    override int MS_CDECL version() { return ARCINT_TRACE_VERSION; }

    NOINLINE void eprint(const char *format, ...) {
	va_list ap;  va_start(ap,format);
	if (callback) callback->vprintf(format,ap);
	else vprintf(format,ap);
        }
    ARC_callback* callback;
    struct My_printf : Printf {
	My_printf(Low_ARC *low) { this->low = low; }
	Low_ARC *low;
        override void printf(const char *format, ...) {
	    va_list ap;  va_start(ap,format);
	    if (low->callback) low->callback->vprintf(format,ap);
	    else vprintf(format,ap);
	    }
	} my_printf;

    override void MS_CDECL receive_callback(ARC_callback*cb) {
        this->callback = cb;
	}

    /*********************************************************
    * Function : supports_feature() 
    *
    * A check to see if the banked regs are supported
    **********************************************************/
    override int MS_CDECL supports_feature() { return ARC_FEATURE_banked_reg; }

    /*********************************************************
    * Function : id() 
    *
    * Returns indentification string relating to Low layers of DLL
    **********************************************************/
    override const char *MS_CDECL id() { return protocol->id(); }

    /*********************************************************
    * Function : set_memory_size(unsigned S) 
    *
    * Not implemented here, sets memory size
    **********************************************************/
    int MS_CDECL set_memory_size(unsigned S) { return 0; }

    /*********************************************************
    * Function : memory_size() 
    *
    * Not implemented here, returns memory size
    **********************************************************/
    unsigned MS_CDECL memory_size() { return 0; }

    override void MS_CDECL destroy() {
        if (protocol) delete protocol;
        if (blast_loader) blast_loader = 0;
        delete this;
	}

    DLL_load *blast_loader;  //dll obj used for blast dll
    char blastdll[256];     //holds the blast dll name
    char aa3blastdll[256];  //holds the blast dll name
    int dbg;
    Protocol *protocol;
    

    /*********************************************************
    * Function : prepare_for_new_program(int willdownload) 
    *
    * Prepares for new program by ensuring board in correct state
    **********************************************************/
    override int MS_CDECL prepare_for_new_program(int willdownload) {
        static const ULONG  FH_M = 0x00000002;
        
        if (!willdownload) {
            //Avoid init of low level IO due to attaching to a running process
            //board's config would become unstable otherwise ! 
            
            //ensure that in valid state, this is all we can do to try to ensure that port is in
            //normal state.  Can't reset board here because downloading a program.
            
            protocol->ResetState();
            
	    }
        else {  //not attaching to a running process
            protocol->ResetBoard();    
	    if (reset_delay_msec) {
		// TBH 23 JUL 2002 added for Simpod connection 
		// failing on download ELF.
		eprint("Sleeping %d milliseconds after reset and halt "
		    "in lowio prepare_for_new_program().\n", reset_delay_msec);
		os->sleep(reset_delay_msec);
		}

            write_reg (reg_DEBUG, FH_M); // Force Halt the ARC
    
            /* Start talking to the board */
            int ret_val = protocol->Test_Comms_Link(); // do a small comm test
    
            if (ret_val!=1) {
                //CJ-- eprint("Communications could not be established; aborting.\n");
        
                if (dbg) {
                    printInFile("Error %d\n", ret_val);
                    failure();
		    }
        
                return 0;
		}

	    }

        write_reg (reg_DEBUG, FH_M) ; // Force Halt the ARC        
        
        //get the arc version number
        //NOTE:  Assumed all arcs in possible
        //       multi arc set-up are of the same
        //       version
        if (arc_version == UNINITIALISED)
            arc_version = get_arc_version_number();    
        
        //success
        return 1;
	}


    /*********************************************************
    * Function : read_memory(unsigned long adr, void *buf,
                unsigned long amount, int context = 0)
    *
    * Reads a block of mem from the slave device
    **********************************************************/
    override int MS_CDECL read_memory(unsigned long adr, void *buf,
                unsigned long amount, int context = 0) {
        // Do the low-level stuff.
        int ret_val = protocol->MemRead((char *) buf, adr, amount);
        if (dbg) {
            if (ret_val!=1) {
                printInFile("Error %d\n", ret_val);
                failure();
            }
	    }

        return ret_val == 1 ? amount : 0;  //Returns the number of BYTEs read
	}
    
    
    /*********************************************************
    * Function : write_memory(unsigned long adr, void *buf,
                unsigned long amount, int context = 0) 
    *
    * Writes a block of mem to the slave device
    **********************************************************/
    override int MS_CDECL write_memory(unsigned long adr, void *buf,
                unsigned long amount, int context = 0) {
        // Do the low-level stuff.
        int ret_val;
        ULONG status, halt;
        /* When setting software breakpoints memory containing code is altered
        it is neccessary to invalidate the icache (any value written to
        auxiliary register 10h) so that the new code is read into the cache. */

        /* check before whether the ARC is running or not.
        A read_reg error will occur if the host is trying to access an aux
        register when ARC is running. */

	/* Need to determine if its ARCCompact or not */

	if (arc_version >= 10) {
	    ULONG   AC_HALT_BIT_M = 0x00000001;      // Halt Bit

	    read_reg(reg_AC_STATUS32,&status);
	    halt = (status & AC_HALT_BIT_M) != 0; // gets the halt bit
	    }
	else {

	    ULONG   HALT_BIT_M = 0x02000000;      // Halt Bit

	    read_reg(reg_STATUS,&status);
	    halt = (status & HALT_BIT_M) != 0; // gets the halt bit

	    }
	// For hostlink the program takes off when SeeCode writes the
	// response buffer.  For programs that wish to have no more
	// debug accesses afterwards (for careful timing purposes),
	// compute the halt condition before the memwrite.
	// That's why it's above.
        ret_val = protocol->MemWrite((char *) buf, adr, amount);

        if (dbg) {
            if (ret_val!=1) {
                printInFile(" Error %d\n", ret_val);
                failure();
		}
	    }
    
	static const int FIRST_A7_IDENT = 0x31;
        if (halt && invalidate_icache && 
		(icache_present || arc_version < FIRST_A7_IDENT))
	    // ARC is not running and we have to invalidate icache
	    // or at least reset the detected-brk signal.
            ret_val = write_reg(reg_ICACHE, 0xffffffff); // invalidate cache

        return ret_val == 1 ? amount : 0; //Returns the number of BYTEs written.
	}

    #define NOINLINE virtual
    NOINLINE void resetClockConfig() {
	int i;
	iPLLUsage = 0;
	PLLClocks[0] = 25.0;
	PLLClocks[1] = 25.0;
	for (i=0; i<4; i+=1)  clockSource[i] = DefaultGClkSources[i];
	for (i=0; i<4; i+=1)  clockSourceSet[i] = 0;
	bHarvard = 0;
	}

    NOINLINE int setPLLClock(unsigned int iClockId, double dblClockValue) {
	// Has this freq. already been asigned to a PLL output ?
	int iFound = -1;
	for (int i=0; i<iPLLUsage; i+=1) {
	    if (dblClockValue==PLLClocks[i])  iFound = i;
	    }

	double ActualMClock = 0.1;
	double ActualVClock = 0.1;

	// Do we need another PLL output
	if (-1 == iFound && 2==iPLLUsage) {
	    // If so, and there aren't any left....
	    fprintf(stderr, "Can't set GCLK%d to \"%.2lf\".  There are no more PLL outputs available.\n", iClockId, dblClockValue);
	    double delta = fabs(PLLClocks[0]-dblClockValue);
	    if (delta < fabs(PLLClocks[1]-dblClockValue))
		iFound = 0;
	    else
		iFound = 1;
	    }
	else {
	    // Can we use an existing output ?
	    if (-1 == iFound) {
		// If not asign the next PLL output ...
		PLLClocks[iPLLUsage] = dblClockValue;
		iFound = iPLLUsage;
		iPLLUsage += 1;
		}
				
	    setPLL(protocol->get_port(), 
		PLLClocks[0], PLLClocks[1], &ActualMClock, &ActualVClock);
	    }

	if (bHarvard) {
	    clockSource[iClockId] = 
		(0==iFound 
		    ? CLOCK_SOURCE_PLL_MCLK_HARVARD 
		    : CLOCK_SOURCE_PLL_VCLK_HARVARD);
	    }
	else {
	    clockSource[iClockId] = 
	    (0==iFound 
		? CLOCK_SOURCE_PLL_MCLK 
		: CLOCK_SOURCE_PLL_VCLK);
	    }
	clockSourceSet[iClockId] = 1;
			
	return iFound;
	}


    NOINLINE int setClock(unsigned int iClockId, const char *pClockData) {
	int iRetVal = 0;	// 0 == OK
		
	double iClockValue = 25.0;

	if (iClockId > 3) iRetVal = -1;
	else if (NULL==pClockData) iRetVal = -2;
	else if (0==os->stricmp("crystal", pClockData)) {
	    clockSource[iClockId] = CLOCK_SOURCE_CRYSTAL;
	    clockSourceSet[iClockId] = 1;
	    eprint("GCLK%d   << %s\n", 
	    	iClockId, CLOCK_SOURCE_STRINGS[clockSource[iClockId]]);
	    }
	else if (0==os->strnicmp("dip", pClockData, 3)) {
	    clockSource[iClockId] = CLOCK_SOURCE_CRYSTAL_DIVIDED;
	    clockSourceSet[iClockId] = 1;
	    if (3==iClockId)
		eprint("GCLK%d   << %s\n", iClockId, 
			GCLOCK3_SOURCE_STRINGS[clockSource[iClockId]]);
	    else
		eprint("GCLK%d   << %s\n", iClockId, 
			CLOCK_SOURCE_STRINGS[clockSource[iClockId]]);
	    }
	else if (0==os->strnicmp("highimp", pClockData, 7)) {
	    clockSource[iClockId] = CLOCK_SOURCE_HIGH_IMPEDANCE;
	    clockSourceSet[iClockId] = 1;
	    eprint("GCLK%d   << %s\n", iClockId, 
		CLOCK_SOURCE_STRINGS[clockSource[iClockId]]);
	    }
	else if (0==os->strnicmp("host", pClockData, 4)) {
	    clockSource[iClockId] = CLOCK_SOURCE_HOST_STROBE;
	    clockSourceSet[iClockId] = 1;
	    eprint("GCLK%d   << %s\n", iClockId, 
		CLOCK_SOURCE_STRINGS[clockSource[iClockId]]);
	    }
	// Extract the requested clock value
	else if (1 == sscanf(pClockData, "%lf", &iClockValue)) {
	    int iPLLClock = setPLLClock(iClockId, iClockValue);
	    if (0 <= iPLLClock)
		eprint("GCLK%d   << %s==%.2lf\n", iClockId, 
		    CLOCK_SOURCE_STRINGS[clockSource[iClockId]], 
		    PLLClocks[iPLLClock]);
	    }

	configureCPLD(protocol->get_port(), 
	    clockSource[0], clockSource[1], clockSource[2], clockSource[3]);

	return iRetVal;
	}


    NOINLINE int reconfigureClocks() {
	int iRetVal = 0;

	if (!isFPGAcfg((short)protocol->get_port())) {
	    eprint("Warning: Can not set up clock routing as the FPGA is not configured.\n");
	    iRetVal = -1;
	    }

	double ActualMClock = 0.1;
	double ActualVClock = 0.1;
		
	if (0 == iRetVal) {
	    // configure PLL clocks
	    setPLL(protocol->get_port(), 
		PLLClocks[0], PLLClocks[1], &ActualMClock, &ActualVClock);
					
	    // configure clock routing
	    configureCPLD(protocol->get_port(), 
		clockSource[0], clockSource[1], clockSource[2], clockSource[3]);
	    }

	for (int i=0; 0 == iRetVal && i<4; i+=1) {
	    if (0 == clockSourceSet[i]) ; // do nothing
	    else if (clockSource[i] == CLOCK_SOURCE_PLL_MCLK) 
		eprint("GCLK%d   << %s==%.2lf\n", i, 
		    CLOCK_SOURCE_STRINGS[clockSource[i]], PLLClocks[0]);
	    else if (clockSource[i] == CLOCK_SOURCE_PLL_MCLK_HARVARD)
		eprint("GCLK%d   << %s==%.2lf\n", i, 
		    GCLOCK3_SOURCE_STRINGS[clockSource[i]], PLLClocks[0]/2);
	    else if (clockSource[i] == CLOCK_SOURCE_PLL_VCLK)
		eprint("GCLK%d   << %s==%.2lf\n", i, 
		    CLOCK_SOURCE_STRINGS[clockSource[i]], PLLClocks[1]);
	    else if (clockSource[i] == CLOCK_SOURCE_PLL_VCLK_HARVARD)
		eprint("GCLK%d   << %s==%.2lf\n", i, 
		    GCLOCK3_SOURCE_STRINGS[clockSource[i]], PLLClocks[1]/2);
	    else if (3 == i)
		eprint("GCLK%d   << %s\n", i, 
			    GCLOCK3_SOURCE_STRINGS[clockSource[i]]);
	    else
		eprint("GCLK%d   << %s\n", i, 
			    CLOCK_SOURCE_STRINGS[clockSource[i]]);
	    }

	return iRetVal;
	}


    NOINLINE int enableHarvardClock() {
	int iRetVal = 0;

	// Does GClk3 come from the PLL
	if (clockSource[3] == CLOCK_SOURCE_PLL_MCLK ||
	    clockSource[3] == CLOCK_SOURCE_PLL_VCLK) {
	    eprint("Reconfiguring clocks to drive Harvard Ctl_Clk.\n");

	    // save existing settings
	    int savedclockSource[4] = {
		clockSource[0], clockSource[1], clockSource[2], clockSource[3]
		};

	    double savedPLLClocks[2] = { PLLClocks[0], PLLClocks[1] };
			
	    // Reset clock config
	    resetClockConfig();

	    // Now reasign the Harvard inputs and double the 
	    // requested freq
	    double dblClockValue;
	    if (CLOCK_SOURCE_PLL_MCLK == savedclockSource[3])
		dblClockValue = savedPLLClocks[0];
	    else
		dblClockValue = savedPLLClocks[1];
			
	    int iPLLClock = setPLLClock(3, 2*dblClockValue);

	    clockSource[3] = CLOCK_SOURCE_PLL_MCLK_HARVARD;

	    if (0 <= iPLLClock) {
		eprint("GCLK3   << %s==%.2lf\n", 
		    GCLOCK3_SOURCE_STRINGS[clockSource[3]], 
		    PLLClocks[iPLLClock]/2);
		if (0) eprint("Ctl_Clk << %s==%.2lf [-90degrees]\n", 
		    GCLOCK3_SOURCE_STRINGS[clockSource[3]], PLLClocks[iPLLClock]);
		}

	    // reasign any existing PLL clocks
	    for (int iClockId=0; iClockId<3; iClockId+=1) {
		if (savedclockSource[iClockId] == CLOCK_SOURCE_PLL_MCLK ||
		    savedclockSource[iClockId] == CLOCK_SOURCE_PLL_VCLK) {
		    if (CLOCK_SOURCE_PLL_MCLK == savedclockSource[iClockId])
			dblClockValue = savedPLLClocks[0];
		    else
			dblClockValue = savedPLLClocks[1];
		    int iPLLClock = setPLLClock(iClockId, dblClockValue);
		    if (0 <= iPLLClock)
			eprint("GCLK%d   << %s==%.2lf\n", iClockId, 
			    CLOCK_SOURCE_STRINGS[clockSource[iClockId]], 
			    PLLClocks[iPLLClock]);
		    }
		}
			
	    configureCPLD(protocol->get_port(), 
		clockSource[0], clockSource[1], clockSource[2], clockSource[3]);
	    }
	    #if 0
	    else if (clockSource[3] == CLOCK_SOURCE_CRYSTAL_DIVIDED) {
		eprint("GCLK3   << %s\n", 
		    GCLOCK3_SOURCE_STRINGS[clockSource[3]]);
		eprint("Ctl_Clk << %s [-90degrees]\n", 
		    CLOCK_SOURCE_STRINGS[clockSource[3]]);
		}
	    #endif

	return iRetVal;
	}
    

    #if BLAST_DATA_FROM_FILE
    struct Blast_input_from_file : Blast_input {
	FILE *fp;
	override int version() { return 1; }
	override int read(void *buf, unsigned amount) {
	    return fread(buf,1,amount,fp);
	    }
	override int size() {
	    // size also resets the seek pointer to BOF.
	    fseek(fp,0,SEEK_END);
	    long sz = (long)ftell(fp);
	    fseek(fp,0,SEEK_SET);
	    return sz;
	    }
	override int eof() { return feof(fp); }
	override const char *name() { return fname; }
	const char *fname;
	int err_code;
	Blast_input_from_file(const char *fname) {
	    // try opening the file
	    fp = fopen(fname,"rb");
	    if (err_code = fp == 0, err_code) 
		printf("Error: Could not open file `%s'.", fname);
	    this->fname = fname;
	    }
	};
    #endif
    struct Blast_input_from_memory: Blast_input {
	Blast_input_from_memory(char *addr, unsigned len, const char *spec) {
	    this->addr = addr;
	    this->len = len;
	    this->spec = spec;
	    pointer = addr;
	    }
	char *addr; unsigned len;
	const char *spec;
	char *pointer;
	override int version() { return 1; }
	override int read(void *buf, unsigned amount) {
	    memcpy(buf,pointer,amount);
	    pointer += amount;
	    return amount;
	    }
	override int size() {
	    // size also resets the seek pointer to BOF.
	    pointer = addr;
	    return len;
	    }
	override int eof() { return pointer >= addr+len; }
	char buf[512];
	override const char *name() { 
	    sprintf(buf,"<data from memory:%s>",spec);
	    return buf;
	    }
        };
    
    /*********************************************************
    * Function : process_property(const char *key, const char *value)
    *
    * Process a property, blast related features of the DLL, checks
    * if lower level layers need to process a property
    **********************************************************/
    override int MS_CDECL process_property(const char *key, const char *value) {
        bool mblast = false;
        if (!os->stricmp(key,"dbg_int")) dbg = *value == '1';

        // Either low didn't respond to blast, or we have no low.
        /* download configuration file to the ARCAngel */
    
        else if (os->stricmp(key, "blastdll") == 0) {
            //The sc.cnf file will tell us where the blast dll is, if
            //we're using the default configuration
            strcpy(blastdll, value);
	    }
        else if (os->stricmp(key, "aa3blastdll") == 0) {
            //The sc.cnf file will tell us where the blast dll is, if
            //we're using the default configuration
            strcpy(aa3blastdll, value);
	    }

	else if (!os->stricmp(key,"reset_delay")) { 
            reset_delay_msec = strtoul(value,0,0);
            eprint("Reset delay is now %d milliseconds.",reset_delay_msec );
	    }
	else if (!os->stricmp(key,"gverbose")) gverbose = *value == '1';

        else if (os->stricmp(key,"blast") == 0 || 
		// rblast = remote_blast; not intercepted by SCIT client.
                 os->stricmp(key,"rblast") == 0 || 
		 // mblast = memory blast.
		(mblast=os->stricmp(key,"mblast")==0,mblast) ) {
	    /*
Normal blasting occurs when a target DLL gets a property "blast" whose value
is a filename.  E.g.:
	Property	value
	--------	------
	blast		zorch.xbf

To allow the file to reside where the debugger is rather than the remote
arcangel3, a target DLL should implement memory blast -- property "mblast" whose value
is "address:length:file info".  The address and length are the only things of
important; the file info is incididentally the name of the file that was copied
from the debugger to the remote machine's memory.  E.g.:

	Property	value
	--------	------
	mblast	0x69f0d8:0x1360c4:(copy of zorch.xbf)

The target DLL needs only to read the contents of memory at 0x69f0d8
for length 0x1360c4 (= 1269956 decimal, an aa3 blast file length).
and ship it out to the target.  The target DLL does not need to worry
about how the contents of memory got initialized with a blast file,
nor should it worry about deallocating said memory.
	    */
	    Blast_input *bin = 0;
	    int blast_size = 0;
	    const char *download_function = "DownloadConfig";
	    if (mblast) {
		// Blast from local memory.  Value is addr:len.
		// Someone magically arranged that the stuff is in my memory.
		unsigned addr,len;
		sscanf(value,"%i:%i",&addr,&len);
		bin = new Blast_input_from_memory((char*)addr,len,
			value);
		blast_size = len;
		download_function = "DownloadConfigData";
		}
	    else {
		// File is "value".
		FILE *fp = fopen(value,"rb");
		if (fp) {
		    fseek(fp,0,SEEK_END);
		    blast_size = ftell(fp);
		    fclose(fp);
		    }
		else {
		    printf("Error: Could not open file `%s'.\n", value);
		    return 0;
		    }
		#if BLAST_DATA_FROM_FILE
    		const bool data_interface = 0;
		if (data_interface) {
		    // Test using the data interface.
		    bin = new Blast_input_from_file(value);
		    printf("!TESTING LOW-LEVEL DATA INTERFACE\n");
		    }
		#endif
		}
	    // Auto-detect use of AA3 vs AA2: if the size is big enough,
	    // assume AA3.
	    bool use_aa3_blast = blast_size >= 1269956;
	    char *which_blast = use_aa3_blast ? aa3blastdll : blastdll;
            if (gverbose) eprint("Blast with %s; "
	    	"using blast dll %s.\n", 
		bin ? bin->name() : value, which_blast);
	    if (!use_aa3_blast && bin) {
	        eprint("Blast from memory not supported with "
			"old blast files.\n");
		return 0;
		}

            blast_loader = DLL_load::new_DLL_load();
            int OK = blast_loader->load_DLL(which_blast);
        
            if (!OK) {
                eprint("Error: cannot load blast DLL %s.\n", blastdll);
                return 0;
		}

	    // Get the download function.
            //type definition to get the download function from Newblast.dll
            typedef int (*DLLGET_DOWNLOAD) 
	    		(void *parm1, UWORD p_port_base, int WinNT);
            DLLGET_DOWNLOAD download;

	    download = (DLLGET_DOWNLOAD) 
		blast_loader->get_proc_address(download_function);
	    if (download == 0) {
		eprint("Error: cannot find function %s in "
			"blast DLL.\n", download_function);
		return 0;
		}
	    void *parm1 = bin ? (void*)bin : (void*) value;	// filename
	    /* and download the blast file to the Arcangel */
	    unsigned start = time(0);
	    char success = !((*download)(parm1,
		    protocol->get_port(), 
		    // We must pass the port object to the DLL.
		    // If we bundle another copy in the DLL, crashes 
		    // happen, probably due to UNIX name sharing among .sos.
		    (int)port
		    ));
	    // aa3blast.dll doesn't return a good condition code,
	    // so ignore it and hope for the best.
	    if (success && !use_aa3_blast) {
		eprint("Error: can't download the configuration file\n");
		}
	    else 
		if (gverbose) eprint("Blast of %s complete.\n",
			bin ? bin->name() : value);
	    unsigned stop = time(0);
	    if (stop-start > 15 /*seconds*/ && use_aa3_blast && gverbose) {
    eprint("**************************************************************\n");
    eprint(">>> AA3 switch 2 = on (down) ensures fastest blast speeds. <<<\n");
    eprint("**************************************************************\n");
		}
	    // FIX
	    bool bClocksSet = false;
	    for (int i=0; i<4; i+=1)  
		if (1==clockSourceSet[i])  bClocksSet = true;
	    if (bClocksSet) {
		eprint("Reconfiguring clock settings after "
		    "FPGA blast.\n");
		reconfigureClocks();
		}
        
            delete blast_loader;
	    if (bin) delete bin;
	    }
    	else if (0 == os->strnicmp(key, "gclk", 4)) {
	    int iLen = strlen(key);
	    if (!isFPGAcfg((short)protocol->get_port())) {
		eprint("\n-%s: Can not set up clock "
		    "routing until FPGA is configured.\n", key);
		}
	    else if (4 == iLen) {
		setClock(3, value);
		}
	    else if (5 != iLen) return 0;
	    else if ('s' == key[4]) {
		// reset count of used PLL clocks
		resetClockConfig();

		unsigned int iClockId = 0;
		unsigned int iIndex = 0;
		char *pClockData = NULL;
		const char *pData = value;
		while ('\0' != pData[iIndex]) {
		    if (',' == pData[iIndex]) {
			pClockData = new char[iIndex+1];
			if (NULL != pClockData) {
			    strncpy(pClockData, pData, iIndex);
			    pClockData[iIndex] = '\0';
			    setClock(iClockId++, pClockData);
			    delete[] pClockData;
			    }
			pData += iIndex+1;
			iIndex = 0;
			}
		    else {
			    iIndex += 1;
			}
		    }
		setClock(iClockId, pData);
		}
	    else if ('0' <= key[4] && '3' >= key[4]) {
		0 && fprintf(stderr, "-gclkn found\n");
		setClock(((int)key[4])-48, value);
		}
	    }
	else if (0 == os->stricmp(key, "harvard")) {
	    bHarvard = 1;
	    enableHarvardClock();
	    }
	else if (0 == os->stricmp(key,"slow_clock")) {
	    // TBH 27 JAN 03. 
	    slow_clock=strtoul(value,0,0);
	    0 && printf("slow clock is %d\n",slow_clock);
	    if (slow_clock) use_extended_driver=FALSE;
	    }
	else if (0 == os->stricmp(key, "invalidate_icache")) {
	    // Some customers like to turn this off when first coming
	    // up with troublesome hardware.
	    // Note: if you do this, SW breakpoints may fail.
	    invalidate_icache = *value == '1';
	    if (invalidate_icache == 0) 
	        eprint("WARNING: with invalidate_icache off, "
			"software breakpoints most likely will fail.\n");
	    }
	else if (0 == os->stricmp(key, "port")) {
	    // Intercept port change that goes to protocol.
	    int ret = protocol->process_property(key,value);
	    if (ret) {
		bool bClocksSet = false;
		for (int i=0; i<4; i+=1)  
		    if (1==clockSourceSet[i]) bClocksSet = true;
		if (bClocksSet) {
		    eprint("Port address changed, "
			"now reconfiguring clock settings.\n");
		    reconfigureClocks();
		    }
		}
	    else return 0;	// problem with port change.
	    }
	else return protocol->process_property(key,value);
	// If we processed it, return 1:
	return 1;
	}


    /*********************************************************
    * Function :  read_reg(ARC_REG_TYPE r, unsigned long *value)
    *
    * Reads a register value from the slave device
    **********************************************************/
    override int MS_CDECL read_reg(ARC_REG_TYPE r, unsigned long *value) { 
    
        int max_core_reg = 0;    //holds the maximum core register number, so range can be adjusted for different arcs
    
        if (arc_version < 10) 
            max_core_reg = 60;   //for arc versions less than 10
        else 
            max_core_reg = 63;   //for arc version 10 and above
    
        int ret_val;

        *value = 0;

        if ( r>=0 && r<=max_core_reg)    /* core registers read */
            ret_val = protocol->Read_ARC_Register(CORE_REGISTER, r, value);
        else 
            if ((unsigned)r>=AUX_BASE) {   /* Auxiliary registers. */ 
                r -= AUX_BASE;
                ret_val = protocol->Read_ARC_Register(AUX_REGISTER, r, value);
		}
            else
                return 0; // Failure. Register doesn't exist

        if (dbg) {
            if (ret_val!=1) {
                printInFile("Error %d\n", ret_val);
                failure();
		}
            else 
                printInFile(" 0x%08x",*value);
	    }

        return ret_val;
	}


    /*********************************************************
    * Function :  write_reg(ARC_REG_TYPE r, unsigned long value) 
    *
    * Writes a register value to a slave device
    **********************************************************/
    override int MS_CDECL write_reg(ARC_REG_TYPE r, unsigned long value) {
    
        int max_core_reg = 0;    //holds the maximum core register number, so range can be adjusted for different arcs
    
        if (arc_version < 10) 
            max_core_reg = 60;   //for arc versions less than 10
        else 
            max_core_reg = 63;   //for arc version 10 and above
    
    
        int ret_val;
        /* Core registers and loop count register */
    
        if ( r>=0 && r<=max_core_reg ) {
            ret_val = protocol->Write_ARC_Register(CORE_REGISTER, r, value);
	    }
        else 
            if ((unsigned)r>=AUX_BASE) {  /* Auxiliary registers. */ 
                r-=AUX_BASE;
                ret_val = protocol->Write_ARC_Register(AUX_REGISTER, r, value);
		}
            else
                return 0;

        if (dbg) {
            if (ret_val!=1) {
                printInFile(" Error %d\n", ret_val);
                failure();
		}
	    }

        return ret_val;
	}

    /*********************************************************
    * Function : read_banked_reg(int bank, ARC_REG_TYPE r, unsigned long *value)  
    *
    * Read a reg from a slave device, using reg banks for access(see arcint.h)
    **********************************************************/
    override int MS_CDECL read_banked_reg(int bank, 
    	ARC_REG_TYPE r, unsigned long *value) {
        switch(bank) {
            case reg_bank_core: return read_reg(r,value);
            case reg_bank_aux: 
                return protocol->Read_ARC_Register(AUX_REGISTER, r, value);
            case reg_bank_madi: 
                return protocol->Read_MADI_Register(r,value);
	    }
        
        return 0;
	}

    /*********************************************************
    * Function : write_banked_reg(int bank, ARC_REG_TYPE r, unsigned long *value)
    *
    * Write a value to a slave device register, using reg banks for access(see arcint.h)
    **********************************************************/
    override int MS_CDECL write_banked_reg(int bank, 
    	ARC_REG_TYPE r, unsigned long *value) {
        switch(bank) {
            case reg_bank_core: return write_reg(r,*value);
            case reg_bank_aux: 
                return protocol->Write_ARC_Register(AUX_REGISTER,r,*value);
            case reg_bank_madi: 
                return protocol->Write_MADI_Register(r,*value);
	    }
        
        return 0;
	}
    };

#define IMPNAME Low_ARC
//CJ-- CHECK_OVERRIDES(ARC)

ARC *get_low_ARC() {
    // Get linked-in protocol.
    Protocol *p = get_protocol();
    return new Low_ARC(p);
    }


/*********************************************************
* int get_arc_version_number() 
* 
* Return the ARC version number
**********************************************************/
int Low_ARC::get_arc_version_number() {
    ULONG identity = 0, icache_build;

    read_reg(reg_IDENTITY, &identity);
    read_reg(64+0x77 /* icache build */, &icache_build);
    icache_present = icache_build && icache_build != identity;
    0 && printf("!ICACHE IS PRESENT %d\n",icache_present);
    return identity & 0x000000ff;
    }

