#pragma once

#include "GlobalDef.h"
#include "DecisionFormDataT.h" //for BankInfoT

// Used in BMSimDataT.h
#ifndef UCHAR
typedef unsigned char uchar;
#endif 
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned __int64 u64;
//typedef LARGE_INTEGER Quad;
typedef char *LnP;
typedef float *FeP;

#pragma region LoanKonstT
/// Constants used in the laon prepayment equations:
struct PrePayT
{
  float p, v, a, b, c, d;
};


const int MaxLnTypes = 20; // 16 loan types are used, but room is made for 20 in the EC.DAT file.
const int MaxCQ = 3, MaxPrem = 4;

enum
{
  NC_CL, NC_Term, MM_CL, MM_Term, SB_CL, SB_Term, Trade,
  //         0,       1,     2,       3,     4,       5,     6,
  Construction, Commercial_RE, Multi_family, Fixed_rate, Variable_rate,
  //           7,             8,            9,         10,            11,
  Home_equity, Personal, Credit_card, Installment, MaxL
};

/// Numerous loan constants:
struct LoanKonstT
{
  float LNFCQI[MaxLnTypes];
  struct
  {
    float a, b, c, d;
  } ChrgOffPor[MaxLnTypes];
  struct
  {
    float a, b, c, d;
  } PastDuePor[MaxLnTypes];
  struct
  {
    float a, b;
  } NonAccPor[MaxLnTypes];
  PrePayT PrePay[MaxLnTypes];
  //  float NormPay[MaxLnTypes],
  float MaxMat[MaxLnTypes], RevCA, RevCB, RevCC, CurUsage[MaxLnTypes], LoanSize[MaxLnTypes], TransBal, MerDis, Mat[MaxLnTypes], Mat1[MaxLnTypes], MatShorter[MaxLnTypes], EfRate[MaxLnTypes], CompRate[MaxLnTypes], ExpRate[MaxLnTypes], Var[MaxLnTypes], Wanted[MaxLnTypes], NormProp[MaxLnTypes], Subst[MaxLnTypes], SBTorCPL;
  struct
  {
    float HProp, MProp, HExp, MExp;
  } Qual[MaxLnTypes];
  float MkShare1[MaxLnTypes], MkShare2[MaxLnTypes], MkShare3[MaxLnTypes], MkShare4[MaxLnTypes][MaxPrem + 1], MkShare5[MaxLnTypes], MkShare6[MaxLnTypes], MkShare7[MaxLnTypes][MaxCQ], MkShare8[MaxLnTypes][MaxCQ], MkShare9[MaxLnTypes][MaxCQ], MkShare10[MaxLnTypes][MaxCQ], MkShare11[MaxLnTypes][MaxCQ], 
	  RateLim[MaxLnTypes][2], LLExp, ELOS[MaxLnTypes], ECAS[MaxLnTypes], PP[MaxLnTypes], MinKeepBal, MinCreateAmnt, MinNewTyp[MaxL], MBStartup, LnSaleMatAdj[MaxL], LnSaleCQAdj[MaxL], LnSaleMktAdj[MaxL],
#define MaxTypsSold 6
  Srv, SrvFix, SrvVar[MaxTypsSold];
} ; // SF.DAT [ 4-9.2 Used 10,11 Reserved ]
#pragma endregion

#pragma region ResKonstT
/// Premises by adjusted, current, and Old
struct ResPremisesT
{
  float Adj, Cur, Old;
};

/// Data strcture for the resouces constant. 
struct ResKonstT
{
  float UpAdj, DwnAdj, Train, Admin, GenOps, TrainW, AdminW, GenOpsW, MediaEff[30], MediaKick[31];
  ResPremisesT Premises[MaxPrem + 1];
  float ReqFloor;
}; // SF.DAT [12]
#pragma endregion

#pragma region CostKonstT
// CostKonstT
// 5 salary constants for deposits.
const int MaxDepSal = 5;
/// Data structure for fixed cost in the accounting. 
struct SalT
{
  float Admin[5], Ops[10], Loans[10], Cred[10], Dep[MaxDepSal];
}; // 40

/// Data structure for the salary cost.  
struct SalCostT
{
  float a, b, c;
};

enum
{
  RV_DemBusM1, RV_BusSaveM1, RV_TimeBusM1, RV_DemBusM5, RV_BusSaveM5, RV_TimeBusM5, RV_DemPub, RV_DemInd, RV_IndNOW, RV_IndMon, RV_IndSave, RV_TimeInd, RV_PrivBkng, Sz_RVs
};

