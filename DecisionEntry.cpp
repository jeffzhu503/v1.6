//////////////////////////////////////////////////////////////////////////////////////
// The file was created in March, 2011 in order to separate the functions that draws
// the decision entry form. The DecisionEntry.cpp contains all functions only used by entry form
// It all starts with Input() by creating WinProcI Class.
// Some functions used across UI and Bank calculations such as DatIO(), Bound(), DecSec()
// Launch(), Load_Bank(), MessBox(), PrnDecs(), Round()
//////////////////////////////////////////////////////////////////////////////////////
#include "UI.h"

extern char BankName[50], TimeStamp[88]; 

extern int _In;
extern char Mess[400], _T[400], _For[10];
extern long rv, EconSet_I;
extern HCURSOR SysC, Wait_C;
extern char FromCom, Community, _Com[10], DatFile[40], FileMode[8];

extern HINSTANCE PID;
RECT r;
RECT Val_Rec;

extern HDC DC;
extern long Scr[4];
HWND BT[5];
long Editing;
RECT SubRec, CharRec;
long Key, B_Loc, _Col, _Row;

extern float Scale, CharW, CharH;

long CurDecForm, CurScr, HeaderSize, ScrCnt, CurMenuSel;
extern HWND But[300],  *_Win, Win, PW_Win;
POINTS MouseXY;

char Opts[] =
{
  'P', 'S', 'L', 'M', 'C', 'D', 'T', 'G', 'R'
};
char *Menu[] =
{
  "P   Purchase Securities", "S   Sell Securities", "L   Loans", "M   Mortgage Banking And Loan Sales", "C   Credit Administration", "D   Deposits", "T   Treasury Management", "G   General Administration", "R   Print", "E   Exit"
};

extern HBRUSH Black, Back;
FldsT Flds[128],  *FldP;

extern SecPurDecT SecPurDec[MaxB];
extern SecSaleDecT SecSaleDec[MaxB];
extern LoanDecT LoanDec[MaxB];
extern SellLnsDecT SellLnsDec[MaxB];
extern CredDecT CredDec[MaxB];
extern DeposDecT DeposDec[MaxB];
extern TreasDecT TreasDec[MaxB];
extern AdminDecT AdminDec[MaxB];

int DecSz[]= { sizeof* SecPurDec, sizeof* SecSaleDec,
	sizeof* LoanDec, sizeof* SellLnsDec, sizeof* CredDec,
	sizeof* DeposDec, sizeof* TreasDec, sizeof* AdminDec };

/// <summary> This function assigns Blush(Black) to Blush(Back), sets DC backgroup color to black, sets the text color to yellow. </summary> 
void SelColors()
{
  Back = Black;
  SetBkColor(DC, _Blac);
  FnC(_Ye)
}

/// <summary> This function writes each character at the specified location using selected font. </summary>
/// <param name="*s"> is the pointer to the string </param>
void Say(char *s)
{
  int L = strlen(s);
  Loop(L)TextOut(DC, Round(_Col++ *CharW), Round(_Row *CharH), s++, 1);
}

/// <summary> This function draws the small window for each menu. </summary> 
/// <param name="Col"> is the width to create x-coordinate of the upper-left corner of GDI Rect window. </param>
/// <param name="Row"> is the height to create y-coordinate of the upper-left corner of GDI Rect window. </param>
/// <param name="*S"> is the pointer to the string. </param>
void SayF(int Col, int Row, char *S)
{
  RECT R =
  {
    Col *CharW - 1, Row *CharH, (Col + strlen(S)) *CharW + 2, (Row + 1) *CharH
  };
  _Col = Col;
  _Row = Row;
  FillRect(DC, &R, Back);
  Say(S);
}

void _SayF(int X, int Y, char* S) 
{
	if(FldP->Float!= FldP->F2)
		SetTextColor(DC, _Re); 
	else
		SetTextColor(DC, _Ye);
	//FnC( FldP->Float!= FldP->F2?_Re: _Ye) 
	SayF(X,Y,S); 
}

extern long PopOut;
/// <summary> This function gets the messages from keys. </summary>
long GetKey()
{
  MSG Msg;
  HWND W;
  SetFocus(Win);
  PopOut = 0;
  while( ! PopOut)
  {
    GetMessage(&Msg, 0, 0, 0);
    W = Msg.hwnd;
    if(Msg.message == WM_NCLBUTTONDOWN && Msg.wParam == HTCLOSE)
      return Key = VK_ESCAPE;
    if(Msg.message == WM_LBUTTONDOWN)
    {
      static long K[] =
      {
        5, VK_ESCAPE, 6, VK_PRIOR + 256, VK_NEXT + 256
      };
      Loop(5)if(W == BT[J])
        return Key = K[J];
      MouseXY = MAKEPOINTS(Msg.lParam);
      if(MouseXY.y > B_Loc)
        continue;
      return Key = 254;
    }
    if(Msg.message == WM_KEYDOWN)
    {
      long VKey = Msg.wParam;
      if(VKey >= VK_NUMPAD0 && VKey <= VK_NUMPAD9)
        Key = VKey - VK_NUMPAD0 + '0';
      else
        Key = MapVirtualKey(VKey, 2);
      if(Key)
      {
        if(GetKeyState(VK_SHIFT) &0x8000 && Key == VK_TAB)
          Key += 256;
        if(GetKeyState(VK_CONTROL) &0x8000 && Key > 64)
          Key -= 64;
        return Key;
      }
#define or || VKey ==
      if(VKey == VK_UP or VK_DOWN or VK_PRIOR or VK_NEXT or VK_LEFT or VK_RIGHT or VK_DELETE or VK_HOME or VK_END)
        return Key = VKey + 256;
    }
    Call_WinProc
  }//EndWhile
  return 0;
}

/// <summary> This function is to draw the menu on the Decision Entry form. </summary>
void MenuDraw()
{
  Loop(MenuCnt)
  {
    if(J == CurMenuSel)
      SelColors();
    SayF(26, 4+2 * J, Menu[J]);
    if(J == CurMenuSel)
      NormColors();
  }
}

/// <summary> This function sets _Col to column number and _Row to row number. </summary>
/// <param name="Col"> is the column number. </param>
/// <param name="Row"> is the row number. </param>
void GoToColRow(int Col, int Row)
{
  _Col = Col;
  _Row = Row;
}

extern HFONT Font_I; 
extern HBRUSH Blue; 

/// <summary> This function clears the screen by releasing some DC, resetting DC, Color, Font, and refilling the DC </summary>
void ClearScr()
{
  ReleaseDC(Win, DC);
  DC = GetDC(Win);
  SetBkMode(DC, OPAQUE);
  NormColors();
  SelectObject(DC, Font_I);
  FillRect(DC, &SubRec, Blue);
}


#define Exit_I {ReleaseDC(Win,DC);DestroyWindow(Win);Win=_Win; ShowWindow(Win,SW_SHOW); SetCursor(SysC);return;} //ShowI(Win)SetCursor(SysC);return;}

