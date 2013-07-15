#include "BMSim.h"

extern FundsGloT FundsGlo[MaxB]; 
extern TreasDecT TreasDec[MaxB];
extern ECNT EconBOQ, EconEOQ;
extern SecKT SecK; 

extern BOAT BOA[MaxB]; 
extern int YrsQtr; 
extern int Bank;

extern float B501a[MaxB][B501aR][B501aC];
extern float B501b[MaxB][B501bR][B501bC];

// --------  Treasury (void) ------
/// <summary> This function process maturing CD, new CDs and sets interest rate and amount for CDs.  </summary>
/// <remarks> These rates and amount saved in B501a[Bank] will be shown in Reports B501 and B502 </remarks>
void CDsMatNewInt(void)
{
  AcT *Ac = BOA[Bank] + YrsQtr;
  IncomeOutGoT *Incm = &Ac->IncomeOutGo;
  BalT *Bal = &Ac->Bal;
  LiaT *Lia = &Bal->Liabilities;
  FundsGloT *Fns = FundsGlo + Bank;
  TreasDecT *Trs = TreasDec + Bank;
  float(*R)[B501aC] = B501a[Bank]; //Jeff2: bad design here.  B501 should not be here.
  memset(R, 0, sizeof(*B501a));
  {
    TrsTmT *p = Fns->CDs - 1; //Where Fns-CD is mapped to decision form? 
    //Loop(MaxCD)
	for(int J = 0; J < MaxCD; J++)
    { //Process the mature CDs
      p++;
      if( ! p->Amnt || p->Mat-- > 0)  //If Amnt exists and Mature is 0 or 1. 
        continue;
      Fns->Day1Out.CDsMat += p->Amnt; //any matured CD amount will be added to Day 1 total. 
      R[0][2] -= p->Amnt; // Not Added to Col Total
      memset(p, 0, sizeof(TrsTmT)); //It takes out the mature CD from FundsGlo. 
    }
  }
  {
    float TotByMat[MaxCDA];
    memset(TotByMat, 0, sizeof(TotByMat));
    {//Process new CDs (Treasure Decision entered in the form) 
      //Loop(MaxTreasDec)
	  for(int J = 0; J < MaxTreasDec; J++)
      {
        long Mat = (long)Trs->Funds[J].Mat;
				long Type = (long)Trs->Funds[J].Type;
        float Amnt = Trs->Funds[J].Amnt;
        if(Type != CDS || Amnt <= 0 || Mat <= 0)  
          continue;
				//TotByMat is classified to 8 groups.  
        TotByMat[--Mat % MaxCDA] += Amnt;  //only if it is CD and Amount/Mature is not equal to 0. 
      }
    }
    {
      RateAmntT *pAvl = Fns->CDsAvail - 1;
      //Loop(MaxCDA)
	  for(int J = 0; J < MaxCDA; J++)
      {//Set fields in B501a[11][10] from GloFunds. But DecAmnt is from TreasDec through TotByMat.
        long Mat = J, r = Mat + 2;
        float DecAmnt = TotByMat[Mat];
				float DecRate = (++pAvl)->Rate;
				//Set the fields for Rate, Amount of Available Funds in B501. 
        R[r][6] = DecRate;
        R[r][7] = pAvl->Amnt;
        R[10][7] += Round(pAvl->Amnt);
        if( ! DecAmnt)
          R[r][8] = R[r][9] = UnSet;
        else
        {
          R[r][8] = DecAmnt;
          R[10][8] += DecAmnt;
        }
        if( ! DecAmnt)
          continue;
        {
          TrsTmT *p = Fns->CDs - 1;
          //Loop(MaxCD)
		  for(int J = 0; J < MaxCD; J++)
          {
            p++;
            if(p->Amnt)
              continue;
            {
              float Av = Round(pAvl->Amnt);
							float Amnt = min(Av *1.5, DecAmnt);
							float Rt = Amnt <= Av ? DecRate : DecRate *(.9 + .1 * Amnt / Av);
              p->Amnt = Amnt;
              R[r-1][2] = Amnt;
              R[10][2] += Amnt;
              R[r][9] = p->Rate = Rt;
              p->Mat = Mat;
              Lia->Deposits.BOA_Dep.Jumbo += Amnt;
              Fns->Day1In.CDs += Amnt;
              break;
            }
          }
        }
      }
    }
  }
  Lia->Deposits.BOA_Dep.Jumbo = Fns->ZMatCDs = 0;
  {
    TrsTmT *p = Fns->CDs - 1;
    //Loop(MaxCD)
	for(int J = 0; J < MaxCD; J++)
    {
      p++;
      if( ! p->Amnt)
        continue;
      {
        float A = p->Amnt, AR = A * p->Rate, I = AR * .0025;
        long M = (long)p->Mat + 1;
        if(M == 1)
          Fns->ZMatCDs += A;
        Lia->Deposits.BOA_Dep.Jumbo += A;
        Incm->Interest.Expenses.BOA_Dep.Jumbo += I;
        R[M][0] += A;
        R[10][0] += A;
        R[M][3] += I;
        R[10][3] += I;
        R[M][1] += AR;
        R[10][1] += AR; // Divides ARs By A
      }
    }
  }
}
/// <summary> This function calculates bank liabilties and funds based on the stock repurchased(buy back) and issued. </summary>
/// <remarks> It is called by Treasury(). </remarks>
void StocksRetiredAndIssued(void)
{
  AcT *Ac = BOA[Bank] + YrsQtr;
  LiaT *Lia = &Ac->Bal.Liabilities;
  FundsGloT *Fns = FundsGlo + Bank;
  TreasDecT *Trs = TreasDec + Bank;
  float SharesToRetire = Trs->RePur / 1000;
  if(SharesToRetire)
  // SharesToRetire Stock
  {
    float CapitalStock = Lia->Stock.Common;
		float TotalShares = 1000 * CapitalStock / PAR;  // In Thousands
    float RetirePrice = Ac->StkPrice *STOCKREPURPREM;
		float RetireRatio = min(SharesToRetire / TotalShares, STOCKREPURLIM);
		float ParAmnt = CapitalStock * RetireRatio;
    Lia->Stock.Common -= ParAmnt;
    SharesToRetire = Fns->SharesRP = RetireRatio * TotalShares;
    {
      float SurAmnt = (RetirePrice - PAR) *.001 * SharesToRetire;
			float Sur = Lia->Stock.Surplus;
			float RE = Lia->RetainedEarnings;
      Lia->Stock.Surplus -= SurAmnt *(Sur / (RE + Sur));
      Lia->RetainedEarnings -= SurAmnt *(RE / (RE + Sur));
      {
        float Amnt = SurAmnt + ParAmnt;
        Lia->_Equity -= Amnt;
        Fns->Day1Out.Stock = Amnt;
        Fns->StockAmntRP = Amnt;
      }
    }
  }
  { //Issue stock from Treasure Management Decision Form. The results will be used for B550
    float p = Fns->IssuePlanAmnt;
    if(p && Trs->Cam && Trs->C101 == SellStock)
    {
      float d = CILIMIT * p; //Capital issue 
			float Amnt = Bound(Trs->Cam, p - d, p + d);
			float MtoB = Ac->StkPrice / Ac->_Book; //MtoB  Market Price/Book Price
			float a = .05 + .05 * Amnt + (MtoB >= 1 ? 0 : .02 *(1-MtoB) *Amnt);
			float Shares = (Amnt + a) / Fns->IssuePricePerShare;
			float CS = Shares * PAR;
      Fns->IssuePrice = Amnt / Shares;
      Lia->Stock.Common += CS;
      Lia->Stock.Surplus += Amnt - CS;
      Lia->_Equity += Amnt;
      Fns->Day1In.Stock = Amnt;
    }
  }
}
/// <summary> This function calculates bank data for subordinated long term debt in Treasure Management.  </summary> 
/// <remarks> The terms on the debt issue will be shown on Report B550.  This function sets 
/// BOA[Bank][YrsQtr].Bal.Liabilities.SubLTD, BOA[Bank][YrsQtr].IncomeOutGo.Interest.Expenses.SubLTD.  It is called by Treasury(). </remarks> 
void SubLTD_IssuedMatNewInt(void)
{
  FundsGloT *Fns = FundsGlo + Bank;
  {
    SubLTD_IssuedT *p = Fns->SubLTD - 1;
    //Loop(MaxLT)
	for(int J = 0; J < MaxLT; J++)
    { //Maturing subordinated long term notes
      p++;
      if( ! p->Amnt || p->Mat-- > 0)
        continue;
      Fns->Day1Out.SubLTD_IssuedMat += p->Amnt;
      memset(p, 0, sizeof(SubLTD_IssuedT));
    }
  }
  { //New subordinated long term notes
    SubLTD_IssuedT *p = Fns->SubLTD - 1;
    float Avail = Fns->IssuePlanAmnt;
		float d = CILIMIT * Avail;
    if(Avail && TreasDec[Bank].Cam && TreasDec[Bank].C101 == SellSubLTD)
    {
      //Loop(MaxLT)
	  for(int J = 0; J < MaxLT; J++)
      {
        if((++p)->Amnt)
          continue;
        p->Amnt = Fns->Day1In.SubLTD = Bound(TreasDec[Bank].Cam, Avail - d, Avail + d);
        {
          SubLTD_IssuedT *Avail = Fns->SubLTD_IssuedAvail + 2;
          p->Rate = Avail->Rate;
          p->Mat = --Avail->Mat;
        }
        break;
      }
    }
  }
  {
    float *p = &BOA[Bank][YrsQtr].Bal.Liabilities.SubLTD;
     *p = 0;
    {
      //Loop(MaxLT)
		for(int J = 0; J < MaxLT; J++)
			*p += Fns->SubLTD[J].Amnt;
    }
  }
  {
    //Loop(MaxLT)
	for(int J = 0; J < MaxLT; J++)
    {//calculate SubLTD interest. 
      if( ! Fns->SubLTD[J].Amnt)
        continue;
      BOA[Bank][YrsQtr].IncomeOutGo.Interest.Expenses.SubLTD += Fns->SubLTD[J].Amnt *Fns->SubLTD[J].Rate *.0025;
    }
  }
}
/// <summary> This function calculates the yield for a public time deposit in the Treasure Management.  </summary>
/// <param name="Mat"> is the PTD maturity, which can be 1 to 4 quarters. </param>
/// <param name="Q"> is 0 if end of quarter, or 1 if beginning of quarter. </param>
/// <returns> PT yield. </returns>
float PubTimYield(long Mat, long Q)
{
  return.5 + 1 * .1 + (Q == EoQ ? EconEOQ.Econ.CDMI : EconBOQ.Econ.CDMI) *YldC(Mat + 1, Q);
}
/// <summary> This function operates on the maturing, new Public Time Deposit and calculates interest rate, amount and yield for Public Time Depsosit.  </summary>
/// <remarks> It set Public Time Deposit section for B501. It is called by Treasury(). </remarks>
void PTsMatNewInt(void)
{
  AcT *Ac = BOA[Bank] + YrsQtr;
  IncomeOutGoT *Incm = &Ac->IncomeOutGo;
  BalT *Bal = &Ac->Bal;
  LiaT *Lia = &Bal->Liabilities;
  FundsGloT *Fns = FundsGlo + Bank;
  TreasDecT *Trs = TreasDec + Bank;
  float(*R)[B501bC] = B501b[Bank], PubTimeTotal = 0;
  Ac->Unpledged -= Lia->Deposits.Demand.Pub;  // Set the same amount of public deposit aside as pledge money.  
  memset(R, 0, sizeof(*B501b));
  {
    TrsTmT *p = Fns->PTs - 1;
    //Loop(MaxPT)
	for(int J = 0; J < MaxPT; J++)
    { //Mature PT
      p++;
      if( ! p->Amnt)
        continue;
      if(p->Mat-- > 0)
      {
        Ac->Unpledged -= p->Amnt;
        PubTimeTotal += p->Amnt;
        continue;
      }
      R[0][2] -= p->Amnt; // Not Part of Col Tot
      Fns->Day1Out.PTsMat += p->Amnt;
      memset(p, 0, sizeof(TrsTmT));
    }
  }
  {
    float TotByMat[MaxPTA];
    Zero(TotByMat);
    {
      //Loop(MaxTreasDec)
	  for(int J = 0; J < MaxTreasDec; J++)
      { //New PT
        float Amnt = Trs->Funds[J].Amnt;
        long Mat = (long)Trs->Funds[J].Mat, Type = (long)TreasDec[Bank].Funds[J].Type;
        if(Type != PTD || Amnt <= 0 || Mat <= 0)
          continue;
        TotByMat[--Mat % MaxPTA] += Amnt;
      }
    }
    {
      float CapAvail = (Lia->SubLTD + Lia->_Equity) *1-PubTimeTotal;
      //Loop(MaxPTA)
	  for(int J = 0; J < MaxPTA; J++)
      {
        long Mat = J, r = Mat + 2;
        float DecAmnt = TotByMat[Mat], DecRate = PubTimYield(Mat, BoQ);
        R[r][4] = PubTimYield(Mat, EoQ);
        R[r][5] = DecRate;
        if( ! DecAmnt)
          R[r][6] = R[r][7] = UnSet;
        else
        {
          R[r][6] = DecAmnt;
          R[6][6] += DecAmnt;
        }
        if( ! DecAmnt ||  ! CapAvail)
          continue;
        {
          TrsTmT *p = Fns->PTs - 1;
          //Loop(MaxPT)
		  for(int J = 0; J < MaxPT; J++)
          {
            p++;
            if(p->Amnt)
              continue;
            {
              float Amnt = min(min(CapAvail, DecAmnt), Ac->Unpledged);
              if(Amnt <= 0)
                continue;
              CapAvail -= Amnt;
              Ac->Unpledged -= Amnt;
              p->Amnt = Amnt;
              R[r - 1][2] = Amnt;
              R[6][2] += Amnt;
              R[r][7] = p->Rate = DecRate;
              p->Mat = Mat;
              Fns->Day1In.PTs += Amnt;
              break;
            }
          }
        }
      }
    }
  }
  Lia->Deposits.BOA_Dep.PubTime = 0;
  {
    TrsTmT *p = Fns->PTs - 1;
    //Loop(MaxPT)
	for(int J = 0; J < MaxPT; J++)
    {
      if( ! (++p)->Amnt)
        continue;
      {
        float A = p->Amnt, AR = A * p->Rate, I = AR * .0025;
        long M = (long)p->Mat + 1;
        Lia->Deposits.BOA_Dep.PubTime += A;
        Incm->Interest.Expenses.BOA_Dep.PubTime += I;
        R[M][0] += A;
        R[6][0] += A;
        R[M][3] += I;
        R[6][3] += I;
        R[M][1] += AR;
        R[6][1] += AR; // Divides ARs By A
      }
    }
  }
}
/// <summary> This function calcuates amount for Federal Fund Repurchase and Federal Reserve Bank Borrowing called by Treasury(). </summary>
void FFsReposAndFRBs(void)
{
  AcT *Ac = BOA[Bank] + YrsQtr;
  BalT *Bal = &Ac->Bal;
  FundsT *LiaF = &Bal->Liabilities.Funds, *ExF = &Ac->IncomeOutGo.Interest.Expenses.Funds;
  AssetsT *Ass = &Bal->Assets;
  FundsGloT *Fns = FundsGlo + Bank;
  FndDecT *Dec = TreasDec[Bank].Funds - 1;
  Fns->Day1In.Funds = Ass->ffSold;
  Fns->Day1Out.Funds = LiaF->Repo + LiaF->FF + LiaF->FRB;
  memset(LiaF, 0, sizeof(FundsT));
  Ass->ffSold = 0;
  if(Fns->FRBClosed) //If the bank violates the rules set FRB, this flag is set to 1. 
    return ;
  {
    float TotFunds = 0;
		float Equ = Bal->Liabilities._Equity *.25;
    if(Equ <= 0)
      return ;
    {
      //Loop(MaxTreasDec)
	  for(int J = 0; J < MaxTreasDec; J++)
      {
        Dec++;
        if(Dec->Type != FRB || Dec->Amnt <= 0)
          continue;
        TotFunds += Dec->Amnt;
      }
    }
    if( ! (TotFunds = min(Equ, TotFunds)))  //Jeff2: could be a bug. George is investigating it.  
      return ;
    ExF->FRB = (EconBOQ.Econ.FRDR + EconEOQ.Econ.FRDR) *.5 * .0025 * TotFunds;  //FRDR is Federal Reserve Discount Rate.  The interest bank needs to pay for Federal.  
    Fns->Day1In.Funds += Fns->FRBs = LiaF->FRB = TotFunds;
  }
}

