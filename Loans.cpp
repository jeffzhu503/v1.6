#include "BMSim.h"
#include "Reporting.h" //for setBankNames()
#include <math.h> //fab()

extern LoanRptT LoanRpt[MaxB]; 
extern LoanGloT LoanGlo[MaxB]; 
extern LoanKonstT LoanKonst; 
extern ECNT EconBOQ; 
extern LnSalesT LnSales[MaxB][MaxLnSales]; 
extern EnvT Env;
extern PricesT Prices; 
extern ResGloT ResGlo[MaxB]; 
extern LnPkgT LnPkg[MaxB][MaxLnPkgs]; 
extern FundsGloT FundsGlo[MaxB]; 

extern LoanDecT LoanDec[MaxB];
extern CredDecT CredDec[MaxB];
extern SellLnsDecT SellLnsDec[MaxB];

extern BOAT BOA[MaxB]; 
extern int YrsQtr, SimQtr; 
extern int Bank; 
extern int NumBanks;
extern float RBanks; //Ratio of each back /total bank
extern pLoansT pLoans[MaxB]; // The actual loan portfolio. MaxB is the max bank number = 6. 

const int MaxActCQ = 11; // sizeof(OffToAct) / 4;
extern int LnSz; //LoanSize = sizeof(LoanT)
//declare the variable for Report 275
R275T R275[MaxB][MaxTypsSold];

float LnFacs[MaxB][MaxL][27];   // Factors for generating new loans
	
struct PRT
{
	float Mat, ProPor;
};
PRT PR[MaxB][MaxL][MaxMats]; // Loan's maturity
/// Links loan types to salaries
int  LnToSal[] =
{
  0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 5, 5, 6, 6, 6
};
int GenCredPol[MaxB], MortBanking[MaxB];

///  How loan quality is distributed.
float OffToAct[] =
{
  1.4, 2.6, 3, 3.6, 4, 4.6, 4.9, 5.4, 5.8, 6.2, 6.8
};
//  Below is a buch of variables for handeling the loan logic
float MarketAmntByMk[MaxM], MarketAmntByType[MaxL], CMR[MaxB][MaxL][MaxCQ], EconRAMP[MaxL], 
	ZMP[MaxB][MaxL], RI[MaxB][MaxL], Wan[MaxB][MaxL], AvWan[MaxL], AvER[MaxL], MarketAmntByTypeByQual[MaxL][MaxCQ],  // Loan market size by loan type and quality
	EBDB[MaxB][MaxL], ELOS[MaxB][MaxMD], ELOS2[MaxB][MaxMD], ECAS[MaxB][MaxMD], EPROC[MaxB][MaxOD], 
	BRCH[MaxB][MaxPrem], ORIG_FEE[MaxB][MaxL], ANN_FEE[MaxB][MaxL], RAMP[MaxB][MaxL], RAMPShare[MaxB][MaxL], 
	Rel_Pol[MaxB][MaxL][MaxCQ], MaxOuts[MaxB][MaxL], LNRR[MaxB][MaxL][MaxCQ], CurInternalMkShare[MaxB][MaxL][MaxCQ], 
	ActSize[MaxB][MaxL][MaxActCQ], Current[MaxB][MaxL][MaxRepCQINPUT + 1], Desired[MaxB][MaxL][MaxRepCQINPUT + 1], 
	CQ_rate[MaxB][MaxL][MaxCQ]; // Rates by credit quality loan type and bank

// These macros assign names to groups of various loan types.
/// <summary> This function decides whether the loan type belongs to Group A. </summary>
/// <param name="t"> is the loan type. </param>
/// <returns> true if loand type is 0-3 and 7-9. Otherwise it returns false. </returns>
/// <remarks> GroupA loan only has one single loan such as national business loan type.  
/// GroupB loan has a number of loans which are bundled together such as personal loan type. </remarks> 
long GroupA(long t)
{
  return t < 10 && t != 4 && t != 5 && t != 6;
}

/// <summary> This function determines whether the loan is Class I Credit Line based on the loan type. </summary>
/// <param name="t"> is the loan type from 1 to 15. </param>
/// <returns> true if loan type t is 0, 2, 4, 6 or 7. Otherwise it returns false. </returns> 
long IsClsI(long t)
{
  return t == 0 || t == 2 || t == 4 || t == 6 || t == 7;
} // CredLn

/// <summary> This function determines whether the loan is Class II Term based on the loan type. </summary>
/// <param name="t"> is the loan type from 1 to 15. </param>
/// <returns> True if loan type t is 1, 3 or 5. Otherwise it returns false. </returns> 
long IsClsII(long t)
{
  return t == 1 || t == 3 || t == 5;
} // Term

/// <summary> This function determines whether the loan is Class III Big RE based on the loan type. </summary>
/// <param name="t"> is the loan type from 1 to 15.  </param>
/// <returns> True if loan type t is 8 or 9. Otherwise it returns false. </returns> 
long IsClsIII(long t)
{
  return t == 8 || t == 9;
} // Big RE

/// <summary> This function determines whether the loan is Class IV House-Install, Car based on the loan type. </summary>
/// <param name="t"> is the loan type from 1 to 15. </param>
/// <returns> True if loan type t is 10 or 15. Otherwise it returns false. </returns> 
long IsClsIV(long t)
{
  return t == 10 || t == 15;
} // House-Instll, Car

/// <summary> This function determines whether the loan is Class VI Home Equity, CC based on the loan type. </summary>
/// <param name="t"> is the loan type from 1 to 15. </param>
/// <returns> True if loan type t is 12 or 14. Otherwise it returns false. </returns> 
long IsClsVI(long t)
{
  return t == 12 || t == 14;
} // HmEqu, CC

/// <summary> This function determines whether the loan has AdjMat based on the loan type. </summary>
/// <param name="t"> is the loan type from 1 to 15. </param>
/// <returns> true if loan type t is Class II, III or type is 11 or 13. Otherwise it returns false. </returns> 
/// <remarks> Per George, t = 13 does not seem to be right. </remarks> 
long IsAdjP(long t)
{
  return IsClsII(t) || IsClsIII(t) || t == 11 || t == 13;
}

/// <summary> This function returns the index of maturity class based on the maturity number. </summary>
/// <param name="Mat"> is the loan maturity. </param>
/// <returns>maturity class, the index in MatClass[]. </returns>
/// <remarks> The loan maturity class can be found in Report B204. </remarks>
long MatClass(long Mat)
{
  // MatClass[] = {1,2,3,4,6,8,12,20};
	int intIndexOfCount;
  for(intIndexOfCount = 0; intIndexOfCount < 8; intIndexOfCount++)
    if(Mat <= LoanGlo[Bank].MatClass[intIndexOfCount])
      break;
  return intIndexOfCount;
}

/// <summary> This function accumulates interest into the income statement. </summary>
/// <param name="t"> is the loan type. </param>
/// <param name="I"> is the interest. </param>
/// <remarks> This interest is added to BOA[Bank][YrsQtr].IncomeOutGo.Interest.Income.Loans.Loans[t], 
/// BOA[Bank][YrsQtr].IncomeOutGo.Interest.Income.Loans.Sum and LoanRpt[Bank].Iin[t]. </remarks>
void InterestCash(ushort t, float I)
{
  BOA[Bank][YrsQtr].IncomeOutGo.Interest.Income.Loans.Loans[t] += I;
  BOA[Bank][YrsQtr].IncomeOutGo.Interest.Income.Loans.Sum += I;
  LoanRpt[Bank].Iin[t] += I;
}

/// <summary> This function accumulates the loan amounts by the reported credit quality for each bank and loan type.</summary>
/// <remarks> It is called by Loans. The function calculates Current[MaxB][MaxL][MaxRepCQINPUT + 1]. 
/// The loan is categorized 5 groups: 1/2, 3, 4, 5, 6. </remarks>
void CurLoans()
{
  memset(Current + Bank, 0, sizeof(*Current));
  {
    for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
    {
      long TypeI = intIndexOfCount;
      LoanT *p = pLoans[Bank][TypeI] - 1;
      long RecCnt = (long)LoanGlo[Bank].LnCnt[TypeI];
      for(int intIndexOfCount = 0; intIndexOfCount < RecCnt; intIndexOfCount++)
      {
        WORD x = (WORD)(++p)->Reported;
        if(x >= 7)
          continue;
        // 1&2,3...6 -- Like INPUT
        Current[Bank][TypeI][x < 3 ? 0 : x - 2] += p->Amnt;
      }
    }
  }
}

/// <summary> This function determines the percentage of loans in varous maturities. </summary>
/// <param name="TypeI"> is the loan type. </param>
/// <param name="Mx"> is the maturity. </param>
/// <returns> This function returns the percentage of loans. </returns>
/// <remarks> ZMP is the maximum maturity decision into a proportion of borrowers demanding that maturity or less. </remarks> 
float ZMPf(int TypeI, float Mx)
{
  float F1, F2, M, M1 = LoanKonst.Mat1[TypeI], M2 = LoanKonst.MatShorter[TypeI];
  if(IsClsVI(TypeI) && Mx >= 60)
    return 1;
  F1 = M2 * 4-M1 * 3-1;
  F2 = 2+M1 * 2-M2 * 4;
  M = Bound((Mx - 1) / (LoanKonst.MaxMat[TypeI] - 1), 0., 1.);
  return max(0, M1 + F1 * M + F2 * M * M);
}

/// <summary> This function allocates the memory that holds a list of loans in a single type. </summary>
/// <param name="T"> is the loan type. </param> 
LoanT *Alloc_A_New_Loan_Rec(int T)
{
	extern char Mess[400]; 
	int LnSz = sizeof(LoanT); 

  if(T < 0 || T >= MaxL)
    return (LoanT*)Mess;
  {
    WORD NewCnt = (WORD)++LoanGlo[Bank].LnCnt[T];
    {
      LoanT *p = pLoans[Bank][T] = (LoanT*)realloc(pLoans[Bank][T], (WORD)NewCnt *(WORD)LnSz);
			return (LoanT*)memset(p + NewCnt - 1, 0, LnSz);
    }
  }
}

/// <summary> This function calculates the projected income for each loan based on type and maturity. </summary>
/// <remarks> It is called by Loans(). 
/// The function calculates LoanRptT->ProjInc and LoanRptT->Prin for each loan type and maturity.</remarks>
// It loops through each loan by MaxL and LnG->LnCnt[TypeI]
// pLoans has the loan information loaded from LN00.DAT. 
// LnG has LoanGloT from BK01.DAT [24:0-215]
void Projected()
{
	LoanRptT *LnR = LoanRpt + Bank;
	LoanGloT *LnG = LoanGlo + Bank;

	/*
	//Review: Warnings. This is where the root of the warnings sits. If you change back
	back to the 2 Loop() macros it will not complain. Also if you take the actual code
	from the macro and put it here it will not complain.
	An easy work around would be to rename my local variable in the nested loop?  (Noam - Feb 24 2005)
	 */
    //Loop(MaxL)
	for(int TypeI = 0; TypeI < MaxL; TypeI++)
	{
		//long TypeI = J; LoanT *p = pLoans[Bank][TypeI] - 1;
		//long TypeI = intIndexOfCount;
		LoanT *p = pLoans[Bank][TypeI] - 1;
		long RecCnt = (long)LnG->LnCnt[TypeI];

	   //Loop(RecCnt)
		for(int intIndexOfCount1 = 0; intIndexOfCount1 < RecCnt; intIndexOfCount1++)
		{
			p++;
			if(p->Reported == 8 || p->Amnt <= 0)
				continue;
			{//Calculates loan amount based on the maturity class and loan type for B204
				float Amnt = p->Amnt;
				float Mat = p->Mat;
				float MatC = MatClass((long)Mat);
				float Paymnt = p->Payment;
				float Rate = p->Rate *.0025;
				
				int NoI = IsClsII(TypeI) || IsClsIII(TypeI) || IsClsVI(TypeI);
				int NoP = IsClsI(TypeI) || TypeI == 13;

				if( ! Paymnt || NoP)
					LnR->Mat[TypeI][(long)MatC] += Amnt;
				else
				{
					long m = 0;
					long Last = min((long)Mat, 21);
					float Bal = FloorZ(Amnt);
					while(Bal && ++m < Last)
					{
						long MatC = MatClass(m);
						float I = NoI ? 0 : Rate *Bal;
						float P = FloorZ(Paymnt - I);
						P = min(Bal, P);
						LnR->Mat[TypeI][MatC] += P;
						Bal -= P;
					}
				    LnR->Mat[TypeI][MatClass(m)] += Bal;
				}
			
				{
					//Calculate the principle payment and interest payment for the loan report in B208. 
					long PreMat = (long)Mat - 1;
					long m = (long)Mat > Max4Q ? Max4Q : PreMat; //Only project the next 4 quarters or less. 

					float Bal = Amnt;

					if(IsClsIV(TypeI) || TypeI == 12)
					{
					    int intIndexOfCount2;
						for(intIndexOfCount2 = 0; intIndexOfCount2 < m; intIndexOfCount2++)
						{
				            float I = Bal * Rate;
							float Amort = Paymnt - I;

							if(Bal <= 0)
								break;
							if(Amort > Bal)
								Amort = Bal;

							LnR->Prin[TypeI][intIndexOfCount2] += Amort;
							LnR->ProjInc[TypeI][intIndexOfCount2] += I;
							Bal -= Amort;
						}
			
						//If the maturity is less than or equal to 4 quarters, it only counts half interest income for the last quarter.  
						if(intIndexOfCount2 == PreMat && Bal > 0)  
						LnR->ProjInc[TypeI][intIndexOfCount2] += (Bal *Rate) / 2;

						LnR->Prin[TypeI][intIndexOfCount2] += Bal;
					}
					else
					{
						if(Paymnt > 0)
						{
				            if(TypeI == 14)
							Rate *= 1-LoanKonst.TransBal;
							{
								int intIndexOfCount3;
								for(intIndexOfCount3 = 0; intIndexOfCount3 < m; intIndexOfCount3++)
								{
									if(Bal < Paymnt)
										break;
									LnR->Prin[TypeI][intIndexOfCount3] += Paymnt;
									LnR->ProjInc[TypeI][intIndexOfCount3] += (Bal - .5 * Paymnt) *Rate;
									Bal -= Paymnt;
								}

					            if(intIndexOfCount3 == PreMat && Bal > .5 *Paymnt)
								LnR->ProjInc[TypeI][intIndexOfCount3] += ((Bal - .5 * Paymnt) *Rate) / 2;

				                LnR->Prin[TypeI][intIndexOfCount3] += Bal;
							}
						}
						else
						{//For some types of loan (e.g. credit line) it may not have loan payment.  
							float IntIncm = Amnt * Rate;
							int intIndexOfCount4;
							for(intIndexOfCount4 = 0; intIndexOfCount4 < m; intIndexOfCount4++)
								LnR->ProjInc[TypeI][intIndexOfCount4] += IntIncm;

							if(intIndexOfCount4 == PreMat)
								LnR->ProjInc[TypeI][intIndexOfCount4] += IntIncm / 2;

							LnR->Prin[TypeI][intIndexOfCount4] += Amnt;
						}
					}
				}
			}
		} // for1
	} // end for 0
}