struct CostKonstT
{
  float OccExp[5], PremRV[5], BenFac, OthAss[2], OthLia[2], LnsPrm, DepPrm, ReqRsv[11];
  SalT FixedCosts;
  SalCostT LnOfcr[MaxL][2], Cred[MaxL][3], Dep[Sz_RVs][3], Ops[4];
  float PMF, Admin[10];
}; // SF.DAT [ 26, 27:180, (28, 29 are Reserved) ]
#pragma endregion

#pragma region SecKT
/// These are the 16 loan types, enumerated:
//enum
//{
//  NC_CL, NC_Term, MM_CL, MM_Term, SB_CL, SB_Term, Trade,
//  //         0,       1,     2,       3,     4,       5,     6,
//  Construction, Commercial_RE, Multi_family, Fixed_rate, Variable_rate,
//  //           7,             8,            9,         10,            11,
//  Home_equity, Personal, Credit_card, Installment, MaxL
//};
//         12,       13,          14,          15,   16


/// 12 security types are allowed
const int MaxSecTypes = 12;

enum 
{MaxStPrAtr = 19, MaxB = 6, MaxM = 9, Max4Q = 4, MaxMD = 7, MaxOD = 3,
  // Jeff: How many MaxOut decisions there are on the CreditAdmin form.
	MaxC = 12, Mill = 1000000, MaxRepCQINPUT = 5, MaxRepCQ = 7, MaxRCQ = 8, MaxMats = 3, 
	IniRead = 0, IniUpdate = 1,  MaxMC = 10
};
/// Prepayment constants in the SF.DAT
struct CMOT
{
  float PrePay, a[12];
};

/// SF.DAT constants for securities
struct SecKT
{
  float StripBuyYield, StripYield[2], BondYield1[2], BondYield4[3], BondYield20[3], MuniYield[3], SwapYield[3], MatClass[MaxMC], TEFRA;
  CMOT CMO;
  float BidAdj[MaxSecTypes][2];
}; // SF.DAT [15, (16 is Reserved) ]
#pragma endregion

#pragma region DepKT
/// Deposit classes:
enum
{
  BusNat1E, BusNat2E, BusNat3E, BusNat4E,  // 0 - 3
  BusMid1E, BusMid2E, BusMid3E, BusMid4E,  // 4
  BusLoc1E, BusLoc2E, BusLoc3E, BusLoc4E,  // 8
  BusRe1E, BusRe2E, BusRe3E, BusRe4E,  // 12
  BusPro1E, BusPro2E, BusPro3E, BusPro4E,  // 16
  BusGen1E, BusGen2E, BusGen3E, BusGen4E,  // 20
  Pub1E, Pub2E, Pub3E, Pub4E, Pub5E, Pub6E, Pub7E,  // 24
  Ind1E, Ind2E, Ind3E, Ind4E, Ind5E, Ind6E, Ind7E,  // 31
  Ind8E, Ind9E, Ind10E, Ind11E, Ind12E, Ind13E, Ind14E,  // 38
  Ind15E, Ind16E,  // 45
  Hot1E
}; // 47
///  These are the 10 Deposit products as maintained internally
enum
{
  DemIndE, DemBusE, DemPubE, IndNOWE, IndMonE, IndSaveE, BusSaveE, TimeBusE, TimeIndE, TimeHotE
};

/// A list of deposits by product and market:
enum
{
  DemIndMPE, DemPubMPE, IndNOWMPE, IndMonMPE, IndSaveMPE, BusNatDemMPE, BusMidDemMPE, BusLocDemMPE, BusReDemMPE, BusProDemMPE, BusGenDemMPE, BusNatSaveMPE, BusMidSaveMPE, BusLocSaveMPE, BusReSaveMPE, BusProSaveMPE, BusGenSaveMPE, BusNatTimeMPE, BusMidTimeMPE, BusLocTimeMPE, BusReTimeMPE, BusProTimeMPE, BusGenTimeMPE, TimeIndMPE, TimeHotMPE
};

// These larger deposit catagories effect loan generation:
#define MaxLoy  BusProE - BusNatE + 1
/// The number of business classes, pubic classes, and individual person classes:
const int MaxBusCls = 4, MaxPubCls = 7, MaxIndCls = 16;

// DEPOSITS Constants
/// The buisness deposit class K data structure.

