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
* File name:          port.cpp
* 
* Author:             Tom Pennello
*
* Creation Date:      15 0ct 2000
*
* Description:
*
*    Reduces code repetition by storing all similarities of jtag/par port layer of dll
*    in this file.
*   
* History:
*
*   Version    Date        Author           Description
*   
*   1.0        15/10/00    Tom Pennello     File created. 
* 
*******************************************************************************/


/**********************
 Include Files
 **********************/

#include "port.h"
#include <stdio.h>
#include "gportio.h" 
#include "globals.h"


/**********************
 Defines
 **********************/

char dbg_port = 0;
unsigned epp_addr = 0x378;       // parallel port address default to 0x378
unsigned epp_control_reg_addr = 0x378+CONTROL_REG_OFFSET;
unsigned epp_status_reg_addr = 0x378+STATUS_REG_OFFSET;
unsigned char driver_major_version = 1;
unsigned char use_extended_driver = FALSE;
unsigned slow_clock = 0;
static double d = 1234567;
void slow_clock_loop() { 
    for (unsigned i = 0; i < slow_clock; i++) {
        d = 1234567;
	for (int j = 0; j < 100; j++) d /= 1.000001;
	}
    }

bool Win_NT_Host;   // Defining occurrence.

unsigned int Port::output_array(gpio_array_struct *s, unsigned long size) { 
    return win32OutpArray(s,size);
    }

#if _MSC_VER

#include <windows.h>
#include <winbase.h>
#include <conio.h>

struct NT95_port : Port {

    char gportio_version[2];
    NT95_port() { memset(gportio_version,0,sizeof(gportio_version)); }

    override int setup(unsigned port_num, int num) {
	/*********************************************************
	* Setup 95/98/nt io port stuff
	* return TRUE if success, ERR_SET_PORT if fail 
	**********************************************************/

	if (Win_NT_Host) {
	    // windows NT 
	    if (dbg_port) {
		printInFile("\t\n set up NT port driver at address 0x%x", port_num);
		}

	    // Init io port driver 
	    if (gpioInit() !=1) {
		if (dbg_port) 
		    printInFile("\t\n Parallel Port Driver startup failed !");
        
		return ERR_SET_PORT;
		}
	    else {
		if (dbg_port) 
		    printInFile("\t\nNT Parallel Port Driver startup OK!");
		gportioVersion(gportio_version);
		driver_major_version = gportio_version[0];
		use_extended_driver = driver_major_version >= 2 && 
			slow_clock == 0;
		// Delayed:
		// helpful_messages();
		}

	    // map the driver to the port address 
	    if (gpioMapPort(port_num,num) !=1) {
		if (dbg_port) printInFile("\t\nCould Not map port!");
            
		return ERR_SET_PORT;
		}
	    else {
		if (dbg_port) printInFile("\t\nPort Mapped OK");        
		// allow programming of parallel interface 
		}
	    }
    
	epp_addr = port_num;
	epp_status_reg_addr = port_num + STATUS_REG_OFFSET;
	epp_control_reg_addr = port_num + CONTROL_REG_OFFSET;

	return 1;
	}

    override int shutdown() {
	/*********************************************************
	* SHutdown the NT io port stuff
	* return ERR_SHUT_PORT if fail, TRUE otherwise
	**********************************************************/
	if (Win_NT_Host==TRUE) {
	    if (dbg_port) 
		printInFile("\t\n shutdown NT port...");

	    if (gpioFini()!=1) {
		if (dbg_port) printInFile("FAIL");
		return ERR_SHUT_PORT;
		}
	    else {
		//CJ-- if (dbg_port) printInFile("OK");
		return 1;
		}
	    }
	else return 1;
	}