/// <summary> This function accumulates information for the loan portfolio. </summary>
/// <remarks> The function calcuates the non-Accural Total, the loan amount based on different credit quality and the Past-Due amount for Report B205.
/// The information are including LoanGlo->NAT[TypeI], LoanRpt->CurByRep[TypeI] and LoanRpt->PastDueAmnt[TypeI]. </remarks> 
void EoQRpt()
{
  LoanRptT *LnR = LoanRpt + Bank;
  LoanGloT *LnG = LoanGlo + Bank;
  for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
  {
    long TypeI = intIndexOfCount;
    if(GroupA(TypeI))
      LnG->NAT[TypeI] = 0;  //Non-Accrual Total
    else
      LnR->CurByRep[TypeI][7] = LnG->NAT[TypeI];
    {
      LoanT *p = pLoans[Bank][TypeI] - 1;
      long RecCnt = (long)LnG->LnCnt[TypeI];
      for(int intIndexOfCount = 0; intIndexOfCount < RecCnt; intIndexOfCount++)
      {
        p++;
        if(p->Amnt <= 0)
          continue;
        LnR->CurByRep[TypeI][(long)p->Reported - 1] += p->Amnt; //the loan amount based on different credit quality
        if(GroupA(TypeI) && p->Reported == 8) 
          LnG->NAT[TypeI] += p->Amnt;
        else
          LnR->PastDueAmnt[TypeI] += p->Amnt *p->PastDue;
      }
    }
  }
}
/// <summary> This function calculates the payment based on the rate and maturity.  </summary>
/// <param name="QtrDecRate"> is the loan rate. </param>
/// <param name="QtrsToMat"> is the quarters to the maturity. </param> 
/// <returns> the payment.  </returns> 
float Pmnt(float QtrDecRate, float QtrsToMat)
{
  if(QtrDecRate < 0 || QtrsToMat <= 0)
    return 0;
  if( ! QtrDecRate)
    return 1 / QtrsToMat;
  return QtrDecRate / (1-Pow(1+QtrDecRate,  - QtrsToMat));
}

/// <summary> This function calculates the expected duration of a loan on the books given four factors. </summary>
/// <param name="A"> is the loan amount. </param>
/// <param name="P"> is the payment.  </param>
/// <param name="R"> is the rate. </param>
/// <param name="M"> is the maturity.  </param>
/// <returns> the duration. </returns>
float Duration(float A, float P, float R, float M)
{
  float Y, Z, Q, F;
  if(A < 0 || R <= 0 || P < 0)
    return  - 1;
  if(M <= 0)
    return 0;
  if( ! P)
    return M;
  F = 1+R;
  Q = A / P;
  Y = Pow(F, M) + Q * R - 1;
  Z = Q * F + M *(1-Q * R);
  return Y ? F / R - Z / Y:  - 1;
}

// Retruns a catagory of commitments given a loan type
int LnToCL[] =
{
  0,  - 1, 1,  - 1, 2,  - 1, 3, 4,  - 1,  - 1,  - 1,  - 1, 5,  - 1, 6,  - 1
};

#pragma region NewLoans Definition
/// Data structure for amount and maturity used in ByActT. 
struct ByMatT
{
  float Amnt, Mat;
};
/// Data structure used in NewLoansT. 
struct ByActT
{
  ByMatT ByMat[MaxMats];
  float Amnt;
  long Reported;
};
/// For calculating new loans generated.
struct NewLoansT
{
  ByActT ByAct[MaxActCQ];
  float Base, Spread, Fee, AdjP, OrigFee;
};
NewLoansT NewLoans[MaxB][MaxL];
#pragma endregion

// Creates new loans.  Design begins on p 60.
/// <summary> This function creates new loans based on NewLoans[Bank] and also prepares the loan report in LoanRpt. </summary>
/// <remarks> It is called by Loans(). Basically this function updates LoanRpt 
/// based on NewLoans by MaxL (Loan Type), by MaxActCQ(Loan Quality), and by MaxMats (Loan Maturity).</remarks>
// Data structures involved: LoanRptT, NewLoansT, ByActT, ByMatT, LoanT, LoanKonst, 
void CreateNewLoans()
{
  long Id = 0;
  LoanRptT *LnR = LoanRpt + Bank;
	//loop through NewLoans.ByAct.ByMat
  for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
  {
    long TypeI = intIndexOfCount;
    NewLoansT *NwLn = NewLoans[Bank] + intIndexOfCount;
    for(int intIndexOfCount = 0; intIndexOfCount < MaxActCQ; intIndexOfCount++)
    {
      float Act = OffToAct[intIndexOfCount]; //Distributed credit quality
      ByActT *ActP = NwLn->ByAct + intIndexOfCount;
      for(int intIndexOfCount = 0; intIndexOfCount < MaxMats; intIndexOfCount++)
      {
        float Amnt = ActP->ByMat[intIndexOfCount].Amnt;
        if( ! Amnt)
          continue;
        {
          float AP = NwLn->AdjP;
					float AFee = NwLn->Fee;
					float Rep = ActP->Reported;
					float Mat = ActP->ByMat[intIndexOfCount].Mat;
					float Cnt = GroupA(TypeI) ? 1 : Amnt / LoanKonst.LoanSize[TypeI];
					float Rate = NwLn->Base + NwLn->Spread *((Rep - 1) / (MaxRepCQ - 1)) + (Rep == 7);
          {	//q is pointing to pLoans[0][0]+120
						//What is Alloc_A_New_Loan_Rec doing? allocates the memory that holds a list of loans in a single type.
            LoanT *q = Alloc_A_New_Loan_Rec(TypeI);
            q->Id = ((long)SimQtr *10000+ ++Id); 
            q->Type = TypeI + 1;
            q->MktVal = q->Amnt = Amnt;
            q->Mat = Mat;
            q->Reported = Rep;
            q->Actual = Act;
            q->Rate = Rate;
            if( ! IsClsIV(TypeI))
            {
              float SpreadF = EconBOQ.Econ.Prime;
              if( ! IsClsI(TypeI))
                SpreadF = IsClsII(TypeI) || IsClsIII(TypeI) ? YldC(AP, BoQ): EconBOQ.Econ.One_Qtr_Treas;
              q->Spread = Rate - SpreadF;
            }
            if(IsClsVI(TypeI) || IsClsI(TypeI))
              q->AnnFee = AFee;
            if(IsAdjP(TypeI))
            {
              q->AdjP = AP;
              q->AdjMat = min(AP, Mat);
            }
            if(TypeI == 7)
              q->Commit = q->Mat *Amnt;
            else if(IsClsI(TypeI))
              q->Commit = Amnt / ((float).4 + .1 * AFee);
            if(TypeI == 12)
              q->Commit = Amnt / ((float).3 + .01 * AFee);
            if(TypeI == 14)
              q->Commit = 2 * Amnt;
            if(q->Commit)
              q->UsageRate = Amnt / q->Commit, LnR->NewCommit[LnToCL[TypeI]] += q->Commit;
            if(IsClsII(TypeI))
              q->Payment = Amnt / q->Mat;
            if(IsClsIII(TypeI))
              q->Payment = Amnt / 120;
            if(IsClsIV(TypeI) || TypeI == 11)
              q->Payment = Amnt * Pmnt(q->Rate *.0025, q->Mat);
            if(IsClsVI(TypeI) && q->Mat < 61)
              q->Payment = Amnt / q->Mat;
            {
              float Dur = 0;
              if(IsAdjP(TypeI) && AP < Mat)
                Dur = AP - 1;
              else if(IsClsIV(TypeI) || IsAdjP(TypeI))
                Dur = Duration(Amnt, q->Payment, Rate *.0025, Mat);
              q->Dur = Dur;
            }
          }
					//Calculate LoanRpt
          LnR->New[TypeI] += Amnt;
          LnR->NewByRep[TypeI][(long)Rep - 1] += Amnt;
          LnR->NewCnt[TypeI] += Cnt;
          {
            float t14 = TypeI == 14 ? 1-LoanKonst.TransBal: 1;
            InterestCash(TypeI, Amnt *Rate * .00125 * t14); //0.00125: assuming the bank only gets the interest of half quarter when making the new loan.
            {
              float OFee = Amnt * NwLn->OrigFee *.01 * t14 + (TypeI == 14 ? Amnt *LoanKonst.MerDis: 0);
              LnR->OrgFees[TypeI] += OFee;
              {
                float AnFee = 0;
                if(IsClsI(TypeI))
                  AnFee = Amnt / 100;
                if(IsClsVI(TypeI))
                  AnFee = Cnt * .000001;
                LnR->AnnFees[TypeI] += (AnFee *= AFee * .25);
              }
            }
          }
        }
      }
    }
  }
}
//gathers up loans that are too small and puts them in other credit quality catagories. 
/// <summary> This function gathers up loans that have too small amount and add them together to a single loan which amount is larger than the Tiny value.</summary>
/// <param name="*p"> is the array of floating point numbers in ActSize[]. </param>
/// <param name="L"> is the account number. </param>
/// <param name="Tiny"> is the minimum create amount.  </param>
/// <remarks> The function loops through a collection of loan amount and add the loan amount that is smaller than Tiny value in order to create a single non-Tiny loan. </remarks>  
void Gel(float *p, long L, float Tiny)
{
  while(1)
  {
    long Found = 0;
		long BstDelta = 1000;
		long BstL, BstR;
    float BstSum = 10000;
    for(int intIndexOfCount = 0; intIndexOfCount < (L - 1); intIndexOfCount++)
    {
      long Left = intIndexOfCount;
      if( ! p[Left])
        continue;
      {
        for(int intIndexOfCount = 0; intIndexOfCount < (L - (Left + 1)); intIndexOfCount++)
        {
          long Right = Left + (intIndexOfCount + 1);
          if( ! p[Right])
            continue;
          if(p[Left] >= Tiny && p[Right] >= Tiny)
            break;
          Found = 1;
          {
            long Delta = Right - Left;
            if(Delta < BstDelta)
            {
              BstDelta = Delta;
              BstL = Left;
              BstR = Right;
              break;
            }
            {
              float Sum = p[Left] + p[Right];
              if(Delta == BstDelta && Sum < BstSum)
              {
                BstSum = Sum;
                BstL = Left;
                BstR = Right;
              }
            }
            break;
          }
        }
      }
    }
    if( ! Found)
      break;
    {
      float S = p[BstL] + p[BstR];
      p[BstL] = p[BstR] = 0;
      p[BstL + (BstR - BstL) / 2] = S;
    }
  }
  {
    for(int intIndexOfCount = 0; intIndexOfCount < L; intIndexOfCount++)
    if(p[intIndexOfCount] && p[intIndexOfCount] < Tiny)
    {
      p[intIndexOfCount] = 0;
      break;
    }
  }
}

