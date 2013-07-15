#include "BMSim.h"

//  --- Expenses() ---  See pp 130-137.
extern ResKonstT ResKonst;
extern CostKonstT CostKonst; 
extern LoanGloT LoanGlo[MaxB]; 
extern LoanRptT LoanRpt[MaxB]; 
extern CostsGloT CostsGlo[MaxB]; 
extern ECNT EconBOQ, EconEOQ; 
extern CostsReportT CostsReport[MaxB];
extern ResGloT ResGlo[MaxB]; 

extern BOAT BOA[MaxB]; 
extern int YrsQtr; 
extern int Bank; 

float WageCst, ReqAdj, SIR;
// SalT  TSal;
/// <summary> This function calculates wage cost inside WageCosts(). </summary>
/// <param name="*r"> is required value(RV), e.g. ResGlo[Bank].Req.Sal.Loans. </param>
/// <param name="c"> is current value(CV), e.g. ResGlo[Bank].Cur.Sal.Loans. </param>
/// <param name="f"> is the loan fixed cost. </param>
/// <param name="*b"> is the loan budget. </param>
/// <param name="*t"> is CostsReport[Bank].Act_Sal.Loans. </param>
void Wages(float *r, float c, float f, float *b, float *t)
{

  *r +=  !  * r ? 0 : f;
   *r *= ReqAdj;
   *r = FloorZ(*r);
  *b = max(c,  *r * SIR);
  float a = ResKonst.ReqFloor **r - c;
  a *= a <= 0 ? 0 : 1.5;
  WageCst += (*t = (c + a)) + c * CostKonst.BenFac;
}
/// <summary> This function calculates the salary expense. </summary> 
/// <remarks> It is called by Expenses(). </remarks>
void WageCosts()
{
	extern int LnToSal[]; 
	extern long PrivBkng[MaxB];
	extern float RV_Factors[MaxB][All_Cnt][Sz_RVs];

  Zero(ResGlo[Bank].Req.Sal);
  LoanRptT *LnR = LoanRpt + Bank;
  LoanGloT *LnG = LoanGlo + Bank;
  float NP = CostsGlo[Bank].AdOpsPremInv += ResGlo[Bank].Cur.Premises[3];
	float PMF = 1-CostKonst.PMF * NP * .001 / CostsGlo[Bank].Prem.AdOps;
	float SI = EconEOQ.LowerEcon.SalIndex;
	float oe = CostsGlo[Bank].EVTA;
	float e = .4 * ResGlo[Bank].Eff.Sal.Admin[1] + (1-.4)*(oe <= 0 ? 1 : oe);
	float EVTA = 1-.1 *(e-1);
  ReqAdj = SI * PMF * EVTA;
  CostsGlo[Bank].EVTA = e;
  SIR = 1.015 + EconEOQ.LowerEcon.SalIndexChg * .0025;
  WageCst = 0;
  {
    FeP CV = ResGlo[Bank].Cur.Sal.Loans;
		FeP RV = ResGlo[Bank].Req.Sal.Loans;
		FeP LnVar = CostsReport[Bank].LnVar;
    float ProdActivity[2][MaxL];
    //  memset( RV, 0, sizeof( ResGlo[Bank].Req.Sal.Loans ) );
    { //Copy 16 values in NewCnt to ProdActivity[0]
      //Loop(MaxL)
	  for(int J = 0; J < MaxL; J++)
	  {
		ProdActivity[0][J] = LnR->NewCnt[J];
	  }
    }
    { //Copy 16 values in Loans to ProdActivity[1]
      //Loop(MaxL)
	  for(int J = 0; J < MaxL; J++)
	  {
		  ProdActivity[1][J] = BOA[Bank][YrsQtr].Bal.Assets.Loans.Loans[J];
	  }
    }
    {//caculates RV(ResGlo[Bank].Req.Sal.Loans) and LnVar(CostsReport[Bank].LnVar)
      //Loop(MaxL)
	  for(int t = 0; t < MaxL; t++)
      {
        //long t = J;
        //Loop(2)
		for(int J = 0; J < 2; J++)
        {
          SalCostT *K = &CostKonst.LnOfcr[t][J];  // Loan Officer Salary Costs
          float A = ProdActivity[J][t];
					// K->a, b, c may be New Ln Cnt?
		  float T = .001 * FloorZ(A *(K->a + K->b / (1+K->c *A)));
          RV[LnToSal[t]] += T;
          LnVar[t] += T;
        }
      }
    }
    {//Add wage cost from loans into WageCst. 
      //Loop(7)
	  for(int J = 0; J < 7; J++)
		Wages(RV + J, CV[J], CostKonst.FixedCosts.Loans[J], CostsReport[Bank].Budg.Loans + J, CostsReport[Bank].Act_Sal.Loans + J);
    }
  }
  {//Add wage cost from credit line into WageCst.
    FeP RV = ResGlo[Bank].Req.Sal.Cred;
	FeP CV = ResGlo[Bank].Cur.Sal.Cred;
	FeP CredVar = CostsReport[Bank].CredVar;
    float ProdActivity[3][MaxL];
    {
      //Loop(MaxL)
	  for(int J = 0; J < MaxL; J++)
      {
        ProdActivity[0][J] = LnR->CurByRep[J][4] + LnR->CurByRep[J][5];
        ProdActivity[1][J] = LnR->CurByRep[J][6];
        ProdActivity[2][J] = LnG->NAT[J] + LnR->PastDueAmnt[J];
      }
    }
    //  memset( RV, 0, sizeof(ResGlo[Bank].Req.Sal.Cred) );
    {
      //Loop(MaxL)
	  for(int t = 0; t < MaxL; t++)
      {
        //long t = J;
        //Loop(3)
		for(int J = 0; J < 3; J++)
        {
          SalCostT *K = &CostKonst.Cred[t][J];
          float A = ProdActivity[J][t], T = .001 * FloorZ(A *(K->a + K->b / (1+K->c *A)));
          RV[LnToSal[t]] += T;
          CredVar[t] += T;
        }
      }
    }
    {
      //Loop(7)
		for(int J = 0; J < 7; J++)
			Wages(RV + J, CV[J], CostKonst.FixedCosts.Cred[J], CostsReport[Bank].Budg.Cred + J, CostsReport[Bank].Act_Sal.Cred + J);
    }
  }
  {//Add wage cost from Desposits into WageCst.
    FeP RV = ResGlo[Bank].Req.Sal.Dep;
		FeP CV = ResGlo[Bank].Cur.Sal.Dep;
    //  Zero( xxx * RV );
    {
      //Loop(Sz_RVs -  ! PrivBkng[Bank])
	  for(int t = 0; t < Sz_RVs - !PrivBkng[Bank]; t++)
      {
        //int t = J;
        //Loop(All_Cnt)
		for(int J = 0; J < All_Cnt; J++)
		{
          SalCostT &K = CostKonst.Dep[t][J];
          float A = RV_Factors[Bank][J][t];
					float _RV = .001 * FloorZ(A *(K.a + K.b / (1+K.c * A)));
          const int RV_ToSal[] =
          {
            0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 3
          };
          RV[RV_ToSal[t]] += _RV;
          CostsReport[Bank]._46X_RV[t] += _RV;
        }
      }
    }
    {
      Loop(4- ! PrivBkng[Bank])
				Wages(RV + J, CV[J], CostKonst.FixedCosts.Dep[J], CostsReport[Bank].Budg.Dep + J, CostsReport[Bank].Act_Sal.Dep + J);
    }
  }
  {//Add wage cost from Operations into WageCst.
    float ProdActivity[4] = { 0 };
    FeP RV = ResGlo[Bank].Req.Sal.Ops;
		FeP CV = ResGlo[Bank].Cur.Sal.Ops;
		FeP p = BOA[Bank][YrsQtr].Bal.Assets.Loans.Loans;
    {
      Loop(7)ProdActivity[0] += p[0+J];
    }
    {
      Loop(6)ProdActivity[1] += p[7+J];
    }
    {
      Loop(3)ProdActivity[2] += p[13+J];
    }
    {
      Loop(Sz_RVs)ProdActivity[3] += RV_Factors[Bank][TransCnt][J];
    }
    //  Zero( * RV );
    {
      Loop(4)
      {
        SalCostT *K = &CostKonst.Ops[J];
        float A = ProdActivity[J];
				float T = FloorZ(A *(K->a + K->b / (1+K->c *A))) / 1000;
        RV[J] += T;
        CostsReport[Bank].OpsVar += T;
        Wages(RV + J, CV[J], CostKonst.FixedCosts.Ops[J], CostsReport[Bank].Budg.Ops + J, CostsReport[Bank].Act_Sal.Ops + J);
      }
    }
  }
  {//Add wage cost from Administration into WageCst.
    float *RV = ResGlo[Bank].Req.Sal.Admin;
		float *CV = ResGlo[Bank].Cur.Sal.Admin;
		float *OCV = ResGlo[Bank].Cur.Sal.Ops;
		float ProdActivity[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    {
      FeP c = (FeP) &ResGlo[Bank].Cur.Sal;
			FeP o = (FeP) &ResGlo[Bank].Old.Sal;
      int To = sizeof(SalT) / 4;
      //Loop(To)
	  for(int J = 0; J < To; J++)
      {
        ProdActivity[0] += c[J] - o[J];
        ProdActivity[3] += c[J];
      }
    }
    ProdActivity[2] = ProdActivity[0];
    {
      FeP r = (FeP) &ResGlo[Bank].Req.Sal;
      int To = sizeof(SalT) / 4;
      Loop(To)ProdActivity[1] += r[J];
    }
    ProdActivity[2] = ProdActivity[0];
    {
      Loop(3)ProdActivity[4] += OCV[J];
    }
    ProdActivity[5] += OCV[3];
    memmove(ProdActivity + 6, &CostsGlo[Bank].Prem, 4 *4);
    //  memset( RV, 0, sizeof(ResGlo[Bank].Req.Sal.Admin) );
    {
      Loop(2)
      {
        float T = ProdActivity[J] *CostKonst.Admin[J];
        RV[0] += T;
        CostsReport[Bank].AdminVar += T;
      }
    }
    {
      Loop(2)
      {
        float T = ProdActivity[2+J] *CostKonst.Admin[2+J];
        RV[1] += T;
        CostsReport[Bank].AdminVar += T;
      }
    }
    {
      Loop(6)
      {
        float T = ProdActivity[4+J] *CostKonst.Admin[4+J];
        RV[2] += T;
        CostsReport[Bank].AdminVar += T;
      }
    }
    CV[2] = ResGlo[Bank].Cur.Sal.Ops[4]; // Because  Central-Ops is Sometimes grouped With  Ops.
    {
      Loop(3)Wages(RV + J, CV[J], CostKonst.FixedCosts.Admin[J], CostsReport[Bank].Budg.Admin + J, CostsReport[Bank].Act_Sal.Admin + J);
    }
    ResGlo[Bank].Req.Sal.Ops[4] = RV[2];
  }
}
/// <summary> This function calculates the premise expense. </summary> 
/// <remarks> It is always called inside Expenses(). </remarks> 
void PremCosts()
{
  AcT *Ac = BOA[Bank] + YrsQtr;
  IncomeOutGoT *Inc = &Ac->IncomeOutGo;
  CostsGloT *C = CostsGlo + Bank;
  CostsReportT *Rpt = CostsReport + Bank;
  FeP G = (FeP) &C->Prem;  // G points to the gross premises
	FeP D = (FeP) &C->Depr;  // D points to the accumulated depreciation
	FeP Sales = (FeP) &Rpt->Sales;  //Sales points to sales of fixed assets
	FeP Depr = (FeP) &Rpt->Depr;  //Depreciation (Ptr) will be the depreciation expense calculated for this quarter
	FeP OccExp = (FeP) &Rpt->OccExp; //  Occupancy expense (Ptr) for bank reports
  CostKonstT *K = &CostKonst; // Cost constant structure
  ResGloT *R = ResGlo + Bank; // See data structures on prior page
	//Calculates the depreciation value for each type of premise and total depcreciation.
  {
    FeP g = G - 1, d = D - 1;
    float D, N, T = 0;
    Loop(4) //Calculates the depreciation value.  
    {
      g++;
      d++;
      if(*g < 0)
      {
         *g = 0;
        continue;
      }
      D =  *g * .02;  // Quarterly depreciation at 8% annual) Why 0.02 here?
      N =  *g -  *d;  //  Asset less accum deprec
			// If the Net value - Qtr Depreciation < 0.1 of Net Value then Depreciation = .9 of Net Value.  
			// Keeps asset on the books.  Not likely a problem with current bank asset data.
      if(N - D < .1 *N)
        D = .9 * N;
      *d += D;  //Add deprec. to accum deprec
      T += D;   //total depreciation
      Depr[J] = D;
    }
    Inc->Expenses.Premises += T; //Deprec. added to Income Statement Premises account
  }
	//Calculates the occupany expense for each type of premise and total depcreciation.
  {
    float T = 0;//reset T as total
		//Occupancy expenses = gross premises * 4 occupancy expense factors
    {
      float *k = K->OccExp,  *g = G;
      {
        Loop(4)T += OccExp[J] =  *g++ * *k++;
      }
    }
    Inc->Expenses.Premises += T; //Occ Exp added to Income Statement Premises account
  }
	// The program now processes any premises decisions for this quarter. Assumed to occur  
	// at the end of the quarter.  New additions will not be subject to depreciation this quarter.
  {
    float S, N;
		float *PremDec = R->Cur.Premises - 1;
		float *g = G - 1;
		float *d = D - 1;
		float *s = Sales - 1;
		float *SaleDep = (FeP) &Rpt->SaleDep;
		// There are four asset types with possible decisions.   
		// The decision to add to premises is positive, a sale is negative
    Loop(4)
    {
      PremDec++;
      g++;
      d++;
      s++;
      S =  *PremDec * .001;//*PremDec holds the premise number entered from the decision form.
      N =  *g -  *d;
      if( ! S ||  ! N)
        continue;
      if(S > 0)
      {
         *g += S;
         *s =  - S;
        continue;
      }
			//if the user sells any premise which S is negative.
      *s = S =  - S;
      {
        float T = S /  *g;
				float SD = SaleDep[J] =  *d * T;
         *g -= S;
         *d -= SD;
        Inc->Nets.AssetSales -= (S - SD) *PremLossOnSale;
      }
    }
  }
  //  Now we are going to determine the amount to remove from accumulated 
	//  depreciation for the amount of premises sold.
  //  Calculate the proportion T of the asset sale to the total gross amount for that asset type.  
  //  Multiply the this proportion T times the accum. depreciation.  
  //  Remove this amount (SD) from the  accum. depreciation.  
  
  {//  Calculate Gross premise and accumulated depreciation.  
    float Grs = 0;
		float Depr = 0;
		float *g = G;
		float *d = D;
    {
      Loop(4)
      {
        Grs +=  *g++;
        Depr +=  *d++;
      }
    }
    Ac->Bal.Assets.NetPremises.GrossPremises = Grs;
    Ac->Bal.Assets.NetPremises.AccumDepre = Depr;
  }
	// Calculate CostsReport->Req.Premises
  {
    SalT s = R->Cur.Sal;
    float C, D = 0, O;
		float A = s.Admin[0] + s.Admin[1];
    FeP r = R->Req.Premises, rc = K->PremRV;
    {
      Loop(4)D += s.Dep[J];
    }
    O = s.Ops[0] + s.Ops[1] + s.Ops[2];
    C = D + s.Loans[2] + s.Loans[5] + s.Loans[6] + s.Ops[3];
    *r++ =  *rc++ * C; // City ResGlo[0]->Req.Premises[0]
    *r++ =  *rc++ * (A + D + O + s.Loans[0] + s.Loans[1] + s.Loans[3] + s.Loans[4] + s.Loans[5]); // Region
    *r++ =  *rc++ * (C + O + s.Loans[1] + s.Loans[4]); // Berb
    C = 0;
    {
      Loop(7)C += s.Cred[J];
    }
    *r++ =  *rc++*(A + C + s.Ops[4]);
  }
} // AdOps

float PrmoOth;

/// <summary> This function calculates the promotion cost. </summary>
/// <remarks> The result will be kept in BOA[Bank][YrsQtr].IncomeOutGo.Expenses.Media. </remarks>
void PromoCosts(void)
{
  MediaT *M = &ResGlo[Bank].Cur.Media;
  float Prm = M->Gen;
  PrmoOth = 0;
  {//Deposit promotion
		//PrmoOth is used later for Exp->OtherOpEx. 
    float K = CostKonst.DepPrm;
		float K2 = 1-K;
		float *p = M->Dep - 1;
    Loop(sizeof(BusiDevT) / 4)
    {
      Prm += K **++p;
      PrmoOth += K2 **p;
    }
  }
  {//Loan promotion
    float K = CostKonst.LnsPrm;
	float K2 = 1-K;
	float *p = M->Loans - 1;
    //Loop(MaxL)
	for(int J = 0; J < MaxL; J++)
    {
      p++;
      Prm += K **p;
      PrmoOth += K2 **p;
    }
  }
  BOA[Bank][YrsQtr].IncomeOutGo.Expenses.Media = Prm * .001;
}

/// <summary> This is the main function for calculating costs called by RunForward(). </summary> 
/// <remarks> It sets Exp->SalariesBenes and Exp->OtherOpEx. </remarks>
void Expenses()
{
  Zero(CostsReport[Bank]);
  PremCosts();
  WageCosts();
  PromoCosts();
  float W = WageCst * .001;
  AcT *Ac = BOA[Bank] + YrsQtr;
  ExpensesT *Exp = &Ac->IncomeOutGo.Expenses;
  Exp->SalariesBenes = W;
  Exp->OtherOpEx += W * .05 + PrmoOth * .001 + Ac->CoreDep *.0001;
}
/// <summary> This function sets the various funds based on the parameter B. </summary> 
/// <param name="B"> is - Fns->NetDay1 or (Balncr - FundsIntr). </param>
/// <param name="*FS"> is the federal fund sold. </param>
/// <param name="*R"> is the fund repurchased. </param>
/// <param name="*F"> is the fedural fund available. </param>
/// <returns> the unpledged securities in the book of account. </returns>
/// <remarks> It is called by Books(). </remarks>