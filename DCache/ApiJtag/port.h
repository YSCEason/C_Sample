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
* File name:          port.h
* 
* Author:             Tom Pennello
*
* Creation Date:      15 Oct 2000
*
* Description:
*
*       Header file for common routines used by both jtag=> parallel 
*       and parallel alone(port.cpp).
* 
*
*  History:
*
*   Version    Date         Author           Description
*
*   1.0        15/10/00     Tom Pennello     File created. 
*
*******************************************************************************/

// Common routines used by both jtag=> parallel and parallel alone.
extern unsigned epp_addr;
extern unsigned epp_control_reg_addr;
extern unsigned epp_status_reg_addr;
extern unsigned char driver_major_version;
extern unsigned char use_extended_driver;
extern unsigned slow_clock;
extern void slow_clock_loop();
extern char dbg_port;

struct gpio_array_struct;

struct Port {
    virtual int setup(unsigned port_num, int num) = 0;
    virtual int shutdown() = 0;
    virtual int outp(unsigned port, int value) = 0;
    virtual int inp(unsigned port) = 0;
    virtual void helpful_messages() = 0;
    virtual int OK() { return 1; }
    virtual unsigned int output_array(gpio_array_struct *, unsigned long size);
    };
extern Port *port;

//Port error codes
#define ERR_BUSY -5
#define ERR_CHECK_FAIL -6
#define ERR_COMM -7
#define ERR_SET_PORT -8
#define ERR_SHUT_PORT -9
#define ERR_REG_OUT_OF_RANGE -11
#define ERR_PORT_ERROR -12

// Port offsets.
#define STATUS_REG_OFFSET  1
#define CONTROL_REG_OFFSET 2
