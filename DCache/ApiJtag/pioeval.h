//************* PIOEVAL.H ***************

/*** Copyright 1998 MetaWare Incoporated ***/

#ifndef __PIOEVAL_H__
#define __PIOEVAL_H__ 1
// FOR THE DOCUMENTATION OF THIS INTERFACE SEE gportio.h.

#define PIO_PUSH 1
#define PIO_POP 2
#define PIO_INP 3
#define PIO_OUTP 4
#define PIO_I_TRANSFER 5
#define PIO_LOOP_BEGIN 6
#define PIO_LOOP_END 7
#define PIO_BREAK_ON_TRUE 8
#define PIO_BREAK_ON_FALSE 9
#define PIO_BIT_AND 10
#define PIO_BIT_OR 11
#define PIO_BIT_XOR 12
#define PIO_EQ 13
#define PIO_NE 14

#define MAX_IN_BUF_SIZE 256

#ifdef __cplusplus
extern "C" {
#endif

extern void pioInitInstr();
extern void pioInstr(unsigned char code);
extern void pioConst(unsigned char data);
extern void pioOutp(unsigned char port);
extern void pioInp(unsigned char port);
extern int pioSize();

}

#define MAX_LOOP_STACK 5
#define MAX_PIO_EVAL_STACK 128
#define MAX_PIO_INSTR_BUF 4096


typedef struct PioInstr_ {
    unsigned char code;
    unsigned char data;
    } PioInstr;

typedef struct PioInstrStream_ {
    unsigned long pioInstrBufIdx;
    PioInstr pioInstrBuf[MAX_PIO_INSTR_BUF];
    } PioInstrStream;


#define pioInitInstr_(stream) stream.pioInstrBufIdx = 0

#define pioInstr_(stream, c) \
    stream.pioInstrBuf[stream.pioInstrBufIdx++].code = c

#define pioConst_(stream, d) \
    stream.pioInstrBuf[stream.pioInstrBufIdx].code = PIO_PUSH; \
    stream.pioInstrBuf[stream.pioInstrBufIdx++].data = d

#define pioOutp_(stream, port) \
    stream.pioInstrBuf[stream.pioInstrBufIdx].code = PIO_OUTP; \
    stream.pioInstrBuf[stream.pioInstrBufIdx++].data = port

#define pioInp_(stream, port) \
    stream.pioInstrBuf[stream.pioInstrBufIdx].code = PIO_INP; \
    stream.pioInstrBuf[stream.pioInstrBufIdx++].data = port

#define pioSize_(stream) stream.pioInstrBufIdx

#define pioBuf_(stream)  stream.pioInstrBuf


#endif

