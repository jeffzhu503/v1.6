// Excel automation error codes:  20003  MemberNotFound,  20006  UnKnownName,  20009  Exception

// BMSim.VCProj turns all warnings on, then selected warnings are turned off here:
//#pragma warning( disable: 4003 4005 4010 4028 4033 4047 4051 4100 4057 4101 4127 4133 4136 4189 4201 4204 \
//4214 4220 4244 4305 4390 4508 4514 4554 4611 4701 4702 4706 4709 4710 4715 4761 )

#pragma warning( disable: 4305 4244 4508)
// This makes  Windows.H  smaller, and makes the list of  #include's more explicit:
#define WIN32_LEAN_AND_MEAN
// This undo's what  Windows.H  did:
//#pragma warning( disable: 4035 4214 )
//#include <stdio.h>
//#include <string.h>
//#include <ole2.h>

#include "StartingPoint.h"
#include "OleHelpers.h"
#include "GlobalVariables.h"
#include "UIVariables.h"
#include "UI.h"  //All_Com()
#include "BMSim.h"
#include "FileIOHelper.h"

// On 1/28/2011, move MaxStPrAtr - MaxMC into constants.h SecKT

// All extern variables from GlobalVariables.h
/// BMSim's main set of global variable: Communities
extern ComT Com[MaxComm]; 
/// BMSim's main set of global variable: Book of Accounts
extern BOAT BOA[MaxB]; 
/// BMSim's main set of global variable: Year to Date income data.
extern YTDT YTD[MaxB]; 

//Used in OleHelpers
//Ole Excel
extern OleT Excel, Workbooks, Bank_Book, ComBook, Sheet, Range, Selection;
extern VARIANT OleRV, Var;

//May move into RunForward and be local variables
extern char Mess[400], _T[400];
extern long rv;




//  These are the same products as reported on B401
// BusDD_N,BusDD_M,BusDD_L,BusDD_O,InDD,PubDDR,NowR,BusSavR,InSav,MMR,TimeR,B401_C

// -------- Start of the  EC.DAT  file.

//const int MaxLnTypes = 20; // 16 loan types are used, but room is made for 20 in the EC.DAT file.

// ------- Start of the  SF.DAT  file.

// These are the 5 types of premises: 
// Never used in the program and they are commented out now
// typedef float PremisesT[5];

// #define MaxTypsSold 6
//  Srv, SrvFix, SrvVar[MaxTypsSold];
//}; // SF.DAT [ 4-9.2 Used 10,11 Reserved ]

/// some constants for displaying input's pages.  
//char *ScrList[] = { "SECPUR" . . .

// OPEN   BK01.DAT [ 10, 11, 12, 13, 14 ]      Unused sectors

// BK01.DAT [25,26,27] Open

// COSTS -- Globals

// Just Used 33,34,35,36.    37 is Open.

// Limits X to a number between L and W inclusively.

////////////////////////////////////////////////////////////////////////////////////
// The following section has all of global variables used across multiple functions
////////////////////////////////////////////////////////////////////////////////////
/// the 10 possible economies.
EconT EconList[10]; 
/// The main data for a set of economies.
EnvT Env, LocEnv; 
/// The main data for a set of economies by the start and end of a quarter.
ECNT EconBOQ, EconEOQ; 
/// 8 quarters of economy data history.
EconQtrT EconQtr8[8]; 
/// Prices in the community
PricesT Prices; 
/// Securities constants
SecKT SecK; 
/// Resourse management constants
ResKonstT ResKonst;
/// Constants for deposits
DepKT DepK; 
/// Constants for treasuries
TreasKT TreasK; 
/// Constants for Loans
LoanKonstT LoanKonst; 
/// Constants for Cost accounting
CostKonstT CostKonst; 
/// Loan pakages
LnPkgT LnPkg[MaxB][MaxLnPkgs]; 
/// Loans sales
LnSalesT LnSales[MaxB][MaxLnSales]; 
/// Globals for securities
SecGloT SecGlo[MaxB]; 
/// The securities portfolio
SecT Sec[MaxB][MaxSecs]; 
/// Funds management globals
FundsGloT FundsGlo[MaxB]; 
// The list of bools which is set to true if FundsGlo.IssuePlanAmnt has non zero value
bool FundsGloHasIssuePlan[MaxB];
/// Resourse management globals
ResGloT ResGlo[MaxB]; 
/// Globals for loans
LoanGloT LoanGlo[MaxB]; 
/// Reporting on loan activities
LoanRptT LoanRpt[MaxB]; 
/// actual deposits, non-time
DepT Dep[MaxB];
/// actual time deposits
TimeDepT TimeDep[MaxB][MaxTimeDep]; 
/// Reporting on deposit activities
DepOutT DepOut[MaxB]; 
/// Globals for Cost accounting
CostsGloT CostsGlo[MaxB]; 
/// Reporting on Cost accounting
CostsReportT CostsReport[MaxB]; 

SecPurDecT SecPurDec[MaxB];
SecSaleDecT SecSaleDec[MaxB];
LoanDecT LoanDec[MaxB];
SellLnsDecT SellLnsDec[MaxB];
CredDecT CredDec[MaxB];
DeposDecT DeposDec[MaxB];
TreasDecT TreasDec[MaxB];
AdminDecT AdminDec[MaxB];
/// The actual decisions of security purchase for a quarter

/// Data structure to license BMSim
struct LicenseT
{
  char Client[MaxClient + 2], Expires[MaxExpires + 2];
};
/// The license to run BMSim
LicenseT License; 

long PrivBkng[MaxB]; //also used in deposit and expense
float FHLB_Avail_BoQ[MaxB], FHLB_Avail_EoQ[MaxB]; //used in Treasury, Loan, Security
// MktProd_2_B401 maps the Markets below to how they are reported  .
// DemIndMPE, DemPubMPE, IndNOWMPE, IndMonMPE, IndSaveMPE, BusNatDemMPE, BusMidDemMPE, BusLocDemMPE,  BusReDemMPE, BusProDemMPE, BusGenDemMPE,
// BusNatSaveMPE, BusMidSaveMPE, BusLocSaveMPE, BusReSaveMPE, BusProSaveMPE, BusGenSaveMPE, BusNatTimeMPE, BusMidTimeMPE, BusLocTimeMPE, BusReTimeMPE, BusProTimeMPE, BusGenTimeMPE, TimeIndMPE, TimeHotMPE
int MktProd_2_B401[] =  //also used in Deposit
{
  InDD, PubDDR, NowR, MMR, InSav, BusDD_N, BusDD_M, BusDD_L, BusDD_O, BusDD_O, BusDD_O, BusSavR, BusSavR, BusSavR, BusSavR, BusSavR, BusSavR, TimeR, TimeR, TimeR, TimeR, TimeR, TimeR, TimeR,  - 1
};

// The sizes of various structures.
int QtrSz = sizeof(AcT), IncmSz = sizeof(IncomeOutGoT);

/// <summary> This function creates a new quarter at the end of RunForward(). </summary>

/// <summary> This function sets an array of intergers (L4) to locate the right Book of Account. </summary>
void DoL4()
{
  long SimQ = SimQtr;

  for(int intIndexOfCount = 0; intIndexOfCount < 5; intIndexOfCount++)
  {
    L4[intIndexOfCount] = SimQ % 4+4 *(SimQ / 4 != SimQtr / 4);
    SimQ--;
  }
}

void NewQtr()
{
	// Make sure Prices.Qtr will not be larger MaxQtr. (Change on 1/7/2007, made by Gilad)
	int MaxQtr = sizeof(Prices.NewDL)/sizeof(Prices.NewDL[0]);
	if (Prices.Qtr >= MaxQtr) {
		MessageBox(NULL,"You have reached the limit of available quarters for running BMSim","BmSim Limit",0);
		return;
	}
			
  long PrevQtr = SimQtr % 4;
  YrsQtr = ++SimQtr % 4;
  Prices.Qtr++;

  SimYr = BaseSimYear + SimQtr / 4;
  Com[ComI].SimQtr = SimQtr;
  LB
  {
    if( ! YrsQtr)
    {
      if( ! Bank)
        PrevQtr += 4;
      memmove(BOA[Bank] + 4, BOA[Bank], 4 *QtrSz);
      memset(BOA[Bank], 0, 4 *QtrSz);
      memmove(YTD[Bank] + 4, YTD[Bank], 4 *IncmSz);
      memset(YTD[Bank], 0, 4 *IncmSz);
    }
    else
      memmove(YTD[Bank] + YrsQtr, YTD[Bank] + PrevQtr, IncmSz);
    memmove(BOA[Bank] + YrsQtr, BOA[Bank] + PrevQtr, QtrSz);
    {
      AcT *Ac = BOA[Bank] + YrsQtr;
      IncomeOutGoT *Incm = &Ac->IncomeOutGo;
      memset(Incm, 0, IncmSz);
    }
  }
}

int LnSz = sizeof(LoanT);
pLoansT pLoans[MaxB]; // The actual loan portfolio. MaxB is the max bank number = 6. 
_602_T _602; //Variable used in RunForward and various reports. 


