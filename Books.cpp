#include "BMSim.h"

extern int SimYr, YrsQtr; 
extern int Bank; 
extern BOAT BOA[MaxB];
extern ECNT EconBOQ, EconEOQ;

float SetF(float B, float *FS, float *R, float *F)
{
  float U = BOA[Bank][YrsQtr].Unpledged;
  *FS =  *R =  *F =  *R = 0;
	// If there are excess funds sell in Fed Funds market
  if(B < 0)
  {
     *FS =  - B;
    return 0;
  }
	// If funds needed and unpledged greater than amount needed
	// use repo's to get required funds
  if(U >= B)
  {
     *R = B;
    return B;
  }
	// If no unpledged securities available use Fed Funds
  if(U <= 0)
  {
     *F = B;
    return 0;
  }
	// If there are some unpledged securities but not enough to cover 
  // needs the repo the rest of the unpledged and use Fed Funds for the balance
  *R = U;
   *F = B - U;
  return U;
}

/// <summary> This function computes interest for Fed Funds Sold, FF Purchased, and Repo's. </summary>
/// <param name="SE"> is stockholder equity </param>
/// <param name="S0"> is the federal fund sold <see cref="FundGloT"></param>
/// <param name="R0"> is the repurchase agreement <see cref="FundGloT"> </param>
/// <param name="F0"> is the federal fund borrowed<see cref="FundGloT">. </param>
/// <param name="S1"> is federal fund sold in <see cref="AssetsT">. </param>
/// <param name="R1"> is the federal fund repurchased in <see cref="LiaT">. </param>
/// <param name="F1"> is the federal fund borrowed in <see cref="LiaT">.</param>
/// <param name="*Si"> is income fed funds sold<see cref="IntInT"></param>
/// <param name="*Ri"> is expense repurchase agreements in <see cref="FundT"></param>
/// <param name="*Fi"> is expense fed fund in <see cref="FundT"></param>
void FundsInt(float SE, float S0, float R0, float F0, float S1, float R1, float F1, float *Si, float *Ri, float *Fi)
{

	// George would like to rewrite the following statement. 
  // float Fr = (EconBOQ.Econ.FFR + EconEOQ.Econ.FFR) *.5 * .0025, R = (F0 + F1) / SE, Br = R <= .5 ? Fr + .0003: (R > 2 ? Fr + .0009 + .0008 *(R - 2): Fr + .0003 + .0004 *(R - .5)), Rr = (EconBOQ.Econ.RPR + EconEOQ.Econ.RPR) *.5 * .0025;
  // Average rate for quarter - Fed Funds
	float Fr = (EconBOQ.Econ.FFR + EconEOQ.Econ.FFR) *.5 * .0025;
	// Average rate for quarter - Repurchase agreements
	float Rr = (EconBOQ.Econ.RPR + EconEOQ.Econ.RPR) *.5 * .0025;

	float R = (F0 + F1) / SE;
  // Funds borrowed rate - Fed  up to limit and then open market
	float Br = R <= .5 ? Fr + .0003: (R > 2 ? Fr + .0009 + .0008 *(R - 2): Fr + .0003 + .0004 *(R - .5));
	// Set  interest income and expense  for funds activities to 0
	*Si =  *Ri =  *Fi = 0;
  if(S0 && S1)
  {
     *Si = Fr * .5 *(S0 + S1);
    return ;
  }
  if(S0 &&  ! S1)
  {
    float D = S0 + R1 + F1;
    *Si = Fr * S0 * .5 * S0 / D;
     *Fi = Br * F1 * .5 * F1 / D;
     *Ri = Rr *(F1 + .5 * R1) *R1 / D;
    return ;
  }
  if( ! S0 && S1)
  {
    float D = S1 + R0 + F0;
    *Si = Fr * S1 * .5 * S1 / D;
     *Fi = Br * F0 * .5 * F0 / D;
     *Ri = Rr *(F0 + R0 * .5) *R0 / D;
    return ;
  }
  if(F0 &&  ! F1)
  {
    float D = R0 + F0 - R1;
    *Fi = Br * F0 * .5 * F0 / D;
     *Ri = Rr *(R0 *F0 + .5 *(R0 + R1)*(R0 - R1)) / D;
    return ;
  }
  if( ! F0 && F1)
  {
    float D = R1 + F1 - R0;
    *Fi = Br * F1 * .5 * F1 / D;
     *Ri = Rr *(F1 *R1 + .5 *(R1 + R0)*(R1 - R0)) / D;
    return ;
  }
  *Ri = Rr * .5 *(R0 + R1);
   *Fi = Br * .5 *(F0 + F1);
  return ;
}

