#include "BMSim.h"
#include "Reporting.h" //RptEOQ
#include <math.h> //fabs

extern LoanRptT LoanRpt[MaxB]; 
extern ECNT EconBOQ; 
extern DepKT DepK;
extern ResGloT ResGlo[MaxB]; 
extern DepOutT DepOut[MaxB]; 
extern TimeDepT TimeDep[MaxB][MaxTimeDep]; 
extern PricesT Prices; 
extern DepT Dep[MaxB];

extern DeposDecT DeposDec[MaxB];

extern BOAT BOA[MaxB]; 
extern int Bank, NumBanks; 
extern int YrsQtr; 
extern long PrivBkng[MaxB]; //RunForward(), Expense()

const int MaxDepMedia = 8;

float NewDep[MaxB];  //used in NewTime and Stock()
long Id[MaxB]; // To generate uniue ID's for securites.

/// The dollar amound of deposits in the market.
float MkAmntByCl[Ind16E + 1]; //also used in RunForward()

EconInfoT EconInfo[TimeIndC16PE + 1];
BankInfoT BankInfo[MaxB][TimeIndC16PE + 1];
DepTypeInfoT DepTypeInfo[MaxB][BusSaveE + 1]; //also in RunForward()
float FinalAttribByCls[Ind16E + 1]; // Determins what the market looks like for each class of depositor

float LnsByBusMk[MaxB][MaxLoy], SumLnsByBusMk[MaxLoy], CusLns[MaxB], SumCusLns;
int ClsI, ProdI, MktI, MktProdI;
float MaxHot[MaxB]; /// The hot money that is available
float EffMedia[MaxB][MaxDepMedia], EffSal[MaxB][MaxDepSal], EffOps[MaxB], EffPrem[MaxB][MaxPrem]; // Various factors that determine how well a bank is performing.


/// <summary> This function distributes the growth to the various deposit classes for each market. </summary> 
/// <remarks> The results are saved in MkAmntByCl[] based on BusXXXClk (which has six different business market), PubDepClK and IndDepClK. 
/// The calculation is ClsK->Portion * MkAmnt[i]</remarks> 
void ClGrowth()
{
  float MkAmnt[IndE + 1]; //New deposit amount in the market
  memmove(MkAmnt, EconBOQ.LowerEcon.PrGrowth + 10, sizeof(MkAmnt));
  {
    int i =  - 1;
    BusDepClKT *ClsK = DepK.BusNatClK - 1;
		//Distribute the portion of total amount in one business market to different classes.  
    while(++i <= BusGenE) //BusGenE=5
    {// MkAmntByCl[0-23]
      {
        int j =  - 1;
				int k = i * MaxBusCls; //MaxBusCls = 4
        float Amnt = MkAmnt[i];
        while(++j < MaxBusCls)
        {
          ++ClsK;
          MkAmntByCl[k + j] = ClsK->Portion *Amnt;
        }
      }
    }
  } 
  { // Distribute the portion of total amount in a public market to different classes. 
		// MkAmntByCl[24-30]
    PubDepClKT *ClsK = DepK.PubDepClK - 1;
    float Amnt = MkAmnt[IndE];
    int i = BusGen4E;
    while(++i <= Pub7E)
    {
      ++ClsK;
      MkAmntByCl[i] = ClsK->Portion *Amnt;
    }
  } 
  { //Distribute the portion of total amount in one individual market to different classes. 
		// MkAmntByCl[31-46]
    IndDepClKT *ClsK = DepK.IndDepClK - 1;
    float Amnt = MkAmnt[PubE];
    int i = Ind1E - 1;
    while(++i <= Ind16E)
    {
      ++ClsK;
      MkAmntByCl[i] = ClsK->Portion *Amnt;
    }
  }
}

/// <summary> This function maps decisions from the input screen to a more useable format. </summary> 
/// <param name="T"> is the deposit type. </param>
/// <param name="MnFee"> is the monthly fee. </param>
/// <param name="ItemFee"> is Debit Fee charged to the account. </param>
/// <param name="OthFee"> is Deposit Fee charged to the account. </param>
/// <param name="MinBal"> is the minimum balance. </param>
/// <remarks> This function converts data from DeposDecT to DepTypeInfoT. </remarks>
void DecToInfo(int T, float MnFee, float ItemFee, float OthFee, float MinBal)
{
  DepTypeInfoT *p = DepTypeInfo[Bank] + T;
  p->MnFee = MnFee;
  p->ItemFee = ItemFee;
  p->OthFee = OthFee;
  p->MinBal = MinBal;
}

/// <summary> This function loads the depsoit information in the community into EconInfo. </summary>
/// <param name="ClsProdI"> is the deposite type by class and product. </param>
/// <param name="ProdI"> is the Deposit product as maintained internally. </param>
/// <param name="MktProdI"> is the deposit by product and market. </param>
/// <param name="AvgSize"> is the average size of the deposit. </param>
/// <param name="Pref"> ??? </param>
/// <remarks> It is called by SetRel(). </remarks>
void SetRel2(int ClsProdI, int ProdI, int MktProdI, float AvgSize, float Pref)
{
  EconInfoT *e = EconInfo + ClsProdI;
  e->Cls = ClsI;
  e->Mkt = MktI;
  e->Prod = ProdI;
  e->MktProd = MktProdI;
  if(Pref < .01)
    Pref = 0;
  e->Pref = Pref;
  {
    float as = AvgSize * Pref;
    e->AvgSize = as < 30 ? 0 : as;
  }
}

/// <summary> This function creates a common format for the community's deposits. </summary> 
/// <remarks> This function writes EconInfo[158] </remarks>
void SetRel()
{
  memset(EconInfo, 0, sizeof(EconInfo));
  ClsI = Hot1E; //47 Deposit Class
  MktI = HotE;  //8 Large Deposit Category
  SetRel2(TimeHotCPE, TimeHotE, TimeHotMPE, 1, 1);
  {
    BusDepClKT *ClsK = DepK.BusNatClK - 1;
    long DemI = BusNatDemC1PE - 1;
		long SavI = BusNatSaveC1PE - 1;
		long TimI = BusNatTimeC1PE - 1;
    ClsI = BusNat1E - 1;
    while(++ClsI <= BusGen4E)  //if the deposit class is business class
    {
      ++DemI;
      ++SavI;
      ++TimI;
      ++ClsK;
      {
        long MktO = (ClsI - BusNat1E) / MaxBusCls;
        float a = ClsK->AvgSize;
        MktI = BusNatE + MktO;
        SetRel2(DemI, DemBusE, BusNatDemMPE + MktO, a, ClsK->DemandPorPref);
        SetRel2(SavI, BusSaveE, BusNatSaveMPE + MktO, a, ClsK->SavingsPorPref);
        SetRel2(TimI, TimeBusE, BusNatTimeMPE + MktO, a, 1-ClsK->DemandPorPref - ClsK->SavingsPorPref);
      }
    }
  }
  {
    PubDepClKT *ClsK = DepK.PubDepClK - 1;
    long DemI = DemPubC1PE - 1;
    ClsI = Pub1E - 1;
    MktI = DemPubE;
    while(++ClsI <= Pub7E)
    {
      ++DemI;
      ++ClsK;
      SetRel2(DemI, DemPubE, DemPubMPE, ClsK->AvgSize, 1);
    }
  }
  {
    IndDepClKT *ClsK = DepK.IndDepClK - 1;
    long DemI = DemIndC1PE - 1;
		long SavI = IndSaveC1PE - 1;
		long NowI = IndNOWC1PE - 1;
		long MonI = IndMonC1PE - 1;
		long TimI = TimeIndC1PE - 1;
    ClsI = Ind1E - 1;
    MktI = IndE;
    while(++ClsI <= Ind16E)
    {
      ++DemI;
      ++SavI;
      ++NowI;
      ++MonI;
      ++TimI;
      ++ClsK;
      {
        float a = ClsK->AvgSize;
        SetRel2(DemI, DemIndE, DemIndMPE, a, ClsK->CheckPorPref);
        SetRel2(SavI, IndSaveE, IndSaveMPE, a, ClsK->SavingsPorPref);
        SetRel2(NowI, IndNOWE, IndNOWMPE, a, ClsK->CheckPorPref);
        SetRel2(MonI, IndMonE, IndMonMPE, a, ClsK->MoneyMkPorPref);
        SetRel2(TimI, TimeIndE, TimeIndMPE, a, 1-ClsK->CheckPorPref - ClsK->SavingsPorPref - ClsK->MoneyMkPorPref);
      }
    }
  }
}

/// <summary> This function gathers various deposit data about each bank and then puts it into DepTypeInfo[Bank]. </summary> 
/// <remarks> It is called by Deposits(). This function groups LoanRpt.New to CusLns[Bank], SumCusLns, LnsByBusMk[Bank] and SumLnsByBusMk.
/// It also calls DecToInfo() to convert data from DeposDecT to DepTypeInfoT and set MaxHot[Bank] that is hot money for the bank.</remarks>
void DepStartUp()
{
  PrivBkng[Bank] = ResGlo[Bank].Cur.Sal.Dep[3] > 0; //If the buiness development in Private Banking is zero, the PrivBkng=0
  NewDep[Bank] = 0;
  if( ! Bank)
    SumCusLns = 0;
  Zero(DepOut[Bank]);
  CusLns[Bank] = 0;
  {
    for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
    {
      if(intIndexOfCount <= Multi_family)
        continue;
      CusLns[Bank] += LoanRpt[Bank].New[intIndexOfCount];
    }
  }
  SumCusLns += CusLns[Bank]; //The total of CusLns for all banks. 
  {
    int LnToMk[] = //map loan type to each market
    {
      BusNatE, BusNatE, BusMidE, BusMidE, BusLocE, BusLocE, BusLocE, BusReE, BusReE, BusReE,  - 1,  - 1,  - 1, BusProE,  - 1,  - 1
    };
    for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
    {
      float N = LoanRpt[Bank].New[intIndexOfCount];
      int Mkt = LnToMk[intIndexOfCount];
      if(Mkt < 0)
        continue;
      LnsByBusMk[Bank][Mkt] += N;
      SumLnsByBusMk[Mkt] += N;
    }
  }
  memmove(EffMedia[Bank], ResGlo[Bank].Eff.Media.Dep, MaxDepMedia *4);
  memmove(EffSal[Bank], ResGlo[Bank].Eff.Sal.Dep, MaxDepSal *4);
  EffOps[Bank] = ResGlo[Bank].Eff.Sal.Ops[3];
  memmove(EffPrem[Bank], ResGlo[Bank].Eff.Premises, MaxPrem *4);
  Zero(DepTypeInfo[Bank]);
  {
    DeposDecT *p = DeposDec + Bank;
    DecToInfo(DemIndE, p->Demand.MonthlyFee.Indiv, p->Demand.Fee1.Indiv *.01, p->Demand.Fee2.Indiv, 0);
    DecToInfo(DemBusE, p->Demand.MonthlyFee.Busi, p->Demand.Fee1.Busi *.01, p->Demand.Fee2.Busi, 0);
    DecToInfo(DemPubE, p->Demand.MonthlyFee.Pub, p->Demand.Fee1.Pub *.01, p->Demand.Fee2.Pub, 0);
    DecToInfo(IndNOWE, p->InterestBearing.NowMonthlyFee, p->InterestBearing.NowItemFee / 100, p->InterestBearing.NowNsfFee, p->InterestBearing.NowMinBal);
    DecToInfo(IndMonE, p->InterestBearing.MmMonthlyFee, 0, 0, p->InterestBearing.MmMinBal);
    DecToInfo(IndSaveE, p->InterestBearing.Indiv.Fee, 0, 0, 0);
    DecToInfo(BusSaveE, p->InterestBearing.Busi.Fee, 0, 0, 0);
    MaxHot[Bank] = p->Time.MaxBal;
  }
}