/// <summary> This is the main function to launch/access Decision Entry Form. </summary>
/// <param name="Banner"> the title of Decision Entry Window.  It consists of the starting quarter, Community name and bank name. </param>
/// <returns> Nothing but decision Entry Form will be closed. </returns>
/// <remarks> If there is any inupt saved to the file, it will jump to Label L02. </remarks>
/// 2013 the program shall load the decisions from files in the order: BK0n_DI.txt, .IN and then. DAT
/// BK0n_DI.txt will be writen to .DAT 
void Input(LnP Banner)
{
  strcpy(TimeStamp, Banner);

  if( ! _In)
  {
	  int Ripe = 0; 
	  bool LoadedFromCsv = false; 

    FundsGloT F_D, F_I;

	if(!loadDecisionFromCsvToDat(0))
	//if(LoadBankDecisions(2))
	{
		//Read from BK00.IN
		_In = 1;
		DatIO("BK00", "r", 21, &F_I, sizeof F_I);
		Ripe = F_I.Dirty == 0xDDCCEEAB;
		strcpy(Mess, DatFile);
		//Read from BK00.DAT
		_In = 0;
		DatIO("BK00", "r", 21, &F_D, sizeof F_D);
	}
	else
	{
		Ripe = 0;
		//LoadedFromCsv = true; 

		Str(_T, "%s's decisions are now loaded from csv files.\n", DatFile);
		MessBox(_T);
	}

	// 12/9/2011. Bill and Ernie would like to bring this validation back to the bmsim. 
    //if(F_D._ID != F_I._ID)
    //{
    //  Ripe = 0;
    //  Str(_T, "The student's .IN decisions were not transferred\n""because  %s  and  %s  did not match:\n\n"".DAT:  %s\n"" .IN:  %s\n", Mess, DatFile, F_D.Banner, F_I.Banner);
    //  MessBox(_T);
    //}

	// 12/9/2011ToDo: We need to check the _ID in the csv file and F_D.ID
	// csv_ID = 0 when the csv file is not found
	/*int csv_ID = GetIdFromCsv(Bank); 
	if(csv_ID != 0 && F_D._ID != csv_ID) 

      Ripe = 0;
	  Str(_T, "The student's .csv decisions were not transferred\n""because  %s  and  %s  did not match:\n\n"".DAT:  %s\n"" ", Mess, DatFile, F_D.Banner);

      MessBox(_T);
    }*/

    if(Ripe) //if F_I is dirty, which means has new decision
    {
      float T = DecSec(MaxForms), B = DecSec(0);
      int Sz = (T - B) *1024;
      LnP Bu = (LnP)malloc(Sz);
      F_I.Dirty = 0;

      _In = 1;

      DatIO("BK00", "r+", 21, &F_I, sizeof F_I);

      _In = 0;
      F_D.Dirty = 1;

      DatIO("BK00", "r+", 21, &F_D, sizeof F_D);
	  Str(_T, "%s's decisions are now loaded from .IN files.\n", DatFile);

      MessBox(_T);

	  _In = 1;
	  DatIO("BK00", "r", B, Bu, Sz);
	  _In = 0;
	  DatIO("BK00", "r+", B, Bu, Sz);

      Fr(Bu);
    }

    _In = 0;
  }

  HWND _Win = Win;
  //HideI(_Win)
    ShowWindow(_Win,SW_HIDE);

  CurMenuSel = 0;
  Win = CW(0, "WinProcI", Banner, MaS, Scr[0], Scr[1], Scr[2], Scr[3], 0, 0, PID, 0);
  GetClientRect(Win, &r);
  B_Loc = r.bottom - 40 * Scale;
  rv = 26 * Scale;

  BT[0] = CW(0, "BUTTON", "^E: Field Erase", Chi, 30 *Scale, B_Loc, 130 *Scale, rv, Win, 0, PID, 0);
  BT[1] = CW(0, "BUTTON", "ESC: Abort", Chi, 170 *Scale, B_Loc, 90 *Scale, rv, Win, 0, PID, 0);
  BT[2] = CW(0, "BUTTON", "^F: Save and Exit", Chi, 270 *Scale, B_Loc, 150 *Scale, rv, Win, 0, PID, 0);
  BT[3] = CW(0, "BUTTON", "PgUp", Chi, 450 *Scale, B_Loc, 70 *Scale, rv, Win, 0, PID, 0);
  BT[4] = CW(0, "BUTTON", "PgDn", Chi, 530 *Scale, B_Loc, 70 *Scale, rv, Win, 0, PID, 0);

  GetClientRect(Win, &SubRec);
  Val_Rec = SubRec;
  SubRec.bottom -= 65 * Scale;

  L02: DatIO("BK00", "r", 0, &rv, 4);

  Key = 0;
  //#define MenuBack 
	ClearScr();
	GoToColRow(30,2);
	Say(" DECISION ENTRY "); 
	//Loop(5)
	for(int J = 0; J < 5; J++)
	{
		ShowWindow(BT[J],SW_HIDE);
	}
  while(1)
  {
    Editing = 0;
    MenuDraw();

    if(Key != 255)
      GetKey();

    if(Key == 27)
    Exit_I if(Key == VK_RETURN || Key == 255)
    {
      Key = Menu[CurMenuSel][0];
      {
        Loop(MenuCnt - 1)if(Key == Opts[J])
          break;

        if(J == MenuCnt - 1)
        Exit_I if(J == MenuCnt - 2)
        {
          {
            DirA if(DirI("%s\\?DB??.txt", _Com))
            while(1)
            {
              Str(_T, "%s\\%s", _Com, Next);
              unlink(_T);
              if(DirC)
                break;
            }
          }

          if(LoadBankDecisions(1))
            PrnDecs(1);

          Str(_T, "PRINT %s\\?DB??.TXT", _Com);
          Launch(_T);
        }
        else
        {
          CurDecForm = J;
          if( ! Edit_Form())
            MessBox(_T);
        }
        goto L02;
      }
    }

    switch(Key)
    {
      case VK_UP + 256: CurMenuSel = (CurMenuSel - 1+MenuCnt) % MenuCnt;
      break;

      case VK_DOWN + 256: CurMenuSel = (CurMenuSel + 1+MenuCnt) % MenuCnt;
      break;

      case VK_HOME + 256: CurMenuSel = 0;
      break;

      case VK_END + 256: CurMenuSel = MenuCnt - 1;
      break;

      case 254:
        Key = 255;

        CurMenuSel = Bound((Round(MouseXY.y / CharH) - 4) / 2, 0, MenuCnt - 1);
        break;

      default:
        {
          Loop(MenuCnt)if(Key == Menu[J][0])
            CurMenuSel = J;
        }
    }
  }
}



//Local variables to ReDrawSrc() and Edit_Form() only
struct
{
  FldsT *FirstField,  *LastField;
  char *FormP;
  long lines;
} Scrs[MaxScrs];

