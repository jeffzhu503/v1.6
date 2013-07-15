//#include <WinUser.h>
#include <WTypes.h> 
#include <WinDef.h>
#include <Ole2.h>
#include "UIHelper.h"
#include "UIVariables.h"
#include "GlobalDef.h"
#include "SimpleCalculation.h"
#include "FileIOHelper.h"
#include "BMSimHelper.h"
#include "ReportingDataT.h"

extern int Bank, Bank_On, ComOn, UI_Up, Drive, NumBanks, NumComs, SimQtr, SimYr, YrsQtr, ComI, rvs, len;

// Here are the fonts used:
extern HFONT Font, Font2, Font3;
extern HDC DC;
extern HBRUSH SysB, Green, Blue;
extern MSG Msg;
extern HINSTANCE PID;
extern FILE *fp;
extern RECT r;
extern long Scr[4];
#define W_H  ER( Scr[ 0 ], ( Scr[ 2 ] - W) / 2 ), ER( Scr[ 1 ], ( Scr[ 3 ] - H) / 2 ), W, H

// These are the various windows:
extern HWND But[300],  *_Win, Win, PW_Win;
extern long FromInstall, PW_Cnt, Cur_B, Tot_B, PopOut;

//extern HWND BT[5];
//extern RECT SubRec, CharRec;
extern HBRUSH Black, Back;

extern ListT InstrList, BankList, FormList;

//extern __int64 _Time, Secnd;
//extern int _In;
extern char Mess[400], _T[400], _For[10];
extern long rv, EconSet_I, CapSz;
extern HCURSOR SysC, Wait_C;
//extern HICON ICon;

/// <summary> This function destroys the current dialog and puts back the parent dialog window, or exits BMSim if there is no parent window.</summary>
void Back_Win()
{
  if( ! _Win)
    return ;
  DestroyWindow(Win);
  Win = _Win[1];
  Tot_B = (long)_Win[2];
  Cur_B = (long)_Win[3];
  memmove(But, _Win + 4, Tot_B *4);
  Flip(1);
  {
    HWND *p = _Win;
    _Win = (HWND*) * _Win;
    Fr(p);
  }
  Show(Win);
}

/// <summary> This function shows a new dialog window. </summary>
/// <param name="W"> is a window handle. </param>
void Show(HWND W)
{
  ShowWindow(W, SW_HIDE);
  ShowWindow(W, SW_SHOW);
}


/// <summary> This function ensures that window get the keyboard focus and stay on top. </summary>
void Get_Msgs(void)
{
  char b[4];
  PopOut = 0;
  SetForegroundWindow(Win);
  FLUSH_Msgs();
  if(Tot_B && (GetClassName(But[Cur_B], b, 4),  *b == 'E'))
    SetFocus(But[Cur_B]);
  while( ! PopOut && GetMessage(&Msg, 0, 0, 0))
    MASK_Msg();
}

long Keyed; // True is a key was pressed, used in Button_Event
const int Eg = 4; // the standard spacing between buttons

extern int Do_Flip;
extern HFONT Font_I; // Large monospaced font for input routines and popup messages
extern float Scale, CharW, CharH; // The size of a character for input and error messages

extern HWND But[300],  *_Win, Win, PW_Win;
extern long FromInstall, PW_Cnt, Cur_B, Tot_B, PopOut;


/// <summary> A window's main event handler, including child controls. In the end, it calls WinProc(). </summary>
void MASK_Msg(void)
{
  DWORD M = Msg.message, C = (WORD)Msg.wParam;
  HWND W = Msg.hwnd;
  char b[4];
  if(Do_Flip == 1)
    Do_Flip = 2, Flip(1), Do_Flip = 0;
  if(Tot_B && M == WM_NCLBUTTONDOWN && Msg.wParam == HTCLOSE)
  {
    PopOut = (long)GetMenu(*But);
    return ;
  }
  if(M == WM_KEYDOWN && Tot_B && (C == 13 || C == 9 || C == VK_UP || C == VK_DOWN) && (GetClassName(W, b, 4),  *b == 'E'))
  {
    Button_Event(0, C, W);
    return ;
  }
  Call_WinProc
}

/// <summary> This function handles or discards old messages. </summary>
void FLUSH_Msgs()
{
  while(PeekMessage(&Msg, 0, 0, 0, 0) && GetMessage(&Msg, 0, 0, 0))
    MASK_Msg();

}