/// <summary> This function distributes new loans to maturity and credit quality classifications. </summary>
/// <remarks> It is called by Loans(). </remarks>
void BkDecs()
{

	static float AmntAv[MaxB][MaxL][MaxCQ];
	LB
	{//Calculate AmntAv[MaxB][MaxL][MaxCQ]
		for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
		{
			float *av = AmntAv[Bank][intIndexOfCount];
			float *a = MarketAmntByTypeByQual[intIndexOfCount];
			float *s = CurInternalMkShare[Bank][intIndexOfCount];
			for(int intIndexOfCount = 0; intIndexOfCount < MaxCQ; intIndexOfCount++) //MaxCQ = 3
				av[intIndexOfCount] = a[intIndexOfCount] * s[intIndexOfCount];
		}
	}
	
	LB
	{// Process the data for bank report 275
		//Calculates LoanRptT->New, Sales, NewCnt, OrgFees and LnSalesT->Amnt, Pay, Mat, Rate for Mortgage Banking.
		if( ! MortBanking[Bank]) //Ac->MortBanking
			continue;
		{
			LoanRptT *LnR = LoanRpt + Bank;
			R275T *Sales = R275[Bank];
			for(int intIndexOfCount = 0; intIndexOfCount < MaxCQ; intIndexOfCount++)
			{
				int M = LoanKonst.Mat[Fixed_rate];
				float R = CMR[Bank][Fixed_rate][intIndexOfCount];
				float A = AmntAv[Bank][Fixed_rate][intIndexOfCount];
				float F = NewLoans[Bank][Fixed_rate].OrigFee;
				float P = A * Pmnt(R *.0025, M);
				LnR->New[Fixed_rate] += A;
				LnR->Sales[Fixed_rate] += A;
				Sales[5].A += A;  //The total loan sales in report 275
				LnR->NewCnt[Fixed_rate] += A / LoanKonst.LoanSize[Fixed_rate];
				LnR->OrgFees[Fixed_rate] += A * F * .01;
				{
					LnSalesT *t = LnSales[Bank] + 5; 
					float NewA = t->Amnt + A;
					float W = A / NewA;
					float W0 = 1-W;
					t->Amnt = NewA;
					t->Pay += P;
					t->Mat = t->Mat *W0 + M * W;
					t->Rate = t->Rate *W0 + R * W;
				}
			}
		}
	}

	{//Calculate LoanRpt.NewByRep
		static int SpLp[] =
		{
		  1, 5, 5
		};
		static float Split[] =
		{
		  1, .2, .2, .2, .2, .2, .2, .2, .3, .3, .2
		};
		LB
		{//Distribut the new loan amount by the reported credit quality
			LoanRptT *LnR = LoanRpt + Bank;
			for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
			{
				int ActI = 0;
				int t = intIndexOfCount;
				int MB = t == Fixed_rate && MortBanking[Bank];
				float *av = AmntAv[Bank][intIndexOfCount];
				float *act = ActSize[Bank][intIndexOfCount];
				float *sp = Split;
				for(int intIndexOfCount = 0; intIndexOfCount < MaxCQ; intIndexOfCount++)
				{
					float AV = av[intIndexOfCount];
					float Max = SpLp[intIndexOfCount];
					for(int intIndexOfCount = 0; intIndexOfCount < Max; intIndexOfCount++)
					{
						*act++ = AV * *sp++;
						if(MB && ActI < 10)
						LnR->NewByRep[t][Round(OffToAct[ActI++]) - 1] += act[intIndexOfCount];  //new loan by reported quality
					}
				}
			}
		}
	 }
  
	{
		float Conditions = Pow(EconBOQ.Econ.LLI, LoanKonst.LLExp); 
		LB
		{
			int Pol = GenCredPol[Bank];  //General Credit Policy including Restricted - 1, Controlled - 3, Moderate - 4, and Expansive - 6
			LoanRptT *LnR = LoanRpt + Bank;
			for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
			{
				int TypeI = intIndexOfCount;
				if(TypeI == Fixed_rate && MortBanking[Bank])
				continue;
				static float PolToWght[] = { 1.1, 1, 1, .95 };
				struct
				{
					float Act[MaxActCQ];
				} Reported[MaxRepCQ] =
				{
					0
				};
				float Portion[MaxRepCQINPUT + 1] = //Portion of amount for this credit quality 0, 1 are in the same column in the report.
				{
					0
				};
				Gel(ActSize[Bank][TypeI], MaxActCQ - 1, LoanKonst.MinNewTyp[TypeI]);
			{//Calculate Portion and Reported
				float Sal1 = ELOS2[Bank][LnToSal[TypeI]]; //Effective Loan Officer Salary 
				float Sal2 = ECAS[Bank][LnToSal[TypeI]]; //Effective Credit Administration Salary
				float LL = PolToWght[Pol] *Conditions*Pow(Sal1, Sal1 > 1 ? 0 : LoanKonst.ELOS[TypeI])*Pow(Sal2, Sal2 > 1 ? 0 : LoanKonst.ECAS[TypeI]);
				float L = Bound(LL, .8, 1.12);
				for(int intIndexOfCount = 0; intIndexOfCount < MaxActCQ; intIndexOfCount++)
				{
					int Rep = Bound(Round(OffToAct[intIndexOfCount] *L) - 1, 0, 6);
					float Amnt = ActSize[Bank][TypeI][intIndexOfCount];

					//Remove: using int InpRep instead.
					//#define InpRep ! Rep ? 0 : Rep - 1
					int InpRep = (! Rep ? 0 : Rep - 1);
					Portion[InpRep] += Amnt;
					Reported[Rep].Act[intIndexOfCount] += Amnt;
				}
			}
			{//Re-calculate Portion based on the comparision between NewAvail (=Portion) and NewWanted
			static float LCQ[] =
			{
				0, .1, .2, .4
			}
			, HCQ[] =
			{
				0, .25, .5, 1
			};
			for(int intIndexOfCount = 0; intIndexOfCount < MaxRepCQINPUT; intIndexOfCount++)
			{
				float NewAvail = Portion[intIndexOfCount];
				if( ! NewAvail)
					continue;
            float NewWanted = Desired[Bank][TypeI][intIndexOfCount] = FloorZ(Desired[Bank][TypeI][intIndexOfCount] - Current[Bank][TypeI][intIndexOfCount]);
            if(NewWanted >= NewAvail)
            {
              Portion[intIndexOfCount] = 1;
              continue;
            }
            float P = NewWanted + (intIndexOfCount < 3 ? HCQ[Pol]: LCQ[Pol])*(NewAvail - NewWanted);
            Portion[intIndexOfCount] = FloorZ(P / NewAvail);
          }
          {//Determine Portion[5] based on the credit policy users specifiy in the decision form. 
            static float LowGradeRatio[] =
            {
              0, 0, .1, .25
            };
            Portion[MaxRepCQINPUT] = LowGradeRatio[Pol];  
          }
        }
        {
					NewLoansT *NwLn = NewLoans[Bank] + TypeI;
          {//NwLn->ByAct[intIndexOfCount].Amnt = Portion * Reported.  These two values are calcuated above. 
            for(int intIndexOfCount = 0; intIndexOfCount < MaxRepCQ; intIndexOfCount++)
            {
              int Rep = intIndexOfCount;
							int InpRep = (! Rep ? 0 : Rep - 1);
              float Por = Portion[InpRep];
              if( ! Por)
                continue;
              {
                for(int intIndexOfCount = 0; intIndexOfCount < MaxActCQ; intIndexOfCount++)
                {
                  if( ! Reported[Rep].Act[intIndexOfCount])
                    continue;
                  NwLn->ByAct[intIndexOfCount].Reported = Rep + 1;
                  NwLn->ByAct[intIndexOfCount].Amnt = Por * Reported[Rep].Act[intIndexOfCount];
                  if(Rep < 6)
                    LnR->TurnDwnsAmnt[TypeI] += (1-Por) *Reported[Rep].Act[intIndexOfCount];
                }
              }
            }
          }
          {//Calcuate the NwLn->ByAct[Act].ByMat based on NwLn->ByAct[intIndexOfCount].Amnt calcuated in the previous block. 
						//Jeff2 simple version: NwLn->ByAct[i].ByMat[j] = NwLn->ByAct[i] * PR[Bank][TypeI][intIndexOfCount].ProPor
            for(int intIndexOfCount = 0; intIndexOfCount < MaxActCQ; intIndexOfCount++)
            {
              int Act = intIndexOfCount;
              float A = NwLn->ByAct[intIndexOfCount].Amnt;
							float AmntByMat[MaxMats];
              for(int intIndexOfCount = 0; intIndexOfCount < MaxMats; intIndexOfCount++)
                AmntByMat[intIndexOfCount] = A * PR[Bank][TypeI][intIndexOfCount].ProPor;
              Gel(AmntByMat, MaxMats, LoanKonst.MinCreateAmnt);
              {
                ByMatT *n = NwLn->ByAct[Act].ByMat - 1;
                for(int intIndexOfCount = 0; intIndexOfCount < MaxMats; intIndexOfCount++)
                {
                  float A = AmntByMat[intIndexOfCount];
                  if( ! A)
                    continue;
                  n++;
                  n->Amnt = A;
                  n->Mat = PR[Bank][TypeI][intIndexOfCount].Mat;
                }
              }
            }
          }
        }
      }
    }
  }
}

/// <summary> This function determines the market share for each bank.. </summary>
/// <remarks> It is called by Loans(). This function is in the design document section NEW LOANS AND MARKET SHARES. </remarks>
void MktShare()
{	
	extern long EconSet_I;

	//Calculates EBDB[Bank][TypeI] - the effective value of the business development budget for each bank for the loan type
	static float AnnFeeRatio[MaxB][MaxL];
	{
		for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
		{
			long TypeI = intIndexOfCount;
			float X = 0;
			LB X += EBDB[Bank][TypeI];
			X *= RBanks;
			LB EBDB[Bank][TypeI] = max(.6, (EBDB[Bank][TypeI] + 1) / (X + 1.2));
		}
	}
	{	//Calculate ORIG_FEE[Bank][TypeI] - the origination fee plus two divided by the community average plus 2.0
		for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
		{
			long TypeI = intIndexOfCount;
			float X = 0;
			LB X += ORIG_FEE[Bank][TypeI];
			X *= RBanks;
			LB ORIG_FEE[Bank][TypeI] = (ORIG_FEE[Bank][TypeI] + 2) / (X + 2);
		}
	}
	memmove(AnnFeeRatio, ANN_FEE, sizeof(ANN_FEE));
  
	{//Calculates ANN_FEE[Bank][TypeI] - the annual fee plus 2.0 (percentage) or 20 (dollars) 
		//Type 0,2,4,6,7 are 2 percentage.  
	    for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
		{
			long TypeI = intIndexOfCount;
			float X = 0;
			LB X += ANN_FEE[Bank][TypeI];
			X *= RBanks;
			LB
			{
		       long x = 20;
				switch(TypeI)
				{
					case 0:
					case 2:
					case 4:
					case 6:
						x = 2;
				}
				ANN_FEE[Bank][TypeI] = (ANN_FEE[Bank][TypeI] + x) / (X + x);
			}
		}
	}
	{//Calculate RAMPShare[Bank][TypeI] - the rate adjustment maturity preference.
		//RAMP for the loan type devided by the community average.  
		long EconNum = (long)Env.Seq[EconSet_I - 1][SimQtr];
		for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
		{
			long TypeI = intIndexOfCount;
			float X = 0;
			LB X += RAMP[Bank][TypeI]; //RAMP is calculated in LoanDecisions(). 
			if(EconNum == 10)
				X /= NumBanks;
			else
			{
				X += (10-NumBanks) *EconRAMP[TypeI];
				X /= 10;
			}
			LB RAMPShare[Bank][TypeI] = RAMP[Bank][TypeI] / X;
		}
	}
	//General credit policy divided by community average.  
	//R(estricted)=1.0, C(ontrolled)=3.0, M(oderate)=4.0, E(xpansive)=6.0
	float RelPol[MaxB];
	static float PolToFac[] =
	{
		1, 3, 4, 6
	};
	float X = 0;
	LB X += PolToFac[GenCredPol[Bank]];
	X *= RBanks;
	LB RelPol[Bank] = PolToFac[GenCredPol[Bank]] / X;
	{//Calculate Rel_Pol[Bank][TypeI] and Sum_Pol. 
		for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
		{
			float Sum_Pol[MaxCQ] =
			{
				0
			};
			int TypeI = intIndexOfCount;
			int MortFxd = TypeI == Fixed_rate;
			int NonMBs = 0;
			LB
			{	
				if( ! MortFxd ||  ! MortBanking[Bank])
				{;
					NonMBs++;
					for(int intIndexOfCount = 0; intIndexOfCount < MaxCQ; intIndexOfCount++)
						Sum_Pol[intIndexOfCount] += Rel_Pol[Bank][TypeI][intIndexOfCount];
				}
			}
			{
				for(int intIndexOfCount = 0; intIndexOfCount < MaxCQ; intIndexOfCount++)
				{
					float AvgPol =  ! NonMBs ? 0 : Sum_Pol[intIndexOfCount] / NonMBs;
					LB
					{
						float &Pol = Rel_Pol[Bank][TypeI][intIndexOfCount];
						
			            if(MortFxd && MortBanking[Bank])
						{
							Pol = 1.25;
							continue;
						}
						Pol =  ! AvgPol ? 1 : Pol / AvgPol;
					}
				}	
			}
		}
	}
	{//LNRR is the relative interest rate for the bank and divide by the community average relative rate
		long EconNum = (long)Env.Seq[EconSet_I - 1][SimQtr];
		for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
		{
			long TypeI = intIndexOfCount;
			float Sum[MaxCQ] =
			{
				0, 0, 0
			};
			LB
			{
				for(int intIndexOfCount = 0; intIndexOfCount < MaxCQ; intIndexOfCount++)
				Sum[intIndexOfCount] += LNRR[Bank][TypeI][intIndexOfCount] = CQ_rate[Bank][TypeI][intIndexOfCount] / CMR[Bank][TypeI][intIndexOfCount];
			}
			{
				for(int intIndexOfCount = 0; intIndexOfCount < MaxCQ; intIndexOfCount++)
				{
					float AR = EconNum == 10 ? Sum[intIndexOfCount] / NumBanks: (Sum[intIndexOfCount] + 10-NumBanks) / 10;
					LB LNRR[Bank][TypeI][intIndexOfCount] /= AR;
				}
			}
		}
	}
	{//All these relative factors determine the market share for each bank for each loan. CurInternalMkShare  
		float Them = 10-NumBanks, AllBanks[MaxCQ], Fac[11], F6;
		long ESOps[] =
		{
			0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2
		};
		for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
		{
			long TypeI = intIndexOfCount;
			for(int intIndexOfCount = 0; intIndexOfCount < MaxCQ; intIndexOfCount++)
			AllBanks[intIndexOfCount] = Them;
			LB
			{
				Fac[0] = Pow(EBDB[Bank][TypeI], LoanKonst.MkShare1[TypeI]);
				Fac[1] = Pow(ELOS[Bank][LnToSal[TypeI]], LoanKonst.MkShare2[TypeI]);
				Fac[2] = Pow(EPROC[Bank][ESOps[TypeI]], LoanKonst.MkShare3[TypeI]);
				Fac[3] = (Pow(BRCH[Bank][0], LoanKonst.MkShare4[TypeI][0]) + Pow(BRCH[Bank][1], LoanKonst.MkShare4[TypeI][1]) + Pow(BRCH[Bank][2], LoanKonst.MkShare4[TypeI][2]) + Pow(BRCH[Bank][3], LoanKonst.MkShare4[TypeI][3])) / 4; //Fac[3] = 1;
				Fac[4] = Pow(ORIG_FEE[Bank][TypeI], LoanKonst.MkShare5[TypeI]);
				{
					float X = LoanGlo[Bank].PlayersMS[TypeI];
					Fac[5] = X == 1 ? 1 : Pow(X *NumBanks / 2+.5, LoanKonst.MkShare6[TypeI]);
				}
				F6 = 1;
				memmove(LnFacs[Bank][TypeI] + 6, Fac, 6 *4);
				{
					for(int intIndexOfCount = 0; intIndexOfCount < 6; intIndexOfCount++)
						F6 *= Fac[intIndexOfCount];
				}
				{
					for(int intIndexOfCount = 0; intIndexOfCount < MaxCQ; intIndexOfCount++)
					{
						Fac[6] = Pow(ANN_FEE[Bank][TypeI], LoanKonst.MkShare7[TypeI][intIndexOfCount]);
						Fac[7] = Pow(RAMPShare[Bank][TypeI], LoanKonst.MkShare8[TypeI][intIndexOfCount]);
						Fac[8] = Pow(RelPol[Bank], LoanKonst.MkShare9[TypeI][intIndexOfCount]);
						Fac[9] = Pow(Rel_Pol[Bank][TypeI][intIndexOfCount], LoanKonst.MkShare10[TypeI][intIndexOfCount]);
						Fac[10] = Pow(LNRR[Bank][TypeI][intIndexOfCount], LoanKonst.MkShare11[TypeI][intIndexOfCount]);
						{
							float X = F6;
							{
								long i = 5;
								while(++i < 11)
									X *= Fac[i];
							}
							AllBanks[intIndexOfCount] += CurInternalMkShare[Bank][TypeI][intIndexOfCount] = X;
							{
								long j = intIndexOfCount;
								for(int intIndexOfCount = 0; intIndexOfCount < 5; intIndexOfCount++)
									LnFacs[Bank][TypeI][6+6+3 * intIndexOfCount + j] = Fac[6+intIndexOfCount];
							}
				            LnFacs[Bank][TypeI][intIndexOfCount] = X;
						}
					}
				}
			}
			float X = LoanKonst.RateLim[TypeI][0];
			float Y = LoanKonst.RateLim[TypeI][1];
			LB
			{
				{
					for(int intIndexOfCount = 0; intIndexOfCount < MaxCQ; intIndexOfCount++)
					{
						LnFacs[Bank][TypeI][intIndexOfCount + 3] = AllBanks[intIndexOfCount];
            //CurInternalMkShare[Bank][TypeI][intIndexOfCount] = .6 *CurInternalMkShare[Bank][TypeI][intIndexOfCount] / AllBanks[intIndexOfCount] + .4 *LoanGlo[Bank].InternalMkShare[TypeI][intIndexOfCount];
            //007-1  Loan market share change in weights  from .6 and .4 to  .75 and .25  (per memo of C. Haley  Jan, 2007)
						CurInternalMkShare[Bank][TypeI][intIndexOfCount] = .75 *CurInternalMkShare[Bank][TypeI][intIndexOfCount] / AllBanks[intIndexOfCount] + .25 *LoanGlo[Bank].InternalMkShare[TypeI][intIndexOfCount];
						if(LNRR[Bank][TypeI][intIndexOfCount] > X)
							CurInternalMkShare[Bank][TypeI][intIndexOfCount] = max(0, CurInternalMkShare[Bank][TypeI][intIndexOfCount]*(Y - LNRR[Bank][TypeI][intIndexOfCount]) / (Y - X));
						if(AnnFeeRatio[Bank][TypeI] > 2)
							CurInternalMkShare[Bank][TypeI][intIndexOfCount] = max(0, CurInternalMkShare[Bank][TypeI][intIndexOfCount]*(5-AnnFeeRatio[Bank][TypeI]) / (5-2));
						LoanGlo[Bank].InternalMkShare[TypeI][intIndexOfCount] = CurInternalMkShare[Bank][TypeI][intIndexOfCount];
					}
				}
			}
		}
	}	
}

