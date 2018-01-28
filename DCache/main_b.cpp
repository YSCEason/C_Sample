#include <windows.h>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include "arcint.h"
#include "crint.h"
#include "arcreg.h"
using namespace std;

#define APIJTAG	"apijtag.dll"
// #define APIJTAG	"apidglnt.dll"
#define PORT_ADDRESS	0x378
#define EXIT_SUCCESSED	0x0
#define EXIT_FAILED		0xaabb

typedef int (*proc)(void);

const static ULONG DCACHE_ENABLED_MASK=0x1;
const static ULONG LINELENGTH = 0x40;

// /* MADI register */
// static const ULONG   SINGLE_CAST_MODE = 0x000;
// static const ULONG   MULTI_CAST_MODE =  0x100;


ULONG m_ulIdentity;
CoreReg	*REG_NOW;
ARC *parc;

unsigned long g_dc_size=0;

unsigned int CheckBoardReady(ARC *);
void write4Byte(ARC *parc, unsigned long address, unsigned long value);
ULONG read4Byte(ARC *parc, unsigned long address);
void FillRand(ULONG adr, unsigned int length);
#define REGREAD(BX, CX)		parc->get_ftab()->read_reg(parc, (ARC_REG_TYPE)BX, CX)
#define REGWRITE(BX, CX)	parc->get_ftab()->write_reg(parc, (ARC_REG_TYPE)BX, CX)
#define MEMWRITE(BX, CX)	write4Byte(parc, BX, CX)
#define MEMREAD(BX)			read4Byte(parc, BX)
	
void SD930_Initial_R512Mb(void);

void DecideCacheSize(unsigned int reg);

char CPU_Number[80] = {0};

int main(int argc, char *argv[])
{

    if( argc < 2 )
    {
        printf( "\nICache_test.exe [CPU Number]\n" );    
        return 1;
    }
    strcpy(CPU_Number, argv[1]);

	REG_NOW = new CoreReg;
	HINSTANCE jtagdll;
	
	jtagdll = LoadLibrary(APIJTAG);
	if(!jtagdll)
		return 1;
	proc ProcGet = (proc)GetProcAddress(jtagdll, "get_ARC_interface");
	parc = (ARC*)ProcGet();
	//Show the jtag Connection status
	if(EXIT_FAILED == CheckBoardReady(parc))
	{
		cout<<"======================open jtag failed========================="<<endl;
		system("PAUSE");
		return 1;
	}
	else
	{
		cout<<"======================open jtag success======================="<<endl;	
	}
	
	unsigned long input; 	

	REGREAD(0x44, &input);
	printf("read CPU ID register:%lx\n", input);

	REGREAD(DCACHE_BUILD_REG, &input);
	printf("read dcache build register:%lx\n", input);
	DecideCacheSize(input);
	
	
	REGREAD(ICACHE_BUILD_REG, &input);
	printf("read icache build register:%lx\n", input);
	//DecideCacheSize(input);

    
	// REGWRITE(DCACHE_CONTROL_REG, 0x1);
	REGWRITE(DCACHE_CONTROL_REG, 0xc0);
	REGREAD(DCACHE_CONTROL_REG, &input);		
	printf("dcache control register 0x%lx\n",input);
	
	ULONG start_address = 0xA0000 + 0x80000000; //modify by Easan "start_address+0x8000 0000"   
	ULONG checkLeng = g_dc_size;
	

	REGWRITE(0x905+0x40, 1);

	FillRand(start_address, checkLeng);
	system("PAUSE");

	ULONG end_address = start_address + checkLeng;		
    while(start_address<end_address)
	{
		REGWRITE(DCACHE_LOCK_LINE, start_address);
		REGREAD(DCACHE_CONTROL_REG, &input);	
        
		if((input&0x04)==0)
		{
			printf("dcache lock line failed at %lx  %lx\n", start_address, input);
			//break;
		}
		start_address+=LINELENGTH;
		//system("PAUSE");
        //return 0;	
	}
	
	int ok_num=0, err_num=0;
	start_address = 0xa0000 + 0x80000000; //modify by Easan "start_address+0x8000 0000"   
	ULONG count=0, data;
	while(start_address<end_address)
	{
        REGWRITE(0x98, count); // Data Cache External Access Address, DC_RAM_ADDR
        REGREAD(0x9B, &input); // Data Cache Data Access, DC_DATA
        data = MEMREAD(start_address);

        if(data!=input)
		{
            // printf("Compare error %08lx %08lx in %04lx\n", data, input, count);
			err_num +=1;
		}
		else
		{	
			// printf("Compare    OK %08lx %08lx in %04lx\n", data, input, count);
			ok_num +=1;
		}
		
		count+=4;
        start_address += 4;
    }	

	printf("dcache lock line success. OK: %d, ERR: %d\n", ok_num, err_num);

	system("PAUSE");
    return EXIT_SUCCESSED;
}

