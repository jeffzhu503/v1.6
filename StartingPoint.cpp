#include <fstream>
#include <iostream>
#include "StartingPoint.h"

using namespace std;

// This function is used for experiment purpose and currently is not called inside BMSim. 
void PrintLoanKonst(LoanKonstT t)
{
	ofstream os("LoanKonst.txt");
	PrintFloat(t.LNFCQI, sizeof(t.LNFCQI)/sizeof(float), os); 
	PrintNestFloat((float*)t.ChrgOffPor, MaxLnTypes, sizeof(t.ChrgOffPor)/MaxLnTypes/4, os);
	PrintNestFloat((float*)t.PastDuePor, MaxLnTypes, sizeof(t.PastDuePor)/MaxLnTypes/4, os);
	PrintNestFloat((float*)t.NonAccPor, MaxLnTypes, sizeof(t.NonAccPor)/MaxLnTypes/4, os);
	PrintNestFloat((float*)t.PrePay, MaxLnTypes, sizeof(t.PrePay)/MaxLnTypes/4,os);  
	PrintFloat(t.MaxMat, MaxLnTypes, os);
	os << t.RevCA << " " << t.RevCB << " " << t.RevCC << endl;
	PrintFloat(t.CurUsage, MaxLnTypes, os);
	PrintFloat(t.LoanSize, MaxLnTypes, os);
	os << t.TransBal << " " << t.MerDis << " " << endl;
	PrintFloat(t.Mat, MaxLnTypes, os);
	PrintFloat(t.Mat1, MaxLnTypes, os);
	PrintFloat(t.MatShorter, MaxLnTypes, os);
	PrintFloat(t.EfRate, MaxLnTypes, os);
	PrintFloat(t.ExpRate, MaxLnTypes, os);
	PrintFloat(t.Var, MaxLnTypes, os);
	PrintFloat(t.Wanted, MaxLnTypes, os);
	PrintFloat(t.NormProp, MaxLnTypes, os);
	PrintFloat(t.Subst, MaxLnTypes, os);
	os << t.SBTorCPL << endl; 
	PrintNestFloat((float*)t.Qual, MaxLnTypes, sizeof(t.Qual)/MaxLnTypes/4, os); 
	PrintFloat(t.MkShare1, MaxLnTypes, os); 
	PrintFloat(t.MkShare2, MaxLnTypes, os);
	PrintFloat(t.MkShare3, MaxLnTypes, os); 
	PrintNestFloat((float*)t.MkShare4, MaxLnTypes, sizeof(t.MkShare4)/MaxLnTypes/4, os); 
	PrintFloat(t.MkShare5, MaxLnTypes, os);
	PrintFloat(t.MkShare6, MaxLnTypes, os); 
	PrintNestFloat((float*)t.MkShare7, MaxLnTypes, MaxCQ, os); 
	PrintNestFloat((float*)t.MkShare8, MaxLnTypes, MaxCQ, os); 
	PrintNestFloat((float*)t.MkShare9, MaxLnTypes, MaxCQ, os); 
	PrintNestFloat((float*)t.MkShare10, MaxLnTypes, MaxCQ, os); 
	PrintNestFloat((float*)t.MkShare11, MaxLnTypes, MaxCQ, os); 
	PrintNestFloat((float*)t.RateLim, MaxLnTypes, 2, os); 
	os << t.LLExp << endl; 
	PrintFloat(t.ELOS, MaxLnTypes, os);
	PrintFloat(t.ECAS, MaxLnTypes, os);
	PrintFloat(t.PP, MaxLnTypes, os);
	os << t.MinKeepBal << " " << t.MinCreateAmnt << endl;
	PrintFloat(t.MinNewTyp, MaxL, os);
	os << t.MBStartup << endl;
	PrintFloat(t.LnSaleMatAdj, MaxL, os);
	PrintFloat(t.LnSaleCQAdj, MaxL, os);
	PrintFloat(t.LnSaleMktAdj, MaxL, os);
	os << t.Srv << " " << t.SrvFix << endl;
	PrintFloat(t.SrvVar, MaxTypsSold, os);

	os.close(); 
}

void UpdateTimeDepTFromFile(TimeDepT timeDep[], int total, char* fileName)
{
	ifstream is(fileName);
	if(is == NULL) 
		return;
	for(int i = 0; i < total; i++)
	{
		is >> timeDep[i].Id >> timeDep[i].Market >> timeDep[i].Bal >> timeDep[i].RateOrSpread >> timeDep[i].Mat 
			>> timeDep[i].IsFixed >> timeDep[i].Cnt >> timeDep[i].MktVal >> timeDep[i].MktRate >> timeDep[i].Dur; 
		for(int j = 0; j < 16; j++)
		{
			int temp; 
			is >> temp;
			timeDep[i].Por[j] = (char)temp;
		}
		is.close();
	}
}

void UpdateConstFromFile(float* pData, int total, char* fileName)
{
	ifstream is(fileName);
	if(is == NULL) 
		return;
	for(int i = 0; i < total; i ++)
	{
		is >> *pData;
		pData++;
	}
	
	is.close();
}

void UpdateConstFromFile(float* pData, char* fileName)
{
	int count = 0;
	ifstream is(fileName);
	if(is == NULL) 
		return;
	while(!is.eof())
	{
		is >> *pData;
		pData++;
		count++;
	}	
	is.close();
}

void UpdateConstFromTimeDepFile(TimeDepT *p, char* fileName)
{
	int count = 0;
	ifstream is(fileName);
	if(is == NULL) 
		return;

	while(!is.eof())
	{
		is >> p->Id >> p->Market >> p->Bal >> p->RateOrSpread >> p->Mat >> p->IsFixed >> p->Cnt >> p->MktVal >> p->MktRate >> p->Dur;

		int temp; 
		char a; 
		for(int i  = 0; i < 16; i++)
		{
		//	is >> p->Por[i]; 
			is >> temp; 
			if(temp == 0 )
			{
				p->Por[i] = 0x0; 
			}
			else
			{
				p->Por[i] = static_cast<char>(temp); 
			} 
		}
		p++;
		count++;
	}	
	is.close();
}

void UpdateConstFromFundsGloFile(FundsGloT *p, char* fileName)
{
	int count = 0;
	ifstream is(fileName);
	if(is == NULL) 
		return;

	float *pFF = &(p->Repo.Rate);  
	int numOfFloatingPoint = (sizeof(FundsGloT) - sizeof(ulong)*2 - sizeof(__int64) - 88)/4; 
	while(!is.eof() && count < numOfFloatingPoint)
	{
		is >> *pFF;
		*pFF++; 
		count++;
	}	
	is >> p->SimQtr >> p->Dirty; 
	is.close();
}

//This has been replaced by UpdateConstFromFile()
void UpdateLoanKonstFromFile(LoanKonstT& t)
{
	ifstream is("LoanKonst.txt");
	if(is == NULL) 
		return; 
	int total = sizeof(LoanKonstT)/4;
	float* pData = &t.LNFCQI[0];
	for(int i = 0; i < total; i ++)
	{
		is >> *pData;
		pData++;
	}
	
	is.close();
}

void PrintFloat(float t[], int size, ofstream& out)
{
	for(int i = 0; i < size; i++)
	{
		out << *t << " ";
		t++; 
	} 
	out << endl; 
}

void PrintNestFloat(float t[], int outer, int inner, ofstream& out)
{
	for(int i = 0; i < outer; i++)
	{
		for(int j = 0; j < inner; j++)
		{
			out << *t << " ";
			t++;
		}
		out<<endl;
	} 
}

LoanKonstT UpdateLoanKonst()
{
	LoanKonstT t; 
	float a;
	ifstream infile("StartingPoint.txt"); 
	for(int i= 0; i < 20; i++)
	{
		infile >> a;
	}
	return t;
}