/// <summary> This function calculates the proportal loan amount based on loan grade. </summary>
/// <remarks> It is called by loans(). The function takes MarketAmntByType[Type] and calculates MarketAmntByTypeByQual[Type][2]. 
/// It is in the design document section Credit Quality Breakout. </remarks>
void Quality()
{
  for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
  {
    float A = MarketAmntByType[intIndexOfCount];
		float Qual = EconBOQ.Econ.LMQI; //LMQI is loan market quality index
		float H = LoanKonst.Qual[intIndexOfCount].HProp;  //the normal proportions of high grade loans
		float HX = LoanKonst.Qual[intIndexOfCount].HExp; //the exponent (negative or zero) for high grade loans
		float M = LoanKonst.Qual[intIndexOfCount].MProp; //the normal proportions of median grade loans
		float MX = LoanKonst.Qual[intIndexOfCount].MExp; //the exponent (negative or zero) for median grade loans
    H *= Pow(Qual, HX);
    M *= Pow(Qual, MX);
    MarketAmntByTypeByQual[intIndexOfCount][0] = H * A;
    MarketAmntByTypeByQual[intIndexOfCount][1] = M * A;
    MarketAmntByTypeByQual[intIndexOfCount][2] = (1-(H + M)) *A;
  }
}

/// <summary> This function allocates various maturities given a maximum maturity and a loan type. </summary>
/// <param name="TypeI"> is the loan type. </param>
/// <param name="Mx"> is the maximum maturity. </param>
void MatDistrib(long TypeI, float Mx)
{
  float Z, ZZ, Cap[] =
  {
    40, 60, 30, 60, 20, 40, 20, 12, 100, 100, 120, 120, 120, 20, 120, 60
  };
  PRT *P = PR[Bank][TypeI];
  Mx = min(Cap[TypeI], Mx);
  if(TypeI == 7)
  {
    if(Mx < 3)
    {
      P[0].Mat = Mx;
      P[0].ProPor = 1;
      return ;
    }
    if(Mx < 6)
    {
      P[0].Mat = 2;
      P[0].ProPor = Z = ZMPf(TypeI, 2);
      P[1].Mat = Mx;
      P[1].ProPor = 1-Z;
      return ;
    }
    P[0].Mat = 2;
    P[0].ProPor = Z = ZMPf(TypeI, 2);
    P[1].Mat = 4;
    P[1].ProPor = (ZZ = ZMPf(TypeI, 4)) - Z;
    P[2].Mat = Mx;
    P[2].ProPor = 1-ZZ;
    return ;
  }
  if(Mx < 4)
  {
    P[0].Mat = Mx;
    P[0].ProPor = 1;
    return ;
  }
  if(TypeI == 10 || TypeI == 11)
  {
    if(Mx < 71)
    {
      P[0].Mat = Mx;
      P[0].ProPor = 1;
      return ;
    }
    P[0].Mat = 60;
    P[0].ProPor = Z = ZMPf(TypeI, 60);
    P[1].Mat = Mx;
    P[1].ProPor = 1-Z;
    return ;
  }
  if(IsClsII(TypeI) || IsClsIII(TypeI) || TypeI == 12)
  {
    if(Mx < 25)
    {
      P[0].Mat = Mx;
      P[0].ProPor = 1;
      return ;
    }
    P[0].Mat = 20;
    P[0].ProPor = Z = ZMPf(TypeI, 20);
    P[1].Mat = Mx;
    P[1].ProPor = 1-Z;
    return ;
  }
  if(Mx < 16)
  {
    P[0].Mat = 4;
    P[0].ProPor = Z = ZMPf(TypeI, 4);
    P[1].Mat = Mx;
    P[1].ProPor = 1-Z;
    return ;
  }
  P[0].Mat = 4;
  P[0].ProPor = Z = ZMPf(TypeI, 4);
  P[1].Mat = 12;
  P[1].ProPor = (ZZ = ZMPf(TypeI, 12)) - Z;
  P[2].Mat = Mx;
  P[2].ProPor = 1-ZZ;
}

// Determine a loan type belonging to which market.
int LMk[] =
{
  0, 0, 1, 1, 2, 2, 3, 4, 5, 6, 7, 7, 8, 8, 8, 8
};
//  0  National corporations
//  1  Middle market corporations
//  2  Small businesses
//  3  Importers/Exporters (trade finance)
//  4  Construction
//  5  Commercial real estate
//  6  Multi-family residential
//  7  1-4 Family residential
//  8  Consumers

/// <summary>This function loads the decisions into data structures in preperation for the market share logic. </summary>
/// <param name="TypeI"> is the loan type ID.</param>
/// <param name="D"> is the data structure for loan decision. </param>
/// <param name="A"> determines whether Adj uses D.AdjP or 0.  </param>
/// <remarks> It is called by Volume() inside Loans(). 
/// The function calculates NewLoans[Bank][Type], MarketAmntByMk[LMk[TypeI]], CMR[Bank][TypeI], RI[Bank][TypeI], AvER[TypeI], AvWan[TypeI]</remarks>
void LoanDecisions(long TypeI, ETA D, long A)
{
  NewLoansT *NwLn = NewLoans[Bank] + TypeI;
  long MB = MortBanking[Bank] && TypeI == 10;
  float V;
	float EAdj = Prices.LnFixPrices[TypeI].QRAP;
	float EMx = LoanKonst.Mat[TypeI];
	float Mx = MB ? EMx : D.MaxMat;
	float C, Y;
	float Adj = A ? D.AdjP : 0;
  ORIG_FEE[Bank][TypeI] = D.OrigFee;
  ZMP[Bank][TypeI] = ZMPf(TypeI, Mx);
  MatDistrib(TypeI, Mx);
  V = LoanKonst.Var[TypeI]; 
	//RAMP is the rate adjustment maturity preference.  
  RAMP[Bank][TypeI] = ZMP[Bank][TypeI]*(V + (1-V) *Adj / Mx);
  if( ! Bank)
    EconRAMP[TypeI] = ZMPf(TypeI, EMx)*(V + (1-V) *EAdj / EMx);
	//CQ_rates are Credit Quality interest rate.  
  CQ_rate[Bank][TypeI][0] = D.BaseRate;
  CQ_rate[Bank][TypeI][1] = (D.MaxRate + D.BaseRate) *.5;
  CQ_rate[Bank][TypeI][2] = D.MaxRate;
  NwLn->Base = D.BaseRate;
  NwLn->Spread = D.MaxRate - D.BaseRate;
  NwLn->AdjP = NwLn->Fee = D.AdjP;
  NwLn->OrigFee = D.OrigFee;
  MarketAmntByMk[LMk[TypeI]] += LoanRpt[Bank].Repaymnts[TypeI];
	//The market comparison rate(CMR) appropriate for a given bank depends on the bank's RAP and maximum maturity decisions. 
	//We ajust the market spread SM by a coefficient that varis by loan type and credit quality to calculate the CMR.  
  Y = YldC(Adj && Mx > Adj ? Adj : Mx, BoQ);
  C = Y + (EconBOQ.Econ.LoanCompRates[TypeI] - YldC(Adj ? EAdj : LoanKonst.Mat[TypeI], BoQ))*Pow(Mx / LoanKonst.Mat[TypeI], LoanKonst.CompRate[TypeI]);
  CMR[Bank][TypeI][0] = .97 * C - .2;
  CMR[Bank][TypeI][1] = C;
  CMR[Bank][TypeI][2] = 1.03 * C + .2;
  if(MB) //if mortgage banking
    memmove(CQ_rate[Bank][TypeI], CMR[Bank][TypeI], 4 *3);
  {
    float ER = D.BaseRate + LoanKonst.EfRate[TypeI]*(D.MaxRate - D.BaseRate); //ER is effective rate
    RI[Bank][TypeI] = Pow(ER / C, LoanKonst.ExpRate[TypeI]);
    AvER[TypeI] += ER;
  }
  AvWan[TypeI] += Wan[Bank][TypeI] = Pow(RAMP[Bank][TypeI], LoanKonst.Wanted[TypeI]) / RI[Bank][TypeI];
}

/// <summary> The function breaks out the loans in the market by credit quality--gross categories (quality grade).  </summary>
/// <remarks> It is called by Loans(). This function calculates the amount of available loans of each loan type for the community(MarketAmntByType[]) after 
/// converting the students loan decision from the screen to the internal data structures.  </remarks>
void Volume()
{
  memmove(MarketAmntByMk, EconBOQ.LowerEcon.PrGrowth, sizeof(MarketAmntByMk));
  memset(AvER, 0, sizeof(AvER)); //  AvER is the average midrange rate offered by the competing banks.
  memset(AvWan, 0, sizeof(AvWan));
  memset(PR, 0, sizeof(PR));
  LB
  {
    LoanDecisions(NC_CL, *(ETA*) &LoanDec[Bank].Nat, 0);
    LoanDecisions(NC_Term, LoanDec[Bank].NatTerm, 1);
    LoanDecisions(MM_CL, *(ETA*) &LoanDec[Bank].MidM, 0);
    LoanDecisions(MM_Term, LoanDec[Bank].MidMTerm, 1);
    LoanDecisions(SB_CL, *(ETA*) &LoanDec[Bank].SmallB, 0);
    LoanDecisions(SB_Term, LoanDec[Bank].SmallBTerm, 1);
    LoanDecisions(Trade, *(ETA*) &LoanDec[Bank].ImpExp, 0);
    LoanDecisions(Construction, *(ETA*) &LoanDec[Bank].Cons, 0);
    LoanDecisions(Commercial_RE, LoanDec[Bank].CommRe, 1);
    LoanDecisions(Multi_family, LoanDec[Bank].MulFam, 1);
    LoanDecisions(Fixed_rate, *(ETA*) &LoanDec[Bank].SingFam, 0);
    LoanDecisions(Variable_rate, LoanDec[Bank].SingFamVar, 1);
    LoanDecisions(Home_equity, *(ETA*) &LoanDec[Bank].HomeEqu, 0);
    LoanDecisions(Personal, LoanDec[Bank].Per, 1);
    LoanDecisions(Credit_card, *(ETA*) &LoanDec[Bank].CCard, 0);
    LoanDecisions(Installment, *(ETA*) &LoanDec[Bank].Instal, 0);
  }
  {
    static float SMk[MaxM];
    memset(SMk, 0, sizeof(SMk));
    {//Calculates AvWan, AvER and SMK. 
      float *Prop = LoanKonst.NormProp - 1;
      for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
      {
        Prop++;
        AvWan[intIndexOfCount] *= RBanks; 
        AvER[intIndexOfCount] *= RBanks;
        SMk[LMk[intIndexOfCount]] +=  *Prop * AvWan[intIndexOfCount];
      }
    }
    {//Calculate MarketAmntByType. 
      float *Prop = LoanKonst.NormProp - 1;
			float *Sub = LoanKonst.Subst - 1;
      for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
      {
        Prop++;
        Sub++;
        {
          float P;
					float D = SMk[LMk[intIndexOfCount]];
					float Wan =  ! D ? 0 : AvWan[intIndexOfCount] / D;
          P = (1- *Sub) **Prop +  *Sub **Prop * Wan;
          MarketAmntByType[intIndexOfCount] = MarketAmntByMk[LMk[intIndexOfCount]] *P;
        }
      }
    }
  }
  float Shift = LoanKonst.SBTorCPL * MarketAmntByType[SB_Term]*(AvER[SB_Term] - AvER[Personal]) / AvER[SB_Term];

  MarketAmntByType[SB_Term] += Shift;
  MarketAmntByType[Personal] -= Shift;
}

