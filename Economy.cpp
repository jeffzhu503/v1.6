#include "BMSim.h" 

extern ECNT EconBOQ, EconEOQ;
extern EconT EconList[10]; 
extern EnvT Env, LocEnv; 
extern EconQtrT EconQtr8[8]; 
extern PricesT Prices; 

float PrGrowth[25];
long Forcast;

/// <summary> This function accumulates Fcst1Q and Fcst4Q for Econonmy from LowerEconT.PrGrowth[25] in EconQtr(). </summary>
/// <remarks> Fcst1Q = EconEOQ.LowerEcon.Fcst[0], Fcst4Qs = EconEOQ.LowerEcon.Fcst[1]. 
/// In the report B900, Fcst1Q is the first quarter and Fcst4Qs is 4 quarters starting from the first quarter.</remarks>
void FcstCompress()
{

  int MkI =  - 1, DenserMarket[] = //This decides which Fcst the FcstGrowth adds to. 
  {
    0, 0, 0, 0, 0, 1, 1, 1, 2,  - 1, 3, 3, 3, 3, 3, 3, 3, 3,  - 1,  - 1,  - 1,  - 1,  - 1,  - 1,  - 1
  };
  FeP Fcst1Q = (FeP)EconEOQ.LowerEcon.Fcst, Fcst4Qs = (FeP)(EconEOQ.LowerEcon.Fcst + 1);
  FeP FcstGrowth = PrGrowth - 1;
  while(++MkI < 25)
  {
    long k = DenserMarket[MkI];  //
    ++FcstGrowth;
    if(k < 0)
      continue;
    if( ! Forcast)
      Fcst1Q[k] +=  *FcstGrowth;
    Fcst4Qs[k] +=  *FcstGrowth;
  }
}

/// <summary> This function smoothes two preset economies, beginning and ending quarter economies,
/// adjusts GrowRates, MarketSize, and sets Econ and EconEOQ.LowerEcon.PrGrowth. </summary>
/// <param name="qtr"> is the quarter number to simulate. </param>
void EconQtr(long qtr)
{
	extern int NumBanks;
	extern long EconSet_I;

  static float GrowthRate[25];
  float w1 = LocEnv.Weight[0];
	float w2 = LocEnv.Weight[1];
	float tswght = w1 + w2;
  EconT NextEcon;
  int Smooth_Sz = NextEcon.DepositMarkets+10 - &NextEcon.One_Qtr_Treas;  //the diff between memory address. 
  { 
		FeP TE1 = ((FeP) &EconList[(int)LocEnv.Seq[EconSet_I - 1][qtr] - 1]) - 1;   
		FeP	TE2 = ((FeP) &EconList[(int)LocEnv.Seq[EconSet_I - 1][qtr + 1] - 1]) - 1;  
		FeP NextEconP = ((FeP) &NextEcon) - 1;
    // This smoothes together two preset economies.
    //Loop(Smooth_Sz)
	for(int J = 0; J < Smooth_Sz; J++)
    {
      NextEconP++;
      TE1++;
      TE2++;
       *NextEconP = (w1 **TE1 + w2 **TE2) / tswght;
    }
  } 

  float T = 1-tswght;
  if( ! Forcast)
  {
    FeP NextRates = (FeP) &NextEcon;
    Loop(RateSz) *NextRates++ += LocEnv.BaseRate;  //Add base rate (3.0%) to 45 (RateSz) rates in NextEcon, which stops at Dep1QTD.  
    NextEcon.INFR += LocEnv.InflRateBase;
    {
      FeP NextEconP = ((FeP) &NextEcon) - 1;
	  FeP E1Econ = ((FeP) &EconBOQ.Econ) - 1;
	  FeP E2Econ = ((FeP) &EconEOQ.Econ) - 1;
      // This introduces a new index,  LMQI, without creating a new starting point.
      // NextEcon.LMQI is from 2 preset economies, smoothed together.
      float X = NextEcon.LMQI;  //Loan Market Quality Index
	  // 1/27/2009, Build a custom Bmsim for George to test LMQI. The original boundary is 0.5 - 1.5
	  if(X < .01 || X > 100)
      //if(X < .5 || X > 1.5)
        X = 1;
      if( ! EconBOQ.Econ.LMQI)
        EconBOQ.Econ.LMQI = X;
      if( ! EconEOQ.Econ.LMQI)
        EconEOQ.Econ.LMQI = X;
     // This creates the end of quarter economy by smoothing together the new economy (0.7 weight) with the beginning of this quarter economy (0.3 weight). 
      //Loop(Smooth_Sz)
	  for(int J = 0; J < Smooth_Sz; J++)
      {
        NextEconP++;
        E1Econ++;
        E2Econ++;
         *E2Econ =  *NextEconP * tswght +  *E1Econ * T;
      }
    }
    {
	  // Another issue - Min and Max are always 0 and 99.  Why creates so complicate comparison? 
      PriceRangeT *Lim = Prices.Legal - 1;
      FeP E2Rates = ((FeP) &EconEOQ.Econ.LoanCompRates) - 1;  //Comparison rates for loan in the market. 
      // 35 Comparison Rates: 20 Loans 10 Deps, 5 other.
			// Jeff2: bug - the LoanCompRates which E2Rates points to only has 20 rates. 
			// But it loops 35 and modifies 15 rates (10 Deposit Comparision Rate and 5 other products rate) after LoanCompRates in memory.   
			Loop(35)
      {
        Lim++;
        E2Rates++;
         *E2Rates = Bound(*E2Rates, Lim->Min, Lim->Max);
      }
    }
    memmove(GrowthRate, EconEOQ.Econ.LoanMarkets, sizeof(GrowthRate));
  }
  // EconEOQ.Econ is Done at this time
  else
  {
    FeP E2GrowthRate = ((FeP) &NextEcon.LoanMarkets) - 1;
		FeP E1GrowthRate = ((FeP) &EconBOQ.Econ.LoanMarkets) - 1;
		FeP NextGrowthRate = GrowthRate - 1;
    Loop(25)
    {
      E2GrowthRate++;
      E1GrowthRate++;
      NextGrowthRate++;
      *NextGrowthRate =  *E2GrowthRate * tswght +  *E1GrowthRate * T;
    }
  }
  int YearsQtr = qtr &3;
  float OutsideBankPortion = 1-NumBanks / 10.; //This is the portion the system sets for outside banks. NumBanks+OutsideBanks=10
  FeP PrGrowthP = PrGrowth - 1;
	FeP Size = ((FeP) &LocEnv.MkSize) - 1;
	FeP OutSideTurnAways = ((FeP) &LocEnv.Volume) - 1;
  FeP Sea = ((FeP) &LocEnv.SeasonalSel) - 1;
	FeP GrowthRateP = GrowthRate - 1;
  // Adjust PrGrowth and MkSize in LocEnv.  
  Loop(25)
  {
    PrGrowthP++;
    GrowthRateP++;
    Size++;
    OutSideTurnAways++;
    Sea++;
    {
      float K =  *Sea ? LocEnv.SeasonalWt[(long) *Sea - 1][YearsQtr]: (float)1;
      *PrGrowthP = (*GrowthRateP * .25 * K +  *OutSideTurnAways * OutsideBankPortion) **Size;
    }
    *Size +=  *GrowthRateP * .25 * *Size;
  }
  if( ! Forcast)
  {
    memmove(EconEOQ.LowerEcon.PrGrowth, PrGrowth, sizeof(PrGrowth));
    memmove(&Env, &LocEnv, sizeof(Env));
  }
  FcstCompress();  
}

