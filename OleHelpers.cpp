
#include <Ole2.h>
//#include <stdio.h>
#include <direct.h>
#include "GlobalDef.h" 
#include "UIHelper.h"  //MessBox(), PW_E
#include "FileIOHelper.h" //Cop()

OleT Excel, Workbooks, Bank_Book, ComBook, Sheet, Range, Selection;
VARIANT OleRV, Var;
/// <summary> This function releases an Ole object, and sets it to zero. </summary>
/// <param name="&X">IDispatch to the Ole object. </param>
void Rel(IDispatch * &X)
{
  if( ! X)
  {
    X = 0;
  }
  else if((short)X ==  - 1)
  {
    X = 0;
  }
  else
  {
    X->Release();
    X = 0;
  }
}

/// <summary> This function releases an Ole object, and sets it to zero. </summary>
/// <param name="&X"> IUnknown is the base interface of every other COM interface. </param> 
void RelUnk(IUnknown * &X)
{
  if( ! X)
  {
    X = 0;
  }
  else if((short)X ==  - 1)
  {
    X = 0;
  }
  else
  {
    X->Release();
    X = 0;
  }
}

/// <summary> This function frees the memory of OLE's VARIANT structure. </summary>
/// <param name="X"> is OLE's VARIANT structure. </param>
void FrV(VARIANT X)
{
  if(X.vt == VT_DISPATCH)
  {
    //X = 0;
    return ;
  }
  else
  {
    VariantClear(&X);
    memset(&X, 0, sizeof X);
  }
}

// Handshakes with OLE to automate Excell
/// <summary> This function invokes the workbook from Excel. </summary> 
/// <param name="IdenType"> is the Flag for IDispatch::Invoke.  </param>
/// <param name="pDisp"> is the address of IDispatch to Excel.</param>
/// <param name="*ptName"> is the pointer to the string for example L"Workbooks".</param>
/// <param name="Cnt_Comming"> </param>
/// <param name="*SP"> </param>
//void Ole(int IdenType, OleT pDisp, /*ushort ptName*/ LPCWSTR ptName, int Cnt_Comming, char *SP)
//{
//  if( ! Bank_On &&  ! ComOn)
//  {
//    return ;
//  }
//  HRESULT rv;
//  while(1)
//  {
//    if( ! pDisp || (short)pDisp ==  - 1)
//    {
//      rv =  - 1;
//      break;
//    }
//
//    FrV(OleRV);
//
//    Respond();
//
//    DISPPARAMS ParamList =
//    {
//      0, 0, 0, 0
//    };
//
//    DISPID IdenID;
//    char buf[200], szName[200];
//
//    WideCharToMultiByte(CP_ACP, 0, ptName,  - 1, szName, 256, 0, 0);
//
//    rv = pDisp->GetIDsOfNames(IID_NULL, *ptName, 1, LOCALE_USER_DEFAULT, &IdenID);
//
//    if(FAILED(rv))
//    {
//      Str(buf, "IDispatch::GetIDsOfNames(\"%s\") failed w/err 0x%08lx", szName, rv);
//      // if(!UI_Up)MessageBox(0, buf, 0, 0); 
//      break;
//    }
//
//    VARIANT *pArgs = new VARIANT[Cnt_Comming + 1];
//    {
//      for(int intIndexOfCount = 0; intIndexOfCount < Cnt_Comming; intIndexOfCount++)
//        pArgs[Cnt_Comming - intIndexOfCount - 1] = *(VARIANT*)SP, SP += sizeof(VARIANT);
//    }
//
//    ParamList.cArgs = Cnt_Comming;
//    ParamList.rgvarg = pArgs;
//
//    if(IdenType &DISPATCH_PROPERTYPUT)
//    {
//      ParamList.cNamedArgs = 1;
//      DISPID dispidNamed = DISPID_PROPERTYPUT;
//      ParamList.rgdispidNamedArgs = &dispidNamed;
//    }
//
//    rv = pDisp->Invoke(IdenID, IID_NULL, LOCALE_SYSTEM_DEFAULT, IdenType, &ParamList, &OleRV, 0, 0);
//
//    if(FAILED(rv))
//    {
//      Str(buf, "IDispatch::Invoke(\"%s\"=%08lx) failed w/err 0x%08lx", szName, IdenID, rv);
//      // if(!UI_Up)MessageBox(0, buf, 0, 0); 
//      break;
//    }
//
//    delete []pArgs;
//    break;
//  }
//
//  if(rv < 0 || OleRV.vt <= 1 || OleRV.vt == VT_ERROR)
//  {
//    FrV(OleRV);
//  }
//}
/// <summary> This function gets the Excel Workbooks. </summary>
/// <param name="X"> is the address of IDispatch to Excel. </param>
/// <param name="*Y"> is the pointer to the string for example L"Workbooks".</param>
/// <param name="Z"> is the integer. </param>
void Get(OleT X, /*ushort *Y*/ LPCWSTR Y, int Z, ...)
{
//  Ole(DISPATCH_PROPERTYGET, X, Y, Z, (LnP) &Z + 4);
//  FrV(Var);
}