void DecideCacheSize(unsigned int reg)
{
	int result = (reg>>12)&0x07;

	switch(result)
	{
		case 0:
			cout<<"0.5KByte"<<endl;
			g_dc_size=0x200;
			break;
		case 1:
			cout<<"1 KByte"<<endl;
			g_dc_size=0x400;
			break;
		case 2:	
			cout<<"2 KByte"<<endl;
			g_dc_size=0x800;
			break;
		case 3:
			cout<<"4 KByte"<<endl;
			g_dc_size=0x1000;
			break;
		case 4:
			cout<<"8 Kbyte"<<endl;
			g_dc_size=0x2000;
			break;
        case 5:
			cout<<"16 Kbyte"<<endl;
			g_dc_size=0x4000;
			break;
		case 6:
			cout<<"32 Kbyte"<<endl;
			g_dc_size=0x8000;
			break;
		case 7:
			cout<<"64 Kbyte"<<endl;
			g_dc_size=0x10000;
			break;
	}		
}


ULONG read2Byte(ARC *parc, unsigned long address)
{
	unsigned long result;
	unsigned char token[4];
	parc->get_ftab()->read_memory(parc, (ULONG)address, (void*)token, (ULONG)sizeof(token)/sizeof(token[0]), 0);
	result = (ULONG)token[3]<<24|(ULONG)token[2]<<16|(ULONG)token[1]<<8|(ULONG)token[0];
	return result;
}


void write4Byte(ARC *parc, unsigned long address, unsigned long value)
{
	unsigned char token[4];
	//token[3] = (value>>24)&0x00FF;
	token[3] = 0;
	token[2] = (value>>16)&0x00FF;
	token[1] = (value>>8)&0x00FF;
	token[0] = value&0x00FF;
	parc->get_ftab()->write_memory(parc, (ULONG)address, token, sizeof(token)/sizeof(token[0]), 0);
	parc->get_ftab()->write_reg(parc, (ARC_REG_TYPE)0x40+0x10, 0x0);
}

ULONG read4Byte(ARC *parc, unsigned long address)
{
	unsigned long result;
	unsigned char token[4];
	parc->get_ftab()->read_memory(parc, (ULONG)address, (void*)token, (ULONG)sizeof(token)/sizeof(token[0]), 0);
	result = (ULONG)token[3]<<24|(ULONG)token[2]<<16|(ULONG)token[1]<<8|(ULONG)token[0];
	return result;
}

	
unsigned int CheckBoardReady(ARC *parc)
{	
	if(!parc->get_ftab()->process_property(parc, "port","0x378"))
	{
		printf("Connected FAILED!\n");
		return EXIT_FAILED;	
	}
	printf("Connected SUCCESS!\n");

	if(!parc->get_ftab()->process_property(parc, "jtag_chain","A:A:A:A"))
	{
		printf("JTAG Chain FAILED!\n");
		return EXIT_FAILED;	
	}
	printf("JTAG Chain SUCCESS!\n");

	if(!parc->get_ftab()->process_property(parc, "reset_board",""))
	{
		if(!parc->get_ftab()->prepare_for_new_program(parc, 1))
		{
			printf("download for one time\n");
		}	
		printf("Reset Board FAILED!\n");
		return EXIT_FAILED;	
	}
	printf("Reset Board SUCCESS!\n");


//-------------------------------------------------------------------
	if(!parc->get_ftab()->process_property(parc, "cpunum", CPU_Number))
	{
		printf("Changed CPU FAILED!\n");
		return EXIT_FAILED;	
	}
    printf("Changed CPU SUCCESS -1!\n");

    if(!parc->get_ftab()->prepare_for_new_program(parc, 1))
	{
		printf(">>>> CPU number test\n");
		return EXIT_FAILED;	
	}	
    printf("Changed CPU SUCCESS -2!\n");

//-------------------------------------------------------------------
 //    unsigned long arcnum = 0x01;
 //    unsigned long madi_ctrl;
 //    //we have to switch
 //    madi_ctrl = (arcnum-1) | SINGLE_CAST_MODE;
 //    if(!parc->write_banked_reg( reg_bank_madi, reg_MCR, &madi_ctrl))
	// {
	// 	printf("Changed CPU FAILED!\n");
	// 	return EXIT_FAILED;	
	// }
 //    printf("Changed CPU SUCCESS!\n");
//-------------------------------------------------------------------

	return EXIT_SUCCESSED;
}

void FillRand(ULONG adr, unsigned int length)
{
    unsigned int i;
    srand(time(NULL));
    length = (length/4+1)*4;
    for(i=0; i<(length/4); i++)
	{	
		MEMWRITE(adr+i*4, ((rand() << 16) | rand()) ^ 0xffffffff);
	}
}