/// <summary> This function calculates the prepayments made on loans and reduce the loan amount and change the UsageRate for certain loan type. </summary> 
/// <remarks> It is called by Loans(). It calculates LoanRpt[Bank].Repaymnts[TypeI]. </remarks>
void Prepayments()
{
  for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
  {
    int TypeI = intIndexOfCount;
    float Pa = LoanKonst.PrePay[TypeI].a;
		float Pb = LoanKonst.PrePay[TypeI].b;
		float Pc = LoanKonst.PrePay[TypeI].c;
		float Pd = LoanKonst.PrePay[TypeI].d;
		float Pp = LoanKonst.PrePay[TypeI].p;
		float Pv = LoanKonst.PrePay[TypeI].v;
		float PrePay = EconBOQ.Econ.ECI_Nat;
		float Upper = Pc + Pd * PrePay;
		float Lower = Pa + Pb * PrePay;
    LoanT *p = pLoans[Bank][TypeI] - 1;
    long RecCnt = (long)LoanGlo[Bank].LnCnt[TypeI];
    for(int intIndexOfCount = 0; intIndexOfCount < RecCnt; intIndexOfCount++)
    {
      p++;
      if(p->Amnt <= 0)
        continue;
      {
        float A = p->Amnt;
				float P = p->PastDue;
				float M = p->Mat;
				float R = p->Rate *.0025;
				float AM =  ! IsAdjP(TypeI) ?  - 1: p->AdjMat;
				float CompR = EconBOQ.Econ.LoanCompRates[TypeI] *.0025; //Competitive rates
				float Mat = AM ==  - 1 ? M : min(M, AM);
				float x = (1-Pow(1+R,  - Mat))*(R - CompR) / R;
				float Por = M == 1 || AM == 1 ? Lower: Bound(Pp *(x - Pv), Lower, Upper);
				//BugFix 06/12/06 by Jeff2: removing x.0025 because compR has been multiplied .0025 above. 
        //if(p->Reported >= 8 || P == 1 || CompR *.0025 >= R)  
				if(p->Reported >= 8 || P == 1 || CompR >= R)  
          continue;
        if((IsClsII(TypeI) || IsClsIII(TypeI)) && TypeI != 5)  //The entire loan will be paid off if customers choose to prepay.   
        {
          if(p->Id > 1000 && Por > Rand())  //Whether the customer decides to prepay the loan is a random factor in the simulation. 
          {
            LoanRpt[Bank].Repaymnts[TypeI] += A;
            memset(p, 0, LnSz);
          }
          continue;
        }
        if(IsClsI(TypeI) || IsClsVI(TypeI))
        {
          A *= Por;
          p->Amnt -= A;
          LoanRpt[Bank].Repaymnts[TypeI] += A;
          p->UsageRate = p->Amnt / p->Commit;
          continue;
        }
        { //when TypeI = 5,10,11,13,15
          float PD = P * A;
          {
            float Amnt = Por *(A - PD);
            LoanRpt[Bank].Repaymnts[TypeI] += Amnt;
            A -= Amnt;
          }
          if(A <= 0) //Question: it may never happen that A <= 0
          {
            memset(p, 0, LnSz);
            continue;
          }
          p->Amnt = A;
          p->PastDue = PD / A;
        }
      }
    }
  }
}

/// <summary> This function calculates the repayment of loans, annual fee, draw for loan reports depending on the loan type and maturity.  
/// It also calculate interest cash by calling InterestCash(). </summary>
/// <remarks> It is called by Loans(). </remarks>
void Repayments()
{
  LoanRptT *LnR = LoanRpt + Bank;
  for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
  {
    long TypeI = intIndexOfCount;
    LoanT *p = pLoans[Bank][TypeI] - 1;
    long RecCnt = (long)LoanGlo[Bank].LnCnt[TypeI];
    for(int intIndexOfCount = 0; intIndexOfCount < RecCnt; intIndexOfCount++)
    {
      p++;
      if(p->Amnt <= 0 || p->Reported >= 8)
        continue;
      {
		long II_III = IsClsII(TypeI) || IsClsIII(TypeI);
		long M = (long)p->Mat;
		long AdjR = M > 0 &&  ! p->AdjMat;
		long AP = (long)p->AdjP;
        float A = p->Amnt;
		float Commit = p->Commit;
		float P = p->PastDue;
		float S = p->Spread;
		float PrevR = p->Rate;

		float NewR = p->Rate = IsClsI(TypeI) ? EconBOQ.Econ.Prime + S: (II_III && AdjR ? YldC(AP, BoQ) + S: (TypeI == 11 || IsClsVI(TypeI) || TypeI == 13 ? S + EconBOQ.Econ.One_Qtr_Treas: PrevR));
		float R = NewR * .0025;
		float I = A * R;  //interest
		float F =  ! IsClsI(TypeI) ? 0 : p->AnnFee *.0025;
        
		if(II_III && AdjR)
          p->AdjMat = min(M, AP);
        if(M < 1)
        {
          if(P == 1)  //if it is past due.  
          {
            p->Commit = (p->Amnt += I);
            if(Commit)
              LnR->MatCommit[LnToCL[TypeI]] += Commit - p->Commit;
            p->UsageRate = p->Amnt / p->Commit;
            LnR->Draws[TypeI] += I;
            continue;
          }
					//if it is not past due
          LnR->AnnFees[TypeI] += (F *= .5);
          LnR->Repaymnts[TypeI] += A;
          InterestCash(TypeI, I *= .5);
          if(Commit)
            LnR->MatCommit[LnToCL[TypeI]] += Commit;
          memset(p, 0, LnSz);
          continue;
        }
				// M >= 1
        if(IsClsI(TypeI))
        {
					//Type 7 is a contruction loan
          float E = TypeI == 7 ? A + (p->Commit - A) / M: p->Commit *p->UsageRate *EconBOQ.Econ.ECI_Nat*LoanKonst.CurUsage[TypeI];
					float D = E-A;
          LnR->AnnFees[TypeI] += F * .5 *(E+A);
          LnR->Draws[TypeI] += D;
          InterestCash(TypeI, I = (E+A) *.5 * R);
          A = p->Amnt = E;
          p->UsageRate = A / p->Commit;
          continue;
        }
        if(GroupA(TypeI))
        {
          float P = A - p->Payment < LoanKonst.MinKeepBal ? A : p->Payment, E = A - P;
          LnR->Repaymnts[TypeI] += P;
          InterestCash(TypeI, I = (E+A) *.5 * R);
          if( ! (p->Amnt = E))
            memset(p, 0, LnSz);
          continue;
        }
        // Class B
        if(TypeI == 13)
        {
          InterestCash(TypeI, I);
          continue;
        }
        if(IsClsVI(TypeI))
        {
          long CreditCard = TypeI == 14;
          float P = p->Payment;
					float Amnt = A - P;
					float E = p->Commit *p->UsageRate *EconBOQ.Econ.ECI_Local * LoanKonst.CurUsage[TypeI];
					float D = E-Amnt;
					float TT = LoanKonst.TransBal;
					float TP = TT * E+(D > 0 ? (1-TT) *D: 0);
					float Fee = CreditCard ? TP *LoanKonst.MerDis: F;
					float AvgBal = (E+A) *.5;
          Fee += AvgBal * .001 * p->AnnFee / (LoanKonst.LoanSize[TypeI] *4000);
          LnR->Draws[TypeI] += D;
          LnR->AnnFees[TypeI] += Fee;
          InterestCash(TypeI, I = AvgBal * R *(CreditCard ? (1-TT): 1));
          LnR->Repaymnts[TypeI] += P;
          if(P)
            p->Payment = E / M;
          if( ! (p->Amnt = E))
            memset(p, 0, LnSz);
          continue;
        }
        //[11,16; IV; IsClsIV] And [12; V; (Var Mort)] and 5 (Base 0)
        {
          float TotPay = p->Payment *(1-P);
					float PrinPay = (TotPay - I) / (1-R * .5);
					float PrinPay2 = (A - PrinPay < LoanKonst.MinKeepBal) ? A : PrinPay;
					float E = A - PrinPay2;
          if(TotPay <= PrinPay2 *(1+R))
            I = PrinPay2 * R / 2;
          LnR->Repaymnts[TypeI] += PrinPay2;
          InterestCash(TypeI, I);
          if(TypeI == 11)
            p->Payment = R * E / (1-Pow(1+R,  - M));
          if( ! (p->Amnt = E))
            memset(p, 0, LnSz);
        }
      }
    }
  }
}