char LngDate[18], _LngDate[18];
long NoDumps; 
long FirstEverRun;
/// <summary>This function initializes the community's banks, runs forward one or more quarters, writes out all the reports, and saves the state of the community.</summary>
/// <remarks> This is core function in the BMSim to run each quarter. </remarks>
void RunForward()
{
	extern long _NoDumps, NumRuns; //from UI

	_int64 _Time;
	/// Returns the time in something like microseconds, or even nanoseconds.  _Time stores the result.
	#define G_Time (QueryPerformanceCounter( ( LARGE_INTEGER * ) & _Time ), _Time )

	extern float MarketAmntByType[MaxL]; //from Loans
	//from Deposits
	extern float MkAmntByCl[Ind16E + 1]; 
	extern EconInfoT EconInfo[TimeIndC16PE + 1];
	extern DepTypeInfoT DepTypeInfo[MaxB][BusSaveE + 1];
	extern BankInfoT BankInfo[MaxB][TimeIndC16PE + 1];

  if(SimQtr < 4 && NumRuns == 1)
	NumRuns = 2;

  memset(pLoans, 0, sizeof pLoans);  //J2: Why cleans up pLoans here? 

  // Load data from data files. 
  DatIO("EC", "r", 0, &EconList, sizeof(EconList));
  DatIO("EC", "r", 5, &Env, sizeof(Env));
  DatIO("EC", "r", 6, &EconBOQ, sizeof(EconBOQ));
  DatIO("EC", "r", 7, &EconEOQ, sizeof(EconEOQ));
  DatIO("EC", "r", 8, EconQtr8, sizeof(EconQtr8));
  DatIO("EC", "r", 18, &Prices, sizeof Prices);

  DatIO("SF", "r", 0, &License, sizeof(License));
  DatIO("SF", "r", 4, &LoanKonst, sizeof(LoanKonst));
  DatIO("SF", "r", 12, &ResKonst, sizeof(ResKonst));
  DatIO("SF", "r", 15, &SecK, sizeof(SecK));
  DatIO("SF", "r", 17, &DepK, sizeof(DepK));
  DatIO("SF", "r", 24, &TreasK, sizeof(TreasK));
  DatIO("SF", "r", 26, &CostKonst, sizeof(CostKonst));

  char Warn[1000] = "",  *W = Warn;

  for(Bank = 0; Bank < NumBanks; Bank++) //  LB
  {
	  if( ! LoadBankDecisions(0))
		W += Str(W, "Bank %d has NO NEW DECISIONS.\n", Bank + 1); 
  }

  if(*Warn)
  {
    Str(_T, "%s\nRun Q%d/%02d for Community \"%s\"?", Warn, ConfA, _Com);

    if( ! Ok(_T))
    {
      Str(Mess, "The Run is Canceled");
      MessBox(Mess);
      return ;
    }
  }

  DoL4();
  if( ! EconQtr8[L4[1]].One_Qtr_Treas)
  {
    YrsQtr = --SimQtr % 4;
    Economy();
    YrsQtr = ++SimQtr % 4;
  }

  SetStateNames();

  Copy_Qtr(*PQ, 0);

  {
    //Loop(NumRuns)
	for(int J = 0; J < NumRuns; J++)
    {
      int LastLap = J == NumRuns - 1;
      NoDumps =  ! LastLap || _NoDumps;

      DoL4();
      SetStateNames();

      Str(Mess, "Running Quarter %s",  *PQ);
	  // Jeff2: Disable this for debugging purpose
      // PW_S(Mess);  
      Economy();
      FirstEverRun =  ! EconQtr8[L4[3]].One_Qtr_Treas;

	  LB { Securities(); } 
	  LB { Resources(); }
      Loans();
	  LB {  Treasury(); }
      Deposits();
	  LB {  Expenses(); }
	  LB {  Books(); }

      if(FromInstall)
		Prices.Qtr = 0;
	  //Prices.Qtr is assigned to a huge random number after it goes higher than 11 in the previous quarter. The crash is found by Ernie 
      //April 2009 update with SimQtr == 16 per the request from George as the first quarter is 01/13 now
	  //2013, Bill and the team are creating the new starting point, we no longer need this logic here. 
	  //if(SimQtr == 16)
		// Prices.Qtr = 0;
      
	  float *P = Prices.NewDL[Prices.Qtr][MaxB];
	  memmove(P + B401_C, MarketAmntByType, MaxL *4);
	  memset(P, 0, B401_C *4);
      EconInfoT *e = EconInfo - 1;		
      //Loop(TimeIndC16PE)  //loop 158 times
	  for(int J = 0; J < TimeIndC16PE; J++)
      {
        ++e;
        P[MktProd_2_B401[e->MktProd]] += e->Pref *MkAmntByCl[e->Cls];
      }
      for(Bank = 0; Bank < NumBanks; Bank++)  //LB
      {
        float *P = Prices.NewDL[Prices.Qtr][Bank];
		float *PP = DepOut[Bank]._401[NewC01] - 1;
		P--;
        //Loop(Num_401_Rows - 1)
		for(int J = 0; J < Num_401_Rows - 1; J++)
		{
			if(J == TotBusR01 || J == TotDemR01 || J == TotSavR01)
				PP++;
			else
				*++P = 100 **++PP;
		}
        PP = LoanRpt[Bank].New - 1;
		//Loop(MaxL)
		for(int J = 0; J < MaxL; J++)
			*++P = 100 **++PP;
      }
	  
	  for(Bank = 0; Bank < NumBanks; Bank++) {  Stock(); }
	  for(Bank = 0; Bank < NumBanks; Bank++) {  AvailableFunds(); }

      if(LastLap)
      {
        static char *Qn[] =
        {
          "First", "Second", "Third", "Fourth"
        };

        Str(ForQtr, "For Quarter Ending %s", _LngDate);
        Str(ForQtrs, "For Quarters Ending %s and %s", _LngDate, Date[1]);
        Str(AsOf, "As of End of %s Quarter, %d", Qn[YrsQtr], SimYr);
        Str(EndOf, "At End of %s Quarter, %d", Qn[YrsQtr], SimYr);

        Page = Bank = Rpt_I = 0;
		I980("I980", "Stock Price Analysis", 46, AsOf);

		//ToDo: understand where this code is here
		//It seems that just reset all field into blank before running the report B602 in the main block below
        {
          _602_T &S = _602;
          FeP P = FeP(_602.R) - 1;
          {
             //Loop(NumBanks *S.Rows * S.Cols)
			for(int J = 0; J < NumBanks *S.Rows * S.Cols; J++)
				*++P = UnSet;
          }
          P = FeP(_602.ComN_D) - 1;
          //Loop(2 *S.Rows)
		  for(int J = 0; J < 2 *S.Rows; J++)
			  *++P = UnSet;
        }

#pragma region Excel Ole Section 1
        //  Don't try to launch Excel if we have no community template.
        //  Bank_On and ComOn are set by command-line switches
        //  to create Bank and/or community spreadsheets from templates.
				
        if(fp = fopen("Com.XLS", "r"))
          fclose(fp);
        else
          ComOn = Bank_On = 0;

        if(Bank_On || ComOn)
        {
		  HRESULT hresult;
          hresult = CoInitialize(0);
          CLSID clsid;
          hresult = CLSIDFromProgID(L"Excel.Application", &clsid);
          IUnknown *_Excel;
          hresult = GetActiveObject(clsid, 0, (IUnknown **) &_Excel);
          Excel = 0;

          if(_Excel)
            _Excel->QueryInterface(IID_IDispatch, (void **) &Excel);

          if(Excel)
          {
            //  Try to close all Excel Workbooks, up to 4.
            //Loop(4)
			for(int J = 0; J < 4; J++)
            {
              Var.vt = VT_I4;
              Var.lVal = J + 1;

              Get(Excel, L"Workbooks", 1, Var);
              OleT aBook;
              if( ! (aBook = OleRV.pdispVal))
                break;
              Str(_T, "%s\\%s\\_.XLS", _getdcwd(0, Mess, 400), _Com);
              //ushort S[200];
			  wchar_t S[200];
			  mbstowcs(S, _T, 200);
              Var.vt = VT_BSTR;
              Var.bstrVal = SysAllocString(S);
              VARIANT N, X, Y;
              N.vt = VT_ERROR;
              N.scode = DISP_E_PARAMNOTFOUND;
              X.vt = VT_I4;
              X.lVal = 2 /* xlShared */;
              Y.vt = VT_I4;
              Y.lVal = 3 /* xlOtherSessionChanges */;
              Get(aBook, L"SaveAs", 8, Var, N, N, N, N, N, X, Y);
              Ole_(aBook, L"Close");
              Rel(aBook);
              DeleteFile(_T);
            }

            Ole_(Excel, L"Quit");

            //Review: RelUnk
            //Rel( Excel );
            //Rel( _Excel );

            Rel(Excel);
            RelUnk(_Excel);
          }
					//  Try to launch Excel.
          if(CoCreateInstance(clsid, 0, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void **) &Excel) < 0)
		  {
            Bank_On = ComOn = 0;
			Excel = Workbooks = Bank_Book = ComBook = 0;
			MessBox("=== Warning ===  No XLS files will be created.\n""You must have Excel 97  ( or Higher )");
		  }
        }

        //  Str(Mess,"%5.5s",strchr(asctime(localtime(&(rv=time(0)))),':')+1);
        //  { VARIANT x; x.vt = VT_I4; x.lVal = 1; Put( Excel, L"Visible", 1, x); }
        //  Visible= 1;

        if(Excel)
		{
          Get(Excel, L"Workbooks", 0);
		  Workbooks = OleRV.pdispVal;
		}
        {
          DirA if(DirI("%s\\*.XLS", _Com))
          do //This delete all of Excel files under the community folder. 
          {
            Str(_T, "%s\\%s", _Com, Next);
            unlink(_T);
          }
          while( ! DirC);
        }

        _control87(_MCW_EM, _MCW_EM);   //  Because I don't trust Ole2
#pragma endregion 

        int _601_Page[MaxB],_601_Rpt_I[MaxB]; 
		int _000_Rpt_I[MaxB];
		
		// Fix to 2010-01 - B550 Stock Issue Report Error in May 2009
		LB
		{
			//reasDecT *Trs = TreasDec + Bank;
			FundsGloT *Fns = FundsGlo + Bank;
			FundsGloHasIssuePlan[Bank] = Fns->IssuePlanAmnt; 
		}

        for(Bank = 0; Bank < NumBanks; Bank++) //LB
        {
          Page = 1;
          Rpt_I = 0;
          _Cop(Bank_On, Bank_Book, "Bank", "Bank_%s%d_%s", _Com, Bank + 1, LngDate);
          if(Bank + 1 == NumBanks)
            _Cop(ComOn, ComBook, "Com", "Community_%s", _Com);
		  
		  B001("B001", "Balance Sheet", 74, AsOf);
          B002("B002", "Income Statement", 44, AsOf);
          B021("B021", "Community Balance Sheets", 63, ForQtrs);
          B022("B022", "Community Income Statements", 63, ForQtr);
          B102("B102", "Security Portfolio", 50, AsOf);
          B104("B104", "Security Maturities and Portfolio Activity", 62, AsOf);
          B108("B108", "Financial Markets", 70, EndOf);
          B202("B202", "Loan Activity", 70, ForQtr);
          B204("B204", "Scheduled Loan Maturities", 64, AsOf);
          B205("B205", "Loan Credit Quality", 49, ForQtr);
          B206("B206", "New Loan Credit Quality", 49, ForQtr);
          B208("B208", "Interest Income and Average Rates", 70, ForQtr);
          B210("B210", "Community Loan Information", 63, ForQtr);
          B260("B260", "Loan Profitability Analysis", 60, ForQtr);
          B262("B262", "Loan Losses, Loan Reviews, and Loan Profitability", 60, ForQtr);
          B275("B275", "Mortgage Banking, Loan Sales, and Servicing Portfolio", 60, ForQtr);
          B401("B401", "Core Deposit Balances and Activity", 70, ForQtr);
          B421("B421", "Retail Certificates", 70, ForQtr);
          B423("B423", "Retail Certificates -- Maturing Amounts and Average Rates", 70, AsOf);
          B450("B450", "Community Deposit Markets", 68, ForQtr);
          B460("B460", "Deposit Cost Analysis", 60, ForQtr);
          if(PrivBkng[Bank])
            B465("B465", "Private Banking Account Activities", 60, ForQtr);
          else
          {
            char f[20];
            Str(f, "%s\\%dB465.TXT", _Com, Bank + 1);
            unlink(f);
          }
          B501("B501", "Summary of Purchased Funds", 54, ForQtr);
          B502("B502", "Summary of Housing Finance Borrowing", 70, ForQtr);
          B506("B506", "Purchased Funds Portfolio Listing", 57, ForQtr);
          B530("B530", "Interest Rate Sensitivity and Liquidity: Assets", 60, AsOf);
          B531("B531", "Interest Rate Sensitivity and Liquidity: Liabilities and SUMMARY", 60, AsOf);
          B550("B550", "Bank Capital Markets", 60, ForQtr);
          B600("B600", "Salary Budget Requests", 60, EndOf);
          B601("B601", "Premises Accounting and Utilization", 55, ForQtr);

          /*{
            _602_T &S = _602;
		    
            int GT = S.Tot_Ern_Ass;
            RC_T &RC = S.R[Bank];
            //Loop(S.Rows)
			for(int J = 0; J < S.Rows; J++)
            {
              int Top =  - 1, Bot = J;
              if(Bot == S.Tot_Ern_Ass)
              {
                GT = S.Tot_Fnds;
                continue;
              }
              if(Bot == S.Tot_Invst)
                Top = S.FF_Sold;
              if(Bot == S.Tot_Lns)
                Top = S.BusLns;
              if(Bot == S.Tot_Dep)
                Top = S.DemndDep;
              if(Bot == S.Tot_Fnds)
                Top = S.Borrwd_Fnds;
              if(Top !=  - 1)
              {
                //Loop(S.Cols)
				for(int J = 0; J < S.Cols; J++)
                {
                  int Col = J;
                  //Loop(Bot - Top)
				  for(int J = 0; J < Bot - Top; J++)
                  {
                    int Line = Top + J;
                    float F = RC[Line][Col];
                    if(Bot != GT)
                      Add(F, RC[Bot][Col]);
                    Add(F, RC[GT][Col]);

                    rv = Col == S.AvgBal;
                    if(rv || Col == S.Net_Inc_Exp)
                    {
                      Add(F, S.ComN_D[Line][rv]);
                      if(Bot != GT)
                        Add(F, S.ComN_D[Bot][rv]);
                      Add(F, S.ComN_D[GT][rv]);
                    }
                  }
                }
                continue;
              }
              FeP F = RC[Bot];
              float &Inc = F[S.TotInc], &Exp = F[S.Tot_Exp], &Net = F[S.Net_Inc_Exp];
              Add(F[S.Int_Inc], F[S.FeesChrgs], Inc);
              Add(F[S.Int_Exp], F[S.OpExp], Exp);

              if(Bot == S.LLP)
                Add( - Exp, Net);
              else
                if(Inc == UnSet || Exp == UnSet)
                  Add(Inc, Exp, Net);
                else
                  if(GT == S.Tot_Ern_Ass)
                    Add(Inc,  - Exp, Net);
                  else
                    Add( - Inc, Exp, Net);
            }

            Add(RC[S.Tot_Ern_Ass][S.Net_Inc_Exp] - RC[S.Tot_Fnds][S.Net_Inc_Exp], S.ComN);
            Add(RC[S.Tot_Ern_Ass][S.AvgBal], S.ComD);
          } */
		  //Jeff disable it

          _601_Page[Bank] = Page++;
		  _601_Rpt_I[Bank] = Rpt_I++;
		  //Str(Rpt_Index[Rpt_I++], " %s: %s -- Page %d", RptNeu, MainTitle, Page);
		  B602_Community_Margin(); 
  		  B602(_601_Page[Bank], _601_Rpt_I[Bank]);
          B900("B900", "Economic Report Summary", 56, EndOf);
          B901("B901", "Economic History and Competitive Rates", 70, EndOf);
          _000_Rpt_I[Bank] = Rpt_I ;
		  
		  //Enable I980 and E990 in 2013

		  E990("E990", "Examiner Summary", 70, 0);
		  B000( "B000", "Financial Reports", 50, ForQtr );  
		   
#pragma region Excel Worksheet Funds
		  Get_(Bank_Book, L"Worksheets", "Funds");

          if(Sheet = OleRV.pdispVal)
          {
            SetBankNames(1);
            Str(_T, "For %s", FQ[1]);
            SetA("F2", _T);
            SetA("A2", BankName);
            SetA("A1", BankNeu);
            Rel(Sheet);
          }

//Remove: using procedure _XC() instead.
//#define _XC Str( C,"%c%d",'A'+ ++Col,8+ Bank);

//Remove: using procedure XC() instead;
//#define XC( F) _XC Set(C,F);

//#define Ra( _T, _B) { float T=_T,B= _B; _XC Set(C,!B? 0: 100* T/ B); }

//#define RaI( _T, _B) { float T=_T,B= _B; _XC Set(C,!B? 0: 400* T/ B); }
//#define RaI( _T, _B) { float T=_T,B= _B; _XC(C, Col, Bank); Set(C,!B? 0: 400* T/ B); }

          if(ComBook)
          {
/* currently we are not using Excel as input/output
            Get_(ComBook, L"Worksheets", "Peer");
            if(Sheet = OleRV.pdispVal)
            {
              Str(_T, "Avg %s", _Com);
              SetA("A7", _T);
              char C[4];
              int Bank;
              LB
              {
                int Col =  - 1;

                Str(_T, "%s-%d", _Com, Bank + 1);

                //_XC
								_XC(C, Col, Bank);
					
								SetA(C, _T);

                AcT *Ac = BOA[Bank] + YrsQtr;

                BalT *Bal = &Ac->Bal;

                AssetsT *Ass = &Bal->Assets;

                LiaT *Lia = &Bal->Liabilities;

                //XC(Ass->Total)
								XC(C, Col, Ass->Total, Bank);
				
								//XC(Ass->ffSold - Lia->Funds.FF)
								XC(C, Col, Ass->ffSold - Lia->Funds.FF, Bank);
					
								//Ra(Ass->Loans.Sum - Ass->LLR, Lia->Deposits.Sum)
								Ra(C, Col, (Ass->Loans).Sum - Ass->LLR, (Lia->Deposits).Sum, Bank);
					
								//Ra(Ac->NonPerformLns, Ass->Loans.Sum)
								Ra(C, Col, Ac->NonPerformLns, (Ass->Loans).Sum, Bank);
				
								//XC(LoanYield[Bank])
								XC(C, Col, LoanYield[Bank], Bank);
				
								//XC(Cost_Funds[Bank])
								XC(C, Col, Cost_Funds[Bank], Bank);
					
								IncomeOutGoT *Incm = &Ac->IncomeOutGo;

                {
                  InterestT *Int = &Incm->Interest;

                  IntInT *In = &Int->Income;

                  ExpT *Exp = &Int->Expenses;

                  float *SecI = (FeP) &In->Securities,  *Fees = (FeP) &Incm->Fees,  *OpEx = (FeP) &Incm->Expenses;

                  BalT *Bal = &Ac->AveBal;

                  AssetsT *Ass = &Bal->Assets;

                  SecuritiesT *Sec = &Ass->Securities;

                  LiaT *Lia = &Bal->Liabilities;

                  //RaI(Incm->Interest.Net + Ac->TaxedMuniIntTaxEqiv - SecI[1], ( Ass->ffSold + Ass->Loans.Sum - Ass->LLR + SecA ))
									RaI(C, Col, (Incm->Interest).Net + Ac->TaxedMuniIntTaxEqiv - SecI[1],(Ass->ffSold + (Ass->Loans).Sum - Ass->LLR + SecA), Bank);

                  //Ra(NonIntInc, NIIF)
								  Ra(C, Col, NonIntInc, NIIF, Bank);
				  
								  //Ra(NonIntExp, NIIF)
								  Ra(C, Col, NonIntExp, NIIF, Bank);
				  
								 //RaI(Incm->OpEarnings, Ass->Total)
									RaI(C, Col, Incm->OpEarnings, Ass->Total, Bank);

                  //RaI(Incm->Nets.Income, Ass->Total)RaI(Incm->Nets.Income, Lia->_Equity)
									RaI(C, Col, (Incm->Nets).Income, Ass->Total, Bank);
									RaI(C, Col, (Incm->Nets).Income, Lia->_Equity, Bank);
                }

                float Cat = Ass->Cash + Ass->Securities.Treasuries, Cat2 = Ass->ffSold + Ass->Securities.Munis + Ass->Securities.Mort, _Cat3 = Ass->Loans.Loans[10] + Ass->Loans.Loans[11], AT = Ass->Total, Cat4 = AT - (Cat + Cat2 + _Cat3), Cat3 = _Cat3 + .2 * Ac->SwapRisk, RAA = .2 * Cat2 + .5 * Cat3 + Cat4, T2 = min(.0125 *RAA, Ass->LLR), Equ = Lia->_Equity, Cap = Equ + Lia->SubLTD + T2;
                
								//Ra(Equ, AT)
								Ra(C, Col, Equ, AT, Bank);
					
								//Ra(Cap, RAA)
								Ra(C, Col, Cap, RAA, Bank);
					
								//XC(Incm->Nets.Income / (Lia->Stock.Common / PAR))
								XC(C, Col, Incm->Nets.Income / (Lia->Stock.Common / PAR), Bank);

								//XC(Ac->Div)
								XC(C, Col, Ac->Div, Bank);

                float P = Ac->StkPrice;

                //XC(P / Ac->_Book)
								XC(C, Col, P / Ac->_Book, Bank);
				
								//XC(P)
								XC(C, Col, P, Bank);
              }
              Rel(Sheet);
            }

            _Dwn(ComBook);
*/
          }
#pragma endregion

#pragma region Excel Worksheet CoreDep

          Get_(Bank_Book, L"Worksheets", "CoreDep");

          if(Sheet = OleRV.pdispVal)
          {
            // Pointers to advance through all products and classes.
            BankInfoT *b = BankInfo[Bank] - 1;
            EconInfoT *e = EconInfo - 1;
            BusDepClKT *ClsK = DepK.BusNatClK - 1;

            static char Let [] = { 'D', 'F', 'H', 'J', 'L' };
            const int Cols = sizeof Let ;
            float T [ Cols ],  Premium [ 2 ] = { 0, 0 }, Low [ 2 ];

            {
              //Loop(BusGenSaveC4PE + 1)
			  for(int J = 0; J < BusGenSaveC4PE + 1; J++)
              {
                int ClsProdI = J;
                ++b;
                ++e;

                // A NOW or individual  demand deposit.
                int NOW_Ind_NSF = e->MktProd == DemIndMPE || e->MktProd == IndNOWMPE;

                float Bal = * b->Bal, Num = * b->Cnt ;
                //  B,  D and A  are the past,  current and next product,
                //  as reported: 
                //     BusDD_N, BusDD_M, BusDD_L, BusDD_O, InDD, PubDDR
                //     , NowR, BusSavR, InSav, MMR, TimeR, B401_C
                //   MktProd_2_B401 maps the Markets to that reporting.
                int B = ! ClsProdI ? 0 : MktProd_2_B401 [ e [ -1 ].MktProd ]
                , D = MktProd_2_B401 [ e->MktProd ]
                , A = MktProd_2_B401[ e[ 1].MktProd];
                if ( ! ClsProdI || B != D ) 
                  Zero( T ); 
                //  Totals a Row_B401 of activity for each class.
                //  Num is the number of accounts in a class.
                T [ 0 ] += Num, T [ 1 ] += Bal ;
                //  Average Monthly Items.
                T [ 2 ] += Num * ( NOW_Ind_NSF ? e->MoCnt1 : e->MoCnt1 + e->MoCnt2 );
                //  Total NSFs.
                T [ 3 ] += 3 * Num * ( NOW_Ind_NSF ? e->MoCnt2 : -1 );
                //  Number of Deposits.
                T [ 4 ] +=  3 * Num * e->MoCnt2 ;

               int MoneyMarket = e->MktProd == IndMonMPE;

                // If this class keeps the balance that the bank requires for the premium rate.
                if(MoneyMarket && e->MinBal >= DepTypeInfo[Bank][IndMonE].MinBal)
                {
                  Premium[0] += Num;
                  Premium[1] += Bal;
                }

                if(D != A)
                {
                  Num = max(.0001,  *T);
                  //  Divides the row of totals by the number of all accounts in a class to get the average.
				  int V []= { Num, Mill * T [ 1 ] / Num, T [ 2 ] / Num, T [ 3 ], T [ 4 ] }; 
				  if(MoneyMarket) 
				  {  
					  Low [ 0 ] = Num -  Premium [ 0 ];
					  Low [ 1 ] =  Mill * ( T [ 1 ] - Premium [ 1 ] ) / Low [ 0 ];
					  Premium [ 1 ] =  Mill * Premium [ 1 ] / Premium[ 0 ]; 
				  }
				  static int Row [] = { 8, 9, 10, 11, 13, 14, 16, 18, 19, 20 };
				  //Loop( Cols ) 
				  for(int J = 0; J < Cols; J++)
				  { 
                    int Column = J, _Let = Let [ Column ];
                    if ( Column == 2 && D > NowR ) 
                       break;
                    if ( D == InDD || D == NowR ) { if ( _Let == 'L' ) break ; }
                    else if ( _Let == 'J' )  continue;
					
                    //Loop(MoneyMarket ? 3 : 1)
                    for(int J = 0; J < MoneyMarket ? 3 : 1; J++)
					{
                      if(J)
                        V[Column] = J == 1 ? Premium[Column]: Low[Column];
                      char C[4];
                      Str(C, "%c%d", _Let, Row[D] + J + 3);
                      Get_(Sheet, L"Range", C);
                      Range = OleRV.pdispVal;
                      Var.vt = VT_I4;
                      // Places  V[ Column ]  in a cell.
                      Var.lVal = V[Column];
                      Put(Range, L"Value", 1, Var);
                      Rel(Range);
                    }
                  }
                }
              }
            }
            Rel(Sheet);
          }
#pragma endregion

#pragma region Excel Worksheet Share
          Get_(Bank_Book, L"Worksheets", "Share");

          if(Sheet = OleRV.pdispVal)
          {
            int SimQ = SimQtr - Prices.Qtr;
            //Loop(Prices.Qtr + 1)
			for(int J = 0; J < Prices.Qtr + 1; J++)
            {
              int YrsQ = SimQ % 4, SimY = BaseSimYear + SimQ++ / 4-2000;
			  Str(_T, "Q%d/%02d", YrsQ + 1, SimY);
              char C[4];
              Str(C, "%c3", 'E' + J);
              SetA(C, _T);

              float *P = Prices.NewDL[J][ ! J ? 0 : Bank] - 1, *PP = Prices.NewDL[J][MaxB] - 1; // VARIANT VarF; VarF.vt = VT_BSTR; VarF.bstrVal = SysAllocString( L"0.0%");  Put( Range, L"NumberFormat", 1, VarF);
              int R = 8;
              float X, N = 0, D = 0, NN = 0, DD = 0;

              //Loop(B401_C + MaxL)
			  for(int J = 0; J < B401_C + MaxL; J++)
              {
                int JJ = J, _J = J - B401_C;
                //Loop(3)
				for(int J = 0; J < 3; J++)
                {
                  if(!J)
                  {
                    X =  *++P /  *++PP;
                    N +=  *P;
                    D +=  *PP;
                    NN +=  *P;
                    DD +=  *PP;
                  }

                  Str(C + 1, "%d", R++);

                  if(_isnan(X))
                    SetA(C, "NMF");
                  else
                  {
                    Get_(Sheet, L"Range", C);
                    Range = OleRV.pdispVal;
                    Var.vt = VT_R4;
                    Var.fltVal = X;
                    Put(Range, L"Value", 1, Var);
                    Rel(Range);
                  }

                  if(JJ == NowR || JJ == MMR || J == 1 && (JJ == PubDDR || JJ == InSav || _J == Trade || _J == Home_equity))
                    R += 1+(_J == Trade || _J == Home_equity), N = D = 0;

                  if(_J == NC_Term || _J == MM_Term || _J == Multi_family)
                  {
                    R++;
                    break;
                  }

                  if(J == 1 && JJ == TimeR)
                  {
                    R += 4;
                    break;
                  }

                  if(JJ == TimeR || J == 1 && _J == Installment)
                    R++, X = NN / DD, NN = DD = 0;

                  else if(_J == Trade)
                    X = NN / DD;

                  else if(JJ != BusDD_O && JJ != PubDDR && JJ != InSav && _J != Trade && _J != Home_equity && _J != Installment)
                    break;

                  else
                    X = N / D, N = D = 0;

                  if(J == 1 && _J != Installment)
                    break;
                }
              }
            }

            Rel(Sheet);
          }
#pragma endregion

          StartingDecs();
          SetBankNames(1);
          PrnDecs(0);
          _Dwn(ComBook);
          _Dwn(Bank_Book);
        } //End of LB of reporting
		
		//2013 B602 requires a separate LB loop, after  B602_Community_Margin
		LB
		{
			B602(_601_Page[Bank],  _601_Rpt_I[Bank]);
		}

        XL_Dwn();
	}
  }

  //2013 We need to make a copy of each E990.txt and rename it to B990.txt in order to print them in the student package
  CopyE990(); 
    extern float AttS[MaxB][MaxStPrAtr + 1]; //from Stock()
    memmove(Prices.SPA, AttS, sizeof AttS);

#pragma region Write Stock Attributes in S.txt
    if( ! NoDumps)
    {
      ODF("s", "txt", "w");
      NumLen = 10;
      DotLen = 3;

      fprintf(fp, "\n%s\n\nCommunity %c\n\nSimQtr %s\n\n""Stock Attributes\n----------------\n\n\n\n ", TimeStamp, FromCom ? FromCom : Community, FQ[0]);

      for(Bank = 0; Bank < NumBanks; Bank++) //LB
      {
		  extern float Att[MaxB][MaxStPrAtr]; //from Stock()

        SetBankNames(1);
        fprintf(fp, "\nBank %d (One-Based) \"%s\"\n\n", Bank + 1, BankName);
        {
          //Loop(10)
	   	  for(int J = 0; J < 10; J++)
			  fprintf(fp, "%s", FA(Att[Bank][J]));
        }
        fprintf(fp, "\n\n\n     ");
        {
          //Loop(MaxStPrAtr - 10)
		  for(int J = 0; J < MaxStPrAtr - 10; J++)
			  fprintf(fp, "%s", FA(Att[Bank][10+J]));
        }
        fprintf(fp, "\n\n\n");
      }
      fprintf(fp, "\xc\nModified Attributes\n\n");
      {
        float T[] =
        {
          0, 0, 0, 0, 0, 0
        };
        //Loop(MaxStPrAtr)
		for(int J = 0; J < MaxStPrAtr; J++)
        {
			for(Bank = 0; Bank < NumBanks; Bank++) {  fprintf(fp, "%3.3f\t", AttS[Bank][J]), T[Bank] += AttS[Bank][J]; }
          fprintf(fp, "\n");
        }
		for(Bank = 0; Bank < NumBanks; Bank++) {  fprintf(fp, "%2.2f\t", 1+T[Bank]); } 
        fprintf(fp, "\n");
        for(Bank = 0; Bank < NumBanks; Bank++)  //LB
        {
          AcT *Ac = BOA[Bank] + YrsQtr;
          fprintf(fp, "%2.2f\t", Ac->_Book);
        }
        fprintf(fp, "\n");
        for(Bank = 0; Bank < NumBanks; Bank++) //LB
        {
          AcT *Ac = BOA[Bank] + YrsQtr;
          fprintf(fp, "%2.2f\t", Ac->StkPrice);
        }
        fprintf(fp, "\n");
      }
      fclose(fp);
    }
#pragma endregion

    NewQtr();
    PW_E();
	//int closeWin = PW_E();
  }
  
  All_Coms(Update_All_Coms);
  DatIO("EC", "r+", 5, &Env, sizeof(Env));
  DatIO("EC", "r+", 6, &EconBOQ, sizeof(EconBOQ));
  DatIO("EC", "r+", 7, &EconEOQ, sizeof(EconEOQ));
  DatIO("EC", "r+", 8, EconQtr8, sizeof(EconQtr8));
  DatIO("EC", "r+", 18, &Prices, sizeof(Prices));

  for(Bank = 0; Bank < NumBanks; Bank++) //LB
  {
    DatIO("BK00", "r+", 0, BOA[Bank], 8 *QtrSz);
    DatIO("BK00", "r+", 8, YTD[Bank], 8 *IncmSz);
    DatIO("BK00", "r+", 15, LnPkg + Bank, sizeof(*LnPkg));
    DatIO("BK00", "r+", 16, LnSales + Bank, sizeof(*LnSales));
    DatIO("BK00", "r+", 17, Sec + Bank, sizeof(*Sec));


    FundsGloT *F = FundsGlo + Bank;

    F->Dirty = 0;

//    F->_ID = G_Time;
    F->_ID = (QueryPerformanceCounter( ( LARGE_INTEGER * ) & _Time ), _Time );

    F->SimQtr = SimQtr;

    Str(F->Banner, "Start of %s,  Community %s,  Bank %d,  %s", FQ[1], _Com, Bank + 1, Com[ComI].BankName[Bank]); //88

    DatIO("BK00", "r+", 21, F, sizeof *F);
    DatIO("BK00", "r+", 24, LoanGlo + Bank, sizeof(*LoanGlo));
    DatIO("BK00", "r+", 28, ResGlo + Bank, sizeof(*ResGlo));
    DatIO("BK00", "r+", 30, CostsGlo + Bank, sizeof(*CostsGlo));
    DatIO("BK00", "r+", 38, Dep + Bank, sizeof(*Dep));
    DatIO("BK00", "r+", 40, TimeDep + Bank, sizeof(*TimeDep));

    Str(_T, "%s\\BK0%d.IN", _Com, Bank + 1);

    Cop(DatFile, _T);

    CloseLoans();
  }

  CopyForms(*PQ);

  Copy_Qtr(FQ[1], 0);

  int i = 0;

  if(FromCom)
    i += Str(Mess, "( From Community %c )\n\n", FromCom);

  Str(Mess + i, "Done. In community \"%c\"\n\nAt end of %s", Community, FQ[0]);

  MessBox(Mess);
}

