#include <math.h>
#include "BMSim.h"

extern TreasKT TreasK; 
extern LoanGloT LoanGlo[MaxB]; 
extern LoanRptT LoanRpt[MaxB];
extern FundsGloT FundsGlo[MaxB]; 
extern ECNT EconBOQ, EconEOQ; 
//for Gap()
extern SecGloT SecGlo[MaxB]; 
extern TimeDepT TimeDep[MaxB][MaxTimeDep]; 
extern DepKT DepK;

extern TreasDecT TreasDec[MaxB];

extern BOAT BOA[MaxB]; 
extern int YrsQtr; 
extern int Bank;

float Gap, Gap4Qtr;

/// <summary> This function accumulate either ending balance or interest rate for each field in Report B423.  </summary>
/// <param name="Row"> is the row number in Report B423. It could be Business, Individual, Non-Customer, or Total. </param>
/// <param name="IsFixed"> is either Fixed interest if 1 or Variable interest if 0. </param>
/// <param name="MatI"> is the maturity. </param>
/// <param name="Bal"> is the balance. </param>
/// <param name="Int"> is the interest. </param>
/// <param name="tokenBalSumByMat"> is array of ending balance by maturity.  </param>
/// <param name="tokenIntSumByMat"> is array of average rate by maturity. </param>
/// <remarks> it is called by GenGap(). </remarks>
void Sum423B(int Row, long IsFixed, long MatI, float Bal, float Int,
		float tokenBalSumByMat[][Max423Cols],
		float tokenIntSumByMat[][Max423Cols]) {

	long TotI = Row + 2, FixI = Row + !IsFixed;
	float *TotP_B = tokenBalSumByMat[ TotI ],
		*FixP_B = tokenBalSumByMat[ FixI ],
		*TotP_I = tokenIntSumByMat[ TotI ], 
		*FixP_I = tokenIntSumByMat[ FixI ];

	TotP_B[MatI] += Bal;
	*TotP_B += Bal;

	TotP_I[MatI] += Int;
	*TotP_I += Int;

	FixP_B[MatI] += Bal;
	*FixP_B += Bal;

	FixP_I[MatI] += Int;
	*FixP_I += Int; 
}



/// <summary> This function calculates three security rows in B530. </summary>
/// <param name="token"> decides type of security which can be bills, municiple or mortgage backed.  </param>
/// <param name="type"> is the security amount.</param>
/// <param name="type2"> is the security interest. </param>
/// <param name="*T"> is the array of floating point numbers. The total number is columns on B530</param>
void GapSec530(int token, float type, float type2, float *T) {
	
	AcT *Ac = BOA[Bank] + YrsQtr;
  BalT *Bal = &Ac->Bal;
  AssetsT *Ass = &Bal->Assets;
  SecuritiesT *Sec = &Ass->Securities;
	SecSumT *SecPort = SecGlo[Bank].Sec;


	//float F = Sec->Type;
	float F = type;
	T[0] += F; 
	{ 
		//float I = SecInc->Type;
		float I = type2;
		T[1] += I; 
	} 
	{ 
		float Tot = 0;
		//Loop(Qtrs) 
		for(int J = 0; J < Qtrs; J++)
		{ 
			//float F = Mac(J);
			//HACK: workaround circular relashionship from macro:
			float F;
			switch (token) {
				case 0:
					F = SecPort[BillsE].MatDistrib[J].Book + 
						SecPort[StripsE].MatDistrib[J].Book + 
						SecPort[BondsE].MatDistrib[J].Book;
					break;
				case 1:
					F = SecPort[MunisE].MatDistrib[J].Book;
					break;
				case 2:
					F = SecPort[MortE].MatDistrib[J].Book;
					break;
			}
			 
			Tot += F; 
			T[ J + 2 ] += F; 
		}

		T[6] += Tot; 
		{ 
			//float x = Sec->Type - Tot; 
			float x = type - Tot; 
			T[7] += x; 
		} 
	} 
	{ 
		//float x = Mac(0);
		//HACK: workaround circular relashionship from macro:
		float x;
		switch (token) {
			case 0:
				x = SecPort[BillsE].MatDistrib[0].Book + 
					SecPort[StripsE].MatDistrib[0].Book + 
					SecPort[BondsE].MatDistrib[0].Book;
				break;
			case 1:
				x = SecPort[MunisE].MatDistrib[0].Book;
				break;
			case 2:
				x = SecPort[MortE].MatDistrib[0].Book;
				break;
		}

		T[8] += x; 
		T[10] += x;
		{
			//float x2 = Mac(1);
			float x2;
			switch (token) {
				case 0:
					x2 = SecPort[BillsE].MatDistrib[1].Book + 
						SecPort[StripsE].MatDistrib[1].Book + 
						SecPort[BondsE].MatDistrib[1].Book;
					break;
				case 1:
					x2 = SecPort[MunisE].MatDistrib[1].Book;
					break;
				case 2:
					x2 = SecPort[MortE].MatDistrib[1].Book;
					break;
			}

			T[11] += x2;
			{
				//float x3 = Sec->Type - ( x + x2 );
				float x3 = type - ( x + x2 );
				T[12] += x3; 
			} 
		} 
	}
}

/// <summary> This function adds more values to each field in a loan row in B530. </summary>
/// <param name="LT"> is the row number of loan. </param>
/// <param name="*T"> is the array of floating point number in a row. </param> 
/// <remarks> This function is only used in row in the Loans section, which has no 2-4 qtr Estimated Interest Rate Sensitivity. </remarks> 
void GapV530(int LT, float *T) {

	AcT *Ac = BOA[Bank] + YrsQtr;
  BalT *Bal = &Ac->Bal;
  AssetsT *Ass = &Bal->Assets;
  SecuritiesT *Sec = &Ass->Securities;
  float *Lns = Ass->Loans.Loans;
	IncomeOutGoT *Incm = &Ac->IncomeOutGo;
  InterestT *Int = &Incm->Interest;
  IntInT *In = &Int->Income;
  SecuritiesT *SecInc = &In->Securities;
  float *LnInt = In->Loans.Loans;
  SecSumT *SecPort = SecGlo[Bank].Sec;
  LoanRptT *LnR = LoanRpt + Bank;
  LoanGloT *LnG = LoanGlo + Bank;
  float(*Mat)[MaxMC] = LnR->Mat;    

	float F = Lns[LT];
	T[0] += F; 
	{
		float I = LnInt[LT];
		T[1] += I; 
	}
	T[2] += F; 
	T[6] += F; 
	{
		float x = Mat[LT][0];
		T[9] += x; 
		T[10] += x;
			
		{
			float x2 = Mat[LT][1];
			T[11] += x2;
			{
				float x3 = Lns[LT] - ( x + x2 ); 
				T[12] += x3; 
			} 
		}
	}
}

