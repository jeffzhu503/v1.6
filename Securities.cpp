#include "BMSim.h"
#include <math.h> 

extern EconQtrT EconQtr8[8]; 
extern SecGloT SecGlo[MaxB]; 
extern FundsGloT FundsGlo[MaxB]; 
extern SecT Sec[MaxB][MaxSecs];
extern SecKT SecK; 
extern PricesT Prices; 
extern ECNT EconBOQ;

extern SecSaleDecT SecSaleDec[MaxB];
extern SecPurDecT SecPurDec[MaxB];

extern BOAT BOA[MaxB]; 
extern int YrsQtr, SimQtr; 
extern int Bank; 
extern float TefAdj, TaxAdj; 
extern float FHLB_Avail_BoQ[MaxB], FHLB_Avail_EoQ[MaxB];
extern long L4[5];

//int Row, Last_Rpt, Rpt_I move Row into GlobalDef.h

/// <summary> This function returns the bond yield for a given maturity, with a certain number of quarters. </summary>
/// <param name="Mat"> is the bond maturity. </param>
/// <param name="Qtr"> is the number of quarter. </param>
/// <returns> the bond yield. </returns>
/// <remarks>  This function always calls YldC() and then based on different maturity, it has different formular to calculate the yield.
/// The number of quarter decides which set of economy data to use in the formular. </remarks>
// if maturity M <=4, the yield is 0.35 x 
// See pp 23-25.
float BondYield(float Mat, int Qtr)
{
  if(Mat <= 4)
    return SecK.BondYield1[0] *YldC(1, Qtr) + SecK.BondYield1[1] *YldC(Mat, Qtr);
  if(Mat <= 20)
    return SecK.BondYield4[0] *YldC(Mat *.25, Qtr) + SecK.BondYield4[1] *YldC(Mat *.75, Qtr) + SecK.BondYield4[2] *YldC(Mat, Qtr);
  return SecK.BondYield20[0] *YldC(Mat *.25, Qtr) + SecK.BondYield20[1] *YldC(Mat *.75, Qtr) + SecK.BondYield20[2] *YldC(Mat, Qtr);
}

/// <summary> This function returns the municiple bond yield for a given maturity, with a certain number of quarters. </summary>
/// <param name="Mat"> is the municiple bond maturity. </param>
/// <param name="Qtr"> is the number of quarter. </param>
/// <returns> The municiple bond yield. </returns>
/// <remarks> The number of quarter decides which set of economy data to use in the formular. </remarks>
float MuniYield(float Mat, long Qtr)
{
  float *k = SecK.MuniYield, T = 1-EconBOQ.LowerEcon.TaxRate * .01;
  return BondYield(Mat, Qtr) *(*k * T + k[1] *min(Mat, 20) + (Mat <= 20 ? 0 : k[2]*(Mat - 20)));
}

/// <summary> This function returns the swap yield for a given maturity, with a certain number of quarters. </summary>
/// <param name="Mat"> is the Swap maturity. </param>
/// <param name="Qtr"> is the number of quarter. </param>
/// <returns> the swap yield. </returns>
/// <remarks> The number of quarter decides which set of economy data to use in the formular. </remarks>
float SwapYield(float Mat, long Qtr)
{
  EconQtrT *E = EconQtr8 + L4[Qtr];
  if(Mat <= 1)
    return E->CPR;
  float *k = SecK.SwapYield;
  return BondYield(Mat, Qtr) + (k[0] + k[1] *Mat);
}