/// <summary> This function MakeDats() initializes the variables to create the starting point, which loads all data into the system, before students run the first quarter.  
/// This contains humongous predefined values for various data structures in BMSim.  For example LoanKonstT. </summary> 
/// This function is now moved to a separated file
/// 1/20/2011, Jeff seperates all of structure initiatives into StartingPoint.cpp
/// And the data can be loaded from txt files
/// <summary> This function initializes the variables to create the starting point, which loads all data into the system, before students run the first quarter.  
/// This contains humongous predefined values for various data structures in BMSim.  For example LoanKonstT. </summary> 
void MakeDats()
{
  // --------- SF()
	int BuffSize;  //ToDo: investigate where this variable is initialized. 
	//Read the original data from StartingPoint.cpp
	LoanKonstT LoanKonst;
	LoanKonst = GetLoanKonst();

	ResKonstT ResKonst;
	ResKonst = GetResKonst();

	CostKonstT CostKonst; 
	CostKonst = GetCostKonst(); 

	SecKT SecK;
	SecK = GetSecK(); 

	DepKT DepK;
	DepK = GetDepK(); 

	TreasKT TreasK;
	TreasK = GetTreasK();

	EconT EconList[10];
	GetEconList(EconList, 10);

	EnvT Env;
	Env = GetEnv();

	PricesT Prices;
	Prices = GetPrices();

	LowerEconT LowerEcon; 
	LowerEcon = GetLowerEcon();

	AcT AccountsI;
	AccountsI = GetAccounts();

	SecT Sec[12];
	GetSec(Sec);

	LoanGloT LoanGlo;
	LoanGlo = GetLoanGlo();

	//LoanT Loans[64];
	//GetLoan(Loans);

	#pragma region LoanT
	LoanT Loans[] =
  {  //Changelist: Jan 23, 2006 update Loans for Community 0 per Chip's request. 
    {
      1, 12, 2, 6, 0, 2, 1.8, 0, 1.5, 0, 0, 0, 2, 20, .5, 0, 1, 0
    }
    ,
    {
      1, 2.4, 5, 6, 0, 3, 3.2, 0, 2.5, 0, 0, 0, 0, 4, .6, 0, 2, 0
    }
    ,
    {
      2, 12, 12, 7, 1, 2, 2, 0, 2.5, 0, 1, 0, 0, 0, 0, 1, 3, 0
    }
    ,
    {
      2, 6.0, 15, 7.9, 1.5, 3, 2.6, 0, 3, 1.5, 4, 2, 0, 0, 0, .4, 4, 0
    }
    ,
    {
      3, 10, 4, 8.5, .5, 3, 3.7, 0, 1.2, 0, 0, 0, 1.5, 15, .8, 0, 5, 0
    }
    ,
    {
      3, 5.0, 6, 6.5, .5, 3, 2.9, 0, 1, 0, 0, 0, 1.5, 10, .5, 0, 6, 0
    }
    ,
    {
      //4, 10, 20, 9.4, 1.5, 3, 3, 0, 1.5, 0, 1, 0, 0, 0, 0, .5, 7, 0
			4, 10, 10, 9.4, 1.5, 3, 3, 0, 1.5, 0, 1, 0, 0, 0, 0, 1.0, 7, 0
    }
    ,
    {
      4, 12, 6, 8, 2, 4, 4, 0, 1.8, 4, 6, 6, 0, 0, 0, 2, 8, 0
    }
    ,
    {
      5, 10, 7, 8, 2, 4, 4, 0, .3, 0, 0, 0, 2, 10, 1, 0, 9, 0
    }
    ,
    {
      5, 30, 2, 10, 2, 3, 3.2, 0, .5, 0, 0, 0, 2, 12, .8, 0, 10, 0
    }
    ,
    {
      //6, 10, 20, 9.8, 3, 2.4, 2, 1, .54, 8.2, 18, 18, 0, 0, 0, .5, 11, 0
			6, 10, 5, 9.8, 3, 2.4, 2, 1, .54, 8.2, 18, 18, 0, 0, 0, 2, 11, 0
    }
    ,
    {
      //6, 50, 5, 8.5, 0, 6, 5.8, 0, .4, 4.1, 5, 5, 0, 0, 0, 10, 12, 0
			6, 40, 5, 8.5, 0, 6, 5.8, 0, .4, 4.1, 5, 5, 0, 0, 0, 8, 12, 0    
    }
    ,
    {
      7, 8, 4, 8.5, 2.5, 5, 4.7, 0, .6, 0, 0, 0, 1.5, 15, .67, 0, 13, 0
    }
    ,
    {
      7, 5, 6, 9.5, 1.5, 3, 2.9, 0, .8, 0, 0, 0, 1.5, 5, 1, 0, 14, 0
    }
    ,
    {
      8, 1.2, 6, 9.5, 3.5, 6, 6.4, 0, 1.2, 0, 0, 0, 0, 3, .4, 0, 15, 0
    }
    ,
    {
      8, 12, 4, 9, 3, 2, 2.4, 0, 1.8, 0, 0, 0, 0, 12, .67, 0, 16, 0
    }
    ,
    {
    //9, 10, 36, 8, 4, 2, 3.5, 0, 5, 0, 1, 0, 0, 0, 0, .25, 17, 0
		  9, 24, 8,  8, 4, 5, 5,   0, 5, 0, 1, 0, 0, 0, 0,   3, 17, 0 
    }
    ,
    {
    //9, 12, 16, 11, 4, 3, 2, 0, 5, 0, 8, 8, 0, 0, 0, .8, 18, 0
			9, 12,  8, 11, 4, 3, 2, 0, 5, 0, 8, 8, 0, 0, 0,1.5, 18, 0 
    }
    ,
    {
    //9, 12, 3, 8, 3, 6, 5.5, 0, 5, 0, 1, 0, 0, 0, 0, 3, 19, 0
			9, 12, 6, 8, 3, 6, 5.5, 0, 5, 0, 1, 0, 0, 0, 0, 2, 19, 0
    }
    ,
    {
    //9, 6, 20, 8, 4, 2, 2, 0, 5, 0, 1, 0, 0, 0, 0, .3, 20, 0
			9, 6, 10, 8, 4, 2, 2, 0, 5, 0, 1, 0, 0, 0, 0, .6, 20, 0
    }
    ,
    {
    //9, 3, 22, 7.5, 3.5, 5, 6.6, .1, 2, 0, 1, 0, 0, 0, 0, .05, 21, 0
			9, 3, 10, 7.5, 3.5, 5, 6.6, .1, 2, 0, 1, 0, 0, 0, 0, .3,  21, 0
    }
    ,
    {
    //10, 5, 28, 8.4, 3.5, 1, 1, 0, 2.4, 0, 4, 4, 0, 0, 0, .18, 22, 0
			10, 5, 5,  8.4, 3.5, 1, 1, 0, 2.4, 0, 4, 4, 0, 0, 0,  1,  22, 0 
    }
    ,
    {
    //10, 8, 34, 10, 4, 3, 3, 0, 2.4, 0, 10, 10, 0, 0, 0, .11, 23, 0
			10, 8, 8,  10, 4, 3, 3, 0, 2.4, 0, 10, 10, 0, 0, 0,   1, 23, 0 
    }
    ,
    {
    //10, 4, 16, 7, 4, 4, 4, 0, 2.4, 0, 4, 4, 0, 0, 0, .05, 24, 0
			10, 4, 4,  7, 4, 4, 6, 0, 2.4, 0, 4, 4, 0, 0, 0,   1, 24, 0 
    }
    ,
    {
    //10, 40, 3, 8.5, 0, 4, 3.6, 0, 2.4, 0, 4, 1, 0, 0, 0, 8, 25, 0
      10, 40, 8, 8.5, 0, 4,   5, 0, 2.4, 0, 4, 1, 0, 0, 0, 5, 25, 0
    }
    ,
    {
      11, 10, 115, 9.5, 0, 3, 3.7, .01, 1.2, 3.5, 0, 0, 0, 0, 0, .13, 26, 0
    }
    ,
    {
      11, 20, 40, 10, 0, 3, 2.9, .02, 1, 1, 0, 0, 0, 0, 0, .571, 27, 0
    }
    ,
    {
      12, 10, 110, 8.0, 1.5, 3, 3, .015, 1.5, 56, 4, 3, 0, 0, 0, .135, 28, 0
    }
    ,
    {
      12, 10, 110, 7.5, 1.5, 3, 3, .015, 1.5, 56, 4, 3, 0, 0, 0, .135, 28, 0
    }
    ,
    {
      12, 18, 60, 8, 2, 2, 2, .02, 1.8, 40, 4, 1, 0, 0, 0, .4, 29, 0
    }
    ,
    {
      13, 10, 12, 7, 3, 2, 1.8, 0, 1.5, 0, 0, 0, 30, 20, .5, 0, 30, 0
    }
    ,
    {
      13, 12, 20, 9, 3, 3, 3.2, 0, 2.4, 0, 0, 0, 20, 10, .6, 0.6, 31, 0
    }
    ,
    {
      14, 5, 12, 8.5, 2.5, 1, 1, 0, 1.2, 0, 5, 5, 0, 0, 0, 0, 32, 0
    }
    ,
    {
      14, 6.5, 10, 9.5, 3.5, 3, 3.6, .05, 1, 0, 1, 1, 0, 0, 0, 0, 33, 0
    }
    ,
    {
    //15, 25, 5, 11, 6, 5, 5.4, .03, 1.5, 0, 0, 0, 20, 10, .5, 5, 34, 0
			15, 25, 5, 11, 6, 5, 6.4, .03, 1.5, 0, 0, 0, 20, 10, .5, 5, 34, 0
    }
    ,
    {
      15, 15, 62, 12, 7.5, 4, 4, .02, 1.8, 0, 0, 0, 20, 18, .7, 0, 35, 0
    }
    ,
    {
      16, 25, 10, 9, 0, 4, 3.7, .005, 2.5, 3, 0, 0, 0, 0, 0, 3, 36, 0
    }
    ,
    {
    //16, 40, 20, 10.5, 0, 5, 4.6, .11, 3, 7, 0, 0, 0, 0, 0, 3.12, 37, 0
			16, 40, 20, 10.5, 0, 5, 4.6, .11, 3, 7, 0, 0, 0, 0, 0, 3.12, 37, 0
    }
  };

#pragma endregion 

	LnSalesT LnSales[6];
	GetLnSales(LnSales, 6);

	ResGloT ResGlo;
	ResGlo = GetResGlo();

	CostsGloT CostsGlo;
	CostsGlo = GetCostsGlo(); 

	DepT Dep;
	Dep = GetDep();

	//TimeDepT TimeDep[22];
	//GetTimeDep(TimeDep, 22);

#pragma region TimeDepT
  TimeDepT TimeDep[] =
  {
    {
      1, HotE, 10, 7.1, 2, 1, 0, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    }
    ,
    {
      2, BusProE, 4, .4, 5, 0, 12, 0, 0, 0,
      {
        10, 20, 30, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    }
    ,
    {
      3, BusLocE, 3, 6, 9, 1, 20, 0, 0, 0,
      {
        0, 10, 40, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    }
    ,
    {
      4, IndE, 5, 5.1, 8, 1, 1500, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      5, IndE, 5, 7.5, 9, 1, 1000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      6, IndE, 3.5, 7.2, 10, 1, 1000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      7, IndE, 4, 10.4, 11, 1, 250, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      8, IndE, 4, 0.1, 12, 0, 500, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      9, IndE, 5, 0, 9, 0, 200, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 30, 0, 40, 0
      }
    }
    ,
    {
      10, IndE, 3, 9.8, 13, 1, 300, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      11, IndE, 3, 7.5, 14, 1, 1000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      12, IndE, 4, .1, 15, 0, 500, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      13, IndE, 3, 10.2, 16, 1, 100, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 20, 30, 40
      }
    }
    ,
    {
      14, IndE, 3, 0, 17, 0, 400, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 60, 30
      }
    }
    ,
    {
      15, IndE, 3, 7.4, 18, 1, 400, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50
      }
    }
    ,
    {
      16, IndE, 2, 8.75, 19, 1, 500, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50
      }
    }
    ,
    {
      17, IndE, 8,  - .1, 10, 0, 500, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50
      }
    }
    ,
    {
      18, IndE, 12, 7.1, 25, 1, 3000, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 50, 30, 0, 0
      }
    }
    ,
    {
      19, IndE, 8,  - .2, 30, 0, 3000, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 0
      }
    }
    ,
    {
      20, IndE, 9, 9.4, 35, 1, 3000, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 20, 60, 0, 0
      }
    }
    ,
    {
      21, IndE, 50, 5.5, 3, 1, 25000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 12, 7, 9, 9, 5, 10
      }
    }
    ,
    {
      22, IndE, 8, 7.5, 25, 1, 25000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 12, 9, 9, 10, 5
      }
    }
    ,
  };