/// <summary>This function calculates ChrgOffAmnt and Repaymnts for loan reports and adjusts the credit quality, maturity and rate.</summary>
/// <remarks> It is called inside Loans(). 
/// Loans in group A can be reviewed individually and loans in group B has to be bundled together such as family and small business. </remarks>
//void NonPerforming()
//{
//	extern long L4[5];
//
//  LoanRptT *LnR = LoanRpt + Bank;
//  LoanGloT *LnG = LoanGlo + Bank;
//  float NATtemp[MaxL];
//	float CORtemp[MaxL];
//  memset(NATtemp, 0, sizeof(NATtemp));
//  memset(CORtemp, 0, sizeof(CORtemp));
//  {
//    for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
//    {//Run only if loan type = 4,5,6 or >= 10
//			//It sets LoanGlo.ChrgOffAmnt, LoanGlo.Repaymnts
//      if(GroupA(intIndexOfCount))
//        continue;
//      {
//        //float PCO;
//		float NAT = LnG->NAT[intIndexOfCount]; //NAT is None Accrual Total (credit quality is 8). George is not sure about this. 
//		float ES = ResGlo[Bank].Eff.Sal.Loans[LnToSal[intIndexOfCount]];  //ES is effective salary. 
//        float PCO = LnG->CORatio[intIndexOfCount] * BOA[Bank][L4[1]].Bal.Assets.Loans.Loans[intIndexOfCount]; //PCO is percentage of Charge Off. 
//        if(ES >= 1)//enough staff
//        {
//          float CGoff = min(PCO, NAT);
//          LnG->CORatio[intIndexOfCount] = 0;
//					LnG->NAT[intIndexOfCount] = 0;
//          NAT -= CGoff;
//          LnR->ChrgOffAmnt[intIndexOfCount] += CGoff;
//          LnR->Repaymnts[intIndexOfCount] += NAT;
//        }
//        else //understaffed. 
//        {
//          float CGoff = min(ES * PCO, NAT);
//          LnG->NAT[intIndexOfCount] = PCO > NAT ? 0 : FloorZ(PCO - CGoff);
//          LnR->ChrgOffAmnt[intIndexOfCount] += CGoff;
//          LnR->Repaymnts[intIndexOfCount] += FloorZ(NAT - PCO);
//        }
//        NATtemp[intIndexOfCount] += LnG->NAT[intIndexOfCount];
//        CORtemp[intIndexOfCount] += LnG->NAT[intIndexOfCount] *LnG->CORatio[intIndexOfCount];
//      }
//    }
//  }
//  {
//    for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
//    {
//      long LnT = intIndexOfCount;
//			long GrpA = GroupA(LnT);
//      float R, PP, COPow, Ac;
//			float Ca = LoanKonst.ChrgOffPor[LnT].a;
//			float Cb = LoanKonst.ChrgOffPor[LnT].b;
//			float Cc = LoanKonst.ChrgOffPor[LnT].c;
//			float Cd = LoanKonst.ChrgOffPor[LnT].d;
//			float Pa = LoanKonst.PastDuePor[LnT].a;
//			float Pb = LoanKonst.PastDuePor[LnT].b;
//			float Pc = LoanKonst.PastDuePor[LnT].c;
//			float Pd = LoanKonst.PastDuePor[LnT].d;
//			float Na = LoanKonst.NonAccPor[LnT].a;
//			float Nb = LoanKonst.NonAccPor[LnT].b;
//			float ES = ResGlo[Bank].Eff.Sal.Loans[LnToSal[LnT]];
//			float ChrgOffPor, PastDuePor, NonAccPor;
//      LoanT *p = pLoans[Bank][LnT] - 1;
//      long RecCnt = (long)LnG->LnCnt[LnT];
//      for(int intIndexOfCount = 0; intIndexOfCount < RecCnt; intIndexOfCount++)
//      {
//        p++;
//        if(p->Amnt <= 0)
//          continue;
//        COPow = Cb * Pow(EconBOQ.Econ.LLI, Ca);
//        Ac = p->Actual;
//        ChrgOffPor = COPow * (Pow(Cc, Ac) - Cd * Ac);
//        PastDuePor = Pa + Pb * ChrgOffPor + Pc * ( ! p->Mat ? 1 : Pow(p->Mat, Pd));
//        NonAccPor = Na * (PastDuePor - Pa) * (1-Nb + Nb / ES);
//        R = Rand();
//        if(GrpA)
//        {
//          if(p->Id > 1000)
//            continue;
//          if(p->Reported == 8)
//          {
//            if(p->Pkg ==  - 1)
//            {
//              p->Pkg = 0;
//              continue;
//            }
//            PP = LoanKonst.PP[LnT] * ChrgOffPor;
//            if(PP > R)  //Loan is OK. 
//            {
//              Ac++;
//              p->PastDue = 0;
//              p->Actual = Ac = Ac > 7.5 ? 7.5 : Ac;
//              p->Reported = Round(Ac);
//              continue;
//            }
//            if((PP *= .5) > R)  //Reschduling the loan
//            {
//              LoanT *q = p; // Big-Loan Work Out; No Reporting
//              q->Mat = LnT == 7 ? 20 : q->Mat + 8;
//              q->PastDue = 0;
//              if(IsClsI(LnT))
//              {
//                q = Alloc_A_New_Loan_Rec((long)q->Type++);
//                memmove(q, p, LnSz);
//                q->Pkg =  - 1;
//                memset(p, 0, LnSz);
//              }
//              q->Rate = YldC(q->Mat, BoQ) + 2.5;
//              q->AdjP = q->Mat; //It changes to a fixed rate loan by assign maturity to the rate ajusted period.  
//              Ac += 2;
//              q->Actual = Ac = min(7.5, Ac);
//              q->Reported = Round(Ac);
//              continue;
//            }
//            // A Charge Off loan, which the bank may get the half of amount back.   
//            if(1 < ES || R > PP * ES)
//            {
//              float A = p->Amnt * .5;
//              LnR->Repaymnts[LnT] += A;
//              LnR->ChrgOffAmnt[LnT] += A;
//              memset(p, 0, LnSz);
//              continue;
//            }
//            // Understaffed
//            p->Actual = 9;
//            p->Reported = 8;
//            continue;
//          }
//          if( ! p->PastDue)
//          {
//            p->PastDue = R <= PastDuePor;
//            continue;
//          }
//          if(R <= NonAccPor)
//            p->Reported = 8;
//          else
//            p->PastDue = 0;
//          continue;
//        }
//        // Group B Types 11-16 (10-15)
//        {
//          float NAP = min(.9 *p->PastDue, NonAccPor), NA = p->Amnt *NAP;
//          p->PastDue = PastDuePor;
//          p->Amnt -= NA;
//          NATtemp[LnT] += NA;
//          CORtemp[LnT] += NA * ChrgOffPor;
//        }
//      }
//    }
//  }
//  {
//    for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
//    {
//      if(GroupA(intIndexOfCount))
//        continue;
//      LnG->NAT[intIndexOfCount] = NATtemp[intIndexOfCount];
//      LnG->CORatio[intIndexOfCount] = CORtemp[intIndexOfCount];
//      if(NATtemp[intIndexOfCount])
//        LnG->CORatio[intIndexOfCount] /= NATtemp[intIndexOfCount];
//    }
//  }
//}
void NonPerforming()
{
	extern long L4[5];
	extern ECNT EconEOQ; 

  LoanRptT *LnR = LoanRpt + Bank;
  LoanGloT *LnG = LoanGlo + Bank;
  float NATtemp[MaxL];
	float CORtemp[MaxL];
  memset(NATtemp, 0, sizeof(NATtemp));
  memset(CORtemp, 0, sizeof(CORtemp));
	float RNum, PP, COPow, ActCQ, RptCQ, PDue;	// Revised was 02/05/2012: Added 'PDue' so it could be watched in debug
	float ChrgOffPor, PastDuePor, NonAccPor;
	// Constant - the value of Nonaccrual credit quality  = 8.0 (This shopuld be at top of function - never changes)
  float NonAccrual = 8.0;
 	{
		// Section I  Nonaccruals and Charge-offs of ClassB loans
		for(int idLnT = 0; idLnT < MaxL; idLnT++)
		{
			if(GroupA(idLnT))
				continue;
			
			float NAT = LnG->NAT[idLnT]; //NAT is None Accrual Total (credit quality is 8).
			float ES = ResGlo[Bank].Eff.Sal.Loans[LnToSal[idLnT]]; //ES is effective salary. 
			float PCO = LnG->CORatio[idLnT] *	BOA[Bank][L4[1]].Bal.Assets.Loans.Loans[idLnT];  //PCO is calculated Charge Off.

			if(ES >= 1)//enough staff
      {
				float CGoff = min(PCO, NAT);          
        NAT -= CGoff;
        LnR->ChrgOffAmnt[idLnT] += CGoff;
        LnR->Repaymnts[idLnT] += NAT;
      }
      else //understaffed. 
      {
        float CGoff = min(ES * PCO, NAT);
        LnG->NAT[idLnT] = PCO > NAT ? 0 : FloorZ(PCO - CGoff);
        LnR->ChrgOffAmnt[idLnT] += CGoff;
        LnR->Repaymnts[idLnT] += FloorZ(NAT - PCO);
      }
      
		  LnG->CORatio[idLnT] = 0;
			LnG->NAT[idLnT] = 0;
		}
		// End Section I  Nonperforming() loans function
		
		// Section II
		for(int idLnT = 0; idLnT < MaxL; idLnT++)
    {
      //Loan constants for charge-off and past due
			float KCa = LoanKonst.ChrgOffPor[idLnT].a;
			float KCb = LoanKonst.ChrgOffPor[idLnT].b;
			float KCc = LoanKonst.ChrgOffPor[idLnT].c;
			float KCd = LoanKonst.ChrgOffPor[idLnT].d;
			float KPa = LoanKonst.PastDuePor[idLnT].a;
			float KPb = LoanKonst.PastDuePor[idLnT].b;
			float KPc = LoanKonst.PastDuePor[idLnT].c;
			float KPd = LoanKonst.PastDuePor[idLnT].d;
			float KNa = LoanKonst.NonAccPor[idLnT].a;
			float KNb = LoanKonst.NonAccPor[idLnT].b;

			// George: temporary patch in May 2009 due to the error in SF.Dat 
			if(idLnT == 9)
			{
				KCb = 0.00007;
			}

			float ES = ResGlo[Bank].Eff.Sal.Loans[LnToSal[idLnT]];
			
      LoanT *p = pLoans[Bank][idLnT] - 1;
      long RecCnt = (long)LnG->LnCnt[idLnT];

			long GrpA = GroupA(idLnT);
			
			//   For each loan record in the loan type
			for(int intIndexOfCount = 0; intIndexOfCount < RecCnt; intIndexOfCount++)
			{
				p++;
				//Credit qualities in the loan record
				ActCQ = p->Actual;
				RptCQ = p->Reported;
				PDue = p->PastDue;	// Revised wes 02/05/2012: Added 'PDue' so it could be watched in debug

				// If there is no balance left in record do not process and 
				//We do not calculate any of the loan loss and nonperforming loans information if their ID is less than 1000 in ClassA
				if(p->Amnt <= 0 || p->Id < 1000)
					continue;

				// Portion values for Charge-offs, Past-Due & Nonaccrual for loan record
				//COPow = KCb * Pow(EconBOQ.Econ.LNP, KCa);
				COPow = KCb * Pow(EconEOQ.Econ.LNP, KCa);
				//COPow = KCb * Pow(1.15, KCa);	// Revised wes 01/19/2012: Can't find where LNP is set or calculated, but exists in original Colorado 2011 runs
				//COPow = KCb * Pow(1, KCa);	// Revised wes 02/05/2012: Changed again to neutralize LNP
				ChrgOffPor = COPow * (Pow(KCc, ActCQ) - KCd * ActCQ);
				PastDuePor = KPa + KPb * ChrgOffPor + KPc * ( ! p->Mat ? 1 : Pow(p->Mat, KPd));
				NonAccPor = KNa * (PastDuePor - KPa) * (1-KNb + KNb / ES);
				PP = LoanKonst.PP[idLnT] * ChrgOffPor;

				// Generate a random number Rnum
				RNum = Rand(); 
					
				if(GrpA)
				{
					
					// First a little housekeeping:
					// Set t0 -1when a loan was rescheduled in prior quarter. Reset to normal status for future
					if(p->Pkg == -1) 
					{
								p->Pkg = 0;  // now 
								continue;
					}
					/////////////////////////////Processing each   GroupA loan record//////////////
					//	
					//	Processing alternatives:
					//      1a.  A performing loan goes into past due status when RNum < PP (ActCQ and RptCQ set to 7.0  PastDue = 1
					//	    1b.  A performing loan with no change in status when RNum >= PP (ActCQ < 7.0  RptCQ <= 6 no change in status
					//	  
					//	   2    A loan with ActCQ = 7.0 to 7.9 as set in MatureAdjEcon() Set RptCQ = 7.0 Past-due = 1
					//	   3    A past-due comes into qtr RptCQ and ActCQ = 7.0  Past-due = 1.0 
					//              Determine whether loan returns to performing status or as nonaccrual. ActCQ and RptCQ = 8.0
					//     4.   A non accrual loan comes into quarter with ActCQ and RptCQ = 8.0 Past-due = 0
					//              4 outcomes possible
					//
					//////////////////////////////////////////////////////////////////////////////////////////////////	
	
				  ////1a.  and 1b   
					if(p->Reported <= 6.0 && p->Actual < 7.0)
					{
						if (RNum < PP)
						{
							p->PastDue = 1.0;          
							p->Actual = 7.0;
							p->Reported = 7.0;
							continue;
		        }
					}
					////2
					if(p->Actual >= 7.0 && p->Actual < 8.0 && abs(p->PastDue) < 0.00001)
					{
						p->PastDue = 1.0;          
						p->Reported = 7.0;
						p->Actual = max(7.5, p->Actual);  //set this at the maximum of 7.5 or the current value of Actual which might already be at 7.7 or 7.8 or 7.9
						continue;
					}
					// 3    
					if(abs(p->PastDue - 1.0) < 0.00001)
					{
						//if(p->Actual < 7.0)
						if(p->Actual < 6.8)	// Revised wes 02/05/2012: Force small portion of relatively bad CQ loans to nonaccrual test
						{
							p->PastDue = 0.0;
							p->Actual = 6.5;
							p->Reported = 6.0;
						}
						else
						{
							//if (RNum < PP)    //  goes to non-accrual status otherwise will return to performing
							//if (RNum > PP)	// Revised wes 01/18/2012: Original produces tiny portion of an already tiny portion
							if (RNum < PP * 3.0)	// Revised wes 02/06/2012: Change back to original but push more to nonaccrual
							{	
								p->PastDue = 0.0;          
								p->Actual = 8.0;
								p->Reported = 8.0;
							}
							else
							{
								p->PastDue = 0.0;          
								p->Actual = 6.5;
								p->Reported = 6.0;
							}
						}
						continue;
					}

					// 4 Nonaccrual loans
					if(abs(RptCQ - NonAccrual) < 0.0001)
					{            
						// Program cannot reach this point without RptCQ 8.0
						// Outcome 1  Return to peforming status  -  small chance
			            //if(PP > RNum)
						if(RNum < PP)	// Revised wes 02/05/2012: Maintain comparison sequence already established
					    {  
							p->PastDue = 0;
							p->Actual = 6.0;
							p->Reported = 6.0;
							continue;  //done with loan record
					    }
						// Outcome 2 Reschduling the loan if PP *=1.5  > Rnd  we will make a new loan arrangement
						if(idLnT == 0 || idLnT == 2 || idLnT == 7)
						{
							//if(PP *= 1.5 > RNum)
							if(RNum < (PP *= 1.5))	// Revised wes 02/05/2012: Maintain comparison sequence and CORRECT CALCULATION INTENT
							{
								LoanT *q = p; // Big-Loan Work Out; No Reporting
								q->Mat = idLnT == 7 ? 20 : q->Mat + 8;
								q->PastDue = 0;

								q = Alloc_A_New_Loan_Rec((long)q->Type++);
								memmove(q, p, LnSz);
								q->Pkg =  - 1;
								memset(p, 0, LnSz);
								
								q->Rate = YldC(q->Mat, BoQ) + 2.5;
								q->AdjP = q->Mat; //changes tofixed rate loan by maturity 							// = to the rate ajusted period.  
								ActCQ += 2;
								q->Actual = ActCQ = min(6.0, ActCQ);
								q->Reported = Round(ActCQ);
								continue;  // to next loan record
							}
						}
			            // Outcome 3a A Charge Off loan, Charge Off with half of amount paid. Clear out loan record .  Continue to next loan record  
						if(1 < ES || RNum > PP * ES)
						{
							//float A = p->Amnt * .5;
							float A = p->Amnt * .4;		// Revised wes 02/05/2012: With good credit risk mgmt, C/O s/b less, say, 40%
							float Ap = p->Amnt * .6;	// Revised wes 02/05/2012: Repayment w/b 60%
							// LnR->Repaymnts[idLnT] += A;
							LnR->Repaymnts[idLnT] += Ap;				// Revised wes 02/05/2012: Use new 'Ap' variable
							LnR->ChrgOffAmnt[idLnT] += A;
							memset(p, 0, LnSz);
							continue;
						}
						// Outcome 3b  Entirely written off.
						float A = p->Amnt * .8;		// Revised wes 02/05/2012: 100% C/O on large loans is rare; C/O s/b less, say, 80%
						float Ap = p->Amnt * .2;	// Revised wes 02/05/2012: Repayment w/b 20%
						LnR->Repaymnts[idLnT] += Ap;				// Revised wes 02/05/2012: Use new 'Ap' variable
						//LnR->ChrgOffAmnt[idLnT] += p->Amnt;
						LnR->ChrgOffAmnt[idLnT] += A;	// Revised wes 02/05/2012: Use 'A' variable
						memset(p, 0, LnSz);
						continue;
					}
				}
				else //GroupB loan types
				{							
					float NAP = min(.9 *p->PastDue, NonAccPor);
					float NA = p->Amnt *NAP;
					p->PastDue = PastDuePor;
					NATtemp[idLnT] += NA;
					CORtemp[idLnT] += NA * ChrgOffPor;
				}
			}
		}

		//Section III  Values for GroupB next quarter after all loan types have been processed.	
		for(int idLnT = 0; idLnT < MaxL; idLnT++)
    {
      if(GroupA(idLnT))
        continue;
      LnG->NAT[idLnT] = NATtemp[idLnT];
      LnG->CORatio[idLnT] = CORtemp[idLnT];
      if(NATtemp[idLnT])
        LnG->CORatio[idLnT] /= NATtemp[idLnT];
    }
	}
}

// Updates the reported quality of loans
//Remove: using procedure ChngRep() instead.
//#define ChngRep( Rep )  if ( Rep < p->Reported ) LnR->RepUpGr[TypeI]  += Amnt;  \
//if ( Rep > p->Reported ) LnR->RepDwnGr[TypeI] += Amnt;   p->Reported = Rep;
/// <summary> This function add the loan amount to the total amount based on upgrade or downgrade. </summary>
/// <param name="Rep"> is actual credit quality. </param>
/// <param name="TypeI"> is the loan type. </param>
/// <param name="**p"> is the pointer to LoanT />. </param>
/// <param name="**LnR"> is the pointer to LoanRptT />. </param>
/// <param name="Amnt"> is the loan amount. </param>
/// <remarks> The loan report will have the total upgraded loan amount in RepUpGr[TypeI] and downgraded loan amount in RepDwnGr[TypeI].
/// Upgrade means the credit quality gets better and downgrade means the credit quality get worse. </remarks> 
void ChngRep(int Rep, int TypeI, LoanT **p, LoanRptT **LnR, float Amnt) {

	if ( Rep < (*p)->Reported ) 
		(*LnR)->RepUpGr[TypeI]  += Amnt;
	
	if ( Rep > (*p)->Reported ) 
		(*LnR)->RepDwnGr[TypeI] += Amnt;
	
	(*p)->Reported = Rep;
}