struct BusDepClKT
{
  float  // of DepKT
  Portion, ItemCntMo, DeposCntMo, AvgSize, MatPref, FixPorPref, DemandPorPref, SavingsPorPref, DemandSubsti, SavingsSubsti, Loy, Loy1, Loy2, Open[3];
};
/// Public deposits class K data structure.  
struct PubDepClKT
{
  float  // of DepKT
  Portion, ItemCntMo, DeposCntMo, AvgSize, Float, Loyalty, Open[2];
};
/// Individual deposit class K data structure. 
struct IndDepClKT
{
  float  // of DepKT
  Portion, ItemCntMo, NSFCntMo, AvgSize, MinBalCheck, MinBalMoneyMk, MatPref, FixPorPref, CheckPorPref, SavingsPorPref, MoneyMkPorPref, CheckSubsti, SavingsSubsti, MoneyMkSubsti, Loyalty, Open;
};

/// The main list of SF.DAT constants for Deposits
struct DepKT
{
  BusDepClKT BusNatClK[MaxBusCls], BusMidClK[MaxBusCls], BusLocalClK[MaxBusCls], BusREClK[MaxBusCls], BusProfClK[MaxBusCls], BusGenClK[MaxBusCls];
  PubDepClKT PubDepClK[MaxPubCls];
  IndDepClKT IndDepClK[MaxIndCls];
  float MinKeepBal, MinCreateAmnt, TimeWith[TimeIndE - TimeBusE + 1], NonTimeWith[BusSaveE + 1], Comp[TimeIndMPE + 1], BusAttrib[BusGen4E - BusNat1E + 1][9], PubAttrib[Pub7E - Pub1E + 1][7], IndAttrib[Ind16E - Ind1E + 1][11], RateTrap[TimeIndMPE + 1], Response[TimeIndMPE + 1];
}; // SF.DAT [ 17 - 21:132, (22 & 23 are Reserved) ]
#pragma endregion

#pragma region TreasKT
// Some restrictions on various input fields
enum
{
  MaxCDA = 8, MaxHF = 40, MaxLT = 8, MaxLTA = 5, MaxPT = 10, MaxPTA = 4, MaxCD = 36
};
/// The data structure for stock attributes. 
struct StkAttrT
{
  float Targ, Pos, Neg, Max, Min;
}; // StkPrT
/// The data structure for stock earnings. 
struct StkEarnT
{
  float Abs, Low, On, Hi;
}; // StkPrT
/// SF.DAT's stock price constants
struct StkPrT
{
  float  // TreasKT
  WghtStkPr, WghtDiv, WghtGap, Min, Max, Asst;
  StkEarnT Earn;
  StkAttrT Attr[MaxStPrAtr + 1];
};

const int MaxScrs = 5, MaxRsk = 6, MaxCR = 6;

struct FnsAvlT
{
  // SF.DAT constants for funds.
  float Rsk1[MaxRsk], Rsk2[MaxRsk], CredRating[MaxCR], CDFract[2], CDAvlDst[MaxCDA];
};
/// holds constants for the treasury logic.
struct TreasKT
{
  StkPrT StkPr;
  FnsAvlT FnsAvl;
  float SubLTD_IssuedRate[13];
};
// SF.DAT [ 24:150, (25 is Reserved) ]
#pragma endregion

#pragma region EconT
/// <summary> The EconT structure defines the possible economies </summary>
/// <example> Econ, EconList[10], </example>
struct EconT  ///The EconT structure defines the possible economies.  It reads the data from EC.dat[0-4]  
{
  float
  // SecMk[20]
	/// 1 quarter Treasury yield
  One_Qtr_Treas,  
	/// 3 quarter Treasury yield (second point on curve)
  Three_Qtr_Treas, 
	/// Long-term Treasury yield
  Thirty_Year_Treas, 
	/// commercial paper rate
  CPR,  
	/// federal funds rate
  FFR,  
	/// Federal Reserve discount rate
  FRDR,  
	/// repurchase agreement rate
  RPR,  
	/// London interbank offer rate
  LIBOR,  
	/// national prime rate
  Prime,  
	/// corporate bond rate
  BondAAA,  
	/// corporate bond rate
  BondAA,  
	/// corporate bond rate
  BondA,  
	/// corporate bond rate
  BondBBB,  
	/// corporate bond rate
  BondBB,  
	/// corporate bond rate
  BondB,  
	/// State and local bond rate
  BondSL,  
  SecMkOpen4, SecMkOpen3, SecMkOpen2, SecMkOpen1,
  //
  LoanCompRates[MaxLnTypes],
  // DepCompRates[10],
	/// individual demand deposits
  IDD,  
	/// NOW accounts
  NOW,  
	/// savings accounts
  SAV, 
	/// money market accounts
  MM,  
	/// rate on one quarter time deposits
  Dep1QTD,  
  DepCompRatesOpen5, DepCompRatesOpen4, DepCompRatesOpen3, DepCompRatesOpen2, DepCompRatesOpen1,
  //
  OthrCompRates[5],
  //  Indicators[10], -- For Reporting, NOT Code
  /// Inflation rate (national)
	INFR, 
	/// Change in gross domestic product
  GDP,  
	/// Change in leading economic indicators
  LECI,  
	/// National unemployment rate
  NUP,  
	/// Local unemployment rate
  LUP,  
	/// New housing starts (local)
  NHS,  
	/// New construction permits (local)
  NCP,  
	/// Retail sales (local)
  RETS,  
  IndicatorsOpen2, IndicatorsOpen1,
  //  InterMkIndx[10],
	/// stock market index
  SMI,  
	/// CD market index
  CDMI, 
	/// loan loss index -- Use to determin PastDues (LNQUAL)
  LLI,  
	/// economic conditions index -- PrePay && PrinPay??
  ECI_Nat,  
  ECI_Local, LMQI, 
  // 4/11/2009 George request a new constant LNP in EconT, which reduces the non used constant to 3. 
	LNP,
	InterMkIndxOpen3, InterMkIndxOpen2, InterMkIndxOpen1,
  //  MkGrRates[25],
 