LoanKonstT GetLoanKonst()
{
  // --------- SF()
  LoanKonstT LoanKonst =
  {
    // Loan quality Konst
    // LNFCQI[20] =
    {
      1, 1, 1.2, 1.2, 1.5, 1.5, 1.2, 2.5, 2, 1.7, 1, 1, 1, 1.5, 2.5, 1.8, 1, 1, 1, 1
    }
    ,
    // Charge Off Proportion Konst
    //  struct { float a,b,c,d; } ChrgOffPor[20] =
    {
      {
        1.1, 0.00009, 1.8, 1.4
      }
      ,
      {
        1.1, 0.00011, 1.8, 1.4
      }
      ,
      {
        1.2, .0001, 1.8, 1.4
      }
      ,
      {
        1.2, 0.00012, 1.8, 1.4
      }
      ,
      {
        1.3, 0.00009, 1.9, 1.5
      }
      ,
      {
        1.3, 0.00011, 1.9, 1.5
      }
      ,
      {
        1.3, 0.00009, 1.95, 1.6
      }
      ,
      {
        1.8, 0.0001, 2, 1.75
      }
      ,
      {
        1.8, 0.00007, 2, 1.75
      }
      ,
      {
        1.4, 0.0007, 1.9, 1.5
      }
      ,
      {
        1.1, 0.00003, 1.9, 1.5
      }
      ,
      {
        1.15, 0.00004, 1.9, 1.5
      }
      ,
      {
        1.2, 0.00005, 1.9, 1.5
      }
      ,
      {
        1.5, 0.00008, 2, 1.75
      }
      ,
      {
        1.7, 0.00025, 1.8, 1.4
      }
      ,
      {
        1.4, 0.00007, 2, 1.75
      }
      ,
      {
        1, 0.0001, 1.8, 1.4
      }
      ,
      {
        1, 0.0001, 1.8, 1.4
      }
      ,
      {
        1, 0.0001, 1.8, 1.4
      }
      ,
      {
        1, 0.0001, 1.8, 1.4
      }
      ,
    }
    ,
    // Past Due Proportion Konst
    //  struct { float a,b,c,d; } PassDuePor[20] =
    {
      {
        0, 1.0, 0.003,  - 1.0
      }
      ,
      {
        0, 1.0, 0.003, 0.1
      }
      ,
      {
        0, 1.1, 0.004,  - 1.0
      }
      ,
      {
        0, 1.1, 0.004, 0.12
      }
      ,
      {
        0, 1.3, 0.006,  - 0.8
      }
      ,
      {
        0, 1.3, 0.006, 0.15
      }
      ,
      {
        0, 1.3, 0.008,  - 0.8
      }
      ,
      {
        0, 1.0, 0.01,  - 2.0
      }
      ,
      {
        0.003, 1.3, 0, 0
      }
      ,
      {
        0.003, 1.4, 0, 0
      }
      ,
      {
        0.005, 1.2, 0, 0
      }
      ,
      {
        0.005, 1.2, 0, 0
      }
      ,
      {
        0.008, 1.2, 0, 0
      }
      ,
      {
        0.003, 1.2, .01,  - 2.0
      }
      ,
      {
        0.017, 1.3, 0.008, 0.1
      }
      ,
      {
        0.01, 1.2, 0.005, 0.2
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
    }
    ,
    // Nonaccrual Proportion Konst
    // struct { float a,b; } NonAccPor[20] =
    {
      {
        .75, .9
      }
      ,
      {
        .75, .9
      }
      ,
      {
        .75, .9
      }
      ,
      {
        .75, .9
      }
      ,
      {
        .75, .9
      }
      ,
      {
        .75, .9
      }
      ,
      {
        .75, .9
      }
      ,
      {
        .75, 1.5
      }
      ,
      {
        .75, 1.2
      }
      ,
      {
        .75, 1.1
      }
      ,
      {
        .75, .9
      }
      ,
      {
        .75, .9
      }
      ,
      {
        .75, .9
      }
      ,
      {
        .75, .9
      }
      ,
      {
        .75, .9
      }
      ,
      {
        .75, .9
      }
      ,
      {
        0, 0
      }
      ,
      {
        0, 0
      }
      ,
      {
        0, 0
      }
      ,
      {
        0, 0
      }
    }
    ,
    // Prepayment Konst
    // struct { float p,v,a,b,c,d; } PrePay[20] =
    {
      {
        1, 0, 0, 0.00, 0.03, 0.17
      }
      ,
      {
        1, 0, 0, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0, 0.00, 0.03, 0.17
      }
      ,
      {
        1, 0, 0, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0, 0.00, 0.03, 0.17
      }
      ,
      {
        1, 0, 0, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0, 0.00, 0.03, 0.17
      }
      ,
      {
        0, 0, 0, 0.00, 0.03, 0.17
      }
      ,
      {
        1, 0, 0.005, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0.005, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0.006, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0.006, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0.002, 0.005, 0.03, 0.17
      }
      ,
      {
        1, 0, 0.006, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0.001, 0.004, 0.03, 0.17
      }
      ,
      {
        1, 0, 0.004, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0, 0.01, 0.03, 0.17
      }
      ,
      {
        1, 0, 0, 0.01, 0.03, 0.17
      }
    }
    ,
    // Maximum Maturites
    // MaxMat[20] =
    {
      40, 60, 30, 60, 20, 40, 20, 12, 100, 100, 120, 120, 120, 20, 120, 60, 0, 0, 0, 0
    }
    ,
    // 3 review factors
    //  RevCA =  ,RevCB =  ,RevCC =  ,
    .002, .001, .0003,
    // Loan Usage Konst
    //  CurUsage[20] =
    {
      1.0, 0, 1.01, 0, 1.02, 0, 1.02, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0
    }
    ,
    // Average Loan Sizes
    //  LoanSize[20] =
    {
      1, 1, .5, .5, .2, .2, .5, 1.5, 2, 2, .14, .14, .02, .05, .002, .01, 0, 0, 0, 0
    }
    ,
    // Transfer Balance & Merchant Discount factors
    //  TransBal = ,MerDis = ,
    0.22, 0.015,
    // Normal Maturities
    //  Mat[20] =
    {
      12, 20, 8, 20, 8, 20, 4, 3, 40, 40, 120, 120, 20, 4, 8, 12, 0, 0, 0, 0
    }
    ,
    // Proportion of borrowers wanting 1 quarter loans
    //  Mat1[20] =
    {
      0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.2, 0.2, 0, 0, 0, 0, 0.1, 0.2, 0, 0, 0, 0, 0, 0
    }
    ,
    // Proportion of borrowers wanting loans with
    // up to half the Maximum Maturity
    // MatShorter[20] =
    {
      0.9, 0.9, 0.9, 0.9, 0.6, .9, 0.95, 0.9, 0.9, 0.9, 0.2, 0.2, 0.9, 1, 1, .8, 0, 0, 0, 0
    }
    ,
    // Effective Interest Rate Konst
    // EfRate[20] =
    {
      0.6, 0.4, 0.4, 0.4, 0.6, 0.6, 0.4, 0.6, 0.4, 0.3, 0.3, 0.3, 0.4, 0.5, .4, .6, 0.5, 0.5, 0.5, 0.5
    }
    ,
    // Comparative Market Rate Konst
    // CompRate[20] =
    {
      0.1, 0.5, 0.1, 0.5, 0.1, 0.5, 0.1, 0, 0.2, 0.2, 0.9, 0.2, 0.1, 0.5, 0, .3, 0, 0, 0, 0
    }
    ,
    // Exponent for final IRI calculation
    // ExpRate[20] =
    {
      2, 2, 1.5, 1.5, .9, .9, .9, .9, 1, 1, 1.5, 1.5, 1, 0.7, 1, .8, 1, 1, 1, 1
    }
    ,
    // RAMP, is 1, Unless there's a Rate Adjustment Period
    // Var[20] =
    {
      1, 0.4, 1, 0.3, 1, 0.1, 1, 1, 0.4, 0.2, 1, .6, 1, 0.5, 1, 1, 1, 1, 1, 1
    }
    ,
    // Loan Desirability Index
    // Wanted[20] =
    {
      0.1, 0.2, 0.1, 0.2, 0.1, 0.2, 0.1, 0.4, 0.3, 0.3, 0.3, 0.3, 0.1, 0.1, .05, .3, 0.2, 0.2, 0.2, 0.2
    }
    ,
    // Each Loan's "normal proportion" within it's market.
    // Chip. .000-.999  From Page 22 of Loans
    //  NormProp[20] =
    {
      0.75, 0.25, 0.7, 0.3, 0.65, 0.35, 1, 1, 1, 1, 0.8, 0.2, 0.35, 0.15, .20, .3, 0, 0, 0, 0
    }
    ,
    // Each loan's "substitution" within it's market.
    // Chip. .000-.999  From Page 22 of Loans
    //  Subst[20] =
    {
      0.5, 0.5, 0.4, 0.4, 0.3, 0.3, 0, 0, 0, 0, 0.5, 0.5, 0.2, 0.2, .2, .2, 0, 0, 0, 0
    }
    ,
    // Chip. SmallBusinessTerm Or ConsumerPersonal?
    // SBTorCPL =
    .2,
    // Loan Quality Konst (High and Medium):
    //   [H,M]Prop,: Proportion
    //   [H,M]Exp,: Negative exponent
    // struct { float HProp,MProp,HExp,MExp; } Qual[20] =
    {
      {
        0.2, 0.4,  - .5,  - .7
      }
      ,
      {
        0.2, 0.4,  - .5,  - .7
      }
      ,
      {
        0.15, 0.4,  - .6,  - .7
      }
      ,
      {
        0.15, 0.4,  - .6,  - .7
      }
      ,
      {
        0.15, 0.3,  - .7,  - .8
      }
      ,
      {
        0.15, 0.3,  - .7,  - .8
      }
      ,
      {
        0.2, 0.3,  - .5,  - .7
      }
      ,
      {
        0.15, 0.3,  - .9,  - 1
      }
      ,
      {
        0.15, 0.4,  - .5,  - 1
      }
      ,
      {
        0.2, 0.4,  - .4,  - .8
      }
      ,
      {
        0.25, 0.4,  - .3,  - .6
      }
      ,
      {
        0.20, 0.4,  - .3,  - .6
      }
      ,
      {
        0.25, 0.3,  - .3,  - .7
      }
      ,
      {
        0.20, 0.3,  - .4,  - .8
      }
      ,
      {
        0.20, 0.3,  - .2,  - .9
      }
      ,
      {
        0.15, 0.4,  - .2,  - .9
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
    }
    ,
    // Business development On Market Share
    // MkShare1[20] =
    {
      0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.4, 0.5, 0.5, 0.7, 0.7, 0.7, .7, .5, 0.7, 0.5, 0.5, 0.5
    }
    ,
    // Line Officers salary On Market Share
    // MkShare2[20] =
    {
      0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.8, 0.8, 0.8, 0.8, 0.5, 0.5, 0.5, 0.5, .5, .5, 0.5, 0.5, 0.5, 0.5
    }
    ,
    // Loan processing salary On Market Share
    // MkShare3[20] =
    {
      0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.4, 0.3, 0.3, 0.3, 0.3, 0.4, 0.4, 0.3, .3, .3, 0.3, 0.3, 0.3, 0.3
    }
    ,
    // (4-Urban,Region,Burb,AdminOps) Premise On Mkt Share
    // MkShare4[20][4+1] =
    {
      {
        0, 1.5, 0, 1.0, 0
      }
      ,
      {
        0, 1.5, 0, 1.0, 0
      }
      ,
      {
        1.0, 1.5, 0.8, 1.0, 0
      }
      ,
      {
        1.0, 1.5, 0.8, 1.0, 0
      }
      ,
      {
        1.2, 1.2, 1.4, 0.8, 0
      }
      ,
      {
        1.2, 1.2, 1.4, 0.8, 0
      }
      ,
      {
        1.0, 1.1, 1.0, 0.8, 0
      }
      ,
      {
        1.0, 1.1, 1.0, 0.8, 0
      }
      ,
      {
        1.0, 1.0, 1.0, 1.0, 0
      }
      ,
      {
        1.0, 1.0, 1.0, 1.0, 0
      }
      ,
      {
        1.2, 1.0, 1.2, 0.5, 0
      }
      ,
      {
        1.2, 1.0, 1.2, 0.5, 0
      }
      ,
      {
        1.2, 1.0, 1.2, 0.5, 0
      }
      ,
      {
        1.2, 1.0, 1.2, 0.5, 0
      }
      ,
      {
        1.0, 1.0, 1.0, 1.0, 0
      }
      ,
      {
        1.2, 1.0, 1.2, 0.5, 0
      }
      ,

      {
        1.0, 1.5, 1.0, 1.0, 0
      }
      ,
      {
        1.0, 1.5, 1.0, 1.0, 0
      }
      ,
      {
        1.0, 1.5, 1.0, 1.0, 0
      }
      ,
      {
        1.0, 1.5, 1.0, 1.0, 0
      }
    }
    ,
    // Orinination Fee On Market Share
    // MkShare5[20] =
    {
      0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, .5, .5, 0.5, 0.5, 0.5, 0.5
    }
    ,
    // Prior Quarter's Actual-Market-Share
    // MkShare6[20] =
    {
      1, 1, 1, 1, 1.1, 1.1, 1.2, 1.2, 1.2, 1.2, 1, 1, 1, 1, 1, 1, 1, 1, 1
    }
    ,
    // Annual Fee On Market Share - For the 3 Cred Quals.
    // MkShare7[20][3] =
    {
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,

      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
      ,
      {
         - .5,  - .3,  - .2
      }
    }
    ,
    // Maturity Preference On Market Share - For the 3 Cred Quals.
    // MkShare8[20][3] =
    {
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.1, 0.15, 0.2
      }
      ,
      {
        0.1, 0.15, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
      ,
      {
        0.4, 0.3, 0.2
      }
    }
    ,
    // Bank's Gen Credit Policy On Market Share - For the 3 Cred Quals.
    // MkShare9[20][3] =
    {
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
      ,
      {
        0, 0.4, 1.0
      }
    }
    ,
    // Loan Policy On Market Share - For the 3 Cred Quals.
    // MkShare10[20][3] =
    {
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
      {
        1, 1, 1
      }
      ,
    }
    ,
    // Interest Rate On Market Share - For the 3 Cred Quals.
    // MkShare11[20][3] =
    {
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,
      {
         - 1.2,  - 1.1,  - 0.9
      }
      ,

    }
    ,
    // Rate Trap to Limit Rates to an Range.
    // RateLim[20][2] =
    {
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
      ,
      {
        1.1, 1.25
      }
    }
    ,
    // Credit Qualitie's Loan Loss Index Exponent; LLExp =
    1,
    // Credit Qualitie's Effective Loan Officer; Salary Exponent; ELOS[20] =
    {
      .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5
    }
    ,
    // Credit Qualitie's Effective Credit; Administraions Salary Exponent
    // ECAS[20] =
    {
      .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5
    }
    ,
    // Probbability that a "Non-Acrueing (8)" Loan; will   actually be "Charge-Off-able (9)"
    // PP[20] =
    {
      8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
    }
    ,
    //  MinKeepBal, MinCreateAmnt
    .02, .098,
    // MinNewTyp
    {
      .35, .35, .25, .25, .2, .2, .2, .2, .25, .25, .2, .2, .2, .2, .2, .2
    }
    ,
    // MBStartup: Thousands-$
    25,
    //      LnSaleMatAdj[MaxL], LnSaleCQAdj[MaxL], LnSaleMktAdj[MaxL];
    {
      .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1
    }
    ,
    {
      .45, .45, .45, .45, .45, .45, .45, .45, .45, .45, .45, .45, .45, .45, .45, .45
    }
    ,
    {
      .9, .9, .9, .9, .9, .9, .9, .9, .9, .9, .9, .9, .9, .9, .9, .9
    }
    ,
    //     Srv, SrvFix, SrvVar[MaxTypsSold];
    .000625, 1200,
    {
      .00025, .00015, .00015, .00035, .00035, .00035
    }
    ,
  };

  return LoanKonst; 
}

ResKonstT GetResKonst()
{
	#pragma region ResKonstT
  ResKonstT ResKonst =
  {
    // Resource
    // Adjustment when salaries are Higher than the CPI
    // UpAdj =    Weights: .01 <-> .99
    0.2,
    // Adjustment when salaries are Lower than the CPI
    // DwnAdj =    Weights:  .01 <-> .99
    0.4,
    // Konst for Training, General Administration,
    // and General Operations
    // Train,Admin,GenOps,
    .25, .2, .4,
    // Weights for Training, General Administration,
    // and General Operations
    // TrainW,AdminW,GenOpsW,
    0.4, 0.15, 0.25,
    // The Effectiveness of Promotions
    // MediaEff[30]  Loans 0-15, Deposits 20-28
    {
      0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.3, 0.3, 0.3, 0.3, 0.3, 0, 0, 0, 0, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3
    }
    ,
    // Increased Prompotions Konst
    // MediaKick[31],  Loans 0-15, Deposits 20-28, General 30
    {
      .1, .1, .1, .1, .15, .15, .1, .1, .1, .1, .2, .2, .2, .2, .3, .2, 0, 0, 0, 0, .2, .2, .3, .3, .3, .3, .3, .3, .3, .3, .3
    }
    ,
    // 1-Adj,2-Cur,& 3-Old Value factors
    // for each of the 4(+1) kinds of premises
    // ResPremisesT Premises[5],
    {
      {
        0.98, 0.8, 0.4
      }
      ,
      {
        0.98, 0.7, 0.4
      }
      ,
      {
        0.99, 0.7, 0.4
      }
      ,
      {
        0.99, 0.6, 0.5
      }
      ,
      {
        0, 0, 0
      }
    }
    ,
    // ReqFloor
    .85,
  };

  return ResKonst; 
#pragma endregion
}

CostKonstT GetCostKonst()
{
	#pragma region ConstKonstT
  CostKonstT CostKonst =
  {
    {
      .025, .02, .023, .025, .028
    }
    ,  //  OccExp[5];
    {
      8.5, 4.2, 5.5, 7.8, 1
    }
    ,  //  PremRV[5],
    //  BenFac, Used to Derive Benefits from salary Decisions
    .28,
    //   float OthAss[2], OthLia[2];
    {
      .3, .04
    }
    ,
    {
      .25, .045
    }
    , .4, .7,  // LnsPrm, DepPrm
    // ReqRsv[11] =
    {
      .16, .25, .12, 0, 0, .13, .02, .04, 0, 0, .01
    }
    ,
    {
      // FixedCosts
      {
        50, 10, 40, 0, 0
      }
      ,  // Admin[5],
      //  Ops[10],
      {
        12, 8, 8, 20, 0, 0, 0, 0, 0, 0
      }
      ,  // Note: Ops[4] is In Admin[2]!
      {
        15, 12, 12, 13, 14, 12, 10, 0, 0, 0
      }
      ,  //  Loans[10],
      //  Cred[10],
      {
        12, 12, 12, 12, 12, 9, 9, 0, 0, 0
      }
      ,
      //  Dep[5],
      {
        20, 8, 20, 14, 0
      }
    }
    ,
    {
      // Loan Officer Salary Costs    // SalCostT LnOfcr[MaxL][2]
      // New Ln Cnt    ||  EoQAmnt
      {
        {
          800, 260, .02
        }
        ,
        {
          300, 50, .04
        }
      }
      ,
      {
        {
          800, 260, .02
        }
        ,
        {
          275, 50, .04
        }
      }
      ,
      {
        {
          700, 250, .02
        }
        ,
        {
          250, 45, .04
        }
      }
      ,
      {
        {
          700, 250, .02
        }
        ,
        {
          225, 45, .04
        }
      }
      ,
      {
        {
          600, 240, .02
        }
        ,
        {
          550, 45, .04
        }
      }
      ,
      {
        {
          600, 240, .02
        }
        ,
        {
          525, 45, .04
        }
      }
      ,
      {
        {
          700, 250, .02
        }
        ,
        {
          680, 45, .04
        }
      }
      ,
      {
        {
          800, 260, .02
        }
        ,
        {
          920, 50, .04
        }
      }
      ,
      {
        {
          900, 250, .02
        }
        ,
        {
          200, 45, .04
        }
      }
      ,
      {
        {
          900, 250, .02
        }
        ,
        {
          150, 45, .04
        }
      }
      ,
      {
        {
          510, 240, .08
        }
        ,
        {
          80, 20, .1
        }
      }
      ,
      {
        {
          515, 240, .04
        }
        ,
        {
          80, 25, .09
        }
      }
      ,
      {
        {
          300, 60, .04
        }
        ,
        {
          110, 20, .09
        }
      }
      ,
      {
        {
          200, 40, .01
        }
        ,
        {
          220, 25, .05
        }
      }
      ,
      {
        {
          80, 30, .05
        }
        ,
        {
          100, 22, .08
        }
      }
      ,
      {
        {
          160, 40, .01
        }
        ,
        {
          120, 50, .10
        }
      }
      ,
    }
    ,
    {
      // Loan Credit Salary Costs  // SalCostT Cred[MaxL][3]
      //  Qual 5&6 Amnt ||  Qual 7 Amnt   || PastDue/Non-Accruel Amnt
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
      {
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .25
        }
        ,
        {
          500, 400, .1
        }
      }
      ,
    }
    ,
    {
      //  Deposit Salary Costs  ( ReqVal )  SalCostT Dep[ Sz_RVs ][ 3 ]
      //  Three constants each for  { NewCnt, EoQ_Cnt, TransCnt }
      {
        {
          20, 4, .01
        }
        ,
        {
          4, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_DemBusM1
      {
        {
          12, 4, .01
        }
        ,
        {
          3, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_BusSaveM1
      {
        {
          12, 4, .01
        }
        ,
        {
          3, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_TimeBusM1
      {
        {
          20, 4, .01
        }
        ,
        {
          4, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_DemBusM5
      {
        {
          20, 4, .01
        }
        ,
        {
          3, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_BusSaveM5
      {
        {
          12, 4, .01
        }
        ,
        {
          3, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_TimeBusM5
      {
        {
          25, 4, .01
        }
        ,
        {
          4, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_DemPub
      {
        {
          10, 4, .01
        }
        ,
        {
          3, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_DemInd
      {
        {
          10, 4, .01
        }
        ,
        {
          3, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_IndNOW
      {
        {
          10, 4, .01
        }
        ,
        {
          3, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_IndMon
      {
        {
          10, 4, .01
        }
        ,
        {
          3, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_IndSave
      {
        {
          12, 4, .01
        }
        ,
        {
          3, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  //  RV_TimeInd
      //  For Ind15E/Ind16E classes and BusProE/BusGenE Markets
      //  when  PrivBkng [ Bank ]  salary decision is non-zero.
      {
        {
          20, 4, .002
        }
        ,
        {
          5, 1, .01
        }
        ,
        {
          .025, .005, .002
        }
      }
      ,  // RV_PrivBkng
    }
    ,
    // Operations Salary Costs: 3 Loan, 1 Dep  // SalCostT Ops[4]
    //  LnCommerAmnt ||  LnRealEstAmnt  ||  LnConsumerAmnt   ||  Dem/Now TransVOls
    {
      {
        150, 70, .05
      }
      ,
      {
        500, 160, .25
      }
      ,
      {
        1500, 1100, 1.0
      }
      ,
      {
        .06, .02, .001
      }
    }
    ,
    //  PMF, Admin[10];   Adm          Train      Central Ops
    .5,
    {
      .04, .06, .03, .01, .05, .25, 2, 3, 5, 6
    }
    ,
  };

  return CostKonst;
#pragma endregion
}

SecKT GetSecK()
{
	#pragma region SecKT
  SecKT SecK =
  {
    .2,  // Strip-Buy: yld(m) - [[.2]] /m
    {
      .002,  - .9
    }
    ,  // Strip
    {
      .35, .7
    }
    ,
    {
      .25, .25, .55
    }
    ,
    {
      .25, .25, .55
    }
    ,  // Bond
    {
      1, .005, .001
    }
    ,  // Muni
    {
      .5, .005, .0025
    }
    ,  // Swaps
    //  MatClass[MaxMC],
    {
      0, 1, 2, 3, 4, 6, 8, 12, 20, 32
    }
    ,
    // TEFRA
    .2,
    //  struct { float PrePay, a[12]; } CMO;
    {
      .01,
      {
        UnSet, .6, .3, .4, .3, .5, .0025, 1, 0, .06, .001, .5
      }
      ,
    }
    ,
    //   float BidAdj[MaxSecTypes][2];
    {
      {
        1, 0
      }
      ,
      {
        1.01, .001
      }
      ,
      {
        1, 0
      }
      ,
      {
        1.01, .001
      }
      ,
      {
        1.01, .001
      }
      ,
      {
        1.01, .001
      }
      ,
      {
        1.01, .001
      }
      ,
      {
        1.01, .001
      }
      ,
      {
        1.01, .001
      }
      ,
      {
        UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet
      }
      ,
      {
        UnSet, UnSet
      }
      ,
    }
    ,
  };
  return SecK;
#pragma endregion
}

DepKT GetDepK()
{
	#pragma region KepKT
  DepKT DepK =
  {
    {
      // BusNatClK[MaxBusCls]
      {
        .10, 3000, 10, 50000, 0, .5, 1, 0, 0, 0, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .20, 8000, 20, 80000, 0, .5, 1, 0, 0, 0, .4, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .30, 10000, 20, 150000, 0, .5, 1, 0, 0, 0, .2, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .40, 15000, 20, 250000, 0, .5, 1, 0, 0, 0, .2, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
    }
    ,
    {
      // BusMidClK[MaxBusCls]
      {
        .10, 1000, 8, 20000, 0, .5, 1, 0, 0, 0, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .20, 2000, 20, 60000, 0, .5, 1, 0, 0, 0, .4, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .30, 5000, 20, 200000, 0, .5, 1, 0, 0, 0, .2, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .40, 10000, 20, 300000, 0, .5, 1, 0, 0, 0, .2, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
    }
    ,
    {
      // BusLocalClK[MaxBusCls]
      {
        .10, 50, 4, 10000, 2, .5, .8, 0, .1, 0, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .20, 100, 8, 20000, 2, .5, .7, .2, .1, .2, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .30, 150, 12, 50000, 3, .5, .7, .2, .1, .2, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .40, 200, 20, 150000, 3, .5, .6, .2, .1, .1, .4, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
    }
    ,
    {
      // BusREClK[MaxBusCls]
      {
        .10, 50, 4, 10000, 2, .5, .8, 0, .1, 0, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .20, 100, 8, 20000, 2, .5, .7, .2, .1, .2, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .30, 100, 8, 50000, 2, .5, .7, .2, .1, .2, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .40, 200, 20, 200000, 1, .5, .6, .2, .1, .1, .4, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
    }
    ,
    {
      // BusProfClK[MaxBusCls]
      {
        .10, 50, 4, 10000, 0, .5, 1, 0, 0, 0, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .20, 75, 4, 50000, 2, .5, .7, .2, .1, .1, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .30, 100, 8, 75000, 2, .5, .7, .2, .1, .1, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .40, 100, 20, 120000, 2, .5, .7, .1, .1, .5, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
    }
    ,
    {
      // BusGenClK[MaxBusCls]
      {
        .10, 5, 2, 1000, 0, .5, 1, 0, 0, 0, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .20, 20, 2, 3500, 0, .5, 1, 0, 0, 0, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .30, 50, 8, 10000, 1, .5, 1, 0, 0, 0, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
      {
        .40, 70, 8, 100000, 2, .5, .6, .2, .5, .5, .5, .4, .5,
        {
          0, 0, 0
        }
      }
      ,
    }
    ,
    //PubDepClKT PubDepClK[MaxPubCls] =
    {
      {
        .05, 20, 2, 2500, .1, .5,
        {
          0, 0
        }
      }
      ,
      {
        .10, 50, 5, 10000, .15, .4,
        {
          0, 0
        }
      }
      ,
      {
        .10, 100, 8, 12000, .15, .4,
        {
          0, 0
        }
      }
      ,
      {
        .15, 120, 8, 15000, .2, .4,
        {
          0, 0
        }
      }
      ,
      {
        .20, 200, 12, 25000, .2, .4,
        {
          0, 0
        }
      }
      ,
      {
        .20, 2500, 20, 100000, .25, .3,
        {
          0, 0
        }
      }
      ,
      {
        .20, 5000, 20, 200000, .25, .3,
        {
          0, 0
        }
      }
      ,
    }
    ,
    {
      // IndDepClKT IndDepClK [ MaxIndCls ] =
      //    Portion, ItemCntMo, NSFCntMo, AvgSize, MinBalCheck, MinBalMoneyMk, MatPref, FixPorPref, 
      //    CheckPorPref, SavingsPorPref, MoneyMkPorPref, CheckSubsti, SavingsSubsti, MoneyMkSubsti, 
      //    Loyalty,  Open
      {
        .02, 10, .5, 1500, 100, 0, 1, .5, .5, .5, 0, 0, 0, 0, .3, 0
      }
      ,
      {
        .05, 20, .1, 3000, 210, 400, 2, .5, .3, .3, .3, .1, .2, .5, .25, 0
      }
      ,
      {
        .06, 25, .06, 6000, 410, 900, 2, .5, .2, .2, .4, .2, .3, .3, .1, 0
      }
      ,
      {
        .05, 40, .05, 10000, 950, 1050, 1, .5, .18, .1, .6, .2, .4, .4, .1, 0
      }
      ,
      {
        .05, 40, .04, 25000, 1010, 1300, 2, .5, .15, .2, .55, .2, .4, .4, .1, 0
      }
      ,
      {
        .06, 40, .02, 25000, 1100, 1500, 3, .5, .15, .2, .55, .2, .4, .4, .1, 0
      }
      ,
      {
        .06, 40, .02, 25000, 1200, 1900, 4, .5, .15, .2, .55, .2, .4, .4, .1, 0
      }
      ,
      {
        .06, 40, .02, 25000, 1350, 2000, 5, .5, .15, .2, .55, .2, .4, .4, .1, 0
      }
      ,
      {
        .06, 40, 0, 30000, 1500, 2400, 6, .5, .15, .2, .55, .2, .4, .4, .2, 0
      }
      ,
      {
        .06, 40, 0, 30000, 1600, 2900, 10, .5, .15, .2, .55, .2, .4, .4, .2, 0
      }
      ,
      {
        .07, 40, 0, 30000, 1800, 4000, 12, .5, .15, .2, .55, .2, .4, .4, .2, 0
      }
      ,
      {
        .07, 40, 0, 50000, 2000, 6000, 20, .5, .15, .2, .5, .2, .4, .4, .2, 0
      }
      ,
      {
        .08, 20, 0, 80000, 2400, 9000, 30, .8, .07, .1, .45, 0, .5, .5, .1, 0
      }
      ,
      {
        .08, 20, 0, 200000, 2550, 20000, 40, .8, .08, .1, .45, 0, .5, .5, .1, 0
      }
      ,
      {
        .08, 45, 0, 200000, 5000, 25000, 4, .5, .06, .1, .6, .3, .4, .5, .25, 0
      }
      ,
      {
        .08, 45, 0, 200000, 4000, 30000, 8, .5, .06, .1, .6, .3, .4, .5, .25, 0
      }
      ,
    }
    ,
    //  MinKeepBal, MinCreateAmnt
    .02, .1,
    //    TimeWith[ TimeIndE - TimeBusE + 1 ],//    41
    {
      1, 1
    }
    ,
    //    NonTimeWith[ BusSaveE + 1 ];  //    7
    {
      //   DemIndE,   DemBusE,  DemPubE,
      .05, .04, .04,
      //  IndNOWE,  IndMonE, IndSaveE,  BusSaveE,
      .05, .06, .02, .04
    }
    ,
    // Comp[TimeIndMPE + 1]; Competitive Market Price Constants
    {
      //  DemIndMPE,     DemPubMPE,   IndNOWMPE,    IndMonMPE,   IndSaveMPE,
      6, 10, 8, 12, 10,
      //    BusNatDemMPE, BusMidDemMPE, BusLocDemMPE,
      //      BusReDemMPE, BusProDemMPE, BusGenDemMPE,
      10, 10, 10, 10, 10, 10,
      //    BusNatSaveMPE, BusMidSaveMPE, BusLocSaveMPE,
      //      BusReSaveMPE, BusProSaveMPE, BusGenSaveMPE,
      10, 10, 10, 10, 10, 10,
      //    BusNatTimeMPE, BusMidTimeMPE, BusLocTimeMPE,
      //      BusReTimeMPE, BusProTimeMPE, BusGenTimeMPE,
      10, 10, 10, 10, 10, 10,
      //  TimeIndMPE,
      15
    }
    ,
    //    BusAttrib[ BusGen4E - BusNat1E + 1 ][11],
    {
      {
        1, .1, .1, .1, .1, .1, .1, .2, .03
      }
      ,  // BusNat1E
      {
        1, .1, .1, .1, .1, .1, .1, .2, .03
      }
      ,  // BusNat2E
      {
        1, .1, .1, .1, .1, .1, .1, .2, .03
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .1, .2, .03
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .1, .3, .03
      }
      ,  // BusMid1E
      {
        1, .1, .1, .1, .1, .1, .1, .3, .03
      }
      ,  // BusMid2E
      {
        1, .1, .1, .1, .1, .1, .1, .3, .03
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .1, .3, .03
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .1, .4, .03
      }
      ,  // BusLoc1E
      {
        1, .1, .1, .1, .1, .1, .1, .4, .03
      }
      ,  // BusLoc2E
      {
        1, .1, .1, .1, .1, .1, .1, .4, .03
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .1, .4, .03
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .1, .3, .03
      }
      ,  // BusRe1E
      {
        1, .1, .1, .1, .1, .1, .1, .3, .03
      }
      ,  // BusRe2E
      {
        1, .1, .1, .1, .1, .1, .1, .3, .03
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .1, .3, .03
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .1, .1, .03
      }
      ,  // BusPro1E
      {
        1, .1, .1, .1, .1, .1, .1, .1, .03
      }
      ,  // BusPro2E
      {
        1, .1, .1, .1, .1, .1, .1, .1, .03
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .1, .1, .03
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .1, .1, .03
      }
      ,  // BusGen1E
      {
        1, .1, .1, .1, .1, .1, .1, .1, .03
      }
      ,  // BusGen2E
      {
        1, .1, .1, .1, .1, .1, .1, .1, .03
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .1, .1, .03
      }
      ,
    }
    ,
    //    PubAttrib[ Pub7E - Pub1E + 1 ][7],
    {
      {
        1, .1, .1, .1, .1, .1, .02
      }
      ,  // Pub1E
      {
        1, .1, .1, .1, .1, .1, .02
      }
      ,  // Pub2E
      {
        1, .1, .1, .1, .1, .1, .02
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .02
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .02
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .02
      }
      ,
      {
        1, .1, .1, .1, .1, .1, .02
      }
      ,
    }
    ,
    //    IndAttrib[ Ind16E - Ind1E + 1 ][13];
    {
      {
        1, .5, .1, .6, .1, .1, .01, .2, .01, .1, .04
      }
      ,  // Ind1E
      {
        1, .5, .1, .6, .1, .1, .01, .2, .01, .1, .04
      }
      ,  // Ind2E
      {
        1, .5, .1, .7, .1, .1, .01, .2, .01, .2, .04
      }
      ,
      {
        1, .5, .1, .7, .1, .1, .01, .2, .01, .2, .04
      }
      ,
      {
        1, .5, .1, .7, .1, .1, .01, .2, .01, .2, .04
      }
      ,
      {
        1, .5, .1, .7, .1, .1, .01, .2, .01, .2, .04
      }
      ,
      {
        1, .5, .1, .7, .1, .1, .01, .2, .01, .1, .04
      }
      ,
      {
        1, .5, .1, .7, .1, .1, .01, .2, .01, .1, .04
      }
      ,
      {
        1, .5, .1, .7, .1, .1, .01, .2, .01, .1, .04
      }
      ,
      {
        1, .5, .2, .8, .1, .1, .01, .2, .01, .1, .04
      }
      ,
      {
        1, .5, .2, .8, .1, .1, .01, .2, .01, .1, .04
      }
      ,
      {
        1, .5, .2, .8, .1, .1, .01, .2, .01, .1, .04
      }
      ,
      {
        1, .5, .2, .8, .2, .1, .01, .2, .01, .1, .04
      }
      ,
      {
        1, .5, .2, .8, .2, .1, .01, .2, .01, .1, .04
      }
      ,
      {
        1, .5, .2, .8, .2, .1, .01, .2, .3, .1, .04
      }
      ,
      {
        1, .5, .2, .8, .2, .1, .01, .2, .3, .1, .04
      }
      ,
    }
    ,
    // RateTrap[TimeIndMPE + 1];
    {
      //  DemIndMPE,     DemPubMPE,   IndNOWMPE,    IndMonMPE,   IndSaveMPE,
      1.5, 2, 2, 3, 2,
      //    BusNatDemMPE, BusMidDemMPE, BusLocDemMPE,
      //      BusReDemMPE, BusProDemMPE, BusGenDemMPE,
      2, 2, 2, 2, 2, 2,
      //    BusNatSaveMPE, BusMidSaveMPE, BusLocSaveMPE,
      //      BusReSaveMPE, BusProSaveMPE, BusGenSaveMPE,
      2, 2, 2, 2, 2, 2,
      //    BusNatTimeMPE, BusMidTimeMPE, BusLocTimeMPE,
      //      BusReTimeMPE, BusProTimeMPE, BusGenTimeMPE,
      2, 2, 2, 2, 2, 2,
      //  TimeIndMPE,
      2
    }
    ,
    // Response[TimeIndMPE + 1];
    {
      //  DemIndMPE,     DemPubMPE,   IndNOWMPE,    IndMonMPE,   IndSaveMPE,
      0, 0, 0, 0, 0,
      //    BusNatDemMPE, BusMidDemMPE, BusLocDemMPE,
      //      BusReDemMPE, BusProDemMPE, BusGenDemMPE,
      0, 0, 0, 0, 0, 0,
      //    BusNatSaveMPE, BusMidSaveMPE, BusLocSaveMPE,
      //      BusReSaveMPE, BusProSaveMPE, BusGenSaveMPE,
      0, 0, 0, 0, 0, 0,
      //    BusNatTimeMPE, BusMidTimeMPE, BusLocTimeMPE,
      //      BusReTimeMPE, BusProTimeMPE, BusGenTimeMPE,
      0, 0, 0, 0, 0, 0,
      //  TimeIndMPE,
      0
    }
    ,
  };
  return DepK; 
#pragma endregion
};

TreasKT GetTreasK()
{
	#pragma region TreasKT
  TreasKT TreasK =
  {
    // StkPrT
    {
      // WghtStkPr, WghtDiv, WghtGap
      .75, .4, .5,
      // Min, Max, Asst
      .25, 2.25, .7,
      // Earn. // Abs, Low, On, Hi
      {
        .15,  - .02, .05,  - .05
      }
      ,
      // Attr
      {
        // Targ, Pos, Neg, Max, Min
        {
          .061, .5, 1, .05,  - .05
        }
        ,  //  1
        {
          .09, .5, .8, .04,  - .04
        }
        ,  //  2
        {
          .004,  - 25,  - 10, .04,  - .04
        }
        ,  //  3
        {
          .008, 4, 3, .04,  - .04
        }
        ,  //  4
        {
          .015,  - 6,  - 5, .02,  - .04
        }
        ,  //  5
        {
          0, .4, .5, .08,  - .08
        }
        ,  //  6
        {
          0,  - .3, .5, 0,  - .10
        }
        ,  //  7
        {
          .005,  - 15.,  - 20., .1,  - .1
        }
        ,  //  8
        {
          .05, .5, .25, .01,  - .01
        }
        ,  //  9
        {
          8, .015, .02, .05,  - .05
        }
        ,  // 10
        {
          .002, 30, 30, .08,  - .08
        }
        ,  // 11
        {
          .03, 10, 15, .15,  - .15
        }
        ,  // 12
        {
          .1, .5, .8, .06,  - .08
        }
        ,  // 13
        {
          .04,  - 0.8,  - .5, .03,  - .10
        }
        ,  // 14
        {
          0, 1, 2, .06,  - .05
        }
        ,  // 15
        {
          .01, 1, 2, .08,  - .05
        }
        ,  // 16
        {
          .02, .5, .8, .08,  - .15
        }
        ,  // 17
        {
          .001, 15, 20, .15,  - .15
        }
        ,  // 18
        {
          0, .5, 1.5, .15,  - .15
        }
        ,  // 19
        {
          0, 0, 0, 0, 0
        }
        ,
      }
      ,
    }
    ,
    //  FnsAvl
    {
      // Rsk1[MaxRsk]
      {
        .12, .005, .06, .0035, .1, .01
      }
      ,
      // Rsk2[MaxRsk]
      {
        15, 50, 4, 500, 5, 20
      }
      ,
      // CredRating[MaxCR]
      {
        1.2, 2.2, 3.2, 4.2, 5.2, 6.2
      }
      ,
      // CDFract[2], CDAvlDst[MaxCDA]
      {
        .2, .03
      }
      ,
      {
        .29, .2, .1, .15, .05, .1, .04, .07
      }
      ,
    }
    ,
    // SubLTD_IssuedRate[13]
    {
       - .01,  - .01, 0, .1, 0, 0, 0,  - .02,  - .01, .15, 1, .05, .35
    }
    ,
  };
  return TreasK;
#pragma endregion 
}

void GetEconList(EconT econList[], int size)
{
	#pragma region EconT10
	EconT EconList[10] = 
	{ 
		{  
      //  Economy 1, 2004

			- 2.5,  - 1.9, .5,  - 2.4,  - 3.0,  - 2.75,  /*- 3.0*/ -2.00,  - 2.45, 0.0, .6,

			.75, .95, 1.14, 1.36, 1.6,  - .5, 0, 0, 0, 0,

			{
			0.0, .5, .1, .6, .15, .7, .12, 2.5, 2.5, 2.25,

			1.0,  - .5, 0.0,  - .5, 4.5, 2.5, 0, 0, 0, 0
			}
			,

			- 3,  - 2.9,  - 2.5,  - 2.8,  - 2.4, 0, 0, 0, 0, 0,

			{
			0.2, 1.5, 0, 0, 0
			}
			,

			- 2.0,  - 1.0,  - 1.5, 8.0, 9.5, 0.0, 1.0,  - 5.0, 0, 0,

			1.1, 1.0, 1.5, 1.0, 1.0, 0.6, 0, 0, 0, 0,

			{
				- 0.04,  - 0.02,  - 0.05,  - 0.03, 0.01,  - 0.01, 0.02,  - 0.01,  - 0.02, 0
			}
			,

			{
			.01, .01, 0.01, 0.01,  - 0.01, 0.01, 0.05,  - 0.01, 0, 0
			}
			,

			{
			0, 0, 0, 0, 0
			}
			,
			{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			}
			,
			{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			}

		}
		,
		{

		  // Economy 2, 2004

		   - 1.5,  - .6, 1.1,  - 1.0,  - 1.4,  - 1.5,  /*- 1.4*/ -1.35,  - 1.3, .75,

		  1.5, 1.68, 1.9, 2.2, 2.6, 3.0, .5, 0, 0, 0, 0,

		  {
			0.9, 1.3, 1.15, 1.65, 1.25, 2.0, 1.2, 2.5, 2., 1.9,

			2.2, 0, 3.0, 1.5, 6, 3.5, 0, 0, 0, 0
		  }
		  ,

		   - 2.5,  - 2,  - 2,  - 1.0,  - 1.2, 0, 0, 0, 0, 0,

		  {
			0.2, 1.5, 0, 0, 0
		  }
		  ,

		   - 1.2,  - 1.5,  - 2, 7.2, 7.4, 1.5, 0.0, 0.0, 0, 0,  // Fixed, 0.2 deleted, 0 added back .

		  .9, 1.1, 1.7, 0.95, 0.9, 0.8, 0, 0, 0, 0,

		  {
			 - .03,  - .03,  - .03,  - .02, 0.01,  - .06,  - .05, .03,  - .02, 0
		  }
		  ,

		  {
			.01, .01, .01, .01, .01, .01, .02,  - .02, 0, 0
		  }
		  ,

		  {
			0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }

		}
		,
		{


		  // Economy 3, 2004

		  1, 1.3, 1.6, 1.85, 1.3, 1, /*.85*/ 1.10, 1.25, 3,

		  3.1, 3.30, 3.50, 3.80, 4.25, 4.80, 1.5, 0, 0, 0, 0,

		  {
			3.25, 3.75, 3.5, 4.0, 3.75, 4.25, 3.7, 4.5, 4.0, 3.9, 3.3, 2.60, 3.8,

			4.1, 7.5, 4.1, 0, 0, 0, 0
		  }
		  ,

		   - 1.5,  - 1,  - 1, 0.25, 0.60, 0, 0, 0, 0, 0,

		  {
			0.2, 1.5, 0, 0, 0
		  }
		  ,

		  1.2, 3.0, 2.5, 5.6, 5.4, 1.8, 3.0, 4.0, 0, 0,

		  1.05, 1.0, 0.9, 1.0, 1.0, 1, 0, 0, 0, 0,

		  {
			.10, .10, .12, .10, .08, .07, .07, .05, .12, 0
		  }
		  ,

		  {
			.05, .05, .05, .05, .05, .07, .10, .04, 0, 0
		  }
		  ,

		  {
			0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }

		}
		,
		{

		  // Economy 4, 2004

		  1.25, 1.6, 2.0, 1.4, 1.2, 1.25, /*1.0*/ 1.70, 1.5, 3.5,

		  3.2, 3.40, 3.60, 3.90, 4.30, 4.80, 2.0, 0, 0, 0, 0,

		  {
			3.75, 4.2, 3.8, 4.3, 4.0, 4.6, 4.0, 5.3, 4.2, 4.0, 3.2, 2.90, 4.1,

			4.4, 8.5, 4.2, 0, 0, 0, 0
		  }
		  ,

		   - 1.0,  - 0.8,  - 0.0, 2.65, 1.65, 0, 0, 0, 0, 0,

		  {
			0.2, 1.5, 0, 0, 0
		  }
		  ,

		  1.5, 2.5, 1.5, 5.4, 5.0, 2.0, 3.0, 4.0, 0, 0,

		  1.0, 1.0, .9, 1.0, 0.9, 1.1, 0, 0, 0, 0,

		  {
			.12, .12, .14, .12, .10, .09, .09, .09, .15, 0
		  }
		  ,

		  {
			.05, .05, .05, .05, .05, .07, .07, .05, 0, 0
		  }
		  ,

		  {
			0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }

		}
		,
		{

		  // Economy 5, 2004

		  3.4, 2.9, 2.4, 3.5, 3.6, 3.5, /*3.2*/ 3.80, 3.5, 5.4,

		  3.0, 3.15, 3.35, 4.0, 4.60, 5.0, 2.0, 0, 0, 0, 0,

		  {
			5.65, 5.85, 5.75, 6.0, 5.5, 5.65, 5.80, 6.9, 5.20, 5.0, 4.2, 3.8, 5.5,

			6.2, 15.0, 5.4, 0, 0, 0, 0
		  }
		  ,

		  0.0, 1.0, 1.5, 2.75, 2.95, 0, 0, 0, 0, 0,

		  {
			0.2, 1.5, 0, 0, 0
		  }
		  ,

		  3.1, 4.5,  - .5, 4.0, 4.0, 0.2, 1.0, 5.0, 0, 0,

		  1.5, 1.0, 1.1, 1.0, 1.0, 1.5, 0, 0, 0, 0,

		  {
			.10, .10, .12, .10, .05, .06, .06, .03, .12, 0
		  }
		  ,

		  {
			.06, .06, .06, .06, .06, .07, .10, .06, 0, 0
		  }
		  ,

		  {
			0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }

		}
		,
		{

		  // Economy 6, 2004

		  3.25, 3.4, 3.8, 3.7, 3.45, 2.5, /*3.0*/ 3.50, 3.4, 5.0, 3.5,

		  3.62, 3.90, 4.25, 4.70, 5.0, 3.0, 0, 0, 0, 0,

		  {
			5.25, 5.8, 5.3, 5.9, 5.5, 6.05, 5.50, 6.80, 6.0, 5.9, 4.8, 3.5, 5.0,

			5.4, 10.5, 4.95, 0, 0, 0, 0
		  }
		  ,

		  0.0, 1.0, 1.5, 2.65, 3.0, 0, 0, 0, 0, 0,

		  {
			0.2, 1.5, 0, 0, 0
		  }
		  ,

		  2.5, 2.9,  - 1.0, 5.1, 4.0, 2.5, 3.2, 6.0, 0, 0,

		  1.0, 1.1, .9, 1.0, 1.1, 1.2, 0, 0, 0, 0,

		  {
			.10, .10, .12, .10, .08, .07, .07, .07, .12, 0
		  }
		  ,

		  {
			.05, .05, .05, .05, .05, .07, .07, .04, 0, 0
		  }
		  ,

		  {
			0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }

		}
		,
		{

		  // Economy 7, 2004

		   - 1.0,  - 0.5, 0.5,  - 0.5,  - 1.0,  - 1.0,  - 1.1,  - 0.8, 2.0, 1.0,

		  1.15, 1.3, 1.45, 1.7, 2.0,  - 0.5, 0, 0, 0, 0,

		  {
			2.15,2.45,2.4,2.75,2.5,3.0,2.50,3.5,2.4,2.3,1.4,0.0,1.8,

			2.5, 6.0, 3.0, 0, 0, 0, 0
		  }
		  ,

		  0.0,  - 2.5,  - 2.25,  - 2.3,  - 1.5, 0, 0, 0, 0, 0,

		  {
			0.2, 1.5, 0, 0, 0
		  }
		  ,

		   - 1.2, 3.0, 1.5, 7.0, 7.8, 2.0,  - 3.0,  - 4.0, 0, 0,

		  1.0, 1.0, 1.1, 1.0, 1.0, 1.0, 0, 0, 0, 0,

		  {
			.03,.03,.04,.04,.02,.02,.02,.04,.04,0
		  }
		  ,

		  {
			.008, .008, .008, .008, .01, .02, .01, .01, 0, 0
		  }
		  ,

		  {
			0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }

		}
		,
		{

		  // Economy 8, 2004

		   - 0.5,  - 0.1, 2.0,  - .2,  - 0.7,  - 1,  - 0.8,  - .25, 1.2, 2.2,

		  2.38, 2.6, 2.9, 3.3, 4.0, 1.2, 0, 0, 0, 0,

		  {
			1.4, 1.6, 1.5, 1.75, 1.6, 2.0, 1.4, 3.0, 3., 2.25,

			2.25, 0, 3.25, 1.5, 7, 4.4, 0, 0, 0, 0
		  }
		  ,

		   - 2,  - 1.7,  - 1.5,  - 1.3,  - 1.0, 0, 0, 0, 0, 0,

		  {
			0.2, 1.5, 0, 0, 0
		  }
		  ,

		   - .8, 1.0, 2.0, 6.5, 7.0,  - 0.5, 2.0, 2.5, 0, 0,

		  1, .9, 1.0, 1.2, 0.9, 0.9, 0, 0, 0, 0,

		  {
			 - .01,  - .01,  - .01, .01,  - .01,  - .0,  - .01,  - .01,  - .01, 0
		  }
		  ,

		  {
			.01, .01, 0, 0.01, 0, .02,  - .02, 0, 0, 0
		  }
		  ,

		  {
			0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }

		}
		,
		{

		  // Economy 9, 2004

		  0.5, 0.6, 1.35, 0.6, 0.7, 0.5, 0.4, 0.5, 2.8,

		  1.8, 2.0, 2.2, 2.5, 2.9, 3.4, 0.5, 0, 0, 0, 0,

		  {
			3.10, 3.35, 3.25, 3.5, 3.60, 3.95, 3.50, 4.0, 3.7, 3.4, 3.3, 2.50, 3.2,

			4.0, 8.0, 3.8, 0, 0, 0, 0
		  }
		  ,

		   - 1.5,  - 1.2,  - 1.,  - 0.45, 0.55, 0, 0, 0, 0, 0,

		  {
			0.2, 1.5, 0, 0, 0
		  }
		  ,

		   - 0.7, 4.0, 1.5, 6.4, 5.9, 1.8, 2.5, 4, 0, 0,

		  1.0, 1.0, 1.05, 1.0, 1.0, 1, 0, 0, 0, 0,

		  {
			.05,.05,.06,.05,.04,.04,.04,.04,.06,0
		  }
		  ,

		  {
			0, 0, 0, 0, 0, 0,  - .04, 0, 0, 0
		  }
		  ,

		  {
			0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }

		}
		,
		{

		  // Economy 10 , 2004
		  // This Economy has documentation and is used in initialization.

		  // --- -=FROM=- Base Rate ---

		  //   One_Qtr_Treas, Three_Qtr_Treas, 1, 3 Qtr, 30Yr Thirty_Year_Treas, Treasury yield

		  //  CPR, // commercial paper rate

		  //  FFR, // federal funds rate

		  //  FRDR, // Federal Reserve discount rate

		  //  RPR, // repurchase agreement rate

		  //  LIBOR, // London interbank offer rate

		  //  Prime, // national prime rate

		  //  BondAAA,  . . . // corporate bond rate

		  // SecMk[20] =

		  1.0, 1.4, 2.8, 1.3, 0.9, 0.9, 0.8, 1.5, 3.0, 3.2,

		  3.40, 3.60, 3.90, 4.30, 4.80, 2.0, 0, 0, 0, 0,

		  // Loan comparison rates

		  // LoanCompRates[20] =

		  {
			3.0, 3.25, 3.25, 3.5, 3.50, 3.90, 3.40, 4.50, 4.5, 4.0, 3.0, 2.50, 3.0,

			4.0, 6.0, 3.5, 0, 0, 0, 0
		  }
		  ,

		  // Deposit comparision rates

		  //  IDD, // individual demand deposits

		  //  NOW, // NOW accounts

		  //  SAV, // savings accounts

		  //  MM, // money market accounts

		  //  Dep1QTD, // rate on one quarter time deposits

		  // DepCompRates[10] =

		   - 1.5,  - 1.5,  - 1, 0.45, 0.65, 0, 0, 0, 0, 0,

		  // Other comparison rates

		  // OthrCompRates=

		  {
			0.2, 1.5, 0, 0, 0
		  }
		  ,

		  // Economic indicators

		  // Indicators[10] =

		  0.0, 2.5, 1.5, 6.0, 6.0, 2.0, 3.0, 4.0, 0, 0,

		  // Internal Market Indicies

		  // InterMkIndx[10] =
		  //  SMI, // stock market index
		  //  CDMI, // CD market index
		  //  LLI, // loan loss index -- Use to determin PastDues (LNQUAL)
		  //  ECI_Nat, // economic conditions index -- PrePay && PrinPay??
		  //  ECI_Local,
		  //  LMQI,
		  //  InterMkIndxOpen4, InterMkIndxOpen3, InterMkIndxOpen2, InterMkIndxOpen1,

		  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0, 0, 0, 0,

		  // Product market growth rates--High for Development

		  // MkGrRates[25] =

		  //    LoanMarkets[10],

		  {
			 .08,  .08,  .09,  .09,  .08,  .09,  .09,  .09,  .12,  0
		  }
		  ,

		  //    DepositMarkets[10],

		  {
			.01, .015, .025, .020, .025, .03, .02, 0, 0, 0
		  }
		  ,

		  //    OtherMarkets[5],CompPrices[10],Open[18],

		  {
			0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }
		  ,
		  {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		  }
		  ,
		}
	};
	#pragma endregion

	for(int i = 0; i < size; i++)
	{
		econList[i] = EconList[i]; 
	}

}

EnvT GetEnv()
{
	  // Begin Static Economic Data p. 7
#pragma region EnvT
  EnvT Env =
  {

    // BaseData[30]=

    //    BaseRate, // set level of interest rates

    //    InflRateBase, // set level of inflation rate

    //    TaxRate,

    3.0, 2.0, 34,

    //    Weight[2],

    // This gives 3 weights ( Weight[3] = 1 - Weight[1&2])

    // For The 2 Transitional Economies & ECN1 (Current).

    {
      .3, .4
    }
    ,

    // SeasonalWt[5][4],

    {

      {
        0.97, 1.01, 1.00, 1.02
      }
      ,  // #1--Business seasonal

      {
        0.94, 1.05, 1.02, 0.99
      }
      ,  // #2--Real estate seasonal

      {
        0.96, 1.01, 0.99, 1.04
      }
      ,  // #3--Consumer seasonal

      {
        0.94, 1.06, 1.02, .98
      }
      ,  // #4--Public deposit seasonal

      {
        0.98, 1.02, .99, 1.01
      } // #5--Interest rate seasonal

    }
    ,

    // Open[5],

    {
      0, 0, 0, 0, 0
    }
    ,

    //  MkSize[25] = -- Community Market size

    //     1 National corporations

    //     2  Middle market corporations

    //     3  Small businesses

    //     4  Importers/Exporters (trade finance)

    //     5  Construction

    //     6  Commercial real estate

    //     7  Multi-family residential

    //     8 1-4 Family residential

    //     9  Consumers

    //     10  Open

    //    11 National accounts

    //    12 Middle market accounts

    //    13  Local business accounts

    //    14  Real estate related

    //    15 Professional

    //    16  Other (not loan related)

    //    17  Individual

    //    18  Public

    {
      900, 1200, 900, 250, 400, 500, 400, 2500, 1800, 0,

      200, 330, 380, 400, 300, 180, 4400, 220, 0, 0,

      100, 50, 0, 0, 0
    }
    ,

    // Volume[25] = -- Volume adjustment factor

    {
      .05, .06, .08, .15, .08, .02, .015, .018, .08, 0,

      .02, .02, .02, .02, .02, .02, .03, 0.02, 0,

      0, .055, 0.01, 0, 0, 0
    }
    ,

    // Seasonal Selector

    // SeasonalSel[25] = --  Seasonal Pattern Identification

    {
      1, 1, 1, 0, 2, 2, 2, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0
    }
    ,

    // Open1[23]=

    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,

    //  Seq[3][40] =
	{
		// Update the economy data for 2008, 
		// Jeff 6/30/2008. The second set of changes will be to change the schedules for 
		// Economies 1, 2, 3. 
		//Economy sequence #1
		//{ 10, 10, 10, 10, 10, 10, 10, 7, 7, 7, 9, 9, 9, 3, 4, 2, 8, 3, 4, 5,
			//  5, 3, 3, 3, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10}
		{
			10, 10, 10, 10, 10, 10, 10, 7, 7, 7, 
			9, 9, 9, 2, 1, 8, 3, 3, 2, 3,
			2, 1, 3, 4, 3, 10, 10, 10, 10, 10,
			10, 10, 10, 10, 10, 10, 10, 10, 10, 10
		}
		,

		//Economy sequence #2
		//{ 10, 10, 10, 10, 10, 10, 10, 7, 7, 7, 9, 9, 9, 2, 1, 8, 3, 5, 5, 6,
		//  6, 3, 5, 3, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10}
		{
			10, 10, 10, 10, 10, 10, 10, 7, 7, 7,
			9, 9, 9, 2, 1, 8, 3, 3, 3, 4,
			2, 2, 8, 4, 5, 10, 10, 10, 10, 10, 
			10, 10, 10, 10, 10, 10, 10, 10, 10, 10
		}
		,

		//Economy sequence #3
		//{ 10, 10, 10, 10, 10, 10, 10, 7, 7, 7, 9, 9, 9, 4, 4, 3, 5, 5, 6, 6,
		//  3, 3, 4, 3, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10}
		{
			10, 10, 10, 10, 10, 10, 10, 7, 7, 7, 
			9, 9, 9, 2, 1, 8, 3, 3, 3, 1, 
			2, 3, 4, 6, 4, 10, 10, 10, 10, 10,
			10, 10, 10, 10, 10, 10, 10, 10, 10, 10
		}
		,
	}
    ,

    // Open2[8]=
    {
      0, 0, 0, 0, 0, 0, 0, 0
    }
	,
  };
  return Env;
#pragma endregion
}

PricesT GetPrices()
{
	    // -- End of New_Econ.TXT --
#pragma region PricesT
  PricesT Prices =
  {
    // { float Fee, QRAP, CreditQ; } LnFixPrices[20]=
    {
      {
        1.2, 1, 3
      }
      ,
      {
        0, 1, 3
      }
      ,
      {
        1.2, 1, 3
      }
      ,
      {
        0, 1, 3
      }
      ,
      {
        1.2, 1, 3
      }
      ,
      {
        0, 1, 3
      }
      ,
      {
        1.2, 1, 3
      }
      ,
      {
        1.2, 1, 3
      }
      ,
      {
        0, 4, 3
      }
      ,
      {
        0, 4, 3
      }
      ,
      {
        0, 120, 3
      }
      ,
      {
        0, 4, 3
      }
      ,
      {
        30, 1, 3
      }
      ,
      {
        0, 1, 3
      }
      ,
      {
        10, 1, 3
      }
      ,
      {
        0, 12, 3
      }
      ,
      {
        0, 0, 0
      }
      ,
      {
        0, 0, 0
      }
      ,
      {
        0, 0, 0
      }
      ,
      {
        0, 0, 0
      }
      ,
    }
    ,
    {
      // DepTypeInfo [ 10 ]
      //  MnFee, ItemFee, OthFee, MinBal,  Attrib
      //                  NSF
      {
        1, .10, 10, 0, 2.5
      }
      ,  // DemIndE
      //                Deposit fee
      {
        10, .10, 2.0, 0, 2.5
      }
      ,  // DemBusE
      {
        10, .10, 2.5, 0, 1
      }
      ,  // DemPubE
      //                 NSF
      {
        1, .10, 10, 1000, 2.2
      }
      ,  // IndNOWE
      {
        .5, 0, 0, 2000, 2.2
      }
      ,  // IndMonE
      {
        .5, 0, 0, 0, 2.2
      }
      ,  // IndSaveE
      {
        1, 0, 0, 0, 1.5
      }
      ,  // BusSaveE
      {
        0, 0, 0, 0, 1.5
      }
      ,  // TimeBusE
      {
        0, 0, 0, 0, 2
      }
      ,  // TimeIndE
      {
        0, 0, 0, 0, 0
      } // TimeHotE
    }
    ,
    //  float MaxHotMat, aBanksHotAvailByMat[10];
    4,
    {
      30, 50, 60, 80, 0, 0, 0, 0, 0, 0
    }
    ,
    // { float Fee, Mat, Other; } OthFixPrices[5]=
    {
      {
        0, 0, 0
      }
      ,
      {
        0, 0, 0
      }
      ,
      {
        0, 0, 0
      }
      ,
      {
        0, 0, 0
      }
      ,
      {
        0, 0, 0
      }
    }
    ,
    // { float Max, Min; } Legal[ 35 ]
    // Legal Minimum, Maximum rates
    // 20 Loan Rates, 10 Deposit rates, 5  " Other ".
    {
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
      {
        0, 99
      }
      ,
    }
    ,
    //  Security Pricing Constants
    // float SecPri[30] =
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    //#define MaxTE_Avl 9
    //  TE_AvlAmnt;   MatRateT TE_Avl[MaxTE_Avl];
    5,
    {
      {
        8, .94
      }
      ,
      {
        12, .94
      }
      ,
      {
        16, .95
      }
      ,
      {
        20, .95
      }
      ,
      {
        30, .96
      }
      ,
      {
        40, .96
      }
      ,
      {
        60, .97
      }
      ,
      {
        80, .98
      }
      ,
      {
        100, .98
      }
      ,
    }
    ,
    {
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
      ,  //SPA[MaxB][MaxStPrAtr + 1]
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    }
  }; // Economy Price Constants
  return Prices;
#pragma endregion 
}

LowerEconT GetLowerEcon()
{
	#pragma region LowerEconT
  LowerEconT LowerEcon =
  {
    // South 128
    //   TaxRate,
    0,
    //   TreasYld[8],
    {
      0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    //   PrGrowth[25],
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    // 1Qtr-&-4Qtrs BY 4 Markets
    {
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
    }
    ,
    //   PLNQUAL,
    0,
    // SalIndex, SalIndexChg,
    1.0, 2,
    //#define MaxTE_MatAvail 3
    //  TE_MatAvail[MaxTE_MatAvail];
    {
      0, 0, 0
    }
    ,
  };
  return LowerEcon; 
#pragma endregion 
}

AcT GetAccounts()
{
#pragma region AcT
  AcT AccountsI =
  {
    {
      // Start of Bal
      {
        // Start of Assets
        //    float Total,Cash,Open_1, ffSold;
        0, 8, 0, 80,
        //    struct { float , TreasSec, LocalSec,
        //    { float Treasury, Local, Mort, Other; } SecuritiesT;
        {
          0, 0, 0, 0
        }
        ,
        //    { float Loans[16], Open_1, Open_2, Other, Sum; } LoansT;
        {
          {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
          }
          , 0, 0, 0, 0
        }
        ,
        //    LLR // 29
        4.0,
        //    struct { float GrossPremises, AccumDepre; } NetPremises; //30
        {
          0, 0
        }
        ,
        //    float OtherRE, Other, Open_2, Balancer; //32
        0, 0, 0, 0
      }
      ,  // End of Assets;
      {
        // Start of Liabilities
        {
          // Start of Deposits
          //      struct { float Bus, Pub, Indiv, Open, Open2; } Demand; //36
          {
            0, 0, 0, 0, 0
          }
          ,
          //  float Now, Sav, MonMkt, Jumbo, PubTime, SavCerts, Open;
          {
            0, 0, 0, 0, 0, 0, 0
          }
          ,
          //      float Open_1, Open_2, Sum;
          0, 0, 0
        }
        ,  // End of Deposits
        //    { float FRB, Repo, FF, ShortTermHF; } FundsT;
        {
          0, 0, 0, 0
        }
        ,
        //    float PayableDividends, Other, Open, Balancer, LTD_HF, SubLTD,
        0, 0, 0, 0, 0, 0,
        //    struct { float Common, Surplus; } Stock;
        {
          10, 20.5
        }
        ,
        //    RetainedEarnings, _Equity;
        8.5, 39.0
      } // End of Liabilities;
    }
    ,  // End of Bal
    {
      // Start of BalAve
      {
        0, 0, 0, 0,
        {
          0, 0, 0, 0
        }
        ,
        {
          {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
          }
          , 0, 0, 0, 0
        }
        , 0,
        {
          0, 0
        }
        , 0, 0, 0, 0
      }
      ,
      {
        {
          {
            0, 0, 0, 0, 0
          }
          ,
          {
            0, 0, 0, 0, 0, 0, 0
          }
          , 0, 0, 0
        }
        ,
        {
          0, 0, 0, 0
        }
        , 0, 0, 0, 0, 0, 0,
        {
          0, 0
        }
        , 0, 0
      }
    }
    ,
    //{ // Start of IncomeOutGo
    {
      //  { // Start of Interest
      {
        //    { // Start of Income
        {
          //      float FF; //129
          0,
          //      { float Treasury, Local, Mort, Other; } Securities //130
          {
            0, 0, 0, 0
          }
          ,
          //      { float Loans[16], Open_1, Open_2, Other, Sum; } Loans //134
          {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
          }
          ,
          //      float Other; //154
          0
          //    } Income;
        }
        ,
        //    { // Start of Expenses
        {
          //  float Now, Sav, MonMkt, Jumbo, PubTime, SavCerts, Open;
          {
            0, 0, 0, 0, 0, 0, 0
          }
          ,
          //      { float FRB, Repo, FF, ShortTermHF; } FundsT; //161
          {
            0, 0, 0, 0
          }
          ,
          //      LTD_HF, SubLTD, //165
          0, 0,
          //    } Expenses;
        }
        ,
        //    float NetSwaps, CoreDep, Open_1, Net; //167
        0, 0, 0, 0
        //  } Interest;
      }
      ,
      //  float LoanLossProvision; //172
      0,
      //  { float DepServiceChrgs, LoanFees, //173
      //      CommLCFees, StandbyLCFees, Other;
      //  } Income;
      {
        0, 0, 0, 0, 0
      }
      ,
      //  float Open_1
      0,
      //  struct { float SalariesBenes,Rent,Media,Other; } Expenses
      {
        0, 0, 0, 0
      }
      ,
      //  { float UnRealizedPortf,AssetSales,
      //    IncomeTaxes,Income,IncomePerShare; } Nets
      {
        0, 0, 0, 0, 0
      }
      ,
      // OpEarnings, TEFRA
      0, 0
      //} // End of IncomeOutGo
    }
    ,
    //typedef struct { float Bus, Pub, Ind; } DemIntT;
    {
      0, 0, 0,
    }
    ,
    //  float SwapRisk, Open8[7],
    0, 0, 0, 0, 0, 0, 0, 0,
    //   Munis, FundsCost,
    0, 0,
    //    EPS_Forcast, NetChargeOffs, PastDue,
    0, 0, 0,
    //    CreditRating, Unpledged,
    0, 0,
    //    UnRealizedSecurities,
    0,
    //    NonTreasSec1yr, ProfitsMeas, NetEPS, LoanRiskMeas,
    0, 0, 0, 0,
    //    RiskAdjAss, RateSens, OneYrGap, LoanMkShare, DepMkShare,
    0, 0, 0, 0, 0,
    //    ShortTermRisk, LongTermRisk,
    0, 0,
    //    Open4, Open5, CoreDep, NOEPS, EarnRate, SmthDiv, NonPerformLns,
    0, 0, 400, 1.0, 0, 0, 0.6,
    //    TaxedMuniIntTaxEqiv, TaxedMuniIntTaxEqivYTD,
    //    No_Sec_Int_YTD, No_Sec_Int, LnChrgOffsYTD, MortBanking,Liquid_Bonds,
    0, 0, 0, 0, 0, 0, 0,
    //    Open3[11],
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    // Div, DivYTD,
    0, 0,
    //    _Book, Profit, Risk,
    //    Leverage, Liquidity, MarketShare, EPS_ForcastAccuracy,
    //    DividendPol, Other, StkPrice
    0, 0, 0, 0, 0, 0, 0, 0, 0, 40
  };
  return AccountsI;
#pragma endregion
}

void GetSec(SecT sec[])
{
	  #pragma region SecT
  SecT Sec[] =
  {
    // Not Sorted by Type
    {
      BillsE, 121,
      {
        5, 4.87, 0
      }
      , 0, 1, 6, 0, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    {
      BillsE, 122,
      {
        5, 6, 0
      }
      , 0, 0, 6, 0, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    {
      BillsE, 123,
      {
        5, 5.7, 5.9
      }
      , 0, 2, 6, 0, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    {
      StripsE, 101,
      {
        5, 5, 5
      }
      , 0, 0, 6, 0, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    {
      StripsE, 102,
      {
        1, 0.8, 0.85
      }
      , .05, 2, 6, 3.2, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    {
      StripsE, 119,
      {
        1, .7, 0.75
      }
      , .05, 4, 6, 3.2, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    {
      BondsE, 103,
      {
        15, 15, 15
      }
      , 0, 20, 6.0, 4, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    {
      BondsE, 104,
      {
        8, 8, 8
      }
      , .2, 32, 5.75, 3.2, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    {
      BondsE, 105,
      {
        10, 10, 10
      }
      , 0, 16, 7.25, 0, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    {
      MunisE, 106,
      {
        5, 5, 5
      }
      , 0, 12, 5.5, 0, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    {
      MunisE, 107,
      {
        5, 5, 5
      }
      , 0, 62, 4.25, 0, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    {
      MunisE, 108,
      {
        5, 5, 5
      }
      , 0, 84, 4.8, 0, 0, 0, 0, 1, 0, 0, 0
    }
    ,
    //  { No_Sec_E,
    //    109, {10,   10,      0 },   0,   11,  7.0,    0, 0, 0, 0, 1, 0, 0, 0 },
  };

  for(int i = 0; i < 12; i++)
  {
	  sec[i] = Sec[i];
  }
#pragma endregion 
}

LoanGloT GetLoanGlo()
{
	  #pragma region LoanGloT
  LoanGloT LoanGlo =
  {
    // LnCnt[MaxLnTypes],
    {
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0
    }
    ,
    //  NAT[MaxLnTypes], CORatio[MaxLnTypes],
    {
      0, 0, 0, 0, .15, 0, .2, 0, .35, 0, 0, .150, .05, .25, .2, .3, 0, 0, 0, 0
    }
    ,
    {
      .001, .001, .002, .002, .004, .004, .002, .005, .011, .003, .002, .0025, .006, .004, .015, .012, 0, 0, 0, 0
    }
    ,
    //  UCB[MaxLnTypes],
    {
      0, 0, 0, 0, 0, 0, 0, 0, .05, 0, 0, 0, 0, 0, 0, 0.02, 0, 0, 0, 0
    }
    ,
    //  PlayersMS[MaxLnTypes], == 1 / Numbanks
    {
      .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5, .5
    }
    ,
    //  InternalMkShare[MaxLnTypes][MaxCQ], == 1 / 10
    {
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
      ,
      {
        .1, .1, .1
      }
    }
    ,
    //  MatClass[MaxMC],
    {
      1, 2, 3, 4, 6, 8, 12, 20, 32, 999
    }
    ,
    // float B_Com[CLTyps],E_Com[CLTyps];
    {
      0, 0, 0, 0, 0, 0, 0
    }
    ,
    {
      0, 0, 0, 0, 0, 0, 0
    }
    ,
  };
  return LoanGlo;
  #pragma endregion
}

void GetLoan(LoanT loans[])
{
#pragma region LoanT
	LoanT Loans[] =
  {  //Changelist: Jan 23, 2006 update Loans for Community 0 per Chip's request. 
    {
      1, 12, 2, 6, 0, 2, 1.8, 0, 1.5, 0, 0, 0, 2, 20, .5, 0, 1, 0
    }
    ,
    {
      1, 2.4, 5, 6, 0, 3, 3.2, 0, 2.5, 0, 0, 0, 0, 4, .6, 0, 2, 0
    }
    ,
    {
      2, 12, 12, 7, 1, 2, 2, 0, 2.5, 0, 1, 0, 0, 0, 0, 1, 3, 0
    }
    ,
    {
      2, 6.0, 15, 7.9, 1.5, 3, 2.6, 0, 3, 1.5, 4, 2, 0, 0, 0, .4, 4, 0
    }
    ,
    {
      3, 10, 4, 8.5, .5, 3, 3.7, 0, 1.2, 0, 0, 0, 1.5, 15, .8, 0, 5, 0
    }
    ,
    {
      3, 5.0, 6, 6.5, .5, 3, 2.9, 0, 1, 0, 0, 0, 1.5, 10, .5, 0, 6, 0
    }
    ,
    {
      //4, 10, 20, 9.4, 1.5, 3, 3, 0, 1.5, 0, 1, 0, 0, 0, 0, .5, 7, 0
			4, 10, 10, 9.4, 1.5, 3, 3, 0, 1.5, 0, 1, 0, 0, 0, 0, 1.0, 7, 0
    }
    ,
    {
      4, 12, 6, 8, 2, 4, 4, 0, 1.8, 4, 6, 6, 0, 0, 0, 2, 8, 0
    }
    ,
    {
      5, 10, 7, 8, 2, 4, 4, 0, .3, 0, 0, 0, 2, 10, 1, 0, 9, 0
    }
    ,
    {
      5, 30, 2, 10, 2, 3, 3.2, 0, .5, 0, 0, 0, 2, 12, .8, 0, 10, 0
    }
    ,
    {
      //6, 10, 20, 9.8, 3, 2.4, 2, 1, .54, 8.2, 18, 18, 0, 0, 0, .5, 11, 0
			6, 10, 5, 9.8, 3, 2.4, 2, 1, .54, 8.2, 18, 18, 0, 0, 0, 2, 11, 0
    }
    ,
    {
      //6, 50, 5, 8.5, 0, 6, 5.8, 0, .4, 4.1, 5, 5, 0, 0, 0, 10, 12, 0
			6, 40, 5, 8.5, 0, 6, 5.8, 0, .4, 4.1, 5, 5, 0, 0, 0, 8, 12, 0    
    }
    ,
    {
      7, 8, 4, 8.5, 2.5, 5, 4.7, 0, .6, 0, 0, 0, 1.5, 15, .67, 0, 13, 0
    }
    ,
    {
      7, 5, 6, 9.5, 1.5, 3, 2.9, 0, .8, 0, 0, 0, 1.5, 5, 1, 0, 14, 0
    }
    ,
    {
      8, 1.2, 6, 9.5, 3.5, 6, 6.4, 0, 1.2, 0, 0, 0, 0, 3, .4, 0, 15, 0
    }
    ,
    {
      8, 12, 4, 9, 3, 2, 2.4, 0, 1.8, 0, 0, 0, 0, 12, .67, 0, 16, 0
    }
    ,
    {
    //9, 10, 36, 8, 4, 2, 3.5, 0, 5, 0, 1, 0, 0, 0, 0, .25, 17, 0
		  9, 24, 8,  8, 4, 5, 5,   0, 5, 0, 1, 0, 0, 0, 0,   3, 17, 0 
    }
    ,
    {
    //9, 12, 16, 11, 4, 3, 2, 0, 5, 0, 8, 8, 0, 0, 0, .8, 18, 0
			9, 12,  8, 11, 4, 3, 2, 0, 5, 0, 8, 8, 0, 0, 0,1.5, 18, 0 
    }
    ,
    {
    //9, 12, 3, 8, 3, 6, 5.5, 0, 5, 0, 1, 0, 0, 0, 0, 3, 19, 0
			9, 12, 6, 8, 3, 6, 5.5, 0, 5, 0, 1, 0, 0, 0, 0, 2, 19, 0
    }
    ,
    {
    //9, 6, 20, 8, 4, 2, 2, 0, 5, 0, 1, 0, 0, 0, 0, .3, 20, 0
			9, 6, 10, 8, 4, 2, 2, 0, 5, 0, 1, 0, 0, 0, 0, .6, 20, 0
    }
    ,
    {
    //9, 3, 22, 7.5, 3.5, 5, 6.6, .1, 2, 0, 1, 0, 0, 0, 0, .05, 21, 0
			9, 3, 10, 7.5, 3.5, 5, 6.6, .1, 2, 0, 1, 0, 0, 0, 0, .3,  21, 0
    }
    ,
    {
    //10, 5, 28, 8.4, 3.5, 1, 1, 0, 2.4, 0, 4, 4, 0, 0, 0, .18, 22, 0
			10, 5, 5,  8.4, 3.5, 1, 1, 0, 2.4, 0, 4, 4, 0, 0, 0,  1,  22, 0 
    }
    ,
    {
    //10, 8, 34, 10, 4, 3, 3, 0, 2.4, 0, 10, 10, 0, 0, 0, .11, 23, 0
			10, 8, 8,  10, 4, 3, 3, 0, 2.4, 0, 10, 10, 0, 0, 0,   1, 23, 0 
    }
    ,
    {
    //10, 4, 16, 7, 4, 4, 4, 0, 2.4, 0, 4, 4, 0, 0, 0, .05, 24, 0
			10, 4, 4,  7, 4, 4, 6, 0, 2.4, 0, 4, 4, 0, 0, 0,   1, 24, 0 
    }
    ,
    {
    //10, 40, 3, 8.5, 0, 4, 3.6, 0, 2.4, 0, 4, 1, 0, 0, 0, 8, 25, 0
      10, 40, 8, 8.5, 0, 4,   5, 0, 2.4, 0, 4, 1, 0, 0, 0, 5, 25, 0
    }
    ,
    {
      11, 10, 115, 9.5, 0, 3, 3.7, .01, 1.2, 3.5, 0, 0, 0, 0, 0, .13, 26, 0
    }
    ,
    {
      11, 20, 40, 10, 0, 3, 2.9, .02, 1, 1, 0, 0, 0, 0, 0, .571, 27, 0
    }
    ,
    {
      12, 10, 110, 8.0, 1.5, 3, 3, .015, 1.5, 56, 4, 3, 0, 0, 0, .135, 28, 0
    }
    ,
    {
      12, 10, 110, 7.5, 1.5, 3, 3, .015, 1.5, 56, 4, 3, 0, 0, 0, .135, 28, 0
    }
    ,
    {
      12, 18, 60, 8, 2, 2, 2, .02, 1.8, 40, 4, 1, 0, 0, 0, .4, 29, 0
    }
    ,
    {
      13, 10, 12, 7, 3, 2, 1.8, 0, 1.5, 0, 0, 0, 30, 20, .5, 0, 30, 0
    }
    ,
    {
      13, 12, 20, 9, 3, 3, 3.2, 0, 2.4, 0, 0, 0, 20, 10, .6, 0.6, 31, 0
    }
    ,
    {
      14, 5, 12, 8.5, 2.5, 1, 1, 0, 1.2, 0, 5, 5, 0, 0, 0, 0, 32, 0
    }
    ,
    {
      14, 6.5, 10, 9.5, 3.5, 3, 3.6, .05, 1, 0, 1, 1, 0, 0, 0, 0, 33, 0
    }
    ,
    {
    //15, 25, 5, 11, 6, 5, 5.4, .03, 1.5, 0, 0, 0, 20, 10, .5, 5, 34, 0
			15, 25, 5, 11, 6, 5, 6.4, .03, 1.5, 0, 0, 0, 20, 10, .5, 5, 34, 0
    }
    ,
    {
      15, 15, 62, 12, 7.5, 4, 4, .02, 1.8, 0, 0, 0, 20, 18, .7, 0, 35, 0
    }
    ,
    {
      16, 25, 10, 9, 0, 4, 3.7, .005, 2.5, 3, 0, 0, 0, 0, 0, 3, 36, 0
    }
    ,
    {
    //16, 40, 20, 10.5, 0, 5, 4.6, .11, 3, 7, 0, 0, 0, 0, 0, 3.12, 37, 0
			16, 40, 20, 10.5, 0, 5, 4.6, .11, 3, 7, 0, 0, 0, 0, 0, 3.12, 37, 0
    }
  };

  for(int i = 0; i < 38; i++)
  {
	  loans[i] = Loans[i];
  }
#pragma endregion 
}

void GetLnSales(LnSalesT lnSales[], int size)
{
	  #pragma region LnSalesT
    LnSalesT LnSales[6] =
  {
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
  };
#pragma endregion

	for(int i = 0; i < size; i++)
	{
		lnSales[i] = LnSales[i];
	}
}

ResGloT GetResGlo()
{
		#pragma region ResGloT 
  ResGloT ResGlo =
  {
    {
      // Start of == Cur ==
      {
        // Start of Sal
        //    Admin[5],
        {
          150, 22, 0, 0, 0
        }
        ,
        //    Ops[10],
        {
          50, 100, 150, 150, 150, 0, 0, 0, 0, 0
        }
        ,
        //    Loans[10],
        {
          50, 40, 100, 40, 90, 150, 200, 0, 0, 0
        }
        ,
        //    Cred[10],
        {
          20, 20, 20, 12, 40, 40, 50, 0, 0, 0
        }
        ,
        //    Dep[5]
        {
          80, 20, 350, 25, 0
        }
        ,
      }
      ,  // End of Sal
      {
        // Start of Media
        //    Loans[20],
        {
          10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        }
        ,
        //    Dep[10],
        {
          50, 10, 20, 55, 50, 40, 50, 50, 15, 0,
        }
        ,
        //    Gen;
        20
      }
      ,  // End of Media
      //  float Rent[5];
      {
        8, 1.7, 1.7, 3.8, 0
      }
    }
    ,  // End of (Cur)
    {
      // Start of == Req ==
      {
        // Start of Sal
        //    Admin[5],
        {
          40, 12, 0, 0, 0
        }
        ,
        //    Ops[10],
        {
          12, 8, 8, 20, 25, 0, 0, 0, 0, 0
        }
        ,
        //    Loans[10],
        {
          15, 12, 12, 13, 14, 12, 10, 0, 0, 0
        }
        ,
        //    Cred[10],
        {
          12, 12, 12, 12, 12, 9, 9, 0, 0, 0
        }
        ,
        //    Dep[5]
        {
          10, 5, 10, 7, 0
        }
        ,
      }
      ,  // End of Sal
      {
        // Start of Media
        //    Loans[20],
        {
          10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        }
        ,
        //    Dep[10],
        {
          100, 10, 20, 55, 50, 40, 50, 50, 15, 0,
        }
        ,
        //    Gen;
        33
      }
      ,  // End of Media
      //  float Rent[5];
      {
        8000, 6500, 6200, 3800, 0
      } // Set to SumOfSals * 1
    }
    ,  // End of (Req)
    {
      // Start of == Eff ==
      {
        // Start of Sal
        //    Admin[5],
        {
          1, 1, 1, 1, 1
        }
        ,
        //    Ops[10],
        {
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1
        }
        ,
        //    Loans[10],
        {
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1
        }
        ,
        //    Cred[10],
        {
          1, 1, 1, 1, 1, 1, 1, 1, 1, 1
        }
        ,
        //    Dep[5]
        {
          1, 1, 1, 1, 1
        }
        ,
      }
      ,  // End of Sal
      {
        // Start of Media
        //    Loans[20],
        {
          10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
        }
        ,
        //    Dep[10],
        {
          100, 10, 20, 55, 50, 40, 50, 50, 15, 0,
        }
        ,
        //    Gen;
        33
      }
      ,  // End of Media
      //  float Rent[5];
      {
        1, 1, 1, 1, 1
      }
    }
    ,  // End of (Eff)
    {
      // Start of == Adj ==
      {
        // Start of Sal
        //    Admin[5],
        {
          40, 12, 0, 0, 0
        }
        ,
        //    Ops[10],
        {
          12, 8, 8, 20, 25, 0, 0, 0, 0, 0
        }
        ,
        //    Loans[10],
        {
          15, 12, 12, 13, 14, 12, 10, 0, 0, 0
        }
        ,
        //    Cred[10],
        {
          12, 12, 12, 12, 12, 9, 9, 0, 0, 0
        }
        ,
        //    Dep[5]
        {
          10, 5, 10, 7, 0
        }
        ,
      }
      ,  // End of Sal
      {
        // Start of Media
        //    Loans[20],
        {
          10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 20, 10, 10, 10, 10,
        }
        ,
        //    Dep[10],
        {
          50, 10, 20, 55, 50, 40, 50, 50, 15, 0,
        }
        ,
        //    Gen;
        33
      }
      ,  // End of Media
      //  float Rent[5];
      {
        142, 234, 197, 141, 0
      } // Set to SumOfSals * 1
    }
    ,  // End of (Adj)
    {
      // Start of == Old ==
      {
        // Start of Sal
        //    Admin[5],
        {
          40, 12, 0, 0, 0
        }
        ,
        //    Ops[10],
        {
          12, 8, 8, 20, 25, 0, 0, 0, 0, 0
        }
        ,
        //    Loans[10],
        {
          40, 50, 60, 40, 80, 60, 120, 0, 0, 0
        }
        ,
        //    Cred[10],
        {
          12, 12, 12, 12, 12, 9, 9, 0, 0, 0
        }
        ,
        //    Dep[5]
        {
          80, 40, 20, 100, 0
        }
        ,
      }
      ,  // End of Sal
      {
        // Start of Media
        //    Loans[20],
        {
          40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
        }
        ,
        //    Dep[10],
        {
          100, 10, 20, 55, 50, 40, 50, 50, 15, 0,
        }
        ,
        //    Gen;
        33
      }
      ,  // End of Media
      //  float Rent[5];
      {
        3, 5, 4, 3, 0
      }
    }
    ,  // End of (Old)
  };
  return ResGlo; 
  #pragma endregion 
}

CostsGloT GetCostsGlo()
{
#pragma region CostsGloT
  CostsGloT CostsGlo =
  {
    //  PremT Prem, Depr;
    {
      8.56, 5.85, 8.22, 4.1, 0
    }
    ,
    {
      4.7, 2.1, 3, 1.5, 0
    }
    ,
    // AdOpsPremInv
    0,
    // Open[10]
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    // { float Taxable, Rate; } TaxHistT[ 5 MaxTaxH ];
    {
      {
        8, 0.4
      }
      ,
      {
        0, 0
      }
      ,
      {
        0, 0
      }
      ,
      {
        4, 0.4
      }
      ,
      {
        0, 0
      }
    }
    ,
    // EVTA;
    1,
  };
  return CostsGlo; 
  #pragma endregion
}

DepT GetDep()
{
	  #pragma region DepT
  DepT Dep =
  {
    //   DemBal, SavBal, DemCnt, SavCnt, Loy
    {
      // BusNatCl[MaxBusC
      {
        .5, 0, 10, 0, .6
      }
      ,
      {
        4, 0, 20, 0, .6
      }
      ,
      {
        2, 0, 10, 0, .6
      }
      ,
      {
        3, 0, 8, 0, .6
      }
      ,
    }
    ,
    {
      // BusMidCl[MaxBusC
      {
        .5, 0, 25, 0, .7
      }
      ,
      {
        2.7, 0, 30, 0, .7
      }
      ,
      {
        2.5, 0, 15, 0, .7
      }
      ,
      {
        4, 0, 15, 0, .7
      }
      ,
    }
    ,
    {
      // BusLocalCl[MaxBu
      {
        1, 0, 100, 0, .8
      }
      ,
      {
        3.5, .2, 120, 10, .8
      }
      ,
      {
        2.5, .3, 50, 6, .8
      }
      ,
      {
        2.5, .4, 30, 3, .8
      }
      ,
    }
    ,
    {
      // BusRECl[MaxBusCl
      {
        .8, 0, 40, 0, .6
      }
      ,
      {
        2.4, .1, 80, 6, .6
      }
      ,
      {
        3.0, .15, 35, 3, .6
      }
      ,
      {
        2.6, .2, 12, 2, .6
      }
      ,
    }
    ,
    {
      // BusProfCl[MaxBus
      {
        .4, 0, 30, 0, .6
      }
      ,
      {
        1.6, .1, 18, 4, .6
      }
      ,
      {
        2.0, .2, 25, 2, .6
      }
      ,
      {
        1.2, .2, 8, 2, .6
      }
      ,
    }
    ,
    {
      // BusGenCl[MaxBusC
      {
        .3, 0, 120, 0, .6
      }
      ,
      {
        1.5, 0, 50, 0, .6
      }
      ,
      {
        1.8, 0, 30, 0, .6
      }
      ,
      {
        1.2, .1, 4, 2, .6
      }
      ,
    }
    ,
    //  { float Bal; }
    //  PubDepClT PubDepCl[MaxPubCls];   //   192
    {
      {
        1.25, 500
      }
      ,
      {
        2.5, 250
      }
      ,
      {
        2.5, 208
      }
      ,
      {
        3.75, 250
      }
      ,
      {
        5, 200
      }
      ,
      {
        5, 50
      }
      ,
      {
        7, 25
      }
    }
    ,
    //  DemBal, SavBal, NowBal, MonBal,
    //  DemCnt, SavCnt, NowCnt, MonCnt;
    //  IndDepClT IndDepCl[MaxIndCls];   //   199 - 254
    {
      {
        2, 2, 0, 0, 8000, 8000, 8000, 0
      }
      ,
      {
        6, 2, 0, 4, 8000, 4000, 8000, 4000
      }
      ,
      {
        6, 3, 2, 4, 4000, 2000, 4000, 2000
      }
      ,
      {
        3, 3, 3, 6, 3000, 1200, 3000, 2000
      }
      ,
      {
        1, 3, 3, 5, 3000, 1200, 3000, 2000
      }
      ,
      {
        0, 3, 3, 5, 3000, 1200, 3000, 2000
      }
      ,
      {
        0, 3, 3, 5, 3000, 1200, 3000, 2000
      }
      ,
      {
        0, 3, 3, 5, 3000, 1200, 3000, 2000
      }
      ,
      {
        0, 4, 3, 8, 3000, 1200, 3000, 2000
      }
      ,
      {
        0, 4, 4, 8, 3000, 1200, 3000, 2000
      }
      ,
      {
        0, 4, 4, 8, 3000, 1200, 3000, 2000
      }
      ,
      {
        0, 4, 4, 8, 3000, 1200, 3000, 2000
      }
      ,
      {
        0, 4, 5, 9, 3000, 1200, 3000, 2000
      }
      ,
      {
        0, 2, 6, 9, 600, 50, 600, 200
      }
      ,
      {
        0, 1, 6, 10, 400, 10, 400, 400
      }
      ,
      {
        0, 1, 6, 11, 400, 10, 400, 300
      }
      ,
    }
    ,
    {
      //   float BnkShr[ Ind7E + 1 ];
      .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1, .1,
    }
    ,
  };
  return Dep;
  #pragma endregion 
}

void GetTimeDep(TimeDepT timeDep[], int size)
{
	  #pragma region TimeDepT
  TimeDepT TimeDep[] =
  {
    {
      1, HotE, 10, 7.1, 2, 1, 0, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    }
    ,
    {
      2, BusProE, 4, .4, 5, 0, 12, 0, 0, 0,
      {
        10, 20, 30, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    }
    ,
    {
      3, BusLocE, 3, 6, 9, 1, 20, 0, 0, 0,
      {
        0, 10, 40, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      }
    }
    ,
    {
      4, IndE, 5, 5.1, 8, 1, 1500, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      5, IndE, 5, 7.5, 9, 1, 1000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      6, IndE, 3.5, 7.2, 10, 1, 1000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      7, IndE, 4, 10.4, 11, 1, 250, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      8, IndE, 4, 0.1, 12, 0, 500, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      9, IndE, 5, 0, 9, 0, 200, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30, 30, 0, 40, 0
      }
    }
    ,
    {
      10, IndE, 3, 9.8, 13, 1, 300, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      11, IndE, 3, 7.5, 14, 1, 1000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      12, IndE, 4, .1, 15, 0, 500, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 7, 9, 9, 10, 10
      }
    }
    ,
    {
      13, IndE, 3, 10.2, 16, 1, 100, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 20, 30, 40
      }
    }
    ,
    {
      14, IndE, 3, 0, 17, 0, 400, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 60, 30
      }
    }
    ,
    {
      15, IndE, 3, 7.4, 18, 1, 400, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50
      }
    }
    ,
    {
      16, IndE, 2, 8.75, 19, 1, 500, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50
      }
    }
    ,
    {
      17, IndE, 8,  - .1, 10, 0, 500, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 50
      }
    }
    ,
    {
      18, IndE, 12, 7.1, 25, 1, 3000, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 50, 30, 0, 0
      }
    }
    ,
    {
      19, IndE, 8,  - .2, 30, 0, 3000, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 0
      }
    }
    ,
    {
      20, IndE, 9, 9.4, 35, 1, 3000, 0, 0, 0,
      {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 20, 60, 0, 0
      }
    }
    ,
    {
      21, IndE, 50, 5.5, 3, 1, 25000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 12, 7, 9, 9, 5, 10
      }
    }
    ,
    {
      22, IndE, 8, 7.5, 25, 1, 25000, 0, 0, 0,
      {
        0, 2, 4, 6, 6, 6, 6, 6, 6, 6, 7, 12, 9, 9, 10, 5
      }
    }
    ,
  };
  #pragma endregion

	for(int i = 0; i < size; i++)
	{
		timeDep[i] = TimeDep[i];
	}
}

FundsGloT GetFundsGlo()
{
	  #pragma region FundsGloT
  FundsGloT FundsGlo =
  {
    //  RateAmntT Repo,FRB,FF;
    {
      0, 0
    }
    ,
    {
      0, 0
    }
    ,
    {
      0, 0
    }
    ,
    //  struct { float Amnt,Mat,Rate,Duration; } CDs[MaxCD],PTs[MaxPT];
    {
      {
        30, 4, 7, 5
      }
      ,
      {
        25, 6, 5.5, 5
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
    }
    ,
    {
      {
        20, 5, 5.2, 5
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0
      }
    }
    ,
    //  RateAmntT CDsAvail[MaxCDA],
    {
      {
        10, 8
      }
      ,
      {
        10, 8
      }
      ,
      {
        10, 8
      }
      ,
      {
        10, 8
      }
      ,
      {
        10, 8
      }
      ,
      {
        10, 8
      }
      ,
      {
        10, 8
      }
      ,
      {
        10, 8
      }
      ,
    }
    ,
    //  SubLTD_IssuedT SubLTD_IssuedAvail[MaxLTA];
    {
      {
        0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0
      }
      ,
      {
        10, 60, 0, 0, 8, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0
      }
      ,
    }
    ,
    //  Open[2], CeditRating
    0, 0, 0,
    //  struct { float Amnt,Rate,Fixed,Mat,Duration; } HFs[MaxHF];
    {
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0
      }
      ,
    }
    ,
    //  SubLTD_IssuedT SubLTD[MaxLT];
    {
      {
        0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0
      }
      ,
      {
        0, 0, 0, 0, 0, 0
      }
      ,
    }
    ,
    //  float Open_A[2], HFMem, HFAmnt, Open_B;
    {
      0, 0
    }
    , 1, 0, 0,
    //  Loans, Sec, SecMat, Stock, Funds, HFs,           SubLTD,     CDs,    PTs, Taxes
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    //  Loans, Sec,         Stock, Funds, HFs, HFsMat, SubLTD_IssuedMat, CDsMat, PTsMat, Taxes
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
    ,
    //  NetDay1, FundsSold, Repos
    0, 0, 0,
    //   FRBs, FF, FRBClosed
    0, 0, 0,
    //   StockAmntRP, SharesRP, ZMatCDs,     IssuePlanAmnt, IssuePricePerShare, IssuePrice, NewUniqueName; SimQtr,Dirty,_ID
    0, 0, 0, 0, 0, 0, 0, 3, 1,
  }; // FundsGloT; // BK01.DAT [21-23]
  return FundsGlo;
  #pragma endregion 
}

void GetSecPurDec(SecPurDecT decisions[], int total)
{
	for(int i = 0; i < total; i++)
	{
		decisions[0][i].type = UnSet;
		decisions[0][i].amount = UnSet;
		decisions[0][i].maturity = UnSet;
	}
}