/// <summary> This function does a loan review based on a loan type. </summary>
/// <param name="TypeI"> is the loan type. </param>
/// <remarks> This function calculutes LnR->RevCost[TypeI] (Review Cost) and 
/// calculates the loan amount for Upgrade or Downgrade loan credit quality by calling ChngRep().  </remarks>  
// Jeff2 rewrites continue statement to else if statement in this function. 
void Review(int TypeI)
{
  LoanRptT *LnR = LoanRpt + Bank;
  LoanT *p = pLoans[Bank][TypeI] - 1;
  AcT *Ac = BOA[Bank] + YrsQtr;
  IncomeOutGoT *Incm = &Ac->IncomeOutGo;
  float Amnt;
	float SAmnt = 0;
	float SWeight = 0;
  long RecCnt = (long)LoanGlo[Bank].LnCnt[TypeI];
  for(int intIndexOfCount = 0; intIndexOfCount < RecCnt; intIndexOfCount++)
  {
    p++;
    if(p->Amnt <= 0)
      continue;
    SAmnt += (Amnt = p->Amnt);
    SWeight += fabs(p->Actual - p->Reported) *Amnt;
    if(p->Actual == 8)  // 8 is non-performing loan
    {
			ChngRep(8, TypeI, &p, &LnR, Amnt);
    }
    else if(p->Actual == 9)  // 9 is charge-off loan after non-performing.  
    {
      LnR->RevChrgOff[TypeI] += Amnt;
      LnR->ChrgOffAmnt[TypeI] += Amnt;
      memset(p, 0, LnSz);
    }
    else if(p->Actual >= 6.5) // >6.5 is risky loan. 
    {
			ChngRep(7, TypeI, &p, &LnR, Amnt);
    }
		else
		{
			ChngRep(Round(p->Actual), TypeI, &p, &LnR, Amnt);
		}
  }
  Incm->Expenses.OtherOpEx += LnR->RevCost[TypeI] = LoanKonst.RevCA + LoanKonst.RevCB *SWeight + LoanKonst.RevCC * SAmnt;
}

/// <summary> This function does a loan review based on a loan type and market size. </summary>
/// <param name="DoRevF"> is 1 to run the loan review.  If it is 0, it will not run Review(). </param>
/// <remarks> Whether to review the loan is determined by the user input from the Credit Administration Decisions form.  
/// This function is not well designed originally.  There is no reason to enter this function if DoRevF is 0. </remarks> 
void RevMk(float DoRevF)
{
  static int LnType =  - 1, Mk =  - 1;
  int LRev[] =
  {
    2, 2, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1
  };
  int MkSize = (int)LRev[Mk = ++Mk % MaxC];
	int DoRev = (int)DoRevF;
  for(int intIndexOfCount = 0; intIndexOfCount < MkSize; intIndexOfCount++)
  {
    LnType = ++LnType % MaxL;
    if(DoRev)
      Review(LnType);
  }
}

/// <summary>This function goes through each loan type to calculate review cost and loan amount of upgrade or downgrade credit quality 
/// if it requires a loan review (DoRev is 1) from Credit Administration Decision form. </summary>
/// <remarks> It is called by Loans(). 
/// This function reviews each type of loan in Cred data structure based on DoRev field.</remarks>
void Reviews()
{
  CredDecT Cred = CredDec[Bank];
  RevMk(Cred.Nat.DoRev);
  RevMk(Cred.MidM.DoRev);
  RevMk(Cred.SmallB.DoRev);
  RevMk(Cred.ImpExp.DoRev);
  RevMk(Cred.Cons.DoRev);
  RevMk(Cred.CommRe.DoRev);
  RevMk(Cred.MulFam.DoRev);
  RevMk(Cred.SingFam.DoRev);
  RevMk(Cred.HomeEqu.DoRev);
  RevMk(Cred.Personal.DoRev);
  RevMk(Cred.CredC.DoRev);
  RevMk(Cred.Instal.DoRev);
}

/// <summary> This function goes through every loan to decrement their maturities and to add the influence of the economy on their credit qualities. </summary>
/// <remarks> It adjusts pLoans[Bank][TypeI]->Mat (maturity), pLoans[Bank][TypeI]->AdjMat for certain loan type with adjusted maturity and pLoans[Bank][TypeI]->Actual (actual credit quality). </remarks>
void MatureEconAdj()
{
  for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
  {
    int TypeI = intIndexOfCount;
    LoanT *p = pLoans[Bank][TypeI] - 1;
		//LLI is the Loan Loss Index, which is used to determine the past due (PLINQUAL)
    float X = LoanKonst.LNFCQI[TypeI]*(EconBOQ.Econ.LLI - EconBOQ.LowerEcon.PLNQUAL); 
    long RecCnt = (long)LoanGlo[Bank].LnCnt[TypeI];
    {
      for(int intIndexOfCount = 0; intIndexOfCount < RecCnt; intIndexOfCount++)
      {
        p++;
		//After the change of NonPerformance() logic, one issue is zero Maturity Past Due Loans remaining in loan files.
		//George suggest that we deal with Maturity == 0 here
		if(p->Mat == 0 && abs(p->PastDue - 1.0) < 0.0001)
		{
			p->Actual = 8.0;
			p->Reported = 8.0;
			p->PastDue = 0;
			continue;
		}
        if(p->Mat < 1)
          continue;
        --p->Mat; //decrement the loan maturities
        if(IsAdjP(TypeI))
          p->AdjMat = min(p->Mat, p->AdjMat >= 1 ? p->AdjMat - 1: 0);
        if(p->Actual >= 8)
          continue;
        p->Actual = Bound(p->Actual + X, 1., 7.9); //Adjust the credit qualities.  
      }
    }
  }
}

/// <summary> This function breaks out the amount of loans a bank desired by credit quality and calcuates dollar 
/// amount based on the percentage values from the Credit Administration Decision form. </summary>
/// <param name="*Targ"> is the perentage value of Portfolio Target. </param>
/// <param name="*MaxOut"> is the Maximum Outstanding value. </param>
/// <remarks> It calculates Desired[Bank][TypeI][intIndexOfCount].</remarks>
void DesiredMk(float *Targ, float *MaxOut)
{
	static int TypeI =  - 1, Mk =  - 1;
	//  Tells if we're looking at one or two MaxOut fields per CQ Target list
	//  as found on the CreditAdmin decision form.
	int Two[] = //1 or 2 is determeined by the number of rows used in the Credit Administration Decision form
	{
		2, 2, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1,  - 1
	};
	int _1_2 = Two[Mk = ++Mk % MaxC];
	float T = 0, Frac[MaxRepCQINPUT];  //the number of column
	{
		Loop(MaxRepCQINPUT)T += Frac[J] = Targ[J];
	}
	//  Use the decisions to distribute dollar amounts.
	//  T is typically 100 at at this point.
	if(T)
		T = 1 / T;
	{
    //Loop(MaxRepCQINPUT)
		for(int J = 0; J < MaxRepCQINPUT; J++)
			Frac[J] *= T;
	}
	MaxOut--;
	//{
		//Loop(_1_2)
	for(int J = 0; J < _1_2; J++)
	{
		TypeI = ++TypeI % MaxL;
		float R;
		float Max = *++MaxOut;
		float *P = Rel_Pol[Bank][TypeI] - 1; 
		float *F = Frac - 1;
		float *C = Current[Bank][TypeI] - 1;
		//{
		for(int intIndexOfCount = 0; intIndexOfCount < MaxCQ; intIndexOfCount++)
		{
			R = (*++F + ( ! intIndexOfCount ? 0 :  *++F)) *Max / FloorHair(*++C + ( ! intIndexOfCount ? 0 :  *++C));
			*++P = R < 1 ? .5 : er(.9 *R, 1.5);
		}
		//}
		for(int intIndexOfCount = 0; intIndexOfCount < MaxRepCQINPUT; intIndexOfCount++)
			Desired[Bank][TypeI][intIndexOfCount] = Max * Frac[intIndexOfCount];
	}
	//}
}

/// <summary> This function initiates the loan routines and it converts some loan decisions from how they are on the screen to the internal format. </summary>
/// <remarks> This function is called by Loans(). </remarks>
void LoansStartUp()
{
	{
		AcT *Ac = BOA[Bank] + YrsQtr;
		IncomeOutGoT *Incm = &Ac->IncomeOutGo;
		long d = (long)SellLnsDec[Bank].MortBanking;  //This is the input from Sell Loan Decision form.  
		if(d == C)  //  C is security type 10. mnemonics for input fields.
			Ac->MortBanking = 0;
		if(d == I &&  ! Ac->MortBanking)
		{
			Ac->MortBanking = 1;
			Incm->Expenses.OtherOpEx += LoanKonst.MBStartup / 1000;
		}
		MortBanking[Bank] = (long)Ac->MortBanking;
	}
		memset(LoanRpt + Bank, 0, sizeof(*LoanRpt));  //Clean up loan reports
		memset(Current + Bank, 0, sizeof(*Current)); //2014. Clean up the Current
	{
		ResT *e = &ResGlo[Bank].Eff;  //How effective the banks compete with each other when using their resources.  
		//The values in e are calculated in Resources().  
		memmove(EBDB[Bank], e->Media.Loans, 4 *MaxL); //Effective Business Development 
		memmove(ELOS[Bank], e->Sal.Loans, 4 *MaxMD); //Effective Loan Officer Salary
		memmove(ELOS2[Bank], e->Sal.Loans, 4 *MaxMD); //Effective Loan Officer Salary2
		memmove(ECAS[Bank], e->Sal.Cred, 4 *MaxMD); //Effective Credit Administration Salary
		memmove(EPROC[Bank], e->Sal.Ops, 4 *MaxOD); //Effective Operation Salary
		memmove(BRCH[Bank], e->Premises, 4 *MaxPrem); //Effective Branches Cost
	}
	//Calculates annual fees for loans.  
	#define AnnFee(T,N) ANN_FEE[ Bank ][ T ] = LoanDec [ Bank ].N  / Prices.LnFixPrices [ T ].Fee;

	Loop(MaxL) ANN_FEE[Bank][J] = 1;
	AnnFee(0, NatFee)
	AnnFee(2, MidMFee)
	AnnFee(4, SmallBFee)
	AnnFee(6, ImpExpFee)
	AnnFee(7, ConsFee)
	AnnFee(12, HomeEquFee)
	AnnFee(14, CCardFee)
	GenCredPol[Bank] = (ulong)CredDec[Bank].GenCredPol - RE &0x3;  //Genenal Credit Policy is to save R/C/M/E based on user decision.  
	CredDecT Cred = CredDec[Bank];
	//Sets Desired[Bank] 
	DesiredMk(Cred.Nat.Targ, Cred.Nat.MaxOut);
	DesiredMk(Cred.MidM.Targ, Cred.MidM.MaxOut);
	DesiredMk(Cred.SmallB.Targ, Cred.SmallB.MaxOut);
	DesiredMk(Cred.ImpExp.Targ, &Cred.ImpExp.MaxOut);
	DesiredMk(Cred.Cons.Targ, &Cred.Cons.MaxOut);
	DesiredMk(Cred.CommRe.Targ, &Cred.CommRe.MaxOut);
	DesiredMk(Cred.MulFam.Targ, &Cred.MulFam.MaxOut);
	DesiredMk(Cred.SingFam.Targ, Cred.SingFam.MaxOut);
	DesiredMk(Cred.HomeEqu.Targ, &Cred.HomeEqu.MaxOut);
	DesiredMk(Cred.Personal.Targ, &Cred.Personal.MaxOut);
	DesiredMk(Cred.CredC.Targ, &Cred.CredC.MaxOut);
	DesiredMk(Cred.Instal.Targ, &Cred.Instal.MaxOut);
}

float SumXMS[MaxL];

char  *LoanName[] = 
{
  " 1 Business;  National Corporate;  Credit Lines",
  " 2 Business;  National Corporate;  Term",
  " 3 Business;  Middle Market;  Credit Lines",
  " 4 Business;  Middle Market;  Term",
  " 5 Business;  Small Business;  Credit Lines",
  " 6 Business;  Small Business;  Term",
  " 7 Business;  Trade",
  " 8 Real Estate;  Construction",
  " 9 Real Estate;  Commercial",
  "10 Real Estate;  Multi-family",
  "11 Real Estate;  1-4 Family;    Fixed Rate",
  "12 Real Estate;  1-4 Family;    Var Rate",
  "13 Real Estate;  Home Equity",
  "14 Consumer;  Personal",
  "15 Consumer;  Credit Card",
  "16 Consumer;  Installment" 
};

// A detailed dump of each and every loan, for debugging. 
// Shall consider moving out of calculation logic. 
/// <summary> This function writes a report of each loan in this bank. </summary>
/// <remarks> The file name is L(Bank).txt. </remarks>
void LL()
{
	extern char BankName[50], TimeStamp[88], FQ[Qtrs + 1][10]; 
	extern  char FromCom, Community; 

  LB
  {
    LoanGloT *LnG = LoanGlo + Bank;
    {
      char F[4] =
      {
        'L', 49, 0, 0
      };
      F[1] += (char)Bank;
      ODF(F, "TXT", "w");
    }
	FILE *Out;
	extern FILE *fp; //??
    Out = fp;
    fprintf(fp, "\n%s\n\nCommunity %c\nSimQtr %s\n\n", TimeStamp, FromCom ? FromCom : Community,  *FQ);
    SetBankNames(1);
    fprintf(fp, "\nBank %d (One-Based) \"%s\"\n\n", Bank + 1, BankName);
    {
      for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
      {
        long TypeI = intIndexOfCount;
        float *F = LnFacs[Bank][TypeI];
        NumLen = 6;
        DotLen = 3;
        fprintf(Out, "%s    |||  Non-Accruel:%s\n"" MktShare == Hi-CQ: %d%% [%5.2f /%6.2f ] Med: %d%% [%5.2f /%6.2f ] Low: %d%% [%5.2f /%6.2f ] ==\n""     EffMedia:%5.2f, EffLnOffcr:%5.2f, EffOps:%5.2f, EffPremises:%5.2f ORIG_FEE:%5.2f, \n""     PrevMktShare:%5.2f     ANN_FEE: %5.2f, %5.2f, %5.2f, MaturityPref: %5.2f, %5.2f, %5.2f,\n""     GenCQPolicy: %5.2f, %5.2f, %5.2f, CQPolicy:%5.2f, %5.2f, %5.2f, RelRates: %5.2f, %5.2f, %5.2f,\n""     EC: %5.2f,\n "" Amnt,   Mat, Rate,  Sprd, Rp, Ac,   PD,   AP, AM, AFee,  Comm,  UsR, \
 Paym,     Id,   Pkg\n", LoanName[TypeI], FA(LnG->NAT[TypeI]), (long)(100 *F[0] / F[3]), F[0], F[3], (long)(100 *F[1] / F[4]), F[1], F[4], (long)(100 *F[2] / F[5]), F[2], F[5], F[6], F[7], F[8], F[9], F[10], F[11], F[12], F[13], F[14], F[15], F[16], F[17], F[18], F[19], F[20], F[21], F[22], F[23], F[24], F[25], F[26], ResGlo[Bank].Eff.Sal.Loans[LnToSal[intIndexOfCount]]);
        {
          LoanT *p = pLoans[Bank][TypeI];
          int RecCnt = (long)LnG->LnCnt[TypeI];
          for(int intIndexOfCount = 0; intIndexOfCount < RecCnt; intIndexOfCount++)
          {
            float *e = FeP(p++);
            if( !  *e)
              continue;
            {
              long Max = LnSz / 4;
              static long LenSet[] =
              {
                 - 1, 7, 4, 6, 6, 2, 4, 5,  - 1,  - 1, 4, 3, 6, 7, 5, 6, 8, 3
              }
              , DotSet[] =
              {
                 - 1, 3, 0, 2, 2, 1, 1, 2,  - 1,  - 1, 0, 0, 2, 3, 2, 3, 0, 0
              };
              for(int intIndexOfCount = 0; intIndexOfCount < Max; intIndexOfCount++)
              {
                if( ! intIndexOfCount || intIndexOfCount == 8 || intIndexOfCount == 9)
                  continue;
                NumLen = LenSet[intIndexOfCount];
                DotLen = DotSet[intIndexOfCount];
                fprintf(Out, "%s ", FA(e[intIndexOfCount]));
              }
            }
            fprintf(Out, "\n");
          }
        }
      }
    }
    CloseOut();
  }
}