	///     1 National corporations
  ///     2  Middle market corporations
  ///     3  Small businesses
  ///     4  Importers/Exporters (trade finance)
  ///     5  Construction
  ///     6  Commercial real estate
  ///     7  Multi-family residential
  ///     8 1-4 Family residential
  ///     9  Consumers
  ///     10  Open
  LoanMarkets[10],  // i.e.:
  ///    11 National accounts
  ///    12 Middle market accounts
  ///    13  Local business accounts
  ///    14  Real estate related
  ///    15 Professional
  ///    16  Other (not loan related)
  ///    17  Individual
  ///    18  Public
  ///    19-20  Open
  DepositMarkets[10],  // i.e.:

  OtherMarkets[5],  // Not Used 21-25 Open
  CompPrices[10],  // Not used   1-10  Open
  Open[128-20-20-10  // 18 Open
   - 5-10-10-25-10];
}; // EC.DAT [0-4]
#pragma endregion

#pragma region EnvT
/// <summary> EnvT defines the data structure of economic environment. </summary> 
struct EnvT
{
  float  // the community's economy data
  // BaseData [30],
  BaseRate,  // set level of interest rates
  InflRateBase,  // set level of inflation rate
  TaxRate,
  // This gives 3 weights ( Weight[3] = 1 - Weight[1&2])
  // For The 2 Transitional Economies & EconBOQ.
  Weight[2], SeasonalWt[5][4],
  //      1--Business seasonal
  //      2--Real estate seasonal
  //      3--Consumer seasonal
  //      4--Public deposit seasonal
  //      5--Interest rate seasonal
  Open[5], MkSize[25],  // 5:30, match to MkGrRates
  Volume[25],  // match to MkGrRates
  SeasonalSel[25],  // match to MkGrRates
  Open1[23], Seq[3][40],  // econ sequences
  Open2[8];
}; // EC.DAT [5]
#pragma endregion

#pragma region PricesT
enum
{
  BusDD_N, BusDD_M, BusDD_L, BusDD_O, InDD, PubDDR, NowR, BusSavR, InSav, MMR, TimeR, B401_C
};
// See CompPrices[4][50]
const int MaxTE_Avl = 9;
// Open sectors in the EC.DAT file, [ 11 - 17 ] , each sector has 256 floats.
/// <summary> This data structure defines the minimum and maximum price range.  It is used in PriceT. </summary> 
struct PriceRangeT
{
  float Min, Max;
};
/// Deposit Type Information.  
struct DepTypeInfoT
{
  float MnFee, ItemFee, OthFee, MinBal, Attrib;
};
/// This data structure defines the maturity and rate for a security. 
struct MatRateT
{
  float Mat, Rate;
};

/// These are the prices in the community for various deposits:
struct PricesT
{
  struct
  {
    float Fee, QRAP, CreditQ;
  } LnFixPrices[20];
  DepTypeInfoT DepTypeInfo[10];
  float MaxHotMat, aBanksHotAvailByMat[10];
  struct
  {
    float Fee, Mat, Other;
  } OthFixPrices[5];
  PriceRangeT Legal[35];
  float SecPr[30], TE_AvlAmnt;
  MatRateT TE_Avl[MaxTE_Avl];
  float SPA[MaxB][MaxStPrAtr + 1];
	float NewDL[12][MaxB + 1][B401_C + MaxL];
  long Qtr;
}; // EC.DAT [18-?]
#pragma endregion