// Returns the Mortgage backed yield for a given maturity, going Qtr quarters back.
/// <summary> This function returns the Mortgage backed yield for a given maturity, with a certain number of quarters. </summary>
/// <param name="Mat"> is the Mortgage backed bond maturity. </param>
/// <param name="Qtr"> is the number of quarter. </param>
/// <returns> the swap yield. </returns>
/// <remarks> The number of quarter decides which set of economy data to use in the formular. </remarks>
float MortYield(float Mat, long Qtr)
{
  float *k = SecK.CMO.a;
  if(Mat < 5)
    return.4 *k[1] + .5 * YldC(1, Qtr) + .5 * YldC(Mat, Qtr);
  if(Mat < 12)
    return.6 *k[1] + k[2] *YldC(1, Qtr) + k[3] *YldC(Mat / 2, Qtr) + k[4] *YldC(Mat, Qtr);
  return k[1] + k[2] *YldC(Mat / 6, Qtr) + k[3] *YldC(Mat / 2, Qtr) + k[4] *YldC(5 *Mat / 6, Qtr);
}

/// <summary> This function returns the string yield for a given maturity, with a certain number of quarters. </summary>
/// <param name="Mat"> is the Strip maturity. </param>
/// <param name="Qtr"> is the number of quarter. </param>
/// <returns> the swap yield. </returns>
/// <remarks> The number of quarter decides which set of economy data to use in the formular. </remarks>
float StripYield(float Mat, long Qtr)
{
  return YldC(Mat, Qtr) - (Mat <= 4 ? 0 : SecK.StripBuyYield / Mat);
}


//  -------  Securities (void) -----
/// <summary> This function accumulates value of cash from transactions during quarter. It adds Value.Par to SecGlo[Bank].Sec[t].Cash.</summary>
/// <param name="t"> is type of security. </param>
/// <param name="c"> is the value added to SecGlo[Bank].Sec[t].Cash. </param>
/// <remarks> It is called by Securites(). </remarks>
// One line of code.  Jeff2 suggests removing this function.
void SecCash(long t, float c)
{
  SecGlo[Bank].Sec[t].Cash += c;
}

/// <summary> This function is required to process at the beginning of quarter. </summary> 
/// <remarks> This function accumulates Par, Book, Mk_Bid in SecGlo[Bank].Sec[Type].BOQ from the values of Sec[Bank][J] based on the fund Type and whether it matures.
/// If the maturity is 0, it accumulates SecGlo[Bank].Sec[Type].Mat. It is called by Securities().</remarks>
void SecBOQ()
{
	extern long FirstEverRun; 

  SecGloT *g = SecGlo + Bank;
  SecT(*r)[MaxSecs] = Sec + Bank;
  FundsGloT *Fns = FundsGlo + Bank;
  if(FirstEverRun)
    Fns->NewUniqueName = 0;
  memset(g, 0, sizeof(SecGloT));
  {
    //Loop(MaxSecs) //loops every security available in Sec[Bank].
	for(int J = 0; J < MaxSecs; J++)
    {
      SecT *R =  *r + J;
      if( ! (R->Name))  // continue to the next if security name is not available.  
        continue;
      if(FirstEverRun && R->Name > Fns->NewUniqueName) // FirstEverRun??
        Fns->NewUniqueName = R->Name;
      {
        long Type = (long)R->Type;
        SecSumT *G = g->Sec + Type;  // G points to SecGlo[Bank][Type]
        G->BOQ.Par += R->Value.Par;  // Par is the face value
        G->BOQ.Book += R->Value.Book; // Book is another value
        G->BOQ.Mkt_Bid += R->Value.Mkt_Bid; //The current price in the market. 
        if(!R->Mat)
//          continue;
				{
					G->Mat.Par += R->Value.Par;
					G->Mat.Book += R->Value.Book;
					G->Mat.Mkt_Bid += R->Value.Mkt_Bid;
					SecCash(Type, R->Value.Par);
					memset(R, 0, sizeof(SecT));
				}
      }
    }
  }
}

//these variables are also used in B102()
long PurRptCnt[MaxB], PurRpt[MaxB][MAX_PURCHASES];
long SoldRptCnt[MaxB];
SoldRptT SoldRpt[MaxB][MAX_SALES];

