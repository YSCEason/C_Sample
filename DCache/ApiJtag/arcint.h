#ifndef ARC_INT_H
#define ARC_INT_H
/*======================================================================
		    MetaWare ARC Processor Interface

 (C) Copyright 1997-2002;  MetaWare Incorporated;  Santa Cruz, CA 95060


      Please consult the end of the file for extensive documentation.

		      Search for "DOCUMENTATION".
======================================================================*/

// (If you are using this in C, and your C compiler is so old as to not
// support // comments, then just surround these comments with the
// standard C comment convention)
enum {ARCINT_BASE_VERSION = 1,
      ARCINT_BREAK_WATCHPOINT_VERSION = 2,
      ARCINT_SPECIFIC_DISPLAY_VERSION = 3,
      ARCINT_FILL_MEM_VERSION = 4,
      // Trace adds support for getting the last N PC values
      // as in an ICE trace buffer, which MetaWare's simulator maintains
      // anyway.
      ARCINT_TRACE_VERSION = 5,
      };

struct Register_display;

#define ARC_FEATURE_fill_memory 1
#define ARC_FEATURE_instruction_trace 2
#define ARC_FEATURE_data_exchange 4
#define ARC_FEATURE_banked_reg 8
#define ARC_FEATURE_new_bp_cookie 16
#define ARC_FEATURE_at_breakpoint_cookie 32

#define cntxt_data_reference 2	// set if the request is a load/store
				// (This agrees with simext.h)
#define cntxt_cacheless 4	// set if the request is cacheless

// The first 1024 data exchange opcodes are system-defined.
// This opcode is implemented by any transport facility and tells the
// maximum amount allowed to be transferred by the data exchange facility.
#define DATA_XCHG_FIRST_USER_OPCODE 1024
#define DATA_XCHG_UNRECOGNIZED_OPCODE 0xffffffff

// Warning: there seem to have been some versions of this interface
// with ARC_REG_TYPE as unsigned long.  The official type is int.
// If you implement a C++ version of this DLL and use Microsoft C++,
// it *will not* tell you that you have the wrong type in your definition
// of, say, read_banked_reg, and you won't override the base class 
// function; your defined function will be ignored silently.
// If you have source code that uses a different type for the register
// you can change this define, or, preferably, change your code to 
// use ARC_REG_TYPE everywhere.
#define ARC_REG_TYPE int

#define ARC_functions(func,fbdy,OBJ0,OBJ) \
    func(int, version, (OBJ0) )				\
    func(const char *, id, (OBJ0) )			\
    fbdy(void, destroy, (OBJ0), return; )		\
    fbdy(const char *, additional_possibilities, (OBJ0), return 0;)\
    fbdy(void*, additional_information, (OBJ unsigned), return 0;) 	\
    func(int, prepare_for_new_program, (OBJ int will_download))		\
    func(int, process_property, (OBJ const char *key, const char *value)) \
							\
    fbdy(int, is_simulator, (OBJ0), return 0; )		\
    fbdy(int, step, (OBJ0), return 0; ) 		\
    fbdy(int, run,  (OBJ0), return 0; ) 		\
    func(int, read_memory, (OBJ unsigned long adr, void *, \
		unsigned long amount, int context) )	\
    func(int, write_memory, (OBJ unsigned long adr, void *, \
		unsigned long amount, int context) ) \
    func(int, read_reg, (OBJ ARC_REG_TYPE r, unsigned long *value)) 	\
    func(int, write_reg, (OBJ ARC_REG_TYPE r, unsigned long value)) 	\
    fbdy(unsigned, memory_size, (OBJ0), return 0;)		\
    fbdy(int, set_memory_size, (OBJ unsigned S), return 0;) 	\
    /* Version 1 interface ends here. */		\
    /* Version 2 interface includes watchpoint fcns. */ \
    fbdy(int, set_reg_watchpoint, (OBJ ARC_REG_TYPE r, int length), return 0; )    \
    fbdy(int, remove_reg_watchpoint, (OBJ ARC_REG_TYPE r, int length), return 0; ) \
    fbdy(int, set_mem_watchpoint, (OBJ unsigned long addr, int length), return 0; ) \
    fbdy(int, remove_mem_watchpoint, (OBJ unsigned long addr, int length), return 0; ) \
    fbdy(int, stopped_at_watchpoint, (OBJ0), return 0; )\
    fbdy(int, stopped_at_exception, (OBJ0), return 0;)  \
    fbdy(int, set_breakpoint, (OBJ unsigned address, void*cookie), return 0;)\
    fbdy(int, remove_breakpoint, 			\
		(OBJ unsigned address, void*cookie), return 0;)\
    fbdy(int, retrieve_breakpoint_code, (OBJ unsigned address, 	\
		char *dest, unsigned len, void *cookie_buf), return 0;)\
    fbdy(int, breakpoint_cookie_len, (OBJ0), return 0;)	\
    fbdy(int, at_breakpoint, (OBJ0), return 0;)		\
    /* Version 2 interface ends here. */  		\
    fbdy(int, define_displays, (OBJ struct Register_display *rd),return 0;) \
    /* Version 3 interface ends here. */  		\
    fbdy(int, fill_memory, (OBJ unsigned long adr, void *buf, \
	unsigned long amount, unsigned repeat_count, int context), \
	return 0;)	\
    /* Version 4 interface ends here. */  		\
    fbdy(int, instruction_trace_count, (OBJ0), return 0;)	\
    fbdy(void, get_instruction_traces, (OBJ unsigned *traces), return;) \
    fbdy(void, receive_callback, (OBJ ARC_callback*), return;)	\
    fbdy(int, supports_feature, (OBJ0), return 0;)	\
    fbdy(unsigned, data_exchange,			\
    	(OBJ unsigned what_to_do, unsigned caller_endian, \
    	unsigned send_amount, void *send_buf,		\
        unsigned max_receive_amount, void *receive_buf), return 0;)	\
    fbdy(int, in_same_process_as_debugger, (OBJ0), return 1; )	\
    fbdy(unsigned, max_data_exchange_transfer, (OBJ0), return 0xfffffffe; )	\
    fbdy(int, read_banked_reg, \
    	(OBJ int bank, ARC_REG_TYPE r, unsigned long *value), return 0;) \
    fbdy(int, write_banked_reg, \
    	(OBJ int bank, ARC_REG_TYPE r, unsigned long *value), return 0;) \
    /* Version 5 interface ends here. */                \



// Registers 0..63 are core; 64.. are aux.
enum {
    reg_LP_COUNT = 60,
    AUX_BASE = 64,
    reg_STATUS=AUX_BASE, reg_SEMAPHORE, reg_LP_START, reg_LP_END,
    reg_IDENTITY, reg_DEBUG,
    // ARCompact registers only are designated with reg_AC.
    reg_AC_PC,		    // ARCompact 32-bit PC register.
    reg_ADCR=AUX_BASE+0x7,  // Actionpoint Data Comparison Register
    reg_APCR,   	    // Actionpoint Program Counter Register
    reg_ACR, 		    // Actionpoint Control Register
    reg_AC_STATUS32,	    // ARCompact status register
    reg_AC_STATUS32_L1,	    // ARCompact status register for int level 1
    reg_AC_STATUS32_L2,	    // ARCompact status register for int level 2
    reg_ICACHE=AUX_BASE+0x10, // Invalidate cache
    reg_AP_build=AUX_BASE+0x76,
    reg_MADI_build= AUX_BASE+0x73, // MADI Build Configuration Register