/// <summary> This function adds more values to each field in a loan row in B530. </summary>
/// <param name="LT"> is the row number of loan. </param>
/// <param name="*T"> is the array of floating point number in a row. </param> 
/// <remarks> This function is only used in 1-4 family Fix Rate row and Consumer Installment row. </remarks> 
void GapN530(int LT, float *T) { 

	AcT *Ac = BOA[Bank] + YrsQtr;
    BalT *Bal = &Ac->Bal;
    AssetsT *Ass = &Bal->Assets;
    SecuritiesT *Sec = &Ass->Securities;
    float *Lns = Ass->Loans.Loans;
    IncomeOutGoT *Incm = &Ac->IncomeOutGo;
    InterestT *Int = &Incm->Interest;
    IntInT *In = &Int->Income;
    SecuritiesT *SecInc = &In->Securities;
    float *LnInt = In->Loans.Loans;
    SecSumT *SecPort = SecGlo[Bank].Sec;
    LoanRptT *LnR = LoanRpt + Bank;
    LoanGloT *LnG = LoanGlo + Bank;
    float(*Mat)[MaxMC] = LnR->Mat;

	float F = Lns[LT]; 
	T[0] += F; 
	{
		float I = LnInt[LT];
		T[1] += I; 
	}
	{
		float Tot = 0;
		//Loop(Qtrs) 
		for(int J = 0; J < Qtrs; J++)
		{ 
			float F = Mat[LT][J];
			Tot += F; 
			T[ J + 2 ] += F; 
		}
		
		T[6] += Tot; 
		{
			float x = Lns[LT] - Tot;
			T[7] += x; 
		} 
	} 
	{
		float x = Mat[LT][0];
		T[9] += x; 
		T[10] += x;
		
		{
			float x2 = Mat[LT][1];
			T[11] += x2; 
			{
				float x3 = Lns[LT] - ( x + x2 );
				T[12] += x3; 
			} 
		} 
	}
}

/// <summary> This function adds more values to each field in a loan row in B530. </summary>
/// <param name="LT"> is the row number of loan. </param>
/// <param name="*T"> is the array of floating point number in a row. </param> 
/// <param name="Sensi[MaxL][Qtrs]"> is array of Sensitivity data.</param>
/// <remarks> This function is only used in row with every quarter Estimated Interest Rate Sensitivity. </remarks> 
void GapR530(int LT, float *T, float Sensi[MaxL][Qtrs]) { 

    AcT *Ac = BOA[Bank] + YrsQtr;
    BalT *Bal = &Ac->Bal;
    AssetsT *Ass = &Bal->Assets;
    SecuritiesT *Sec = &Ass->Securities;
    float *Lns = Ass->Loans.Loans;
		IncomeOutGoT *Incm = &Ac->IncomeOutGo;
    InterestT *Int = &Incm->Interest;
    IntInT *In = &Int->Income;
    SecuritiesT *SecInc = &In->Securities;
    float *LnInt = In->Loans.Loans;
    SecSumT *SecPort = SecGlo[Bank].Sec;
    LoanRptT *LnR = LoanRpt + Bank;
    LoanGloT *LnG = LoanGlo + Bank;
    float(*Mat)[MaxMC] = LnR->Mat;
    //float Sensi[MaxL][Qtrs];

	{
	float F = Lns[LT]; 
	T[0] += F; 
	{
		float I = LnInt[LT];
		T[1] += I; 
	}
	}
	{
		float Tot = 0;
		//Loop(Qtrs) 
		for(int J = 0; J < Qtrs; J++)
		{
			float F = Sensi[LT][J]; 
			Tot += F; 
			T[ J + 2 ] += F; 
		}
		
		T[6] += Tot; 
		{
			float x = Lns[LT] - Tot;
			T[7] += x; 
		} 
	} 
	{
		//float x = Sensi[LT][0]; 
		float x = Mat[LT][0];
		T[9] += x; 
		T[10] += x;
		{
			//float x2 = Sensi[LT][1];
			float x2 = Mat[LT][1];
			T[11] += x2;
			{
				float x3 = Lns[LT] - ( x + x2 );
				T[12] += x3; 
			} 
		} 
	}
}

/// <summary> This function calculates a row in the B531. </summary>
/// <param name="Prod"> is the liability balance. </param>
/// <param name="Prod1"> is the liability interest. </param>
/// <param name="KP"> is used to calculate the liquidity. </param>
/// <param name="*T"> is the array of floating point number in a row. </param>
/// <remarks> This function is used to calculate three rows in B531 including Now, Saving and Money Market account. </remarks>
void GapM531(float Prod, float Prod1, float KP, float *T) {

	//float Bal = DepL->Prod, K = KP;
	float Bal = Prod, K = KP;
	T[0] += Bal;
	
	{
		//float I = DepE->Prod;
		float I = Prod1;
		T[1] += I;
	}
	
	T[2] += Bal;
	T[6] += Bal;
	{
		float x = Bal * K;
		T[9] += x;
		T[10] += x;
		{
			float x2 = Bal * K * (1 - K);
			T[11] += x2;
			{
				float x3 = Bal - ( x + x2 );
				T[12] += x3; 
			}
		}
	}
}

