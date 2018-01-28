#ifndef	_ARCREG_H_
#define	_ARCREG_H_
enum REG_CORE_AUX
{
	REG_R0=0,
	REG_R1,	REG_R2,
	REG_R3,	REG_R4, 
	REG_R5,	REG_R6,
	REG_R7, REG_R8,
	REG_R9, REG_R10,
	REG_R11, REG_R12, 
	REG_R13, REG_R14,
	REG_R15, REG_R16,
	REG_R17, REG_R18,
	REG_R19, REG_R20,
	REG_R21, REG_R22, 
	REG_R23, REG_R24, 
	REG_R25, REG_R26,
	REG_R27, REG_R28,
	REG_R29, REG_R30,
	REG_R31,
	REG_MUL_LO=0x39,
	REG_MUL_MID,
	REG_MUL_HI,
	REG_LOOP_COUNT,
	REG_PCL,
	REG_STATUS=0x40,
	REG_SEMAPHORE,
	REG_LP_START,
	REG_LP_END,
	REG_IDENTITY,
	REG_DEBUG,
	REG_PC,
	REG_ADCR,
	REG_APCR, 
	REG_ACR, 
	REG_STATUS32, 
	REG_STATUS32_L1, 
	REG_STATUS32_L2,
	ICACHE_LOCK_LINE   = 0x53,
	ICACHE_CONTROL_REG = 0x11 + 0x40,
	DCACHE_CONTROL_REG = 0x88,
	DCACHE_LOCK_LINE   = 0x89,
	DCACHE_IVDL_REG    = 0x8a,
	DCACHE_FLUSH_REG   = 0x8B,
	DCACHE_BUILD_REG   = 0xB2, 
	ICACHE_BUILD_REG   = 0xB7	
};

typedef unsigned int UINT32;
typedef unsigned long ULONG;
typedef unsigned short USHORT;

class AuxReg
{
	public:
	ULONG reg_status;
	ULONG reg_semaphore;
	ULONG reg_lp_start;
	ULONG reg_lp_end;
	ULONG reg_identity;
	ULONG reg_debug;
	ULONG reg_pc;
	ULONG reg_adcr;
	ULONG reg_apcr;
	ULONG reg_status32;
	ULONG reg_status32_l1;
	ULONG reg_status32_l2;
};


class CoreReg:public AuxReg
{
public:
	ULONG reg_core[32];
	ULONG reg_mul_lo;
	ULONG reg_mul_mid;
	ULONG reg_mul_hi; 
	ULONG reg_PCL;	
};

typedef unsigned short	Elf32_Half;
typedef unsigned long	Elf32_Off, Elf32_Word, Elf32_Addr;
typedef long			Elf32_Sword;

typedef struct Elf32_Sym {
	Elf32_Word	st_name;
	Elf32_Addr	st_value;
	Elf32_Word	st_size;
	unsigned char	st_info;	/* bind, type: ELF_32_ST_... */
	unsigned char	st_other;
	Elf32_Half	st_shndx;	/* SHN_... */
} Elf32_Sym;

#if 0
static ULONG memory_width_scratch_instructions[] = {

	/* 
	//  0: 0x00192200                 stb.di   %r1,[%r0]^M   hi lo 16bit swapped little endian A5
	//  4: 0x08801000                 ldb.di   %r0,[%r0]^M   hi lo 16bit swapped little endian A5
	//  8: 0x00241900                 stw.di   %r1,[%r0]^M  hi lo 16bit swapped little endian A5
	//  c: 0x09001000                 ldw.di   %r0,[%r0]^M  hi lo 16bit swapped little endian A5
	
	
	// read_short:
	*/
    	0x09001000,         /* //        ldw.di	%r0,[%r0]  hi lo 16bit swapped little endian */
		0x00402069,         /* //        flag	1 */
		0x7000264a,         /* //        nop */
		0x7000264a,         /* //        nop */
		0x7000264a,         /* //        nop */
		/* // write_short: */
    	0x00641800,         /* //        stw.di	%r1,[%r0]  hi lo 16bit swapped little endian */
		0x00402069,         /* //        flag	1 */
		0x7000264a,         /* //        nop */
		0x7000264a,         /* //        nop */
		0x7000264a,         /* //        nop */
		/* // read_byte: */
		0x08801000,         /* //        ldb.di	%r0,[%r0]  hi lo 16bit swapped little endian */
		0x00402069,         /* //        flag	1 */
		0x7000264a,         /* //        nop */
		0x7000264a,         /* //        nop */
		0x7000264a,         /* //        nop */
		/* //	write_byte: */
		0x00621800,         /* //        stb.di	%r1,[%r0]  hi lo 16bit swapped little endian */
		0x00402069,         /* //        flag	1 */
		0x7000264a,         /* //        nop */
		0x7000264a,         /* //        nop */
		0x7000264a,         /* //        nop */
}; 

static ULONG memory_width_scratch_instructions_swap[] = {
	/* // read_short: */
    	0x00100009,         /* //        ldw.di	%r0,[%r0] */
		0x40006920,         /* //        flag	1 */
		0x4a260070,         /* //        nop */
		0x4a260070,         /* //        nop */
		0x4a260070,         /* //        nop */
		/* // write_short: */
		0x00186400,         /* //        stw.di	%r1,[%r0] */
		0x40006920,         /* //        flag	1 */
		0x4a260070,         /* //        nop */
		0x4a260070,         /* //        nop */
		0x4a260070,         /* //        nop */
		/* // read_byte: */
		0x00108008,         /* //        ldb.di	%r0,[%r0] */
		0x40006920,         /* //        flag	1 */
		0x4a260070,         /* //        nop */
		0x4a260070,         /* //        nop */
		0x4a260070,         /* //        nop */
		/* //	write_byte: */
	    0x00186200,         /* //        stb.di	%r1,[%r0] */
		0x40006920,         /* //        flag	1 */
		0x4a260070,         /* //        nop */
		0x4a260070,         /* //        nop */
		0x4a260070,         /* //        nop */
}; 
#endif

#endif
