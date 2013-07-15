#include "Reporting.h"
#include "UIHelper.h" // DecSec2(int Dec)

char RptNeu[6];
int _DB; // _DB is true when printing a students decisions at the start of a quarter


//#define DecNeu   { int Ne = ( int ) *P++ ; Str( _T, "%s%s", Ne != *PP++ ? "_": "", \
//Ne < 0 ||Ne >= MAXSTR ? " ": str_line[ Ne ]); AaA( _T ); }
/// <summary> This function converts a number to it's mnemonic and places it in a field for reporting. </summary>
/// <param name="**ptrP"> The number to be converted to string and placed in the current field </param>
/// <param name="**ptrPP"> </param> 
void DecNeu(float **ptrP, float **ptrPP) 
{
	//??? *str_line[] also exists in decisionentry.cpp
	extern char *str_line[];

	int Ne = ( int ) *((*ptrP)++) ;
	
	Str( _T, "%s%s", Ne != *((*ptrPP)++) ? "_": "",
		Ne < 0 ||Ne >= MAXSTR ? " ": str_line[ Ne ]);
	
	AaA( _T );
}

/// <summary> This function always creates a decision form and writes time, bank name, report name and community name in the first line of the report. </summary>
/// <param name="*S"> is the decision form name, for example, 0D010.txt. </param>
void DecFormFirst(char *S)
{
  strcpy(RptNeu, S);
  if(Bank_Book && (Eq(RptNeu, "D050") || Eq(RptNeu, "D060")))
  {
    Get_(Bank_Book, L"Worksheets", RptNeu);
    if(Sheet = OleRV.pdispVal)
      Row =  - 1;
  }
  char Z[10];
  Str(Z, "0D%c%s", _DB ? 'B' : '0', RptNeu + 2);
  ODF(Z, "TXT", "w");  //open the decision form (e.g. 0D010.txt) text file to write
  Out = fp;
  if(_DB)
  {
    fprintf(Out, "\n%s ,  %s\n", TimeStamp, RptNeu);
  }
  else
  {
    char N[] = "                               ";
    int NZ = strlen(N), BZ = strlen(BankName);
    memmove(N + (BZ >= NZ ? 0 : (NZ - BZ) / 2), BankName, BZ >= NZ ? NZ : BZ);
    fprintf(Out, "\n%s  %s  %s  === %s ===  %s\n", TimeStamp, N, _For, RptNeu, BankNeu); ///W 
  }
}

/// <summary> This function writes the footer of a decision report and closes out a decision form. </summary>
/// <param name="L"> is the number of blank lines between the report and the footer. </param> 
/// <param name="Page"> is the page number in the report. </param>
void DecFormLast(int L, int Page)
{
  for(int intIndexOfCount = 0; intIndexOfCount < L; intIndexOfCount++)
    fprintf(Out, "\n");

  fprintf( Out
  , "==========================================================="
  "================================\n"
  "Approval:                       Date:              %s  === %s "
  "===  %s  Page %d",  _DB ? _For : "         ", RptNeu
  , _DB ? BankNeu:"       ", Page ); 

  CloseOut();
  Rel(Sheet);
}

//#define PDM( Cnt, Len, Dot ) NumLen = Len; DotLen = Dot ; { for(int intIndexOfCount = 0; intIndexOfCount < Cnt; intIndexOfCount++) _FA( * P ++, * P != * PP ++ ); }
/// <summary> This function fills in a series of fields using a certain mantissa. </summary>
/// <param name="Cnt"> determines the maximum of the loop inside this function.  </param>
/// <param name="Len"> The length of the number. </param>
/// <param name="Dot"> The length of the mantissa. </param>
/// <param name="**ptrP"> is pointer to structure which has the decision form from the user input. </param>
/// <param name="**ptrPP"> is pointer to structure loading data from the data file. </param>
void PDM(int Cnt, DWORD Len, DWORD Dot, float **ptrP, float **ptrPP) {

	NumLen = Len;
	DotLen = Dot;
	{
		//Increments the pointer, and uses derefrenced value to pass to _FA
		for(int intIndexOfCount = 0; intIndexOfCount < Cnt; intIndexOfCount++) {
			_FA( *((*ptrP)++), *(*ptrP) != *((*ptrPP)++) );
		}
	}
}