/// <summary> This function calculates Time Deposit Corporate CDs row in the B531. </summary>
/// <param name="(*R)[B501aC]"> is one row from the B501. </param>
/// <param name="TR"> is time deposit rate. </param>
/// <param name="*T"> is the array of floating point number in a row. </param>
void GapM531b(float (*R)[B501aC], int TR, float *T ) {

	//switch (token) {
	//	case 0:
	//		(*R)[B501aC] = B501a[Bank];
	//		break;
	//	case 1:
	//		(*R)[B501bC] = B501b[Bank];
	//		break;
	//}
	//float (*R)[C] = D[Bank], 
	float F = R[TR][0];

	T[0] += F;

	{
		float I = R[TR][3];
		T[1] += I;
	}
	{
		float Tot = 0;
		//Loop(Qtrs) 
		for(int J = 0; J < Qtrs; J++)
		{ 
			float F = R[ 1 + J ][0];
			Tot += F;
			T[ J + 2 ] += F; 
		}
		T[6] += Tot;
		{
			float x = F - Tot;
			T[7] += x;  
		}
	}                                                                                
	{
		float x = R[1][0];
		T[8] += x; 
		T[10] += x;
		{
			float x2 = R[2][0];
			T[11] += x2;
			{
				float x3 = F - ( x + x2 );
				T[12] += x3; 
			}
		}
	}                                  
}

/// <summary> This function calculates Time Deposit Public Time row in the B531. </summary>
/// <param name="(*R)[B501bC]"> is one row from the B501. </param>
/// <param name="TR"> is time deposit rate. </param>
/// <param name="*T"> is the array of floating point number in a row. </param>
void GapM531b1(float (*R)[B501bC], int TR, float *T ) {

	float F = R[TR][0];

	T[0] += F;

	{
		float I = R[TR][3];
		T[1] += I;
	}
	{
		float Tot = 0;
		//Loop(Qtrs)
		for(int J = 0; J < Qtrs; J++)
		{ 
			float F = R[ 1 + J ][0];
			Tot += F;
			T[ J + 2 ] += F; 
		}
		T[6] += Tot;
		{
			float x = F - Tot;
			T[7] += x;  
		}
	}                                                                                
	{
		float x = R[1][0];
		T[8] += x; 
		T[10] += x;
		{
			float x2 = R[2][0];
			T[11] += x2;
			{
				float x3 = F - ( x + x2 );
				T[12] += x3; 
			}
		}
	}                                  
}

/// <summary> This function adds values to Totl IntBearLiab row in the B531. </summary>
/// <param name="*x"> a pointer to one row in B531 (the array of floating point number). </param>
/// <param name="token"> is the total value. </param>
void GapM531c(float *x, float token) { 
	//float f = Ass->Total - *x;
	float f = token - *x;
	*x += f;
	x[7] += f;
	x[12] += f;  
}

/// <summary> This function calculates Interest Rt Swap row in the B531. </summary>
/// <param name="t"> is the security type.  In this function, it could be Fix Rate Swap or Variable Rate Swap. </param>
/// <param name="*SecPort"> is the pointer to the securities portfolio data structure. </param>
/// <param name="*N"> is the array of 7 floating point number. </param>
void GapM531d(int t, SecSumT *SecPort, float *N) {
	float S[7]; 
	long Neg = t == SwapVP;
	S[0] = SecPort[t].Interest;
	{
		float Tot = 0;
		Loop(3) {
			float F = SecPort[t].MatDistrib[ J + 1 ].Par;
			
			if ( Neg )
				F *= -1;
			Tot += F;
			S[ J + 2 ] = F;
		}

		S[6] = ( Neg ? -1 : 1 ) * ( SecPort[t].EOQ.Par - SecPort[t].MatDistrib[0].Par) - Tot;
		S[1] = -Tot - S[6]; 
		S[5] = -S[6];
	}
	{
		Loop(7) { 
			N[J] += S[J]; 
		}
	}
}

//used in Stock->GenGap
D531T D531[MaxB];
float D530[13];
float BalSumByMat[Max423Rows][Max423Cols],  IntSumByMat[Max423Rows][Max423Cols];
//Used in Treasury & Stock
float B501a[MaxB][B501aR][B501aC];
float B501b[MaxB][B501bR][B501bC];