/// <summary> This function highlights or deselects a button or check box. </summary>
/// <param name="On"> is a flag. The window will hightlight a button if On is equal to 1 and deselect a button if On is 0. </param>
void Flip(long On)
{
  char b[4];
  HWND B = But[Cur_B];
  if( ! Tot_B || (GetClassName(B, b, 4),  *b == 'E'))
    return ;
  FLUSH_Msgs();
  RedrawWindow(B, 0, 0, RDW_ERASE|RDW_ERASENOW|RDW_INVALIDATE | RDW_INTERNALPAINT|RDW_UPDATENOW);
  if(On)
  {
    GetClientRect(B, &r);
    DC = GetDC(B);
    FrameRect(DC, &r, Blue);
    r.top++;
    r.bottom--;
    r.left++;
    r.right--;
    FrameRect(DC, &r, Green);
    ReleaseDC(B, DC);
  }
}

/// <summary>This function processes event triggered from a dialog window and list of buttons or other controls. </summary>
/// <param name="Btn_Event">is a button event </param>
/// <param name="Key"> is the key that was pressed. </param>
/// <param name="New_Button"> is the window handle of a new button, control or  a dialog window.  (If a window, it becomes the top left button.) </param>
/// <remarks> PopOut, a global variable, tells an event loop what to do. Button_Event() may set PopOut.  
/// <list type="bullet"> 
/// Btn_Event == 0 means a key to exit an edit control.
/// Btn_Event == 1 means a key on a button,
/// Btn_Event == 3 means a new dialog window has popped up, so give the focus to it's top left button.
/// Btn_Event == 4 means our dialog window has gone away, so reset accordingly.
/// Btn_Event == 5 means a button was clicked.
/// </list>
/// </remarks>
void Button_Event(long Btn_Event, long Key, HWND New_Button)
{
  // Old_Button is the button or control that
  //   will be losing it's focus.
  HWND Old_Button = But[Cur_B];
  // Control_Type holds the name of
  // the button/control's class
  // ( i.e. it's type, e.g. "EDIT" is an edit control ).
  char Control_Type[6];
  // If there is no list of buttons then return.
  if( ! Tot_B)
    return ;
  // Make  Keyed, a global, true if a key was pressed.
  Keyed = Btn_Event == 1;
  // The escape key or the ok button button close this dialog.
  if(Key == 27)
  {
    // The user has hit the escape key,
    // which is the same thing as
    // hitting the top left button,
    // i.e. it always exit's the current dialog.
    PopOut = (long)GetMenu(*But);
    return ;
  }
  if(Key == 13 && (Btn_Event || New_Button == Old_Button))
  {
    // This might toggle a check box on and off.
    PopOut = (long)GetMenu(But[Cur_B]);
    return ;
  }
  if(Btn_Event == 3)
  {
    GetWindowText(New_Button, Control_Type, 4);
    // If the new dialog window has no title,
    if( !  *Control_Type)
    {
      // Get the dialog's dimensions so we can
      //   locate it's top left button.
      RECT c, r;
      GetWindowRect(New_Button, &c);

      // Note, Loop() is a macro,
      //   it loops a given number of times:
      //     #define Loop( L ) int J = -1, LLL = L;
      //        while ( ++J < LLL )
      {
        for(int intIndexOfCount = 0; intIndexOfCount < Tot_B; intIndexOfCount++)
        {
          // Loops though our set of buttons
          GetWindowRect(But[intIndexOfCount], &r);

          // If it's the top left button.
          if(r.top - Eg == c.top && r.left - Eg == c.left)
          {
            // New_Button is now actually a button.
            New_Button = But[intIndexOfCount];
            break;
          }
        }
      }
    }
  }

  // Don't do this if our dialog window is being killed.
  if(Btn_Event != 4)
  {
    GetClassName(Old_Button, Control_Type, 4);
    // Checks if this button is an "EDIT" control.
    if(*Control_Type == 'E')
    {
      if(Btn_Event != 5)
      // Before we highlight a new control we must
      //   take away the focus from the old edit control
      //   and give it to the dialog window.
        SetFocus(Win);
    }
    else
    // Or dehighlight the old button.
      Flip(0);
  }
  // If the dialog is not being destroyed
  //   and either a dialog is being initiated
  //   or a button was clicked or otherwise focused.
  if(Btn_Event != 4 && (Btn_Event == 3 || (Btn_Event == 5 ||  ! Btn_Event) && New_Button != Old_Button))
  {
    // Loop through all the buttons to get the index.
    for(int intIndexOfCount = 0; intIndexOfCount < Tot_B; intIndexOfCount++)
    {
      if(But[intIndexOfCount] == New_Button)
      {
        // Make New_Button the current button.
        Cur_B = intIndexOfCount;
        if(Btn_Event == 5)
        // If the button was clicked, we're done.
          return ;
        break;
      }
    }
  }
  else
  {
    for(int intIndexOfCount = 0; intIndexOfCount < Tot_B; intIndexOfCount++)
    {
      // Advance to the next button, or wrap to the first.
      Cur_B = (Key == VK_UP ? Cur_B - 1+Tot_B: Cur_B + 1) % Tot_B;

      // Don't cycle through any hidden buttons
      if(IsWindowVisible(But[Cur_B]))
        break;
    }
  }

  GetClassName(Old_Button = But[Cur_B], Control_Type, 4);

  if(*Control_Type != 'E')
  // Highlight the new button.
    Flip(1);
  else
  // Or set the focus to the new edit control
    SetFocus(Old_Button), SendMessage(Old_Button, EM_SETSEL, 0,  - 1);
  if(Key == 13 &&  ! Btn_Event)
  {
    // The user hit return on a simple button.
    //   an event loop would treat this like a click.
    PopOut = (long)GetMenu(New_Button);
    return ;
  }
}