#pragma endregion

	FundsGloT FundsGlo;
	FundsGlo = GetFundsGlo();

	//Read the data from data files from the current directory if avaiable.
	//PrintLoanKonst(LoanKonst);
	//UpdateLoanKonstFromFile(LoanKonst);
	float* pData = &LoanKonst.LNFCQI[0];
	UpdateConstFromFile(pData, "LoanKonst.txt");

	pData = &ResKonst.UpAdj;
	UpdateConstFromFile(pData, "ResKonst.txt"); 

	pData = &CostKonst.OccExp[0];
	UpdateConstFromFile(pData, "CostKonst.txt");

	pData = &SecK.StripBuyYield;
	UpdateConstFromFile(pData, "SecK.txt");

	//Need to validate this code
	pData = &DepK.BusNatClK[0].Portion;
	UpdateConstFromFile(pData, "DepK.txt");

	pData = &TreasK.StkPr.WghtStkPr;
	UpdateConstFromFile(pData, "TreasK.txt");

	pData = &EconList[0].One_Qtr_Treas;
	UpdateConstFromFile(pData, "EconList.txt");

	pData = &Env.BaseRate;
	UpdateConstFromFile(pData, "Env.txt"); 

	pData = &Prices.LnFixPrices[0].Fee;
	UpdateConstFromFile(pData, "Prices.txt");

	pData = &LowerEcon.TaxRate;
	UpdateConstFromFile(pData, "LowerEcon.txt");

	pData = &AccountsI.Bal.Assets.Total;
	UpdateConstFromFile(pData, "AccountsI.txt"); 

	pData = &Sec[0].Type; 
	UpdateConstFromFile(pData, "Sec.txt"); 

	pData = &LoanGlo.LnCnt[0];
	UpdateConstFromFile(pData, "LoanGlo.txt"); 

	pData = &Loans[0].Type;
	UpdateConstFromFile(pData, "Loans.txt");

	pData = &LnSales[0].Amnt;
	UpdateConstFromFile(pData, "LnSales.txt");

	pData = &ResGlo.Cur.Sal.Admin[0]; 
	UpdateConstFromFile(pData, "ResGlo.txt");

	pData = &CostsGlo.Prem.City;
	UpdateConstFromFile(pData, "CostsGlo.txt");

	pData = &Dep.BusNatCl[0].DemBal;
	UpdateConstFromFile(pData, "Dep.txt");

	//pData = &TimeDep[0].Id; 
	TimeDepT *pTimeDep = TimeDep; 
	UpdateConstFromTimeDepFile(pTimeDep, "TimeDep.txt");  //TimeDepT has some unsigned char in the structure

	FundsGloT *pFundsGlo = &FundsGlo; 
	UpdateConstFromFundsGloFile(pFundsGlo, "FundsGlo.txt"); 

  SecPurDecT SecPurDec[1] =
  {
    {
      // BL = 0, ST, BD, TX, MB, SV, SF
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet
      }
      ,
    }
  };

  LoanDecT LoanDec[1] =
  {
    {
      {
        6.5, 7.0, 1.00, 25, 12
      }
      , .50,
      {
        6.75, 7.0, 1.00, 25, 20, 1
      }
      ,
      {
        6.5, 7.0, 1.00, 25, 8
      }
      , .50,
      {
        6.75, 7.25, 1.00, 25, 24, 8
      }
      ,
      {
        6.8, 7.50, 1.00, 30, 8
      }
      , .50,
      {
        6.0, 7.0, 0.00, 30, 24, 24
      }
      ,
      {
        6.5, 7.0, 1.00, 25, 4
      }
      , .50,
      {
        7.0, 7.5, 1.00, 25, 6
      }
      , .50,
      {
        6.5, 7.0, 1.00, 25, 40, 4
      }
      ,
      {
        6.5, 7.0, 1.00, 25, 40, 10
      }
      ,
      {
        5.5, 5.8, 1.00, 40, 100
      }
      ,
      {
        4.5, 5.0, 1.00, 40, 100, 2
      }
      ,
      {
        6.5, 7.0, 1.00, 40, 20
      }
      , 25,
      {
        6.0, 6.5, 1.00, 40, 8, 8
      }
      ,
      {
        8.0, 8, 0, 50, 12
      }
      , 15,
      {
        7.0, 7.5, 0, 60, 12
      }
      ,
      {
        UnSet, UnSet, UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet, UnSet, UnSet
      }
      ,
    }
  };

  SellLnsDecT SellLnsDec[1] =
  {
    {
      UnSet,
      {
        UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet, UnSet
      }
      ,
    }
  };

  CredDecT CredDec[1] =
  {
    {
      {
        {
          20, 20, 20, 20, 20
        }
        , UnSet,
        {
          80, 80
        }
      }
      ,
      {
        {
          20, 20, 20, 20, 20
        }
        , UnSet,
        {
          80, 80
        }
      }
      ,
      {
        {
          20, 20, 20, 20, 20
        }
        , UnSet,
        {
          80, 80
        }
      }
      ,
      {
        50,
        {
          20, 20, 20, 20, 20
        }
        , UnSet
      }
      ,
      {
        50,
        {
          20, 20, 20, 20, 20
        }
        , UnSet
      }
      ,
      {
        80,
        {
          20, 20, 20, 20, 20
        }
        , UnSet
      }
      ,
      {
        80,
        {
          20, 20, 20, 20, 20
        }
        , UnSet
      }
      ,
      {
        {
          20, 20, 20, 20, 20
        }
        , UnSet,
        {
          90, 90
        }
      }
      ,
      {
        80,
        {
          20, 20, 20, 20, 20
        }
        , UnSet
      }
      ,
      {
        60,
        {
          20, 20, 20, 20, 20
        }
        , UnSet
      }
      ,
      {
        80,
        {
          20, 20, 20, 20, 20
        }
        , UnSet
      }
      ,
      {
        100,
        {
          20, 20, 20, 20, 20
        }
        , UnSet
      }
      , .5, EX
    }
  };

  DeposDecT DeposDec[1] =
  {
    {
      //  Business,  Public, Individual
      {
        {
          7.5, 5, 2
        }
        ,  //  Monthly Fee, dollars
        {
          5, 6, 10
        }
        ,  //  Debit Fee, pennies
        {
          1, 1, 10
        }
        ,  //  Deposit Fee, dollars;  NSF Fee, dollars
        {
          5, 5, 3.5
        }
        ,  //  Credit on Balances, dollars
      }
      ,
      {
        // NOW Accounts,  Money Market Accounts
        4, 5,  //  Interest Rate
        10, 4,  //  Item fee, pennies;  Base Interest
        2.5, 1, 10, 1000, 1500,
        {
          4.5, .5
        }
        ,
        {
          4.5, .5
        }
        ,
      }
      ,
      {
        {
          {
            2, 5.0, F
          }
          ,
          {
            20, 6.5, F
          }
          ,
          {
            4, 5.5, F
          }
          ,
          {
            6, 6.1, V
          }
          ,
          {
            8, 5.5, F
          }
          ,
          {
            8, 6.2, V
          }
          ,
          {
            12, 5.7, F
          }
          ,
          {
            12, 6.5, V
          }
          ,
        }
        , 0,
      }
      ,
      {
        50, 10, 20, 55, 50, 40, 50, 50, 10,
      }
    }
  };

  TreasDecT TreasDec[1] =
  {
    {
      {
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
        {
          UnSet, UnSet, UnSet
        }
        ,
      }
      , 0.0, UnSet, 1.250, UnSet, UnSet, UnSet,
    }
  };

  AdminDecT AdminDec[1] =
  {
    {
      {
        50, 15, 50,

        50, 30, 100, 30, 40, 15, 120, 40, 100,

        180, 40, 350, 50, 200,

        80, 200, 20, 400, 0,

        200, 200, 30, 0.0,
      }
      ,
      {
        UnSet, UnSet, UnSet, UnSet
      }
      , 25
    }
  };

  //DepT Dep =
  //{
  //  //   DemBal, SavBal, DemCnt, SavCnt, Loy
  //  {
  //    // BusNatCl[MaxBusC
  //    {
  //      .5, 0, 10, 0, .6
  //    }
  //    ,
  //    {
  //      4, 0, 20, 0, .6
  //    }
  //    ,
  //    {
  //      2, 0, 10, 0, .6
  //    }
  //    ,
  //    {
  //      3, 0, 8, 0, .6
  //    }
  //    ,
  //  }
  //  ,
  //  {
  //    // BusMidCl[MaxBusC
  //    {
  //      .5, 0, 25, 0, .7
  //    }
  //    ,
  //    {
  //      2.7, 0, 30, 0, .7
  //    }
  //    ,
  //    {
  //      2.5, 0, 15, 0, .7
  //    }
  //    ,
  //    {
  //      4, 0, 15, 0, .7
  //    }
  //    ,
  //  }
  //  ,
  //  {
  //    // BusLocalCl[MaxBu
  //    {
  //      1, 0, 100, 0, .8
  //    }
  //    ,
  //    {
  //      3.5, .2, 120, 10, .8
  //    }
  //    ,
  //    {
  //      2.5, .3, 50, 6, .8
  //    }
  //    ,
  //    {
  //      2.5, .4, 30, 3, .8
  //    }
  //    ,
  //  }
  //  ,
  //  {
  //    // BusRECl[MaxBusCl
  //    {
  //      .8, 0, 40, 0, .6
  //    }
  //    ,
  //    {
  //      2.4, .1, 80, 6, .6
  //    }
  //    ,
  //    {
  //      3.0, .15, 35, 3, .6
  //    }
  //    ,
  //    {
  //      2.6, .2, 12, 2, .6
  //    }
  //    ,
  //  }
  //  ,
  //  {
  //    // BusProfCl[MaxBus
  //    {
  //      .4, 0, 30, 0, .6
  //    }
  //    ,
  //    {
  //      1.6, .1, 18, 4, .6
  //    }
  //    ,
  //    {
  //      2.0, .2, 25, 2, .6
  //    }
  //    ,
  //    {
  //      1.2, .2, 8, 2, .6
  //    }
  //    ,
  //  }
  //  ,
  //  {
  //    // BusGenCl[MaxBusC
  //    {
  //      .3, 0, 120, 0, .6
  //    }
  //    ,
  //    {
  //      1.5, 0, 50, 0, .6
  //    }
  //    ,
  //    {
  //      1.8, 0, 30, 0, .6
  //    }
  //    ,
  //    {
  //      1.2, .1, 4, 2, .6
  //    }
  //    ,
  //  }
  //  ,
  //  //  { float Bal; }
  //  //  PubDepClT PubDepCl[MaxPubCls];   //   192
  //  {
  //    {
  //      1.25, 500
  //    }
  //    ,
  //    {
  //      2.5, 250
  //    }
  //    ,
  //    {
  //      2.5, 208
  //    }
  //    ,
  //    {
  //      3.75, 250
  //    }
  //    ,
  //    {
  //      5, 200
  //    }
  //    ,
  //    {
  //      5, 50
  //    }
  //    ,
  //    {
  //      7, 25
  //    }
  //  }
  //  ,
  //  //  DemBal, SavBal, NowBal, MonBal,
  //  //  DemCnt, SavCnt, NowCnt, MonCnt;
  //  //  IndDepClT IndDepCl[MaxIndCls];   //   199 - 254
  //  {
  //    {
  //      2, 2, 0, 0, 8000, 8000, 8000, 0
  //    }
  //    ,
  //    {
  //      6, 2, 0, 4, 8000, 4000, 8000, 4000
  //    }
  //    ,
  //    {
  //      6, 3, 2, 4, 4000, 2000, 4000, 2000
  //    }
  //    ,
  //    {
  //      3, 3, 3, 6, 3000, 1200, 3000, 2000
  //    }
  //    ,
  //    {
  //      1, 3, 3, 5, 3000, 1200, 3000, 2000
  //    }
  //    ,
  //    {
  //      0, 3, 3, 5, 3000, 1200, 3000, 2000
  //    }
  //    ,
  //    {
  //      0, 3, 3, 5, 3000, 1200, 3000, 2000
  //    }
  //    ,
  //    {
  //      0, 3, 3, 5, 3000, 1200, 3000, 2000
  //    }
  //    ,
  //    {
  //      0, 4, 3, 8, 3000, 1200, 3000, 2000
  //    }
  //    ,
  //    {
  //      0, 4, 4, 8, 3000, 1200, 3000, 2000
  //    }
  //    ,
  //    {
  //      0, 4, 4, 8, 3000, 1200, 3000, 2000
  //    }
  //    ,
  //    {
  //      0, 4, 4, 8, 3000, 1200, 3000, 2000
  //    }
  //    ,
  //    {
  //      0, 4, 5, 9, 3000, 1200, 3000, 2000
  //    }
  //    ,
  //    {
  //      0, 2, 6, 9, 600, 50, 600, 200
  //    }
  //    ,
  //    {
  //      0, 1, 6, 10, 400, 10, 400, 400
  //    }
  //    ,
  //    {
  //      0, 1, 6, 11, 400, 10, 400, 300
  //    }
  //    ,
  //  }
  //  ,
  //  {
  //    //   float BnkShr[ Ind7E + 1 ];
  //    .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1,
  //  }
  //  ,
  //};

 /* TimeDepT TimeDep[] =
  {
    {
      1, HotE, 10, 7.1, 2, 1, 0, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    }
    ,
    {
      2, BusProE, 4, .4, 5, 0, 12, 0, 0, 0,
      {
        10, 20, 30, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    }
    ,
    {
      3, BusLocE, 3, 6, 9, 1, 20, 0, 0, 0,
      {
        0, 10, 40, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    }
    ,
    {
      4, IndE, 5, 5.1, 8, 1, 1500, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      5, IndE, 5, 7.5, 9, 1, 1000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      6, IndE, 3.5, 7.2, 10, 1, 1000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      7, IndE, 4, 10.4, 11, 1, 250, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      8, IndE, 4, 0.1, 12, 0, 500, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      9, IndE, 5, 0, 9, 0, 200, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 30, 0, 40, 0
      }
    }
    ,
    {
      10, IndE, 3, 9.8, 13, 1, 300, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      11, IndE, 3, 7.5, 14, 1, 1000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      12, IndE, 4, .1, 15, 0, 500, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      13, IndE, 3, 10.2, 16, 1, 100, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 20, 30, 40
      }
    }
    ,
    {
      14, IndE, 3, 0, 17, 0, 400, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 60, 30
      }
    }
    ,
    {
      15, IndE, 3, 7.4, 18, 1, 400, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50
      }
    }
    ,
    {
      16, IndE, 2, 8.75, 19, 1, 500, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50
      }
    }
    ,
    {
      17, IndE, 8,  - .1, 10, 0, 500, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50
      }
    }
    ,
    {
      18, IndE, 12, 7.1, 25, 1, 3000, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 50, 30, 0, 0
      }
    }
    ,
    {
      19, IndE, 8,  - .2, 30, 0, 3000, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 0
      }
    }
    ,
    {
      20, IndE, 9, 9.4, 35, 1, 3000, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 20, 60, 0, 0
      }
    }
    ,
    {
      21, IndE, 50, 5.5, 3, 1, 25000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 12, 7, 9, 9, 5, 10
      }
    }
    ,
    {
      22, IndE, 8, 7.5, 25, 1, 25000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 12, 9, 9, 10, 5
      }
    }
    ,
  };
*/

  //FundsGloT FundsGlo =
  //{
  //  //  RateAmntT Repo,FRB,FF;
  //  {
  //    0, 0
  //  }
  //  ,
  //  {
  //    0, 0
  //  }
  //  ,
  //  {
  //    0, 0
  //  }
  //  ,
  //  //  struct { float Amnt,Mat,Rate,Duration; } CDs[MaxCD],PTs[MaxPT];
  //  {
  //    {
  //      30, 4, 7, 5
  //    }
  //    ,
  //    {
  //      25, 6, 5.5, 5
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //  }
  //  ,
  //  {
  //    {
  //      20, 5, 5.2, 5
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0
  //    }
  //  }
  //  ,
  //  //  RateAmntT CDsAvail[MaxCDA],
  //  {
  //    {
  //      10, 8
  //    }
  //    ,
  //    {
  //      10, 8
  //    }
  //    ,
  //    {
  //      10, 8
  //    }
  //    ,
  //    {
  //      10, 8
  //    }
  //    ,
  //    {
  //      10, 8
  //    }
  //    ,
  //    {
  //      10, 8
  //    }
  //    ,
  //    {
  //      10, 8
  //    }
  //    ,
  //    {
  //      10, 8
  //    }
  //    ,
  //  }
  //  ,
  //  //  SubLTD_IssuedT SubLTD_IssuedAvail[MaxLTA];
  //  {
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      10, 60, 0, 0, 8, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //  }
  //  ,
  //  //  Open[2], CeditRating
  //  0, 0, 0,
  //  //  struct { float Amnt,Rate,Fixed,Mat,Duration; } HFs[MaxHF];
  //  {
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0
  //    }
  //    ,
  //  }
  //  ,
  //  //  SubLTD_IssuedT SubLTD[MaxLT];
  //  {
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //    {
  //      0, 0, 0, 0, 0, 0
  //    }
  //    ,
  //  }
  //  ,
  //  //  float Open_A[2], HFMem, HFAmnt, Open_B;
  //  {
  //    0, 0
  //  }
  //  , 1, 0, 0,
  //  //  Loans, Sec, SecMat, Stock, Funds, HFs,           SubLTD,     CDs,    PTs, Taxes
  //  {
  //    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  //  }
  //  ,
  //  //  Loans, Sec,         Stock, Funds, HFs, HFsMat, SubLTD_IssuedMat, CDsMat, PTsMat, Taxes
  //  {
  //    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  //  }
  //  ,
  //  //  NetDay1, FundsSold, Repos
  //  0, 0, 0,
  //  //   FRBs, FF, FRBClosed
  //  0, 0, 0,
  //  //   StockAmntRP, SharesRP, ZMatCDs,     IssuePlanAmnt, IssuePricePerShare, IssuePrice, NewUniqueName; SimQtr,Dirty,_ID
  //  0, 0, 0, 0, 0, 0, 0, 3, 1,
  //}; // FundsGloT; // BK01.DAT [21-23]

  //Remove: Actual code below this used.
  //DecVars
  int DecSz[]= { sizeof* SecPurDec, sizeof* SecSaleDec,
	sizeof* LoanDec, sizeof* SellLnsDec, sizeof* CredDec,
	sizeof* DeposDec, sizeof* TreasDec, sizeof* AdminDec };