/// <summary> This function calculates the difference (forecast - actual) in EPS for the quarter and it also calculates the absolute value of these differences. </summary>
/// <remarks> It is used in Stock() to set Ax[8]. The gap results are saved in Gap and Gap4Qtr. </remarks>
void GenGap()
{
	extern pLoansT pLoans[MaxB]; // The actual loan portfolio
	extern DWORD DotLen;

	//remove static from below

  {
    memset(BalSumByMat, 0, sizeof(BalSumByMat));
    memset(IntSumByMat, 0, sizeof(IntSumByMat));
    {
      long MatCls[] =
      {
        13, 9, 7, 5, 4, 3, 2, 1
      };
      long TimeI =  - 1;
      TimeDepT *t = TimeDep[Bank] - 1;
      enum
      {
        BusF, bv, bt, IndF, iv, it, HotF, hv, ht, TotF, tv, tt
      };
      while(++TimeI < MaxTimeDep)
      {
        ++t;
        if( ! t->Bal)
          continue;
        {
          long IsFixed = (long)t->IsFixed;
          float Bal = t->Bal, Int = Bal *(t->RateOrSpread + (IsFixed ? 0 : EconEOQ.Econ.One_Qtr_Treas));
          long Mat = max((long)t->Mat, 1),  *mP = MatCls, Mkt = (long)t->Market, MatI = Max423Cols - 1;
          while(Mat <  *mP++)
            --MatI;
          //Sum423(TotF); // From B530
					Sum423B(TotF, IsFixed, MatI, Bal, Int, BalSumByMat, IntSumByMat);

          switch(Mkt)
          {
            case HotE:
              //Sum423(HotF);
							Sum423B(HotF, IsFixed, MatI, Bal, Int, BalSumByMat, IntSumByMat);
              break;
            case IndE:
              //Sum423(IndF);
							Sum423B(IndF, IsFixed, MatI, Bal, Int, BalSumByMat, IntSumByMat);
              break;
            default:
              //Sum423(BusF);
							Sum423B(BusF, IsFixed, MatI, Bal, Int, BalSumByMat, IntSumByMat);
          }
        }
      }
    }
  }
  {
    AcT *Ac = BOA[Bank] + YrsQtr;
    BalT *Bal = &Ac->Bal;
    AssetsT *Ass = &Bal->Assets;
    SecuritiesT *Sec = &Ass->Securities;
    float *Lns = Ass->Loans.Loans;
    IncomeOutGoT *Incm = &Ac->IncomeOutGo;
    InterestT *Int = &Incm->Interest;
    IntInT *In = &Int->Income;
    SecuritiesT *SecInc = &In->Securities;
    float *LnInt = In->Loans.Loans;
    SecSumT *SecPort = SecGlo[Bank].Sec;
    LoanRptT *LnR = LoanRpt + Bank;
    LoanGloT *LnG = LoanGlo + Bank;
    float(*Mat)[MaxMC] = LnR->Mat;
    float Sensi[MaxL][Qtrs];
    float *T = D530;
    memset(D530, 0, sizeof(D530));

    {
      float F = Ass->ffSold;
      T[0] += F;
      {
        float I = In->FF;
        T[1] += I;
      }
      T[2] += F;
      T[6] += F;
      T[8] += F;
      T[10] += F;
    }

//#define M530(M,i) SecPort[M].MatDistrib[i].Book
//#define M530i(i) M530(BillsE,i) + M530(StripsE,i) + M530(BondsE,i)
//#define M530i2(i) M530(MunisE,i)
//#define M530i3(i) M530(MortE,i)

//Remove: using procedure GapSec530() instead.
//#define GapSec530( Type, Mac ) { float F = Sec->Type; T[0] += F; { float I = SecInc->Type; T[1] += I; } } \
//{ float Tot = 0; Loop(Qtrs) { float F = Mac(J); Tot += F; T[ J + 2 ] += F; } \
//T[6] += Tot; { float x = Sec->Type - Tot; T[7] += x; } } { float x = Mac(0); T[8] += x; T[10] += x; \
//{ float x2 = Mac(1); T[11] += x2; { float x3 = Sec->Type - ( x + x2 ); T[12] += x3; } } }

	//GapSec530(Treasuries, M530i)
	GapSec530(0, Sec->Treasuries, SecInc->Treasuries, T);

	//GapSec530(Munis, M530i2)
	GapSec530(1, Sec->Munis, SecInc->Treasuries, T);

	//GapSec530(Mort, M530i3)
	GapSec530(2, Sec->Mort, SecInc->Mort, T);

//Remove: using procedure GapV530 instead.
//#define GapV530(LT) { float F = Lns[LT]; T[0] += F; { float I = LnInt[LT]; T[1] += I; } \
//T[2] += F; T[6] += F; } { float x = Mat[LT][0]; T[9] += x; T[10] += x; \
//{ float x2 = Mat[LT][1]; T[11] += x2; { float x3 = Lns[LT] - ( x + x2 ); T[12] += x3; } } }

//Remove: using procedure GapN530() instead.
//#define GapN530(LT) { float F = Lns[LT]; T[0] += F; { float I = LnInt[LT]; T[1] += I; } } \
//{ float Tot = 0; Loop(Qtrs) { float F = Mat[LT][J]; Tot += F; T[ J + 2 ] += F; } \
//T[6] += Tot; { float x = Lns[LT] - Tot; T[7] += x; } } { float x = Mat[LT][0]; T[9] += x; T[10] += x; \
//{ float x2 = Mat[LT][1]; T[11] += x2; { float x3 = Lns[LT] - ( x + x2 ); T[12] += x3; } } }

//Remove: using procedure GapR530() instead.
//#define GapR530(LT) { float F = Lns[LT]; T[0] += F; { float I = LnInt[LT]; T[1] += I; } } \
//{ float Tot = 0; Loop(Qtrs) { float F = Sensi[LT][J]; Tot += F; T[ J + 2 ] += F; } \
//T[6] += Tot; { float x = Lns[LT] - Tot; T[7] += x; } } { float x = Sensi[LT][0]; T[9] += x; T[10] += x; \
//{ float x2 = Sensi[LT][1]; T[11] += x2; { float x3 = Lns[LT] - ( x + x2 ); T[12] += x3; } } }

	memset(Sensi, 0, sizeof(Sensi));
    {
      //Loop(MaxL)
	  for(int J = 0; J < MaxL; J++)
      {
        long LT = J;
        LoanT *p = pLoans[Bank][LT] - 1;
        long RecCnt = (long)LnG->LnCnt[LT];
        long NoI = IsClsII(LT) || IsClsIII(LT), NoP = LT == 13;
        if( ! IsAdjP(LT))
          continue;
        {
          //Loop(RecCnt)
		  for(int J = 0; J < RecCnt; J++)
          {
            p++;
            if(p->Reported == 8 || p->Amnt <= 0)
              continue;
            int M = p->AdjMat;
            if(M)
              --M;
            if(NoP ||  ! p->Payment)
            {
              if(M < Qtrs)
                Sensi[LT][M] += p->Amnt;
            }
            else
            {
              float Rate = p->Rate *.0025;
              long m =  - 1, Last = min(M, Qtrs);
              float Bal = FloorZ(p->Amnt);
              while(Bal && ++m < Last)
              {
                float I = NoI ? 0 : Rate *Bal, P = FloorZ(p->Payment - I);
                P = min(Bal, P);
                Sensi[LT][m] += P;
                Bal -= P;
              }
              if(m < Qtrs)
                Sensi[LT][m] += Bal;
            }
          }
        }
      }
    }
	
    //GapV530(0)GapR530(1)GapV530(2)GapR530(3)GapV530(4)GapR530(5)GapV530(6)GapV530(7)GapR530(8)GapR530(9)GapN530(10)GapV530(11)GapV530(12)GapR530(13)GapV530(14)GapN530(15)
	GapV530(0, T);  //National Corp Credit Line
	GapR530(1, T, Sensi); //National Corp Term
	GapV530(2, T);  //Middle Market Credit Line
	GapR530(3, T, Sensi);  //Middle Market Term
	GapV530(4, T);  //Small Business Credit Line
	GapR530(5, T, Sensi);  //Small Business Term
	GapV530(6, T);  //Trade
	GapV530(7, T);  //Real Estate Construction
	GapR530(8, T, Sensi);  //Real Estate Commercial
	GapR530(9, T, Sensi);  //Real Estate Multi-family
	GapN530(10, T);  //1-4 family fixed rate
	GapV530(11, T);  //1-4 family variable rate
	GapV530(12, T);  //Real Estate Home Equity
	GapR530(13, T, Sensi); //Consumer Personal
	GapV530(14, T); //Consumer Credit Card
	GapN530(15, T);  //Consumer Installment
  }
  {
    // Calculate the first 3 rows in B531. 
    AcT *Ac = BOA[Bank] + YrsQtr;
    BalT *Bal = &Ac->Bal;
    AssetsT *Ass = &Bal->Assets;
    LiaT *Lia = &Bal->Liabilities;
    DepLiaT *LiaD = &Lia->Deposits;
    DemandT *Dem = &LiaD->Demand;
    IncomeOutGoT *Incm = &Ac->IncomeOutGo;
    InterestT *Int = &Incm->Interest;
    ExpT *Exp = &Int->Expenses;
    BOA_DepT *DepE = &Exp->BOA_Dep,  *DepL = &LiaD->BOA_Dep;
    FundsT *FnsE = &Exp->Funds,  *FnsL = &Lia->Funds;
    float *WithK = DepK.NonTimeWith;
    SecSumT *SecPort = SecGlo[Bank].Sec;
    float T[13];
    memset(T, 0, sizeof(T));
    {
      float F = FnsL->FRB + FnsL->Repo + FnsL->FF;
      T[0] += F;
      {
        float I = FnsE->FRB + FnsE->Repo + FnsE->FF;
        T[1] += I;
      }
      T[2] += F;
      T[6] += F;
      T[8] += F;
      T[10] += F;
    }
    {
      float *R = D531[Bank].FnsHF[0];
      {
        float F =  *R;
        T[0] += F;
        {
          float I = R[1];
          T[1] += I;
        }
        T[2] += F;
        T[6] += F;
      }
      {
        float x = R[2];
        T[8] += x;
        T[10] += x;
      }
      {
        float x = R[3];
        T[11] += x;
      }
      {
        float x = R[4] + R[5];
        T[12] += x;
      }
    }
    {
      float *R = D531[Bank].FnsHF[1];
      {
        float F =  *R;
        T[0] += F;
        DotLen = 0;
      }
      {
        float I = R[1];
        T[1] += I;
        DotLen = 1;
      }
      {
        float Tot = 0;
        //Loop(Qtrs)
		for(int J = 0; J < Qtrs; J++)
        {
          float F = R[2+J];
          Tot += F;
          T[J + 2] += F;
        }
        T[6] += Tot;
      }
      {
        float x = R[2];
        T[8] += x;
        T[10] += x;
      }
      {
        float x = R[3];
        T[11] += x;
      }
      {
        float x = R[4] + R[5];
        T[12] += x;
      }
    }
//Remove: using procedure GapM531() instead
//#define GapM531( Prod, KP )                                             \
//{ float Bal = DepL->Prod, K = KP; T[0] += Bal;    \
//{ float I = DepE->Prod; T[1] += I;  }        \
//T[2] += Bal; T[6] += Bal;     \
//{ float x = Bal * K; T[9] += x; T[10] += x;      \
//{ float x2 = Bal * K * (1 - K); T[11] += x2;          \
//{ float x3 = Bal - ( x + x2 ); T[12] += x3; }       \
//}                                                              \
//}                                                                \
//}
    //GapM531(Now, WithK[IndNOWE])GapM531(Sav, .9 *WithK[IndSaveE] + .1 * WithK[BusSaveE])GapM531(MonMkt, WithK[IndMonE])
  GapM531(DepL->Now, DepE->Now, WithK[IndNOWE], T);  //Now accounts in B531
	GapM531(DepL->Sav, DepE->Sav, .9 *WithK[IndSaveE] + .1 * WithK[BusSaveE], T);  //Saving accounts
	GapM531(DepL->MonMkt, DepE->MonMkt, WithK[IndMonE], T);  //Money Market accounts

//Remove: using procedure GapM531b() instead.
//#define GapM531b( C, D, TR )                                                               \
//{ float (*R)[C] = D[Bank], F = R[TR][0]; T[0] += F;              \
//{ float I = R[TR][3]; T[1] += I;  }                \
//{ float Tot = 0;                                                                    \
//Loop(Qtrs) { float F = R[ 1 + J ][0]; Tot += F; T[ J + 2 ] += F; }  \
//T[6] += Tot;  { float x = F - Tot; T[7] += x;  }                \
//}                                                                                   \
//{ float x = R[1][0]; T[8] += x;  T[10] += x;                          \
//{ float x2 = R[2][0]; T[11] += x2;                                        \
//{ float x3 = F - ( x + x2 ); T[12] += x3; }                            \
//}                                                                                 \
//}                                                                                   \
//}

	//GapM531b(B501aC, B501a, 10)GapM531b(B501bC, B501b, 6)
	
	float (*R)[B501aC] = B501a[Bank];
	GapM531b(R, 10, T);

	//HACK: how would I use the same procedure from above?
	float (*R1)[B501bC] = B501b[Bank];
	GapM531b1(R1, 6, T);
    
		{ //Retail Cert-Var row
      long TR = 10;
      float(*R)[Max423Cols] = BalSumByMat, F = R[TR][0];
      T[0] += F;
      {
        float I = IntSumByMat[TR][0] *.0025;
        T[1] += I;
      }
      T[2] += F;
      T[6] += F;
      {
        float x = R[TR][1];
        T[9] += x;
        T[10] += x;
        {
          float x2 = R[TR][2];
          T[11] += x2;
          {
            float x3 = F - (x + x2);
            T[12] += x3;
          }
        }
      }
    }
    {  //Retail Cert-Fix
      long TR = 9;
      float(*R)[Max423Cols] = BalSumByMat, F = R[TR][0];
      T[0] += F;
      {
        float I = IntSumByMat[TR][0] *.0025;
        T[1] += I;
      }
      {
        float Tot = 0;
        //Loop(Qtrs)
		for(int J = 0; J < Qtrs; J++)
        {
          float F = R[TR][1+J];
          Tot += F;
          T[J + 2] += F;
        }
        T[6] += Tot;
        {
          float x = F - Tot;
          T[7] += x;
        }
      }
      {
        float x = R[TR][1];
        T[9] += x;
        T[10] += x;
        {
          float x2 = R[TR][2];
          T[11] += x2;
          {
            float x3 = F - (x + x2);
            T[12] += x3;
          }
        }
      }
    }
    {  //Long Term Debt HFB-Var Rt
      float *R = D531[Bank].LongHF[0];
      {
        float F =  *R;
        T[0] += F;
        {
          float I = R[1];
          T[1] += I;
        }
        T[2] += F;
        T[6] += F;
        T[12] += F;
      }
    } 
    {  //Long Term Debt HFB-Fix Rt
      float *R = D531[Bank].LongHF[1];
      {
        float F =  *R;
        T[0] += F;
        {
          float I = R[1];
          T[1] += I;
        }
        T[7] += F;
        T[12] += F;
      }
    }
    {   // Subord Debt 
      {
        float F = Lia->SubLTD;
        T[0] += F;
        {
          float I = Exp->SubLTD;
          T[1] += I;
        }
        T[7] += F;
        T[12] += F;
      }
    }

//Remove: using GapM531c() instead.
//#define GapM531c(x) { float f = Ass->Total - *x;  *x += f;  x[7] += f;  x[12] += f;  }
	
	//GapM531c(D530)
	GapM531c(D530, Ass->Total);
    
	{
      float B = Dem->Bus, B2 = Dem->Pub, B3 = Dem->Indiv, Bal = B + B2 + B3, K = WithK[DemIndE], K2 = WithK[DemBusE], K3 = WithK[DemPubE];
      T[0] += Bal;
      T[7] += Bal;
      {
        float x = B * K + B2 * K2 + B3 * K3;
        T[9] += x;
        T[10] += x;
        {
          float x2 = B * K *(1-K) + B2 * K2 *(1-K2) + B3 * K3 *(1-K3);
          T[11] += x2;
          {
            float x3 = Bal - (x + x2);
            T[12] += x3;
          }
        }
      }
    }
    //GapM531c(T)
	GapM531c(T, Ass->Total);
    {
      Loop(13)
      {
        T[J] = D530[J] - T[J];
      }
    }
    {
      float N[7];
      memset(N, 0, sizeof(N));

//Remove: using procedure GapM531d() instead.
//#define GapM531d( t )                                                          \
//{ float S[7]; long Neg = t == SwapVP; S[0] = SecPort[t].Interest;   \
//{ float Tot = 0;                                                      \
//Loop(3) { float F = SecPort[t].MatDistrib[ J + 1 ].Par;        \
//if ( Neg ) F *= -1; Tot += F; S[ J + 2 ] = F;                   \
//}                                                                   \
//S[6] = ( Neg ? -1 : 1 ) * ( SecPort[t].EOQ.Par - SecPort[t].MatDistrib[0].Par) - Tot; \
//S[1] = -Tot - S[6];  S[5] = -S[6];                                  \
//}                                                                     \
//{ Loop(7) { N[J] += S[J]; } }         \
//}

	  //GapM531d(SwapFP)GapM531d(SwapVP)
	  GapM531d(SwapFP, SecPort, N);
	  GapM531d(SwapVP, SecPort, N);
	  
	  Gap = (N[1] + T[1+1]) / Ass->Total;
      Gap4Qtr = (N[5] + T[5+1]) / Ass->Total;
    }
  }
}