/// <summary> This function pops up a notice window and waits for a key press. </summary> 
/// <param name="*P"> is the message string that is displayed in the notice window. </param> 
void MessBox(char *P)
{
  // MessageBox( Win, P,"BMSim", MB_TOPMOST);
  int Max = 0, R =  - 1, L = strlen(P);

  // allow 50 lines, but only three are expected.
  char *B = (char*)malloc(L + 50),  *E = B + L,  *Y = B,  *X;
  memmove(B, P, L);
   *E++ = '\n';
   *E = 0;

  // Count the lines and remember the length of the longest line.
  while(1)
  {
    R++;
    X = Y;
    Y = strchr(X, '\n');

    if( ! Y)
    {
      break;
    }

    L = Y - X;
    if(L > Max)
      Max = L;
    memmove(Y + 1, Y, E-Y + 1);
    E++;
    *Y = '\r';
    Y += 2;
  }

  float W = (Max + 4) *CharW, H = (R + 3) *CharH;

  // Creates a system modal window,  centered on the screen
  HWND O, Txt, _Win = CW(0, "WinProcN", "Notice", WS_POPUP|Vis|WS_CAPTION, max(0, Scr[0] + (Scr[2] - W) / 2), Scr[1] + (Scr[3] - H) / 2, W, H, 0, 0, PID, 0);

  // Creates an editable box with our message, so it can be copied.
  Txt = CW(0, "EDIT", B, Chi|ES_MULTILINE|ES_NOHIDESEL|ES_READONLY, CharW, 0, W, R *CharH, _Win, 0, PID, 0);

  SendMessage(Txt, WM_SETFONT, (int)Font_I, 1);

  ShowWindow(Txt, SW_SHOW);

  float _W = CharW * 3.5;

  O = CreateWindow("BUTTON", "Ok", BS_PUSHBUTTON|WS_CHILD, (W - _W) / 2, R *CharH, _W, CharH *1.3, _Win, (HMENU)1, PID, 0);

  SendMessage(O, WM_SETFONT, (int)Font_I, 1);
  ShowWindow(O, SW_SHOW);

  SetForegroundWindow(O);

  while(GetMessage(&Msg, 0, 0, 0))
  {
    DispatchMessage(&Msg);
    int M = Msg.message;
    uint C = Msg.wParam;

    // The escape key or the ok button button close this dialog.
    if(M == WM_CLOSE || M == WM_KEYDOWN && C == 27 || (M == WM_KEYDOWN && C == VK_F4 && GetAsyncKeyState(VK_CONTROL)) || Msg.message == WM_LBUTTONUP && (int)GetMenu(Msg.hwnd) == 1)
    {
      break;
    }
  }

  DestroyWindow(_Win);
  free(B);
}