// Returns a class for each time deposit market.
int MarketToTimeMkC1[] =
{
  BusNatTimeC1PE, BusMidTimeC1PE, BusLocTimeC1PE, BusReTimeC1PE, BusProTimeC1PE, BusGenTimeC1PE,  - 1, TimeIndC1PE, TimeHotCPE
};

/// <summary> This function returns expired time deposits back into the market. </summary>
/// <param name="A"> is the balance. </param>
/// <param name="*t"> is the time deposit data structure.  </param>
/// <remarks> It is called by UpdateTime(). </remarks>
void DrainTimeMksCls(float A, TimeDepT *t)
{
  long TimeMkC1 = MarketToTimeMkC1[(long)t->Market];
  if(TimeMkC1 == TimeHotCPE)
    return ;
  {
    uchar *p = t->Por;
    A *= .01;
    if(TimeMkC1 == TimeIndC1PE)
    {
      long i = Ind1E - 1;
      while(++i <= Ind16E)
        MkAmntByCl[i] +=  *p++ *A;
      return ;
    }
    {
      long i = (long)t->Market *MaxBusCls - 1, To = i + MaxBusCls;
      while(++i <= To)
        MkAmntByCl[i] +=  *p++ *A;
    }
  }
}

/// <summary> This function returns the numerical interest rate of a time deposit either fixed rate or variable rate. </summary> 
/// <param name="*t"> is the pointer to the Time Deposit structure. </param>
/// <returns> the interest rate in the quarter period. </returns> 
float TimeRate(TimeDepT *t)
{
  float I = t->RateOrSpread;
  if( ! t->IsFixed)
    I += EconBOQ.Econ.One_Qtr_Treas;
  return I *.0025;
}

int gC;   // gC is 421's current column.
float gX; // gX  is the value that is added to a cell or cells.

/// <summary> This function accumulates several Total cells in B421 based on row. </summary>
/// <param name="R21b"> is the row in B421. </param>
void Sum421b(long R21b)
{
  DepOut[Bank]._421b[gC][R21b] += gX;
  DepOut[Bank]._421b[TotC21b][R21b] += gX;
}

/// <summary> This function ??? </summary>
/// <param name="*d"> is data structure for Retail Certificates  </param>
/// <param name="*t"> is data structure for Retail Certificates.</param>
/// <param name="X"> is some dollar Amount in millions.</param>
void Rpt421b(TimeDecT *d, TimeDepT *t, float X)
{
  switch((int)t->Market)
  {
    case HotE:
      gC = HotC21b;
      break;
    case IndE:
      gC = IndC21b;
      break;
    default:
      gC = BusC21b;
  }
  gX = X;
  Sum421b(TotR21b);
  Sum421b(((LnP)d - (LnP)DeposDec[Bank].Time.TimeDec) / sizeof(TimeDecT));
}

/// <summary> This function accumulates several Total cells in B421 based on row. </summary>
/// <param name="R21a"> is the row in B421. </param>
/// <remarks> If the row is 1-6, it calculates both Total Total and Customer Total. If the row is 8-12, it calculates only Total Total.  </remarks> 
void Sum421a(int R21a)
{
  DepOut[Bank]._421a[gC][R21a] += gX; //Beginning balance.
  if(R21a >= TotR21aF)
    return ;
  if(R21a < CusR21aT)
    DepOut[Bank]._421a[gC][CusR21aT] += gX;
  switch(R21a)
  {
    case BusR21aF:
    case BusR21aV:
      DepOut[Bank]._421a[gC][BusR21aT] += gX;
      break;
    case IndR21aF:
    case IndR21aV:
      DepOut[Bank]._421a[gC][IndR21aT] += gX;
      break;
    default:
      DepOut[Bank]._421a[gC][HotR21aT] += gX;
  }
}

/// <summary> This function accumulates several Total lines for the Rpt421a section inside Deposits(). </summary>
/// <param name="C21a"> is the B421 Column. </param>
/// <param name="*t"> is pointer to a time deposit structure.</param>
/// <param name="X"> is some dollar Amount in millions.</param>
/// <remarks> If IntC21a row in Report 421, this function calculates BOA[Bank][YrsQtr].IncomeOutGo.Interest.Expenses.BOA_Dep.SavCerts. 
/// It also calcuates DepOut[Bank]._421a[gC][R21a] by using Sum421a(). </remarks> 
void Rpt421a(int C21a, TimeDepT *t, float X)
{
  long MktI = (long)t->Market;
  gC = C21a;
  gX = X;
  if(C21a == IntC21a)
    BOA[Bank][YrsQtr].IncomeOutGo.Interest.Expenses.BOA_Dep.SavCerts += X;
  Sum421a(TotR21aT); //Sum the total for both fixed and variable. 
  Sum421a(t->IsFixed ? TotR21aF : TotR21aV);
  switch(MktI)
  {
    case HotE:
      Sum421a(t->IsFixed ? HotR21aF : HotR21aV);
      break;
    case IndE:
      Sum421a(t->IsFixed ? IndR21aF : IndR21aV);
      break;
    default:
      Sum421a(t->IsFixed ? BusR21aF : BusR21aV);
  }
}

/// <summary> This function accumulates a cell in B401 based on row and column. </summary>
/// <param name="R"> is the row in B401. It is usually a row for total or subtotal.</param>
/// <remarks> This function calculates the Total Beginning Balance, and Total Net Charge. </remarks>
void Sum_401(int R)
{
  // Row,  subtotal and grandtotal sum the dollar amount.
  DepOut[Bank]._401[gC][R] += gX;  //Begnning Balance. 
  if(R == TotCorR01)
  {
    if(gC == EndC01)
      BOA[Bank][YrsQtr].CoreDep += gX;
    if(gC == NewC01)
      NewDep[Bank] += gX;
  }
  if(gC == SerC01)  
    DepOut[Bank]._401[NetC01][R] += gX;  //Add Service Charge Fee to the Total Net Charge 
  if(gC == CredC01)
    DepOut[Bank]._401[NetC01][R] +=  - gX;  //subtract Credit on Bal from the Total Net Charge
}

/// <summary> This function arranges the core deposits data for B401 in order to determine next quarter's loans. </summary>
/// <param name="C01"> is the B401 Column. </param>
/// <param name="X"> is the value to place in a cell (e.g. balance), usually a dollar Amount in millions. </param>
/// <param name="ClsProdI"> is the class of the deposit. </param>
/// <remarks> This function calculates BOA[Bank][YrsQtr].IncomeOutGo.Interest.Expenses.BOA_Dep based on EconInfo.Prod 
/// if IntC01 and BusSave and it calculates DepOut[Bank]._401[gC][R] by Sum_401(). </remarks>
void Rpt_401(int C01, float X, int ClsProdI)
{
  EconInfoT *e = EconInfo + ClsProdI;
  int MktProd = e->MktProd;
  // Here are B401's columns:
  //   BegC01, WitC01, NewC01, IntC01, SerC01, CredC01, NetC01, 
  //   EndC01, NumAccsC01, NumItemsC01, NumNSFsC01, Num_401_Cols
  // These are the rows:
  //   NatR01, MidR01, LocR01, OthR01, TotBusR01, IndDemR01, PubR01, TotDemR01,
  //   NowR01, BusSavR01, IndSavR01, TotSavR01, MonPremR01, MonLowR01, TotMonR01, CdsR01, TotCorR01, Num_401_Rows
  gC = C01;
  gX = X;
  ProdI = e->Prod; //  Sets Globals for subroutines.
  if(C01 == IntC01 && ProdI <= BusSaveE)
  {
    // Payout interest
    BOA_DepT *Int =  &BOA[Bank][YrsQtr].IncomeOutGo.Interest.Expenses.BOA_Dep;
    DemIntT *di = &BOA[Bank][YrsQtr].DemInt;
    switch(ProdI)
    {
      case IndNOWE:
        Int->Now += X;
        break;
      case IndMonE:
        Int->MonMkt += X;
        break;
      case IndSaveE:
      case BusSaveE:
        Int->Sav += X;
        break;
      case DemBusE:
        di->Bus += X;
        break;
      case DemPubE:
        di->Pub += X;
        break;
      case DemIndE:
        di->Ind += X;
        break;
    }
  }
  Sum_401(TotCorR01); //  Everything goes to the bottom line here.
  if(MktProd <= DemPubMPE || (MktProd >= BusNatDemMPE && MktProd <= BusGenDemMPE))
    Sum_401(TotDemR01);   // This is the  " Total Demand "  subtotal.
  if(MktProd >= BusNatDemMPE && MktProd <= BusGenDemMPE)
  {
    Sum_401(TotBusR01); // This is the  " Total Business Demand "  subtotal.
    switch(MktProd)
    {
      case BusNatDemMPE:
        Sum_401(NatR01);
        break;
      case BusMidDemMPE:
        Sum_401(MidR01);
        break;
      case BusLocDemMPE:
        Sum_401(LocR01);
        break;
      default:
        Sum_401(OthR01);
        break;
    }
  }
  switch(MktProd)
  {
    case DemIndMPE:
      Sum_401(IndDemR01);
      break;
    case DemPubMPE:
      Sum_401(PubR01);
      break;
    case IndNOWMPE:
      Sum_401(NowR01);
      break;
    case IndSaveMPE:
    case BusNatSaveMPE:
    case BusMidSaveMPE:
    case BusLocSaveMPE:
    case BusReSaveMPE:
    case BusProSaveMPE:
    case BusGenSaveMPE:
      Sum_401(TotSavR01); // The  " Total Savings "  subtotal.
      Sum_401(MktProd == IndSaveMPE ? IndSavR01 : BusSavR01);
      break;
    case IndMonMPE:
      Sum_401(TotMonR01); // The  " Total Money Market "  subtotal.
      Sum_401(e->MinBal >= DepTypeInfo[Bank][IndMonE].MinBal ? MonPremR01  // Balance is enough for the higher rate.
      : MonLowR01);
      break;
  }
  if(ProdI == TimeBusE || ProdI == TimeIndE)
    Sum_401(CdsR01);  //Retail Certificate row. 
};