LnP _DecP[]= { (LnP) SecPurDec, (LnP) SecSaleDec, (LnP) LoanDec,
	(LnP) SellLnsDec, (LnP) CredDec, (LnP) DeposDec, (LnP) TreasDec,
	(LnP) AdminDec };
	//

  {
    char *EconFlds[] =
    {
       "One_Qtr_Treas","Three_Qtr_Treas","Thirty_Year_Treas","\nCPR","FFR","FRDR","\nRPR","LIBOR","Prime"
       ,"\nBondAAA","BondAA","BondA","\nBondBBB","BondBB","BondB","BondSL","X"
       ,"X","X","X","\n\nNat Cedit Line","NatTerm Loan","Mid Mkt CL"
       , "Mid Mkt Term","\nSmall Busi CL","Small Busi Term","Import/Export"
       ,"Construction","\nCommercial RE","Multi Family", "Sing Fam Fixd"
       ,"Sing Fam Var","\nHomeEqu","Personal","Credit Card","Instalmnt Loan"
       ,"X","X","X","X", "\n\nIndiv Demand Dep","NOW Acct","SAV Acct"
       ,"\nMoney Mkt Acct","Time Dep","X","X","X","X","X",  "X","X","X","X","X"
       ,"\n\nINFL","GDP","Leading ECI","\nNat UnEmply","Loc UnEmply"
       ,"New Housing S","New H Permits","Retail Sales", "X","X","\nStock Index"
       ,"CD Index","Ln Loss Index","ECI Nat","ECI Local","LMQI","X","X","X","X"
       , "\n\n         % Annual Growth Rates\n\nNat Corp Loans","Mid Mkt Corp"
       ,"Small Busi","\nImport/Export","Construction", "Commercial RE"
       ,"\nMulti Res RE","Single Res RE","Consumers","X","\n\nNat Acct"
       ,"Mid Mkt Acct", "Loc Busi Acct","\nReal estate","Professional"
       ,"\nOther","Individual","Public Acct","X","X",
    };
    float Scale = 1;
    ODF("E", "TXT", "w");
    fprintf(fp, "                    ");
    {
      Loop(10)
      {
          fprintf(fp,"Econ%2d%s", J + 1, J == 4 ? "\n                    " : "  ");
      }
    }
    fprintf(fp, "\n\n");
    int Smooth_Sz = EconList->DepositMarkets + 10-&EconList->One_Qtr_Treas;//the diff between memory address.
    {
      Loop(Smooth_Sz)
      {
        char *s = EconFlds[J],  *n = strrchr(s, 10);
        long F = J;
        if(*s == 'X')
          continue;
        fprintf(fp, "%s%*s", s, 18-strlen( ! n ? s : n + 1), " ");
        if( ! strncmp(s, "\n\n         ", 7))
          Scale = 100;
        {
          Loop(10)
          {
              fprintf( fp,"%8.2f", Scale * ( (FeP) & EconList [ J ] ) [ F ] );
              if ( J == 4 ) 
                 fprintf( fp, "\n                  ");
          }
        }
        fprintf(fp, "\n");
      }
    }
    fclose(fp);
  }

  Bank = 0;
  int VarAlso = Community == '0';
  char Zeros [ RECSIZE * 10 ] = { 0 };  //  MakeDats()
  //  if (  ) {
  //  AcT AcT * Ac = BOA [ Bank ] + YrsQtr ;  }
  if(VarAlso)
  {
    {
      char c = 0;
      DatIO("BK00", "w", 0, &c, 1);
    }
    {
      Loop(5)DatIO("BK00", "r+", J *10, &Zeros, sizeof(Zeros));
    }
    {
      Loop(8)DatIO("BK00", "r+", J, &AccountsI, sizeof(AccountsI));
    }
    { //Calculate CouponIntr, EconYield, and TaxEquivYield.
      SecT *s = Sec - 1;
      short To = sizeof(Sec) / sizeof(SecT);
      Loop(To)
      {
        s++;
        s->CouponIntr = s->Value.Book *s->CouponYld *.0025;
        if( ! s->EconYield)
          s->EconYield = s->CouponYld;
        s->TaxEquivYield = s->CouponYld;
      }
    }
    DatIO("BK00", "r+", 17, &Sec, sizeof(Sec));
    DatIO("BK00", "r+",  - 1, Zeros, RECSIZE *4-sizeof(Sec));
    DatIO("BK00", "r+", 21, &FundsGlo, sizeof(FundsGlo));
    DatIO("BK00", "r+",  - 1, Zeros, RECSIZE *3-BuffSize);
    DatIO("BK00", "r+", 24, &LoanGlo, sizeof(LoanGlo));
    DatIO("BK00", "r+", 16, &LnSales, sizeof(LnSales));
    DatIO("LN00", "w", 0, &Loans, sizeof(Loans));
    DatIO("BK00", "r+", 28, &ResGlo, sizeof(ResGlo));
    DatIO("BK00", "r+", 30, &CostsGlo, sizeof(CostsGlo));
    {
      Loop(MaxForms)
      {
        DatIO("BK00", "r+", DecSec(J), DecP(J), DecSz[J]);
        DatIO("BK00", "r+", DecSec2(J), DecP(J), DecSz[J]);
      }
    }
    DatIO("BK00", "r+", 38, &Dep, sizeof(Dep));
    DatIO("BK00", "r+", 40, &TimeDep, sizeof(TimeDep));
    DatIO("BK00", "r+",  - 1, Zeros, sizeof(Zeros) - sizeof(TimeDep));
    ODF("QTR", "TXT", "w");
    fprintf(fp, "3  1111\n   \"Bank\"\n");
    fclose(fp);
  }
  DatIO("EC", VarAlso ? "w" : "r+", 0, &EconList, sizeof(EconList));
  if( ! VarAlso)
  {
    static EnvT Old;
    DatIO("EC", "r", 5, &Old, sizeof(Old));
    memmove(Env.MkSize, Old.MkSize, sizeof(Old.MkSize));
    Env.TaxRate = Old.TaxRate;
  }
  DatIO("EC", "r+", 5, &Env, sizeof(Env));
  if(VarAlso)
  {
    DatIO("EC", "r+", 6, Zeros, RECSIZE);
    {
      FeP B = (FeP)(EconList + 9), P = B - 1;
      Loop(55) *++P += Env.BaseRate;
      EconList[9].INFR += Env.InflRateBase;
      DatIO("EC", "r+", 7, B, sizeof(EconT));
    }
    DatIO("EC", "r+",  - 1, &LowerEcon, sizeof(LowerEcon));
    DatIO("EC", "r+", 8, Zeros, sizeof(Zeros));
  }
  if( ! VarAlso)
  {
    static PricesT Old;
    DatIO("EC", "r", 18, &Old, sizeof(Old));
    memmove(Prices.SPA, Old.SPA, sizeof Old.SPA);
  }
  DatIO("EC", "r+", 18, &Prices, sizeof(Prices));
  if(VarAlso)
  {
    struct
    {
      char Client[MaxClient + 2], Expires[MaxExpires + 2];
    } License;
    short Sz = sizeof(License);
    Str(License.Client, "American Bankers Association");
    Str(License.Expires, "1-1-2010");
    {
      short *S = (short*) &License - 1;
      Loop(Sz / 2) *S = ~ * ++S;
    }
    DatIO("SF", "w", 0, &License, Sz);
    DatIO("SF", "r+",  - 1, Zeros, 4 *RECSIZE - Sz);
  }
  DatIO("SF", "r+", 4, &LoanKonst, sizeof(LoanKonst));
  DatIO("SF", "r+",  - 1, Zeros, 8 *RECSIZE - BuffSize);
  DatIO("SF", "r+", 12, &ResKonst, sizeof(ResKonst));
  DatIO("SF", "r+", 15, &SecK, sizeof(SecK));
  DatIO("SF", "r+", 17, &DepK, sizeof(DepK));
  DatIO("SF", "r+", 24, &TreasK, sizeof(TreasK));
  DatIO("SF", "r+", 26, &CostKonst, sizeof(CostKonst));
  {
    char Mess[90];
    Str(Mess, "Community %c\n\nConstant%s Data are now Re-Initialized", Community, VarAlso ? " and Variable" : "-Only");
    Bow(Mess);
  }
}