/// <summary> This function pops up a non-modal "Busy" notice without input required. </summary>
/// <param name="*M"> is the message string that is displayed in the notice window. </param> 
void PW_S(char *M)
{
  if( ! FromInstall &&  ! UI_Up)
    return ;
  //SIZE s;
  static HWND _PW_Win;
  if( ! PW_Cnt++)
  {
    PW_Win = CW(0, "WinProcN", "Busy: ", WS_POPUP|WS_CAPTION|Vis, 0, 0, 0, 0, 0, 0, PID, 0);
    _PW_Win = CW(0, "STATIC", "", Chi|Vis, CharW, CharH, 1640, 480, PW_Win, 0, PID, 0);
    SendMessage(_PW_Win, WM_SETFONT, (int)Font_I, 1);
  }
  int W = strlen(M) *CharW + 2 * CharW, H = 3 * CharH + CapSz;
  SendMessage(_PW_Win, WM_SETTEXT, 0, (DWORD)M);
  SetWindowPos(PW_Win, HWND_TOPMOST, W_H, 0);
  FLUSH_Msgs();
  SetTimer(PW_Win, 1, 700, 0);
}

// <summary> This function takes down the non-modal "Busy" notice window if the notice is still visible.</summary> 
void PW_E()
{
  if(--PW_Cnt <= 0)
  {
    PW_Cnt = 0;
    if(PW_Win)
    {
      KillTimer(PW_Win, 1);
      DestroyWindow(PW_Win);
      PW_Win = 0;
    }
  }
}

/// <summary> This function pops up a dialog box asking a question and it expects the response of Yes or No. </summary>
/// <param name="*Q"> is the pointer to the string of a question. </param>
long Ok(char *Q)
{
  return IDYES == MessageBox(Win, Q, "Question", MB_YESNO);
}

/// <summary> This function launches an executable such as Printer.exe </summary>
/// <param name="*CL"> is the string of the name to the executable file. </param>
void Launch(char *CL)
{
  char Cmd_Ln[400], Path[400];
  static STARTUPINFO SU;
  static PROCESS_INFORMATION Stat;

  SetCursor(Wait_C);

  FLUSH_Msgs();

  Hide(Win);
  FLUSH_Msgs();

  GetCurrentDirectory(100, Path);
  {
    char *p = strchr(Path, 0);

    if(p &&  *--p == '\\')
       *p = 0;
  }

  strcat(Path, "\\");

  {
    for(int intIndexOfCount = 0; intIndexOfCount < 2; intIndexOfCount++)
    {
      //Str( Cmd_Ln, "%s%s%s",Path,J?"EXE\\":"",CL);

      Str(Cmd_Ln, "%s%s%s", Path, intIndexOfCount ? "EXE\\" : "", CL);
      rv = CreateProcess(0, Cmd_Ln, 0, 0, 0, 0, 0, 0, &SU, &Stat);

      if(rv)
        break;
    }
  }

  if( ! rv)
  {
    char m[400];
    Str(m, "Launch: (%s) Failed: %s", Cmd_Ln, GetLastError() > 1 ? "Executable Not Found" : "File is NOT an Executable");

    MessBox(m);
  }

  Sleep(300);

  WaitForSingleObject(Stat.hProcess, INFINITE);

  CloseHandle(Stat.hProcess);

  CloseHandle(Stat.hThread);

  Show(Win);
  SetCursor(SysC);
}


/// <summary> This function hides a dialog window, brings up the old dialog window, and highlights it's top left button. </summary>
/// <param name="W"> is a handle to the window that will be hidden </param> 
void Hide(HWND W)
{
  ShowWindow(W, SW_HIDE);
  FLUSH_Msgs();

  if(W == But[Cur_B])
  {
    Button_Event(4, 0, 0);
  }
}

////////////////////////////////////////
// Char/string handling
////////////////////////////////////////

/// <summary> This function is one of routines for parsing string to see if it is a comment. </summary>
/// <param name="*p"> the pointer to a string. </param>
/// <returns> True if the first two characters are //.  Otherwise it returns false. </returns>   
long IsCommment(char *p)
{
  return  *p == '/' && p[1] == '/';
}

/// <summary> This function is one of routines for parsing strings.This function is one of routines for parsing strings. </summary>
/// <param name="*p"> the pointer to a string. </param>
/// <returns> True if the character is equal to alphabetic, numeric, ", -, \, : in or comment.   </returns> 
/// <remarks> Black is defined as alphabetic, numeric, \\, \, -, " or :  </remarks> 
long IsBlack(char *p)
{
  return isalnum(*p) || IsCommment(p) ||  *p == '"' ||  *p == '-' ||  *p == '\\' ||  *p == ':';
}

/// <summary> This function is one of routines for parsing strings. </summary>
/// <param name="*p"> the pointer to a string </param>
/// <returns> Return the pointer to the next digit in the string. </returns> 
char *NextDigit(char *p)
{
  while(*p &&  ! isdigit(*p))
    p++;
  return p;
}