const float Mkt_Bk_Ratio = 1.25;
float AttS[MaxB][MaxStPrAtr + 1];
float Att[MaxB][MaxStPrAtr]; // Stock price factors
long TooLittleCapForIssue[MaxB];
/// <summary> This is the main function for Stock Price and Credit Quality inside RunForward().  </summary>
/// <remarks> Ax a collection of bank attributes, which determine the stock price. </remarks> 
void Stock(void)
{
	extern float ShrtSecs[MaxB]; //from Securities.
	extern long L4[5];
	extern float NewDep[MaxB]; 

  AcT *Ac = BOA[Bank] + YrsQtr;
	AcT *Ac2 = BOA[Bank] + L4[1];
	AcT *Ac3 = BOA[Bank] + L4[2];
  IncomeOutGoT *Incm = &Ac->IncomeOutGo;
	IncomeOutGoT *Incm2 = &Ac2->IncomeOutGo;
  BalT *Bal = &Ac->Bal;
  AssetsT *Ass = &Bal->Assets;
  LiaT *Lia = &Bal->Liabilities;
	LiaT *Lia2 = &Ac2->Bal.Liabilities;
  StkPrT *K = &TreasK.StkPr;
  LoanRptT *LnR = LoanRpt + Bank;
  LoanGloT *LnG = LoanGlo + Bank;
  TreasDecT *Trs = TreasDec + Bank;
  FundsGloT *Fns = FundsGlo + Bank;
  float Earnings = Incm->Nets.Income;
	float Shares = Lia->Stock.Common / PAR;
	float Equ = Lia->_Equity;
	float BkShr = Equ / Shares;
	float Div = Ac->Div;
	float LnSum = Ass->Loans.Sum;
	float *Ax = Att[Bank];
	float Sum = Mkt_Bk_Ratio;
  if( ! YrsQtr)
    Ac->DivYTD = 0;
  Ac->DivYTD += Div;
	// Ax points to bank attributes, which include Captial (0,1), Asset Quality (2-5), 
	// Management (6-11), Liquidity (12-14), Divident Policy (15,16), Growth (17,18)
  Ax[0] = Equ / Ass->Total; // Equity/Total asset
  Ax[1] = (Equ + Lia->SubLTD + Ass->LLR) / Ass->Total; // Total Capital/Risk Assets
  {
    float N = 0;
		float *p = LnR->ChrgOffAmnt;
    Loop(MaxL)N += p[J];
    Ax[2] = (Ac->NetChargeOffs = N) / LnSum; // Loan Chargeoffs/Total Loans
  }
  if( ! YrsQtr)
    Ac->LnChrgOffsYTD = 0;
  Ac->LnChrgOffsYTD += Ac->NetChargeOffs;
  Ax[3] = Ass->LLR / LnSum; // Substandard Loans(CQ=7)/Total Loans
  {
    float *p = LnR->PastDueAmnt,  *q = LnG->NAT, P = Ac->PastDue = 0;
    //Loop(MaxL)
	for(int J = 0; J < MaxL; J++)
    {
      P += p[J] + q[J];
      Ac->PastDue += p[J];
    }
    Ax[4] = (Ac->NonPerformLns = P) / LnSum; // (Past Due+Non-accrual Loans)/Total Loans
  }
  Ax[5] = Ac->UnRlzd / Equ; // (Market value - Book Value)of Securities/Equity
  {
    StkEarnT *p = &K->Earn;
    float A = p->Abs;
		float D = Trs->Forecast - Earnings / Shares;
		// EPS Forecast: The accuracy of a bank's forecast of earnings per share for the current quarter.
    Ax[6] = D > A ? p->Hi: (D <  - A ? p->Low: p->On); 
  }
  {
    float w = K->WghtGap;
		// GenGap() sets Gap and Gap4Qtr
    GenGap();
		// Rate-sensitivity Gap/Total Assets: The gap measure used is a weighted average of the 1-quarter gap
		// and 4-quarter cumulative gap as reported to the bank. The weighting used is 
		// "w" times the 1-quarter gap + (1-w)times the 4-quarter gap.
    Ax[8] = w * Gap + (1-w) *Gap4Qtr; 
  }
  {
    float c = Ac->NetChargeOffs;
    Ax[9] =  ! c ? 0 : Ass->LLR / c; //Loan loss reserve/Chargeoffs
  }
  {
    float T = EconBOQ.LowerEcon.TaxRate * .01;
		float TOE = Incm->OpEarnings + Incm->TEFRA - Incm->Interest.Income.Securities.Munis;
		float NOE = Incm->OpEarnings - T * TOE;
		float NOEPS = NOE / Shares;
    Ac->NOEPS = NOEPS; //Net Operating Earning Per Share
    Ac->EarnRate = NOE / Ac2->CoreDep;
    {
      float PrdErn = Ac->CoreDep *.5 *(Ac2->EarnRate + Ac->EarnRate);
			float PrdErnShr = PrdErn / Shares;
			float k = K->Asst *(1-T);
			float AdjNet = Earnings - k * Incm->Nets.AssetSales;
			float AdjNet1 = Incm2->Nets.Income - k * Incm2->Nets.AssetSales;
			float AvgErn = (1.1 *PrdErn + 1.1 * AdjNet + .8 * AdjNet1) / 3;
      {
        float Y[4]; // Earning Volatility
        Y[0] = Ac3->NOEPS;
        Y[1] = Ac2->NOEPS;
        Y[2] = NOEPS;
        Y[3] = PrdErnShr;
        {
          float SumY = 0, SumTY = 0, SumT = 0, SumT2 = 0, SumY2 = 0,  *p = Y - 1;
          Loop(4)
          {
            p++;
            SumY +=  *p;
            SumTY += J * *p;
            SumT += J;
            SumT2 += J * J;
            SumY2 +=  *p * *p;
          }
          {
            float n = 4;
			float x1 = (SumT *SumY - n * SumTY) / (SumT *SumT - n * SumT2);
			float vY = n * SumY2 - SumY * SumY;
			float z = n * SumTY - SumT * SumY;
			float q = n * SumT2 - SumT * SumT;
			float x2 = z * z / (vY *q);
			float UXV = (1-x2) *vY / (n *n);
            Ax[7] = sqrt(fabs(UXV)) / BkShr; //Earning Volatility: this is a measure of earning predictability over the past four quarters.
            Ax[17] = x1 / BkShr; //Earnings growth measure
          }
        }
      }
      Ax[10] = AvgErn / Ac->AveBal.Assets.Total; //Return on assets
      {
        // Per George, remove Ac->AveBal.Liabilities._Equity from the assignment.
        //float a = Ac->AveBal.Liabilities._Equity = .5 *(Lia2->_Equity + Equ + Fns->Day1In.Stock - Fns->Day1Out.Stock);
        float a = .5 *(Lia2->_Equity + Equ + Fns->Day1In.Stock - Fns->Day1Out.Stock);
				Ax[11] = AvgErn / a; //Return on equity
      }
      Ax[18] = (PrdErn - NOE) / Equ; //Change in earnings(1 quarter-projected)
    }
  }
	// Asset Liquidity - Ax[12-14]
  Ax[12] = (Ass->ffSold + Ac->Unpledged + ShrtSecs[Bank]) / Ass->Total; //Net available assets/Total assets
  {
    FundsT *LiaF = &Lia->Funds;
    Ax[13] = LiaF->FF / Ass->Total; //Borrowed Funds/Total Assets
  }
  {
    float N = 0;
		float *p = LnR->New;
    Loop(MaxL)N += p[J];
		// ?? question for chip. the calc does not match the document
    Ax[14] = (NewDep[Bank] - N) / Ass->Total; //New Loans/New Core Deposits
  }
	// Asset Divident Policy - Ax[15,16]
  Ax[15] = Div / BkShr; //Divident yield=Current dividents per share/Book Value per share
  {
    //float O = Ac2->SmthDiv; //Smooth divident is the weighted average of last 4 quarter divident
    //if(Div > 0 &&  ! O)
		float Div_Smoothed_Prior_Quarter = Ac2->SmthDiv;
		if(Div > 0 &&  ! Div_Smoothed_Prior_Quarter)
    {
      Ax[16] = .05; 
      Ac->SmthDiv = Div;
    }
    else
    {
      float w = K->WghtDiv;
			//float N = Ac->SmthDiv = w * Div + (1-w) *O;
			float Div_Smoothed_This_Quarter = Ac->SmthDiv = w * Div + ( 1 - w ) * Div_Smoothed_Prior_Quarter;
			float Div_Increase = Div_Smoothed_This_Quarter - Div_Smoothed_Prior_Quarter ;

      //Ax[16] =  ! O ? 0 : (Bound(N, 0., Ac->NOEPS) - O) / (O + .25); 
			Ax[16] =  ! Div_Smoothed_Prior_Quarter || Div > Ac->NOEPS &&  Div_Increase > 0
       ? 0 : Div_Increase / ( Div_Smoothed_Prior_Quarter + .25 ); //Growth in divident per share
    }
  }

  // Stock Price
  {// Attrs[Bank][J] = Ax[J] - K->Attr->Targ
    StkAttrT *p = K->Attr - 1; //K is TreasKT. K->Attr has Targ, Pos, Neg, Max and Min
    //Loop(MaxStPrAtr)
	for(int J = 0; J < MaxStPrAtr; J++)
    {
      p++;
      if(J == 6)
      {
        Sum += AttS[Bank][J] = Ax[6];
        continue;
      }
			// Fx is the difference between the true value and target value for an attribute.  
      {
        float Fx = Ax[J] - p->Targ;
        Fx *= Fx >= 0 ? p->Pos: p->Neg;
        Sum += AttS[Bank][J] = Bound(Fx, p->Min, p->Max);
      }
    }
  }
  {//Stock Price, BkShre and AttS[Bank][19] = Sum
    float w = K->WghtStkPr;
		float Price = (BkShr *Sum) *w + (1-w) *Ac2->StkPrice;
		float Min = K->Min *BkShr + Div;
    Ac->StkPrice = Bound(Price, Min, K->Max *BkShr);
    Ac->_Book = BkShr; //book value per share
    AttS[Bank][19] = Sum;
  }
  // Subordinated Capital Notes = debt. 
	// Calculate the interest rate of debt 
  {
    float Issue = Trs->Plan;
		float FinalDebt = Lia->SubLTD + Issue;
		float TotCap = FinalDebt + Equ;
		float *K = TreasK.SubLTD_IssuedRate;
		float Cap =  ! TotCap ? 0 : FinalDebt / TotCap;
    long f = Cap > K[12];
    TooLittleCapForIssue[Bank] = 0;
    if( ! Issue || f)
    {
      //  Issue-Plan Rejected
      if(f)
        TooLittleCapForIssue[Bank] = Round(100 *Cap);
    }
		// Calculate a weighted sum of the attributes: 
    else
    {
      float Sum = 0;
      {
        static long in[] =
        {
          0, 1, 2, 3, 4, 5, 8, 10, 17
        };
        Loop(9)Sum += K[J] *Ax[in[J]];
      }
      {
        EconT *E = &EconEOQ.Econ;
        SubLTD_IssuedT *Av = Fns->SubLTD_IssuedAvail + 2;
				//Calculate the Spread which is the difference between the AAA and BBB bond rates taken from the economy common
        float AAA = E->BondAAA;
				float BBB = E->BondBB;
				float Spread = BBB - AAA;
				//Add the result (spread * sum) to the AAA bond rate and then add three additional factors 
				//the first factor may reflect issue costs
				//the second factor may be a capital structure risk factor
				//the third factor may be simple maturity risk premium
        Av->Rate = min(.9 *BBB, AAA + Spread * Sum) + K[9] / Issue + Cap * Cap * K[10] + K[11]*(Av->Mat - 40);
      }
    }
  }
}