/// <summary> This function processes sale decisions. It updates a bank's security portifolio based on the new security sale decision. </summary>
/// <remarks> It searches the existing securities in Sec[Bank]. If the security is found, the function updates 
/// several fields in Sec[], SecGlo[] and updates SecRpt[][].  It is called by Securities().</remarks>
void SecSell()
{
  SoldRptCnt[Bank] = 0; //This is a count for security sold for reporting purpose. 
  {
    //Loop(MAX_SALES)
	for(int J = 0; J < MAX_SALES; J++)
    {
      long DecName = (long)SecSaleDec[Bank][J].Name;
      float Amnt = SecSaleDec[Bank][J].Amnt;
      if(DecName > 0 || Amnt > 0)
      //  continue;
      {
        //Loop(MaxSecs)  //This loop searches Sec[Bank] based on security name. 
		for(int J = 0; J < MaxSecs; J++)
        {
          float Par = Sec[Bank][J].Value.Par;
          long Mat = (long)Sec[Bank][J].Mat;
		  long Name = (long)Sec[Bank][J].Name;
          if( Mat && Name == DecName)
          //  continue;
          {
            long Type = (long)Sec[Bank][J].Type;
            if(Par - Amnt <= MinSecKeep)
              Amnt = Par;
            else if(Type == MortE && Round(Par) == Amnt)
              Amnt = Par;
            {
              float Ratio = (Amnt / Par);
			  float Mkt_Bid = Sec[Bank][J].Value.Mkt_Bid *Ratio;
			  float Book = Sec[Bank][J].Value.Book *Ratio;
			  float UnRealGainOrLoss = Sec[Bank][J].UnRealGainOrLoss *Ratio;
              Sec[Bank][J].CouponIntr *= 1-Ratio;
              Par = (Sec[Bank][J].Value.Par -= Amnt);
              Sec[Bank][J].Value.Book -= Book;
              Sec[Bank][J].Value.Mkt_Bid -= Mkt_Bid;
              Sec[Bank][J].UnRealGainOrLoss -= UnRealGainOrLoss;
              SecGlo[Bank].Sec[Type].Sold.Par += Amnt;
              SecGlo[Bank].Sec[Type].Sold.Book += Book;
              SecGlo[Bank].Sec[Type].Sold.Mkt_Bid += Mkt_Bid;
              if(UnRealGainOrLoss > 0)
                SecGlo[Bank].Sec[Type].RealizedGains += UnRealGainOrLoss;
              else
                SecGlo[Bank].Sec[Type].RealizedLosses -= UnRealGainOrLoss;
              BOA[Bank][YrsQtr].IncomeOutGo.Nets.AssetSales += UnRealGainOrLoss;
              { //Update the SoldRpt.  
                SoldRptT *S = SoldRpt[Bank] + SoldRptCnt[Bank]++; //increment the sold count
                S->Name = Name;
                S->Mat = Mat;
                S->Par = Round(Amnt);
                S->GainLoss = UnRealGainOrLoss;
                S->PerOfHoldng = Round(100 *Ratio);
              }
			  //SecCash(Type, Mkt_Bid);
			  SecGlo[Bank].Sec[Type].Cash += Mkt_Bid;
              if( ! Par)  //It clears this security if the security is all sold. 
                memset(Sec[Bank] + J, 0, sizeof(SecT));
            }
			break;
          }
         // break;
        }
      }
    }
  }
}

/// <summary> This function calculates security duration.</summary>
/// <param name="Par"> is the face value of a security.  </param>
/// <param name="CouponIntr"> is the coupon interest. </param>
/// <param name="QtrDecEconYield"> </param>
/// <param name="Mat"> is the security maturity.  </param>
/// <returns> The security duration. </returns> 
/// <remarks> The measure of the price sensitivity of a fixed-income security to an interest rate change of 100 basis points. It is always called inside Securities(). <remarks>
float SecDur(float Par, float CouponIntr, float QtrDecEconYield, float Mat)
{
  if(Par < 0 || CouponIntr < 0 || QtrDecEconYield <= 0)
    return UnSet;
  if(Mat <= 0)
    return 0;
  if( ! CouponIntr)
    return Mat;
  {
    float MkFac = 1+QtrDecEconYield, Bal = Par / CouponIntr, A = MkFac / QtrDecEconYield, B = Pow(MkFac, Mat) + Bal * QtrDecEconYield - 1, C = Bal * MkFac + Mat *(1-Bal * QtrDecEconYield);
    if( ! B)
      return UnSet;
    return A - C / B;
  }
}