// Char Forms[MaxForms][MaxSrcs]
#pragma region Decision Forms Template
//char *Forms[MaxForms][MaxScrs] =
char *Forms[MaxForms][5] =
{
  {
    "                         PURCHASE SECURITIES\n"
    "    \n"
    "    Security                 Maturity in\n"
    "      Type       Amount        Quarters\n", 
    "     __1           __         ___2     BL: Bills\n"
    "\n"
    "     __1           __         ___2     BD: Bonds\n"
    "\n"
    "     __1           __         ___2     ST: Strips\n"
    "\n"
    "     __1           __         ___2     TX: Tax-Qualified Munis\n"
    "\n"
    "     __1           __         ___2     MB: Mortgage-Backed\n"
    "\n"
    "     __1           __         ___2     SV, SF: Swaps, Variable/Fixed\n"
    "\n"
    "     __1           __         ___2\n"
    , 
    "     __1           __         ___2\n"
    "\n"
    "     __1           __         ___2\n"
    "\n"
    "     __1           __         ___2\n"
    "\n"
    "     __1           __         ___2\n"
    "\n"
    "     __1           __         ___2\n"
    "\n"
    "     __1           __         ___2\n"
    "\n"
    "     __1           __         ___2\n"
    ,
    "     __1           __         ___2\n"
    "\n"
    "     __1           __         ___2\n"
    "\n"
    "     __1           __         ___2\n"
    "\n"
    "     __1           __         ___2\n"
    "\n"
    "     __1           __         ___2\n"
    , 
    ""
  }
  ,
  {
    "                           SELL SECURITIES\n"
    "\n"
    "                     Security No.     Sale Amount\n"
    , 
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    , 
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    ,     "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    "\n"
    "                         ___            __\n"
    , 
    ""
  }
  ,
  {
    "                                LOANS DECISIONS\n"
    , 
    "                  Interest  Rate      Orig     Bus   Max  Rate Adj  Annual\n"
    "NATIONAL            Base    Max       Fee      Dev   Mat   Period    Fee\n"
    "\n"
    "   Credit Lines     __.__   __.__    __.__14    ___   ___13             _.__%\n"
    "\n"
    "   Term Loans       __.__   __.__    __.__14    ___   ___13     __13\n"
    "\n"
    " MID MARKET\n"
    "\n"
    "   Credit Lines     __.__   __.__    __.__14    ___   ___13             _.__%\n"
    "\n"
    "   Term Loans       __.__   __.__    __.__14    ___   ___13     __13\n"
    "\n"
    " SMALL BUSINESS\n"
    "\n"
    "   Credit Lines     __.__   __.__    __.__14    ___   ___13             _.__%\n"
    "\n"
    "   Term Loans       __.__   __.__    __.__14    ___   ___13     __13\n"
    , 
    "IMPORT/EXPORT\n"
    "\n"
    "   Credit Lines     __.__   __.__    __.__14   ___   ___13             _.__%\n"
    "\n"
    " CONSTRUCTION\n"
    "\n"
    "   Credit Lines     __.__   __.__    __.__14   ___   ___13             _.__%\n"
    "\n"
    " COMMERCIAL RE\n"
    "\n"
    "   Var Term         __.__   __.__    __.__14   ___   ___13     __13\n"
    "\n"
    " MULTI-FAMILY\n"
    "\n"
    "   Var Term         __.__   __.__    __.__14   ___   ___13     __13\n"
    , 
    "  1-4 FAMILY\n"
    "\n"
    "   Fix Mortage      __.__   __.__    __.__14   ___   ___13\n"
    "\n"
    "   Var Mortage      __.__   __.__    __.__14   ___   ___13     __13\n"
    "\n"
    "   Home Equity      __.__   __.__    __.__14   ___   ___13            $___\n"
    "\n"
    " CONSUMERS\n"
    "\n"
    "   Personal         __.__   __.__    __.__14   ___   ___13     __13\n"
    "\n"
    "   Credit Card      __.__   __.__    __.__14   ___   ___13            $___\n"
    "\n"
    "   Installment      __.__   __.__    __.__14   ___   ___13\n"
    , 
    ""
  }
  ,
  {
    "                       Loan Sales\n"
    ,
    "     _10  Initiate or Close the Mortgage Banking Division  ( I or C )\n"
    "\n"
    "==== LOAN SALE DECISIONS ====\n"
    "\n"
    "      Package Numbers  ( From B275 )\n"
    "\n"
    "      ___   ___   ___   ___   ___\n"
    "\n"
    "      ___   ___   ___   ___   ___\n"
    "\n"
    "      ___   ___   ___   ___   ___\n"
    "\n"
    "      ___   ___   ___   ___   ___\n"
    , 
    ""
    , 
    ""
    , 
    ""
  }
  ,
  {
    "                          CREDIT ADMINISTRATION DECISIONS\n"
    "\n"
    "                                       Portfolio Target           Request\n"
    "                Maximum      --------  Credit Category  --------    Loan\n"
    "              Outstanding     1&2      3      4      5      6     Review?\n"
    "              -----------    -----   -----  -----  -----  -----\n"
    , 
    "  National                     ___    ___    ___    ___    ___       _8\n"
    "     Credit Lines  ___\n"
    "     Term Loans    ___\n"
    "\n"
    "  Middle Market                ___    ___    ___    ___    ___       _8\n"
    "     Credit Lines  ___\n"
    "     Term Loans    ___\n"
    "\n"
    "  Small Business               ___    ___    ___    ___    ___       _8\n"
    "     Credit Lines  ___\n"
    "     Term Loans    ___\n"
    , 
    "  Import/Export    ___         ___    ___    ___    ___    ___       _8\n"
    "\n"
    "  Construction     ___         ___    ___    ___    ___    ___       _8\n"
    "\n"
    "  Commercial RE    ___         ___    ___    ___    ___    ___       _8\n"
    "\n"
    "  Multi-Family     ___         ___    ___    ___    ___    ___       _8\n"
    "\n"
    "  1-4 Family                   ___    ___    ___    ___    ___       _8\n"
    "     Fixed         ___\n"
    "\n"
    "     Variable      ___\n"
    ,     "  Home Equity      ___         ___    ___    ___    ___    ___       _8\n"
    "\n"
    "  Personal         ___         ___    ___    ___    ___    ___       _8\n"
    "\n"
    "  Credit Card      ___         ___    ___    ___    ___    ___       _8\n"
    "\n"
    "  Installment      ___         ___    ___    ___    ___    ___       _8\n"
    ,     "************************************************************************\n"
    "\n"
    "    Provision for Loan Losses  ( in millions )    _.___\n"
    "\n"
    "    General Credit Policy    __7\n"
    "\n"
    "                             RE: Restricted\n"
    "                             CO: Controled\n"
    "                             MO: Moderate\n"
    "                             EX: Expansive\n"
  }
  ,
  {
    //  D060
    "                                DEPOSIT DECISIONS\n"
    , 
    "    Demand Deposits  *****************************************************\n"
    "                             Business   Public      Individual\n"
    "\n"
    "         Monthly Fee         __.__      __.__               __.__\n"
    "\n"
    "         Debit Fee             .__        .__     Item Fee    .__\n"
    "\n"
    "         Deposit Fee         __.__      __.__     NSF Fee   __.__\n"
    "\n"
    "         Credit on Balances  __.__      __.__               __.__\n"
    , 
    "\n"
    "   Interest-Bearing Deposits  ******************************************\n"
    "         NOW Accounts                       Money Market Accounts\n"
    "\n"
    "         Interest Rate       __.__          Interest Rate       __.__\n"
    "\n"
    "         Item Fee              .__          Base Interest       __.__\n"
    "\n"
    "         Monthly Fee         __.__          Monthly Fee         __.__\n"
    "\n"
    "         NSF Fee             __.__          Minimum Balance     ____\n"
    "                                            for higher rate\n"
    "         Minimum Balance     ____           and no fee. \n"
    "         to waive service \n"
    "         charges. \n"
    "\n"
    "    Savings Accounts  **************************************************\n"
    "         Individual     Interest Rate __.__      Monthly fee   __.__\n"
    "\n"
    "         Business       Interest Rate __.__      Monthly fee   __.__\n"
    , 
    "\n"
    "    Retail Certificates  ***********************************************\n"
    "         Maturity  Rate    F or V         Maturity  Rate    F or V\n"
    "\n"
    "            __17    __.__16    _9             __17    __.__16    _9\n"
    "\n"
    "            __17    __.__16    _9             __17    __.__16    _9\n"
    "\n"
    "            __17    __.__16    _9             __17    __.__16    _9\n"
    "\n"
    "            __17    __.__16    _9             __17    __.__16    _9\n"
    "\n"
    "            __   Maximum - Non-customer Retail Certificates\n"
    "                 ( in millions )\n"
    , 
    "\n"
    "     Business Development  ( in thousands )  **************************\n"
    "\n"
    "            Business Demand     ___           Business Savings     ___\n"
    "\n"
    "            Public Demand       ___           Money Market         ___\n"
    "\n"
    "            Individual Demand   ___           Individual Savings   ___\n"
    "\n"
    "            Now Accounts        ___           Retail Certificates  ___\n"
    "\n"
    "            Private Banking     ___\n"
  }
  ,
  {
    "                                TREASURY DECISIONS\n"
    , 
    "    Purchased Funds  ( in millions )  **********************************\n"
    "\n"
    "       Funds   Amount   Maturity\n"
    "       Type\n"
    "\n"
    "       ___5      ___       ___18      Funds Type Codes\n"
    "                                   for Funds Available\n""       ___5      ___       ___18\n"
    "                                   FRB  Federal Reserve Borrowing\n"
    "       ___5      ___       ___18      CDS  Corporate CDs\n"
    "                                   PTD  Public Time Deposit\n"
    "       ___5      ___       ___18\n"
    "                                   HFF  Housing Finance Bank-Fixed\n"
    "       ___5      ___       ___18      HFV  Housing Finance Bank-Variable\n"
    "                                     ( HFV Maturity must be 4 quarters. )\n"
    "       ___5      ___       ___18\n"
    "                                   HFR  Housing Finance Bank-Repayment\n"
    "       ___5      ___       ___18    ( Place ID-Code from B506 in Maturity field. )\n"
    , 
    "       ___5      ___       ___18\n"
    "\n"
    "       ___5      ___       ___18\n"
    "\n"
    "       ___5      ___       ___18\n"
    "\n"
    "       ___5      ___       ___18\n"
    "\n"
    "       ___5      ___       ___18\n"
    "\n"
    "       ___5      ___       ___18\n"
    "\n"
    "       ___5      ___       ___18\n"
    "\n"
    "       ___5      ___       ___18\n"
    "\n"
    "       ___5      ___       ___18\n"
    , 
    "    Long-Term Debt and Capital Stock  ************************************\n"
    "\n"
    "    Capital Stock                       Capital Issue\n"
    "\n"
    "     __.__ Dividend per share            __ Issue Plan  ( in millions )\n"
    "\n"
    "     __.__ Forecast earnings per share  ___ Issue Code  ( From B550 )\n"
    "\n"
    "                                         __ Issue Amount  ( in millions )\n"
    "\n"
    "     _____  Shares to Repurchase  ( Maximum 99,999 )\n"
    , 
    ""
  }
  ,
  {
    "                          GENERAL ADMINISTRATION DECISIONS\n"
    , 
    "   Lending                         Credit           Operations\n"
    "  Activities                    Administration    and Processing\n"
    " -----------                    --------------    ---------------\n"
    "\n"
    "     ___   National Corporation     ___             ___ Commercial\n"
    "\n"
    "     ___   Middle Market            ___\n"
    "\n"
    "     ___   Local Business           ___\n"
    "\n"
    "     ___   Trade Financing          ___\n"
    "\n"
    "     ___   Commercial Real Estate   ___             ___ Real Estate\n"
    "\n"
    "     ___   Residential/Home Equity  ___\n"
    "\n"
    "     ___   Consumer                 ___             ___ Consumer\n"
    "\n"
    "\n"
    , 
    "   Deposit\n"
    "  Activities\n"
    "\n"
    "     ___  Business Accounts                         ___ Deposit Operations\n"
    "\n"
    "     ___  Public Accounts\n"
    "\n"
    "     ___  Individuals Accounts\n"
    "\n"
    "     ___  Private Banking\n"
    "\n"
    "\n"
    "     General Administration\n"
    "\n"
    "           ___  Administrative                      ___ Central Operations\n"
    "\n"
    "           ___  Personnel/Training\n"
    "\n"
    "           __._ % General Salary Adjustment\n"
    , 
    "\n"
    "                 ************** PREMISES  ***************\n"
    "\n"
    "\n"
    "     ____20  Urban Branches                ____21  Regional Branches\n"
    "\n"
    "     ____22  Suburban/County Branches      ____23  Administrative/Operations\n"
    "\n"
    "\n"
    "                 ************** PROMOTION  ***************\n"
    "\n"
    "\n"
    "        ___  General Advertising and Promotion Budget\n"
    , 
    ""
  }
};
#pragma endregion