    override int outp(unsigned port, int value) {
	/*********************************************************
	* wrapper function to standard outp_95NT funtion (Win95)
	* or MetaWare's gportio driver (WinNT)
	* win32Outp may be either used for NT or 95.
	* Nevertheless, this function is slower than _outp for 95
	**********************************************************/
	static int pcnt = 0;
	// dbg_port && printInFile("%02x => port %d\n",value,++pcnt);
	dbg_port && printInFile("%02x =>   port 0x%x\n",value,port);
	// This is how you can bomb at a certain pcnt if you want to see who's
	// sending a certain data byte.
	// if (dbg_port && pcnt == 12) { char *p = 0; *p = 0; }

	if (Win_NT_Host)    
	    return(int)win32Outp(port, (unsigned int)value); // NT    
	else      
	    #if __HIGHC__
		_outb(port,value); return 1;
	    #else
		return _outp(port,value);// 95/98
	    #endif
	}

    override int inp(unsigned port) {
	/*********************************************************
	* wrapper function to standard inp_95NT funtion (Win95)
	* or MetaWare's gportio driver (WinNT)
	* win32Inp may be either used for NT or 95. Nevertheless, 
	* this function is slower than _outp for 95
	**********************************************************/
      
	unsigned input;

	if (Win_NT_Host)        
	    input = ((int)win32Inp(port)); // NT   
	else        
	    #if __HIGHC__
		input = _inb(port);
	    #else
		input = _inp(port);// 95/98  
	    #endif
    
	dbg_port && printInFile("%02x   <= port 0x%x\n",input,port);
    
	return input;
	}

    override void helpful_messages() {
	// Print out useful info on the || port driver -- but only after
	// gverbose has had a chance to be set.
	// Print these out later when gverbose is known to be set or not.
	if (!gverbose) return;
	//CJ-- printf("Parallel port driver (gportio.sys) ""version %d%s detected.\n",
	//CJ--	gportio_version[0], gportio_version[0] >= 2 ? " (fast)":"");
	if (!use_extended_driver) {
	    printf("\tWe recommended upgrading to a newer gportio.sys\n");
	    printf("\tfor faster board access, especially for JTAG.\n");
	    }
	}
    };

Port *port = new NT95_port();