    // For read_banked_reg and write_banked_reg:
    reg_bank_core = 0,
    reg_bank_aux  = 1,
    reg_bank_madi = 2
    };

// MADI registers.  To read/write these, you must use read/write_banked_regs.
enum {
    reg_MCR = 0,  	// MADI control register.
    reg_MER=  1,	// These are unused in ARC's code.
    reg_MID=  2,
    reg_MAE=  9,
    };


#if OEM_USE_OF_DEBUGGER_HEADER_FILES
#include "crint.h"
#else
#include "../../common/crint.h"
#endif
// This invokes the macro in crint.h to create either C or C++ declarations
// for the ARC interface.
typedef struct ARC_callback ARC_callback;
create_interface_allow_bodies(ARC)

typedef enum {
    MAP_INVALID = 0, 	// There was no entry in any valid map, and there
			// exists one or more maps.
    MAP_VALID = 1,	// This is a valid entry from a  map.
    MAP_THERE_IS_NO_MAP = 2,	// There are NO maps defined so we have
			// computed default values (typically: r/w,
			// 4-wide, noverify).
    } Map_code;

struct Memory_info {
    // See Map_code above for interpreting the value.
    // Ensure a single byte for mapentry, despite different enum defaults
    // for various compilers.
    unsigned char /*Map_code*/ mapentry; // is this memory address in our map?
    // None of these fields have anything reliable if mapentry == 0.
    char width;		// typically 2 or 4.  1?
    char readonly;	// Is the memory read-only? (must use HW bkpts)
    char verify;	// request to verify writes.
    };

#include <stdarg.h>

#define ARC_endian_functions(func,OBJ0,OBJ)		\
    func(int, version, (OBJ0) )				\
    /* Get the endian of the computer executing this function.  \
       Bit 0 is 1 iff the endian is "little". */	\
    func(int, get_host_endian, (OBJ0))			\
    /* If get_host_endian() != "endian", 			\
       these two functions swap a 32-bit or 16-bit value. */	\
    func(unsigned, convert_long, (OBJ int endian, unsigned value)) \
    func(unsigned, convert_short, (OBJ int endian, unsigned value)) \
    /* Version 1 interface ends here. */		\

create_interface(ARC_endian)

struct Debugger_access;

#define ARC_callback_functions(func,OBJ0,OBJ) \
    func(int, version, (OBJ0) )				\
    /* This is the printf in the client.  Use this 	\
       if you want to avoid having a different printf 	\
       in your DLL implementation of the interface.	\
     */							\
    func(int, printf, (OBJ const char *format, ...)) 	\
    /* If you need to know on a memory read/write, you can ask \
       what the memory map says about the memory at 	\
       the given address.  If mapentry is 0, there is 	\
       information available.				\
     */							\
    func(void, meminfo, (OBJ unsigned addr, struct Memory_info *m))	\
    /* Version 1 interface ends here. */		\
    func(int, vprintf, (OBJ const char *format, va_list ap)) 	\
    /* Return whether the client has a "verbose" switch on. */ \
    func(int, get_verbose, (OBJ0))			\
    /* Version 2 interface ends here. */		\
    func(void, sleep, (OBJ unsigned milliseconds))	\
    func(int, in_same_process_as_debugger, (OBJ0))	\
    /* Get endian conversion object. */			\
    func(struct ARC_endian*, get_endian, (OBJ0))	\
    /* Private for debugger use only. */		\
    func(void *, get_tcp_factory, (OBJ0))		\
    func(void *, get_process, (OBJ0))			\
    /* Version 3 interface ends here. */		\
    func(struct Debugger_access *, get_debugger_access, (OBJ0))\
    /* Version 4 interface ends here. */		\

create_interface(ARC_callback)

// This function returns an ARC object.  For a DLL, you must export
// a function by this name.
// On V.4, how can we have multiple DLLs each with the same initialization
// function?  Probably can't; must name the functions differently.
extern ARC *get_ARC_LE_simulator(),
	   *get_ARC_BE_simulator(),
	   *get_ARC_hardware();

#if __HIGHC__ && __cplusplus
class System; class Delayed_memory;
// The debugger obtains this additional information if
// additional_possibilities() has the string MetaWare in it.
struct sim_disarc {
    virtual char *disarc(unsigned addr, unsigned long *memory) = 0;
    };
struct ARC_simext;
struct Addr_counter;
struct Trace_history;
struct Additional_metaware_simulator_communication {
    virtual void set_disarc(sim_disarc *dp) = 0;
    virtual void set_system(System*) = 0;
    virtual void set_simulator_extensions(ARC_simext **) = 0;
    virtual void set_memory(Delayed_memory *) = 0;
    virtual void new_simulator_extension_found(ARC_simext *) = 0;
    virtual void each_addr_counter() -> (Addr_counter*) = 0;
    virtual void set_early_hostlink(int) = 0;
    virtual Trace_history *get_trace_history() = 0;
    };
struct Additional_metaware_hardware_communication {
    virtual void set_system(System*) = 0;
    };
#endif