/// <summary> This function calls Rpt_401() if there is a valid market type deposit. </summary>
/// <param name="C"> is the B401 Column. </param>
/// <param name="*t"> is the pointer to a time deposit structure.</param>
/// <param name="X"> is the balance in millions dollar amount.</param>
void Time401(int C,  TimeDepT *t,  float X)
{
  if(t->Market != HotE)
    Rpt_401(C, X, t->Market == IndE ? TimeIndC1PE : BusNatTimeC1PE);
}
/// <summary> This function returns the row number in B460 based on the EconInfoT->Prod.</summary>
/// <param name="*e"> is the pointer to EconInfoT. </param>
/// <returns> This function returns the row number in report B460. </returns>
int B460_Row(EconInfoT *e)
{
  int m = e->Mkt, c = e->Cls;
  if(PrivBkng[Bank] && (c == Ind15E || c == Ind16E || m == BusProE || m == BusGenE))
    return Privt;
  switch(e->Prod)
  {
    case DemBusE:
      return B_DD;
    case BusSaveE:
      return B_Sav;
    case TimeBusE:
      return B_CD;
    case DemPubE:
      return P_DD;
    case DemIndE:
      return I_DD;
    case IndNOWE:
      return I_NOW;
    case IndMonE:
      return I_MM;
    case IndSaveE:
      return I_Sav;
    case TimeIndE:
    default:
      return I_CD;
  }
}

/// <summary> This function collects time deposit data for the B460 report. </summary>
/// <param name="*R"> is the pointer to B460ST,which could be Beg, End, Int, Fees and Cred in Report B460. </param>
/// <param name="*t"> is the pointer to Time Deposit. </param>
/// <param name="X"> is some dollar amount in millions.</param>
/// <remarks> This function calculates DepOut[Bank].B460.B460ST[row]. The row is determined by EconInfo. </remarks>
void Time460(float *R, TimeDepT *t, float X)
{
  if(t->Market == HotE)
  {
    R[10] += X;
    return ;
  }
  {
    EconInfoT *e = EconInfo + MarketToTimeMkC1[(int)t->Market] - 1;
    uchar *p = t->Por - 1;
    X *= .01;
    {
      for(int intIndexOfCount = 0; intIndexOfCount < MaxTimeCls; intIndexOfCount++)
      {
        p++;
        e++;
        if( !  *p)
          continue;
        R[B460_Row(e)] +=  *p * X;  
      }
    }
  }
}

B465T _465_Rows;  

/// <summary> This function returns the row number in B465 based on the EconInfoT->Prod.</summary>
/// <param name="*e"> is the pointer to EconInfoT. </param>
/// <returns> This function returns the row number in report B465. </returns>
int B465_Row(EconInfoT *e)
{

  int m = e->Mkt, c = e->Cls;
  if( ! (PrivBkng[Bank] && (c == Ind15E || c == Ind16E || m == BusProE || m == BusGenE)))
    return  - 1;
  switch(e->Prod)
  {
    case DemBusE:
      return _465_Rows.B_DD;
    case BusSaveE:
      return _465_Rows.B_Sav;
    case TimeBusE:
      return _465_Rows.B_CD;
    case DemIndE:
      return _465_Rows.I_DD;
    case IndNOWE:
      return _465_Rows.I_NOW;
    case IndMonE:
      return _465_Rows.I_MM;
    case IndSaveE:
      return _465_Rows.I_Sav;
    case TimeIndE:
      return _465_Rows.I_CD;
    default:
      return  - 1;
  }
}

/// <summary> This function collects time deposit data for the B465 report. </summary>
/// <param name="*R"> is the pointer to B465ST[_465_Rows.Rows], which could be Beg, W_M, New, End, Int, Fees and Cred if B465a
/// or Beg, End, Int, Fees, Cred if B465b in Report B465. </param>
/// <param name="*t"> is the pointer to Time Deposit. </param>
/// <param name="X"> is some dollar amount in millions.</param>
/// <remarks> This function calculates DepOut[Bank].B465a(B465b).B465ST[row]. The row is determined by EconInfo by calling B465_Row. </remarks>
void Time465(float *R, TimeDepT *t, float X)
{
	extern long rv;

  if(t->Market == HotE)
    return ;
  EconInfoT *e = EconInfo + MarketToTimeMkC1[(int)t->Market] - 1;
  uchar *p = t->Por - 1;
  X *= .01;
  //Loop(MaxTimeCls)
  for(int J = 0; J < MaxTimeCls; J++)
  {
    if(++e,  !  *++p)
      continue;
    if((rv = B465_Row(e)) >= 0)
      R[rv] +=  *p * X;
  }
}

/// <summary> This function goes through all the time deposits and does quarterly actions on them. 
/// It calculates data for report B401, B421, B460, and B465. </summary>
/// <remarks> It is called by Deposits(). The calculation is different when the maturity is <1, 1-4 and >4. </remarks>
void UpdateTime()
{
  TimeDepT *t = TimeDep[Bank] - 1;
  B460T *B460 = &DepOut[Bank].B460;
  B465aT *B465a = &DepOut[Bank].B465a;
  B465bT *B465b = &DepOut[Bank].B465b;
  {
    //Loop(MaxTimeDep)
	for(int J = 0; J < MaxTimeDep; J++)
    {
      ++t;
      {
        float Bal = t->Bal;
		float Mat = t->Mat;
        if( ! Mat)
          continue;
        Time401(BegC01, t, Bal);
        Rpt421a(BegC21a, t, Bal);
        Time460(B460->Beg, t, Bal);
        Time465(B465a->Beg, t, Bal);
        Time465(B465b->Beg, t, Bal); 
        {
          float R = TimeRate(t);
          if(Mat == 1)
          {
            float I = R * Bal * .5;
            Bal += I;
            Time401(IntC01, t, I);
            Time401(WitC01, t, Bal);
            Rpt421a(IntC21a, t, I);
            Rpt421a(MatC21a, t, Bal);
            DrainTimeMksCls(Bal, t);
            Time460(B460->Int, t, I);
            Time465(B465a->Int, t, I);
            Time465(B465a->W_M, t, Bal);
            Time465(B465b->Int, t, I);
            memset(t, 0, sizeof(*t));
            continue;
          }
          if(Mat > 4)
          {
            while(1)
            {
              if(t->Market == HotE)
                break;
              {
                float I, W;
				float T = .0025 * YldC(Mat, BoQ);
				float With = DepK.TimeWith[t->Market == IndE]*(Pow(1+T, Mat)*(1-2 * R) / Pow(1+R, Mat) - 1.1);
                if(With <= 0)
                  break;
                if(With > 1)
                  With = 1;
                t->Cnt *= 1-With;
                W = t->Bal *With;
                if(t->Bal - W < DepK.MinKeepBal)
                  W = t->Bal;
                I =  - 1.5 * R * W;
                t->Bal += I;
                if(t->Bal - W < DepK.MinKeepBal)
                  W = t->Bal;
                Time401(IntC01, t, I);
                Time401(WitC01, t, W);
                Rpt421a(IntC21a, t, I);
                Rpt421a(WitC21a, t, W);
                Time465(B465a->Int, t, I);
                Time465(B465a->W_M, t, W);
                Time465(B465b->Int, t, I);
                Time460(B460->Int, t, I);
                DrainTimeMksCls(W, t);
                if(t->Bal == W)
                  memset(t, 0, sizeof(*t));
              }
              break;
            }
          }
          if( ! t->Bal)
            continue;
          t->Mat--;
          {
            float I = R * t->Bal;
            t->Bal += I;
            Time401(IntC01, t, I);
            Rpt421a(IntC21a, t, I);
            Time460(B460->Int, t, I);
            Time465(B465a->Int, t, I);
            Time465(B465b->Int, t, I);
          }
        }
      }
    }
  }
}

/// <summary> This function calculates the effective deposit rate after taking fees into account. </summary> 
/// <param name="*p"> is the pointer to the deposit type information data structure.  </param>
/// <param name="MoCnt1"> ??? </param>
/// <param name="MoCnt2"> ??? </param>
/// <param name="AvgSize"> is the average size of deposits. </param>
/// <param name="Rate"> is the deposit rate before considering any fee.  </param>
/// <returns> the effective deposit rate. </returns> 
float RealRate(DepTypeInfoT *p, float MoCnt1, float MoCnt2, float AvgSize, float Rate)
{
  float Costs = 1200 *(p->MnFee + MoCnt1 * p->ItemFee + MoCnt2 * p->OthFee);
  return Rate - ( ! AvgSize ? 0 : Costs / AvgSize);
}

