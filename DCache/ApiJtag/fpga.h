
#ifndef ARC_DLL_CONFIGURECPLD_H
#define ARC_DLL_CONFIGURECPLD_H



extern
// _MSC_VER is microsoft's compiler's define.
#if _MSNT || _MSC_VER	// Microsoft NT.
"C" __declspec(dllexport)
#endif

int configureCPLD(short iPPortBase, int iGClkSource0, int iGClkSource1,
		  int iGClkSource2, int iGClkSource3);


extern const int CLOCK_SOURCE_HIGH_IMPEDANCE;
extern const int CLOCK_SOURCE_PLL_MCLK;
extern const int CLOCK_SOURCE_PLL_VCLK;
extern const int CLOCK_SOURCE_CRYSTAL;
extern const int CLOCK_SOURCE_PLL_MCLK_HARVARD;
extern const int CLOCK_SOURCE_PLL_VCLK_HARVARD;
extern const int CLOCK_SOURCE_HOST_STROBE;
extern const int CLOCK_SOURCE_CRYSTAL_DIVIDED;
extern const int CLOCK_SOURCE_COUNT;
	
extern const char* CLOCK_SOURCE_STRINGS[];
extern const char * GCLOCK3_SOURCE_STRINGS[];
extern const int DefaultGClkSources[];


#endif // ARC_DLL_CONFIGURECPLD_H

#ifndef ARC_DLL_SETPLL_H
#define ARC_DLL_SETPLL_H

extern
// _MSC_VER is microsoft's compiler's define.
#if _MSNT || _MSC_VER	// Microsoft NT.
"C" __declspec(dllexport)
#endif

int setPLL(short iPPortBase, double MClk_freq, double VClk_freq,
	   double *Actual_MClk_freq = 0, double *Actual_VClk_freq = 0);

#endif // ARC_DLL_SETPLL_H

#ifndef ARC_DLL_TARGETIF_H
#define ARC_DLL_TARGETIF_H

int tester();


extern
// _MSC_VER is microsoft's compiler's define.
#if _MSNT || _MSC_VER	// Microsoft NT.
"C" __declspec(dllexport)
#endif

int isFPGAcfg(short iPPortBase);
					

#endif // ARC_DLL_TARGETIF_H