/// <summary> This function calculates the mortgage prepayment. </summary> 
/// <param name="F"> is a floating point number. </param>
/// <param name="Mat"> is the maturity. </param>
/// <param name="c"> is the coupon yield.</param>
/// <param name="Yield"> is the yield. </param>
/// <param name="PrePayP"> is the prepayment. </param>
float MortPay(float F, long Mat, float c, float Yield, float *PrePayP)
{
  CMOT *K = &SecK.CMO;
  float PrePay = K->PrePay;
  if(F < 0)
    return 0;
  {
    float Del = c - Yield;
    if(Del > K->a[6])
      PrePay += K->a[7]*(Del - K->a[6]);
    if(Del < K->a[8])
      PrePay -= K->a[9]*(Del + K->a[8]);
  }
  if(PrePay > K->a[9])
    PrePay = K->a[9];
  if(PrePay < K->a[10])
    PrePay = K->a[10];
  if(PrePayP)
    PrePay =  *PrePayP = K->a[11] *PrePay + (1-K->a[11]) **PrePayP;
  {
    float c1 = 1+c, R = F * c / (1-Pow(c1,  - (Mat + 1)));
    return min(F, R - c * F + PrePay *(F *c1 - R));
  }
}

/// <summary> This function calculates yield for new security purchased. </summary>
/// <param name="i"> is the security index in Sec[Bank][i]. </param>
/// <param name="Pur"> is the quarter number that the security has been purchased. It is 1 if just purchased in the current quarter.</param>
/// <remarks> It sets several fields including Value.Book, Value.Mkt_Bid, Pur, TaxEquivYield, UnRealGainOrLoss in Sec[Bank][i]. </remarks>
void SetSec(int i, int Pur)
{
  SecT *p = Sec[Bank] + i;
  int T = (int)p->Type, Mat = (int)p->Mat;
  float YM;
  switch(T)
  {
    case BillsE:
      YM = YldC(Mat, Pur);
      break;
    case StripsE:
      if(Pur)
      {
        YM = StripYield(Mat, BoQ);
        break;
      }
      YM = YldC(Mat, EoQ) + SecK.StripYield[0] *Pow(Mat, SecK.StripYield[1]);
      break;
    case BondsE:
      YM = BondYield(Mat, Pur);
      break;
    case MunisE:
      YM = MuniYield(Mat, Pur);
      break;
    case MortE:
      YM = MortYield(Mat, Pur);
      if( ! Pur)
        YM += SecK.CMO.a[5]*(p->CouponYld / YM - 1)*(YM - p->EconYield);
      break;
    case SwapFP:
    case SwapVP:
      YM = SwapYield(Mat, Pur);
  }
  if( ! Pur)
  {
    float Dur = p->Dur;
    YM = SecK.BidAdj[T][0] *YM;
    if(Dur)
      YM += SecK.BidAdj[T][1] / Dur;
  }
  p->EconYield = YM;
  if(Pur)
    p->CouponYld = YM;
  p->TaxEquivYield = T != MunisE ? UnSet : (p->CouponYld - 400 * TefAdj) / TaxAdj;
  YM *= .0025;
  switch(T)
  {
    case BillsE:
    case StripsE:
      p->Value.Mkt_Bid = p->Value.Par / Pow(1+YM, Mat);
      if(Pur)
        p->Value.Book = p->Value.Mkt_Bid;
      else
        p->Value.Book += (p->Value.Par - p->Value.Book) / (Mat + 1);
      p->Dur = Mat;
      break;
    case BondsE:
    case MunisE:
      {
        float Par = p->Value.Par;
        if(Pur)
          p->Value.Book = p->Value.Mkt_Bid = Par;
        else
        {
          float Fac = Pow(1+YM,  - Mat);
          p->Value.Mkt_Bid = Par * Fac + ((1-Fac) / YM) *p->CouponIntr;
        }
        p->Dur = SecDur(p->Value.Par, p->CouponIntr, YM, Mat);
      }
      break;
    case MortE:
      {
        ValueT *v = &p->Value;
        float c = p->CouponYld *.0025, F = v->Par;
        p->CouponIntr = c * F;
        if(Pur)
        {
          p->PrePay = SecK.CMO.PrePay;
          v->Book = F;
          v->Mkt_Bid =  - 1;
          break;
        }

        if(v->Mkt_Bid !=  - 1)
        {
          float P = MortPay(F, Mat + 1, c, YM, &p->PrePay);
          F -= P;
          SecGlo[Bank].Sec[MortE].Repayment += P;
          SecCash(T, P);
        }

        if( ! F)
        {
          memset(p, 0, sizeof(SecT));
          break;
        }
        v->Par = v->Book = F;

        if( ! F)
          break;
        {
          float PrePay = p->PrePay, M = 0, D = 0, y1 = 1+YM, e1 = 1-PrePay, c1 = 1+c, R = e1 * F * c / (1-Pow(c1,  - Mat));
          //Loop(Mat)
		  for(int J = 0; J < Mat; J++)
          {
            long t = J + 1;
            float Ft = Pow(c1 *e1, J) *F - Pow(e1, J)*(Pow(c1, J) - 1) *R / c, Xt = Pow(e1, t) *R + PrePay * c1 * Ft, m = Xt / Pow(y1, t);
            M += m;
            D += t * m;
          }
          v->Mkt_Bid = M;
          p->Dur = M ? D / M: UnSet;
        }
      }
      break;
    case SwapFP:
    case SwapVP:
      float Compounded = Pow(1+YM, Mat), PresValAnnuity = YM < 0 || Mat <= 0 ? 0:  ! YM ? Mat: (Compounded - 1) / (YM *Compounded);
      p->Value.Mkt_Bid = (T == SwapVP ?  - PresValAnnuity: PresValAnnuity)*(p->CouponYld *.0025 - YM) *p->Value.Par;
      p->Dur = T == SwapVP ? UnSet : SecDur(0, p->CouponIntr, YM, Mat);
      break;
  }
  p->UnRealGainOrLoss = p->Value.Mkt_Bid - p->Value.Book;
}

