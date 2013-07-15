#ifndef DECISION_FORM_DATAT_H
#define DECISION_FORM_DATAT_H

const int MAX_PURCHASES = 20; // Maximum securities purchases per quarter.
const int MAX_SALES = 20;

// BK01.DAT [33-36,37(Reserved)]
// DECISIONS -- By Bank
/// Structure for the entire Purchase Security Decision form
typedef struct
{
  /// Security type
  float type, 
	/// Amount	
		amount, 
	/// Maturity in quarters
		maturity;
} SecPurDecT[MAX_PURCHASES]; // BK01.DAT [ 33+ (secpur)]

/// Structure for the entire Sale Security  Decision form
typedef struct
{
  /// Security No. 
  float Name, 
		/// Sale amount
		Amnt;
} SecSaleDecT[MAX_SALES]; // BK01.DAT [ 33+ (secsal)]

/// <summary> data structure for each Credit Line decision in Loans Decision form </summary>
struct ET
{
  /// Interest base
  float BaseRate, 
		/// Rate Max
		MaxRate, 
		/// Orig Fee
		OrigFee, 
		/// Dus Dev
		BusDev, 
		/// Max Mat
		MaxMat;
};

/// <summary> data structure for each Term Loans decision in Loans Decision form </summary>
struct ETA
{
  /// Interest base
  float BaseRate, 
		/// Rate Max
		MaxRate, 
		/// Orig Fee
		OrigFee, 
		/// Dus Dev
		BusDev, 
		/// Max Mat
		MaxMat, 
		///Rate Adj Period
		AdjP;
};

/// <summary> Data structure for the entire Loan decision form </summary>
/// <remarks> which fields match the order on input screen, from left to right and from top to bottom. </remarks>
struct LoanDecT
{
  ET Nat;
	/// Annual Fee
  float NatFee;
  ETA NatTerm;
  ET MidM;
  float MidMFee;
  ETA MidMTerm;
  ET SmallB;
  float SmallBFee;
  ETA SmallBTerm;
  ET ImpExp;
  float ImpExpFee;
  ET Cons;
  float ConsFee;
  ETA CommRe, MulFam;
  ET SingFam;
  ETA SingFamVar;
  ET HomeEqu;
  float HomeEquFee;
  ETA Per;
  ET CCard;
  float CCardFee;
  ET Instal;
  // These fields are not used anywhere:
  struct
  {
    float BaseFee, MaxFee, BusDev, MaxMat;
  } ImpExpLet, StandByLet;
}; // BK01.DAT[ 33+ (loans)]

const int MaxLnPkgs = 20, MaxLnSales = 6, MaxSecs = 64; 

/// <summary> Data structure for the entire loan sale decision form </summary>
struct SellLnsDecT
{
	/// Initiate or Close the mortgage banking division
  float MortBanking;
	/// PackageNumber
  float Pkg[MaxLnPkgs];
}; // BK01.DAT[ 33+ (lnprsl)]

/// <summary> Data structure for each credit administration decision with two maximum outstanding. </summary>
struct ThreeTargT
{
  /// 5 portfolio targets
  float Targ[5], 
  /// Request Loan Review field
		DoRev, 
	/// 2 maximum outsanding
		MaxOut[2];
};

/// <summary> data structure for each credit administration decision </summary>
struct OneTargT
{
	/// maximum outsanding
  float MaxOut, 
	/// 5 portfolio targets	
		Targ[5],
	/// Request Loan Review field	
		DoRev;
};

/// <summary> Data structure for the entire Credit administration decisions, in the order shown on the screen. </summary>
struct CredDecT
{
  ThreeTargT Nat, MidM, SmallB;
  OneTargT ImpExp, Cons, CommRe, MulFam;
  ThreeTargT SingFam;
  OneTargT HomeEqu, Personal, CredC, Instal;
  float LossProv, GenCredPol;
}; // BK01.DAT[ 33+ (credadm)]

/// <summary> Data structure for Retail Certificates in Deposit Decisions form </summary>
struct TimeDecT
{
  /// Maturity
  float Mat, 
		/// Rate
		Rate, 
		/// F or V
		Fixed;
};

/// <summary> Data structure for Interest Bearing Deposits in Deposit Decisions form </summary>
struct InterestBearingT
{
///NOW Accounts Interest Rate
  float NowRate, 
		/// Money Market Accounts Interest Rate
		MmRate, 
		/// NOW Accounts Item Fee
		NowItemFee, 
		/// Money Market Accounts Base Interest
		MmBase, 
		/// NOW Accounts Monthly Fee
		NowMonthlyFee, 
		/// Money Market Monthly Fee
		MmMonthlyFee, 
		/// NOW Accounts NSF Fee
		NowNsfFee, 
		/// Money Market Minimum Balance for higher rate and no fee
		MmMinBal, 
		/// NOW Accounts Minumum Balance to waive service charges
		NowMinBal;
	/// Savings Accounts
  struct
  {
    float Rate, Fee;
  } Indiv, Busi;
};