/// <summary> This function is one of routines for parsing strings. </summary>
/// <param name="*p"> is the pointer to the string </param>
/// <returns> Return the pointer to the next black </returns> 
char *NextBlack(char *p)
{
  while(*p &&  ! IsBlack(p))
    p++;
  if(IsCommment(p))
    return NextLine(p);
  return p;
}

/// <summary> This function is one of routines for parsing strings. </summary>
/// <param name="*p"> is the pointer to the string </param>
/// <returns> Return the pointer to the next white </returns> 
char *NextWhite(char *p)
{
  while(*p && IsBlack(p))
    p++;
  return p;
}

/// <summary> This function is one of routines for parsing strings. </summary>
/// <param name="*p"> the string. </param>
/// <returns> Return the pointer to the next word (delimited by space) from the string </returns> 
char *NextWord(char *p)
{
  return NextBlack(NextWhite(p));
}

/// <summary> This function is one of routines for parsing strings.</summary>
/// <param name="*p"> the string.  </param>
/// <returns> Return the pointer to the next line from the string </returns> 
char *NextLine(char *p)
{
  while(*p &&  *p != 10)
    p++;
  return NextBlack(p);
}


/// <summary> This function assigns Blush(Blue) to Blush(Back), sets DC backgroup color to blue and sets the text color to yellow. </summary> 
void NormColors()
{
  Back = Blue;
  SetBkColor(DC, _Blu);
  FnC(_Ye)
}

/// <summary> This function prints all of reports. </summary> 
/// <param name="*s"> is the quarter name.  </param>
/// <param name="D"> determines which to print.  For example Instructor Reports (D=0), Bank Reports (D=1), or Decision Forms. </param>
void PrintList(char *s, long D)
{
  ListT *P;
  if(D == 0)
  {
    P = &InstrList;

    // Delete the old list of instructor reports.
    {
      DirA if(DirI("%s\\???I???.txt", Mess))
      while(1)
      {
        Str(_T, "%s\\%s", Mess, Next);
        unlink(_T);
        if(DirC)
          break;
      }
    }
    // If it's a  B  type report, make a copy for each bank.
    {
      long i = 0;
      char S[80], D[80];
      Loop(P->Cnt)if(P->Act[J])
      {
        long L;
        char s[8];
        strcpy(s, P->Nam[J]);

        // B and E files are one per bank, the rest are one per community.
        L =  *s == 'B' ||  *s == 'E' ? NumBanks : 1;
        if(*s == 'C')
           *s = 'B';
        // The  C  reports are actually  B  files, but one per community.

        if(*s == 'E')
           *s = 'I';
        // The  E  reports are actually  I  files, but one per bank.
        {
          //Loop(L)
		  for(int J = 0; J < L; J++)
          {
            Str(S, "%s\\%d%s.TXT", Mess, J + 1, s);
            Str(D, "%s\\%03dI%s.TXT", Mess, i++, s + 1);
            Cop(S, D);
          }
        }
      }
    }

    Str(s, "PRINT.EXE %s\\???I???.txt", Mess);
  }
  else
  {
    long Bank;
    char C;
    if(D == 1)
    {
      P = &BankList;
      C = 'B';
    }
    else
    {
      P = &FormList;
      C = 'D';
    }
    Str(s, "%s\\_?%c???.txt", Mess, C);
    {
      DirA if(DirI(s, ""))
      while(1)
      {
        Str(_T, "%s\\%s", Mess, Next);
        unlink(_T);
        if(DirC)
          break;
      }
    }
    {
      char S[80], D[80];
      Loop(P->Cnt)if(P->Act[J])
      {
        char *s = P->Nam[J] + 1;
        LB
        {
          Str(S, "%s\\%d%c%s.TXT", Mess, Bank + 1, C, s);
          Str(D, "%s\\_%d%c%s.TXT", Mess, Bank + 1, C, s);
          Cop(S, D);
        }
      }
    }

    Str(s, "PRINT.EXE %s\\_?%c???.txt", Mess, C);
  }
}

////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////
/// <summary> This function is supposed to generate the random seed. </summary>
/// <param name="w"> is the parameter to generate the random number. </param>
/// <returns> 1 if DisableRaondom is equal to true.  Otherwise returns a random number. </returns> 
WORD RtnSeed(WORD w)
{
	extern int DisableRandom; 
	
	if (DisableRandom)
		return 1;
	else 
		return 2 + (w % 9998);
}