/// <summary> This function processes purchase decisions. It sets a bank's security portifolio Sec[Bank] and SecGlo[Bank] 
/// based on the new security purchase purchase decision. </summary>
/// <remarks> It is called by Securities().</remarks>
void SecPur(void)
{
  float MaxTE[MaxTE_MatAvail];
  FundsGloT *Fns = FundsGlo + Bank;
  Loop(MaxTE_MatAvail)MaxTE[J] = Prices.TE_AvlAmnt; //TE_AvlAmnt is Bank Qualified Tax Exempt Securities Available: (5 million dollars)  
  PurRptCnt[Bank] = 0; //a count for security purchase
  {
    //Loop(MAX_PURCHASES)
	for(int J = 0; J < MAX_PURCHASES; J++)
    {
      long Mat = (long)SecPurDec[Bank][J].maturity;
	  long Amnt = (long)SecPurDec[Bank][J].amount;
	  long Type = (long)SecPurDec[Bank][J].type;
      if(Amnt <= 0 || Mat <= 0 || Type < 0 || Type > SF)
        continue;
      if(Type == TX) // Tax Exempt (Muni)
      {
        long f =  - 1;
        Loop(MaxTE_MatAvail)if(Mat == EconBOQ.LowerEcon.TE_MatAvail[J])
          f = J;
        if(f ==  - 1 ||  ! MaxTE[f])
          continue;
        Amnt = min((long)MaxTE[f], Amnt);
        MaxTE[f] -= Amnt;
      }
      {
        //Loop(MaxSecs)
		for(int J = 0; J < MaxSecs; J++)
        {
          if(Sec[Bank][J].Name)
            continue;
          Sec[Bank][J].Name = ++Fns->NewUniqueName;  //This sets the unique fund name to a newly purchased security. 
          Sec[Bank][J].Mat = Mat;
          Sec[Bank][J].Value.Par = Amnt;
          PurRpt[Bank][PurRptCnt[Bank]++] = J;
          SecGlo[Bank].Sec[Type].Pur.Par += Amnt;
          switch(Type)
          {
            case MB:
              Type = MortE;
              break;
            case SV:
              Type = SwapVP;
              break;
            case SF:
              Type = SwapFP;
              break;
          }
          Sec[Bank][J].Type = Type;
          Sec[Bank][J].PurDate = SimQtr;
          SetSec(J, 1);
          SecGlo[Bank].Sec[Type].Pur.Book += Sec[Bank][J].Value.Book;
          SecGlo[Bank].Sec[Type].Pur.Mkt_Bid += Sec[Bank][J].Value.Book;
          SecCash(Type,  - Sec[Bank][J].Value.Book);
          if(Type != MortE)
            Sec[Bank][J].Value.Mkt_Bid = Sec[Bank][J].Value.Book;
          Sec[Bank][J].CouponIntr = Amnt * Sec[Bank][J].CouponYld *.0025;
          break;
        }
      }
    }
  }
}