#pragma region LowerEconT

/// Limits the number of Bank Qualified Tax Exempt Securities to be made available.
const int MaxTE_MatAvail = 3;

/// The lower 128 floats in a segment of 256 floats,  in the community's economy data
struct LowerEconT
{
  float TaxRate, TreasYld[8], PrGrowth[25],
  // 1Qtr-&-4Qtrs BY 4 Markets
  Fcst[2][4], PLNQUAL, SalIndex, SalIndexChg, TE_MatAvail[MaxTE_MatAvail];
}; // EC.DAT [7.5]
#pragma endregion

#pragma region ECNT
/// <summary> This data structure combines two other data structures: EconT and LowerEconT. </summary> 
struct ECNT
{
  EconT Econ;
  LowerEconT LowerEcon;
}; // EconBOQ, EconEOQ [6,7]
#pragma endregion

#pragma region EconQtrT
/// One Quater of Economic History
struct EconQtrT
{
  float
  // See EconBOQ.Econ.SecMk[20]
	/// 1 quarter Treasury yield
  One_Qtr_Treas,  
	/// 3 quarter Treasury yield (second point on curve)
  Three_Qtr_Treas,  
	/// Long-term Treasury yield
  Thirty_Year_Treas,
  /// Commercial paper rate
  CPR,  
	/// Federal funds rate
  FFR,  
	/// Federal Reserve discount rate
  FRDR, 
	/// Repurchase agreement rate
  RPR,  
	/// London interbank offer rate
  LIBOR, 
	/// National prime rate
  Prime,  
	/// Corporate AAA bond rate
  BondAAA, 
	/// corporate AA bond rate
  BondAA, 
	/// Corporate A bond rate
  BondA,  
	/// Corporate BBB bond rate
  BondBBB, 
	/// Corporate BB bond rate
  BondBB, 
	/// Corporate B bond rate
  BondB,  
	/// State and local bond rate
  BondSL,  
	/// 4 Open in the securities markets data
  SecMkOpen4, SecMkOpen3, SecMkOpen2, SecMkOpen1,
  /// Loan Comparison Rates
  LoanCompRates[MaxLnTypes],
  // See DepCompRates[10],
	/// individual demand deposits
  IDD,  
	/// NOW accounts
  NOW,  
	/// savings accounts
  SAV, 
	/// money market accounts
  MM,  
	/// rate on one quarter time deposits
  Dep1QTD,                   // 45

  // See EconBOQ.Econ.Indicators[10], -- For Reporting, NOT Code
  /// Inflation rate (national)
	INFR, 
	/// Change in gross domestic product
  GDP,  

  // See EconBOQ.Econ.InterMkIndx[10],
  /// stock market index
	SMI,  
	/// CD market index
  CDMI, 
	/// loan loss index -- Use to determin PastDues (LNQUAL)
  LLI,  
	/// economic conditions index -- PrePay && PrinPay??
  ECI_Nat,  
	/// economic conditions index
  ECI_Local,  

  // See EconBOQ.LowerEcon
  TaxRate, Fcst[4][2], OpenEcon[9];
}; // EC.DAT [8,9,10] -- 8 (YTD) // Bytes: 280 0x118, 70 floats
#pragma endregion

#pragma region AcT
/// Data structure for securities.
struct SecuritiesT
{
  float Treasuries, Munis, Mort, Other;
};
/// Data structure for loans.
struct LnAcT
{
  float Loans[MaxL], Open_1, Open_2, Other, Sum;
};

/// Asset in the Book of Accounts.
struct AssetsT
{
  float Total, Cash, Open_1, ffSold;
  SecuritiesT Securities;
  LnAcT Loans;
  float LLR;
  struct
  {
    float GrossPremises, AccumDepre;
  } NetPremises;
  float OtherRE, Other, Open_2, Balancer;
};

// ---------- BOA --- Start of BK01.DAT
/// Data structure for the demand deposit service.  
struct DemandT
{
  float Bus, Pub, Indiv, Open, Open2;
};
/// Data structure for the deposit in the book of account. 
struct BOA_DepT
{
  float Now, Sav, MonMkt, Jumbo, PubTime, SavCerts, Open;
};
/// Data structure for the deposit liablity
struct DepLiaT
{
  DemandT Demand;
  BOA_DepT BOA_Dep;
  float Open_1, Open_2, Sum;
};

