/*--------------------------------------------------------------------------
   (C) Copyright 1997-2001; MetaWare Incorporated;  Santa Cruz, CA 95060
---------------------------------------------------------------------------*/
/* Interface construction tools. */

/*
 * The debugger allows external communication to OEM-supplied interfaces.
 * These interfaces are object-oriented yet can be implemented in either
 * C or in C++.
 *
 * The macros in this file allow a single specification of the interface.
 * The macros expand either to text suitable for a C implementation or
 * a C++ implementation, depending on whether the program is being
 * compiled in C or C++.
 *
 * The C++ implementation is used by MetaWare internally; we have also
 * used it with Microsoft C++.  The implementation requires that
 * the C++ virtual function table start at location 0 of the object, and
 * that slot 0 of the virtual function table is skipped (MetaWare's C++
 * uses this slot to point to run-time type information). If this is not
 * the case in your C++ you need to alter the C++ version of the macros
 * below, e.g. to include a dummy 0-th function.
 *
 * It has been verified that Sun's SPARC C++ has virtual function tables
 * incompatible with the the debugger, as Sun's virtual function tables
 * have *two* empy slots at the beginning.  Thus interfaces written
 * with Sun compilers must use C.
 *
 * If you use C++, you face the danger that an incorrectly declared
 * function in your derived class does not match the signature of the
 * function in the base class, and the compiler silently ignores your
 * intended override.  (That is why MetaWare invented the `override'
 * keyword in High C++ in 1996.)  You can do the following to ensure
 * that you have corectly correctly overriden functions in the base class.
 * 	#define IMPNAME My_derived_class
 *	CHECK_OVERRIDES(base_class)
 * For example, suppose the interface name is Addr_count, and you
 * implement My_addr_count.  Then:
 *	#define IMPNAME My_addr_count
 *	CHECK_OVERRIDES(Addr_count)
 * If you get a compile-time error, you have a mismatch in function
 * signatures between the base class and the derived class.  
 * Unfortunately the error message is never good enough to tell you which.
 * You will get a this kind of message from Microsoft C++:
 * myprog.cc(205) : error C2446: 
 	'==' : no conversion from 
	'int (__cdecl My_addr_count::*)(long,unsigned long)' to 
	'int (__cdecl My_addr_count::*)(int,unsigned long)'
         Types pointed to are unrelated; conversion requires reinterpret_cast, 
	 C-style cast or function-style cast
   myprog.cc(205) : error C2116: function parameter lists differed
 * What this is saying is that you declared a function in your derived
 * class of type
 	'int (__cdecl My_addr_count::*)(long,unsigned long)' 
 * (the first prototype mentioned) where it should be instead of type 
	'int (__cdecl My_addr_count::*)(int,unsigned long)'
 * Now you get to go find out which function it was based solely on
 * the signature.
 */
   
/*------------------------------------------------------------------
 * Macros to create the C++ structure or C struct that reprsents
 * an interface.  Do the following to make an interface:
 *	1. Choose a name XXX for your interface.  The created
 *	   C++ structure or C struct will have that name.
 *	2. Provide a macro XXX_functions(func,OBJ0,OBJ).  This macro
 *	   should have a sequence of macro invocations of the form
 *		func(return type, func name, (parms))
 *	   where (parms) is either
 *		(a) (OBJ0)	-- for a 0-parameter function
 *		(b) (OBJ parms) -- for a 1 or more parameter function.
 *	3. Invoke macro create_interface(XXX).
 * Client use of the interface:
 *	C++: XXX *p; p->function(...)
 *	C  : XXX *p; p->ftab->function(p,...)
 * To provide an instance of the interface:
 *	C++: struct MyXXX : XXX { // Here define the funcs };
 *	     Supply new MyXXX() as an instance.
 *      C  : 1. define the functions as global static in the virtual
 *	        function table and place them in the virtual function
 *		table.  For type compatibility, you have to use XXX
 *		as the type of the first argument.  See cast_argument
 *		macro below to cast it from XXX to MyXXX -- if you
 *		name the parameter with a leading underscore.
 *	        XXX_functab XXXfuncs = { 0, the list of functions };
 *	     2. Put the virtual function table at location 0 in your object
 *	        and follow it with any private data you need:
 *	        struct MyXXX { XXX_functab *pft; <your data goes here> };
 *	     3. Supply (XXX*)(address of an MyXXX instance).
 *	    	Set pft to point to XXXfuncs.
 */
	

#if __cplusplus // {

#define microsoft_c_bug	// blank.  Bug in their C preprocessor.

