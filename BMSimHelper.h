#pragma once 

void SetTimeStamp();
void SetStateNames();

; 
//needed in B530 and defined in Loans and Stocks
//long IsClsI(long t); 
long IsClsII(long t);
long IsClsIII(long t); 
long IsAdjP(long t); 
typedef LoanT *pLoansT[MaxL]; // defines an array of pointers to the loan portfolio.

// implemented in Security and also used in reporting.
float YldC(float Mat, long Qtr);
//Defined in Securities
float MuniYield(float Mat, long Qtr); // Used in B108
float StripYield(float Mat, long Qtr);
float BondYield(float Mat, int Qtr); //also used in Treasury.
float MortYield(float Mat, long Qtr); 
float SwapYield(float Mat, long Qtr);

float DecSec(int Dec);
float DecSec2(int Dec);