//determines the withdrawl rate, This calculates the effective rate a depositor is getting at the bank after taking fees into account. 
/// <summary> This function assigns values to several fields in EconInfo and BankInfo.  </summary>
/// <param name="ClsProdI"> is the deposit class. </param>
/// <param name="BankRate"> is the bank rate. </param>
/// <param name="EconRate"> is the Econ rate. </param>
/// <param name="Loy"> is the "loyalty factor" which is unique to each bank as well as varying by market, type of account and customer class. </param>
/// <param name="*Bal"> is the beginning balance. </param>
/// <param name="*Cnt"> </param>
/// <param name="MoCnt1"> </param>
/// <param name="MoCnt2"> </param>
/// <remarks> It is called inside Deposits(). </remarks> 
void SetPrices(int ClsProdI, float BankRate, float EconRate, float Loy, float *Bal, float *Cnt, float MoCnt1, float MoCnt2)
{
  EconInfoT *e = EconInfo + ClsProdI;
  BankInfoT *b = BankInfo[Bank] + ClsProdI;
  DepTypeInfoT *d = DepTypeInfo[Bank] + e->Prod;
  b->BegBal =  *Bal;
  b->Bal = Bal;
  b->BegCnt =  *Cnt;
  b->Cnt = Cnt;
  b->Loy = Loy;
  b->BankRate = BankRate;
  {
    float R = (e->Prod == IndNOWE || e->Prod == IndMonE) && e->MinBal >= d->MinBal ? BankRate : RealRate(d, MoCnt1, MoCnt2, e->AvgSize, BankRate);
    e->Tot += b->RealRate = R;
  }
  if(e->Prod <= DemPubE)
    b->RealRate = min(b->RealRate, 0);
  if(Bank)
    return ;
  e->RealRate = RealRate(Prices.DepTypeInfo + e->Prod, MoCnt1, MoCnt2, e->AvgSize, EconRate);
  if(e->Prod <= DemPubE)
    e->RealRate = min(e->RealRate, 0);
  e->Rate = EconRate;
}

/// <summary> This function computes the withdrawals from a given type of account and class of customer. It may be used for any account which does not have fixed maturities. </summary>
/// <remarks> Time Deposit has the fixed maturity. Demand, NOW, savings, and money market accounts do not have fixed maturities.</remarks> 
void NonTimeWithAttrib()
{
  {
    float BusLoy[MaxLoy];
    memmove(BusLoy, LnsByBusMk[Bank], sizeof(BusLoy));
    {
      //Loop(MaxLoy)
	  for(int J = 0; J < MaxLoy; J++)
      {
        if(BusLoy[J])
          BusLoy[J] /= SumLnsByBusMk[J];
      }
    }
    {
      BusDepClT *Port = Dep[Bank].BusNatCl - 1;
      BusDepClKT *ClsK = DepK.BusNatClK - 1;
      long DemI = BusNatDemC1PE - 1;
	  long SavI = BusNatSaveC1PE - 1;
	  long TimI = BusNatTimeC1PE - 1;
      ClsI = BusNat1E - 1;
      while(++ClsI <= BusGen4E)  //if the deposit class is business class
      {
        ++Port;
        ++DemI;
        ++SavI;
        ++TimI;
        ++ClsK;
        EconInfo[DemI].Checks = ClsK->ItemCntMo;
        {
          float Loy = ClsK->Loy;
          if(ClsI <= BusPro4E)
          {
            float W = ClsK->Loy1, W1 = 1-W;
            Loy = Port->Loy *W + W1 * Loy + W1 * ClsK->Loy2 *Pow(BusLoy[EconInfo[DemI].Mkt], 2);
          }
          BankInfo[Bank][TimI].Loy = Port->Loy = Loy;
          SetPrices(DemI, DeposDec[Bank].Demand.Credit.Busi, EconBOQ.Econ.MM, Loy, &Port->DemBal, &Port->DemCnt, ClsK->ItemCntMo, ClsK->DeposCntMo);
          SetPrices(SavI, DeposDec[Bank].InterestBearing.Busi.Rate, EconBOQ.Econ.SAV, Loy, &Port->SavBal, &Port->SavCnt, 0, 0);
        }
      }
    }
  }
  {
    PubDepClT *Port = Dep[Bank].PubDepCl - 1;
    PubDepClKT *ClsK = DepK.PubDepClK - 1;
    long DemI = DemPubC1PE - 1;
    ClsI = Pub1E - 1;
    while(++ClsI <= Pub7E)
    {
      ++DemI;
      ++Port;
      ++ClsK;
      EconInfo[DemI].Checks = ClsK->ItemCntMo;
      SetPrices(DemI, DeposDec[Bank].Demand.Credit.Pub, EconBOQ.Econ.MM, ClsK->Loyalty, &Port->Bal, &Port->Cnt, ClsK->ItemCntMo, ClsK->DeposCntMo);
    }
  }
  {
    IndDepClT *Port = Dep[Bank].IndDepCl - 1;
    IndDepClKT *ClsK = DepK.IndDepClK - 1;
    long DemI = DemIndC1PE - 1, SavI = IndSaveC1PE - 1, NowI = IndNOWC1PE - 1, MonI = IndMonC1PE - 1, TimI = TimeIndC1PE - 1;
    ClsI = Ind1E - 1;
    while(++ClsI <= Ind16E)
    {
      ++DemI;
      ++SavI;
      ++NowI;
      ++MonI;
      ++Port;
      ++TimI;
      ++ClsK;
      if( ! Bank)
      {
        EconInfo[MonI].MinBal = ClsK->MinBalMoneyMk;
        EconInfo[NowI].MinBal = ClsK->MinBalCheck;
      }
      {
        float Loy = ClsK->Loyalty;
        BankInfo[Bank][TimI].Loy = Loy;
        SetPrices(SavI, DeposDec[Bank].InterestBearing.Indiv.Rate, EconBOQ.Econ.SAV, Loy, &Port->SavBal, &Port->SavCnt, 0, 0);
        SetPrices(MonI, DepTypeInfo[Bank][IndMonE].MinBal < ClsK->MinBalMoneyMk ? DeposDec[Bank].InterestBearing.MmRate: DeposDec[Bank].InterestBearing.MmBase, EconBOQ.Econ.MM, Loy, &Port->MonBal, &Port->MonCnt, 0, 0);
        {
          float MoCnt1 = ClsK->ItemCntMo, MoCnt2 = ClsK->NSFCntMo;
          EconInfo[DemI].Checks = EconInfo[NowI].Checks = MoCnt1;
          SetPrices(DemI, DeposDec[Bank].Demand.Credit.Indiv, EconBOQ.Econ.IDD, Loy, &Port->DemBal, &Port->DemCnt, MoCnt1, MoCnt2);
          SetPrices(NowI, DeposDec[Bank].InterestBearing.NowRate, EconBOQ.Econ.NOW, Loy, &Port->NowBal, &Port->NowCnt, MoCnt1, MoCnt2);
        }
      }
    }
  }
}

/// <summary> This function calcuates non-time deposit early withdrawls for a bank. </summary>
/// <remarks> It is called insdide Deposits(). 
/// The function calculates W for early withdraw. It adds W to MkAmntByCl and subtracts W from BankInfo->Bal. </remarks>
void NonTimeEarlyWith()
{
  BankInfoT *b = BankInfo[Bank] - 1;
  EconInfoT *e = EconInfo - 1;
  //Loop(BusGenSaveC4PE)
  for(int J = 0; J < BusGenSaveC4PE; J++)
  {
    long ClsProdI = J;
    ++b;
    ++e;
    {
      DepTypeInfoT *p = DepTypeInfo[Bank] + e->Prod;
      float T = e->Tot - b->RealRate;
		float E = (T + (12-NumBanks) *e->RealRate) / 11;
		float _R = 2 *(1-b->Loy)*(E-b->RealRate) / e->Rate;
		float R = e->Prod == IndMonE || e->Prod == IndSaveE || e->Prod == BusSaveE ? _R *p->MnFee / 2.5: _R;
		float R2 = R <= 0 ? 0 : Pow(R, 2.5);
		float w = (R2 + 1) *DepK.NonTimeWith[e->Prod];
		float With = min(w, 1);
		float W =  *b->Bal *With;
       *b->Cnt *= 1-With;
      if(*b->Bal - W < DepK.MinKeepBal)
      {
        W =  *b->Bal;
         *b->Cnt = 1;
      }
		*b->Bal -= W;  //Move this statement to here from the bottom.  
      MkAmntByCl[e->Cls] += W;
   
		Rpt_401(WitC01, W, ClsProdI);
      {
        long r = B465_Row(e);
        if(r >= 0)
          DepOut[Bank].B465a.W_M[r] += W;
      }
    }
  }
}

/// <summary> This function calculates the overall favorability of a deposit product for a class. </summary>
/// <param name="ClsProdI"> is the deposit class. </param>
/// <remarks> It sets BankInfo[Bank]->FinalAttrib and EconInfo->FinalAttrib10. </remarks>
void SetBasePrice(long ClsProdI)
{
  BankInfoT *b = BankInfo[Bank] + ClsProdI;
  EconInfoT *e = EconInfo + ClsProdI;
  {
    float FinalAttrib = b->AdjRate *b->Attrib, t = (b->AdjRate *(1+b->Loy) - 1) *DepK.RateTrap[e->MktProd] *Pow(b->Attrib, .5);
    if(t < 0)
      FinalAttrib -= t * t;
    e->FinalAttrib10 += b->FinalAttrib = max(FinalAttrib, 0);
  }
}

/// <summary> This function accumulates some information on how favorable a deposit product for a business class is. </summary>
/// <param name="ClsProdI"> is the deposit class. </param>
/// <param name="Media"> decides EffMedia[Bank][Media]. </param>  
//   given advertizing, etc. .
void SumBusAttrib(long ClsProdI, long Media)
{
  BankInfoT *b = BankInfo[Bank] + ClsProdI;
  float Loy = b->Loy,  *K = DepK.BusAttrib[ClsI - BusNat1E];
  b->Attrib = K[0] *Dep[Bank].BnkShr[ClsI] + K[1] *EffPrem[Bank][0] + K[2] *EffPrem[Bank][1] + K[3] *EffPrem[Bank][2] + K[4] *EffPrem[Bank][3] + K[5] *EffOps[Bank] + K[6] *EffSal[Bank][0 /*All Bus*/] + K[7] *Loy + K[8] *EffMedia[Bank][Media];
}

// Accumulates some information on how favorable a deposit product is for a consumer class,
//   given advertizing, etc. .
/// <summary> This function accumulates some information on how favorable a deposit product for a consumer class is. </summary>
/// <param name="ClsProdI"> is the deposit class. </param>
/// <param name="Media"> decides EffMedia[Bank][Media]. </param>  
//   given advertizing, etc. .
void SumIndAttrib(long ClsProdI, long Media)
{
  BankInfoT *b = BankInfo[Bank] + ClsProdI;
  float *K = DepK.IndAttrib[ClsI - Ind1E];
  b->Attrib = K[0] *Dep[Bank].BnkShr[ClsI] + K[1] *EffPrem[Bank][0] + K[2] *EffPrem[Bank][1] + K[3] *EffPrem[Bank][2] + K[4] *EffPrem[Bank][3] + K[5] *EffOps[Bank] + K[6] *EffSal[Bank][1] + K[7] *EffSal[Bank][2] + K[8] *EffSal[Bank][3] + K[9]*(SumCusLns ? CusLns[Bank] / SumCusLns: 0) + K[10] *EffMedia[Bank][Media];
}