//Remove: calling actual functions instead.
//#define HideI(W)ShowWindow(W,SW_HIDE);
//#define ShowI(W)ShowWindow(W,SW_SHOW);

// THE FOLLOWING ARE all moved to ui.cpp
//char *ScrBuff,  *FormP,  *pHeader;
//long CurDecForm, CurScr, HeaderSize, ScrCnt, CurMenuSel;
//char Opts[] =
//{
//  'P', 'S', 'L', 'M', 'C', 'D', 'T', 'G', 'R'
//}
//
//,  *Menu[] =
//{
//  "P   Purchase Securities", "S   Sell Securities", "L   Loans", "M   Mortgage Banking And Loan Sales", "C   Credit Administration", "D   Deposits", "T   Treasury Management", "G   General Administration", "R   Print", "E   Exit"
//};
//

//enum
//{
//  MAX_WIDTH = 80, MaxFldsPerForm = 128, MenuCnt = 1+sizeof Opts
//};
//
//
//
//FldsT Flds[MaxFldsPerForm],  *FldP;
//POINTS MouseXY;
//


//extern HWND BT[5];
//extern RECT SubRec, CharRec;
//extern HBRUSH Black, Back;
//extern long Editing;
//extern char FieldBuf[40];
//extern long CurInFld;
//extern long Key, B_Loc, _Col, _Row;




