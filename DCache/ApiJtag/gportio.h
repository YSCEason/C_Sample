//********************** GPORTIO.H *********************************

/*** Copyright 1998 MetaWare Incoporated ***/

#ifndef __GPORTIO_H__
#define __GPORTIO_H__ 1
// FOR THE DOCUMENTATION OF THIS INTERFACE SEE THE BOTTOM OF THIS
// FILE.

#ifdef __cplusplus
extern "C" {
#endif

#include "pioeval.h"

int win32PortIOStatus();

int isHostNT();

int gpioInit();
int gpioFini();

unsigned int win32Inp(unsigned short port);
unsigned short win32Inpw(unsigned short port);
unsigned long win32Inpd(unsigned short port);

unsigned int win32Outp(unsigned short port, unsigned int dataValue);
unsigned short win32Outpw(unsigned short port, unsigned short dataValue);
unsigned long win32Outpd(unsigned short port, unsigned long dataValue);

int gpioEval_(PioInstrStream *stream, unsigned char *inBuf, int *inLen);
int gpioEval(unsigned char *inBuf, int *inLen);
int gpioMapPort(unsigned short base, int count);

// Functions added in version 2:
// THESE FUNCTIONS ARE NOT YET DOCUMENTED.

unsigned int win32OutpBytes(unsigned short port, unsigned long number_bytes,
   unsigned char * data_ptr);

typedef struct gpio_array_struct {
    unsigned short port;
    unsigned char  data;
    unsigned char  reserved;
    } GPIO_ARRAY_STRUCT;

unsigned int win32OutpArray(GPIO_ARRAY_STRUCT *, unsigned long size);

/* Here are functions specific to communication with an ARC;
   these functions group a series of commands together and execute
   them all in the driver (if applicable).
   THESE FUNCTIONS RETURN 0 IF SUCCESS and non-zero if FAILURE.
   This is different from other conventions, so be forewarned.
 */
unsigned int win32ReadArcByte(unsigned short port, unsigned char *dataValue);
unsigned int win32ReadArcLong(unsigned short port, unsigned long *dataValue);
unsigned int win32ArcMemWrite(unsigned char *src_ptr, unsigned long destination, 
   long count);
unsigned int win32ArcMemRead(unsigned char *dest_ptr, unsigned long source,
   long count);
unsigned int win32ArcRegWrite(unsigned int reg, unsigned long value);
unsigned int win32ArcRegRead(unsigned int reg, unsigned long *value);
/* End of functions specific to communication with an ARC. */

// Return the version of the driver or this library: 
// version[0] <- MAJOR
// version[1] <- MINOR
unsigned int gportioVersion(char *version);

#ifdef __cplusplus
};
#endif

#endif