/// <summary> This function puts the Excel Workbook. </summary>
/// <param name="X"> is the address of IDispatch to Excel. </param>
/// <param name="*Y"> is the pointer to the string for example L"Workbooks".</param>
/// <param name="Z"> is the integer. </param>
void Put(OleT X, /*ushort *Y*/ LPCWSTR Y, int Z, ...)
{
//  Ole(DISPATCH_PROPERTYPUT, X, Y, Z, (LnP) &Z + 4);
//  FrV(Var);
}

/// <summary> This function is the wrapper of Ole(). </summary> 
/// <param name="X"> is the IDispatch to the file. </param>
/// <param name="*Y"> is the pointer to a string. </param>
void Ole_(OleT X, /*ushort*/ wchar_t *Y)
{
//  Ole(DISPATCH_METHOD, X, Y, 0, 0);
}
/// <summary> This function calls Get() to get the Workbooks from the Excel file. </summary>
/// <param name="X"> is the address of workbooks. </param>
/// <param name="*Y"> is the name of object to get. </param>
/// <param name="*Z"> is the full path to the Excel file. </param>
void Get_(OleT X, /*ushort *Y*/ wchar_t *Y, char *Z)
{
//  ushort S[200];
//  mbstowcs(S, Z, 200);
//  Var.vt = VT_BSTR;
//  Var.bstrVal = SysAllocString(S);
//  Get(X, Y, 1, Var);
}

/// <summary> This function sets a cell in Excel. </summary> 
/// <param name="*X"> is the Excel position for example "E35".  </param>
/// <param name="*Y"> is the string value to set in the field. </param>
void SetA(char *X, char *Y)
{
//  if( ! Sheet)
//    return ;
//  Get_(Sheet, L"Range", X);
//  Range = OleRV.pdispVal;
//  Var.vt = VT_BSTR;
//  ushort S[200];
//  mbstowcs(S, Y, 200);
//  Var.bstrVal = SysAllocString(S);
//  Put(Range, L"Value", 1, Var);
//  Var.vt = VT_I4;
//  Var.lVal = 0;
//  Put(Range, L"WrapText", 1, Var);
//  Rel(Range);
}

//  .HorizontalAlignment = xlGeneral  //  .VerticalAlignment = xlBottom   //  .WrapText = False
//  .Orientation = 0   //  .AddIndent = False  //  .ShrinkToFit = False  //  .MergeCells = False
/// <summary> This function sets the value in the Excel sheet. </summary>
/// <param name="*X"> is the Excel position for example "E35".  </param>
/// <param name="Y"> is the floating point number to set in the field. </param>
void Set(char *X, float Y)
{
//  if( ! Sheet)
//    return ;
//  Get_(Sheet, L"Range", X);
//  Range = OleRV.pdispVal;
//  Var.vt = VT_R4;
//  Var.fltVal = Y;
//  Put(Range, L"Value", 1, Var);
//  Rel(Range);
}


/// <summary> This function saves and closes the file and then release Ole. </summary> 
/// <param name="&B"> is the address of IDispatch to Excel file.</param>
void _Dwn(OleT &B)
{
  if(B)
  {
    Ole_(B, L"Save");
    Ole_(B, L"Close");
    Rel(B);
  }
}

