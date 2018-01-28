#include <windows.h>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <time.h>
//#include "inc\gportio.h"
//#include "inc\pioeval.h"
#include "arcint.h"
#include "crint.h"
#include "arcreg.h"
using namespace std;

#define APIJTAG	"apijtag.dll"
#define PORT_ADDRESS	0x378
#define EXIT_SUCCESSED	0x0
#define EXIT_FAILED		0xaabb

typedef int (*proc)(void);

const static ULONG DCACHE_ENABLED_MASK=0x1;
const static ULONG LINELENGTH = 0x40;

ULONG m_ulIdentity;
CoreReg	*REG_NOW;
ARC *parc;

unsigned int CheckBoardReady(ARC *);
void write4Byte(ARC *parc, unsigned long address, unsigned long value);
ULONG read4Byte(ARC *parc, unsigned long address);
void Shiny1C_Initial_R64Mb(void);
void Shiny1_Initial_R64Mb(void);
void Sunny8_Initial_R256Mb(void);
void Sunny9ES_Initial_DDR2_8bit_F512Mb(void);
void Sunny9ES_Initial_DDR2_16bit_F512Mb(void);
void SD920_Initial_R512Mb();
void SD930_Initial_R512Mb(void);
unsigned char* DummyData(unsigned char*, int, int=0);
void FillDummy(ULONG, unsigned int, char);
void IOIF_3_Enable(void);
long WriteFromData(char *, ULONG );
void FillRand(ULONG adr, unsigned int length);
#define REGREAD(BX, CX)		parc->get_ftab()->read_reg(parc, (ARC_REG_TYPE)BX, CX)
#define REGWRITE(BX, CX)	parc->get_ftab()->write_reg(parc, (ARC_REG_TYPE)BX, CX)
#define MEMWRITE(BX, CX)	write4Byte(parc, BX, CX)
#define MEMREAD(BX)			read4Byte(parc, BX)
#define GO()				parc->get_ftab()->run(parc)				


void DecideCacheSize(unsigned int reg);
int main(int argc, char *argv[])
{
	unsigned long arc_address=0x8000000;
	REG_NOW = new CoreReg;
	BYTE m_buff16byte[32]={0};
	BYTE m_16byte[32]={0};
	HINSTANCE jtagdll;
	int i=0, c=0;
	
	jtagdll = LoadLibrary(APIJTAG);
	if(!jtagdll)
		return 1;
	proc ProcGet = (proc)GetProcAddress(jtagdll, "get_ARC_interface");
	parc = (ARC*)ProcGet();
	//Shwo the jtag Connection status
	if(EXIT_FAILED == CheckBoardReady(parc))
	{
		//MessageBox(NULL, "jtag failed", "JTAG STATUS", 0);
		cout<<"======================open jtag failed========================="<<endl;
		system("PAUSE");
		return 1;
	}
	else
	{
		//MessageBox(NULL, "jtag success", "JTAG STATUS", 0);
		cout<<"======================open jtag success======================="<<endl;	
	}
	//parc->get_ftab()->write_reg(parc, (ARC_REG_TYPE)reg_STATUS, 0x0004 );
	
	
	unsigned long input; 	
	REGREAD(DCACHE_BUILD_REG, &input);
	//parc->get_ftab()->read_reg(parc, (ARC_REG_TYPE)0x72+0x40, &input);
	printf("read dcache build register:%x\n", input);
	DecideCacheSize(input);
	REGREAD(ICACHE_BUILD_REG, &input);
	//parc->get_ftab()->read_reg(parc, (ARC_REG_TYPE)0x77+0x40, &input);
	printf("read icache build register:%x\n", input);
	DecideCacheSize(input);

    
	REGWRITE(DCACHE_CONTROL_REG, 0xc0);
	REGREAD(DCACHE_CONTROL_REG, &input);		
	printf("dcache control register 0x%x\n",input);
	
	// Initial DRAM
       //	SD930_Initial_R512Mb();
	
	ULONG start_address = 0xA0000, checkLeng = 0x2000;
	//FillDummy(start_address, 0x2000, 0x13);
	FillRand(start_address, checkLeng);
	ULONG end_address = start_address+ checkLeng;			// 4k byte
		
    while(start_address<end_address)
	{
		REGWRITE(DCACHE_LOCK_LINE, start_address);
		REGREAD(DCACHE_CONTROL_REG, &input);	
        
		if((input&0x04)==0)
		{
			printf("dcache lock line failed at %x  %x\n", start_address, input);
			break;
		}
		start_address+=LINELENGTH;
		//system("PAUSE");
        //return 0;	
	}

	start_address = 0xA0000;
	ULONG count=0, data;
	while(start_address<end_address)
	{
         REGWRITE(0x98, count);
         REGREAD(0x9B, &input);
         data = MEMREAD(start_address);
         count+=4;
         start_address += 4;
         if(data!=input)
             printf("Compare error %x  %x in %x\n", data, input, count);
    }
    

	//IOIF_3_Enable();
	printf("dcache lock line success.\n");
	start_address = 0xA0000;
	long leng;
	/*leng = WriteFromData("boot_both_4k.bin", start_address);
	cout<<"icache data write ok! "<<leng<<endl;
	end_address = start_address+leng;
	while(start_address<end_address)
	{
		REGWRITE(ICACHE_LOCK_LINE, start_address);
		start_address+=LINELENGTH;
	}
	printf("icache lock line success.\n");
	
	REGWRITE(REG_PC, start_address);
	//MEMWRITE(0xFE80088, 0xC000);
	REGREAD(REG_PC, &input);
	printf("PC now is: %x\n",input);
	*/
    //GO();
	//


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
			break;
		case 1:
			cout<<"1 KByte"<<endl;
			break;
		case 2:	
			cout<<"2 KByte"<<endl;
			break;
		case 3:
			cout<<"4 KByte"<<endl;
			break;
		case 4:
			cout<<"8 Kbyte"<<endl;
			break;
                case 5:
			cout<<"16 Kbyte"<<endl;
			break;
	}		
}