/// <summary> This function calculates security interest. It adds the interest to the SecGlo[Bank].Sec[t].Interest and
///  4 fields (Treasuries, Munis, Mort and Other) in Account.IncomeOutGo.Interest.Income.Securities based on the security type. </summary>
/// <param name="t"> is the security type. </param>
/// <param name="I"> is the interest.</param>
/// <remarks> It is called by SecUpdate() by Securities().</remarks>
void SecInterest(long t, float I)
{
  AcT *Ac = BOA[Bank] + YrsQtr;
  SecuritiesT *p = &Ac->IncomeOutGo.Interest.Income.Securities;
  SecGlo[Bank].Sec[t].Interest += I;
  if(t != BillsE)
    SecCash(t, I);
  switch(t)
  {
    case BillsE:
    case StripsE:
    case BondsE:
      p->Treasuries += I;
      break;
    case MunisE:
      Ac->TaxedMuniIntTaxEqiv += I;
      p->Munis += I;
      break;
    case MortE:
      p->Mort += I;
      break;
    case SwapFP:
    case SwapVP:
      BOA[Bank][YrsQtr].IncomeOutGo.Interest.NetSwaps += I;
  }
}

/// <summary> This function updates the security interest and various fields in Sec[Bank][J] based on the maturity. </summary>
/// <remarks> It is called by Securities().</remarks>
void SecUpdate(void)
{
  //Loop(MaxSecs)
  for(int J = 0; J < MaxSecs; J++)
  {
    if( ! Sec[Bank][J].Name)
      continue;
    {
      long Type = (long)Sec[Bank][J].Type;
      float Interest;
	  float FixedInterest = Sec[Bank][J].CouponIntr; //CouponIntr is the coupon interest.
	  float VarInterest = Sec[Bank][J].Value.Par *EconBOQ.Econ.CPR * .0025; //CPR is the commercial paper rate.

      if(Type == MortE)
        FHLB_Avail_BoQ[Bank] += Sec[Bank][J].Value.Book;

      switch(Type)
      {
        case SwapFP:
          Interest = FixedInterest - VarInterest;
          break;
        case SwapVP:
          Interest = VarInterest - FixedInterest;
          break;
        default:
          Interest = FixedInterest;
      }
      SecInterest(Type, Interest);
      if(Sec[Bank][J].Mat)
        Sec[Bank][J].Mat--;
      if(Sec[Bank][J].Mat)  //if maturity is not equal to 0. 
        SetSec(J, 0);
      else
      {
        Sec[Bank][J].Value.Book = Sec[Bank][J].Value.Mkt_Bid = Type == SwapFP || Type == SwapVP ? 0 : Sec[Bank][J].Value.Par;
        Sec[Bank][J].TaxEquivYield = Sec[Bank][J].EconYield = UnSet;
        Sec[Bank][J].Dur = UnSet;
        Sec[Bank][J].UnRealGainOrLoss = 0;
      }
    }
  }
}