char *ScrBuff,  *FormP,  *pHeader;
/// <summary> This function writes each line in the Decision Edit form. </summary>
/// <remarks> p points to the form and B (q points to the same string as B) contains each line. </remarks>
void WrLn()
{
  int L = strchr(FormP, 10) - FormP;
  char B[100],  *p = FormP - 1,  *q = B - 1;
  while(1)
  {
    p++;
    q++;
    if(*p < 32)
      break;
    if( ! isdigit(*p) || p[ - 1] != '_')
    {
      *q =  *p;
      continue;
    }
    while(isdigit(*p) &&  *p >= 32)
      p++;
    p--;
    q--;
  }

  *q = 0;

  Say(B);

  FormP += L + 1;
}

extern long CurDecForm, CurScr, HeaderSize, ScrCnt, CurMenuSel;
extern HWND BT[5];

/// <summary> This function redraws the screen of Decision Entry form after the data is loaded from the file. </summary>
void ReDrawScr()
{
  ClearScr();
  FormP =  *Forms[CurDecForm];
  {
    //Loop(HeaderSize)
	for(int J = 0; J < HeaderSize; J++)
    {
      GoToColRow(1, 2+J);
      WrLn();
    }
  }
  int L = Scrs[CurScr].lines;
  FormP = Scrs[CurScr].FormP;
  {
	  //Loop(L)
	for(int J = 0; J < L; J++)
    {
      GoToColRow(1, HeaderSize + 3+J);
      WrLn();
    }
  }
  {
    FldsT *FldP = Scrs[CurScr].FirstField - 1;
    int To = Scrs[CurScr].LastField - FldP;
    //Loop(To)
	for(int J = 0; J < To; J++)
    {
      FldP++;
      if(FldP->Float == UnSet)
        continue;
      {
        char s[40];
        FldToAscii(s, FldP);
        _SayF(FldP->x, FldP->y, s);
      }
    }
  }
  {
    Loop(3)
		//ShowI(BT[J])
		ShowWindow(BT[J],SW_SHOW);
  }
  //HideI(BT[3])
  ShowWindow(BT[3],SW_HIDE);
  
  //HideI(BT[4])
  ShowWindow(BT[4],SW_HIDE);

  
  if(CurScr != 0)
    //ShowI(BT[3])
	ShowWindow(BT[3],SW_SHOW);
	if(CurScr < ScrCnt - 1)
      //ShowI(BT[4])
	  ShowWindow(BT[4],SW_SHOW);
	  SelColors();
}

///////////////////////////////////////////
// Functions for Edit_Form(), which don't 
// exist in up.h or uihelper.h header file 
///////////////////////////////////////////

/// <summary> This function deletes chars from the string. </summary>
/// <param name="*s"> is the poitner to the string. </param>
/// <param name="i"> is the beginning position.</param>
/// <param name="L"> is the length to be deleted. </param>
void DelChar(char *s, long i, long L)
{
  memmove(s + i, s + i + 1, --L - i);
  s[L] = ' ';
}