void SD930_Initial_R512Mb(void)
{     
    
    	// AIO setting
	MEMWRITE( 0xfe40000, 0x0008);
	MEMWRITE( 0xfe40004, 0xffff);
	MEMWRITE( 0xfe40008, 0xffff);
	MEMWRITE( 0xfe4000c, 0xffff);
	MEMWRITE( 0xfe40010, 0xffff);
	
	// enable module
	MEMWRITE( 0xfe80080, 0x0000);
	MEMWRITE( 0xfe80084, 0x0000);
	MEMWRITE( 0xfe80088, 0x0000);
	MEMWRITE( 0xfe8008c, 0x0000);
	MEMWRITE( 0xfe80090, 0x0000);
	MEMWRITE( 0xfe80094, 0x0000);
	MEMWRITE( 0xfe80098, 0x0000);
	MEMWRITE( 0xfe8009c, 0x0010);
	MEMWRITE( 0xfe800a0, 0x0000);
	MEMWRITE( 0xfe800a4, 0x0000);
	MEMWRITE( 0xfe800a8, 0x0000);
	MEMWRITE( 0xfe800ac, 0x0000);
	MEMWRITE( 0xfe800b0, 0x0000);
	MEMWRITE( 0xfe800b4, 0x0000);
	MEMWRITE( 0xfe800c0, 0x0000);
	MEMWRITE( 0xfe800c4, 0x0000);
	MEMWRITE( 0xfe800c8, 0x0000);
	MEMWRITE( 0xfe800cc, 0x0000);
	MEMWRITE( 0xfe800f8, 0x0000);
	
	MEMWRITE( 0xfe80040, 0x2b80);
	MEMWRITE( 0xfe80040, 0x2380);
	
	
	// ARB0001.D9 seeting, 3fifo w/OCRAM
	MEMWRITE( 0xfe60fe0, 0x0000);
	MEMWRITE( 0xfe640e0, 0x000e);
	MEMWRITE( 0xfe640e4, 0x000e);
	MEMWRITE( 0xfe640e8, 0x000e);
	MEMWRITE( 0xfe640f0, 0x0666);
	MEMWRITE( 0xfe640fc, 0x0888);
	
	// DRAM setting
	// Use 8-bit mode setting for 16-bit device
	MEMWRITE( 0xfe20008, 0x00ff);
	MEMWRITE( 0xfe2000c, 0x483a); // 512 Mb, 16 Bit Device
	MEMWRITE( 0xfe20010, 0x0400); // Extend Mode Register
	MEMWRITE( 0xfe20014, 0x0024); // wr_reg=001, CL=4, gds must = 0
	//MEMWRITE( 0xfe20014, 0x0025); // wr_reg=001, CL=3, gds must = 1
	MEMWRITE( 0xfe20018, 0x0080); // Auto Refresh Period
	MEMWRITE( 0xfe2001c, 0x0060); // Power On Stable Period
	MEMWRITE( 0xfe20020, 0xffff); // DDR Timing Spec. Setting 1
	MEMWRITE( 0xfe2002c, 0x0003); // Power Consumption Setting
	MEMWRITE( 0xfe2003c, 0xc421); // gds=1
	MEMWRITE( 0xfe20040, 0x1000);
	//MEMWRITE( 0xfe20040, 0xc0cc);
	MEMWRITE( 0xfe20068, 0x0060);
	MEMWRITE( 0xfe20070, 0x0433);
	MEMWRITE( 0xfe20088, 0x5944);
	MEMWRITE( 0xfe2008c, 0x1300);
	MEMWRITE( 0xfe20090, 0x5944);
	MEMWRITE( 0xfe20094, 0x1300);
	MEMWRITE( 0xfe20098, 0x0000);
	MEMWRITE( 0xfe2009c, 0x0000);
	MEMWRITE( 0xfe200a0, 0x0000);
	MEMWRITE( 0xfe200a4, 0x0000);
	MEMWRITE( 0xfe200ac, 0x0000);
	MEMWRITE( 0xfe200b0, 0x0000);
	MEMWRITE( 0xfe200b4, 0x0000);
	MEMWRITE( 0xfe200c8, 0x0000);
	MEMWRITE( 0xfe200cc, 0x0000);
	//MEMWRITE( 0xfe200f4, 0x0203);
	MEMWRITE( 0xfe200f8, 0x8222); // new addr mapping method
	MEMWRITE( 0xfe20000, 0x6040); // total_buffer_depth_off = 1
	MEMWRITE( 0xfe20000, 0x6041);
	MEMWRITE( 0xfe20000, 0x4041); // Initilization Start
	
	MEMWRITE( 0xfef0224, 0x0003); // bit8: dram arb_afifo bypass control

}