/// <summary> This function adds the value to BOA[Bank][YrsQtr].Bal.Assets.Securities based on the security type. </summary>
/// <param name="t"> is the security type. </param>
/// <param name="A"> is the security Book value. </param>
/// <param name="P"> is the security Par value.</param>
/// <remarks> Security Asset uses book value for Bill or Bond, uses par value for Strip, Muni and Mort. </remarks>
void SecAss(long t, float A, float P)
{
  SecuritiesT *p = &BOA[Bank][YrsQtr].Bal.Assets.Securities;
  switch(t)
  {
    case BillsE:
    case BondsE:
      BOA[Bank][YrsQtr].Unpledged += P;
    case StripsE:
      p->Treasuries += A;
      break;
    case MunisE:
      p->Munis += A;
      break;
    case MortE:
      p->Mort += A;
  }
}

float ShrtSecs[MaxB];  //also used in Stocks()
// {0,1,2,3,4,6,8,12,20,32, + };
/// <summary> This function determines the security maturity for reporting. </summary>
/// <param name="Mat"> is the maturity. </param>
/// <returns> the index of MatClass based on the maturity, which is 0,1,2,3,4,6,8,12,20,32.. <returns> 
/// <remarks> It is called by SecEOQ().</remarks>
long SecMC(long Mat)
{
  Loop(MaxMC)if(Mat <= SecK.MatClass[J])
    return J;
  return J;
}