/// <summary> This function finds any missing field from one decision entry form. </summary>
/// <param name="First_F"> is the number of the first field. </param>
/// <param name="Last_F"> is the number of the last field. </param>
/// <param name="Cols"> is the total column number in one decision entry form.  </param>
/// <returns> -1 when every row has the information required.  Otherwise it returns the field number if any missing field from the entry form. </returns> 
int FindBlankInFieldBlock(int First_F, int Last_F, int Cols)
{
  FldP = Flds + First_F - 1;
  {
    int Rows = (Last_F - First_F + 1) / Cols;
    //Loop(Rows)
	for(int J = 0; J < Rows; J++)
    {
      int Row_I = J, F1_S = (++FldP)->Float != UnSet;
      //Loop(Cols - 1)
	  for(int J = 0; J < Cols - 1; J++)
      {
        int S = (++FldP)->Float != UnSet;
        if(F1_S &&  ! S)
          return First_F + Row_I * Cols + J + 1;
        if( ! F1_S && S)
          return First_F + Row_I * Cols;
      }
    }
    return  - 1;
  }
}

extern HWND Win;
/// <summary> This is the wrapper of MessageBox(). It creates a Timer after MessageBox(). </summary>
/// <param name="*Mess"> is the message to be displayed in MessageBox(). </param>
void MessBoxI(char *Mess)
{
  KillTimer(Win, 1);
  MessageBox(Win, Mess, "BMSim", MB_TOPMOST);
  SetTimer(Win, 1, 700, 0);
}

extern long rv;
/// <summary> Display the Question MessageBox with two options "Yes" or "No" </summary>
/// <param name="*Q"> is the string which question will be displayed </param>
/// <returns> The user response, which can be either Yes or No </returns>
int Ask(char *Q)
{
  KillTimer(Win, 1);
  rv = IDYES == MessageBox(Win, Q, " Question ", MB_TOPMOST|MB_YESNO);
  if( ! rv)
    SetTimer(Win, 1, 700, 0);
  return rv;
}

/// <summary> This function verifies the boundary of an input in Edit_Form(). </summary>
/// <param name="*Buf"> is the value of input, which is a string from the decision entry form.  </param>
/// <param name="lower"> is the lower boundary for an input. </param>
/// <param name="upper"> is the upper boundary for an input. </param>
float Bind(char *Buf, float lower, float upper)
{
  float result = (float)atof(Buf);
  if(! isdigit(*Buf) && *Buf != '.' && *Buf != '-')
  {
    MessBoxI(" The entry must be a number");
    return UnSet;
  }
  if(result < lower || result > upper)
  {
    char s[80];
    Str(s, " This entry must be between %0.2f and %0.2f, inclusive.", lower, upper);
    MessBoxI(s);
    return UnSet;
  }
  return result;
}

char *str_line[] =
{
	"BL", "ST", "BD", "TX", "MB", "SV", "SF", "", "", "", "FRB", "CDS", "HFF", "HFV", 
	"PTD", "HFR", "", "", "", "", "RE", "CO", "MO", "EX", "", "", "", "", "", "", "X",
	"N", "I", "C", "F", "V", "", "", "", "", "NCC", "NCT", "MMC", "MMT", "SBC", "SBT", 
	"TFC", "CON", "CRE", "MFR", "RMF", "RMV", "CHE", "CPL", "CCC", "CIL"
};

/// <summary> This function converts a floating point number to a string. </summary>
/// <param name="f"> is the floating point number. </param>
/// <returns> a string.  </returns> 
char *ftostr(float f)
{
  if(f >= 0 && f < MAXSTR)
    return str_line[(int)f];
  return 0;
}

/// <summary> This function converts the structure of Fld into string. </summary> 
/// <param name="*Buf"> is the buffer contains the string. </param>
/// <param name="FldP"> is the pointer to FLd - field structure. </param>
/// <returns> None </returns>
void FldToAscii(char *Buf, FldsT *FldP)
{
  switch(FldP->FldType)
  {
    case 0:
      Str(Buf, "%0*d", FldP->Len, (long)FldP->Float);
      return ;
    case 1:
      if(FldP->Float >= BL && FldP->Float <= SF)
        strcpy(Buf, ftostr(FldP->Float));
      else
        FldP->Float = UnSet;
      return ;
    case 5:
      if(FldP->Float >= FRB && FldP->Float <= HFR)
        strcpy(Buf, ftostr(FldP->Float));
      else
        FldP->Float = UnSet;
      return ;
    case 7:
      if(FldP->Float >= RE && FldP->Float <= EX)
        strcpy(Buf, ftostr(FldP->Float));
      else
        FldP->Float = UnSet;
      return ;
    case 8:
      if(FldP->Float == Y || FldP->Float == N)
        strcpy(Buf, ftostr(FldP->Float));
      else
        FldP->Float = UnSet;
      return ;
    case 9:
      if(FldP->Float == F || FldP->Float == V)
        strcpy(Buf, ftostr(FldP->Float));
      else
        FldP->Float = UnSet;
      return ;
    case 10:
      if(FldP->Float == I || FldP->Float == C)
        strcpy(Buf, ftostr(FldP->Float));
      else
        FldP->Float = UnSet;
      return ;
    case 11:
      if(FldP->Float >= NCC && FldP->Float <= CIL)
        strcpy(Buf, ftostr(FldP->Float));
      else
        FldP->Float = UnSet;
      return ;
#define ModBuf(V,Buf) { long Dot = FldP->Dot; if ( Dot > FldP->Len || Dot < 1 ) Dot = FldP->Len; \
Str( Buf, "%*.*f", FldP->Len, FldP->Len - Dot, V ); } { int Len = FldP->Len, Off = strlen(Buf) - Len; \
if ( Off > 0 ) { memmove( Buf, Buf + Off, Len ); Buf[Len] = 0; } }
    default:
      ModBuf(FldP->Float, Buf)
  }
}

/// <summary> This function closes the decision form. </summary>
/// <param name="discard"> is the flat.  If 0, the decision entries are saved the data file. If -1, discard </param>
int CloseDecFP(long discard)
{
  Fr(ScrBuff);
  ScrBuff = 0;
  if( ! discard)
  {
    FILE *fp = fopen(DatFile, "r+b");
    if( ! fp)
    {
      Str(_T, "Can't W-Open: (%s)", DatFile);
      return 0;
    }
    fseek(fp, (long)(DecSec(rv = CurDecForm) *1024), 0);
    {
      int Sz = DecSz[rv];
      FeP B = (FeP)malloc(Sz);
      {
        Loop(Sz / 4)
			B[J] = Flds[J].Float;
      }
      fwrite(B, 1, Sz, fp);
      Fr(B);
      {
        ulong Dirty = 0xDDCCEEAB;
        FundsGloT *F = 0;
        fseek(fp, 21 *1024+(long) &F->Dirty, 0);
        fwrite(&Dirty, 1, 4, fp);
        fclose(fp);
      }
    }
  }
  return 1;
}

char FieldBuf[40];
long CurInFld;
extern long Key, B_Loc, _Col, _Row;

extern char Mess[400], _T[400], _For[10];
extern char FromCom, Community, _Com[10], DatFile[40], FileMode[8];

extern float Scale, CharW, CharH;
extern RECT SubRec, CharRec; 
extern HDC DC;

extern POINTS MouseXY;

