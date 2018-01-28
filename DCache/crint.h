/*--------------------------------------------------------------------------
   (C) Copyright 1997-2002;   ARC International;  Santa Cruz, CA 95060
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
 * The C++ implementation is used by SeeCode internally; we have also
 * used it with Microsoft C++.  The implementation requires that
 * the C++ virtual function table start at location 0 of the object, and
 * that slot 0 of the virtual function table is skipped (MetaWare C++
 * uses this slot to point to run-time type information). If this is not
 * the case in your C++ you need to alter the C++ version of the macros
 * below, e.g. to include a dummy 0-th function.
 *
 * It has been verified that Sun's SPARC C++ has virtual function tables
 * incompatible with the the debugger, as Sun's virtual function tables
 * have *two* empy slots at the beginning.  Thus interfaces written
 * with Sun compilers may need to use C.  The same problem lies within GCC.
 * See INCOMPATIBLE COMPILERS below for more help.
 *
 * DANGERS OF OMITTED OVERRIDES
 * -----------------------------
 * If you use C++, you face the danger that an incorrectly declared
 * function in your derived class does not match the signature of the
 * function in the base class, and the compiler silently ignores your
 * intended override.  (That is why we invented the `override' keyword 
 * in MetaWare High C++ in 1996.)  You can do the following to ensure
 * that you have corectly correctly overriden functions in the base class.
 * 	#define IMPNAME My_derived_class
 *	CHECK_OVERRIDES(base_class)
 * For example, suppose the interface name is Addr_count, and you
 * implement My_addr_count.  Then:
 *	#define IMPNAME My_addr_count
 *	CHECK_OVERRIDES(Addr_count)
 * If you get a compile-time error, you have a mismatch in function
 * signatures between the base class and the derived class.  
 * The CHECK_OVERRIDES macro uses pointer-to-member-functions to 
 * ensure that a derived class method matches the signature of a
 * base class method.
 * Unfortunately the error message is never good enough to tell you which.
 * You will get a this kind of message from Microsoft C++:
 * myprog.cc(205) : error C2446: '==' : no conversion from 
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

#define c_func(return_val, name, parameters) return_val (*name) parameters;
#define c_func2(return_val, name, parameters, body) return_val (*name) parameters;
#define c_comma ,

#define C_access(interface_name) \
    C_define_vtab(interface_name,interface_name)

#define C_access_ab(interface_name) \
    C_define_vtab_allow_bodies(interface_name,interface_name)

// Use these to generate the virtual function tables for your
// derived type when you are in C++ but must define your derived
// class by-hand because you are using a non-compatible C++ compiler.
#define C_define_vtab_allow_bodies(derived_name,interface_name) \
    typedef struct derived_name ## _functab {         \
        void (*leave_0)();	/* Must be zero. */     \
    interface_name ## _functions(			\
    	c_func, c_func2, derived_name*obj, derived_name*obj c_comma) \
    } derived_name ## _functab;			

// Use these to generate the virtual function tables for your
// derived type in C.
#define C_define_vtab(derived_name,interface_name) \
    typedef struct derived_name ## _functab {         \
        void (*leave_0)();	/* Must be zero. */     \
    interface_name ## _functions(			\
    	c_func, derived_name*obj, derived_name*obj c_comma) \
    } derived_name ## _functab;			

#if __cplusplus // {

// For C++ we allow C access as well, to overcome problems with GCC's 
// vfunc tables, which can't be reconciled to High C++'s.
	
#define C_access_ftab(interface_name) \
    typedef struct interface_name ##_functab ftab_type; \
    /* At slot 0 in the object is High C++'s vfunc table: */ \
    ftab_type * get_ftab() { return *(ftab_type**)this; }