void IOIF_3_Enable(void)
{
	unsigned long i, j, k=0;
	MEMWRITE(0xff04038, 0x0100);
	MEMWRITE(0xff0402C, 0x0000);
	for(i=0; i<5; i++)
	{
		MEMWRITE(0xff04020, 0xFFFF);	
		for(j=0; j<0xFF; j++)
		{
			MEMWRITE(0x100000, 0xFFFF);
		}
		MEMWRITE(0xff04020, 0x0000);
		for(j=0; j<0xFF; j++)
		{
			MEMWRITE(0x100000, 0xFFFF);
		}		
	}
}

long WriteFromData(char *source, ULONG adr)
{
	FILE *input;
	long leng, i, j;
	ULONG temp=0;
	char *buffer;
	input = fopen(source, "rb");
	if(input == NULL)
	{
		cout<<"the data is error"<<endl;
		return -100;
	}
	fseek(input, 0, SEEK_END);
	leng = ftell(input);
	rewind(input);
	buffer = new char[leng];
	fread(buffer, 1, leng, input);
	fclose(input);
	for(i=0; i<leng/4; i++)
	{
		for(j=0; j<4; j++)
			temp += *(buffer+i*4+j)<<(j*8);
		MEMWRITE(adr+i*4, temp);
		temp=0;
	}
	return leng;
}
/*void write2Byte(ARC *parc, unsigned long address, unsigned short value)
{
	unsigned long temo_address;
	unsigned char token[4];
	temp_address=(address/4)*4;
	parc->get_ftab()->read_memory(parc, (ULONG)temp_address, (void*)token, (ULONG)sizeof(token)/sizeof(token[0]), 0);
	//unsigned long result;
	token[3] = (value>>8)&0x00FF;
	token[2] = value&0x00FF;
	//token[1] = 
	//token[0] = value&0x00FF;

	parc->get_ftab()->write_memory(parc, (ULONG)address, token, sizeof(token)/sizeof(token[0]), 0);
	parc->get_ftab()->write_reg(parc, (ARC_REG_TYPE)0x40+0x10, 0x0);
}
*/
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
	//unsigned long result;
	token[3] = (value>>24)&0x00FF;
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
	bool connected;
	if(!parc->get_ftab()->process_property(parc, "port","0x378"))
	{
		connected = FALSE;
		printf("Connected FAILED!\n");
		return EXIT_FAILED;	
	}
	printf("Connected SUCCESS!\n");
	if(!parc->get_ftab()->process_property(parc, "reset_board",""))
	{
		if(!parc->get_ftab()->prepare_for_new_program(parc, 1))
		{
			printf("download for one time\n");
		}	
		connected = FALSE;
		printf("Reset Board FAILED!\n");
		return EXIT_FAILED;	
	}
	printf("Reset Board SUCCESS!\n");
	return EXIT_SUCCESSED;
}