static void compute_host() {
    OSVERSIONINFO os_version;  
    os_version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    // get the os version
    if (GetVersionEx(&os_version) == FALSE) {    
        printf("\nCould not determine OS; assuming Win95/98!!!");    
        Win_NT_Host=FALSE;  
	}
    else 
        if (os_version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
            Win_NT_Host = FALSE;  
        else 
            if (os_version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
                /* windows NT */    
                Win_NT_Host = TRUE;   
		}
    // printf("\nHOST = %s\n", Win_NT_Host? "WinNT":"Win95/98");
    }



// We need to find the host before we can do any || port IO.
// By putting this static initialized variable, C++ invokes compute_host()
// before just about anything else.
static int startup = (compute_host(), 0);

#elif _NOPORT

struct No_port : Port {
    // Stub code for when we have no port connection yet.

    bool claimed;
    int par_fd;

    No_port() { claimed = FALSE; par_fd = -1; }

    NOINLINE int check_setup() {
	return 1;
        }

    override int setup(unsigned port_num, int num) {
	printf("!set up no port at %x\n",port_num);

	return 1;
	}

    override int shutdown() {
	return 1;
	}

    override int outp(unsigned port, int value) {
	0 &&  printf("port %x outp %x\n",port,value);

	return 0;
	}

    override int inp(unsigned port) {
	char c = 0x01;
	0 && printf("%02x <= port 0x%x\n",c,port);
	return c;
	}

    override void helpful_messages() { }
    };

Port *port = new No_port();

#elif _LINUX

#include <unistd.h>
#include <fcntl.h>
#include <linux/ppdev.h>
#include <linux/parport.h>
#include <asm/ioctl.h>
#include <sys/ioctl.h>

#if SLOW_LINUX
// This uses /dev/parport0 to do one byte at a time via the driver (!).
struct Linux_port : Port {

    bool claimed;
    int par_fd;
    static const bool trace = 0;

    Linux_port() { claimed = FALSE; par_fd = -1; }

    NOINLINE int check_setup() {
        if (claimed == FALSE) {
	    par_fd = open("/dev/parport0",O_RDWR);
	    if (par_fd == -1) {
	        perror("open");
		return 0;
		}
	    if (ioctl(par_fd,PPCLAIM)) {
	        perror("claim || port");
	      CLOSE: ;
		close(par_fd);
		return 0;
		}
	    unsigned mode = IEEE1284_MODE_EPP;
	    #if NEGOT
	    if (ioctl(par_fd, PPNEGOT, &mode)) {
	        mode = IEEE1284_MODE_ECP;
		if (ioctl(par_fd, PPNEGOT, &mode)) {
		    perror("switch to EPP/ECP mode.");
		    goto CLOSE;
		    }
		}
	    #else
	    // I don't understand the need for the SETMODE.
	    // Ah: PPNEGOT does it for me, so I don't need to.
	    if (ioctl(par_fd, PPSETMODE, &mode)) {
	        perror("set read/write mode");
		goto CLOSE;
		}
	    #endif
	    // OK, switched to either EPP or ECP; either should work.
	    claimed = TRUE;
	    printf("OK, claimed linux port\n");
	    }
	return 1;
        }

    override int setup(unsigned port_num, int num) {
        if (!check_setup()) return ERR_SET_PORT;

	epp_addr = port_num;
	epp_status_reg_addr = port_num + STATUS_REG_OFFSET;
	epp_control_reg_addr = port_num + CONTROL_REG_OFFSET;

	return 1;
	}

    override int shutdown() {
        if (!check_setup()) return ERR_SET_PORT;
	if (claimed) {
	    claimed = FALSE;
	    close(par_fd);
	    }
	return 1;
	}

    override int outp(unsigned port, int value) {
        check_setup();

	dbg_port && printInFile("%02x =>   port 0x%x\n",value,port);

	unsigned char c = value;
	if (port == epp_addr) {
	    // Data write.
	    trace && printf("data write %x\n",c);
	    return write(par_fd, &c, 1) == 1;
	    }
	if (port == epp_control_reg_addr) {
	    // Control register.
	    trace && printf("control write %x\n",c);
	    return ioctl(par_fd, PPWCONTROL, &c) == 0;
	    }
	if (port == epp_status_reg_addr) {
	    printf("Error: cannot write the status register.\n");
	    // Can't write the status register!  We read it only.
	    return 0;
	    }
	return 0;
	}

    override int inp(unsigned port) {
        check_setup();

        unsigned char c = 0;
	if (port == epp_addr) {
	    // Data read.
	    trace && printf("!read from data\n");
	    if (read(par_fd, &c, 1) == 1) goto OK;
	    else printf("Parallel port data read failed.\n");
	    return 0;
	    }
	if (port == epp_control_reg_addr) {
	    printf("Error: cannot read the control register.\n");
	    return 0;
	    }
	if (port == epp_status_reg_addr) {
	    trace && printf("!read from status\n");
	    if (ioctl(par_fd, PPRSTATUS, &c) == 0) goto OK;
	    else printf("Parallel port status read failed.\n");
	    return 0;
	    }
	return 0;

      OK: ;
	dbg_port && printInFile("%02x <= port 0x%x\n",c,port);
	trace && printf("data read %x\n",c);
	return c;
	}

    override void helpful_messages() { }
    };

#else

#include "gpio.h"

struct Linux_port : Port {

    bool claimed;
    int par_fd;
    static const bool trace = 0;
    bool error;
    override int OK() { return !error; }
    int gpio_version;

    Linux_port() { claimed = FALSE; par_fd = -1; error = FALSE; }

    NOINLINE int check_setup() {
	//! 1. possibly switch to ecp using /dev/parpart
	//  2. initialize port address.
	if (error) return FALSE;
        if (claimed == FALSE) {
	    par_fd = open("/dev/gpio",O_RDWR);
	    if (par_fd == -1) {
	        perror("open");
		printf("Did you forget to install gpio.o?  Read gpio.txt.\n");
		error = TRUE;
		return 0;
		}
	    claimed = TRUE;
	    gpio_version = ioctl(par_fd, GPIO_IOC_VERSION);
	    }
	return 1;
        }

    override int setup(unsigned port_num, int num) {
        if (!check_setup()) return ERR_SET_PORT;

	epp_addr = port_num;
	epp_status_reg_addr = port_num + STATUS_REG_OFFSET;
	epp_control_reg_addr = port_num + CONTROL_REG_OFFSET;
	ioctl(par_fd, GPIO_IOC_SET_PORT_BASE, epp_addr);

	return 1;
	}

    override int shutdown() {
        if (!check_setup()) return ERR_SET_PORT;
	if (claimed) {
	    claimed = FALSE;
	    close(par_fd);
	    }
	return 1;
	}

    override int outp(unsigned port, int value) {
        if (!check_setup()) return 0;

	dbg_port && printInFile("%02x =>   port 0x%x\n",value,port);

	char c[2] = {port-epp_addr, value};
	GPIO_ioctl x = { 2, c, 0,0 };
	ioctl(par_fd, GPIO_IOC_DO_IO, &x);
	return 0;
	}

    override int inp(unsigned port) {
        if (!check_setup()) return 0;

	static const int READ_BIT = 0x80;
	char c[2] = {port-epp_addr+READ_BIT, 0};
	unsigned char output[2] = {0,0x99};
	GPIO_ioctl x = { 2, c, 1, (char*)output };
	if (output[1] != 0x99) printf("DRIVER OVERRUN!\n");
	ioctl(par_fd, GPIO_IOC_DO_IO, &x);
	dbg_port && printInFile("%02x <= port 0x%x\n",c,port);
	trace && printf("data read %x\n",c);
	return (unsigned char)output[0];
	}

    override void helpful_messages() { 
	// Version must be >= 100 (1.00).
	if (gverbose) printf("Linux GPIO driver version is %d.\n",gpio_version);
	}
    };
#endif
Port *port = new Linux_port();

unsigned int win32OutpArray(GPIO_ARRAY_STRUCT *A, unsigned long size) {
    // Fashion the code to invoke the Linux gpio device driver.
    if (!port->OK()) return 0;
    char buf[16384];
    int entries = size/sizeof(A[0]);
    for (int i = 0, j = 0; i < entries; i++,j+=2) {
	if (A[i].port == 0) break;	// Old NT driver needed terminator.
	buf[j] = A[i].port-epp_addr;
	buf[j+1] = A[i].data;
	}
    0 && printf("!use gpio array for entries %d ioctl buf out is %d\n",entries,j);
    GPIO_ioctl x = { j, buf, 0, 0 };
    ioctl(((Linux_port*)port)->par_fd, GPIO_IOC_DO_IO, &x);
    return entries;
    }

#endif

#if _NOPORT
// Temporarily disable this code.
#define BAD printf("SHOULD NOT BE HERE line %d port.cpp\n",__LINE__);
#define BAD0 printf("SHOULD NOT BE HERE line %d port.cpp\n",__LINE__); return 0;
void pioInitInstr() { BAD }
void pioInstr(unsigned char code) { BAD }
void pioConst(unsigned char data) { BAD }
void pioOutp(unsigned char port) { BAD }
void pioInp(unsigned char port) { BAD }
int pioSize() { BAD return 1; }
int gpioEval_(PioInstrStream *stream, unsigned char *inBuf, int *inLen) { BAD0 }
int gpioEval(unsigned char *inBuf, int *inLen) { BAD0 }
#endif