#define C_CALL(p,func) ((p)->get_ftab()->func)

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
    #define MS_DUMMY virtual void MS_dummy(){} 
    #define create_interface(interface_name) \
	struct interface_name; C_access(interface_name) \
	struct interface_name { \
	    C_access_ftab(interface_name) \
	    MS_DUMMY \
	    interface_name ## _functions(cpp_func, microsoft_c_bug, microsoft_c_bug) }; 
    #if ENSURE_OVERRIDES // {
    // If you want to be sure you are overriding functions in the interface,
    // turn off the default bodies, compile, and check that the only 
    // functions missing are those that you don't intend to override.
    // However, CHECK_OVERRIDES does a more reliable job, and we recommend it.
	#define create_interface_allow_bodies(interface_name) \
	    struct interface_name; C_access_ab(interface_name) \
	    struct interface_name { \
		C_access_ftab(interface_name) \
		MS_DUMMY \
		interface_name ## _functions(cpp_func, cpp_func_skip_default, microsoft_c_bug, microsoft_c_bug) } 
    #else // } {
	#define create_interface_allow_bodies(interface_name) \
	    struct interface_name; C_access_ab(interface_name) \
	    struct interface_name { \
		C_access_ftab(interface_name) \
		MS_DUMMY \
		interface_name ## _functions(cpp_func, cpp_func2, microsoft_c_bug, microsoft_c_bug) }; 
    #endif // }
    typedef unsigned __int64 unsigned_long_long;
#else // } {
    #define MS_CDECL
    #define create_interface(interface_name) \
        struct interface_name; C_access(interface_name) \
	struct interface_name { \
	    C_access_ftab(interface_name) \
	    interface_name ## _functions(cpp_func,,) }; 
    #define create_interface_allow_bodies(interface_name) \
        struct interface_name; C_access_ab(interface_name) \
	struct interface_name { \
	    C_access_ftab(interface_name) \
	    interface_name ## _functions(cpp_func,cpp_func2,,) }; 
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
// Several macros needed here to get a typedef name that varies according 
// to the line number, so you can have more than one in a file.
#define CHECK_OVERRIDES(interface_name) CHECK_OVERRIDES2(interface_name,__LINE__)
#define CHECK_OVERRIDES2(interface_name,line) CHECK_OVERRIDES3(interface_name,line)
#define CHECK_OVERRIDES3(interface_name,line) \
    typedef dummy_check##line##interface_name[sizeof( 0 interface_name##_functions(funcbool,funcbodybool,microsoft_c_bug,microsoft_c_bug))];

#else // } {	// C version below.

/* C version; you define the contents of struct interface_name yourself: */
#define create_interface(interface_name) 		\
	typedef struct interface_name interface_name;	\
	C_access(interface_name)

#define create_interface_allow_bodies(interface_name) 	\
	typedef struct interface_name interface_name;	\
	C_access_ab(interface_name)

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

// For convenience in casting the object pointer argument when
// defining an object in C.
// May be used in C++ contexts as well.
#define cast_argument(my_interface_name,p) my_interface_name *p = (my_interface_name*) _ ## p

#if _MSC_VER
#define __EXPORT_FROM_DLL __declspec(dllexport)
#else
#define __EXPORT_FROM_DLL 
#endif

/*
INCOMPATIBLE COMPILERS
======================

You need not read this section if you are using either High C++ or
Microsoft C++.  A Sun C++ or GNU C++ reader should read this material
if he wishes to use C++, not C.

If you are using a non-compatible compiler such as Sun's C++ or GNU C++,
you cannot access objects provided by the debugger directly, or use C++
to define and create objects to be supplied to the debugger.

You could choose to write any code that interfaces with SeeCode objects
in C only, perhaps writing a C++ wrapper to your C code.  This is
one technique that works but is a bit painful.

You can also write your code in C++ and make partial use of C++.
Here's how.

Accessing SeeCode objects
-------------------------

For accessing an object provided by the debugger, you can use the
macro C_CALL to make a call on the object.  
C_CALL allows a C-style call from within C++ code; the C-style call directly
accesses the virtual function table.  So, instead of writing

	p->func(a,b,c)

you must write

	C_CALL(p,func)(p,a,b,c)

which is slightly more verbose, but at least you need not switch 
entirely to C to make use of SeeCode's interfaces.

NOTE 1.  The compiler will not warn you if you use p->func fails.  Your
program will silently fail or worse.  You must take care to use the 
C_CALL yourself.  

NOTE 2. You must use C_CALL for objects you defined as well, if you have
defined them to be compatible with SeeCode.

Defining objects for SeeCode
----------------------------
You can use inheritance to define a subclass of a SeeCode interface
for the purpose of providing objects to SeeCode.  But you cannot
override the base class directly.  Instead, you define them as statics
and explicitly define the virtual function table. 

Here is a real example.

From a SeeCode header file we have:

#define Debugger_interface_callback_functions(func,fbdy,OBJ0,OBJ) \
    func(int, version, (OBJ0) )				\
    func(const char *, id, (OBJ0) )			\
    fbdy(void, destroy, (OBJ0), return; )		\
    func(int, printf, (OBJ const char *format, ...))	
create_interface_allow_bodies(Debugger_interface_callback)

Now to create a subclass of this using C++, define it as follows:

struct My_callback : Debugger_interface_callback {
    bool save_text;
    char buf[1024];
    // Here are the virtual functions we override:
    static int version(My_callback *p) { return 1; }
    static const char *id(My_callback *p) { return "test EVI callback"; }
    static void destroy(My_callback *p) { free(p); }
    static int printf(My_callback *p, const char *format, ...) {
	va_list ap; va_start(ap,format);
	if (p->save_text) return vsprintf(p->buf,format,ap);
	else return vprintf(format,ap);
	}
    // Here are extra functions we introduce; they needn't be virtual:
    void set_save(bool save) { save_text = save; }
    char *get_text() { return buf; }
    };
C_define_vtab_allow_bodies(My_callback,Debugger_interface_callback)

Take care not to introduce any more virtual functions, as such will
not work.  You can declare additional non-virtual functions, as illustrated
by set_save and get_text.  The C_define_vtab_allow_bodies macro invocation
creates a virtual function table with the correct function types; you
then can define that table:

static My_callback_functab my_callback_ftab = {
    0,	// This must always be 0.  High C++ puts 0 or RTTI here.
    My_callback::version, 
    My_callback::id,
    My_callback::destroy,
    My_callback::printf,
    };

Finally, here is the allocator and constructor for the derived class:

static My_callback *create_My_callback() {
    My_callback *p = (My_callback*)malloc(sizeof(My_callback));
    // We rely on the knowledge that for all compilers, the vtab is at
    // location 0 of the object:
    (*(My_callback_functab**)p) = &my_callback_ftab;
    p-> save_text = false; 
    return p;
    }

You can now pass a My_callback* back to SeeCode for use.
*/