/// <summary> This function closes and saves Community and Bank workbooks and release the Oles. </summary>  
void XL_Dwn()
{
  if(Excel)
  {
    _Dwn(ComBook);
    _Dwn(Bank_Book);
    Rel(Workbooks);
    Ole_(Excel, L"Quit");
    Rel(Excel);
    CoUninitialize();
  }
}

/// <summary> This function closes the Excel Ole object and close Notice window if it is still visiable. </summary> 
/// <param name="*m"> is the string to display in the message box after close the Excel Ole Object. </param>
void Bow(char *m)
{
  XL_Dwn();
  PW_E();
  if(m)
    MessBox(m);
  exit(1);
}

/// <summary> This function copies an Excel file from the root folder to the community folder. </summary>
/// <param name="On"> is the bool value.  </param>
/// <param name="&Book"> is the address of Ole Excel Book.  </param>
/// <param name="*P"> is Excel file name.  </param>
/// <param name="*S"> is format of the new Excel file name. </param>
void _Cop(int On, OleT &Book, char *P, char *S, ...)
{
	char Mess[400], _T[400];
	char _Com[10]; 
	FILE *fp;
	extern int Bank_On, ComOn;

  if( ! Workbooks ||  ! On)
    return ;
  char M[300];
  vsprintf(M, S, (char*) &S + 4);
  Str(_T, "%s\\%s\\%s.XLS", _getdcwd(0, Mess, 400), _Com, M);
  //  If Excel blocks the deletion of the old .XLS file.
  if((fp = fopen(_T, "r")) && (fclose(fp),  ! DeleteFile(_T)))
  {
    XL_Dwn();
    Bank_On = ComOn = 0;
    MessBox("== Warning: === No XLS files will be created...\n""Please check your task list (Ctrl-Alt-Del)\n""To see if a _Hidden_ copy of _Excel_ is running.\n""If so, Please \"End Task\" it.");
    return ;
  }
  Str(Mess, "%s.XLS", P);
  Cop(Mess, _T);
  Get_(Workbooks, L"Open", _T);
  Book = OleRV.pdispVal;
}

//Used by RunForward();
/// <summary> This function determines the cell number in the Excel worksheet, e.g. A8. </summary>
/// <param name="*C"> is the cell number in Excel. </param>
/// <param name="Col"> is the column number. </param>
/// <param name="Bank"> is the bank number. </param>
void _XC(char *C, int Col, int Bank) {
	sprintf(C,"%c%d",'A'+ ++Col,8+ Bank);
}
/// <summary> This function calls _XC to define the Excel cell position and sets the value to that cell. </summary>
/// <param name="*C"> is the cell name in Excel. </param>
/// <param name="Col"> is the column number. </param>
/// <param name="F"> is the floating-point number. </param>
/// <param name="Bank"> is the bank number. </param>
void XC(char *C, int Col, float F, int Bank) {
	_XC(C, Col, Bank); 
	Set(C,F);
}

//#define Ra( _T, _B) { float T=_T,B= _B; _XC(C, Col, Bank); Set(C,!B? 0: 100* T/ B); }

//Review: crashes.
/// <summary> This function sets the percentage of T/B to the cell located by _XC(). </summary> 
/// <param name="*C"> is the cell name in Excel. </param>
/// <param name="Col"> is the column number. </param>
/// <param name="_T"> is the floating-point value. </param>
/// <param name="_B"> is the floating-point value. </param>
/// <param name="Bank"> is the bank number. </param>
void Ra(char *C, int Col, float _T, float _B, int Bank) { 
	
	float T = _T, B = _B;
	
	_XC(C, Col, Bank); 
	
	Set(C,!B? 0: 100* T/ B);
}
/// <summary> This function sets 4 times of percentage of T/B to the cell located by _XC(). </summary> 
/// <param name="*C"> is the cell name in Excel. </param>
/// <param name="Col"> is the column number. </param>
/// <param name="_T"> is the floating-point value. </param>
/// <param name="_B"> is the floating-point value. </param>
/// <param name="Bank"> is the bank number. </param>
void RaI(char *C, int Col, float _T, float _B, int Bank) { 

	float T=_T,B= _B; 

	_XC(C, Col, Bank); 

	Set(C,!B? 0: 400* T/ B); 
}