#if _MSC_VER && !__HIGHC__ // {
    // Microsoft C++ insists on passing "this" in ecx and having callee pop
    // the stack unless each member function is explicitly tagged with __cdecl.
    // What is needed is a compilation option to request the __cdecl calling
    // convention, but /Gd doesn't work for C++!
    // The MetaWare convention for C++ is __cdecl, where "this" is simply the
    // first parameter passed.
    #define MS_CDECL __cdecl
    // Microsoft C: put a dummy slot at location 0 of the vtable.
    #define create_interface(interface_name) \
	struct interface_name { virtual void MS_dummy(){} interface_name ## _functions(cpp_func, microsoft_c_bug, microsoft_c_bug) };
    #if ENSURE_OVERRIDES // {
    // If you want to be sure you are overriding functions in the interface,
    // turn off the default bodies, compile, and check that the only 
    // functions missing are those that you don't intend to override.
	#define create_interface_allow_bodies(interface_name) \
	    struct interface_name { virtual void MS_dummy(){} interface_name ## _functions(cpp_func, cpp_func_skip_default, microsoft_c_bug, microsoft_c_bug) };
    #else // } {
	#define create_interface_allow_bodies(interface_name) \
	    struct interface_name { virtual void MS_dummy(){} interface_name ## _functions(cpp_func, cpp_func2, microsoft_c_bug, microsoft_c_bug) };
    #endif // }
    typedef unsigned __int64 unsigned_long_long;
#else // } {
    #define MS_CDECL
    #define create_interface(interface_name) \
	struct interface_name { interface_name ## _functions(cpp_func,,) };
    #define create_interface_allow_bodies(interface_name) \
	struct interface_name { interface_name ## _functions(cpp_func,cpp_func2,,) };
    typedef unsigned long long unsigned_long_long;
#endif // }

#define cpp_func(return_val, name, parameters) \
	virtual return_val MS_CDECL name parameters = 0;
// Use this for defining a default body.
#define cpp_func2(return_val, name, parameters,body) \
	virtual return_val MS_CDECL name parameters { body }
// Use this for discarding the default body.
#define cpp_func_skip_default(return_val, name, parameters,body) \
	cpp_func(return_val, name, parameters) 

#define allow_clients(interface_name)

// Create booleans of the form (int (der::*)(int))0 == &der::foo.
#define funcbool(return_val, name, parameters) \
	&& ( (return_val (MS_CDECL IMPNAME::*)parameters)0 == &IMPNAME::name )
#define funcbodybool(return_val, name, parameters,body) \
	funcbool(return_val,name,parameters)
#define CHECK_OVERRIDES(interface_name) \
    typedef dummy_check##interface_name[sizeof( 0 interface_name##_functions(funcbool,funcbodybool,microsoft_c_bug,microsoft_c_bug))];

#else // } {

#define c_func(return_val, name, parameters) return_val (*name) parameters;
#define c_func2(return_val, name, parameters, body) return_val (*name) parameters;
#define c_comma ,

/* C version; you define the contents of struct interface_name yourself: */
#define create_interface(interface_name) 		\
    typedef struct interface_name interface_name;	\
    typedef struct interface_name ## _functab {         \
        void (*leave_0)();	/* Must be zero. */     \
    interface_name ## _functions(			\
    	c_func, interface_name*obj, interface_name*obj c_comma) \
    } interface_name ## _functab;			\

#define create_interface_allow_bodies(interface_name) 	\
    typedef struct interface_name interface_name;	\
    typedef struct interface_name ## _functab {         \
        void (*leave_0)();	/* Must be zero. */     \
    interface_name ## _functions(			\
    	c_func, c_func2, interface_name*obj, interface_name*obj c_comma) \
    } interface_name ## _functab;			

/* For convenience in casting the object pointer argument. */
#define cast_argument(my_interface_name,p) my_interface_name *p = (my_interface_name*) _ ## p

#define allow_clients(interface_name)			\
    /* This is defined for use by clients: */		\
    struct interface_name {				\
    	interface_name ## _functab *ftab;               \
    	};

/*
 * C users:
 * Implement this for interface XXX by:
 * (1) defining your functions
 * (2) XXX_functab pf = {0, <list of your functions> };
 * (3) struct XXX { XXX_functab *pft; <any data you want> };
 * (4) An instance of XXX is an instance of the interface.
 *     Set pft to point to your XXX_functab.
 * 
 * C++ users:
 * (1) Subclass the interface with:
 *     struct My_XXX : XXX { (your functions go here) };
 * (2) If you are using Microsoft C++, EVERY ONE OF THE FUNCTIONS
 *     must specify __cdecl (or MS_CDECL, as defined above).
 */

#endif // }