/// <summary> This function sets up the accounting in BMSim. </summary>
/// <remarks> It is always called inside RunForward(). </remarks>
void Books()
{
	extern LoanRptT LoanRpt[MaxB]; 
	extern FundsGloT FundsGlo[MaxB];
	extern SecGloT SecGlo[MaxB]; 
	extern CostKonstT CostKonst; 
	extern SecKT SecK; 
	extern CostsGloT CostsGlo[MaxB]; 

	extern TreasDecT TreasDec[MaxB];
	extern CredDecT CredDec[MaxB];

	extern YTDT YTD[MaxB]; 

	extern int IncmSz; 
	extern long L4[5];

	//  Load structures with current data
  AcT *Ac = BOA[Bank] + YrsQtr;
  IncomeOutGoT *Incm = &Ac->IncomeOutGo;
  BalT *Bal = &Ac->Bal;
  AssetsT *Ass = &Bal->Assets;
  LiaT *Lia = &Bal->Liabilities;
  FundsT *LiaF = &Lia->Funds;
  FundsGloT *Fns = FundsGlo + Bank;
  LoanRptT *LnR = LoanRpt + Bank;
	// Liquid bonds  -  those with maturity from 1 to 4 quarters
  Ac->Liquid_Bonds = 0;
  {
    Loop(4)
    {
      long M = J;
      Loop(SwapFP) 
				Ac->Liquid_Bonds += SecGlo[Bank].Sec[J].MatDistrib[M].Book;
    }
  }
	// Item 3 of the final list for Panther
	// On Day 1 Record payment of dividends payable from prior quarter. We would reduce the  
	// Lia->DividendPayable to 0.0 and show the funds required on Day1 in the Day1Out structure
	/* if(YrsQtr > 0)
	{
		AcT *PriorQtrAc = BOA[Bank] + YrsQtr - 1;
		Fns->Day1Out.Dividend_Paid = PriorQtrAc->Bal.Liabilities.DividendPayable;
		//LastQtrAc->Bal.Liabilities.DividendPayable = 0;
	} */

	Fns->Day1Out.Dividend_Paid = Lia->DividendPayable;
	Lia->DividendPayable = 0;
		
	// Accounting for dividends
	// Payment of dividend and reduction of equity accounts and RetainedEarning accounts
  {
		// The dividend decision for a quarter per share - for example $.75
    TreasDecT *Trs = TreasDec + Bank;
		// Compute the number of shares outstanding based on Par value from  TREASURY FUNDS constants   PAR = 10
    float Shares = Lia->Stock.Common / PAR;
		// The per share dividend rate  the decision derived from 
		float Div = Ac->Div = Trs->Div;
		// The total dividend payment
		float TotDiv = Div * Shares;
		// Subtract total dividend from Equity and Retained Earnings. See the _Equity structure below. 
    Lia->_Equity -= TotDiv;
    Lia->RetainedEarnings -= TotDiv;
		// On 10/11/2007, item 3 from the final list of panther.
		// We should set up a liability equal to what we compute for the  total dividend  TotDiv.
		// On Day1 of the next quarter we need to pay for this liability.  We would reduce the  
		// Lia->DividendPayable to 0.0 and show the funds required on Day1 in the Day1Out structure
		Lia->DividendPayable = TotDiv;
  }
	// Pledging
  Ac->Unpledged -= Lia->Deposits.Demand.Pub - BOA[Bank][L4[1]].Bal.Liabilities.Deposits.Demand.Pub;
  // Accounting for Loan Loss and Loss Provision
	{
    float N = 0;
		float *p = LnR->ChrgOffAmnt;
    Loop(MaxL)
			N +=  *p++;
    {
      float LossProv = CredDec[Bank].LossProv;
      Bal->Assets.LLR += LossProv - N;
      Incm->LoanLossProvision += LossProv;
    }
  }

	// Calculate a balance for other assets
  Ass->Other = CostKonst.OthAss[0] * Ass->NetPremises.GrossPremises + CostKonst.OthAss[1] * Ac->CoreDep;
	// Calculate a balance for Other Liablities
  Lia->Other = CostKonst.OthLia[0] * Incm->Expenses.SalariesBenes + CostKonst.OthLia[1] * Ac->CoreDep;

	// Pay last quarter taxes or collect refund due
  {
    float T = Lia->TaxBill;

    if(T < 0)
      Fns->Day1In.Taxes =  - T;
    else
      Fns->Day1Out.Taxes = T;

    Lia->TaxBill = 0;
  }

	//Calculate total Net Funds position In for Day 1 of Qtr
  {
    FeP p = (FeP) &Fns->Day1In;
    int Max = sizeof(Day1InT) / 4;
    Loop(Max)
			Fns->NetDay1 +=  *p++;
  }
	//Subract Net Funds Out for Day 1 of Qtr
  {
    FeP p = (FeP) &Fns->Day1Out;
    int Max = sizeof(Day1OutT) / 4;
    Loop(Max)Fns->NetDay1 -=  *p++;
  }
  SetF( - Fns->NetDay1, &Fns->FFSold, &Fns->Repos, &Fns->FF);

  Fns->FRBClosed = Fns->FFSold && Fns->FRBs;

	//Accumulate net interest income from income sources 
  Incm->Interest.Net = Incm->Interest.Income.FF + Incm->Interest.Income.Loans.Sum + Incm->Interest.Income.Other + Incm->Interest.NetSwaps;
  {
    FeP p = (FeP) &Incm->Interest.Income.Securities;
    Loop(4)
			Incm->Interest.Net +=  *p++;
  }
	//Subtract interest expense from net interest income
  {
    FeP p = (FeP) &Incm->Interest.Expenses;
    Loop(13)
			Incm->Interest.Net -=  *p++;
  }
	//Calculate net operating earnings
  Incm->OpEarnings = Incm->Interest.Net + (Incm->Fees.LoanFees + Incm->Fees.DepServiceChrgs + Incm->Fees.CommLCFees + Incm->Fees.StandbyLCFees + Incm->Fees.Other) - Incm->LoanLossProvision - (Incm->Expenses.SalariesBenes + Incm->Expenses.Premises + Incm->Expenses.Media + Incm->Expenses.OtherOpEx);
  //Calculate net income
	Incm->Nets.Income = Incm->OpEarnings + Incm->Nets.AssetSales - Incm->Nets.IncomeTaxes;

  { 
		//Calculate Required Reserves and set Cash = Req Rsv
    float ReqRsv = 0;
    FeP p = (FeP) &Lia->Deposits;
		FeP W = CostKonst.ReqRsv;
    int Max = sizeof(CostKonst.ReqRsv) / 4;
    Loop(Max)
			ReqRsv +=  *p++ * W[J];
    Ass->Cash = ReqRsv;
  }

	// We do not have a double entry accounting system.  At this point we 
	// need to determine the amount required to make assets = liabilities
  // and balance the books.
  Ass->Balancer = 0;
	Lia->Balancer = 0;
  {//Calculate the asset and liability
    float A = 0, L = 0;
    A += Ass->Cash;
    A += Ass->ffSold;
    A += Ass->Securities.Treasuries;
    A += Ass->Securities.Munis;
    A += Ass->Securities.Mort;
    A += Ass->Securities.Other;
    A += Ass->Loans.Sum - Ass->LLR;
    A += Ass->NetPremises.GrossPremises - Ass->NetPremises.AccumDepre;
    A += Ass->OtherRE + Ass->Other + Ass->Balancer;
    Ass->Total = A;
    L += Lia->Deposits.Sum;
    L += LiaF->FRB + LiaF->Repo + LiaF->FF + LiaF->ShortTermHF;
		// On 10/11/2007, change item 3 from the final list of panther
		// L += Lia->TaxBill + Lia->Other + Lia->Open + Lia->Balancer;
		L += Lia->TaxBill + Lia->Other + Lia->Balancer;
    L += Lia->LTD_HF;
    L += Lia->SubLTD;
    L += Lia->_Equity;
    L += Incm->Nets.AssetSales;

    {
      InterestT *I = &Incm->Interest;
      FundsT *F = &I->Expenses.Funds;

      long i = 0;

      float U;  
			float SE = Lia->_Equity;
			float Balncr = A - L - Incm->OpEarnings;
			float FundsIntr = 0;
			//GP: Not sure about next 8 lines
      while(1)
			{ //it runs twice.
        U = SetF(Balncr - FundsIntr, &Ass->ffSold, &LiaF->Repo, &LiaF->FF);
        if(++i == 3) break;
        FundsInt(SE, Fns->FFSold, Fns->Repos, Fns->FF, Ass->ffSold, LiaF->Repo, LiaF->FF,  &I->Income.FF, &F->Repo, &F->FF);
        FundsIntr = I->Income.FF - F->Repo - F->FF;
      }

      Ac->Unpledged -= U;

			// GeorgeP: This is a patch from some time in the past. Should delete this from the code
      //if ( Community == 'B' && Bank == 5 ) { F->Repo = 1.528;    F->FF = .011; }
			// Adjust net income for Fed Funds interest income and expense
      I->Net += FundsIntr;

      Incm->OpEarnings += FundsIntr;

      Incm->Nets.Income += FundsIntr;

      Ass->Total += Ass->ffSold;
			// Determine final balancing required
      Balncr -= FundsIntr +  - Ass->ffSold + LiaF->Repo + LiaF->FF;
			// If negative, add to liabilities
      if(Balncr <= 0)
        Lia->Balancer = Balncr;
			// Add to assets balancer
      else
      {
        Ass->Balancer = Balncr;
        Ass->Total += Balncr;
      }
			// Determining interest costs for TEFRA (Tax Free Securities)
      {
        float IntExpA = 0;

        {
          FeP p = (FeP) &Incm->Interest.Expenses;
          Loop(13)IntExpA +=  *p++;
        }

        Incm->TEFRA = Ac->QMunis *SecK.TEFRA *(Ac->FundsCost = IntExpA / Ass->Total);

        {
          IncomeOutGoT *YTDI = YTD[Bank] + YrsQtr;

          float Cred = 0;
					float TaxableYTD = YTDI->OpEarnings + Incm->OpEarnings + YTDI->Nets.AssetSales + Incm->Nets.AssetSales - YTDI->Interest.Income.Securities.Munis - Incm->Interest.Income.Securities.Munis + YTDI->TEFRA + Incm->TEFRA;
					//  Calculate Income Taxes
					//GeorgeP: I am not too sure why the YrsQtr = 3 processing here
          if(YrsQtr == 3)
          {
            TaxHistT *h = CostsGlo[Bank].TaxHist;
					  TaxHistT *This = h + SimYr % MaxTaxH;
						TaxHistT *Last = h + (SimYr - 1) % MaxTaxH;

            while(1)
            {//will not run if both TaxableYTD and Last->Taxable > 0
              long Losses = 0;
              if(TaxableYTD > 0 && Last->Taxable > 0)
                break;
              {
                long i =  - 1;
								long Yr = SimYr;

                while(++i < MaxTaxH &&  ! (Losses = h[--Yr % MaxTaxH].Taxable < 0));
              }

              if(TaxableYTD > 0 && Losses)
              {
                long i =  - 1, Yr = SimYr - MaxTaxH;

                while(++i < MaxTaxH && TaxableYTD)
                {
                  long hYr = Yr-- % MaxTaxH;

                  if(h[hYr].Taxable < 0)
                  {
                    float T = min(TaxableYTD,  - h[hYr].Taxable);
                    TaxableYTD -= T;
                    h[hYr].Taxable += T;
                  }
                }

                break;
              }

              if(TaxableYTD >= 0)
                break;
              {
                long i =  - 1, Yr = SimYr, Gains = 0;

                while(++i < 3 &&  ! (Gains = h[--Yr % MaxTaxH].Taxable > 0))
                  ;

                if(Gains)
                {
                  long i =  - 1, Yr = SimYr - MaxTaxH;

                  while(++i < 3 && TaxableYTD)
                  {
                    long hYr = Yr-- % MaxTaxH;

                    if(h[hYr].Taxable > 0)
                    {
                      float T = min( - TaxableYTD, h[hYr].Taxable);

                      Cred += T * h[hYr].Rate *.01;

                      TaxableYTD += T;
                      h[hYr].Taxable -= T;
                    }
                  }
                  break;
                }
              }
              break;
            }

            This->Taxable = TaxableYTD;

            This->Rate = EconBOQ.LowerEcon.TaxRate;
          }

          TaxableYTD = TaxableYTD < 0 ? 0 : TaxableYTD;
          {
            float TaxYTD = TaxableYTD * EconBOQ.LowerEcon.TaxRate * .01 - Cred; 
						float TaxQ = TaxYTD - YTDI->Nets.IncomeTaxes;
						//Set Tax Liability and subtract from Net Income
            Incm->Nets.IncomeTaxes = Lia->TaxBill = TaxQ;
            Incm->Nets.Income -= TaxQ;
          }
        }
      }
			// Add Net Income to Owners Equity
      Lia->_Equity += Incm->Nets.Income;

    }
  }
  {
    // REVIEW this block merge might not have worked
		// Calculate new Year to Date Income Statement
		// This add each field value of AC->IncomeOutGo to YTD[Bank][YrsQtr].
		FeP YtoD = (FeP) &YTD[Bank][YrsQtr];
		FeP Qtr = (FeP) &Ac->IncomeOutGo;
    int Sz = IncmSz / 4;
    Loop(Sz)
			YtoD[J] += Qtr[J];
  }
  {//Calculate the average balance sheet for each quarter
    FeP This = (FeP) &BOA[Bank][YrsQtr].Bal;
		FeP Ave = (FeP) &BOA[Bank][YrsQtr].AveBal;
		FeP Last = (FeP) &BOA[Bank][L4[1]].Bal;
    int Sz = sizeof(BalT) / 4;
    Loop(Sz)
			Ave[J] = .5 *(This[J] + Last[J]);
  }
}