/// <summary> This function creates economic environment in order to run BMSim. </summary>
/// <remarks> The economy can be called by EconMenu() or RunForward(). </remarks>
void Economy(void)
{
	extern int YrsQtr, SimQtr;

  memset(PrGrowth, 0, sizeof(PrGrowth));
  memmove(&EconBOQ, &EconEOQ, sizeof(EconEOQ));
  memset(&EconEOQ, 0, sizeof(EconEOQ));
  memmove(&LocEnv, &Env, sizeof(Env));
  Forcast = 0; //
  EconQtr(SimQtr); // Sets EconEOQ
  // Saves the First Qtr's 4 markets
  {
    float t[4];
    memmove(t, EconEOQ.LowerEcon.Fcst, sizeof(t));
    Forcast = 1; // 3 Loops: Sets LowerEcon.Fcst
    { //Set the PrGrowth for the next 4 quarters
      long Q = SimQtr, MaxQ = Q + 4;
      while(++Q <= MaxQ)
        EconQtr(Q);
    }
    // Restores the First Qtr's 4 markets
    memmove(EconEOQ.LowerEcon.Fcst, t, sizeof(t));
  }
  EconEOQ.LowerEcon.TaxRate = Env.TaxRate;
  EconEOQ.LowerEcon.PLNQUAL = EconBOQ.Econ.LLI;
  EconEOQ.LowerEcon.SalIndexChg = .4 * .8 *(EconEOQ.Econ.INFR *.0025) + (1-.4)*(EconEOQ.LowerEcon.SalIndexChg *.0025);
  EconEOQ.LowerEcon.SalIndex = EconBOQ.LowerEcon.SalIndex;
  EconEOQ.LowerEcon.SalIndex *= 1+EconEOQ.LowerEcon.SalIndexChg;
  EconEOQ.LowerEcon.SalIndexChg *= 400;
  {  
		//At the end of Economy function, it copies the current quarter Econ (EconEOQ.Econ) to EconQtr8[this quarter index]
		//The first four positions of EconQtr8 has 4 quarter econ information for this year. 
		//The second four positions of EconQtr8 has last year four quarters.
		//When the beginning of each year, it moves the first four econ to the second half of EconQtr8.
    long YrSize = sizeof(EconQtrT) *4;
    EconQtrT *Q = EconQtr8 + YrsQtr;  

    EconT *E = &EconEOQ.Econ;
    if( ! YrsQtr)
    {
      memmove(EconQtr8 + 4, EconQtr8, YrSize);
      memset(EconQtr8, 0, YrSize);
    }
    memmove(Q, E, 45 *4);  //Copy EconEOQ.Econ to EconQtr8[]
    Q->INFR = EconEOQ.Econ.INFR;
    Q->GDP = EconEOQ.Econ.GDP;
    Q->TaxRate = EconEOQ.LowerEcon.TaxRate;
    memmove(&Q->SMI, &EconEOQ.Econ.SMI, 5 *4);
    memmove(&Q->Fcst, &EconEOQ.LowerEcon.Fcst, sizeof(Q->Fcst));
  }
}