// Given a loan package, returns a loan type.
// The second 10 is questionable.  
long PkgToTy[] =
{
  5, 8, 9, 10, 11, 10
};

/// <summary> This function add loans to pakages for sale. </summary>
/// <remarks> It is called by Loans(). This function loops through each loan in certain loan type (index=5,8,9,10,11)
/// and it adds the loan to package if the loan pass some qualification. 
/// The function calculates LnPkg[Bank] from pLoans[Bank][TypeI]. </remarks> 
void MakePkgs()
{
  LnPkgT *PKGS = LnPkg[Bank];
  long FP =  - 4;
  LoanGloT *LnG = LoanGlo + Bank;
  memset(PKGS, 0, sizeof(*LnPkg));  //clean up the LnPkg[Bank]
  {
    for(int intIndexOfCount = 0; intIndexOfCount < (MaxTypsSold - 1); intIndexOfCount++)  //#define MaxTypsSold 6
    {
      long TypeI = (long)PkgToTy[intIndexOfCount]; //returns a loan type
			long RecCnt = (long)LnG->LnCnt[TypeI];
			long SmallBus = !intIndexOfCount;
      LnPkgT **Pkg,  *E;
			//For each loan type, the max number of packages is 4 and the total of packages is 20
			//each loop moves 4 packages in PKGS. 
			LnPkgT *PkgA = PKGS + (FP += 4);
			LnPkgT *E_A = PkgA + 2;
			LnPkgT *PkgB = E_A;
			LnPkgT *E_B = PkgB + 2;
      LoanT *r = pLoans[Bank][TypeI] + RecCnt;
      if(SmallBus)
      {
        Pkg = &PkgA;
        E = E_B;
      }
      {
        for(int intIndexOfCount = 0; intIndexOfCount < RecCnt; intIndexOfCount++)
        {
          if((--r)->Amnt <= 0)
            continue;
          r->Pkg = 0;
          {
            float ACQ = r->Actual;
            long SetA = ACQ < 3.5;
						long M = (long)r->Mat;
            float A = r->Amnt;
						//The loan has to meet some qualification in order to be in the package. 
            if(M < 8 || r->PastDue >= 1 || A > 10)
              continue;
            if(SmallBus)
            {
              if(ACQ > 3)
                continue;
            }
            else
            {
              if(ACQ > 6)
                continue;
              Pkg = SetA ? &PkgA: &PkgB; //For the same loan type, the first two packages have better credit quality than the second two packages. 
              E = SetA ? E_A : E_B;
            }
            if((*Pkg)->Amnt + A > 10)
            {
              if(*Pkg + 1 >= E)
                continue;
              ++(*Pkg);
            }
						//Calculate some properties (e.g. Credit Quality, Book Yield)for loan package, some of which are displayed in B275. 
            {
              LnPkgT *P =  *Pkg;
              float NewA = P->Amnt + A;
							float W = A / NewA;
							float W0 = 1-W;
              r->Pkg = 1+P - PKGS;
              P->Amnt = NewA;
              P->Pay += r->Payment;
              P->AvgMat = P->AvgMat *W0 + M * W;
              P->AvgRCQ = P->AvgRCQ *W0 + ACQ * W;
              P->BookYld = P->BookYld *W0 + r->Rate *W;
              if(r->AdjP)
              {
                P->AvgRAP = P->AvgRAP *W0 + r->AdjP *W;
                if(M >= r->AdjP)
                  M = (long)r->AdjP;
              }
              {//Calculates the MarketValue and MarketYield for the loan package.  
                float StdM =  ! r->AdjP ? LoanKonst.Mat[TypeI]: Prices.LnFixPrices[TypeI].QRAP;
								float Spread = EconBOQ.Econ.LoanCompRates[TypeI] - YldC(StdM, EoQ);
								float AdjCMR = .0025 *(YldC(M, EoQ) + Spread * Pow(r->Mat / LoanKonst.Mat[TypeI], LoanKonst.LnSaleMatAdj[TypeI])*Pow(ACQ / 3.5, LoanKonst.LnSaleCQAdj[TypeI]));
								float R = .0025 * r->Rate;
								float Fac = 1-Pow(1+AdjCMR,  - M);
								float MktVal = A *(TypeI == 11 ? R / AdjCMR * Fac / (1-Pow(1+R,  - M)): 1+(R - AdjCMR) *Fac / AdjCMR);
                P->MktVal += MktVal <= A ? MktVal : A *Pow(MktVal / A, LoanKonst.LnSaleMktAdj[TypeI]);
                P->MktYld = P->MktYld *W0 + W * 400 * R * A / MktVal;
              }
            }
          }
        }
      }
    }
  }
}

/// <summary> This function processes the Loan Sales Decisions from SellLnsDec[Bank].Pkg.</summary>
/// <remarks> It is called by Loans(). The function goes through each package marked in the Loan Sales Decision form. 
/// All of loan and package information can be found in Lx.txt. If the loan package is sold, it will wipe out all loans in this package.   
/// It updates structure variables: LoanRpt, R275 and LoanGlo. </remarks>
void SaleDecs()
{
	// Item 5 of final list in Panther Project
	// Loan Sales have not been accounted for in Day1 Funds in earlier versions of BMSim
	FundsGloT *Fns = FundsGlo + Bank;
  float *D = SellLnsDec[Bank].Pkg;
  LnPkgT *PKG = LnPkg[Bank];
  float *AssetSales = &BOA[Bank][YrsQtr].IncomeOutGo.Nets.AssetSales;
  LoanRptT *LnR = LoanRpt + Bank;
  R275T *Rpt = R275[Bank];
  LoanGloT *LnG = LoanGlo + Bank;
  memset(Rpt, 0, sizeof(*Rpt));
  {
    for(int intIndexOfCount = 0; intIndexOfCount < MaxLnPkgs; intIndexOfCount++)
    {
      int i = (int)*(D++) - 201; //The loan package sale code must be larger than 200.  
      if(i < 0 || i >= MaxLnPkgs || PKG[i].Amnt < 2)
        continue;
      {
        LnPkgT *Pkg = PKG + i;
        float M = Pkg->AvgMat;
				float R = Pkg->BookYld;
				float A = Pkg->Amnt;
				float P = Pkg->Pay;
        int p = i / 4;
        LnSalesT *Folio = LnSales[Bank] + p;
        float NewA = Folio->Amnt + A;
				float W = A / NewA;
				float W0 = 1-W;
        Folio->Amnt = NewA;
        Folio->Pay += P;
        Folio->Mat = Folio->Mat *W0 + M * W;
        Folio->Rate = Folio->Rate *W0 + R * W;
        Rpt[p].A += A;
        Rpt[p].MktA += Pkg->MktVal;
				//Item 5 of final list. Add market value to Day1In loans. 
				Fns->Day1In.Loans = Pkg->MktVal;
        i++;
        *AssetSales += Pkg->MktVal - A;
        {//Set Loan Report LoanRpt
          int LnT = (int)PkgToTy[p];
					int c = (int)LnG->LnCnt[LnT];
          LoanT *p = pLoans[Bank][LnT] + c;
          for(int intIndexOfCount = 0; intIndexOfCount < c; intIndexOfCount++)
            if((--p)->Pkg == i)
              memset(p, 0, 8);  //if the package is sold, all of loans in this package will be wiped out. 
          LnR->Sales[LnT] += A;
        }
        Pkg->Amnt = 0;
      }
    }
  }
}

/// <summary> This function services a package of loans sold in SaleDecs().  </summary>
/// <remarks> It is called by Loans(). The function calculates LnSales[Bank] including the total amount and the paid amount. 
/// It also accumulates BOA[Bank].IncomeOutGo.Fees.Other and BOA[Bank].IncomeOutGo.Expenses.OtherOpEx. 
/// If the maturity is less than 1 or the loan amount is less than .025 million, the loan will be wiped out. </remarks>
void ServicePkgs()
{
  LnSalesT *S = LnSales[Bank] - 1;
  PrePayT *PP = LoanKonst.PrePay;
  AcT *Ac = BOA[Bank] + YrsQtr;
  IncomeOutGoT *Incm = &Ac->IncomeOutGo;
  for(int intIndexOfCount = 0; intIndexOfCount < MaxTypsSold; intIndexOfCount++)
  {
    float A = (++S)->Amnt;
    if(A <= 0)
      continue;
    {
      long t = PkgToTy[intIndexOfCount];
      float Pre = A * .5 *(PP[t].a + PP[t].c); //Prepaid amount by customer
			float I = (A -= Pre) *S->Rate *.0025;
			float P = min(A,  ! intIndexOfCount ? A / S->Mat: S->Pay - I);
			float AvgA = A - .5 *(P + Pre); //Average amount
      Incm->Fees.Other += S->Inc = AvgA * LoanKonst.Srv;
      Incm->Expenses.OtherOpEx += S->Exp = LoanKonst.SrvFix * .000001 + AvgA * LoanKonst.SrvVar[intIndexOfCount];
      S->Amnt = A -= P;
      S->Paid = P + Pre;
      if(--S->Mat < 1 || A < .25)	
        memset(S, 0, 8);
    }
  }
}

void SumOfLoans(); 
/// <summary> This is the main routine for all loan activities. </summary>
void Loans()
{
	extern long NoDumps;
	extern float FHLB_Avail_BoQ[MaxB], FHLB_Avail_EoQ[MaxB]; 

  memset(R275, 0, sizeof(R275));
  memset(NewLoans, 0, sizeof(NewLoans));
  memset(Rel_Pol, 0, sizeof(Rel_Pol)); //2014 clean up Rel_Pol for each quarter before creating new loans

  LB LoansStartUp();
  LB SaleDecs();
  LB
  {
    float *Lns = BOA[Bank][YrsQtr].Bal.Assets.Loans.Loans;
    FHLB_Avail_BoQ[Bank] += Lns[10] + Lns[11] + Lns[12] + Lns[9]; //Federal Home Loan Bank += all family mortgage loans
    MatureEconAdj();
    Reviews();
    NonPerforming();
    Repayments();
    Prepayments();
    CurLoans();
  }
  Volume();
  Quality();
  MktShare();
  BkDecs();
  LB
  {
    CreateNewLoans();
    Projected();
    MakePkgs();
    ServicePkgs();
    EoQRpt();
  }
  if( ! NoDumps)
    LL();  //Loan Reports
  memset(SumXMS, 0, sizeof(SumXMS));
  LB
  {
    for(int intIndexOfCount = 0; intIndexOfCount < MaxL; intIndexOfCount++)
      SumXMS[intIndexOfCount] += LoanRpt[Bank].New[intIndexOfCount];
  }

//  LB 2013 Change: Create the new method o SumOfLoans
  Bank = -1; while ( ++Bank < NumBanks )
  {	
	  SumOfLoans(); 
  }
}

void SumOfLoans()
{
	extern float FHLB_Avail_BoQ[MaxB], FHLB_Avail_EoQ[MaxB]; 
	LoanRptT *LnR = LoanRpt + Bank;
	LoanGloT *LnG = LoanGlo + Bank;
	AcT *Ac = BOA[Bank] + YrsQtr;
	IncomeOutGoT *Incm = &Ac->IncomeOutGo;
	LnAcT *AcL = &Ac->Bal.Assets.Loans;
	AcL->Sum = 0;
	  
	memmove(LnG->B_Com, LnG->E_Com, sizeof LnG->B_Com);
    
      for(int LnT = 0; LnT < MaxL; LnT++)
      {
        //int LnT = intIndexOfCount;
		  Incm->Fees.LoanFees += LnR->AnnFees[LnT] + LnR->OrgFees[LnT];
		  if(LnR->New[LnT])
			  LnG->PlayersMS[LnT] = LnR->New[LnT] / SumXMS[LnT];

          float S = GroupA(LnT) ? 0 : LnG->NAT[LnT];
		  float cs = 0;
          int RecCnt = (long)LnG->LnCnt[LnT];
		  int i = LnToCL[LnT];
          LoanT *p = pLoans[Bank][LnT] - 1;
          for(int j = 0; j < RecCnt; j++)
		  {
			  S += (++p)->Amnt; //sum of loan amount
			  cs += p->Commit; //sum of commit
		  }
          if(i >= 0)
			  LnG->E_Com[i] = cs;
          AcL->Sum += AcL->Loans[LnT] = S;    
      }
    
      float *Lns = AcL->Loans;
      FHLB_Avail_EoQ[Bank] += Lns[10] + Lns[11] + Lns[12] + Lns[9];

}