/// Data structure for funds 
struct FundsT
{
  float FRB, Repo, FF, ShortTermHF;
};
/// Liabilities in the Book of Accounts.
// on 10/11/2007, George suggested converting Open to DividendPayable 
// so the code will use DividendPayable in Books() to keep TotDiv
struct LiaT
{
  DepLiaT Deposits;
  FundsT Funds;
  //float TaxBill, Other, Open, Balancer, LTD_HF, SubLTD;
  float TaxBill, Other, DividendPayable, Balancer, LTD_HF, SubLTD;

  struct
  {
    float Common, Surplus;
  } Stock;
  float RetainedEarnings, _Equity;
};
/// Data structure for the balance in the Book of Accounts.
struct BalT
{
  AssetsT Assets;
  LiaT Liabilities;
};
/// Data structure for various fees.  
struct FeeT
{
  float DepServiceChrgs, LoanFees, CommLCFees, StandbyLCFees, Other;
};
/// Data structure for various expenses. 
struct ExpensesT
{
  float SalariesBenes, Premises, Media, OtherOpEx;
};
/// Data structure for net incomes. 
struct NetsT
{
  float UnRlzd, AssetSales, IncomeTaxes, Income, IncomePerShare;
};
/// Data structure for interest incomes.
struct IntInT
{
  float FF;
  SecuritiesT Securities;
  LnAcT Loans;
  float Other;
};
/// Data structure for interest expenses.
struct ExpT
{
  BOA_DepT BOA_Dep;
  FundsT Funds;
  float LTD_HF, SubLTD;
};
/// Data strcuture for interest. 
struct InterestT
{
  IntInT Income;
  ExpT Expenses;
  float NetSwaps, CoreDep, Open_1, Net;
};
/// Income and Outgo in the Book of Accounts.
struct IncomeOutGoT
{
  InterestT Interest;
  float LoanLossProvision;
  FeeT Fees;
  float Open_1;
  ExpensesT Expenses;
  NetsT Nets;
  float OpEarnings, TEFRA;
};
typedef IncomeOutGoT YTDT[8]; // BK01.DAT [8,9:224]  480; 60 * 8

/// Data structure for the demand deposit interest. 
struct DemIntT
{
  float Bus, Pub, Ind;
};

/// The Book of Accounts for a Quarter.
// This structure is documented in George Excel format. 
struct AcT
{
  BalT Bal, AveBal;
  IncomeOutGoT IncomeOutGo;
  DemIntT DemInt;
  float SwapRisk, Open8[7], QMunis, FundsCost, EPS_Forcast, NetChargeOffs, PastDue, CreditRating, Unpledged, UnRlzd, NonTreasSec1yr, ProfitsMeas, NetEPS, LoanRiskMeas, RiskAdjAss, RateSens, OneYrGap, LoanMkShare, DepMkShare, ShortTermRisk, LongTermRisk, Open4, Open5, CoreDep, NOEPS, EarnRate, SmthDiv, NonPerformLns, TaxedMuniIntTaxEqiv, TaxedMuniIntTaxEqivYTD, No_Sec_Int_YTD, No_Sec_Int, LnChrgOffsYTD, MortBanking, Liquid_Bonds, Open3[11], Div, DivYTD, _Book, Profit, Risk, Leverage, Liquidity, MarketShare,
 EPS_ForcastAccuracy, DividendPol, Other, StkPrice;
};

typedef AcT BOAT[8]; // BK01.DAT [0-7]
#pragma endregion

#pragma region SecT
/// These are the types of securities:
enum
{
  BillsE, StripsE, BondsE, MunisE, No_Sec_E, MortE, SwapFP, SwapVP
};
// SECURITIES
/// Data structure for security values. 
struct ValueT
{
  float Par, Book, Mkt_Bid;
}; // Three ways to value securities.
/// Data structure for securities portfolio.
struct SecT
{
  float Type, Name;
  ValueT Value;
  float UnRealGainOrLoss, Mat, CouponYld, EconYield, TaxEquivYield, Dur, CouponIntr, PurDate /* SimQtr */, PrePay, Open[2];
}; // BK01.DAT [17-20] Max = 64, 4 Recs
#pragma endregion

#pragma region LoanGloT
const float STOCKREPURPREM = 1.1, STOCKREPURLIM = .1, LTLIMIT = 1.2, CILIMIT = .2;
const int CLTyps = 7, MAXBUF = 0x400, SellSubLTD = 103, SellStock = 106, PAR = 10;