/*======================================================================
DOCUMENTATION
		    MetaWare ARC Processor Interface

This is an object-oriented interface to an ARC processor.  The debugger
communicates to the ARC processor solely through this interface. This
interface conceals the nature of the communications to the processor.

If you implement this interface in a DLL, you can have the debugger
debug program "foo" with it by specifying the command-line option
-DLL=dllname.  "dllname" is the name of your DLL.

If you implement this DLL in C++, any function in the ARC_functions
macro that uses the "fbdy" macro invocation (fbdy = func_with_body)
has a default body in the base class, and you need not implement that
function unless you intend to make use of the functionality.  Thus a C++
implementation is more convenient than a C implementation; the C
implementation requires implementing all the functions that may get
called, even if the return result is 0.

If you are having trouble with your implementation of this interface --
say, the debugger doesn't seem to be working right -- you can trace all
calls to your functions in the interface by specifying

       -on=trace_board_interface

You can also specify

       -on=trace_board_interface2

This is a bit more verbose, but prints a separate message when the call
to your interface returns to the debugger; this is useful for detecting
if your interface is hanging. Additionally, if you are NOT doing
multi-arc debugging, you can have the debugger implement the step() and
run() functions in the interface, by saying:

       -on=minimal_interface

Future changes for the interface include permitting direct writing of
the ARC MADI registers so that the debugger could control MADI as well;
but at this time, the MADI registers are not accessible via the
interface.

The interface is a pointer to a table of function pointers. The
double-indirection allows the pointer to point to an object. Because
each function receives the object pointer as the first value, the
function can access the object's private data to do its work. The first
entry in the table of pointers doesn't "count" -- leave as 0. This is
the MetaWare C++ convention: the first entry in a virtual function table
is reserved for RTTI information.

Within the macro "functions" are all the functions exported in the
interface.  The reason for the macro is so that a C as well as a C++
version of the interface can be generated.  MetaWare uses the C++
interface in the client and its ARC simulator, but you can implement the
functions in C.

For C++, the macros generate something like this:

    struct ARC {
	virtual int version();
	virtual int read_reg(ARC_REG_TYPE r, unsigned long *value);
	...
	};

and for C:

    struct ARC;	// Opaque object.
    struct ARC_functab {
	int (*version)(ARC*);
	int (*read_reg)(ARC*, ARC_REG_TYPE r, unsigned long *value);
	};

The debugger acquires a pointer to the interface either by directly
calling known entities linked within the debugger
(get_ARC_LE_simulator() and get_ARC_hardware(), or by calling a function
get_ARC_interface in a DLL that is loaded dynamically.  You specify the
name of the DLL to the debugger; it loads it, looks up the address of
the function get_ARC_interface, and calls the function.  A non-zero
result is assumed to be one of the interface pointers described above. A
zero result is taken to be a failure of the DLL. A sample construction
of a dummy DLL in C is shown in the Appendix Sample Implementation
below.

If you use C++, please read about the dangers of silent errors in
overriding base class functions in file crint.h; look for CHECK_OVERRIDES.
We recommend you do as follows:
	#define IMPNAME your_implementation_class
	CHECK_OVERRIDES(ARC)
to ensure that you have properly overriden base class functions.

The functions in the interface are described below.  For a C
implementation, each function will have an additional first parameter
which is the interface pointer passed back to the function.  In C++,
this first parameter is the hidden "this" parameter.

version -- Interface version

    int version()

    The version starts out at ARCINT_BASE_VERSION (1).  The version
    number is increased as additional functions are appended to this
    specification; for backwards compatibility, functions are never
    deleted.

    YOU ARE NOT FREE TO RETURN ANY NUMBER HERE!  The debugger defines
    the interface versions.  You return the number here that reflects
    which version you have implemented.  See the ARCINT_...VERSION enums
    above for the versions currently supported.

id -- Name of interface for reporting and error messages

    const char *id();

    Returns interface ID: implementor, manufacturer, whatever, etc.
    Occasionally used in printing messages.

destroy	-- destroy your interface DLL

    void destroy();

    Your object is being destroyed.  You may release any allocated
    resources. This is equivalent to a C++ destructor.   After calling
    this, the client will never use the object again.

is_simulator	-- tell if you are a simulator, and your characteristics

    int is_simulator();

    This function tells the debugger whether the ARC processor is
    simulated. Generally, a simulator is distinguished from hardware in
    that hardware can continue executing while the debugger can do
    something else.  While running a simulator the debugger can do
    nothing else; the simulator runs in the same process as the debugger
    and so the debugger has to share CPU resources with the simulator.
    To achieve this, the debugger generally will repeatedly single-step
    a simulator, rather than calling "run" and risk the simulator never
    returning (and hanging the debugger).

    Return value:
	bit 0 is on if this is a simulator.
	bit 1 is on if this simulator is "run-capable".

    is_simulator() returns 1 in the bottom-most bit iff the
    implementation is a simulator.  If it is, and unless the simulator
    is run-capable, the debugger repeatedly calls step() rather than
    calls run(), so that it is possible to halt the execution of a
    program by tapping the keyboard.  The debugger checks for the
    keyboard tap every so often during instruction stepping.

    If the the simulator is run-capable (bit 1), you are indicating that
    the simulator that can implement run for a fixed number of cycles.
    For a run-capable simulator, the debugger will call run rather than
    repeatedly stepping.  This is important for cycle-accurate
    simulators.  See run() for more details.

step	-- step an instruction

    int step()

    Effects a single-step of the machine; for the hardware, initiates
    the single step; for the simulator, returns when the single-step is
    completed.  Returns 1 if succeeded, 0 if failed.

    Stepping can take two forms.  If you have received the property
    cycle_step with value 1 (process_property("cycle_step","1"),
    you should cycle step the ARC; otherwise you should instruction-step
    it.  I.e., cycle_step is by default off, or 0.

    Cycle step means to move the PC to the next instruction; it doesn't
    mean "advance the ARC by one cycle".  As part of your job of cycle
    stepping, you need to cycle over a LIMM in an instruction.

    For ARCompact, step() is *not* called when cycle_step is 1.  This
    is because the complexity of decoding instructions to determine
    whether a LIMM is present is high, so we leave that to the debugger.

    Note further that if you have specified -on=minimal_interface to
    the debugger, it will never call step(); it will always write
    the debug register to achieve the effect.  Thus you can just return 0
    for step() if you specify -on=minimal_interface.

run	-- initiate execution

       int run()

    Starts running the CPU.  Returns 1 if succeeded, 0 if failed. The
    target should be started running and the function must immediately
    return without waiting for the target to stop.

    If this is a run-capable simulator, before calling run(), the
    debugger will call process_property("run_cycles",NNN) where NNN is a
    string denoting the number of cycles to run.  The run function
    should run no more than that number of cycles and return.  You may
    choose to run fewer cycles if the program halts due to a breakpoint,
    etc.

    If this is a simulator and not a run-capable simulator, the debugger
    will not use the run() method, but instead repeatedly step, so that
    the execution can be interrupted by, say, pressing a key.  However,
    this means the simulator cannot detect whether this is a single step
    or one of the multitude of steps on behalf of run().  Because of
    this use of step(), the debugger clears the sleep (ZZ) bit in the
    debug register before initiating a single-step or the first step of
    a run.

    If you have specified -on=minimal_interface to the debugger, 
    it will never call run(); it will always write
    the status (or status32, for ARCompact) register to achieve the effect.  
    Thus you can just return 0 for run() if you specify -on=minimal_interface.

read_memory -- Read processor memory

    int read_memory(unsigned long adr, void *, unsigned long amount,
	int context)

    read_memory() is used to read processor memory.  The amount is in
    bytes and the return result is the number of bytes read. Any return
    result less than amount implies a failure.

    For the memory requests, you may assume that requests come in
    multiples of 4 and aligned to 0 mod 4 boundaries.  The maximum
    amount will be 8K -- a convenience for the implementor. Reads memory
    and returns # bytes read.  To support ARC's Harvard architecture,
    the context parameter has bit cntxt_data_reference ON if the memory
    is data memory, versus code memory.

    Note: the context parameter was added 1/14/00 and will syntactically
    invalidate builds of old source code that used the prior interface;
    however, prior implementations are not invalidated, due to the C
    calling convnetion, which has the caller popping the arguments from
    the stack.  When building with this new interface you can either
    change your source code or you can just comment out this parameter
    from your copy of arcint.h.

write_memory	-- Write processor memory

    int write_memory(unsigned long adr, void *, unsigned long amount,
	int context)

    write_memory() is used to write processor memory.  The amount is in
    bytes and the return result is the number of bytes written. Any
    return result less than amount implies a failure.

    You must invalidate any caches after a memory write.  To invalidate
    the icache on a standard ARC, write 0 to aux register 16.

    See the note in read_memory regarding the context parameter.

read_reg	-- Read processor register

    int read_reg(ARC_REG_TYPE r, unsigned long *value)

    Reads processor register r.  The value of register r must be
    assigned to the data pointed to by "value".

    This function is used to read both core and aux registers.  0 <= r
    <= 63 indicates a core register.  A value of r >= AUX_BASE indicates
    aux register r-AUX_BASE; e.g., if r == AUX_BASE, this indicates aux
    register 0.  Unfortunately this prohibits access to the last
    AUX_BASE registers at the high end of the aux numbering scheme.

    The values for registers are in HOST endian format, i.e., the endian
    of the computer that is executing this interface.

    See also read_banked_reg.

    Return the value 1 if the register was successfully read.
    Return 0 otherwise.

write_reg	-- Write processor register

    int write_reg(ARC_REG_TYPE r, unsigned long value)

    Writes processor register r.  The input "value" must be written to
    register r.  See read_reg for how registers are numbered.

    The values for registers are in HOST endian format, i.e., the endian
    of the computer that is executing this interface.

    See also write_banked_reg.

    Return the value 1 if the register was successfully written.
    Return 0 otherwise.

read_banked_reg	-- Read processor register from a specific register bank

    int read_banked_reg(int bank, ARC_REG_TYPE r, unsigned long *value)

    This function is called in lieu of read_reg if your version is >=
    ARCINT_TRACE_VERSION and you return the ARC_FEATURE_banked_reg bit
    bit from supports_feature.  This function was introduced to support
    access to the full range of aux registers, as well as the MADI
    registers.

    The bank is one of reg_bank_core, reg_bank_aux, or reg_bank_madi.
    The register numbering starts a 0 for each bank.

    The value for the register is in HOST endian format, i.e., the endian
    of the computer that is executing this interface.

    Return the value 1 if the register was successfully read.
    Return 0 otherwise.

write_banked_reg-- Write processor register from a specific register bank

    int write_banked_reg(int bank, ARC_REG_TYPE r, unsigned long *value)

    This function is called in lieu of write_reg if your version is >=
    ARCINT_TRACE_VERSION and you return the ARC_FEATURE_banked_reg bit
    bit from supports_feature.  This function was introduced to support
    access to the full range of aux registers, as well as the MADI
    registers.

    The bank is one of reg_bank_core, reg_bank_aux, or reg_bank_madi.
    The register numbering starts a 0 for each bank.

    The value for the register is in HOST endian format, i.e., the endian
    of the computer that is executing this interface.

    Return the value 1 if the register was successfully written.
    Return 0 otherwise.

memory_size	-- Return implementation memory size

    unsigned memory_size()

    Returns the memory size of the implementation.  This helps the
    debugger prevent memory overruns, and is also used for placing a
    breakpoint table in high memory in the case that you are debugging
    on a pre-arcV7 processor with a program lacking a linked-in
    breakpoint table.

set_memory_size	-- Specify memory size

    int set_memory_size(unsigned S)

    Sets the memory size of the implementation.

    If this is a simulator, do your best to allocate the requested
    memory.  If this is harware, and you can't tell what the real memory
    size is, just accept the setting and record it for retrieval in the
    memory_size() function.

prepare_for_new_program	-- Load and restart notification

    int prepare_for_new_program(int will_download)

    This function is called whenever a new program is being loaded, or
    when the current program is being restarted.

    Remove any hardware watchpoint or breakpoint settings left over from
    a previous run.  But the function need not doesn't flush the pipe or
    execute anything ambitious on the processor (the debugger flushes
    the pipe).

    will_download is true if the debugger intends to download the
    program, as opposed to merely attaching to whatever code is running.
    will_download is false if the user specified -off=download.

    This function has the responsibility of testing the initial
    communcations link.  However, note that if you are running multiple
    ARCs, other ARCs may be running, and you should not reset the
    communications link.

    Note: after download has complete and pipe flushing has been
    performed, the debugger sends the property "download_complete" with
    value "1" (process_property is called). You may use this to do
    further resetting of state; for example you may wish to clear
    instruction trace buffers or counters so that they do not reflect
    activity that occurred during a pipe flush.  If either download or
    pipe flushing fails, this property is NOT sent.

    Note: before prepare_... is called, the debugger will call 
    process_property with the key "gverbose" set to "0" or "1".
    If to "0", avoid any printing of copyright, initialization, etc. 
    messages.  gverbose=0 is used in conjunction with -run to get a
    run of the target program without *any* debugger messages.

    Return 1 to indicate success, 0 to indicate failure.

process_property -- Receive property settings

    int process_property(const char *key, const char *value)

    Function process_property is called for any property that the
    debugger hasn't already recognized. The property is named by "key"
    and contains the value given by the "value" input.  If your DLL
    recognizes the property named by "key", return the value 1 (whether
    or not processing the property was successful); if not, return 0.

    Otherwise return the value 0.

    This function is used to communicate arbitrary values to the
    hardware.  The debugger uses this to set timeout and port values to
    the hardware, and to specify run_cycles to a simulator (see
    is_simulator()).  For example:
    
       key		value
       --------		--------
       timeout		a number
       port		a number
       fabcard		0 or 1
       run_cycles	a number
       blast		filename

    At the debugger command-line, you can say

	-prop=key=value

    and (key,value) will be passed to this function.  You can also say

	-props=key1=value1,key2=value2,key3=value3, ...

    However, value1, value2, ... may not contain commas.

    At the debugger command-prompt (SC>) you can say:

	SC> prop key1=val1 key2=val2

    ARC FPGA blasting is done through this function.  When you specify
    -blast=filename in the debugger, process_property("blast",filename)
    is called, and it is the responsibility of this function to implement
    the blast code.

These next two functions are rarely used and you may return 0 for them.

additional_possibilities	-- Identify additional data about the interface

    const char * additional_possibilities();

    A string that might identify data that could be returned from the
    additional_information function.  This is a way for client and
    server to share information without tying it down in an interface.
    Currently MetaWare this function to detect that the debugger is
    talking to MetaWare's simulator or ARC hardware interface, and does
    something special for those two interfaces.

additional_information	-- Return additional information

    void *additional_information(unsigned which)

    Returns something that the client can use to communicate with the
    server, as indicated by additional_possibilities() return value.
    Typically, in C++, this will be an object calls to which are made to
    do the communication.  The parameter is used to allow returning
    different objects.


>>>>>>>>>>>>> VERSION ADDITION <<<<<<<<<<<<<

FOR ALL FUNCTIONS BELOW, your interface version must be
ARCINT_BREAK_WATCHPOINT_VERSION or greater.

The debugger uses hardware breakpoints and watchpoints where possible,
and falls back to software mechanisms otherwise.  Additionally, a
software watchpoint is evaluated only when the processor stops.

set_breakpoint	-- set a breakpoint

    int set_breakpoint(unsigned address, void *cookie_buf);

    Set a breakpoint at the given address; returns 1 if succeeded, 0
    otherwise.  
    
    If supports_feature() includes ARC_FEATURE_new_bp_cookie, the
    cookie_buf is actually a void**; you store a void* cookie into
    *cookie_buf, and we return this same cookie to remove_breakpoint.
    This is the preferred mode of operation.  The cookie must be non-zero.

    Otherwise, we provide you a "cookie_buf" where you can deposit
    information that we'll hand back to you on a remove_breakpoint.  The
    cookie buf is of length returned by your function
    breakpoint_cookie_len().  
    
    If you return 0, the debugger assumes you cannot set the breakpoint,
    and will fall back to using software breakpoints; i.e., using the
    standard jump to "flag 1; nop; nop; nop" for pre-arcv6 processors,
    and the "brk" instruction for arcv7 or later.

    Hardware breakpoints are necessary for setting a breakpoint in ROM.

remove_breakpoint	-- remove a breakpoint

    int remove_breakpoint(unsigned address, void *cookie_buf);

    Remove a breakpoint at the given address that was previously set by
    set_breakpoint.  
    
    If supports_feature() includes ARC_FEATURE_new_bp_cookie, you are
    given back the cookie that set_breakpoint stored into *cookie_buf.
    Otherwise, you are given back the data you may have previously
    written in cooke_buf.  Returns 1 if succeeded, 0 otherwise:

    Note that prepare_for_new_program is responsible for clearing all
    watchpoints and breakpoints.

breakpoint_cookie_len	-- Length of your breakpoint state

    int breakpoint_cookie_len();

    If supports_feature() includes ARC_FEATURE_new_bp_cookie, this
    function is unnecessary and is not called by the debugger.

    Specify the size of the cookie_buf that we need to provide you for
    breakpoint information storage.  The buffer is not guaranteed to be
    aligned.

    You may choose to implement your breakpoint storage locally; if so,
    merely return 0.

    Note: upon reflection, I would no longer design the interface using
    cookie created by the debugger Instead, I would have 
    (a) no breakpoint_cookie_len function;
    (b) set_breakpoint would *return* a cookie to the debugger; 
    (c) remove_breakpoint would hand that same cookie back to the interface.
    In this way, the set/remove_breakpoint methods are easily transportable
    across process address spaces, as the cookie is merely a number
    created by the interface.  In its current implementation, however,
    the cookie must be transported across TCP as well.
    If supports_feature() includes ARC_FEATURE_new_bp_cookie, this
    redesign is used instead.

at_breakpoint	-- Tell whether at breakpoint

    int at_breakpoint();

    Tell whether the reason we have stopped is because we are at a
    breakpoint set by means of a previous call to set_breakpoint.

    There are "old" and "new" methods for this function.

    The new method is used if supports_feature() includes
    ARC_FEATURE_at_breakpoint_cookie; otherwise the old method is used.

    In the old method, return 1 if we are at a breakpoint, and 0
    otherwise. The PC is assumed to point to the location of the broken
    instruction.

    The new method more precisely tells which breakpoint was hit. It is
    used if supports_feature() includes ARC_FEATURE_at_breakpoint_cookie. 
    In this case, at_breakpoint() returns the cookie associated with the
    breakpoint that was hit; that cookie is the one that set_breakpoint
    stored into *cookie_buf (if supports_feature() includes
    ARC_FEATURE_new_bp_cookie), or the cookie that the debugger handed
    you when set_breakpoint was called (if supports_feature() does NOT
    include ARC_FEATURE_new_bp_cookie).
    
    This is needed so the debugger can determine the actual breakpoint
    hit.  This is motivated for ARCompact where action points slide past
    a breakpoint location, and the PC address alone is not sufficient
    for the debugger to determine which breakpoint has been hit. For
    example, if you have action points implementing breakpoints at
    addresses A, A+2, and A+4, and the processor stops due to the action
    point at A, the program counter could be at A, A+2, or A+4,
    depending upon characteristics of the program.  So, return a cookie
    here that allows the debugger to correlate the found action point
    with a code address.

retrieve_breakpoint_code	-- Retrieve memory at a breakpoint

    int retrieve_breakpoint_code(unsigned address,
	char *dest, unsigned len, void *cookie_buf);

    If your set_breakpoint mechanism involved saving and overwriting the
    code at the breakpoint address, this function writes the saved code
    at dest for a maximum length of len. This is typically used to be
    able to present a disassembler the original code for disassembly.

    Post-6/18, this function is no longer used by the debugger; 
    the debugger itself saves 4 bytes at the location of the instruction,
    even if those 4 bytes are not modified by your set_breakpoint method.

Watchpoints are allowed for both registers and memory.

set_reg_watchpoint	-- Set a register watchpoint

    int set_reg_watchpoint(ARC_REG_TYPE r, int length);

    Set a register watchpoint.  This stops the processor whenever the
    register changes.  Returns 1 if succeeded, otherwise 0.

remove_reg_watchpoint	-- Remove a register watchpoint

    int remove_reg_watchpoint(ARC_REG_TYPE r, int length);

    Removes a register watchpoint.  If a previous watchpoint was set,
    remove it, and return 1; otherwise 0.

    Note that prepare_for_new_program is responsible for clearing all
    watchpoints and breakpoints.

set_mem_watchpoint	-- Set memory watchpoint

    int set_mem_watchpoint(unsigned long addr, int length);

    Sets a memory watchpoint.  Similar to set_reg_watchpoint.

remove_mem_watchpoint	-- Remove memory watchpoint

    Removes a memory watchpoint.  Similar to remove_reg_watchpoint.

stopped_at_watchpoint	-- Tell whether we stopped due to a watchpoint.

    int stopped_at_watchpoint();

    Tells whether we stopped at a watchpoint.  Return 1 if so, 0
    otherwise. The reason for this function is that we have no
    processor-defined way to determine this, and the debugger needs to
    know if we stopped at a watchpoint.


stopped_at_exception	-- Tell whether we stopped at an exception.

    int stopped_at_exception();

    Returns a non-zero exception number if the processor stopped due to
    some exception (otherwise unobtainable from processor status regs).
    For example, the simulator stops on bad instruction or bad memory
    reference.

    This is experimental; return 0 for now.


>>>>>>>>>>>>> VERSION ADDITION <<<<<<<<<<<<<

FOR ALL FUNCTIONS BELOW, your interface version must be
ARCINT_SPECIFIC_DISPLAY_VERSION or greater.

define_displays -- Define data displays to view processor state

    int define_displays(struct Register_display *rd);

    Use this function to define any custom displays related to your
    processor implementation.  If displays are defined, return the value
    1.  Otherwise return the value 0.

    For information on defining your own displays, see the header file
    "gtdispl.h" included in the sc/inc subdirectory of your distribution,
    and see also the etc/userwin subdirectory of your distribution.

    This function is called once.


>>>>>>>>>>>>> VERSION ADDITION <<<<<<<<<<<<<

FOR ALL FUNCTIONS BELOW, your interface version must be
ARCINT_FILL_MEM_VERSION or greater.

fill_memory	-- fill memory with a repeated value

    int fill_memory(unsigned long adr, void *buf, unsigned long amount,
	unsigned repeat_count, int context)

    Fills memory and returns # bytes logically written.  The debugger
    can use this to zero bss, but may (in the future) use it for any
    repeated pattern of any length.

    fill_memory is functionally the same as:

       int written = 0;
	for (int i = 0; i < repeat_count; i++, adr += amount)
	   written += write_memory(adr,buf,amount);
       return written;

    See read_memory and write_memory for the context parameter.

    NOTE: If your version is >= ARCINT_TRACE_VERSION, you must return
    the ARC_FEATURE_fill_memory bit from supports_feature to have this
    function called.

>>>>>>>>>>>>> VERSION ADDITION <<<<<<<<<<<<<

FOR ALL FUNCTIONS BELOW, your interface version must be
ARCINT_TRACE_VERSION or greater.

instruction_trace_count	-- Specify number of instruction traces available

    int instruction_trace_count()

    An instruction trace is simply a sequence of PC values.  This
    function returns the non-negative number of such PC values in a the
    sequence that will be returned by a subsequent call to the function 
    get_instruction_traces(), a call to which usually follows immediately.
    The debugger uses the result to allocate storage sufficient to 
    hold these PC values and prior to calling get_instruction_traces().

get_instruction_traces	-- Obtain the instruction traces

    void get_instruction_traces(unsigned *traces)

    Fills the buffer traces with N unsigned integers, where N is the
    result of the just prior call to instruction_trace_count().
    (Therefore, you must not let the instruction_trace_count() change
    between a call to instruction_trace_count() and an immediately following
    call to get_instruction_traces()).

    The last N instructions executed are at addresses traces[i], 0 <= i
    < N, where traces[N-1] is the address of the most recent
    instruction.

    NOTE: You must return the ARC_FEATURE_instruction_trace
    bit from supports_feature to have these functions called.

supports_feature	-- Tell which features you support

    int supports_feature()

    This function is used generally to add new functions to this
    interface without requiring a change of the interface version or
    requiring a DLL to implement the function.  Essentially, we use this
    function to allow non-implementation of functions added to the
    interface.

    Return the "or" of features you support.  Reason: you might have an
    interface version that requires the function but not support it
    nonetheless.  For example, you may support instruction traces but
    not fill_memory.

receive_callback	-- Receive a callback objecdt

    void receive_callback(ARC_callback*)

    This is called once right after get_ARC_interface, and provides you
    with a debugger callback object you can use to access debugger
    facilities.  For example, a printf is provided so your trace output
    can be properly interleaved with that of the debugger's or shown in
    the GUI.  You must supply this function if the interface version is
    ARCINT_TRACE_VERSION or higher.

in_same_process_as_debugger	-- Tell whether you are in the same
				   process as the debugger.
    
    This function should normally return 1.  If you are implementing
    your DLL in C++ you leave this function out and the body defaults to
    "return 1;".  
    
    An ARC object that virtualizes the ARC interface across process
    spaces (such as via TCP) should return 0, indicating that the caller
    of this ARC object cannot expect local memory pointers to work.
    
max_data_exchange_transfer	-- How much data can data_exchange send
			  	   or receive?
    
    This function should normally return 0xffff_fffe, which is a big number.
    However, an ARC object that virtualizes the ARC interface across
    process spaces (such as via TCP) should return its maximum transport
    capacity.  In this way you may choose to program a large data_exchange
    via multiple calls to data_exchange.

data_exchange		-- Exchange data with the ARC object.

    unsigned data_exchange
    	(unsigned what_to_do, unsigned caller_endian,
    	 unsigned send_amount, void *send_buf,
         unsigned max_receive_amount, void *receive_buf);

    NOTE: Your version() must be >= ARCINT_TRACE_VERSION, and you must
    return the ARC_FEATURE_data_exchange bit from supports_feature() to
    have this function called.
    
    This function is intended for general exchange of data with the ARC
    object.  The caller of this function provides data, if any, in
    send_buf; the length of this data is send_amount.  The function is
    allowed to write up to max_receive_amount bytes of return data into
    receive_buf.  For convenience, the what_to_do parameter provides an
    "opcode" to tell the function what it should do; you need not
    transmit the opcode in the send_buf.  Your value for what_to_do
    should start at DATA_XCHG_FIRST_USER_OPCODE; values less than this
    are reserved to the debugger.  If you do not recognize an opcode,
    you should return DATA_XCHG_UNRECOGNIZED_OPCODE.
    
    One use of this function is to provide data for other
    debugger interfaces.  For example, the debugger allows an instance
    of a Semantic Inspection Interface (SII) (see semint.h) to provide
    Addr_count (see adrcount.h) objects to the debugger.  An Addr_count
    object supplies counts for addresses in a program, such as the
    number of times an instruction at an address was executed; the
    counts can be conveniently displayed in other debugger windows.  The
    capability to provide an Addr_count interface is explicitly NOT
    added to this ARC interface.  If you have Addr_count information in
    your ARC interface, you must implement an SII to communicate it to
    the debugger; your SII can access your Addr_count information
    through the use of data_exchange.  The reason that Addr_count is not
    implemented in the ARC interface is that virtualizing the ARC
    interface across TCP is too difficult; therefore the work is
    essentially left up to you.  data_exchange *is* virtualized and can
    be depended on even if your ARC object is running in a different
    process.
    
    The SII can obtain access to the ARC object by calling function
    get_OEM_process_object() provided by the callback object given to an
    SII instance.  For a complete example, please see
    etc/dll/connect/semint in the debugger distribution.
    
    You can implement an Addr_count object in your ARC object and
    transmit the pointer thereto directly to your corresponding SII; but
    to do that, the SII object can call the
    in_same_process_as_debugger() in your ARC object.  The value of this
    function is 1 except when the debugger is talking to a TCP
    virtualization of an ARC object, where the value is 0.  Thus your
    SII can dynamically determine whether it is able to use an
    Addr_count object supplied by the ARC object.
    
    Because the debugger and your ARC object may be running on different
    machines, you should establish a convention for the endianness of
    the input and output data to this function.  We recommend that both
    input and output data be held in the endian of the machine running
    the debugger.  The data_exchange function has a caller_endian
    parameter; this parameter tells whether the caller of the function
    is a little-endian machine.  Within your data_exchange function you
    can invoke callback->endian->convert_long(caller_endian,value) to
    convert a long value from the endian of the machine you are running
    on to the endian of the caller.  The reason the caller is explicitly
    specified is that you may use data_exchange internally in your DLL;
    in that case, you should pass callback->endian->get_host_endian()
    for caller_endian; get_host_endian() computes the endian of the
    machine you are running on.  This allows data_exchange to be called
    either remotely from the debugger or within the ARC object itself
    and still work.  If you use the convention that the data is in the
    host endian format, all the callers of data_exchange need *never*
    worry about the format of the data; only the data_exchange function
    need ensure the proper endianness.
    
    The return value of data_exchange is the number of bytes returned in
    the return buffer. You must not misuse the return value for anything
    else, such as a return code, because a TCP virtualization mechanism
    uses this return value to allocate buffers to transport the return
    buffer.


OEM ARC implementations
-----------------------

The systems ARC_DLL and BRC_DLL are provided for third-party OEM
implementation of the functions the debugger needs to debug programs on
an ARC.  This allows access to an ARC via mechanisms about which the
debugger need not know. For example, you can implement a serial-port
access to a JTAG-enabled ARC.  All that is required is that you
implement the interface described in file arcint.h.  This and other
related files are in ROOT/etc/dll/connect.

Where ARC_DLL is mentioned below, the same holds true for system BRC_DLL.

The command-line options -DLL and -DLL= specify use of the DLL system.
-DLL=xxx specifies the DLL xxx should be opened.  -DLL specifies the
"default" DLL should be opened.  The default DLL name is set in the
bin/sc.cnf file and points to a dummy implementation whose source is
also in ROOT/etc/dll/connect.  Look for the line
    DLL_default=$SCDIR/etc/dll/arcint$DLL
in bin/sc.cnf when you are ready to change it.

You can specify properties to be given to your DLL system; properties
are an important way to configure your system.  Use the command-line
option

	-c="sysprop ARC_DLL prop1=val1 prop2=val2 ..."

or place it in your ARGS= line in bin/sc.cnf to make it permanent.

The debugger allows ARC_DLL to be cloned, so it is possible for multiple
ARC_DLLs to exist.  Each cloned DLL has the same default DLL name,
because the default DLL name is an "initial property" (see the
discussion of cloning).  But you could specify a different DLL name for
a clone via the DLL property that the debugger uses to communicate the
default DLL name.  For example:

	sysclone ARC_DLL D1 D2 D3
	sysprop D1 DLL=arc1.dll mem=50000 baudrate=9600
	sysprop D2 DLL=arc2.dll mem=10000 tracebufsize=5000

You could put these commands in a file called "clones" and specify
-c="read clones" on the debugger command line.

Alternatively, you could allow each to access the same DLL if your DLL
is capable of handling multiple ARC systems.

If you do not want ARC_DLL to be cloneable, change the text
"sysprop ARC_DLL cloneable=1" that appears in bin/sc.cnf to
"sysprop ARC_DLL cloneable=0".

Tracing your arcint implementation
----------------------------------
You can cause the debugger to print out a message to stdout whenever
it calls a function in your interface.  Specify
	-on=trace_board_interface
on the debugger command line.  Or, at the command prompt, you may
specify
	SC> prop trace_board_interface=1
to turn it on, and
	SC> prop trace_board_interface=0
to turn it off.

Due to timing issues the first few calls to your interface will not
be traced.

You may wish to turn off toggle show_reg_diffs to reduce the register read
traffic.

Some low-level detail.
----------------------
Some OEMs have asked about the differences between the various stepping
commands and what functions in the interface are called.  For
terminology, when we say arc.func() below, we mean the debugger is
calling the func() function in the arcint interface.

isi (instruction single-step):
    Calls arc.step().

iso (instruction single-step, but step over calls):
    If the instruction is a call, sets a breakpoint after the call
    and calls arc.run().  If the instruction is NOT a call, same as isi.

ssi (statement single-step):
    Analyzes the assembly language comprising the current statement.
    Computes the possible locations that the processor may stop at
    during execution of the statement.  Sets breakpoints at all those
    locations and calls arc.run().

sso (statement single-step, stepping over calls):
    Same as ssi, except that no breakpoints are set at targets that
    are statements arrived at via calls from the current statement.
    If there are no calls in the current statement, the result is the
    same as with ssi.

These are general descriptions.  For ssi and sso it can get even more
complicated than described above.  You can always see what's happening
by turning on the trace_board_interface toggle, described above.

Note that if you have specified -on=minimal_interface, neither step()
nor run() in this interface are called; the debugger instead writes
to the debug register to effect the step or run.


-------------------------------------------------------------------------------
Here is how to build a DLL out of your arcint implementation:

- Be sure to include the line
    #define OEM_USE_OF_DEBUGGER_HEADER_FILES 1
  in your source code before you
    #include "arcint.h"

- To compile and link:

NT:
    With Microsoft C:
	cl /LD arcint.c
    With MetaWare's High C:
	hc arcint.c -mslink -Hdll
    For NT, be sure that the function
	struct ARC *get_ARC_interface()
    has __declspec(dllexport).

	       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	       !!WARNING!!   !!WARNING!!   !!WARNING!!
	       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   On Windows 95/98 DLL instances in the same program all share the same
   static data.  On Windows NT, this is generally true as well, although
   we have observed static data duplication in some cases, despite NT
   documentation saying this never happens.  We cannot isolate the
   conditions under which it occurs nor find Microsoft Developer Library
   documentation that admits that it occurs and under what conditions.

   The best course of action is to NEVER use any static data in your
   implementation of a DLL.  If your DLL is to be loaded more than once,
   you can not depend on the treatment of static data. In the context of
   multiarc debugging, a DLL that you specify as an extension may well
   be loaded more than once.

			       END OF

	       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	       !!WARNING!!   !!WARNING!!   !!WARNING!!
	       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


UNIX:
    cc arcint.c -G -o arcint.so
   If you call functions that you don't resolve in the link, but know
   will exist by the time your DLL is loaded (for example, printf in
   the debugger), just supply -znodefs with the link.

	       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	       !!WARNING!!   !!WARNING!!   !!WARNING!!
	       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   For UNIX, be SURE that all your variables and functions are declared
   static, except, of course, for the single exported function
   get_ARC_interface.  The reason is that the UNIX dynamic loader
   has a single global address space for names.  If, in two separate
   DLLs, you use the same variable name (e.g., "pftp" in our samples --
   the pointer to the `virtual' function table), the UNIX loader will
   choose one of the names and ignore the second. That means your second
   DLL will use the first's variable; in the "pftp" case the second DLL
   uses the first DLL's set of functions!!  A messy crash is the likely
   result.

   Therefore, keep your symbols hidden from the UNIX dynamic loader by
   making them all static.  If you must export them -- say, because
   you have multiple modules implementing an extension -- choose long
   names tied to your extension so the likelihood of a conflict will
   be small.

			       END OF

	       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	       !!WARNING!!   !!WARNING!!   !!WARNING!!
	       !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

======================================================================*/