//#define PDM2 PDM(3,5,2) PDM(2,3,0)
#define PDM2 PDM(3, 5, 2, &P, &PP); PDM(2, 3, 0, &P, &PP);

//Remove: procedure PDM(cnt = 1) used instead.
// Fills in a field given a specified mantissa
//#define PDM3( Len, Dot ) NumLen = Len; DotLen = Dot; _FA( * P ++, * P != * PP ++ );


// MOVE THESE to DecisionEntry.cpp
//#define DecP( J )  (FeP) ( _DecP [ J ] + DecSz [ J ] * Bank )
/// <summary> Returns the location of a bank's input fields given an decision form number and the bank number. </summary>
/// <param name="inJ"> is a decision form number from 0 to 7. </param>
/// <returns> the pointer to a bank's decsion form structure. </returns>
//float *DecP(short inJ) {
//
//	return (FeP) ( _DecP [ inJ ] + DecSz [ inJ ] * Bank );
//}

//#define _DepP( J ) __DepP( J, PriorDec )
/// <summary> This function reads in a bank's input decisions as they were at the very start of the quarter. </summary>
/// <param name="J"> is an integer from decision form enumeration. </param>
/// <param name="*P"> is the pointer to <see cref="PriorDec" />. </param>
/// <returns> P. </returns> 
float *__DepP(int J, float *P) {

  P += J &1 ? 0 : 512 / 4;  
  DatIO("BK00", "r", DecSec2(J), P, 512);
  return P;
}


