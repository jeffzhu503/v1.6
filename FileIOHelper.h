#pragma once

#include <stdio.h>
#include <string.h> 
#include "GlobalDef.h"
#include "ReportingDataT.h"

char *SecAndField(DWORD ByteAddr);
void fatalRW1(char *m);
void fatalRW2(char *m);
long ODF(char *Name8, char *Ext, char *FMode);
void DatIO(char *Name8, char *Mode, float Sector, void *RamAddress, DWORD ByteCnt);
void CloseOut();
void Cop(char *Src, char *Dst); 
void Copy(char *SrcN);
void CopyComDrv(ComT *c); 
void CopyE990();

void Copy_Qtr(char *_Bac, int Rest);
void CloseLoans(); 
void CopyForms(char *_Bac);
float *DecP(short inJ);
int LoadBankDecisions(int Prn);
bool loadDecisionFromCsvToDat(int Prn);
void StartingDecs();

long _DirI(char *F, char *S, WIN32_FIND_DATA *FF_Rec, HANDLE *FF_List);
long _DirC(WIN32_FIND_DATA *FF_Rec, HANDLE FF_List); 