/// <summary> This function draws the window when user click the decision entry form for entering the data. </summary>
/// <returns> 0 if there is any error. </returns>
/// <remarks> _T will contain the error message. </remarks> 
int Edit_Form()
{
  int FldExitReq = 0;

  Editing = 1;
  SetTimer(Win, 1, 700, 0);
  memset(Flds, 0, sizeof(Flds));
  {
    char *p = *Forms[CurDecForm];
    Loop(9)
	//for(int J = 0; J < 9; J++)
    {
      p = strchr(p, 10);
      if(! p++)
        break;
    }

    HeaderSize = J;
  }
  {
      //Loop(MaxScrs)
	  int J; 
	  for(J = 0; J < MaxScrs; J++)
	  {
		if(! *Forms[CurDecForm][J])
			break;
	  }
    ScrCnt = J - 1;
  }
  {
    long *L, ScrI = 0, FldI = -1;
    //Loop(ScrCnt)
	for(int J = 0; J < ScrCnt; J++)
    {
      L = &Scrs[J].lines;
      {
        char *p = Scrs[J].FormP = Forms[CurDecForm][J + 1];
        Loop(30)
        {
          p = strchr(p, 10);
          if(! p++)
            break;
        }
        *L = J;
      }
      {
        char *CurL = Scrs[J].FormP;
        Loop(*L)
        {
          if(J)
            CurL = strchr(CurL, 10) + 1;
          if(J + HeaderSize > 23)
          {
            Str(_T, "Too many Lines");
            return 0;
          }
          {
            int Line = J, Fudge = 0;
            Loop(100)
            {
              long j = J, len = 0;
              char c = CurL[J];
              if(c == 10)
                break;
              if(c != '_')
                continue;
              if(++FldI > MaxFldsPerForm)
              {
                Str(_T, "Too many Fields");
                return 0;
              }
              Flds[FldI].y = Line + 3+HeaderSize;
              Flds[FldI].x = J + 1-Fudge;
              while(CurL[J] == '_')
                J++;
              if(CurL[J] == '.')
              {
                J++;
                Flds[FldI].Dot = J-j;
              }
              while(CurL[J] == '_')
                J++;
              if(! isdigit(CurL[J]))
              {
                if(CurL[j - 1] == '.')
                  Flds[FldI].FldType = 0;
                else
                  Flds[FldI].FldType = 3;
              }
              else
              {
                len = J;
                Flds[FldI].FldType = atoi(CurL + J);
                while(isdigit(CurL[++len]))
                  ;
                len -= J;
                Fudge += len;
                J += len;
              }

              Flds[FldI].Len = J-- -j -len;
            }
          }
        }
      }
      Scrs[J].LastField = Flds + FldI;

      if(J)
        Scrs[J].FirstField = Scrs[J - 1].LastField + 1;
      else
        Scrs[J].FirstField = Flds;
      {
        FldsT *B = Scrs[J].FirstField,  *E = Scrs[J].LastField,  *P = B-1,  *Q;
        while(++P <= E)
        {
          Q = P->UpLnk = P->DwnLnk = P;
          while(--Q >= B)
          if(Q->y < P->y)
          {
            int Y = Q->y, X = P->x, Nearest = abs(Q->x - X);
            P->UpLnk = Q;

            while(--Q >= B && Q->y == Y)
            if(abs(Q->x - X) < Nearest)
            {
              Nearest = abs(Q->x - X);
              P->UpLnk = Q;
            }
            break;
          }
          Q = P;

          while(++Q <= E)
          if(Q->y > P->y)
          {
            long Y = Q->y, X = P->x, Nearest = abs(Q->x - X);
            P->DwnLnk = Q;

            while(++Q <= E && Q->y == Y)
            if(abs(Q->x - X) <= Nearest)
            {
              Nearest = abs(Q->x - X);
              P->DwnLnk = Q;
            }
            break;
          }
        }
      }
    }
  }
  {
    FILE *fp = fopen(DatFile, "rb");
    if(! fp)
    {
      char f[260];
      GetCurrentDirectory(260, f);
      Str(_T, "Can't R-Open: %s\\%s", f, DatFile);
      return 0;
    }

    fseek(fp, (int)(DecSec(rv = CurDecForm) *1024), 0);
    {
      int Sz = DecSz[rv];
      FeP B = (FeP)malloc(Sz), B2 = (FeP)malloc(Sz);
      fread(B, 1, Sz, fp);
      fseek(fp, (int)(DecSec2(rv) *1024), 0);
      fread(B2, 1, Sz, fp);
      fclose(fp);
      {
        Loop(Sz / 4)Flds[J].Float = B[J], Flds[J].F2 = B2[J];
      }
      Fr(B);
      Fr(B2);
    }
  }

  FldP = Flds;
  CurScr = 0;
  ReDrawScr();

  while(1)
  {
    int TooBig;
    static FldsT *MouseFld;
    if(FldExitReq == 254)
      FldP = MouseFld;
    {
      int Len = FldP->Len, Dot = FldP->Dot, FldType = FldP->FldType, FldX = FldP->x, FldY = FldP->y;
      memset(FieldBuf, 32, Len);
      FieldBuf[Len] = 0;
      SelColors();

      if(FldP->Float != UnSet)
        FldToAscii(FieldBuf, FldP);

      while(1)
       /* Re-Validate Loop */
      {
        if(FldExitReq != 254)
          CurInFld = 0;
        FldExitReq = 0;
        while(1)
         /* Re-Field Loop */
        {
          FieldBuf[Len] = 0;
          if(FldExitReq != 254 && CurInFld >= Len)
            FldExitReq = VK_TAB;
          if(FldExitReq)
          {
            char *p = strchr(FieldBuf, ',');
            if(p)
               *p = '.';
            break;
          }

          _SayF(FldX, FldY, FieldBuf);
          {
            float X = (FldX + CurInFld) *CharW, Y = FldY * CharH;
            CharRec.left = Round(X);
            CharRec.top = Round(Y);
            CharRec.right = Round(X + CharW);
            CharRec.bottom = Round(Y + CharH - 1);
            InvertRect(DC, &CharRec);
          }

          switch(GetKey())
          {
            case VK_TAB:
            case VK_RETURN:
            case VK_TAB + 256: case VK_UP + 256: case VK_DOWN + 256: case VK_PRIOR + 256: case VK_NEXT + 256: case VK_ESCAPE:
              case 'A' - 64: case 'F' - 64: FldExitReq = Key;
              break;
            case 254:
              {
                long X = MouseXY.x / CharW, Y = MouseXY.y / CharH;
                FldsT *f = Scrs[CurScr].FirstField - 1;
                long BestDX, BestDY = 1000;

                while(1)
                {
                  f++;
                  {
                    long DX = abs(X - f->x - f->Len / 2), DY = abs(Y - f->y);

                    if(DY < BestDY)
                    {
                      BestDY = DY;
                      BestDX = 1000;
                    }

                    if(DY == BestDY && DX < BestDX)
                    {
                      BestDX = DX;
                      MouseFld = f;
                    }
                  }

                  if(f == Scrs[CurScr].LastField)
                    break;
                }

                CurInFld = 0;
                if(Y >= MouseFld->y && Y <= MouseFld->y + 1 && X >= MouseFld->x && X <= MouseFld->x + MouseFld->Len)
                  CurInFld = min(X - MouseFld->x, MouseFld->Len - 1);
              }
              if(MouseFld != FldP)
                FldExitReq = Key;
              break;

              case 'E' -64: memset(FieldBuf, 32, Len);
              CurInFld = 0;
              break;

              case 'V' -64: ReDrawScr();
              break;

              case VK_LEFT + 256: if(CurInFld)
                CurInFld--;
              break;

              case VK_RIGHT + 256: if(CurInFld < Len -1)
                CurInFld++;
              break;

            case VK_BACK:
              if(! CurInFld)
                break;
              CurInFld--;
              DelChar(FieldBuf, CurInFld, Len);
              break;

              case VK_DELETE + 256: DelChar(FieldBuf, CurInFld, Len);
              break;

            default:
              if(Key ==' ' && !CurInFld)
              {
                FldExitReq = VK_TAB;
                break;
              }
              if(! isprint(Key))
                break;
              FieldBuf[CurInFld] = (char)Key;
              CurInFld++;
              if(Key ==' ')
              {
                memset(FieldBuf + CurInFld, 32, Len - CurInFld);
                FldExitReq = VK_TAB;
              }
          }
        } /* Re-Field Loop */

        {
          long Blank = 1;
          double Old_F;
          char Old[40], _T[100];
          TooBig = 0;
          memmove(Old, FieldBuf, 40);

          #define ShiftLeft { Loop(Len) if ( FieldBuf[J] > 32) { Blank = 0; break; } \
            if ( !Blank && J ) { memmove( FieldBuf, FieldBuf + J, Len - J );  memset( FieldBuf + Len - J, 32, J );  } }

          ShiftLeft
			if(isdigit(FieldBuf[0]))
          {
            ModBuf(Old_F = atof(FieldBuf), FieldBuf)ShiftLeft if(TooBig = Old_F != atof(FieldBuf))
            {
              _SayF(FldX, FldY, Old);
              Str(_T, "\"%s\"  Is  Too  Big.", Old);
              MessBoxI(_T);
              FldP->Float = UnSet;
              memmove(FieldBuf, Old, 40);
            }
          }
          if(Blank)
          {
            FldP->Float = UnSet;
            memset(FieldBuf, '_', Len);
            if(Dot >= 1 && Dot < Len)
              FieldBuf[Dot - 1] = '.';
            NormColors();
            _SayF(FldX, FldY, FieldBuf);
            break;
          }
        }

        if(! TooBig)
        {
          long Enum = -1;
          FldP->Float = UnSet;
          {
            Loop(MAXSTR)if(! stricmp(FieldBuf, str_line[J]))
            {
              Enum = J;
              break;
            }
          }

          switch(FldType)
          {

            case 1:
              if(Enum >= BL && Enum <= SF)
              {
                FldP->Float = (float)Enum;
                break;
              }
              MessBoxI(" This entry must be BL, ST, BD, TX, MB, SV, or SF");
              continue;

            case 5:
              if(Enum >= FRB && Enum <= HFR)
              {
                FldP->Float = (float)Enum;
                if(Enum == HFV)
                {
                  FldP[2].Float = (float)4;
                }
                break;
              }
              MessBoxI(" This entry must be: FRB, CDS, HFF, HFV, PTD or HFR");
              continue;

            case 7:
              if(Enum >= RE && Enum <= EX)
              {
                FldP->Float = (float)Enum;
                break;
              }
              MessBoxI(" This entry must be: RE, CO, MO, or EX");
              continue;

            case 8:
              if(Enum == Y)
              {
                FldP->Float = (float)Enum;
                break;
              }
              MessBoxI(" This entry must be: 'X' or Blank");
              continue;

            case 9:
              if(Enum >= F && Enum <= V)
              {
                FldP->Float = (float)Enum;
                break;
              }
              MessBoxI(" This entry must be: 'F' or 'V'");
              continue;

            case 10:
              if(Enum >= I && Enum <= C)
              {
                FldP->Float = (float)Enum;
                break;
              }
              MessBoxI(" This entry must be: 'I' or 'C'");
              continue;

            case 11:
              if(Enum >= NCC && Enum <= CIL)
              {
                FldP->Float = (float)Enum;
                break;
              }
              MessBoxI(" Use: ""NCC NCT MMC MMT SBC SBT TFC CON CRE MFR RMF RMV CHE CPL CCC CIL");
              continue;

            case 2:
              {
                float R = (float)atof(FieldBuf);
                Enum = (long)FldP[-2].Float;
                if(R <= 0)
                {
                  MessBoxI(" The entry must be a number");
                  continue;
                }
                if(R > 120)
                {
                  MessBoxI(" Maturity must be 120 quarters or less");
                  continue;
                }

                if(Enum == BL && R > 4)
                {
                  MessBoxI(" Bill maturities must be 4 quarters or less");
                  continue;
                }
                if((Enum == SV || Enum == SF) && (R < 2 || R > 80))
                {
                  MessBoxI(" Swaps maturities must be ""between 2 and 80 quarters");
                  continue;
                }
                FldP->Float = R;
                break;
              }
            case 0:
              FldP->Float = Bind(FieldBuf, 0.f, 99.f);
              break;

            case 3:
              FldP->Float = Bind(FieldBuf, -999999.f, 999999.f);
              break; // Default Field Type!

            case 4:
              FldP->Float = Bind(FieldBuf, 0.f, 99.99f);
              break;

            case 6:
              FldP->Float = Bind(FieldBuf, 0.f, 25.f);
              break;

            case 12:
              FldP->Float = Bind(FieldBuf, 100.f, 150.f);
              break;

            case 13:
              FldP->Float = Bind(FieldBuf, 1.f, 140.f);
              break;

            case 14:
              FldP->Float = Bind(FieldBuf, 0.f, 2.f);
              break;

            case 15:
              FldP->Float = Bind(FieldBuf, 3.f, 30.f);
              break;

            case 16:
              FldP->Float = Bind(FieldBuf, 0.f, 19.f);
              break;

            case 17:
              FldP->Float = Bind(FieldBuf, 0.f, 40.f);
              break;

            case 18:
              {
                float R = (float)atof(FieldBuf);
                Enum = (long)FldP[-2].Float;
                if(Enum == HFR && (R < 501 || R > 540))
                {
                  MessBoxI(" HFR ID-Code must be between 501 and 540.");
                  continue;
                }

                if(Enum == HFF && (R < 1 || R > 80))
                {
                  MessBoxI(" HFF maturities must be between 1 and 80 quarters.");
                  continue;
                }

                if(Enum == HFV && R != 4)
                {
                  MessBoxI(" The HFV maturity must be Exactly 4 quarters.");
                  continue;
                }
                FldP->Float = R;
                break;
              }
			case 20:
				{
					float R = (float)atof (FieldBuf); 
					if ( R < 0 )
					{ 
						float X; 
						FILE *fp = fopen( DatFile, "rb" ); 
						fseek( fp, 30 * 1024L + 0 * 4, 0 ); 
						fread( &X, 1, 4, fp ); 
						fclose(fp); 
						if ( ( X *= 1000 ) + R < 5000 ) 
						{
							char m[80]; 
							Str( m, "%4.0f is the Largest Sale allowed. (For this Bank's Urban Branches .)",Bound( 5000 - X, -999., 0. ) );
							MessBoxI(m); 
							continue; 
						} 
					} 
					FldP->Float = R;
					break; 
				}

			case 21: 
				{
					float R = (float)atof (FieldBuf); 
					if ( R < 0 )
					{ 
						float X; 
						FILE *fp = fopen( DatFile, "rb" ); 
						fseek( fp, 30 * 1024L + 1 * 4, 0 ); 
						fread( &X, 1, 4, fp ); 
						fclose(fp); 
						if ( ( X *= 1000 ) + R < 2500 ) 
						{
							char m[80]; 
							Str( m, "%4.0f is the Largest Sale allowed. (For this Bank's Regional Branches.)", Bound( 2500 - X, -999., 0. ) );
							MessBoxI(m); 
							continue; 
						} 
					} 
					FldP->Float = R;
					break; 
				}
			case 22: 
				{
					float R = (float)atof (FieldBuf); 
					if ( R < 0 )
					{ 
						float X; 
						FILE *fp = fopen( DatFile, "rb" ); 
						fseek( fp, 30 * 1024L + 2 * 4, 0 ); 
						fread( &X, 1, 4, fp ); 
						fclose(fp); 
						if ( ( X *= 1000 ) + R < 5000 ) 
						{
							char m[80]; 
							Str( m, "%4.0f is the Largest Sale allowed. (For this Bank's Suburban Branches.)", Bound( 5000 - X, -999., 0. ) );
							MessBoxI(m); 
							continue; 
						} 
					} 
					FldP->Float = R;
					break; 
				}
			case 23: 
				{
					float R = (float)atof (FieldBuf); 
					if ( R < 0 )
					{ 
						float X; 
						FILE *fp = fopen( DatFile, "rb" ); 
						fseek( fp, 30 * 1024L + 3 * 4, 0 ); 
						fread( &X, 1, 4, fp ); 
						fclose(fp); 
						if ( ( X *= 1000 ) + R < 2500 ) 
						{
							char m[80]; 
							Str( m, "%4.0f is the Largest Sale allowed. (For this Bank's Admin / Ops.)", Bound( 2500 - X, -999., 0. ) );
							MessBoxI(m); 
							continue; 
						} 
					} 
					FldP->Float = R;
					break; 
				}
				

              //           #define PrmLim( Prm, Max, Msg ) case 20 + Prm: { float R = (float)atof ( FieldBuf ); \
              //              if ( R < 0 ) { float X; FILE *fp = fopen( DatFile, "rb" ); \
              //                fseek( fp, 30 * 1024L + Prm * 4, 0 );  fread( &X, 1, 4, fp );  fclose(fp); \
              //                if ( ( X *= 1000 ) + R < Max ) { char m[80]; \
              //                  Str( m, "%4.0f is the Largest Sale allowed. (For this Bank's " Msg ".)", \
              //                    Bound( Max - X, -999., 0. ) ); MessBoxI(m);  continue; } } FldP->Float = R; break; }
			  //PrmLim(0, 5000, "Urban Branches")
			  //PrmLim(1, 2500, "Regional Branches")
			  //PrmLim(2, 5000, "Suburban Branches")
			  //PrmLim(3, 2500, "Admin / Ops")
          }

          if(FldP->Float != UnSet)
          {
            NormColors();
            FldToAscii(FieldBuf, FldP);
            _SayF(FldX, FldY, FieldBuf);
            break;
          }
        }
      }
    }
    switch(FldExitReq)
    {
      case VK_RETURN:

      case VK_TAB:
        if(FldP < Scrs[CurScr].LastField)
          FldP++;
        continue;

        case VK_TAB + 256: if(FldP > Scrs[CurScr].FirstField)
          FldP--;
        continue;

        case VK_UP + 256: if(FldP->UpLnk <= Scrs[CurScr].LastField && FldP->UpLnk >= Scrs[CurScr].FirstField)
          FldP = FldP->UpLnk;
        continue;

        case VK_DOWN + 256: if(FldP->DwnLnk <= Scrs[CurScr].LastField && FldP->DwnLnk >= Scrs[CurScr].FirstField)
          FldP = FldP->DwnLnk;
        continue;

        case VK_PRIOR + 256: if(CurScr > 0)
        {
          CurScr--;
          FldP = Scrs[CurScr].FirstField;
          ReDrawScr();
        }
        continue;

        case VK_NEXT + 256: if(CurScr < ScrCnt - 1)
        {
          CurScr++;
          FldP = Scrs[CurScr].FirstField;
          ReDrawScr();
        }
        continue;

      case VK_ESCAPE:

        case 'A'-64:

        case 'F'-64:
        {
          if(FldExitReq != 'F'-64)
          {
            if(Ask("Exit Without Saving (Y/N)? "))
              return CloseDecFP(1);
          }

          if(Ask("Write decision to file and EXIT (Y/N)? "))
          {
            long Fld = -1;
            switch(CurDecForm)
            {
            case secpur:
              Fld = FindBlankInFieldBlock(0, 56, 3);
              break;

            case secsal:
              Fld = FindBlankInFieldBlock(0, 37, 2);
              break;

            case treas:
              Fld = FindBlankInFieldBlock(51, 52, 2);
              if(Fld == -1)
              {
                int at = 0;
                while(1)
                {
                  Fld = FindBlankInFieldBlock(at, 47, 3);
                  if(Fld == -1)
                    break;

                  if((Fld % 3) == 2 && Flds[Fld - 2].Float == FRB)
                  {
                    at = Fld + 1;
                    continue;
                  }
                  break;
                }
              }
            }

            if(Fld == -1)
              return CloseDecFP(0);

            MessBoxI("A Required Entry Is Missing.\nClick Ok to Go There.");
            FldP = Flds + Fld;
            {
              Loop(ScrCnt)if(FldP <= Scrs[J].LastField && FldP >= Scrs[J].FirstField)
                CurScr = J;
            }

            ReDrawScr();
          }
        }
    }
  }
  return 1;
}