/// <summary> This function adjusts a rate that a bank is offering for a product and class of a time deposit. </summary>
/// <param name="ClsProdI"> is the deposit class. </param>
/// <param name="MatPref"> is the maturity preference. </param>
/// <param name="FixPorPref"> is the fixed or variable rate. </param>
void SetAdjRateTime(long ClsProdI, float MatPref, float FixPorPref)
{
  BankInfoT *b = BankInfo[Bank] + ClsProdI;
  TimeDecT *TDec = DeposDec[Bank].Time.TimeDec - 1;
  if(MatPref < 1)
    MatPref = 1;
  {
    float MaxAdjRate = 0;
    long DecI =  - 1;
    while(++DecI < MaxTimeDec)
    {
      ++TDec;
      {
        float Mat = TDec->Mat;
        long IsFixed = TDec->Fixed == F;
        if(Mat <= 0 || TDec->Rate <= 0 || ( ! IsFixed && TDec->Fixed != V))
          continue;
        {
          float PorPref = IsFixed ? FixPorPref : 1-FixPorPref, x = 1-.08 * fabs(Mat - MatPref) / Pow(MatPref, .7), y = 1.1 *(x < 0 ? 0 : x) *PorPref*TDec->Rate / (IsFixed ? YldC(Mat, BoQ): EconBOQ.Econ.One_Qtr_Treas);
          if(y <= MaxAdjRate)
            continue;
          MaxAdjRate = y;
          b->TimeDec = TDec;
          b->AdjRate = y / PorPref;
        }
      }
    }
  }
}

// returns the number of percentage points added to a product.
float DepConv[] =
{
  1, 1, 1, 1, .5, .3, .3
};

/// <summary> This function determines the market share, calculates what rate a bank is offering for a product and class. </summary>
/// <param name="ClsProdI"> is the deposit class. </param>
/// <param name="MoCnt1"> is the monthly item for an account. </param>
/// <param name="MoCnt2"> is the monthly deposit item for an account. </param>
/// <remarks> It is used inside Deposits(). <remarks>
void SetAdjRate(int ClsProdI, float MoCnt1, float MoCnt2)
{
  BankInfoT *b = BankInfo[Bank] + ClsProdI;
  EconInfoT *e = EconInfo + ClsProdI;
  long Prod = e->Prod;
  DepTypeInfoT *d = DepTypeInfo[Bank] + Prod;
  float AdjRate, EconRate = (Prod == DemBusE || Prod == DemPubE) ? EconBOQ.Econ.RPR : EconBOQ.Econ.One_Qtr_Treas;
  e->MoCnt1 = MoCnt1;
  e->MoCnt2 = MoCnt2;
  float ChargeRate = (e->Prod == IndNOWE || e->Prod == IndMonE) && e->MinBal >= d->MinBal ? 0 : RealRate(d, MoCnt1, MoCnt2, e->AvgSize, 0);
  AdjRate = DepConv[Prod] + (1.25 *ChargeRate + b->BankRate) / EconRate;
  b->AdjRate = (Prod <= DemPubE && AdjRate > 1) ? 1 : AdjRate;
}

/// <summary> This function sets the competitive price of a deposit for a bank. </summary>
/// <remarks> It is called inside Deposits(). </remarks>
void SetCompPrice(long ClsProdI, float Sub, float MoCnt1, float MoCnt2, long PayInt, float _EconRate)
{
  EconInfoT *e = EconInfo + ClsProdI;
  long Prod = e->Prod;
  DepTypeInfoT *m = Prices.DepTypeInfo + Prod;
  float EconRate = (Prod == DemBusE || Prod == DemPubE) ? EconBOQ.Econ.RPR : EconBOQ.Econ.One_Qtr_Treas, CmpPrice = RealRate(m, MoCnt1, MoCnt2, e->AvgSize, _EconRate);
  CmpPrice = DepConv[Prod] + CmpPrice / EconRate;
  e->Sub = Sub;
  e->PayInt = PayInt;
  if( ! PayInt)
    CmpPrice = min(CmpPrice, 1);
  e->Comp = CmpPrice * m->Attrib;
}

// interest paid on demand deposits.
PayIntOnDemDepT PayIntOnDemDep;
/// <summary> This function calculates how attactive bank's products are by class. </summary>
void ClassShareDeterminants()
{
  {
    BusDepClKT *ClsK = DepK.BusNatClK - 1;
    long DemI = BusNatDemC1PE - 1, SavI = BusNatSaveC1PE - 1, TimI = BusNatTimeC1PE - 1;
    ClsI = BusNat1E - 1;
    while(++ClsI <= BusGen4E)
    {
      ++DemI;
      ++SavI;
      TimI++;
      ++ClsK;
      SumBusAttrib(DemI, 0);
      SumBusAttrib(SavI, 1);
      SumBusAttrib(TimI, 7);
      SetAdjRate(DemI, ClsK->ItemCntMo, ClsK->DeposCntMo);
      SetAdjRate(SavI, 0, 0);
      SetAdjRateTime(TimI, ClsK->MatPref, ClsK->FixPorPref);
      SetBasePrice(DemI);
      SetBasePrice(SavI);
      SetBasePrice(TimI);
      if(Bank)
        continue;
      {
        int PayInt = PayIntOnDemDep.Bus;
        SetCompPrice(DemI, ClsK->DemandSubsti, ClsK->ItemCntMo, ClsK->DeposCntMo, PayInt, EconBOQ.Econ.RPR);
        SetCompPrice(SavI, ClsK->SavingsSubsti, 0, 0, PayInt, EconBOQ.Econ.SAV);
      }
      {
        EconInfoT *e = EconInfo + TimI;
        DepTypeInfoT *m = Prices.DepTypeInfo + e->Prod;
        e->Comp = m->Attrib;
      }
    }
  }
  {
    PubDepClKT *ClsK = DepK.PubDepClK - 1;
    long DemI = DemPubC1PE - 1;
    ClsI = Pub1E - 1;
    while(++ClsI <= Pub7E)
    {
      ++DemI;
      ++ClsK;
      {
        float *K = DepK.PubAttrib[ClsI - Pub1E], Bal = Dep[Bank].PubDepCl[ClsI - Pub1E].Bal, Ratio = BOA[Bank][YrsQtr].Bal.Assets.Securities.Treasuries / (Bal ? Bal : .0001);
        BankInfo[Bank][DemI].Attrib = K[0] *Dep[Bank].BnkShr[ClsI] + K[1] *EffPrem[Bank][1] + K[2] *EffPrem[Bank][3] + K[3] *EffOps[Bank] + K[4] *EffSal[Bank][0 /*All Bus*/] + K[5]*(Ratio < 1 ? 0 : (Ratio < 1.1 ? 1 : 1.2)) + K[6] *EffMedia[Bank][2 /*DemPubE*/];
      }
      SetAdjRate(DemI, ClsK->ItemCntMo, ClsK->DeposCntMo);
      SetBasePrice(DemI);
      if(Bank)
        continue;
      SetCompPrice(DemI, 1, ClsK->ItemCntMo, ClsK->DeposCntMo, PayIntOnDemDep.Pub, EconBOQ.Econ.RPR);
    }
  }
  {
    IndDepClKT *ClsK = DepK.IndDepClK - 1;
    long DemI = DemIndC1PE - 1, SavI = IndSaveC1PE - 1, NowI = IndNOWC1PE - 1, MonI = IndMonC1PE - 1, TimI = TimeIndC1PE - 1;
    ClsI = Ind1E - 1;
    while(++ClsI <= Ind16E)
    {
      ++DemI;
      ++SavI;
      ++NowI;
      ++MonI;
      ++TimI;
      ++ClsK;
      SumIndAttrib(DemI, 3);
      SumIndAttrib(SavI, 4);
      SumIndAttrib(NowI, 5);
      SumIndAttrib(MonI, 6);
      SumIndAttrib(TimI, 7);
      SetAdjRate(SavI, 0, 0);
      SetAdjRate(MonI, 0, 0);
      {
        float MoCnt1 = ClsK->ItemCntMo, MoCnt2 = ClsK->NSFCntMo;
        SetAdjRate(DemI, MoCnt1, MoCnt2);
        SetAdjRate(NowI, MoCnt1, MoCnt2);
      }
      SetAdjRateTime(TimI, ClsK->MatPref, ClsK->FixPorPref);
      SetBasePrice(DemI);
      SetBasePrice(SavI);
      SetBasePrice(NowI);
      SetBasePrice(MonI);
      SetBasePrice(TimI);
      {
        BankInfoT *Dem = BankInfo[Bank] + DemI,  *Now = BankInfo[Bank] + NowI;
        if(Now->RealRate <= Dem->RealRate)
          Now->Deleted = 1;
        else
          Dem->Deleted = 1;
      }
      if(Bank)
        continue;
      {
        EconInfoT *Dem = EconInfo + DemI,  *Now = EconInfo + NowI;
        if(Now->RealRate <= Dem->RealRate)
          Now->Deleted = 1;
        else
          Dem->Deleted = 1;
      }
      {
        float Cnt = ClsK->ItemCntMo;
        SetCompPrice(DemI, ClsK->CheckSubsti, Cnt, ClsK->NSFCntMo, PayIntOnDemDep.Ind, EconBOQ.Econ.IDD);
        SetCompPrice(SavI, ClsK->SavingsSubsti, 0, 0, 1, EconBOQ.Econ.SAV);
        SetCompPrice(NowI, ClsK->CheckSubsti, Cnt, ClsK->NSFCntMo, 1, EconBOQ.Econ.NOW);
        SetCompPrice(MonI, ClsK->MoneyMkSubsti, 0, 0, 1, ClsK->MinBalMoneyMk < EconInfo[MonI].MinBal ? EconBOQ.Econ.SAV : EconBOQ.Econ.MM);
      }
      {
        EconInfoT *e = EconInfo + TimI;
        DepTypeInfoT *m = Prices.DepTypeInfo + e->Prod;
        e->Comp = m->Attrib;
      }
    }
  }
}

