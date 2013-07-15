#include "BMSimDataT.h"
#include "GlobalDef.h" 
#include "DecisionFormDataT.h"
#include <fstream>
using namespace std;

#ifndef STARTINGPOINT_H
#define STARTINGPOINT_H

LoanKonstT GetLoanKonst();
ResKonstT GetResKonst();
CostKonstT GetCostKonst();
SecKT GetSecK(); 
DepKT GetDepK();
TreasKT GetTreasK();
void GetEconList(EconT econList[], int size);
EnvT GetEnv();
PricesT GetPrices();
LowerEconT GetLowerEcon(); 
AcT GetAccounts();
void GetSec(SecT sec[]);
LoanGloT GetLoanGlo();
void GetLoan(LoanT loans[]);
void GetLnSales(LnSalesT lnSales[], int size);
ResGloT GetResGlo();
CostsGloT GetCostsGlo(); 
DepT GetDep();
void GetTimeDep(TimeDepT timeDep[], int size);
FundsGloT GetFundsGlo();

void GetSecPurDec(SecPurDecT decisions[], int total);

void PrintLoanKonst(LoanKonstT t); 
void PrintFloat(float t[], int size, ofstream& out);
void PrintNestFloat(float t[], int outer, int inner, ofstream& out);
void UpdateLoanKonstFromFile(LoanKonstT& t);
void UpdateConstFromFile(float* pData, int total, char* fileName);
void UpdateConstFromFile(float* pData, char* fileName);
void UpdateTimeDepTFromFile(TimeDepT timeDep[], int total, char* fileName);
void UpdateConstFromTimeDepFile(TimeDepT *p, char* fileName);
void UpdateConstFromFundsGloFile(FundsGloT *p, char* fileName);

LoanKonstT UpdateLoanKonst(); 

#endif