//#define MenuBack ClearScr();GoToColRow(30,2);Say(" DECISION ENTRY "); {Loop(5) ShowWindow(BT[J],SW_HIDE);}//HideI(BT[J])}





//used in ui.cpp and replaced with code
//#define Get_Qtrs char Qtr[64][10];long Q_I=0;{DirA if(DirI("%s\\Q?_??.",_Com))while(1){if(IsDir)strcpy(Qtr[Q_I++],Next);\
//if(Q_I==64||DirC)break;}}while(1){long Done=1;char Q[10];Loop(Q_I-1){\
//if(atoi(Qtr[J]+3)*4+Qtr[J][1]<atoi(Qtr[J+1]+3)*4+Qtr[J+1][1]){Done=0;strcpy(Q,Qtr[J]);strcpy(Qtr[J],Qtr[J+1]);\
//strcpy(Qtr[J+1],Q);}}if(Done)break;}


// Assembles a group of instructor reports from the existing reports.
// BMSim.TXT has the list, e.g.:
// C021 C022 C550 C900 C901 C108 C210 C450
//   B001 B002 -B102 -B104 -B202 -B204 -B205 -B206 -B208 -B260 -B262 -B275
//   -B401 -B421 -B423 -B460 -B501 -B502 -B506 -B530 -B531 -B600 -B601 -B602
//   I980 E990
//

//#include "EconManagement.cpp"