/// <summary> Data structure for Business Development (in Thousand) in Deposit Decision form </summary>
struct BusiDevT
{
/// Business Demand
  float BusDD, 
		///Business Savings
		BusSav, 
		/// Public Demand
		PubDD, 
		/// Money Market
		MonMkt, 
		/// Individual Demand
		PerDD, 
		/// Individual Savings
		PerSav, 
		///Now Accounts
		Now, 
		///Retail Certificate
		Time, 
		/// Private Banking
		PrvBkng;
};

enum MaxTimeTreasDec
{
  MaxTimeDec = 8, MaxTreasDec = 16
};

/// <summary> Data structure for the entire Deposit decision form </summary> 
struct DeposDecT
{
  ///Demand Deposits
  struct
  {
    struct
    {
		  ///Business, Public, Individual
      float Busi, Pub,	Indiv;
    } MonthlyFee, Fee1, Fee2, Credit;
  } Demand;

	/// Interest Bearing Deposits and Saving Accounts
  InterestBearingT InterestBearing;
	/// Retail Certificates 
  struct
  {
    TimeDecT TimeDec[MaxTimeDec];
    float MaxBal;
  } Time;
  BusiDevT BusiDev;
}; // BK01.DAT[ 33+ (depos)]

/// <summary> Data structure for Purchased Funds</summary>
struct FndDecT
{
/// Funds Type
  float Type, 
		/// Amount
		Amnt, 
		/// Maturity
		Mat;
};

/// <summary> Data structure for the entire Treasury Decision form </summary>
struct TreasDecT
{
 /// Purchased Funds  (in millions) 
  FndDecT Funds[MaxTreasDec];
	/// Divident per share
  float Div, 
		/// Issue Plan
		Plan, 
		/// Forecase earnings per share
		Forecast, 
		/// Issue Code (From B550)
		C101, 
		/// Issue Amount (in millions)
		Cam, 
		/// Shares to Repurchase (max 99,999)
		RePur;
}; // BK01.DAT[ 33+ (treas)]

// genadm   Salary decisions in the order shown on the screen.
/// <summary> Data structure for General Administration Salary Decisions </summary>
struct SalDecT
{
	/// National Corporation Lending Activities
  float NatLn, 
		/// National Corporation Credit Administration
		NatCred, 
		/// Commercial Operations and Processings
		CommOp, 
		/// Middle Market
		MidMLn, MidMCred, 
		/// Local Business 
		SmallBLn, SmallBCred, 
		/// Trade Finance
		ImpExpLn, ImpExpCred, 
		/// Commercial Real Estate
		CommReLn, CommReCred, ReOp,
		/// Residential/Home Equity
		 ResReLn, ResReCred, 
		/// Consumer 
		ConsumLn, ConsumCred, ConsumOp, 
		///Business Account Deposition
		BusDep, 
		/// Deposition Operation
		DepOp, 
		/// Public Accounts
		PubDep, 
		/// Individual Accounts
		IndDep, 
		/// Private Banking
		PrivDep, 
		/// Administrative
		Admin, 
		/// Central Operations
		CenOp, 
		/// Personnel/Training
		Train, 
		/// General Salary Adjustment
		Adj;
};

/// <summary> Data structure for Premises
struct DecPremT
{
/// Urban branches
  float City, 
		/// Regional branches
		Region, 
		///Suburban branches
		Berb, 
		///Administrative/Operations
		AdOps;
};

/// <summary> Data structure for the entire General Administration Decisions form </summary> 
struct AdminDecT
{
  /// Salary Decisions
  SalDecT Sal;
  /// Premises
  DecPremT Premises;
	/// Promotion: General Advertising and Promotion Budget
  float Media;
}; // BK01.DAT[ 33+ (genadm)]

/// <summary> All the decision forms. </summary>
/// <remarks> secpur(security purchase) = 0, secsal(security sale)=1, loans=2, lnprsl()=3,credadm(credit administration)=4
/// depos(deposit)=5, treas=6 , genadm(general administration)=7, MaxForms=8 </remarks>
//enum DecisionForm
//{
//	 = 0; security purchase decision form
//  secpur, 
//	 = 1; security sale decision form
//	secsal, 
//	 = 2; loans decision form
//	loans, 
//	 = 3; Mortgage banking and loan sales decision form
//	lnprsl, 
//	 = 4; Credit administration decision form
//	credadm, 
//	 = 5; Deposits decision form
//	depos, 
//	 = 6; Treasury management decision form
//	treas, 
//	 = 7; General administration decision form
//	genadm, 
//	 = 8; Max number of decision forms
//	MaxForms
//}; 

#endif