/// <summary> This function determines the amount of corporate CDs and the rate available to the bank. </summary>
void AvailableFunds(void)
{
  FnsAvlT *K = &TreasK.FnsAvl;
  AcT *Ac = BOA[Bank] + YrsQtr;
  BalT *Bal = &Ac->Bal;
  AssetsT *Ass = &Bal->Assets;
  LiaT *Lia = &Bal->Liabilities;
  FundsGloT *Fns = FundsGlo + Bank;
	//Calculate the short term risk - S.
  float S = 0;  	
  {
    float x[MaxRsk]; //MaxRsk = 6
		float *Ax = Att[Bank];
    {
      float *R = K->Rsk1;
      x[0] = R[0] - Ax[0];
      x[1] = Ax[4] - R[1];
      x[2] = fabs(Ax[8]) - R[2];
      x[3] = R[3] - Ax[10];
      x[4] = R[4] - Ax[12];
      x[5] = Ax[13] - R[5];
    }
    {
      float *R = K->Rsk2;
      Loop(MaxRsk)
				S += FloorZ(x[J]) * *R++; // S is always > 0 provided that all of constants are positive
    }
  }
  Ac->ShortTermRisk = S;
	//Calculate CD available amount and rates and save to B501a[Bank]
  { 
    float CR = MaxCR;
		float *r = K->CredRating;
    
		Loop(MaxCR)
			if(S <=  *r++)
			{
				CR = J;
				break;
			}
			Ac->CreditRating = ++CR;
   
		{
      float CDMI = EconEOQ.Econ.CDMI; //CD Market Interest
			float *aK = K->CDAvlDst;
			float R = K->CDFract[0];

			// If S >= 7.0, no CDs will be available
      float F = S >= 7.5 ? 0 : (S >= 5.5 ? R *(1. - S / 9): (S >= 3 ? R *(1. - S / 13): (S >= 1.5 ? R *(1. - S / 20): R)));
			float Fract =  ! CDMI ? 0 : F / CDMI;

      float Maturing = Fns->ZMatCDs;
			float Existing = Lia->Deposits.BOA_Dep.Jumbo;
			float A = Fract * Ass->Total - Existing + Maturing;
			float Avail = CR == 7.5 ? 0 : FloorZ(A);
      RateAmntT *a = Fns->CDsAvail - 1;
      //Loop(MaxCDA)
      for(int J = 0; J < MaxCDA; J++)
	  {
        a++;
        if( ! Avail)
        {
          a->Amnt = a->Rate = 0;
          continue;
        }
        a->Amnt = Avail * aK[J];
        {
          float Y = YldC(J + 1, EoQ);
					float R = .1 * Pow(S, 1.8) + CDMI * Y;
          a->Rate = Bound(R, 1.03 *Y, 1.4 *Y);
        }
        {
          float(*R)[B501aC] = B501a[Bank];
          R[J + 2][5] = a->Amnt;
          R[10][5] += Round(a->Amnt);
          R[J + 2][4] = a->Rate;
        }
      }
    }
  }
}