// LOANS
// BK01.DAT [24:0-215]
/// Data structure for loan information. 
struct LoanGloT
{
  float LnCnt[MaxLnTypes], NAT[MaxLnTypes], CORatio[MaxLnTypes], UCB[MaxLnTypes], PlayersMS[MaxLnTypes], InternalMkShare[MaxLnTypes][MaxCQ], MatClass[MaxMC], New[MaxLnTypes];
  //  Commitments :  0, 2, 4, 6, 7, 12, 14
  float B_Com[CLTyps], E_Com[CLTyps];
};
#pragma endregion

#pragma region LoanT
// 50,51,52,53 Copy of EoQ Decs
/// Loan data structure.  See Loan Records in the design document.  
// AdjP is rate ajusted period, Commit is total commitment or quarter 
struct LoanT
{
	///Type(xx)
  float  
  Type, 
	/// current principle outstanding
	Amnt, 
	///Quarters until maturity/term of commitment
	Mat, 
	/// Interest rate - quarterly decimal rate
	Rate, 
	/// Quarterly decimal rate
	Spread, 
	/// Reported credit quality (integer)
	Reported, 
	/// Actual credit quality (decimal number)
	Actual, 
	/// Past due flag 
	PastDue, 
	/// Estimated market value
	MktVal, 
	///Duration
	Dur, 
	/// Rate adjustment period
	AdjP, 
	/// Ajusted maturity
	AdjMat, 
	/// Annual fee
	AnnFee, 
	/// Total commitment
	Commit,
	/// Usage rate
	UsageRate, 
	/// Quarterly payment
	Payment, 
	Id, 
	Pkg;
}; // LN0n.DAT 18 by Open-Ended
#pragma endregion

#pragma region LnSalesT
/// Data structure for the loan sales data. 
struct LnSalesT
{
  float  // Loan sales data
  Amnt, Mat, Rate, Pay, Inc, Exp, Paid, Open[3];
}; // BK01.DAT [16:59] Reserves: 1K; Uses 6 recs of 10 Floats (.2K)
#pragma endregion

#pragma region ResGloT
// RESOURCE MANAGEMENT
/// Data structure for the media in resource management.  
struct MediaT
{
  float Loans[20], Dep[10], Gen;
}; // 40
/// Data structure for the resource.  
struct ResT
{
  SalT Sal;
  MediaT Media;
  float Premises[5];
}; // 75 is last

/// Data structure for resourse global information.
struct ResGloT
{
  ResT Cur, Req, Eff, Adj, Old;
}; // BK01.DAT [28,29]
#pragma endregion

#pragma region CostsGloT
// CostsGloT
/// Data structure for premises. 
struct PremT
{ 
	/// Urban branches
  float City, 
		///Regional branches
		Region, 
		///Suburban branches
		Berb, 
		///Admin/Operations
		AdOps, Open;
};

const int MaxTaxH = 5;
const float MinSecKeep = .1;

struct TaxHistT
{
  float Taxable, Rate;
};
/// Globals for cost accounting.
struct CostsGloT
{
  PremT Prem, Depr;
  float AdOpsPremInv, Open[10];
  TaxHistT TaxHist[MaxTaxH];
  float EVTA;
}; // BK01.DAT [30]
#pragma endregion

#pragma region DepT
// DEPOSIT Variables -- By Bank
/// Individual Deposit Class Data Structure
struct IndDepClT
{
  float DemBal, SavBal, NowBal, MonBal, DemCnt, SavCnt, NowCnt, MonCnt;
};
/// Public Deposit Class Data Structure
struct PubDepClT
{
  float Bal, Cnt;
}; // of DepT
/// Business Deposit Class Data Structure
struct BusDepClT
{
  float DemBal, SavBal, DemCnt, SavCnt, Loy;
};
/// All Deposit Data Structure. 
struct DepT
{
  // The main deposit data.
  BusDepClT BusNatCl[MaxBusCls], BusMidCl[MaxBusCls], BusLocalCl[MaxBusCls], BusRECl[MaxBusCls], BusProfCl[MaxBusCls], BusGenCl[MaxBusCls];
  PubDepClT PubDepCl[MaxPubCls];
  IndDepClT IndDepCl[MaxIndCls];
  float BnkShr[Ind16E + 1];
}; // BK01.DAT [38-39:49]
#pragma endregion

#pragma region TimeDepT
/// These are the larger deposit catagories as found on B401:
enum
{
  BusNatE, BusMidE, BusLocE, BusReE, BusProE, BusGenE, PubE, IndE, HotE
};