/********************************************************************
              *** Copyright 1998 MetaWare Incoporated ***

The MetaWare Generalized Port IO driver for NT (gportio) is a
programmable driver designed to allow x86 port IO on NT.  Unlike Windows
9x, in which one can directly use the x86 IO instructions such as inb
and outb, IO operations on NT are privleged.  One must write a driver to
access the device.

MetaWare uses the gportio driver to communicate with ARC hardware
connected to a parallel port on NT, and supports it only for this
purpose.  However, the driver is not specifically to a parallel port,
and thus may work with a serial port (although we have not tried this).

The driver also supports port access on Windows 9x.  It detects the
presence of NT versus Windows 9x.  On the latter OS the driver simply
writes directly to the port rather than going through the OS.

The documetation of the driver's interface is included here at
Argonaut's request for those who wish to use it to communicate to other
ARC boards through arbitrary x86 ports.


On the x86 you connect to ports by, among others, the instructions:
  inb     inw     ind
  outb    outw    outd
The gportio driver allows you to issue those instructions on NT.  

Because the overhead of issuing switching in and out of "driver mode" on
NT is horribly high, you can also program the driver to issue a sequence
of IO instructions, including the abilitiy to loop with timeout until a
certain incoming result obtains.

Before using the driver, call this function:

    int gpioInit();

This function returns 1 iff it succeeds in opening the device driver.
On W9x it does nothing and returns 1.  You can determine whether you're
running on NT by calling

    int isHostNT();

which returns 1 iff you are running on NT.  

After initialization, you need to map in the port address range you will
be using.  Call:

    int gpioMapPort(unsigned short base, int count);

where you supply a base port address and a count (at least 1) of the
addresses you will be using starting at the base.  For the ARC, we use

    gpioMapPort(0x3f8,4);

The return result is 1 iff the map succeeded.  In what follows, the
value specified as the first argument to gpioMapPort shall be referred
to as "PORTBASE".

Subsquent to this you can now read and write individual bytes, half-words,
and words, with the functions:

    Read  Write   Size
    ----  -----   ----
    win32Inp    win32Outp       1
    win32Inpw win32Outpw  2
    win32Inpd win32Outpd  4

The functions have the following prototypes:  

    unsigned int   win32Inp(unsigned short port);
    unsigned short win32Inpw(unsigned short port);
    unsigned long  win32Inpd(unsigned short port);

    unsigned int   win32Outp(unsigned short port, 
      unsigned int dataValue);
    unsigned short win32Outpw(unsigned short port, 
      unsigned short dataValue);
    unsigned long  win32Outpd(unsigned short port, 
      unsigned long dataValue);

You may use these functions directly for small data sets, or if you are
running on W9x.  However, because NT's transition from driver to user
program mode is so expensive, these functions do not suffice for large
amounts of data.  For the latter, we provide a method of "programming"
the driver by giving it a "program" of input and output instructions
along with loop and test instructions.  The entire program is executed
at once in the driver before returning back to user mode.  We'll call
the program the "PIO" program below.

The general forms of programming the driver are as follows:

(1)     char outbuf[size]; 
  pioInitInstr(); // Initialize
        ... function calls to deliver the PIO program  ...
  gpioEval(outbuf, size); // Execute program

and

(2)     char outbuf[size]; 
        PioInstrStream stream;  // local instr stream buffer.
  pioInitInstr_(stream);  // Initialize
        ... function calls to deliver the PIO program  ...
  gpioEval_(&stream, outbuf, size);  // Execute program

The output buffer need be supplied only if the PIO program contains
certain instructions that read or write data to the output buffer. The
buffer pointer starts out at the beginning of the buffer.  Writes
increment the pointer and reads decrement it.

In method (2), you supply a stream object in which the driver places the
PIO program.  You must not place more than MAX_PIO_INSTR_BUF (4096)
instructions in the buffer.  The PIO program executed is the one in the
stream buffer.  Method (2) allows you to assemble multiple PIO programs
simultaneously.  Method (1) uses a static PioInstrStream object within
the driver; therefore you can create only one PIO program at a time.
We'll say that method (1) uses the "static stream buffer".

A PIO program consists of instructions that operate on a stack machine.
The stack is contains MAX_PIO_EVAL_STACK (128) one-byte values, so you
shouldn't create a program that uses more than that many stack
locations.

You place PIO instructions in the buffer with the following functions:

    extern void pioConst(unsigned char data);
    
  This places a PIO_PUSH instruction with operand data
  on the stack.  This takes two instructions positions
  to store.

    extern void pioOutp(unsigned char port);
    
  This pops the top value from the stack and sends
  it to the IO port PORTBASE+port.  This takes two
  instructions position to sture.  It uses the opcode
  PIO_OUTP.
    
    extern void pioInp(unsigned char port);
  
  This reads a byte from PORTBASE+port and writes it to
  the outbuf buffer, incrementing the buffer pointer.
  Thus if you do any reading from an IO port you must
  supply a non-zero outbuf parameter.
    
  This takes two instructions position to sture.
  It uses the opcode PIO_INP.
          
    extern void pioInstr(unsigned char code);
    
  This places a single opcode in the PIO program, and so
  takes just one instruction position to store.
    
  The possible values for code are:
      PIO_BIT_AND
    Replace the top two stack values by their bitwise and.
      PIO_BIT_XOR
    Replace the top two stack values by their bitwise xor.
      PIO_BIT_OR
    Replace the top two stack values by their bitwise or.
      PIO_EQ
    Replace the top two stack values by 1 if the top two
    values are equal, otherwise 0.
      PIO_NE
    Replace the top two stack values by 1 if the top two
    values are not equal, otherwise 0.
      PIO_I_TRANSFER
    Read a value from the outbuf buffer, decrementing the
    pointer, and place that value on the stack.  This is
    useful if you have read a value from an IO port that you
    do not wish to return to the client, but instead want to
    test as part of a loop condition.
        
      PIO_LOOP_BEGIN
          Initiate a loop.  The byte at the top of the stack is
    popped and sets the "loop count": the maximum number of
    iterations through the loop.  That is, the loop will
    execute that number of of times, unless an early break
    occurs.  Thus you must precede PIO_LOOP_BEGIN with a
    push of a constant or some other calculation of the loop
    iteration maximum.  The iteration maximum might
    profitably be a timeout value for reading data from a
    port, where you loop repeatedly trying to read data back
    from a device.
    
    When a subsequent PIO_LOOP_END instruction occurs, the
    loop count for this loop is decremented.  If it is zero,
    execution continues with the instruction following the
    PIO_LOOP_END.  If it is non-zero, execution continues
    with the first instruction of the loop (again).
          
          The PIO evaluator supports a maximum loop nesting of
    MAX_LOOP_STACK (5).  
        
      PIO_LOOP_END
          See discussion of PIO_LOOP_BEGIN.
      
      PIO_BREAK_ON_TRUE
        Pop the top value on the stack.  If this value is
    non-zero, terminate the current loop, and continue
    execution at the first instruction following this loop's
    PIO_LOOP_END instruction.
      
      PIO_BREAK_ON_FALSE
        Pop the top value on the stack.  If this value is 0,
    terminate the current loop, and continue execution at
    the first instruction following this loop's PIO_LOOP_END
    instruction.
      
      PIO_POP
          Discard the top value of the stack.   
  

The four functions listed above, viz.,

    extern void pioConst(unsigned char data);
    extern void pioOutp(unsigned char port);
    extern void pioInp(unsigned char port);
    extern void pioInstr(unsigned char code);

place instructions in the static stream buffer. If you are using method
(2) of assembling a PIO program, you must instead use the macros

    ioInstr_(stream, c) 
    ioConst_(stream, d) 
    ioOutp_(stream, port) 
    ioInp_(stream, port) 
    
to place the instructions in your buffer.

You can use the function below to see how many instruction positions you
have consumed in a stream buffer:

    extern int pioSize(); 

This is useful when you are preparing a long, repeated sequence of
instructions, and you don't want to carefully count instruction
positions.  Merely place your PIO instruction sequence once, call
pioSize() to get the number of instructions you placed, and divide that
into MAX_PIO_INSTR_BUF to determine how many copies of your sequence you
can place in the instruction buffer.  Use macro

    pioSize_(stream)

when you are placing the instructions in your own stream.  For example:

    pioInitInstr_(stream);
    ... place a single copy of your instructions here ...
    int size = pioSize_(stream);
    int repeat_count_possible = MAX_PIO_INSTR_BUF/size;
    for (int i = 0; i < repeat_count_possible; i++) {
      ... place your instructions here ...
      }


Header files.
-------------
To use gportio, you must include the header files

  gportio.h
  pioeval.h -- if you want to program the driver

These files are included in the debugger's sc/inc/ directory. 


Linking.
--------
To link a program using the driver, use the import library gportio.lib.
This is shipped with the debugger in the bin/ directory.


Examples.
---------
These examples are from the debugger's use of gportio to communicate
with the ARC board.

Here is a C++ example of a nested loop computation from the debugger
that reads four bytes from the parallel port.  It uses the static stream
buffer.

    void read_4bytes(unsigned long &value) {
  unsigned char buf[4];
  int len = 4;

  pioInitInstr();

  pioConst(4);
  pioInstr(PIO_LOOP_BEGIN);

  pioConst((C_BI|C_SS1|C_SS0|C_CNT|C_STR)^C_XOR);
  pioOutp (CONTROL_REG_OFFSET);

  pioConst((C_BI|C_SS1|C_SS0|C_CNT)^C_XOR);
  pioOutp (CONTROL_REG_OFFSET);

  pioConst(TIMEOUT);
  pioInstr(PIO_LOOP_BEGIN);
  pioInp  (STATUS_REG_OFFSET);
  pioInstr(PIO_I_TRANSFER);
  pioConst(S_XOR);
  pioInstr(PIO_BIT_XOR);
  pioConst(S_ACK);
  pioInstr(PIO_BIT_AND);
  pioConst(0);
  pioInstr(PIO_EQ);
  pioInstr(PIO_BREAK_ON_TRUE);
  pioInstr(PIO_LOOP_END);

  pioInp(0);
    
  pioConst((C_BI|C_SS1|C_SS0|C_CNT|C_STR)^C_XOR);
  pioOutp (CONTROL_REG_OFFSET);

  pioInstr(PIO_LOOP_END);

  gpioEval(buf,&len);
  value =  buf[0];
  value |= buf[1] << 8;
  value |= buf[2] << 16;
  value |= buf[3] << 24;
  }

The outer loop executes four times (hopefully), each time reading a byte
of input.  The inner loop executes at most TIMEOUT times, waiting for
the port to respond.  A computation is done within the inner loop to
determine whether the device has responded; if it has, the program
breaks out of the loop.


Here is a non-loop example from the debugger of writing four bytes to
the parallel port by repeating a sequence of instructions four times:

    void write_4bytes(unsigned long value, uchar control) {
  pioInitInstr();
  unsigned char oval = C_SS1 | (control*C_CNT);
  for (int i = 0; i<4; i++) {
      uchar val = (value >> (i*8)) & 0xff;
      pioConst((oval|C_STR) ^ C_XOR); 
      pioOutp(CONTROL_REG_OFFSET); 
      pioConst(oval ^ C_XOR); 
      pioOutp(CONTROL_REG_OFFSET); 
      pioConst(val); pioOutp(0); 
      pioConst(val); pioOutp(0); 
      pioConst((C_SS1|(control*C_CNT)|C_STR)^C_XOR); pioOutp(CONTROL_REG_OFFSET);
      }
  gpioEval(0,0);
  }

********************************************************************/
 

