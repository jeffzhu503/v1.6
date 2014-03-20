#ifndef GLOBALDEF_H
#define GLOBALDEF_H

#include <ole2.h>
#include <stdio.h>
#include "BMSimDataT.h"
#include "SimpleCalculation.h"

// Const variables 
const float UnSet =  - 9876; // This is an unset input/output field.

// Used in BMSimDataT.h
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned __int64 u64;
typedef char *LnP;
typedef float *FeP;

/// Set an object memory to zero. E.g.  used in B001
#define Zero( X ) memset( & ( X ), 0, sizeof( X ) )

/// This loops L times:
#define Loop( L ) int J = -1, LLL = L; while ( ++J < LLL ) 
//relace Loop with For
//for(int J = 0; J < L; J++)

/// This looks once for each active bank in the community:
#define LB Bank = -1; while ( ++Bank < NumBanks )
//for(Bank = 0; Bank < NumBanks; Bank++)

// These are type casts
/// Frees the memory and sets the pointer to zero.
#define Fr(A) (free(A),A=0)
#define Str sprintf
/// Strings are equal if they return zero.  Case insensitive.
#define Eq !stricmp
#define FnC(C) SetTextColor(DC,C);

/// DirA declares the data structures for getting a directory listing. 
#define DirA WIN32_FIND_DATA FF_Rec;HANDLE FF_List;
/// This gets the directory listing:
#define DirI(F,S) _DirI(F,S,&FF_Rec,&FF_List)
/// Returns true if the filename is a directory:
#define IsDir (FF_Rec.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
#define Next FF_Rec.cFileName
/// Gets the next filename or returns false.
#define DirC _DirC( & FF_Rec, FF_List )
/// Used to report a quarter name like  Q4_07
#define ConfA YrsQtr + 1, SimYr - 2000
/// This calls the  WinProc,  which is a procedure tied to a window:
#define Call_WinProc {TranslateMessage(&Msg);DispatchMessage(&Msg);}
#define SeTxt SetWindowText
/// Create a window
#define CW CreateWindowExA
/// This creates a proportional font:
#define CFont( Sz ) CreateFont(Sz,0,0,0,700,0,0,0,0,0,0,0,FF_ROMAN,0);

// Some repetitive parameter declarations for report routines
#define RptParDefs char *RptNeu, char *MainTitle, long Center, char *QtrStamp

// The percent that is lost due to overhead in the a sale of premises:
const float PremLossOnSale = .05;
// Some repetitive parameter passing to report routines
#define HdPars RptNeu, MainTitle, Center, QtrStamp
// Some repetitive parameter declarations for report routines
// Move define to GlobalDef.h
//#define RptParDefs char *RptNeu, char *MainTitle, long Center, char *QtrStamp
// Standard footer routine parameters
#define FtPars(L) RptNeu, MainTitle, (L), QtrStamp
// Increments to another field for reporting, wraps around:
#define Skip_pFld CFld = ( 1 + CFld ) % MaxFields
#define Inc_pFld StrA[ Skip_pFld ]

#define FA( F) _FA( F,0)

typedef float RC_T[21][8];

// On 1/7/2007 Per Erine request, change BaseSimYear from 2007 to 2009 (change made on 1/7/2007)
// On 1/8/2012, Bill would update the 2009 to 2012 for BaseSimYear 
// On 10/23/2013, update the BaseSimYear from 2012 to 2014
// For debugging purpose for G community, change back to 2012 for now. 
enum
{
	RECSIZE = 0x400, MaxFloats = 256, RateSz = 45, BaseSimYear = 2012, _Blac = RGB(0, 0, 0), 
	_Blu = RGB(0, 0, 255), _Gr = RGB(0, 255, 0), Sz_Ptr = sizeof(LnP), _Ye = RGB(255, 255, 0), _Re = RGB(255, 177, 0)
};

enum
{
  Chi = WS_CHILD, Vis = WS_VISIBLE, EdS = WS_THICKFRAME|Chi, MaS = Vis|WS_MINIMIZEBOX|WS_SYSMENU, EoQ = 0, BoQ = 1, MaxComm = 40, Qtrs = 4, MaxClient = 46, MaxExpires = 20, MaxCharCols = 122, MaxFields = 420, FldSz = 46, Read_All_Coms = 0, Update_All_Coms = 1, B501aR = 11, B501aC = 10, B501bR = 7, B501bC = 8, B502R = 26, B502C = 4, Max423Cols = 9, Max423Rows = 12, MaxBufCols = 140, MaxLines = 54
};

enum // used in drawing buttons
{
  bLM = 24, bTM = 10
};

/// <summary> All the decision forms. </summary>
/// <remarks> secpur(security purchase) = 0, secsal(security sale)=1, loans=2, lnprsl()=3,credadm(credit administration)=4
/// depos(deposit)=5, treas=6 , genadm(general administration)=7, MaxForms=8 </remarks>
enum DecisionForm
{
	/// = 0; security purchase decision form
  secpur, 
	/// = 1; security sale decision form
	secsal, 
	/// = 2; loans decision form
	loans, 
	/// = 3; Mortgage banking and loan sales decision form
	lnprsl, 
	/// = 4; Credit administration decision form
	credadm, 
	/// = 5; Deposits decision form
	depos, 
	/// = 6; Treasury management decision form
	treas, 
	/// = 7; General administration decision form
	genadm, 
	/// = 8; Max number of decision forms
	MaxForms
}; 