/// <summary> This function determines what the market looks like for each class of depositor. </summary>
/// <remarks> It sets FinalAttribByCls[]. It is always called by Deposits(). </remarks>
void SetClsPrices()
{
  EconInfoT *e = EconInfo - 1;
  int ClsProdI =  - 1;
  memset(FinalAttribByCls, 0, sizeof(FinalAttribByCls));
  while(++ClsProdI <= TimeIndC16PE)
  {
    ++e;
    if(e->Deleted)
      continue;
    float C = max(e->Comp, 0), R = DepK.Response[e->MktProd], E = ((10-NumBanks)*(C + R * e->FinalAttrib10 / NumBanks)) / (1+R);
    FinalAttribByCls[e->Cls] += (e->FinalAttrib10 += E);
  }
}

float ProdUsed[MaxB][Ind16E + 1];

/// <summary> This function determines the market share each bank gets by class and product. </summary>
/// <remarks> It adds the share to Dep[Bank].BnkShr[e->Cls] </remarks> 
void ClsShare()
{
  EconInfoT *e = EconInfo - 1;
  long ClsProdI =  - 1, ClsI =  - 1;
  memset(ProdUsed, 0, sizeof(ProdUsed));
  while(++ClsProdI <= TimeIndC16PE)
  {
    ++e;
    {
      int LastProd = e->Mkt == PubE || e->Prod >= TimeBusE;
      float C10 = FinalAttribByCls[e->Cls], P10 = e->FinalAttrib10, Chance = e->Pref + (C10 <= 0 ? 0 : e->Sub *(P10 / C10 - e->Pref));
      LB
      {
        BankInfoT *b = BankInfo[Bank] + ClsProdI;
        if(b->Deleted)
          continue;
        {
          float Prop = LastProd ? 1-ProdUsed[Bank][e->Cls]: Chance;
          ProdUsed[Bank][e->Cls] += Prop;
          Prop = Bound(Prop, 0., 1.);
          {
            float Shr = (P10 <= 0 ? 0 : b->FinalAttrib / P10 * Prop);
            Dep[Bank].BnkShr[e->Cls] += b->Shr = Shr = max(Shr, 0);
          }
        }
      }
    }
  }
}
/// <summary> This function returns RV_Index based on the deposit product from EconInfoT. </summary> 
/// <param name="*e"> is a EconInfoT. </param>
/// <returns> RV_Index from 0 - 12. </returns>
/// <remarks> It is called inside Deposits(). </remarks>  
int ToReqSals460(EconInfoT *e)
{
  int m = e->Mkt, c = e->Cls;
  if(PrivBkng[Bank] && (c == Ind15E || c == Ind16E || m == BusProE || m == BusGenE))
    return RV_PrivBkng;
  int B = (m >= BusProE) *3;
  switch(e->Prod)
  {
    case DemBusE:
      return RV_DemBusM1 + B;
    case BusSaveE:
      return RV_BusSaveM1 + B;
    case TimeBusE:
      return RV_TimeBusM1 + B;
    case DemPubE:
      return RV_DemPub;
    case DemIndE:
      return RV_DemInd;
    case IndNOWE:
      return RV_IndNOW;
    case IndMonE:
      return RV_IndMon;
    case IndSaveE:
      return RV_IndSave;
    case TimeIndE:
    default:
      return RV_TimeInd;
  }
}
/// Data structure for all Retail Certificates. 
typedef struct
{
  float TotNew, TotCnt, New[MaxTimeCls];
  TimeDecT *TimeDec;
} SelT;
SelT Sel[MaxTimeDec];
float RV_Factors[MaxB][All_Cnt][Sz_RVs];
float New, Cnt;

/// <summary> This function calculates new Time Deposit in a bank and sets the value to reports B401, B421, B460, B465. </summary>
/// <param name="MktI"> is deposit market class. </param>
void NewTime(long MktI)
{
  SelT *s = Sel - 1;
  //Loop(MaxTimeDec)
  for(int J = 0; J < MaxTimeDec; J++)
  {
    ++s;
    {
      TimeDecT *d = s->TimeDec;
      float New = s->TotNew, Cnt = s->TotCnt;
      if( ! d || New < DepK.MinCreateAmnt)
        break;
      {
        TimeDepT *t = TimeDep[Bank] - 1;
        //Loop(MaxTimeDep)
		for(int J = 0; J < MaxTimeDep; J++)
        {
          ++t;
          if(t->Mat)
            continue;
          t->Bal = New;
          t->Cnt = Cnt;
          t->Mat = d->Mat;
          t->Market = MktI;
          t->Id = (float)++Id[Bank];
          t->MktRate = t->Dur = t->MktVal = UnSet;
          {
            long IsFixed = d->Fixed == F;
            float Rate = d->Rate;
            t->IsFixed = IsFixed;
            t->RateOrSpread = IsFixed ? Rate : Rate - EconBOQ.Econ.One_Qtr_Treas;
          }
          Time401(NewC01, t, New);
          Rpt421a(NewC21a, t, New);
          Rpt421b(d, t, New);
          {
            EconInfoT *e = EconInfo + MarketToTimeMkC1[(long)t->Market] - 1;
            float *NewC = s->New - 1;
            //Loop(MaxTimeCls)
			for(int J = 0; J < MaxTimeCls; J++)
            {
              NewC++;
              e++;
              if( !  *NewC ||  ! e->AvgSize)
                continue;
              RV_Factors[Bank][NewCnt][ToReqSals460(e)] += (*NewC * Mill) / e->AvgSize;
              t->Por[J] = (uchar)Round(100 **NewC / New);
            }
          }
          Time465(DepOut[Bank].B465a.New, t, New);
          {
            float I = d->Rate *.0025 * t->Bal / 2;
            t->Bal += I;
            Time401(IntC01, t, I);
            Rpt421a(IntC21a, t, I);
            Time460(DepOut[Bank].B460.Int, t, I);
            Time465(DepOut[Bank].B465a.Int, t, I);
            Time465(DepOut[Bank].B465b.Int, t, I);
          }
          break;
        }
      }
    }
  }
}

/// <summary> This function calculates new deposit for a bank and writes to report B401. </summary> 
void NewProd()
{
	extern long rv;

  EconInfoT *e = EconInfo - 1;
  BankInfoT *b = BankInfo[Bank] - 1;
  Id[Bank] = (int)SimQtr *1000;
  Zero(Sel);
  Zero(RV_Factors[Bank]);
  {
    int ClsProdI =  - 1;
    MktI =  - 1;
    while(++ClsProdI <= TimeIndC16PE)
    {
      ++e;
      ++b;
      if( ! e->AvgSize)
        New = Cnt = 0;
      else
      {
        New = MkAmntByCl[e->Cls] *b->Shr;
        Cnt = New / (e->AvgSize *.000001);
      }
      if(ClsProdI < BusNatTimeC1PE)
      {
        if(New < DepK.MinCreateAmnt)
          continue;
        Rpt_401(NewC01, New, ClsProdI);
         *b->Bal += New;
         *b->Cnt += Cnt;
        if((rv = B465_Row(e)) >= 0)
          DepOut[Bank].B465a.New[rv] += New;
        RV_Factors[Bank][NewCnt][ToReqSals460(e)] += Cnt;
        continue;
      }
      {
        TimeDecT *d = b->TimeDec;
        static long ClsZ;
        if(MktI != e->Mkt)
        // First Class
        {
          MktI = e->Mkt;
          ClsZ =  - 1;
          memset(Sel, 0, sizeof(*Sel));
        }
        ClsZ++;
        if(d && New > .001)
        {
          long i =  - 1;
          SelT *s = Sel - 1;
          while(++i < MaxTimeDec)
          {
            ++s;
            {
              TimeDecT *D = s->TimeDec;
              if(D && D != d)
                continue;
              s->TotNew += New;
              s->TotCnt += Cnt;
              s->New[ClsZ] = New;
              s->TimeDec = d;
              break;
            }
          }
        }
        if(MktI != e[1].Mkt)
          NewTime(MktI);
      }
    }
  }
}

/// <summary> This function calculates new hot money available for a bank and writes it to report B421, B460. </summary> 
void NewHot()
{
  float TotNew = 0, NewByDec[MaxTimeDec], MaxNew = MaxHot[Bank];
  if( ! MaxNew)
    return ;
  memset(NewByDec, 0, sizeof(NewByDec));
  {
    TimeDecT *d = DeposDec[Bank].Time.TimeDec - 1;
    long DecI =  - 1;
    while(++DecI < MaxTimeDec)
    {
      ++d;
      {
        long IsFixed = d->Fixed == F;
        float Mat = d->Mat, Rate = d->Rate, AboveGov = Rate - (IsFixed ? YldC(Mat, BoQ): EconBOQ.Econ.One_Qtr_Treas);
        if(Mat <= 0 || Rate <= 0 || ( ! IsFixed && d->Fixed != V) || Mat > Prices.MaxHotMat || AboveGov < 0)
          continue;
        TotNew += NewByDec[DecI] = Prices.aBanksHotAvailByMat[(long)Mat - 1] *AboveGov * AboveGov;
      }
    }
  }
  if( ! TotNew)
    return ;
  {
    TimeDecT *d = DeposDec[Bank].Time.TimeDec - 1;
    float Scale = MaxNew / TotNew, ScaleDown = Scale > 1 ? 1 : Scale;
    //Loop(MaxTimeDec)
	for(int J = 0; J < MaxTimeDec; J++)
    {
      float New = ScaleDown * NewByDec[J];
      ++d;
      if(New <= 0)
        continue;
      {
        long IsFixed = d->Fixed == F;
        float Mat = d->Mat, Rate = d->Rate;
        TimeDepT *t = TimeDep[Bank] - 1;
        long i =  - 1;
        while(++i < MaxTimeDep)
        {
          ++t;
          if(t->Mat)
            continue;
          t->Bal = New;
          t->Cnt = 1;
          t->Mat = Mat;
          t->Market = HotE;
          t->Id = (float)++Id[Bank];
          t->IsFixed = IsFixed;
          t->MktRate = Rate;
          t->RateOrSpread = IsFixed ? Rate : Rate - EconBOQ.Econ.One_Qtr_Treas;
          t->MktRate = t->Dur = t->MktVal = UnSet; // EOQ Stuff
          Rpt421a(NewC21a, t, New);
          Rpt421b(d, t, New);
          {
            float I = Rate * .0025 * t->Bal / 2;
            t->Bal += I;
            Rpt421a(IntC21a, t, I);
            Time460(DepOut[Bank].B460.Int, t, I);
          }
          break;
        }
      }
    }
  }
}