/// <summary> This function processes at the end of quarter. It sets the SecGlo->Sec->EOQ. </summary>
/// <remarks> The function updates SecGlo[Bank].EOQ, SecGlo[0].Sec[0].EOQ, SecGlo[Bank].Sec[0].MatDistrib[0],  It is called by Securities().</remarks>
void SecEOQ()
{
	extern float BillsAndBonds[MaxB];

  FundsGloT *Fns = FundsGlo + Bank;
  AcT *Ac = BOA[Bank] + YrsQtr;
  SecGloT *Rpt = SecGlo + Bank;
  Ac->QMunis = ShrtSecs[Bank] = 0; //QMunis is qualified munis.
  Ac->SwapRisk = 0;
  FHLB_Avail_EoQ[Bank] = 0;
  {
    FundsGloT *Fns = FundsGlo + Bank; //this is redundent.  
    float *b = (FeP) &Fns->Day1In,  *e = (FeP) &Fns->FRBClosed; //Federal Reserved Bank.
    memset(b, 0, (e-b) *4);
  }
  memset(&Ac->Bal.Assets.Securities, 0, sizeof(SecuritiesT));
  {//Update Funds based on the security sold, mature and purchased.
    Loop(SwapVP + 1)
    {
      SecSumT *Sum = Rpt->Sec + J; 
      Fns->Day1In.Sec += Sum->Sold.Book;
      Fns->Day1In.SecMat += Sum->Mat.Book;
      Fns->Day1Out.Sec += Sum->Pur.Book;
    }
  }
  Ac->UnRlzd = Ac->Unpledged = 0;
  {
    //Loop(MaxSecs)
	for(int J = 0; J < MaxSecs; J++)
    {
      SecT *S = Sec[Bank] + J;
      float Par = S->Value.Par, Book = S->Value.Book, Mkt_Bid = S->Value.Mkt_Bid;
      if( ! S->Name)
        continue;
      Rpt->Cnt++;
      Rpt->EOQ.Par += Par;
      Rpt->EOQ.Book += Book;
      Rpt->EOQ.Mkt_Bid += Mkt_Bid;
      {
        long Type = (long)S->Type, IsMort = Type == MortE;
        float Mat = S->Mat;
        SecSumT *Sum = Rpt->Sec + Type;
		
		//Review: warning fix...
        //if(Mat < 8 &Type >= MunisE && Type <= MortE)
		if((Mat < 8) && (Type >= MunisE) && (Type <= MortE))
          ShrtSecs[Bank] += Book;
        if(IsMort)
          FHLB_Avail_EoQ[Bank] += Book;
        Ac->UnRlzd += Mkt_Bid - Book;
        if(Mkt_Bid > 0 && (Type == SwapFP || Type == SwapVP))
          Ac->SwapRisk += Mkt_Bid;
        else
          SecAss(Type, Book, Par);
        if(Type == MunisE)
          Ac->QMunis += Book;
        Sum->Cnt++;
        Sum->EOQ.Par += Par;
        Sum->EOQ.Book += Book;
        Sum->EOQ.Mkt_Bid += Mkt_Bid;
        {
          long M = (long)S->Mat;
          ValueT *m = Sum->MatDistrib + SecMC(M);
          if(IsMort)
          {
            float c = S->CouponYld *.0025;
			// April 2009. George request the change that if the maturity is 1 there should be no calculation of prepayments.  
			if(M > 1)
			{
            //Loop(M)
				for(int J = 0; J < M; J++)
				{
				  float P = MortPay(Par, M - J, c, .0025 *MortYield(M - J, EoQ), 0);
				  {
					ValueT *m = Sum->MatDistrib + SecMC(J);
					m->Par += P;
					m->Book += P;
				  }
				  if( ! (Par -= P))
					break;
				}
			}
            Book = Par;
          }
          m->Par += Par;
          m->Book += Book;
          m->Mkt_Bid += Mkt_Bid;
        }
      }
    }
  }
  BillsAndBonds[Bank] = Ac->Unpledged;
}

/// <summary> The function processes bank securities. </summary>
/// <remarks> It is called by RunForward() and sets the security portofolio SecGlo[Bank] for a bank.</remarks>
void Securities()
{
  float TR = EconBOQ.LowerEcon.TaxRate * .01;
  AcT *Ac = BOA[Bank] + YrsQtr;
  Ac->TaxedMuniIntTaxEqiv = 0;  //Grandfather tax exmempt municipal interest.  
	// The formular for the taxable-equivalent yield on a bank-qualified bond is the pre-tax yield minus 
	// 0.2 times the bank's average interest cost of funds times the tax rate all divided by (1.0-tax ratee).
  TefAdj = SecK.TEFRA * TR * BOA[Bank][ ! YrsQtr ? 4 : YrsQtr - 1].FundsCost; //TEFRA is 0.2
  TaxAdj = 1-TR;
  memset(SecGlo + Bank, 0, sizeof(*SecGlo));
  SecBOQ();
  SecSell();
  SecPur();

  FHLB_Avail_BoQ[Bank] = 0; //Federal Home Loan Bank.  
  SecUpdate();
  SecEOQ();
  if( ! YrsQtr)
    Ac->TaxedMuniIntTaxEqivYTD = 0;
  Ac->TaxedMuniIntTaxEqivYTD += Ac->TaxedMuniIntTaxEqiv = (Ac->TaxedMuniIntTaxEqiv - TefAdj * SecGlo[Bank].Sec[MunisE].EOQ.Par) / TaxAdj;
}