#if TEST
/////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------//
// 			Appendix Sample Implementation.		       //
// 	       A sample construction of a dummy DLL in MS C++.         //
//---------------------------------------------------------------------//
/////////////////////////////////////////////////////////////////////////
// Dummy implementation of the simulator that implements a single
// hardware breakpoint.
// This is loadable as a DLL.
// #include "arcint.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
enum { halt_bit = (1 << 25) };
#define _24_bits 0xffffff
#if __HIGHC__
#define override _Override
#else
#define override 	// No protection in MSC.
#endif

static char trace = 0;

struct My_ARC: ARC {
    My_ARC() {
	#if __HIGHC__ // Much more readable big number:
	memsize = 1_000_000;
	#else
	memsize = 1000000;
	#endif
	mem = (char*)malloc(memsize);
	bpreg = 0;
	status = debug = 0;
	stepcnt = 0;
	}
    unsigned status, debug;
    unsigned stepcnt;
    int memsize;
    char *mem;
    // Breakpoint register.
    int bpreg;
    override int MS_CDECL version() { return ARCINT_TRACE_VERSION+1; }
    override const char  *MS_CDECL id() { return "dummy implementation"; }
    override void MS_CDECL destroy() {
	// printf("destroy was called.\n");
	delete this;
	}
    override int MS_CDECL is_simulator() { return 1; }
    override int MS_CDECL step() {
	if (trace || stepcnt %500 == 0)
	    printf("[%d]step. PC %x\n",stepcnt,status);       
	stepcnt++;
	// If at BP, set halted and return.
	if (((status<<2) & _24_bits) == bpreg) {
	    status |= halt_bit;
	    return 1;
	    }
	status+=1;
	// status |= halt_bit;	// We get done immediately, so say we're halted.
	//status &= ~halt_bit;
	trace && printf("after step, status %x\n",status);
	// printf("trying sleep in callback\n");
	// callback->sleep(1000);
	return 1;
	}
    override int MS_CDECL run() { return 0; }	// Simulator can't run.
    override int MS_CDECL read_memory(unsigned long adr, void *buf,
		unsigned long amount, int context) {
	if (adr+amount > memsize) return 0;
	memcpy(buf,mem+adr,amount);
	return amount;
	}
    override int MS_CDECL write_memory(unsigned long adr, void *buf,
		unsigned long amount, int context) {
	// NOTE!!!
	// You must invalidate any caches after a memory write.  To invalidate the
	// icache on a standard ARC, write 0 to aux register 16.
	if (adr+amount > memsize) return 0;
	memcpy(mem+adr,buf,amount);
	return amount;
	}
    override int MS_CDECL read_reg(ARC_REG_TYPE r, unsigned long *value) {
	if (r == reg_STATUS) {
	    trace && printf("read status; it's %x\n",status);
	    *value = status;
	    }
	else if (r == reg_IDENTITY) *value = 6;
	else if (r == reg_DEBUG) *value = debug;
	else *value = 0x100+r;
	return 1;
	}
    override int MS_CDECL write_reg(ARC_REG_TYPE r, unsigned long value) {
	if (r == reg_STATUS) {
	    if ((status & halt_bit) == 0 && (value & halt_bit)) {
	       // If running, just turn on halt bit.
	       trace && printf("Just set the halt bit; before %x...\n",status);
	       status |= halt_bit;
	       }
	    else status = value;
	    trace && printf("write status with %x new value %x\n",value,status);
	    }
	else if (r == reg_DEBUG) {
	    trace && printf("writing %x to debug reg\n");
	    if (value & 1) {
		// Single-step the processor.  Write nothing.
		// Whether it's instruction (value_IS) or cycle-step doesn't
		// matter for this simulator.
		return step();
		}
	    debug = value;
	    if (value & 2) {
		trace && printf("halt the processor!\n");
		status |= halt_bit;	// Halt the processor.
		}
	    }
	return 1;
	}
    override unsigned MS_CDECL memory_size() { return memsize; }
    override int MS_CDECL set_memory_size(unsigned S) {
	free(mem);
	mem = (char*)malloc(memsize = S);
	return 1;
	}
    override const char * MS_CDECL additional_possibilities() {
	// Request that additional_information be called with parameter "which"
	// == 1 so we can supply addr counters.
	// return "adrcount";  // Not implemented yet.
	return 0;
	}