char *ClsProdNames[] =
{
  "DemIndC1PE", "DemIndC2PE", "DemIndC3PE", "DemIndC4PE", "DemIndC5PE", "DemIndC6PE", "DemIndC7PE", "DemIndC8PE", "DemIndC9PE", "DemIndC10PE", "DemIndC11PE", "DemIndC12PE", "DemIndC13PE", """DemIndC14PE""", "DemIndC15PE", "DemIndC16PE", "DemPubC1PE", "DemPubC2PE", "DemPubC3PE", "DemPubC4PE", "DemPubC5PE", "DemPubC6PE", "DemPubC7PE", "IndNOWC1PE", "IndNOWC2PE", "IndNOWC3PE", "IndNOWC4PE", "IndNOWC5PE", "IndNOWC6PE", "IndNOWC7PE", "IndNOWC8PE", "IndNOWC9PE", "IndNOWC10PE", "IndNOWC11PE", "IndNOWC12PE", "IndNOWC13PE", \
	  "IndNOWC14PE", "IndNOWC15PE", "IndNOWC16PE", "IndMonC1PE", "IndMonC2PE", "IndMonC3PE", "IndMonC4PE", "IndMonC5PE", "IndMonC6PE", "IndMonC7PE", "IndMonC8PE", "IndMonC9PE", "IndMonC10PE", "IndMonC11PE", "IndMonC12PE", "IndMonC13PE", "IndMonC14PE", "IndMonC15PE", "IndMonC16PE", "IndSaveC1PE", "IndSaveC2PE", "IndSaveC3PE", "IndSaveC4PE", "IndSaveC5PE", "IndSaveC6PE", "IndSaveC7PE", "IndSaveC8PE", "IndSaveC9PE", "IndSaveC10PE", "IndSaveC11PE", "IndSaveC12PE", "IndSaveC13PE", "IndSaveC14PE", "IndSaveC15PE", \
	  "IndSaveC16PE", "BusNatDemC1PE", "BusNatDemC2PE", "BusNatDemC3PE", "BusNatDemC4PE", "BusMidDemC1PE", "BusMidDemC2PE", "BusMidDemC3PE", "BusMidDemC4PE", "BusLocDemC1PE", "BusLocDemC2PE", "BusLocDemC3PE", "BusLocDemC4PE", "BusReDemC1PE", "BusReDemC2PE", "BusReDemC3PE", "BusReDemC4PE", "BusProDemC1PE", "BusProDemC2PE", "BusProDemC3PE", "BusProDemC4PE", "BusGenDemC1PE", "BusGenDemC2PE", "BusGenDemC3PE", "BusGenDemC4PE", "BusNatSaveC1PE", "BusNatSaveC2PE", "BusNatSaveC3PE", "BusNatSaveC4PE", "BusMidSaveC1PE", \
	  "BusMidSaveC2PE", "BusMidSaveC3PE", "BusMidSaveC4PE", "BusLocSaveC1PE", "BusLocSaveC2PE", "BusLocSaveC3PE", "BusLocSaveC4PE", "BusReSaveC1PE", "BusReSaveC2PE", "BusReSaveC3PE", "BusReSaveC4PE", "BusProSaveC1PE", "BusProSaveC2PE", "BusProSaveC3PE", "BusProSaveC4PE", "BusGenSaveC1PE", "BusGenSaveC2PE", "BusGenSaveC3PE", "BusGenSaveC4PE", "BusNatTimeC1PE", "BusNatTimeC2PE", "BusNatTimeC3PE", "BusNatTimeC4PE", "BusMidTimeC1PE", "BusMidTimeC2PE", "BusMidTimeC3PE", "BusMidTimeC4PE", "BusLocTimeC1PE", "BusLocTimeC2PE", \
	  "BusLocTimeC3PE", "BusLocTimeC4PE", "BusReTimeC1PE", "BusReTimeC2PE", "BusReTimeC3PE", "BusReTimeC4PE", "BusProTimeC1PE", "BusProTimeC2PE", "BusProTimeC3PE", "BusProTimeC4PE", "BusGenTimeC1PE", "BusGenTimeC2PE", "BusGenTimeC3PE", "BusGenTimeC4PE", "TimeIndC1PE", "TimeIndC2PE", "TimeIndC3PE", "TimeIndC4PE", "TimeIndC5PE", "TimeIndC6PE", "TimeIndC7PE", "TimeIndC8PE", "TimeIndC9PE", "TimeIndC10PE", "TimeIndC11PE", "TimeIndC12PE", "TimeIndC13PE", "TimeIndC14PE", "TimeIndC15PE", "TimeIndC16PE", "TimeHotCPE"
};

char *MktNames[] =
{
  "BusNatE", "BusMidE", "BusLocE", "BusReE", "BusProE", "BusGenE", "PubE", "IndE", "HotE",
};
/// <summary> This function accumates the balance for different account. </summary>
/// <param name="*pD"> is the deposit liability structure.</param>
/// <param name="Prod"> is the account type. </param>
/// <param name="Bal"> is the balance.  </param>
void SumAssD(DepLiaT *pD, long Prod, float Bal)
{
  pD->Sum += Bal;
  switch(Prod)
  {
    case  - 1: pD->BOA_Dep.SavCerts += Bal;
    break;
    case IndNOWE:
      pD->BOA_Dep.Now += Bal;
      break;
    case IndMonE:
      pD->BOA_Dep.MonMkt += Bal;
      break;
    case IndSaveE:
    case BusSaveE:
      pD->BOA_Dep.Sav += Bal;
      break;
    case DemIndE:
      pD->Demand.Indiv += Bal;
      break;
    case DemBusE:
      pD->Demand.Bus += Bal;
      break;
    case DemPubE:
      pD->Demand.Pub += Bal;
  }
}

