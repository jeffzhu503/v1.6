#pragma once 
#include "GlobalDef.h"

void Get_Msgs(void);
void MASK_Msg(void);
void FLUSH_Msgs();
void Back_Win();
void Show(HWND W);
void Flip(long On);
void Button_Event(long Btn_Event, long Key, HWND New_Button);
void MessBox(char *P);
void PW_S(char *M);
void PW_E();
long Ok(char *Q);
void Launch(char *CL);
void Hide(HWND W);
long GetKey();

long IsCommment(char *p);
long IsBlack(char *p);
char *NextDigit(char *p);
char *NextBlack(char *p);
char *NextWhite(char *p);
char *NextWord(char *p);
char *NextLine(char *p);
void DelChar(char *s, long i, long L);

void FldToAscii(char *Buf, FldsT *FldP);

void SelColors();
void NormColors(); 

void PrintList(char *s, long D); 

WORD RtnSeed(WORD w); 