void FillRand(ULONG adr, unsigned int length)
{
     int i;
     srand(time(NULL));
     length = (length/4+1)*4;
     for(i=0; i<(length/4); i++)
	{	
		MEMWRITE(adr+i*4, rand()%0xffffffff);
	}
}
void FillDummy(ULONG adr, unsigned int length, char startdata)
{
	int i=0, temp=0, j;
	length = (length/4+1)*4;
	for(i=0; i<(length/4); i++)
	{	
		for(j=0; j<32; j+=8)
		{
			temp += startdata<<j;
			startdata+=2;
		}
		MEMWRITE(adr+i*4, temp);
		temp = 0;		
	}
}

unsigned char* DummyData(unsigned char *adr, int length, int type)
{	
	int data=0x25, i=0;
	adr = new unsigned char[length];
	switch(type)
	{
		case 0:			//increasing sequence
			while(i<length)
			{
				*(adr+i) = data++;
				i++;
			}
			break;
		default:
			break;
	}
	return adr;
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

void SD920_Initial_R512Mb(void)
{     
    
    // AIO setting
    MEMWRITE( 0xfe40000,  0x0008);
    MEMWRITE( 0xfe40004,  0xffff);
    MEMWRITE( 0xfe40008,  0xffff);
    MEMWRITE( 0xfe4000c,  0xffff);
    MEMWRITE( 0xfe40010,  0xffff);
    
    // enable module
    MEMWRITE( 0xfe80080,  0x0000);
    MEMWRITE( 0xfe80084,  0x0000);
    MEMWRITE( 0xfe80088,  0x0000);
    MEMWRITE( 0xfe8008c,  0x0000);
    MEMWRITE( 0xfe80090,  0x0000);
    MEMWRITE( 0xfe80094,  0x0000);
    MEMWRITE( 0xfe80098,  0x0000);
    MEMWRITE( 0xfe8009c,  0x0010);
    MEMWRITE( 0xfe800a0,  0x0000);
    MEMWRITE( 0xfe800a4,  0x0000);
    MEMWRITE( 0xfe800a8,  0x0000);
    MEMWRITE( 0xfe800ac,  0x0000);
    MEMWRITE( 0xfe800b0,  0x0000);
    MEMWRITE( 0xfe800b4,  0x0000);
    MEMWRITE( 0xfe800c0,  0x0000);
    MEMWRITE( 0xfe800c4,  0x0000);
    MEMWRITE( 0xfe800c8,  0x0000);
    MEMWRITE( 0xfe800cc,  0x0000);
    MEMWRITE( 0xfe800f8,  0x0000);
    
    MEMWRITE( 0xfe80040,  0x2b80);
    MEMWRITE( 0xfe80040,  0x2380);
    
    // 64-bit BVCI setting
    MEMWRITE( 0xfe640fc,  0x0001); // FIFO 32-bit to 64-bit soft_rst
    MEMWRITE( 0xfe600e0,  0x0000);
    
    // DRAM setting
    // Use 8-bit mode setting for 16-bit device
    MEMWRITE( 0xfe20008,  0x00ff);
    MEMWRITE( 0xfe2000c,  0x483a); // 512 Mb, 16 Bit Device
    MEMWRITE( 0xfe20010,  0x0400); // Extend Mode Register
    MEMWRITE( 0xfe20014,  0x0024); // wr_reg001, CL4
    
    MEMWRITE( 0xfe20018,  0x0080); // Auto Refresh Period
    MEMWRITE( 0xfe2001c,  0x0060); // Power On Stable Period
    MEMWRITE( 0xfe20020,  0xffff); // DDR Timing Spec. Setting 1
    MEMWRITE( 0xfe2002c,  0x0003); // Power Consumption Setting
    MEMWRITE( 0xfe2003c,  0xc421); // gds1
    MEMWRITE( 0xfe20040,  0x1000);
    MEMWRITE( 0xfe20068,  0x0060);
    MEMWRITE( 0xfe20070,  0x0433);
    MEMWRITE( 0xfe20088,  0x5944);
    MEMWRITE( 0xfe2008c,  0x1300);
    MEMWRITE( 0xfe20090,  0x5944);
    MEMWRITE( 0xfe20094,  0x1300);
    MEMWRITE( 0xfe20098,  0x0000);
    MEMWRITE( 0xfe2009c,  0x0000);
    MEMWRITE( 0xfe200a0,  0x0000);
    MEMWRITE( 0xfe200a4,  0x0000);
    MEMWRITE( 0xfe200ac,  0x0000);
    MEMWRITE( 0xfe200b0,  0x0000);
    MEMWRITE( 0xfe200b4,  0x0000);
    MEMWRITE( 0xfe200c8,  0x0000);
    MEMWRITE( 0xfe200cc,  0x0000);
    
    MEMWRITE( 0xfe200f8,  0x8222);  // new addr mapping method
    MEMWRITE( 0xfe20000,  0x6040);  // total_buffer_depth_off  1
    MEMWRITE( 0xfe20000,  0x6041);
    MEMWRITE( 0xfe20000,  0x4041); // Initilization Start
    MEMWRITE( 0xfef0224,  0x0003); // bit8: dram arb_afifo bypass control
}

void Shiny1C_Initial_R64Mb(void)
{
    // AIO setting
    MEMWRITE(0xfe40000, 0x0008);
    MEMWRITE(0xfe40004, 0x4321);
    MEMWRITE(0xfe40008, 0x4321);
    MEMWRITE(0xfe4000c, 0x4321);
    MEMWRITE(0xfe40010, 0x4321);
                  
                   
    // ClkGen setting 
    MEMWRITE(0xfe8009c, 0xec60); // sensor clock from PLL
    MEMWRITE(0xfe800fc, 0x0000); // sensor_clk_base0
    MEMWRITE(0xfe800d0, 0x0460); // base: 12MHz, PLL: 384MHz
    MEMWRITE(0xfe800f8, 0x0000); // slow_clk = 0
    MEMWRITE(0xfe800f4, 0x0017); // sync. sensor/SDRAM clock, sensor delay factor
              
    MEMWRITE(0xfe80000, 0x0001); // DSP8, 128MHz
    MEMWRITE(0xfe80004, 0x0001); // DSP7, 128MHz
    MEMWRITE(0xfe80008, 0x0002); // SDRAM, 96MHz
    MEMWRITE(0xfe80010, 0x0001); // JMPEG, 128MHz
    MEMWRITE(0xfe80014, 0x0012); // DMA, clock from SDRAM, see SDRAM frequency
    MEMWRITE(0xfe80018, 0x0006); // Media, 48MHz
    MEMWRITE(0xfe8001c, 0x001b); // sensor_clk_base0, depend on sensor, 13.24MHz
    MEMWRITE(0xfe80038, 0x0003); // SIF, sensor_clk_base1, 76.8MHz
    MEMWRITE(0xfe80020, 0x0001); // Capture, 64MHz
    MEMWRITE(0xfe80024, 0x0002); // Display Scalar, 48MHz
    MEMWRITE(0xfe80028, 0x0002); // Display Engine, 48MHz
    MEMWRITE(0xfe8002c, 0x0002); // VIF, 96MHz
    MEMWRITE(0xfe8003c, 0x0008); // I2C/PWM, 38.4MHz
    MEMWRITE(0xfe80034, 0x0001); // SPI, 9.6MHz
    
    MEMWRITE(0xfe80080, 0xc000); // WPL, MADI
    MEMWRITE(0xfe80084, 0x5000); // DSP7, SWIRP
    MEMWRITE(0xfe80084, 0x0000); // DSP7, SWIRP
    MEMWRITE(0xfe80088, 0x5540); // MEM
    MEMWRITE(0xfe80088, 0x0000); // MEM
    MEMWRITE(0xfe80090, 0x5014); // JMPEG
    MEMWRITE(0xfe80090, 0x0000); // JMPEG
    MEMWRITE(0xfe80094, 0x5501); // DMA
    MEMWRITE(0xfe80094, 0x0000); // DMA
    MEMWRITE(0xfe80098, 0x0005); // Media
    MEMWRITE(0xfe80098, 0x0000); // Media
    MEMWRITE(0xfe8009c, 0xec60); // Sensor
    MEMWRITE(0xfe8009c, 0x4420); // Sensor
    MEMWRITE(0xfe8009c, 0x0000); // Sensor
    MEMWRITE(0xfe800a0, 0x5400); // CAP
    MEMWRITE(0xfe800a0, 0x0000); // CAP
    MEMWRITE(0xfe800a4, 0x4000); // DISP SLR
    MEMWRITE(0xfe800a4, 0x0000); // DISP SLR
    MEMWRITE(0xfe800a8, 0x4000); // DISP ENG
    MEMWRITE(0xfe800a8, 0x0000); // DISP ENF
    MEMWRITE(0xfe800ac, 0x0400); // VIF
    MEMWRITE(0xfe800ac, 0x0000); // VIF
    MEMWRITE(0xfe800b4, 0x4000); // SPI
    MEMWRITE(0xfe800b4, 0x0000); // SPI
    MEMWRITE(0xfe800c0, 0x4400); // P1/LCM
    MEMWRITE(0xfe800c0, 0x0000); // P1/LCM
    MEMWRITE(0xfe800c4, 0x0000); // P2
    MEMWRITE(0xfe800c8, 0x5400); // I2C/PWM
    MEMWRITE(0xfe800c8, 0x0000); // I2C/PWM
                
    // SDRAM setting  
    MEMWRITE(0xfe2000c, 0x0041);
    MEMWRITE(0xfe20010, 0x0110); // sdram + lvttl + 2.5V + dll off
    MEMWRITE(0xfe20014, 0x0202);
    MEMWRITE(0xfe20018, 0x0080);
    MEMWRITE(0xfe800f0, 0x00b6); // delay mux setting
    MEMWRITE(0xfe20000, 0x0001);
}

void Shiny1_Initial_R64Mb(void)
{
	    MEMWRITE(0xfe90004, 0x0001); // good ARC
    MEMWRITE(0xfe90078, 0x0000);
                     
    // AIO setting   
    MEMWRITE(0xfe40000, 0x0008);
    MEMWRITE(0xfe40004, 0x4321);
    MEMWRITE(0xfe40008, 0x4321);
    MEMWRITE(0xfe4000c, 0x4321);
    MEMWRITE(0xfe40010, 0x4321);
                   
    // ClkGen setting 
    MEMWRITE(0xfe8009c, 0xece0); // sensor clock from PLL
    MEMWRITE(0xfe800fc, 0x0000); // sensor_clk_base0
    MEMWRITE(0xfe800d0, 0x04bb); // base: 13MHz, PLL: 383.5MHz
    MEMWRITE(0xfe800f8, 0x0000); // slow_clk = 0
    MEMWRITE(0xfe800f4, 0x0017); // sync. sensor/SDRAM clock, sensor delay factor
    MEMWRITE(0xfe800f0, 0x0049); // sif/vif/mem delay factor
                   
    MEMWRITE(0xfe80000, 0x0001); // ARC, 127.83MHz
    MEMWRITE(0xfe80008, 0x0002); // SDRAM, 95.88MHz
    MEMWRITE(0xfe8000c, 0x0001); // HWIRP, 127.83MHz
    MEMWRITE(0xfe80010, 0x0001); // JMPEG, 127.83MHz
    MEMWRITE(0xfe80014, 0x0012); // DMA, clock from SDRAM, 95.88MHz
    MEMWRITE(0xfe80018, 0x0006); // Media, 47.9375MHz
    MEMWRITE(0xfe8001c, 0x001b); // sensor_clk_base0, depend on sensor, 13.224MHz
    MEMWRITE(0xfe80038, 0x0003); // sensor_clk_base1, 76.7MHz
    MEMWRITE(0xfe80020, 0x0001); // Capture, 63.916MHz
    MEMWRITE(0xfe80024, 0x0002); // Display Scalar, 47.94MHz
    MEMWRITE(0xfe80028, 0x0002); // Display Engine, 47.94MHz
    MEMWRITE(0xfe8002c, 0x0002); // Video Interface, 95.875MHz
    MEMWRITE(0xfe8003c, 0x0008); // I2C/PWM, 38.35MHz
    MEMWRITE(0xfe80034, 0x0001); // SPI, 9.6MHz
                
    MEMWRITE(0xfe80080, 0xc000); // WPL
    MEMWRITE(0xfe80088, 0x5540); // MEM
    MEMWRITE(0xfe80088, 0x0000); // MEM
    MEMWRITE(0xfe8008c, 0x4000); // HWIRP
    MEMWRITE(0xfe8008c, 0x0000); // HWIRP
    MEMWRITE(0xfe80090, 0x5014); // JMPEG
    MEMWRITE(0xfe80090, 0x0000); // JMPEG
    MEMWRITE(0xfe80094, 0x5501); // DMA
    MEMWRITE(0xfe80094, 0x0000); // DMA
    MEMWRITE(0xfe80098, 0x0005); // Media
    MEMWRITE(0xfe80098, 0x0000); // Media
    MEMWRITE(0xfe8009c, 0xece0); // Sensor
    MEMWRITE(0xfe8009c, 0x4420); // Sensor
    MEMWRITE(0xfe8009c, 0x0000); // Sensor
    MEMWRITE(0xfe800a0, 0x5400); // CAP
    MEMWRITE(0xfe800a0, 0x0000); // CAP
    MEMWRITE(0xfe800a4, 0x4000); // DISP SLR
    MEMWRITE(0xfe800a4, 0x0000); // DISP SLR
    MEMWRITE(0xfe800a8, 0x4000); // DISP ENG
    MEMWRITE(0xfe800a8, 0x0000); // DISP ENF
    MEMWRITE(0xfe800ac, 0x0400); // VIF
    MEMWRITE(0xfe800ac, 0x0000); // VIF
    MEMWRITE(0xfe800b4, 0x4000); // SPI
    MEMWRITE(0xfe800b4, 0x0000); // SPI
    MEMWRITE(0xfe800c0, 0x4400); // P1/LCM
    MEMWRITE(0xfe800c0, 0x0000); // P1/LCM
    MEMWRITE(0xfe800c4, 0x0000); // P2
    MEMWRITE(0xfe800c8, 0x5400); // I2C/PWM
    MEMWRITE(0xfe800c8, 0x0000); // I2C/PWM
                  
    // SDRAM setting  
    MEMWRITE(0xfe2000c, 0x0041);
    MEMWRITE(0xfe20010, 0x0110); // sdram + lvttl + 2.5V + dll off
    MEMWRITE(0xfe20014, 0x0202);
    MEMWRITE(0xfe20018, 0x0080);
    MEMWRITE(0xfe800f0, 0x0049); // delay mux setting
    MEMWRITE(0xfe20000, 0x0001);
}

void Sunny8_Initial_R256Mb(void)
{
    // RTC setting
    MEMWRITE(0xfe90004, 0x0001); // good ARC
    MEMWRITE(0xfe90078, 0x0000);
                     
    // AIO setting  
    MEMWRITE(0xfe40000, 0x0008);
    MEMWRITE(0xfe40004, 0x4321);
    MEMWRITE(0xfe40008, 0x4321);
    MEMWRITE(0xfe4000c, 0x4321);
    MEMWRITE(0xfe40010, 0x4321);
                     
    // ClkGen setting 
    MEMWRITE(0xfe800d0, 0x0016); // stop_pad_ddr_clk=0, factor=22, PLL_mem=12*22=264MHz
    MEMWRITE(0xfe800fc, 0x0757); // sensor_clk_src_sel=PLL_sensor, factorA=13, factorB=23, PLL_sensor=216*23/13=382.15MHz
    MEMWRITE(0xfe800f4, 0x0017); // syn_mem_sensor=1, sensor_del_factor=7
    MEMWRITE(0xfe800f8, 0x0000); // stop_dsp_trap=0, stop_dsp_from_base=0, enable all PLL, slow_clk=0
                    
                     
    MEMWRITE(0xfe80000, 0x0001); // base ARC, 144MHz
    MEMWRITE(0xfe80004, 0x0001); // DSP ARC, 144MHz
    MEMWRITE(0xfe80008, 0x0010); // mem_clk_bp=0, mem_clk_src_sel=PLL_sensor, DRAM, 95.53MHz
    MEMWRITE(0xfe8000c, 0x0011); // irp_clk_src_sel=PLL_mem, HWIRP, 88MHz
    MEMWRITE(0xfe80010, 0x0011); // jmpeg_clk_src_sel=PLL_mem, JMPEG, 88MHz
    MEMWRITE(0xfe80014, 0x0010); // dma_clk_src_sel=ddr_1xclk, DMA, clock from 1x DRAM, see 1x DRAM frequency
    MEMWRITE(0xfe80018, 0x0007); // media_clk_exp=0, Media, 48MHz
    MEMWRITE(0xfe8001c, 0x000e); // Sensor, 23.88MHz
    MEMWRITE(0xfe80020, 0x0007); // Capture, 24MHz
    MEMWRITE(0xfe80024, 0x0007); // Display Scalar, 24MHz
    MEMWRITE(0xfe80028, 0x000e); // Display Engine, 13.5MHz
    MEMWRITE(0xfe8002c, 0x0012); // Display TG, 10.8MHz
    MEMWRITE(0xfe80030, 0x0003); // UART, 0: 72MHz, 1: 36MHz, 2: 24MHz, 3: 18MHz
    MEMWRITE(0xfe80034, 0x0001); // SPI, 0: 19.6MHz, 1: 9.8MHz, 2: 4.9MHz, 3: 1.2MHz
                    
    MEMWRITE(0xfe80080, 0xc000); // WPL, MADI
    MEMWRITE(0xfe80084, 0x4000); // DSP ARC
    MEMWRITE(0xfe80084, 0x0000); // DSP ARC
    MEMWRITE(0xfe80088, 0x5550); // MEM
    MEMWRITE(0xfe80088, 0x0000); // MEM
    MEMWRITE(0xfe8008c, 0x5000); // HWIRP
    MEMWRITE(0xfe8008c, 0x0000); // HWIRP
    MEMWRITE(0xfe80090, 0x5014); // JMPEG
    MEMWRITE(0xfe80090, 0x0000); // JMPEG
    MEMWRITE(0xfe80094, 0x5515); // DMA
    MEMWRITE(0xfe80094, 0x0000); // DMA
    MEMWRITE(0xfe80098, 0x5540); // Media
    MEMWRITE(0xfe80098, 0x0000); // Media
    MEMWRITE(0xfe8009c, 0xcc70); // Sensor
    MEMWRITE(0xfe8009c, 0x4430); // Sensor
    MEMWRITE(0xfe8009c, 0x0010); // Sensor
    MEMWRITE(0xfe800a0, 0x5400); // Capture
    MEMWRITE(0xfe800a0, 0x0000); // Capture
    MEMWRITE(0xfe800a4, 0x4000); // Display Scalar
    MEMWRITE(0xfe800a4, 0x0000); // Display Scalar
    MEMWRITE(0xfe800a8, 0x4000); // Display Engine
    MEMWRITE(0xfe800a8, 0x0000); // Display Engine
    MEMWRITE(0xfe800ac, 0x3400); // Display TG
    MEMWRITE(0xfe800ac, 0x3000); // Display TG
    MEMWRITE(0xfe800b0, 0x5000); // UART
    MEMWRITE(0xfe800b0, 0x0000); // UART
    MEMWRITE(0xfe800b4, 0x4000); // SPI
    MEMWRITE(0xfe800b4, 0x0000); // SPI
    MEMWRITE(0xfe800b8, 0xf000); // Folder
    MEMWRITE(0xfe800c0, 0x4400); // Dock, MADC, ADC IP, PWM1, INTR, LMC
    MEMWRITE(0xfe800c0, 0x0000); // Dock, MADC, ADC IP, PWM1, INTR, LMC
    MEMWRITE(0xfe800c4, 0x0000); // General Register, RTC
    MEMWRITE(0xfe800c8, 0x1500); // I2C, PWM0, IO_MEM
    MEMWRITE(0xfe800c8, 0x0000); // I2C, PWM0, IO_MEM
    MEMWRITE(0xfe800cc, 0x5000); // Audio
    MEMWRITE(0xfe800cc, 0x0000); // Audio
                      
    // DRAM setting  
    MEMWRITE(0xfe2000c, 0x0046);
    MEMWRITE(0xfe20010, 0x0170); // sdram + ttl + 3.3V + dll off
    MEMWRITE(0xfe20014, 0x0202);
    MEMWRITE(0xfe20018, 0x0080);
    MEMWRITE(0xfe800f0, 0x0045); // delay mux setting
    MEMWRITE(0xfe20000, 0x0001);
}

void Sunny9ES_Initial_DDR2_8bit_F512Mb(void)
{
	    // AIO setting
    MEMWRITE(0xfe40000, 0x0008);
    MEMWRITE(0xfe40004, 0xffff);
    MEMWRITE(0xfe40008, 0xffff);
    MEMWRITE(0xfe4000c, 0xffff);
    MEMWRITE(0xfe40010, 0xffff);
                     
    // enable module 
    MEMWRITE(0xfe80080, 0x0000);
    MEMWRITE(0xfe80084, 0x0000);
    MEMWRITE(0xfe80088, 0x0000);
    MEMWRITE(0xfe8008c, 0x0000);
    MEMWRITE(0xfe80090, 0x0000);
    MEMWRITE(0xfe80094, 0x0000);
    MEMWRITE(0xfe80098, 0x0000);
    MEMWRITE(0xfe8009c, 0x0010);
    MEMWRITE(0xfe800a0, 0x0000);
    MEMWRITE(0xfe800a4, 0x0000);
    MEMWRITE(0xfe800a8, 0x0000);
    MEMWRITE(0xfe800ac, 0x0000);
    MEMWRITE(0xfe800b0, 0x0000);
    MEMWRITE(0xfe800b4, 0x0000);
    MEMWRITE(0xfe800c0, 0x0000);
    MEMWRITE(0xfe800c4, 0x0000);
    MEMWRITE(0xfe800c8, 0x0000);
    MEMWRITE(0xfe800cc, 0x0000);
    MEMWRITE(0xfe800f8, 0x0000);
                      
    // clock mux      
                      
    // DRAM setting   
    MEMWRITE(0xfe2000c, 0x484b); // 512 Mb, 8 Bit Device
    MEMWRITE(0xfe20010, 0x0400); // Extend Mode Register
    MEMWRITE(0xfe20014, 0x0024); // 8 Bit Mode
    MEMWRITE(0xfe20018, 0x0080); // Auto Refresh Period
    MEMWRITE(0xfe2001c, 0x0060); // Power On Stable Period
    MEMWRITE(0xfe20020, 0xffff); // DDR Timing Spec. Setting 1
    MEMWRITE(0xfe2002c, 0x0003); // Power Consumption Setting
    MEMWRITE(0xfe20000, 0x0001); // Initilization Start
}

void Sunny9ES_Initial_DDR2_16bit_F512Mb(void)
{
    // AIO setting
    MEMWRITE(0xfe40000, 0x0008);
    MEMWRITE(0xfe40004, 0xffff);
    MEMWRITE(0xfe40008, 0xffff);
    MEMWRITE(0xfe4000c, 0xffff);
    MEMWRITE(0xfe40010, 0xffff);
                     
    // enable module 
    MEMWRITE(0xfe80080, 0x0000);
    MEMWRITE(0xfe80084, 0x0000);
    MEMWRITE(0xfe80088, 0x0000);
    MEMWRITE(0xfe8008c, 0x0000);
    MEMWRITE(0xfe80090, 0x0000);
    MEMWRITE(0xfe80094, 0x0000);
    MEMWRITE(0xfe80098, 0x0000);
    MEMWRITE(0xfe8009c, 0x0010);
    MEMWRITE(0xfe800a0, 0x0000);
    MEMWRITE(0xfe800a4, 0x0000);
    MEMWRITE(0xfe800a8, 0x0000);
    MEMWRITE(0xfe800ac, 0x0000);
    MEMWRITE(0xfe800b0, 0x0000);
    MEMWRITE(0xfe800b4, 0x0000);
    MEMWRITE(0xfe800c0, 0x0000);
    MEMWRITE(0xfe800c4, 0x0000);
    MEMWRITE(0xfe800c8, 0x0000);
    MEMWRITE(0xfe800cc, 0x0000);
    MEMWRITE(0xfe800f8, 0x0000);
                     
    // DRAM setting  
    MEMWRITE(0xfe2000c, 0x084a); // 512 Mb, 16 Bit Device
    MEMWRITE(0xfe20010, 0x0400); // Extend Mode Register
    MEMWRITE(0xfe20014, 0x0023); // 16 Bit Mode
    MEMWRITE(0xfe20018, 0x0080); // Auto Refresh Period
    MEMWRITE(0xfe2001c, 0x0060); // Power On Stable Period
    MEMWRITE(0xfe20020, 0xffff); // DDR Timing Spec. Setting 1
    MEMWRITE(0xfe2002c, 0x0003); // Power Consumption Setting
    MEMWRITE(0xfe20000, 0x0001); // Initilization Start
}