    override void *MS_CDECL additional_information(unsigned which) {
	switch(which) {
	    case 1: {
		// Supply a zero-terminated array of addr counters.
		return 0;	// Not implemented yet.
		}
	    }
	return 0;
	}
    override int MS_CDECL prepare_for_new_program(int will_download) {
	printf("I am preparing!\n"); return 1;
	}
    override int MS_CDECL process_property(const char *key, const char *value) {
	if (strcmp(key,"blast") == 0) {
	    // You should implement blast here.  value is the file name.
	    printf("I'm having a blast! -- filename %s\n",value);
	    return 1;
	    }
	return 0;	// No properties recognized.
	}
    // Breakpoints.  We provide one breakpoint register.
    override int MS_CDECL retrieve_breakpoint_code(unsigned address,
		    char *dest, unsigned len, void *cookie_buf) {
	// We don't overwrite the code, so no need to do anything here.  
	return 1;
	}

    override int MS_CDECL at_breakpoint() {
	unsigned pcval = (status<<2)&_24_bits;
	return pcval == bpreg;
	}
    override int MS_CDECL set_breakpoint(unsigned addr, void *cookie) {
	if (bpreg == 0) { bpreg = addr; return 1; }
	else return 0;	// Couldn't set: only 1 bp avail.
	}
    override int MS_CDECL remove_breakpoint(unsigned addr, void *cookie) {
	if (addr == bpreg) { bpreg = 0; return 1; }
	else return 0;	// No BP here.
	}
    override int MS_CDECL breakpoint_cookie_len() { return 0; }
    override void MS_CDECL receive_callback(ARC_callback*cb) {
	callback = cb;
	}
    override unsigned MS_CDECL data_exchange(
    	unsigned what_to_do, unsigned caller_endians, 
    	unsigned send_amount, void *send_buf,	       
        unsigned max_receive_amount, void *receive_buf) {
        printf("in data exchange.\n");
        if (what_to_do == 1) {
            unsigned amt = max_receive_amount;
            if (send_amount < amt) amt = send_amount;
            memcpy(receive_buf,send_buf,amt);
            return amt;
            }
        else return 999;
        }
    ARC_callback *callback;
    };

extern
// _MSC_VER is microsoft's compiler's define.
#if _MSNT || _MSC_VER	// Microsoft NT.
"C" __declspec(dllexport)
#endif
ARC *get_ARC_interface() {
    ARC *p = new My_ARC();
    // Sleep for a long time.
    //extern "C" void _Dstdcall Sleep(long);
    //Sleep(5000);
    return p;
    }
#endif
/*
// This can be built with MetaWare's High C and the command:
// 	hc arcint.c -mslink -Hdll
// Or with Microsoft C's command:
// 	cl /LD arcint.c
// But you should be able to use any W95 C compiler to do this.
// Be sure that the __declspec(...) above gets enabled.  Currently
// it's enabled for just High C on W95/NT and MSC; it has to remain
// off for UNIX, of course.
// On UNIX, use the command:
//	cc arcint.c -G -o arcint.so
*/
#endif