//Prints all the decision forms, both at the end of the quarter, and at the start of the quarter, after students have made their decisions.
/// <summary> This function writes the decision form. </summary>
/// <param name="DB"> is the integer which decides the value of _For such as "For Q2/10". </param>
void PrnDecs(int DB)
{
	extern float IssuePricePerShare; //from B550
	extern int MortBanking[MaxB]; //from loans
	extern int CFld, OFld;

  _DB = DB;

  Str(_For, DB ? "         " : "For %s", FQ[1]);

  float PriorDec[1024 / 4];

  int Page = 0;
  // write security purchase and sale decision into report D010.txt
  DecFormFirst("D010");

  {
    //float *P = DecP(secpur),   *PP = _DepP(secpur),  *x = DecP(secsal),  *X = _DepP(secsal);
    
	float *P = DecP(secpur);  // return the starting location of security purchase form 
	float  *PP = __DepP(secpur, PriorDec);
	float *x = DecP(secsal);
	float *X = __DepP(secsal, PriorDec);

    OFld = CFld;

    {
      for(int intIndexOfCount = 0; intIndexOfCount < 18; intIndexOfCount++)
      {
        //DecNeu
				DecNeu(&P, &PP);
				//PDM3(2, 0)
				PDM(1, 2, 0, &P, &PP);
				//PDM3(3, 0)
				PDM(1, 3, 0, &P, &PP);
		
				_FA(*x++,  *x !=  *X++);
        NumLen = 2;
        _FA(*x++,  *x !=  *X++);
      }
    }
  }

  {
  Paint(ReadReport("ReportTemplates\\PrnDecsa.rpt")); //Write D010.txt using PrnDecsa.rpt template
  }
  DecFormLast(2, ++Page);
  // Write the loan decisions into report D030.txt
  {
    //float *P = DecP(loans),  *PP = _DepP(loans);
    float *P = DecP(loans);
		float *PP = __DepP(loans, PriorDec);

    DecFormFirst("D030");
    OFld = CFld;

    PDM2
	//PDM3(4, 2)
		PDM(1, 4, 2, &P, &PP); 
	
		PDM2 
	//PDM3(2, 0)
		PDM(1, 2, 0, &P, &PP); 
	
		PDM2 
	//PDM3(4, 2)
		PDM(1, 4, 2, &P, &PP); 

		PDM2 
	//PDM3(2, 0)
		PDM(1, 2, 0, &P, &PP);

		PDM2
	//PDM3(4, 2)
		PDM(1, 4, 2, &P, &PP); 
	
		PDM2
	//PDM3(2, 0)
		PDM(1, 2, 0, &P, &PP); 

		PDM2
	//PDM3(4, 2)
		PDM(1, 4, 2, &P, &PP); 

		PDM2
	//PDM3(4, 2)
		PDM(1, 4, 2, &P, &PP); 

		PDM2
	//PDM3(2, 0)
		PDM(1, 2, 0, &P, &PP); 

		PDM2
	//PDM3(2, 0)
		PDM(1, 2, 0, &P, &PP); 

		PDM2
		PDM2
		//PDM3(2, 0)
		PDM(1, 2, 0, &P, &PP); 

		PDM2 
		//PDM3(3, 0)
		PDM(1, 3, 0, &P, &PP); 

		PDM2
		//PDM3(2, 0)
		PDM(1, 2, 0, &P, &PP); 
	
		PDM2 
		//PDM3(3, 0)
		PDM(1, 3, 0, &P, &PP); 

		PDM2

    {
			Paint(ReadReport("ReportTemplates\\PrnDecsb.rpt"));
    }

    DecFormLast(1, ++Page);
  }
  
	//Write mortgage banking and loan sale decision into report D040.txt
  DecFormFirst("D040");

  {
    //float *P = DecP(lnprsl),  *PP = _DepP(lnprsl);
		float *P = DecP(lnprsl);
		float *PP = __DepP(lnprsl, PriorDec);
	
    OFld = CFld;

    //DecNeu
		DecNeu(&P, &PP);
	
		AaA(DB ? " " : (MortBanking[Bank] ? " ON" : "OFF"));

    //PDM(20, 3, 0)
		PDM(20, 3, 0, &P, &PP);
  }

  {
		Paint(ReadReport("ReportTemplates\\PrnDecsc.rpt"));
  }

  DecFormLast(37, ++Page);
  // Write credit adminstration decision to report D050.txt
  {
    //float *P = DecP(credadm),  *PP = _DepP(credadm);
		float *P = DecP(credadm);
		float *PP = __DepP(credadm, PriorDec);
	

    DecFormFirst("D050");

    OFld = CFld;

    //PDM(5, 3, 0)
		PDM(5, 3, 0, &P, &PP);

		//DecNeu 
		DecNeu(&P, &PP);

		//PDM(7, 3, 0)
		PDM(7, 3, 0, &P, &PP);
	
		//DecNeu 
		DecNeu(&P, &PP);
	
		//PDM(7, 3, 0)
		PDM(7, 3, 0, &P, &PP);
	
		//DecNeu 
		DecNeu(&P, &PP);

		//PDM(2, 3, 0)
		PDM(2, 3, 0, &P, &PP);
    {
      for(int intIndexOfCount = 0; intIndexOfCount < 4; intIndexOfCount++)
      {
        //PDM(6, 3, 0)
				PDM(6, 3, 0, &P, &PP);
		
				//DecNeu
				DecNeu(&P, &PP);
      }
    }

    //PDM(5, 3, 0)
		PDM(5, 3, 0, &P, &PP);
	
		//DecNeu
		DecNeu(&P, &PP);
	
		//PDM(2, 3, 0)
		PDM(2, 3, 0, &P, &PP);
    {
      for(int intIndexOfCount = 0; intIndexOfCount < 4; intIndexOfCount++)
      {
        //PDM(6, 3, 0)
				PDM(6, 3, 0, &P, &PP);
		
				//DecNeu
				DecNeu(&P, &PP);
      }
    }
    //PDM3(5, 3)
		PDM(1, 5, 3, &P, &PP); 
	
		//DecNeu
		DecNeu(&P, &PP);
  }
  {
  Paint(ReadReport("ReportTemplates\\PrnDecsd.rpt"));
  }
  DecFormLast(1, ++Page);
	//Write deposits decision into report D060.txt
  {
    //float *P = DecP(depos),  *PP = _DepP(depos);
    float *P = DecP(depos);
		float *PP = __DepP(depos, PriorDec);

		DecFormFirst("D060");
    OFld = CFld;
    //PDM(3, 5, 2)
		PDM(3, 5, 2, &P, &PP);
		NumLen = 4;

    {
      for(int intIndexOfCount = 0; intIndexOfCount < 3; intIndexOfCount++)
        _FA(*P++ / 100,  *P !=  *PP++);
    }

    //PDM(8, 5, 2)
		PDM(8, 5, 2, &P, &PP);
		_FA(*P++ / 100,  *P !=  *PP++);
    
		//PDM(4, 5, 2)
		PDM(4, 5, 2, &P, &PP);
	
		//PDM(2, 7, 0)
		PDM(2, 7, 0, &P, &PP);
	
		//PDM(4, 5, 2)
		PDM(4, 5, 2, &P, &PP);

    for(int intIndexOfCount = 0; intIndexOfCount < 8; intIndexOfCount++)
    {
      NumLen = 3;
      DotLen = 0;
      _FA(*P++,  *P !=  *PP++);
      NumLen = 5;
      DotLen = 2;
      _FA(*P++,  *P !=  *PP++);
      
			//DecNeu
			DecNeu(&P, &PP);
    }

    //PDM(10, 3, 0)
		PDM(10, 3, 0, &P, &PP);
	}

  {
  Paint(ReadReport("ReportTemplates\\PrnDecse.rpt"));
  }

  DecFormLast(0, ++Page);
  //Write Treasury management decision into report D070.txt
  {
    //float *P = DecP(treas),  *PP = _DepP(treas);
    float *P = DecP(treas);
		float *PP = __DepP(treas, PriorDec);
    
    DecFormFirst("D070");
    OFld = CFld;

    for(int intIndexOfCount = 0; intIndexOfCount < 16; intIndexOfCount++)
    {
			//DecNeu 
			DecNeu(&P, &PP);
			
			//PDM(2, 3, 0)
			PDM(2, 3, 0, &P, &PP);
    }

    //PDM3(5, 2)
		PDM(1, 5, 2, &P, &PP); 
		//PDM3(2, 0)
		PDM(1, 2, 0, &P, &PP); 
		//PDM3(5, 2)
		PDM(1, 5, 2, &P, &PP);
		//PDM3(3, 0)
		PDM(1, 3, 0, &P, &PP);
		//PDM3(2, 0)
		PDM(1, 2, 0, &P, &PP);

	// TODO: add IssuePricePerShare as AdjustedIssuePricePerShare to FundGloT
    if(!IssuePricePerShare)
      AaA("  N/A");
    else
    {
      NumLen = 5;
      DotLen = 2;
      FA(IssuePricePerShare);
    }

    {
      AcT *Ac = BOA[Bank] + YrsQtr;
      LiaT *Lia = &Ac->Bal.Liabilities;
      NumLen = 10;
      DotLen = 0;

      FA(Mill *Lia->Stock.Common / PAR);

      {
        NumLen = 5;
        DotLen = 2;
        FA(Ac->StkPrice *(float)STOCKREPURPREM);
      }
    }

    //PDM3(6, 0)
		PDM(1, 6, 0, &P, &PP);
  }

  {
	  Paint(ReadReport("ReportTemplates\\PrnDecsf.rpt"));
  }

  DecFormLast(1, ++Page);
  //write general administration decision in report D080.txt
	{
    //float *P = DecP(genadm),  *PP = _DepP(genadm);
    float *P = DecP(genadm);
		float *PP = __DepP(genadm, PriorDec);

    DecFormFirst("D080");
    OFld = CFld;

    //PDM(25, 3, 0)
		PDM(25, 3, 0, &P, &PP);
		//PDM3(4, 1)
		PDM(1, 4, 1, &P, &PP);
		//PDM(5, 4, 0)
		PDM(5, 4, 0, &P, &PP);
  }
  {
		Paint(ReadReport("ReportTemplates\\PrnDecsg.rpt"));
  }

  DecFormLast(1, ++Page);
}