/// <summary> This function calculates bank data for Housing Finance bank borrowing in Treasure Management.  </summary> 
/// <remarks> This function also tries to set some fields in B502.  It is called by Treasury(). </remarks>
void HFsMatNewInt()
{
	extern float D502[MaxB][B502R][B502C]; //variable defined in B502
	extern float FHLB_Avail_BoQ[MaxB], FHLB_Avail_EoQ[MaxB];
	extern D531T D531[MaxB];

  AcT *Ac = BOA[Bank] + YrsQtr;
  IncomeOutGoT *Incm = &Ac->IncomeOutGo;
  BalT *Bal = &Ac->Bal;
  AssetsT *Ass = &Bal->Assets;
  LiaT *Lia = &Bal->Liabilities;
  FundsGloT *Fns = FundsGlo + Bank;
  TreasDecT *Trs = TreasDec + Bank;
  float(*R)[B502C] = D502[Bank];
  memset(R, 0, sizeof(*D502));
  FundsGlo[Bank].HFAmnt = 0;
  {
    static HFsT Buf[MaxHF];
    HFsT *NextP = Buf - 1;
    long Sz = sizeof(HFsT);
    memset(Buf, 0, sizeof(Buf));
    {//sorting the FundsGlo[Bank].HFs. It loops through the array, finds the least matury, and copy this fund to a buffer. 
      //Loop(MaxHF)
	  for(int J = 0; J < MaxHF; J++)
      {  
        NextP++;
        {
          HFsT *L,  *p = FundsGlo[Bank].HFs - 1;
          float LowestMat = 9876;
          //Loop(MaxHF)
		  for(int J = 0; J < MaxHF; J++)
          {//find the lowest maturity among all HF fund. 
            if( ! (++p)->Amnt)
              continue;
            if(p->Mat < LowestMat)
            {
              LowestMat = p->Mat;
              L = p;
            }
          }
          if(LowestMat == 9876)
            break;
          memmove(NextP, L, Sz);
          memset(L, 0, Sz);
        }
      }
    }
    memmove(FundsGlo[Bank].HFs, Buf, sizeof(Buf));
  }
  {//Maturing HF funds.
    HFsT *p = Fns->HFs - 1;
    //Loop(MaxHF)
    for(int J = 0; J < MaxHF; J++)
	{
      if( ! (++p)->Amnt)
        continue;
      if(p->Mat-- > 0)
      {
        Fns->HFAmnt += p->Amnt;
        continue;
      }
      R[0][p[J].Fixed ? 0 : 2] -= p->Amnt;  //Matured field in B502
      Fns->Day1Out.HFsMat += p->Amnt;
      memset(p, 0, sizeof(HFsT));
    }
  }
  {
    HFsT *p = Fns->HFs;
    //Loop(MaxTreasDec)
	for(int J = 0; J < MaxTreasDec; J++)
    {//New Housing Finance fund (Repayment) from Decision form.
      float Dec = Trs->Funds[J].Amnt;
      if(Trs->Funds[J].Type != HFR || Dec <= 0)
        continue;
      {
        HFsT *Q = p + (long)Trs->Funds[J].Mat - 501;  //501 is the ID-Code entered for HFR.  (Id from N506 in Maturity Col). 
        if( ! Q->Amnt)
          continue;
        Q->Amnt -= (Dec = min(Dec, Q->Amnt));
        Fns->HFAmnt -= Dec;
        R[0][Q->Fixed ? 1 : 3] -= Dec; //Repaid fields in B502. 
        Fns->Day1Out.HFs += Dec;
        if(Q->Fixed && Q->Mat >= 4)
          Incm->Expenses.OtherOpEx += Dec * .005 * Q->Rate;
        if( ! Q->Amnt)
          memset(Q, 0, sizeof(HFsT));
      }
    }
  }
  if(Fns->HFMem)
  {
    //Loop(MaxTreasDec)
	for(int J = 0; J < MaxTreasDec; J++)
    {
      float Amnt = Trs->Funds[J].Amnt;
      long Type = (long)Trs->Funds[J].Type;
      if(Amnt <= 0 || (Type != HFF && Type != HFV))
        continue;
      Amnt = min(Amnt, FHLB_Avail_BoQ[Bank] - Fns->HFAmnt); //Available HF Borrowing for this quarter. (B502)
      if(Amnt <= 0)
        break;
      {
        long IsFixed = Type == HFF;
		long Mat = (long)Trs->Funds[J].Mat;
        //Loop(MaxHF)
		for(int J = 0; J < MaxHF; J++)
        {
          long j = J;
          if(Fns->HFs[j].Amnt)
            continue;
          {//Adding new HF funds to FundsGlo. 
            float A = Fns->HFs[j].Amnt = Amnt;
            long M = Mat, F = IsFixed;
            R[25][0] += A; //Total HFB field. 
            {
              long m =  - 1;
              while(++m < MaxMC && M > SecK.MatClass[m])
                ;
              R[m + (F ? 1 : 13)][0] += A;
              R[F ? 12 : 24][0] += A;
            }
          }
          Fns->HFs[j].Mat = Mat - 1;
          Fns->HFs[j].Fixed = IsFixed;
          Fns->HFs[j].Rate =  ! IsFixed ? 0 : BondYield(Mat, BoQ) + .245 + .005 * Mat;
          Fns->HFAmnt += Amnt;
          Fns->Day1In.HFs += Amnt;
          break;
        }
      }
    }
  }
  memset(D531 + Bank, 0, sizeof(*D531));
  Lia->Funds.ShortTermHF = Lia->LTD_HF = 0;
  {//set data for B531 in Short and Long. set data for B502 in R.
    HFsT *p = Fns->HFs - 1;
    float(*Short)[6] = D531[Bank].FnsHF, (*Long)[2] = D531[Bank].LongHF;
    //Loop(MaxHF)
	for(int J = 0; J < MaxHF; J++)
    {
      p++;
      if( ! p->Amnt)
        continue;
      {
        long M = (long)p->Mat, F = (long)p->Fixed;
        float A = p->Amnt, AR = A *(F ? p->Rate: EconBOQ.Econ.CPR), I = AR * .0025;
        if(M < 4) //short term in B531
        {
          Incm->Interest.Expenses.Funds.ShortTermHF += I;
          Lia->Funds.ShortTermHF += A;
          Short[F][0] += A;
          Short[F][1] += I;
          Short[F][M + 2] += A;
        }
        else  //long term in B531
        {
          Incm->Interest.Expenses.LTD_HF += I;
          Lia->LTD_HF += A;
          Long[F][0] += A;
          Long[F][1] += I;
        }
        R[25][1] += A;
        R[25][3] += AR;
        R[25][2] += I;
        {
          long m =  - 1;
          while(++m < MaxMC && M > SecK.MatClass[m]);
          m += F ? 1 : 13;
          M = F ? 12 : 24;
          R[m][1] += A;
          R[m][3] += AR;
          R[m][2] += I;
          R[M][1] += A;
          R[M][3] += AR;
          R[M][2] += I;
        }
      }
    }
  }
}
/// <summary> This is the main function for Treasure management in Bmsim. </summary>
void Treasury()
{
  CDsMatNewInt();
  StocksRetiredAndIssued();
  PTsMatNewInt();
  FFsReposAndFRBs();
  SubLTD_IssuedMatNewInt();
  HFsMatNewInt();
}

