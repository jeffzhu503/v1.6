#pragma once 

#include "BMSimDataT.h"
#include "GlobalDef.h"
#include "ReportingDataT.h"
#include "DecisionFormDataT.h"

// BMSim's main set of globals.
 ComT Com[MaxComm]; // Communities
 BOAT BOA[MaxB]; // Book of Accounts
 YTDT YTD[MaxB]; // Year to Date income data.

//Variables used in reporting and set in UI
int Page, Plus, CFld, OFld, EconPreview;  ///Plus is the sign of the number. It prints "+" if Plus=0. 
int Row, Last_Rpt, Rpt_I;
// General data used on reports:
char BankName[50], TimeStamp[88], BankNeu[20], StrA[MaxFields][FldSz], ForQtr[40], ForQtrs[60], AsOf[40], EndOf[40], Date[Qtrs][10], FQ[Qtrs + 1][10], PQ[Qtrs][10];

int _In;
char Mess[400], _T[400], _For[10];
long rv, EconSet_I;

 //variables used in FileIOHelper
FILE *Out;
float Version,  *BuffAddress;
/// NumLen is the total length of the string to which the number is converted for print.  DotLen is the length after the dot. 
DWORD NumLen, DotLen, DiskAddress;
 
int Bank =  - 1, Bank_On, ComOn, UI_Up, Drive, NumBanks, NumComs, SimQtr, SimYr, YrsQtr, ComI =  - 1, rvs, len;
char FromCom, Community, _Com[10], DatFile[40], FileMode[8];

float TefAdj, TaxAdj; //used in security and B100
//float Adj; //used in Resources, Loan
float BillsAndBonds[MaxB]; //Securities, and B500