long InPaint;

/// <summary> This is the Window Procedure for a Decision Entry Forms. <summary> 
long __stdcall WinProcI(HWND W, uint M, uint C, long C2)
{

  if(M == WM_CLOSE || M == WM_KEYDOWN && C == 27 || (M == WM_KEYDOWN && C == VK_F4 && GetAsyncKeyState(VK_CONTROL)))
  {
    PopOut = 2;
    Key = 27;
  }
  if( ! InPaint && M == WM_PAINT && W == Win)
  {
    InPaint = 1;
    if( ! Editing)
    {
      //#define MenuBack 
		ClearScr();
		GoToColRow(30,2);
		Say(" DECISION ENTRY "); 
		//Loop(5)
		for(int J = 0; J < 5; J++)
		{
			ShowWindow(BT[J],SW_HIDE);
		}
		MenuDraw();
      ValidateRect(W, &Val_Rec);
    }
    else
    {
      int X = FldP->x, Y = FldP->y;
      ValidateRect(W, &SubRec);
      ReDrawScr();
      _SayF(X, Y, FieldBuf);
      {
        float x = (X + CurInFld) *CharW, y = Y * CharH;
        CharRec.left = Round(x);
        CharRec.top = Round(y);
        CharRec.right = Round(x + CharW);
        CharRec.bottom = Round(y + CharH - 1);
        InvertRect(DC, &CharRec);
        {
          Loop(3)
		  //ShowI(BT[J])
		  ShowWindow(BT[J],SW_SHOW);
        }
      }
    }
    InPaint = 0;
  }
  if(M == WM_TIMER && Editing && FldP && W == Win)
  {
    int X = FldP->x, Y = FldP->y;
    float x, y;
    CharRec.left = Round(x = (X + CurInFld) *CharW);
    CharRec.top = Round(y = Y * CharH);
    CharRec.right = Round(x + CharW);
    CharRec.bottom = Round(y + CharH - 1);
    InvertRect(DC, &CharRec);
    SetTimer(W, 1, 700, 0);
  }
  if(M == WM_PARENTNOTIFY && (WORD)C == WM_CREATE)
  {
    SendMessage((HWND)C2, WM_SETFONT, (DWORD)Font_I, 1);
  }
  return DefWindowProc(W, M, C, C2);
}
