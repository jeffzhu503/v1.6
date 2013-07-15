/////////////////////////////////////////////////////////////////
// All functions are used in Calculation, Reports and UI
/////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "GlobalDef.h"

extern char LngDate[18], _LngDate[18];
extern int SupressTimeStamp;
// General data used on reports:
// General data used on reports:
extern char BankName[50], TimeStamp[88], BankNeu[20], StrA[MaxFields][FldSz], ForQtr[40], ForQtrs[60], AsOf[40], EndOf[40], Date[Qtrs][10], FQ[Qtrs + 1][10], PQ[Qtrs][10];
extern int Bank, Bank_On, ComOn, UI_Up, Drive, NumBanks, NumComs, SimQtr, SimYr, YrsQtr, ComI, rvs, len;


char *Months[] =
{
  "Jan", "Feb", "March", "April", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec"
};

/// <summary> This function sets the current date and time in TimeStamp if SupressTimeStame is not equal to true. </summary>
void SetTimeStamp()
{
  if(SupressTimeStamp)
    Str(TimeStamp, "                        ");
  else
  {
    time_t Seconds = time(0);
    struct tm *TimeRec = localtime(&Seconds);
    strcpy(TimeStamp, asctime(TimeRec));
     *strchr(TimeStamp, '\n') = 0;
  }
}

/// <summary> This function sets names for the past and future quarters in which quarter/year the community starts, e.g. Q4/09. </summary>
/// <remarks> It uses the ending date for each quarter, PQ has past 4 quarters in the year. FQ has the future 5 quarters. </remarks> 
void SetStateNames()
{
  SetTimeStamp();
  {
    long SimQ = SimQtr;

    for(int intIndexOfCount = 0; intIndexOfCount < Qtrs; intIndexOfCount++) //Set the past 4 quarters
    {
      long YrsQ = SimQ % 4, SimY = BaseSimYear + SimQ-- / 4-2000;

      if( ! intIndexOfCount)
      {
        Str(LngDate, "%d_%s_3%d", SimY + 2000, Months[(YrsQ + 1) *3-1], YrsQ == 1 || YrsQ == 2 ? 0 : 1); //2009_Dec_31

        Str(_LngDate, "%s 3%d, %d", Months[(YrsQ + 1) *3-1], YrsQ == 1 || YrsQ == 2 ? 0 : 1, SimY + 2000);  //Dec 31 2009
      }

      Str(Date[intIndexOfCount], "%2d/3%d/%02d", (YrsQ + 1) *3, YrsQ == 1 || YrsQ == 2 ? 0 : 1, SimY);  //12/31/09

      Str(PQ[intIndexOfCount], "Q%d/%02d", YrsQ + 1, SimY);   //Set PQ = each past quarter and the first index is Q4/11
    }
  }

  { //Set the future 5 quarters
    int SimQ = SimQtr;

    for(int intIndexOfCount = 0; intIndexOfCount < (Qtrs + 1); intIndexOfCount++)
    {
      int YrsQ = SimQ % 4, SimY = BaseSimYear + SimQ++ / 4-2000;

      Str(FQ[intIndexOfCount], "Q%d/%02d", YrsQ + 1, SimY);
    }
  }
}

extern EconQtrT EconQtr8[8]; 
extern long L4[5];
/// <summary> This function calculates interest rate for this maturity based on U.S. Treasury yields. </summary>
/// <param name="Mat"> is given a certain maturity. </param>
/// <param name="Qtr"> is how many quarters back. </param>
/// <returns> Interest rate based on three U.S. Treasury security including 1 quarter treasury yield, 3 quarter treasury yield and 30 years treasury yield. </returns> 
float YldC(float Mat, long Qtr)
{
  float B, C, EMat;
  EconQtrT *E = EconQtr8 + L4[Qtr];
  float p1 = E->One_Qtr_Treas, p2 = E->Three_Qtr_Treas, p3 = E->Thirty_Year_Treas;
  if(p1 <= 0 || p2 <= 0 || p3 <= 0)
    return 0;
  if(Mat <= 0)
    return 0;
  B = 1.6105 *(p3 - p1) - 4.378 *(p3 - p2);
  C = 3.7380 *(p3 - p1) - 5.680 *(p3 - p2);
  EMat = exp( - (Mat *= .5));
  return p3 + B *(1-EMat) / Mat - C * EMat;
}

/// <summary> This function returns the appropriate sector for this quarters based on the decision form. </summary>
/// <param name="Dec"> Decicion form number. </param>
/// <returns> The sector for this quarter decisions. </returns> 
float DecSec(int Dec)
{
  return 33+Dec / 2.;
}

/// <summary> This function returns the appropriate sector for next quarter based on the decision form. </summary>
/// <param name="Dec"> Decicion form number. </param>
/// <returns> The sector for this quarter decisions. </returns> 
float DecSec2(int Dec)
{
  return 50+Dec / 2.;
}