/// These are enumeration for input fields determined 
/// RE, CO, MO, EX are genenal policy. 
enum
{
   /* Type 1 */BL = 0, ST, BD, TX, MB, SV, SF,  /* Type 5 */FRB = 10, CDS, HFF, HFV, PTD, HFR,  /* Type 7 */RE = 20, CO, MO, EX,  /* Type 8 */Y = 30, N,  /* Type 10 */I, C,  /* Type 9 */F, V,  /* Type 11 */NCC = 40, NCT, MMC, MMT, SBC, SBT, TFC, CON, CRE, MFR, RMF, RMV, CHE, CPL, CCC, CIL, MAXSTR
};

enum
{
  MAX_WIDTH = 80, MaxFldsPerForm = 128, MenuCnt = 10 //Opts has char, each has one byte. 
  //MAX_WIDTH = 80, MaxFldsPerForm = 128, MenuCnt = 1+sizeof(Opts)
};

enum
{
  NewCnt, EoQ_Cnt, TransCnt, All_Cnt
};

//const int MaxScrs = 5, MaxRsk = 6, MaxCR = 6;

// OleT Excel, Workbooks, Bank_Book, ComBook, Sheet, Range, Selection; in GlobalVariables
typedef IDispatch *OleT;

///Data structure for field in the decision entry form.  
typedef struct _FldsT
{
  long x, y, Len, Dot, FldType;
  float Float, F2;
  struct _FldsT *UpLnk,  *DwnLnk;
} FldsT;

/// <summary> This is the list of all deposite types by class and product.  </summary>
enum DepositType //used in Deposit and RunForward
{
  // 0
  DemIndC1PE, DemIndC2PE, DemIndC3PE, DemIndC4PE, DemIndC5PE, DemIndC6PE, DemIndC7PE, DemIndC8PE, DemIndC9PE, DemIndC10PE, DemIndC11PE, DemIndC12PE, DemIndC13PE, DemIndC14PE, DemIndC15PE, DemIndC16PE,
  // 16
  DemPubC1PE, DemPubC2PE, DemPubC3PE, DemPubC4PE, DemPubC5PE, DemPubC6PE, DemPubC7PE,
  // 23
  IndNOWC1PE, IndNOWC2PE, IndNOWC3PE, IndNOWC4PE, IndNOWC5PE, IndNOWC6PE, IndNOWC7PE, IndNOWC8PE, IndNOWC9PE, IndNOWC10PE, IndNOWC11PE, IndNOWC12PE, IndNOWC13PE, IndNOWC14PE, IndNOWC15PE, IndNOWC16PE,
  // 39
  IndMonC1PE, IndMonC2PE, IndMonC3PE, IndMonC4PE, IndMonC5PE, IndMonC6PE, IndMonC7PE, IndMonC8PE, IndMonC9PE, IndMonC10PE, IndMonC11PE, IndMonC12PE, IndMonC13PE, IndMonC14PE, IndMonC15PE, IndMonC16PE,
  // 55
  IndSaveC1PE, IndSaveC2PE, IndSaveC3PE, IndSaveC4PE, IndSaveC5PE, IndSaveC6PE, IndSaveC7PE, IndSaveC8PE, IndSaveC9PE, IndSaveC10PE, IndSaveC11PE, IndSaveC12PE, IndSaveC13PE, IndSaveC14PE, IndSaveC15PE, IndSaveC16PE,
  // 71
  BusNatDemC1PE, BusNatDemC2PE, BusNatDemC3PE, BusNatDemC4PE, BusMidDemC1PE, BusMidDemC2PE, BusMidDemC3PE, BusMidDemC4PE, BusLocDemC1PE, BusLocDemC2PE, BusLocDemC3PE, BusLocDemC4PE, BusReDemC1PE, BusReDemC2PE, BusReDemC3PE, BusReDemC4PE, BusProDemC1PE, BusProDemC2PE, BusProDemC3PE, BusProDemC4PE, BusGenDemC1PE, BusGenDemC2PE, BusGenDemC3PE, BusGenDemC4PE,
  // 95
  BusNatSaveC1PE, BusNatSaveC2PE, BusNatSaveC3PE, BusNatSaveC4PE, BusMidSaveC1PE, BusMidSaveC2PE, BusMidSaveC3PE, BusMidSaveC4PE, BusLocSaveC1PE, BusLocSaveC2PE, BusLocSaveC3PE, BusLocSaveC4PE, BusReSaveC1PE, BusReSaveC2PE, BusReSaveC3PE, BusReSaveC4PE, BusProSaveC1PE, BusProSaveC2PE, BusProSaveC3PE, BusProSaveC4PE, BusGenSaveC1PE, BusGenSaveC2PE, BusGenSaveC3PE, BusGenSaveC4PE,
  // 119
  BusNatTimeC1PE, BusNatTimeC2PE, BusNatTimeC3PE, BusNatTimeC4PE, BusMidTimeC1PE, BusMidTimeC2PE, BusMidTimeC3PE, BusMidTimeC4PE, BusLocTimeC1PE, BusLocTimeC2PE, BusLocTimeC3PE, BusLocTimeC4PE, BusReTimeC1PE, BusReTimeC2PE, BusReTimeC3PE, BusReTimeC4PE, BusProTimeC1PE, BusProTimeC2PE, BusProTimeC3PE, BusProTimeC4PE, BusGenTimeC1PE, BusGenTimeC2PE, BusGenTimeC3PE, BusGenTimeC4PE,
  // 143
  TimeIndC1PE, TimeIndC2PE, TimeIndC3PE, TimeIndC4PE, TimeIndC5PE, TimeIndC6PE, TimeIndC7PE, TimeIndC8PE, TimeIndC9PE, TimeIndC10PE, TimeIndC11PE, TimeIndC12PE, TimeIndC13PE, TimeIndC14PE, TimeIndC15PE, TimeIndC16PE,
  // 159
  TimeHotCPE
};

//used in UI Menu
#endif 