const int MaxTimeDep = 182, MaxTimeCls = 16;
/// Time Deposit data structure
struct TimeDepT
{
  float  // The time deposits.
  Id, Market, Bal, RateOrSpread, Mat, IsFixed, Cnt, MktVal, MktRate, Dur;
  //uchar Por[MaxTimeCls];
  uchar Por[MaxTimeCls];
}; // BK01.DAT [40-49:243] Reserved: Max=182(x14), 10K
#pragma endregion

#pragma region FundsGloT
struct SubLTD_IssuedT
{
  float Amnt, Mat, callable, principal, Rate, Duration;
};
/// The data structure for the Certificates of Deposits and Public Time Deposits.
struct TrsTmT
{
  float Amnt, Mat, Rate, Duration;
};
/// Housing Finance data structure in Treasure Management
struct HFsT
{
  float Amnt, Rate, Fixed, Mat, Duration;
};
// George: Certain transactions during a quarter or the previous quarter will provide funds 
// (amounts paid in) or require funds (Amount paid out)  on the first day of the 
// quarter referred to as Day 1 funds.  Below from BMSim.cpp are the two data structures for accounting for those funds.
/// Data structure for for transactions providing funds on the first day of the quarter 
struct Day1InT
{
  float Loans, Sec, SecMat, Stock, Funds, HFs, SubLTD, CDs, PTs, Taxes;
};
/// Data structure for transactions requiring funds on the first day of the quarter.  
// On 10/11/2007, change item 3 from the final list of panther
// After Taxes we would add Dividend_Paid 
struct Day1OutT
{
  float Loans, Sec, Stock, Funds, HFs, HFsMat, SubLTD_IssuedMat, CDsMat, PTsMat, Taxes, Dividend_Paid;
};
/// Data structure for rate and amount of CD, Repo, FRB, FF.  
struct RateAmntT
{
  float Rate, Amnt;
};
/// The main globals for funds
struct FundsGloT
{
  RateAmntT Repo, FRB, FFs;
  TrsTmT CDs[MaxCD], PTs[MaxPT];
  RateAmntT CDsAvail[MaxCDA];
  SubLTD_IssuedT SubLTD_IssuedAvail[MaxLTA];
  float Open[2], CeditRating;
  HFsT HFs[MaxHF];
  SubLTD_IssuedT SubLTD[MaxLT];
  float Open_A[2], HFMem, HFAmnt, Open_B;
  Day1InT Day1In;
  Day1OutT Day1Out;
  float NetDay1, FFSold, Repos, FRBs, FF, FRBClosed, StockAmntRP, SharesRP, ZMatCDs, IssuePlanAmnt, IssuePricePerShare, IssuePrice, NewUniqueName;
  ulong SimQtr, Dirty;
  // This is the very unique id that ties the BK01.IN and the BK01.DAT files together.
  __int64 _ID;
  // This is the banner for printing the students decisions at the beginning of the quarter.
  char Banner[88];
}; // BK01.DAT [21,22,23:40]
#pragma endregion

/// Data structure for interest paid on demand deposits.
struct PayIntOnDemDepT
{
  long Bus, Pub, Ind;
};

///  This keeps a summery of a securities portfolio
struct SecSumT
{
  float Cnt, Cash, Interest, RealizedGains, RealizedLosses, Repayment;
  ValueT BOQ, EOQ, Mat, Pur, Sold, MatDistrib[MaxMC + 1];
};

/// Data structure for the security global information.
struct SecGloT
{
  float Cnt;
  ValueT EOQ;
  SecSumT Sec[MaxSecTypes];
};

///Data structure for loan packaging data.
struct LnPkgT
{
  float  // Loan packaging data
  Amnt, MktVal, BookYld, MktYld, AvgRCQ, AvgMat, AvgRAP, Pay;
}; // BK01.DAT [15:159] Reserves: 1K; Uses 20 recs of 8 Floats (.6K)

/// Deposits in the community.
struct EconInfoT
{
  long PayInt, Deleted, MktProd, Mkt, Cls, Prod;
  float AvgSize, Rate, MinBal, Pref, Sub, FinalAttrib10, Comp, RealRate, Tot, Checks, MoCnt1, MoCnt2;
};

/// A bank's deposits
struct BankInfoT
{
  TimeDecT *TimeDec;
  long Deleted;
  float BegBal, BegCnt, Attrib, Shr, FinalAttrib, BankRate, AdjRate,  *Bal,  *Cnt, Loy, RealRate;
};