//This function shall move out of Deposits and move to either Report or FileIO modules.
/// <summary> This function does the end of quarter accounting for Deposits.</summary>
/// <remarks> It writes deposites to D_.txt file and is called by Deposits(). </remarks> 
void RptEOQ()
{
	extern int MktProd_2_B401[];
	extern long NoDumps;

  LB
  {
    int Lns = 1;
    AcT *Ac = BOA[Bank] + YrsQtr;
    IncomeOutGoT *Incm = &Ac->IncomeOutGo;
    InterestT *Int = &Incm->Interest;
    BankInfoT *b = BankInfo[Bank] - 1;
    EconInfoT *e = EconInfo - 1;
		DeposDecT *p = DeposDec + Bank; //For the change required in Item 7
    DepTypeInfoT *d = DepTypeInfo[Bank];
    DepLiaT *pD = &Ac->Bal.Liabilities.Deposits;
    B460T *B460 = &DepOut[Bank].B460;
    B465aT *B465a = &DepOut[Bank].B465a;
    B465bT *B465b = &DepOut[Bank].B465b;
    if(Out = (FILE*) ! NoDumps)
    {
      {
        char F[4] =
        {
          'D', 49, 0, 0
        };
        F[1] += (char)Bank;
        ODF(F, "TXT", "w");
      }
      Out = fp;
      fprintf(fp, "\n%s\n\nCommunity %c\nSimQtr %s\n\n", TimeStamp, FromCom ? FromCom : Community,  *FQ);
      SetBankNames(1);
      fprintf(fp, "\nBank %d (One-Based) \"%s\"\n\n", Bank + 1, BankName);
    }
    memset(&pD->Demand, 0, sizeof(DemandT));
    pD->BOA_Dep.Now = pD->BOA_Dep.Sav = pD->BOA_Dep.MonMkt = pD->BOA_Dep.SavCerts = 0;
    BOA[Bank][YrsQtr].CoreDep = 0;
    pD->Sum = pD->BOA_Dep.Jumbo + pD->BOA_Dep.PubTime;
    {
      Loop(BusGenSaveC4PE + 1)
      {
        int ClsProdI = J;
        ++b;
        ++e;
        {
          DepTypeInfoT *D = d + e->Prod;
          float R = er(1., (*b->Bal = ER(0.,  *b->Bal)) / FloorHair(b->BegBal));
          //  EoQ and BoQ weightings
          float eW = .5 + .5 *(1-R);
					float bW = 1-eW;
					float I = (bW *b->BegBal + eW **b->Bal) *b->BankRate *.0025;
					// Cnt then is the average number of accounts we have had each month in that deposit and class
					// float Cnt =  ! e->AvgSize ? 0 :  *b->Bal *Mill / e->AvgSize;
					
					// Final project item 7. 
					// We will calculate Fee2 (NSF charge) for DemIndE and IndNOWE differently.  
					// DemIndE is the individual demand deposits,
          // IndNOWE is the NOW accounts
					//float Cnt = *b->Bal * Mill / FloorHair(e->AvgSize);
          // float Cnt2 =  (e->Prod != DemIndE && e->Prod != IndNOWE) ? e->MoCnt2: e->MoCnt2 *15 / (10+D->OthFee);

          // float AvgCnt = (bW *b->BegCnt + eW * Cnt) *.000003;
					float AvgCnt = (bW *b->BegCnt + eW * Cnt);
 
					// float Fee1 = e->MinBal >= D->MinBal && ( e->Prod == IndNOWE || e->Prod == IndMonE ) ? 0 : AvgCnt * ( D->MnFee + e->MoCnt1 * D->ItemFee );
          // float Fee2 = AvgCnt * Cnt2 * D->OthFee;

          // Fee1 is service charges.       
          // Waive Fee1 for NOW accounts when a class  ( e-> )
          // has a high enough balance for a bank  ( D-> ).
          // Fee2 is either NSFs for DemIndE and IndNowE or Deposit_Fees for the rest accounts.       
 
					float Fee1, Fee2, Fees;

					if(e->Prod == DemIndE)
					{
						Fee1 = AvgCnt * (D->MnFee +(e->MoCnt1 * D->ItemFee)) * 3.0;  
						Fee2 = AvgCnt * e->MoCnt2 * p->Demand.Fee2.Indiv * 3.0;
					}
					else if(e->Prod == IndNOWE)
					{
						Fee1 = AvgCnt * (D->MnFee +(e->MoCnt1 * D->ItemFee)) * 3.0;
						Fee2 =  AvgCnt  *e->MoCnt2 * p->InterestBearing.NowNsfFee	* 3.0;
					}
					else
					{
						Fee1 = AvgCnt * (D->MnFee + e->MoCnt1 * D->ItemFee) * 3.0;
						Fee2 =  AvgCnt *  D->OthFee * e->MoCnt2  * 3.0;
					}

					Fee1 = Fee1 * 0.000001;
					Fee2 = Fee2 * 0.000001;

          int Dem = e->Prod <= DemPubE, B465R = B465_Row(e);
          if( ! Dem || e->PayInt)
          {
            Rpt_401(IntC01, I, ClsProdI);
            B460->Int[B460_Row(e)] += I;
            if(B465R >= 0)
            {
              B465a->Int[B465R] += I;
              B465b->Int[B465R] += I;
            }
          }
          else
          {

            I = er(Fee1, I);
            Rpt_401(CredC01, I, ClsProdI);
            B460->Cred[B460_Row(e)] += I;
            if(B465R >= 0)
            {
              B465a->Cred[B465R] += I;
              B465b->Cred[B465R] += I;
              I = 0;
            }
          }
          BOA[Bank][YrsQtr].IncomeOutGo.Fees.DepServiceChrgs += Fees = Fee1 + Fee2;
          Rpt_401(SerC01, Fees, ClsProdI);
          B460->Fees[B460_Row(e)] += Fees;
          if(B465R >= 0)
          {

            B465a->Fees[B465R] += Fees;
            B465b->Fees[B465R] += Fees;
          }

          {
            float End =  *b->Bal = ER(0.,  *b->Bal + I - Fees), Beg = b->BegBal;
            SumAssD(pD, e->Prod, End);
            Rpt_401(BegC01, Beg, ClsProdI);
            Rpt_401(EndC01, End, ClsProdI);
            B460->Beg[B460_Row(e)] += Beg;
            B460->End[B460_Row(e)] += End;
            if(B465R >= 0)
            {
              B465a->Beg[B465R] += Beg;
              B465b->Beg[B465R] += Beg;
            }
            if(B465R >= 0)
            {
              B465a->End[B465R] += End;
              B465b->End[B465R] += End;
            }
          }
          // Sets the final number of accounts.
          *b->Cnt =  *b->Bal *Mill / FloorHair(e->AvgSize);
          RV_Factors[Bank][EoQ_Cnt][ToReqSals460(e)] +=  *b->Cnt;
          Rpt_401(NumAccsC01,  *b->Cnt, ClsProdI); // Reports the total number of accounts.

          // MktProd_2_B401 maps the Markets to how they are reported:
          //   BusDD_N, BusDD_M, BusDD_L, BusDD_O, InDD, PubDDR, NowR, BusSavR, InSav, MMR, TimeR, B401_C
          if(MktProd_2_B401[e->MktProd] <= NowR)
          {
            int NOW_Ind_NSF = e->MktProd == DemIndMPE || e->MktProd == IndNOWMPE;
            //  Reports the total monthy items.
            Rpt_401(NumItemsC01,  *b->Cnt *(NOW_Ind_NSF ? e->MoCnt1: e->MoCnt1 + e->MoCnt2), ClsProdI);
            if(NOW_Ind_NSF)
            // Reports the total NSFs.
              DepOut[Bank]._401[NumNSFsC01][e->MktProd == DemIndMPE ? IndDemR01 : NowR01] += 3 **b->Cnt *e->MoCnt2;
          }

          if(e->Prod <= IndNOWE)
            RV_Factors[Bank][TransCnt][ToReqSals460(e)] +=  *b->Cnt *e->Checks;
          if(Out)
          {
            if( ! (Lns %= 82) ||  ! ClsProdI || e->Prod != e[ - 1].Prod)
            {
              if(Lns == 81)
              {
                fprintf(Out, "\n");
                Lns++;
              }
              Lns++;
              fprintf(Out, "               Dl, BgBal, BgCnt, Attr, Shr,  FAtr, Rate, AjRate, EndBal, EndCnt, Loy, EffRate\n");
            }
            Lns++;
            fprintf(Out, "%14s: ", ClsProdNames[ClsProdI]);
            NumLen = 1;
            DotLen = 0;
            fprintf(Out, "%s ", FA(b->Deleted));
            NumLen = 5;
            DotLen = 1;
            fprintf(Out, "%s ", FA(b->BegBal));
            NumLen = 7;
            DotLen = 0;
            fprintf(Out, "%s ", FA(b->BegCnt));
            NumLen = 5;
            DotLen = 1;
            fprintf(Out, "%s ", FA(b->Attrib));
            NumLen = 5;
            DotLen = 2;
            fprintf(Out, "%s ", FA(b->Shr));
            NumLen = 4;
            DotLen = 1;
            fprintf(Out, "%s ", FA(b->FinalAttrib));
            NumLen = 6;
            DotLen = 1;
            fprintf(Out, "%s ", FA(b->BankRate));
            NumLen = 6;
            DotLen = 1;
            fprintf(Out, "%s ", FA(b->AdjRate));
            NumLen = 7;
            DotLen = 1;
            fprintf(Out, "%s ", FA(*b->Bal));
            NumLen = 7;
            DotLen = 0;
            fprintf(Out, "%s ", FA(*b->Cnt));
            NumLen = 5;
            DotLen = 1;
            fprintf(Out, "%s ", FA(b->Loy));
            NumLen = 7;
            DotLen = 1;
            fprintf(Out, "%s\n", FA(b->RealRate));
          }
        }
      }
    }
    if(Out)
    {
      BankInfoT *b = BankInfo[Bank] + BusNatTimeC1PE - 1;
      long ClsProdI = BusNatTimeC1PE - 1;
      while(++ClsProdI <= TimeIndC16PE)
      {
        ++b;
        if( ! (Lns %= 82) || ClsProdI == BusNatTimeC1PE)
        {
          if(Lns == 81)
          {
            fprintf(Out, "\n");
            Lns++;
          }
          Lns++;
          fprintf(Out, "                Attr, Shr, FinAtt, AdjRate, Loy,  Mat, Rate, Fixed\n");
        }
        Lns++;
        fprintf(Out, "%14s: ", ClsProdNames[ClsProdI]);
        NumLen = 4;
        DotLen = 1;
        fprintf(Out, "%s ", FA(b->Attrib));
        NumLen = 4;
        DotLen = 2;
        fprintf(Out, "%s ", FA(b->Shr));
        NumLen = 6;
        DotLen = 1;
        fprintf(Out, "%s ", FA(b->FinalAttrib));
        NumLen = 8;
        DotLen = 1;
        fprintf(Out, "%s ", FA(b->AdjRate));
        NumLen = 6;
        DotLen = 1;
        fprintf(Out, "%s ", FA(b->Loy));
        if(b->TimeDec)
        {
          NumLen = 3;
          DotLen = 0;
          fprintf(Out, "%s ", FA(b->TimeDec->Mat));
          NumLen = 6;
          DotLen = 1;
          fprintf(Out, "%s ", FA(b->TimeDec->Rate));
          NumLen = 4;
          DotLen = 0;
          fprintf(Out, "%s ", FA(b->TimeDec->Fixed == F));
        }
        fprintf(Out, "\n");
      }
    }
    {
      TimeDepT *t = TimeDep[Bank] - 1;
      long Up = 0;
      //Loop(MaxTimeDep)
	  for(int J = 0; J < MaxTimeDep; J++)
      {
        ++t;
        if( ! t->Mat)
          continue;
        //          t->MktRate = .9 * YldC( Mat, EoQ ); // jcr
        if(Out)
        {
          if( ! (Lns %= 82) ||  ! Up)
          {
            Up = 1;
            if(Lns == 81)
            {
              fprintf(Out, "\n");
              Lns++;
            }
            Lns++;
            fprintf(Out, "   Id,      Mkt,   Bal,  Mat, Rate,  Cnt, F,   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16\n");
          }
          Lns++;
          NumLen = 6;
          DotLen = 0;
          fprintf(Out, "%s  ", FA(t->Id));
          fprintf(Out, "%8s ", MktNames[(long)t->Market]);
          NumLen = 6;
          DotLen = 3;
          fprintf(Out, "%s ", FA(t->Bal));
          NumLen = 3;
          DotLen = 0;
          fprintf(Out, "%s ", FA(t->Mat));
          NumLen = 6;
          DotLen = 1;
          fprintf(Out, "%s ", FA(t->RateOrSpread));
          NumLen = 5;
          DotLen = 0;
          fprintf(Out, "%s ", FA(t->Cnt));
          NumLen = 2;
          DotLen = 0;
          fprintf(Out, "%s  ", FA(t->IsFixed));
          if(t->Market != HotE)
          {
            uchar *p = t->Por - 1;
            //Loop(MaxTimeCls)
			for(int J = 0; J < MaxTimeCls; J++)
            {
              p++;
              fprintf(Out, " %3d",  *p);
            }
          }
          fprintf(Out, "\n");
        }
        {
          float Bal = t->Bal;
          SumAssD(pD,  - 1, Bal);
          Time401(EndC01, t, Bal);
          // Report the total number of accounts on B401.
          Time401(NumAccsC01, t, t->Cnt);
          Rpt421a(EndC21a, t, Bal);
          Time460(B460->End, t, Bal);
          Time465(B465a->End, t, Bal);
          Time465(B465b->End, t, Bal);
        }
      }
    }
    Int->CoreDep = DepOut[Bank]._401[IntC01][TotCorR01];
    CloseOut();
  }
}
/// <summary> This function writes the rate, maturity and F/V to DepOut[Bank] for 421B.  </summary>
/// <remarks> It is called by Deposits().</remarks>
void RptTimeOffers()
{
  TimeDecT *d = DeposDec[Bank].Time.TimeDec - 1;
  float(*R)[LastR21b] = DepOut[Bank]._421b;
  //Loop(TotR21b)
  for(int J = 0; J < TotR21b; J++)
  {
    ++d;
    if(d->Rate <= 0)
    {
      R[0][J] = UnSet;
      continue;
    }
    R[RateC21b][J] = d->Rate;
    R[MatC21b][J] = d->Mat;
    R[FixC21b][J] = d->Fixed;
  }
}

/// <summary> This is the main routine for all deposit activities called from RunForward(). </summary>
void Deposits()
{
  Zero(EconInfo);
  Zero(BankInfo);
  Zero(DepTypeInfo);
  Zero(FinalAttribByCls);
  SetRel();
  Zero(LnsByBusMk);
  Zero(SumLnsByBusMk);
  LB DepStartUp();
  ClGrowth();
  LB UpdateTime();
  LB NonTimeWithAttrib();
  LB NonTimeEarlyWith();
  LB ClassShareDeterminants();
  SetClsPrices();
  LB Zero(Dep[Bank].BnkShr);
  ClsShare();
  LB NewProd();
  LB NewHot();
  RptEOQ();
  LB RptTimeOffers();
}
