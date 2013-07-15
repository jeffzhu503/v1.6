//******************************************************************************************************
// Change List: 
// April, 2008 by Jeff2
// - Add Copies text book so the print model can print mulitple copies of reports and forms.
// - Reduce the length of Files and Port textbox
// *****************************************************************************************************
#pragma warning(disable: 4003 4005 4010 4028 4033 4047 4051 4100 4057 4101 4127 4133 4136 4189 4201 4204 \
 4214 4220 4244 4305 4390 4508 4514 4554 4611 4701 4702 4706 4709 4710 4715 4761)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning( disable: 4035 4214 )
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <commdlg.h>
#include <math.h>
#include <direct.h>

#define Loop( _LLL ) int J = -1, LLL = _LLL ;  while ( ++ J < LLL )
#define LOOP while ( 1 )
#define Fr(A) (free(A),A=0)
#define SetPtr SelectObject
#define Str sprintf
#define Eq !stricmp
#define FnC(C) SetTextColor(PnDC,C); 
#define CW CreateWindowExA
#define DirA WIN32_FIND_DATA FF_Rec;HANDLE FF_List;
#define DirI(F,S)_DirI(F,S,&FF_Rec,&FF_List)
#define Dir (FF_Rec.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
#define Next FF_Rec.cFileName
#define DirC _DirC(&FF_Rec,FF_List)
#define Call_WinProc {TranslateMessage(&Msg);DispatchMessage(&Msg);}
#define Set_TXT(W,T)SetWindowText(W,T)
#define Ctr_Close  ( M == WM_KEYDOWN && C == VK_F4 && GetAsyncKeyState( VK_CONTROL ) )

typedef unsigned char uchar;
typedef unsigned short ushort; 
typedef unsigned long ulong ; 
typedef unsigned int uint;  
typedef char * Line ;

enum
{
	Chi= WS_CHILD, 
	Vis= WS_VISIBLE, 
	_Blac= RGB(0,0,0), 
	_Ye= RGB(255,255,0),
	_Blu= RGB(0,0,255),
	_Gr= RGB(0,255,0), 
  _Re= RGB(255,0,0),
	MaxF_Sort= 600
};

RECT r, SubRec; 
HWND But[ MaxF_Sort],*_Win,Win,WinP,PW_Win; 
MSG Msg; 
int  Paint_Stat; 
char Mess[300],_T[200]; 
int Do_Flip, rv,FL,mx,my,But_H,Row, Cur_File, FileCnt;  
const float WHR= 49./ 81.;
int Keyed, PW_Cnt,Cur_B,Tot_B,PopOut,PervPO;
long Scr[4];
HFONT OF,ChipFont;HDC DC; HBRUSH SysB,Green,Blue;HINSTANCE PID;
FILE *fp; 
char * Cmd_Ln, LanDir [ 99 ];

void Move_Cur(long WP,long p,HWND UB);

void Show(HWND W) 
{ 
	ShowWindow(W,SW_HIDE); 
	ShowWindow(W,SW_SHOW); 
}
void Hide(HWND W)
{ 
	ShowWindow(W,SW_HIDE); 
	if(W==But[Cur_B])Move_Cur(4,0,0); 
}

void MessBox(char*m);
void Bow(char*m); 

long _DirI(char*F,char*S,WIN32_FIND_DATA*FF_Rec,HANDLE*FF_List)
{
  char *p;
	Str(_T,F,S);
  p=_T;
	while(p=strchr(p,'\\'))
		if(*++p=='\\') memmove(p-1,p,strlen(p)+1);
  if((*FF_List=FindFirstFile(_T,FF_Rec))==INVALID_HANDLE_VALUE)
	{
    rv=GetLastError();
		if(rv<=3)return 0;
    Str(Mess,"Error %d Searching for >%s<",rv,_T);
		MessBox(Mess);
		return 0;
	}
  LOOP
	{
		if(*(*FF_Rec).cFileName!='.')return 1;
		if(!FindNextFile(*FF_List,FF_Rec))
		{
			FindClose(*FF_List);
			return 0;
		}
	}
}

long _DirC(WIN32_FIND_DATA*FF_Rec,HANDLE FF_List)
{
	LOOP
	{
		if(!FindNextFile(FF_List,FF_Rec))
		{
			FindClose(FF_List);
			return 1;
		}
		if(*(*FF_Rec).cFileName!='.')
			return 0;
	}
}

void MASK_Msg();  

void Resp()
{
	while(PeekMessage(&Msg,0,0,0,0)&&GetMessage(&Msg,0,0,0))
		MASK_Msg();
}

void Flip(long On)
{
	char b[4];
	HWND B=But[Cur_B];
	if(!Tot_B||(GetClassName(B,b,4),*b=='E'))
		return;
	Resp();
  RedrawWindow(B,0,0,RDW_ERASE|RDW_ERASENOW|RDW_INVALIDATE|RDW_INTERNALPAINT|RDW_UPDATENOW);
  if(On)
	{
		GetClientRect(B,&r);
		DC=GetDC(B);
		FrameRect(DC,&r,Blue);
    r.top++;
		r.bottom--;
		r.left++;
		r.right--;
		FrameRect(DC,&r,Green);
    ReleaseDC(B,DC);
	}
}

void PW_E()
{
	if(--PW_Cnt<=0)
	{
		PW_Cnt=0;
		if(PW_Win)DestroyWindow(PW_Win),PW_Win=0;
	}
}

//*M is the file name to print
void PW_S(char *M)
{
	SIZE s;
	long MCW,h;
	static HWND _PW_Win;
  if(!PW_Cnt++)
	{
		PW_Win= CW ( 0, "WinProcN","Busy: ",Vis,0,0,0,0,Win,0,PID,0);
		_PW_Win= CW ( 0, "STATIC","",Chi,16,4,1640,480,PW_Win,0,PID,0);
	}
  DC=GetDC(_PW_Win);
	OF=(HFONT)SetPtr(DC,ChipFont); 
	GetTextExtentPoint(DC,M,strlen(M),&s);
	MCW=s.cx+40;
  h=s.cy+20+GetSystemMetrics(SM_CYCAPTION);
	if(OF)SetPtr(DC,OF);
	ReleaseDC(_PW_Win,DC);
	SendMessage(_PW_Win,WM_SETTEXT,0,(DWORD)M);
  SetWindowPos(PW_Win,HWND_TOPMOST,max(0,(640-MCW)/2),100,min(640,MCW),h,0);
	Resp();
}

void Move_Cur(long WP,long p,HWND UB){HWND T,B=But[Cur_B];char b[6];
  if(!Tot_B)return;Keyed=WP==1;
  if(p==27){PopOut=(long)GetMenu(*But);return;}
  if(p==13&&(WP||UB==B)){PopOut=(long)GetMenu(But[Cur_B]);return;}
  if(WP==3){GetWindowText(UB,b,4);if(!*b){RECT c,r;GetWindowRect(UB,&c);
    {Loop(Tot_B){GetWindowRect(But[J],&r);if(r.top-4==c.top&&
      r.left-4==c.left){UB=But[J];break;}}}}}
  if(WP!=4){GetClassName(B,b,4);if(*b=='E'){if(WP!=5)SetFocus(Win);}else 
    Flip(0);}
  if(WP!=4&&(WP==3||(WP==5||!WP)&&UB!=B))
    {Loop(Tot_B)if(But[J]==UB){Cur_B=J;if(WP==5)return;break;}}
  else {Loop(Tot_B){Cur_B=(p==VK_UP?Cur_B-1+Tot_B:Cur_B+1)%Tot_B;
     if(IsWindowVisible(But[Cur_B]))break;}}
  GetClassName(B=But[Cur_B],b,4);
  if(*b!='E')Flip(1);else SetFocus(B),SendMessage(B,EM_SETSEL,0,-1);
  if(p==13&&!WP){PopOut=(long)GetMenu(UB);return;}}    

void Get_Msgs(){char b[4];PopOut=0;SetForegroundWindow(Win);Resp();
  if(Tot_B&&(GetClassName(But[Cur_B],b,4),*b=='E'))SetFocus(But[Cur_B]);
  while(!PopOut&&GetMessage(&Msg,0,0,0))MASK_Msg();}

void Back_Win(void){if(!_Win)return;
  DestroyWindow(Win);Win=_Win[1];Tot_B=(long)_Win[2];
  Cur_B=(long)_Win[3];memmove(But,_Win+4,Tot_B*4);Flip(1);
  {HWND *p=_Win;_Win= (HWND *)*_Win;Fr(p);}Show(Win);}

long __stdcall WinProc( HWND W, uint M, uint C, long C2 )
{  // WriteWM;
  if(M==WM_CLOSE)
	{
		PopOut=(long)GetMenu(*But);
		return 1;}
  if(M==WM_COMMAND&&C>0&&C<2000)
	{
		HWND W=GetFocus();
		PopOut=C;
    if(W!=Win)
		{
			SetFocus(Win),Resp();
			Move_Cur(3,0,W);
		}
	}
  if(M==WM_COMMAND&&HIWORD(C)==EN_SETFOCUS&&(HWND)C2!=But[Cur_B])
		Move_Cur(5,0,(HWND)C2);
  if(M==WM_CREATE&&strncmp(((CREATESTRUCT*)C2)->lpszName,"Busy: ",6))
	{
    if(Win)
		{
			HWND *C= (HWND *)malloc((4+Tot_B)*4);
			*C=(HWND)_Win;
			_Win=C;
			C[1]=Win;
			Hide(Win);
      C[2]=(HWND)Tot_B;
			C[3]=(HWND)Cur_B;
			memmove(C+4,But,Tot_B*4);
			Win=0;
			Tot_B=0;
		}
		Cur_B=0;
	}
  if(M==WM_PARENTNOTIFY&&(WORD)C==WM_CREATE)
	{
		long f=HIWORD(C);
    SendMessage((HWND)C2,WM_SETFONT,(int)ChipFont,1);
		Show((HWND) C2);
	}
  if( M == WM_CLOSE || M == WM_KEYDOWN && C == 27 || Ctr_Close )
		M = WM_KEYDOWN,  C = 27 ;
  if ( Tot_B && M == WM_KEYDOWN ) 
	{
		Move_Cur ( 1, C, W );
		return 1;
	}
  if(M==WM_CTLCOLORSTATIC||M==WM_CTLCOLORBTN)
	{// 2 Win_32_S Fixs: 
    if(Tot_B&&(HWND)C2==But[Cur_B])
			Do_Flip=1;
		SetBkColor((HDC)C,GetSysColor(COLOR_3DFACE));
    SetTextColor((HDC)C,GetSysColor(COLOR_BTNTEXT));
		return(LRESULT)SysB;
	}
	return DefWindowProc(W,M,C,C2);
} 

PRINTDLG PrnRec;HDC PnDC;HFONT ScFnt, BigScFnt,  PrnFon, BigPrnFon;
char *Buff,*Cur,*Old,File_List[290],Copies[3]=" 1",PrnName[290],PrnPort[290],MemErr[] = 
 "Possible Out of Memory Error.\n     Try turning off the Print spooler";
LOGBRUSH Black={BS_SOLID,_Blac,0},Yellow={BS_SOLID,_Ye,0};
HPEN OP,YPen,BPen,WidePen;enum{Portrate,Land};
long Len,Way,Queued,Line_Able,MiddleOfPage;
float BigSc= 1.5, Scale,ChW,BigChW,ScrCharW,BigScrCharW,CharH,_CharH,CharH_L,CharH_P,Margin,MarginL,MarginP,LM,PrnCharW,BigPrnCharW;
typedef struct{long T,cx,dx,Len;float y[13];}GT;

void Bow(char*m){PW_E();if(Queued)EndDoc(PnDC);if(m)MessBox(m);exit(1);}

long Approves(char*Q){return IDYES==MessageBox(Win,Q,"Question",MB_YESNO);}

int Rnd ( float F ) { return ( int ) floor ( F + .5 ); }

// _Say() and Say() work the same,
//   no matter if the HDC is a printer or a screen.
void _Say ( float X, float Y, char * Bu, int L ) { 
  if( L <= 0) return; 
  int _Y = Y- CharH * .3, YY = Rnd( Y ), LL; 
  char * P = Bu ;
  LOOP { 
    Bu = P ; 
    while ( L && ( uchar ) * P < 128 ) { L--; P++; } 
    LL = P- Bu ; 
    if ( LL > 0) { 
      X -= ChW;
      Loop ( LL ) TextOut( PnDC, 
        Rnd ( X += ChW ), YY, Bu + J, 1 ); }
    if ( L ) { TextOut ( PnDC, Rnd ( X += ChW ), _Y, P, 1 );
      X += ChW ; L -- ; P ++ ; }  if( ! L ) break; } }

#define FuSay \
  if( P > Bu) _Say ( LM + ( Bu- Old ) * ChW, Y, Bu,P- Bu); \
  Bu = P + 1 ;

#define FuFuSay \
  if ( P > Bu) {  SetBkMode( PnDC, OPAQUE ); \
    float W = ChW ; ChW = BigChW ;  \
    _Say( LM + ( Bu - Old ) * W, \
      Y - CharH * BigSc, Bu, P - Bu );  ChW = W ; \
    SetBkMode( PnDC, TRANSPARENT ); }  Bu = P + 1 ;

void Say ( ) {  Resp ( ); // Resp() handles mouse events
  char C, * P = Old, * Bu = Old ; 
  float Y = Row * CharH + Margin ; 
  if ( Line_Able ) { 
    while( P < Cur) {
      if ( * P == '_' && P [ 1 ] !='_') { 
        FuSay while ( ++ P <= Cur && ( uchar ) * P > 32 );
        rv = FnC ( _Re );  SetPtr ( PnDC, OF ); 
        OF = ( HFONT ) SetPtr ( PnDC, BigPrnFon ); 
        FuFuSay SetPtr ( PnDC, OF ); 
        OF = ( HFONT ) SetPtr ( PnDC, PrnFon ); 
        FnC ( rv ); } 
      else if ( * P == '|' ) {  FuSay 
        MoveToEx ( PnDC, 
          rv = Rnd ( LM + ( P - Old ) * ChW + ChW / 2),
          Rnd ( Y ), 0 ); 
        LineTo ( PnDC, rv, Rnd ( Y + CharH ) ); }
      else if ( ( * P == '-' || * P == '=' )
        && P [ 1 ] == ( C = * P ) ) {  FuSay 
          OP = C == '-' ? 
            0 : ( HPEN ) SetPtr( PnDC, WidePen ); 
          MoveToEx ( PnDC, 
            Rnd ( LM + ( P- Old ) * ChW ), 
            rv = Rnd ( Y + CharH / 2 ), 0 );
          while ( ++ P <= Cur &&  * P == C );  Bu = P ;  
          LineTo ( PnDC, 
            Rnd ( LM + ( P- Old ) * ChW ), rv ); 
          if ( OP ) SetPtr ( PnDC, OP ); continue; }
    P++; } }   FuSay Old = Cur + 1; }

void Graph(){long R=(Len-(Cur-Buff))/sizeof(GT);Say();if(!R)return;
  {static char*a="*";long n;float X,Y,_X,_Y,x,y,l,d,hh=CharH/2,hw=ChW/2,ds=hw;GT*G=((GT*)Cur)-1;
  OP=(HPEN)SetPtr(PnDC,WidePen);
  {Loop(R){if((++G)->T<0||G->T>2)break;{Loop(G->Len){X=LM+Rnd((G->cx+J*G->dx+.5)*ChW);Y=Margin+Rnd((G->y[J]+.5)*CharH);
    if(G->T==2)TextOut(PnDC,X-hw,Y-hh,a,1);else if(!J)MoveToEx(PnDC,_X=X,_Y=Y,0);else if(!G->T)LineTo(PnDC,X,Y);
    else{x=X-_X;y=Y-_Y;if(!x){l=y;y=ds;}else if(!y){l=x;x=ds;}else{l=sqrt(x*x+y*y);d=atan(y/x);x=ds*cos(d);y=ds*sin(d);}
      n=Rnd(l/ds/2);{Loop(n){LineTo(PnDC,Rnd(_X+=x),Rnd(_Y+=y));MoveToEx(PnDC,Rnd(_X+=x),Rnd(_Y+=y),0);}}
      MoveToEx(PnDC,_X=X,_Y=Y,0);}}}}}if(OP)SetPtr(PnDC,OP);}}

void FreshPage()
{ 
	DEVMODE* Printer; 
	Printer= (DEVMODE*)GlobalLock(PrnRec.hDevMode);
  Printer->dmOrientation=Way==Land?DMORIENT_LANDSCAPE:DMORIENT_PORTRAIT;
  ResetDC(PnDC,Printer);
	GlobalUnlock(PrnRec.hDevMode);
  CharH= Way== Land? CharH_L: CharH_P; 
	Margin= Way== Land? MarginL:MarginP;
  // == 76 Lines By 93 Columns === 54 Lines By 122 Columns
  StartPage(PnDC);
	ChW=PrnCharW; 
	BigChW=BigPrnCharW; 
	if(OF)SetPtr(PnDC,OF);
	OF= (HFONT)SetPtr(PnDC,PrnFon); 
	MiddleOfPage=1;
	Row=0;
}

// The following two lines are in some routines somewhere ...
// CharH, a float, is scaled to fit a screen. 
//  Land is for landscape.
// CharH = ( Way == Land ? 9 : 6.5 ) * Scale ;
// or a sheet of paper.
// CharH = Way == Land ? CharH_L : CharH_P ;

// CF() creates a font for a printer or a screen.
// The aspect Ratio _Must_Be_ determined first.
//   Tries for Bold.
// F, for font, is a bound reference, 
//   one of the few C++ features that I use. 
void CF ( HDC DC, HFONT & F, float Targ_Width ) { 
  F = CreateFont( - Rnd ( 81 * Targ_Width / 49 ), 0,0,0
      , 700,0,0,0,0,0,0,0, FIXED_PITCH, 0 );
  int Tmp =! DC ; if ( Tmp ) DC = GetDC ( 0 ); 
  HFONT OF = ( HFONT ) SetPtr ( DC, F ); SIZE Sz; 
  GetTextExtentPoint ( DC,"XXXXX", 5, & Sz ); 
  float Act_W = Sz.cx / 5 ; SetPtr ( DC, OF );
  if( Act_W > Targ_Width ) { DeleteObject( F );
    F =  CreateFont( rv = -Sz.cy * Targ_Width / Act_W, 0,0,0
         , 700,0,0,0,0,0,0,0, FIXED_PITCH, 0);
    SetPtr( DC, F); 
    GetTextExtentPoint( DC,"XXXXX", 5, & Sz );  Act_W = Sz.cx / 5 ; 
    SetPtr( DC, OF );
    if ( Act_W > Targ_Width ) {  DeleteObject( F );
      F =  CreateFont( rv * Targ_Width / Act_W, 0,0,0
           , 0,0,0,0,0,0,0, DRAFT_QUALITY, FIXED_PITCH, 0 ); } } 
  if ( Tmp ) ReleaseDC ( 0, DC ); }

// Gets details about the selected printer.
void GetPrinter ( ulong F, int Passive ) {
  HDC Old_DC = PnDC; PrnRec.Flags = F | PD_RETURNDC ;
  if( ! PrintDlg ( & PrnRec ) ) {
    if ( Passive ) return; int Err = CommDlgExtendedError();
    if ( Err ) { 
      Str( Mess,"Printer Setup Dialog Error %d",
        Err ); Bow( Mess ); } if ( ! Err ) return ; }
  if ( ! ( PnDC = PrnRec.hDC ) ) 
    if ( Passive ) return ; else Bow("No Printer Selected");
  { DEVNAMES * Printers = 
    ( DEVNAMES * ) GlobalLock ( PrnRec.hDevNames );
    Str ( PrnName,"%s", 
      ( Line ) Printers + Printers->wDeviceOffset );
    Str ( PrnPort, 
      ( Line ) Printers + Printers->wOutputOffset );
    SetDlgItemText ( Win, 1006, PrnName );
    SetDlgItemText ( Win, 1020, PrnPort );
    GlobalUnlock ( PrnRec.hDevNames ); }
    if ( PrnFon ) DeleteObject ( PrnFon );
    Line_Able = GetDeviceCaps ( PnDC, LINECAPS );
  if ( Old_DC && Old_DC != PnDC ) DeleteDC( Old_DC );
  SetBkMode( PnDC, TRANSPARENT );
  SetMapMode( PnDC, MM_ISOTROPIC );
  { DEVMODE * Printer = 
    ( DEVMODE * ) GlobalLock( PrnRec.hDevMode);
    _CharH = GetDeviceCaps(PnDC,LOGPIXELSY) * 9.4 / 72 * 
      ( Printer->dmPaperSize == DMPAPER_A4 ?
        21 / 2.54 / 8.5 : 1 );
    GlobalUnlock ( PrnRec.hDevMode ); }
      // CF Creates a font. 
      CF ( PnDC, PrnFon, PrnCharW = _CharH * WHR );
      CF ( PnDC, BigPrnFon, BigPrnCharW = 
        _CharH * WHR * BigSc );
  LM = 7 * PrnCharW ; // Sets various margins.
  MarginP = CharH_P = _CharH * 1.032 ; 
  MarginL = ( CharH_L = _CharH * 1.06 ) * 2 ; }

enum{ MaxNamLen= 11 };
char Files[MaxF_Sort][MaxNamLen+ 1],Files_Dir[290];

int MCH,MCW,M,mwW,Files_B,Rem,MaxRows;

void List_Files( int _Rem){char *Slash;Rem|=_Rem;if(!*File_List)return;
  strcpy(Files_Dir,strupr(File_List));Slash=strrchr(Files_Dir,'\\');
  if(!Slash)Slash=strchr(Files_Dir,':');if(Slash)Slash[1]=0;else*Files_Dir=0;FileCnt=-1;
  { DirA if(!DirI(File_List,"")){
    Str(Mess,"_ %s _  was  Not  Found\nin  _ %s _",_T, LanDir );
    MessBox(Mess);return;}LOOP{ 
      if(! Dir&& strlen( Next) <= MaxNamLen && ++FileCnt< MaxF_Sort) strcpy( Files[ FileCnt], strupr( Next) ); 
        if( DirC) break; } }
  if(++FileCnt>=MaxF_Sort){Str(Mess,"%d Files requested; %d is "
    "the limit.",FileCnt,MaxF_Sort);MessBox(Mess);FileCnt=MaxF_Sort;}
  LOOP{long done=1;Loop(FileCnt-1){if(stricmp(Files[J],Files[J+1])>0)
    { strcpy(Mess,Files[J+1]); strcpy(Files[J+1],Files[J]); 
    strcpy(Files[J],Mess);done=0;}}if(done)break;}if(!Rem)return;
  {Loop(Tot_B-Files_B)DestroyWindow(But[Files_B+J]);Tot_B=Files_B;}
  { int C,W= 0; Loop(FileCnt) { C= J/ MaxRows; strcpy(_T,Files[J]); char*P= strrchr(_T,'.'); if(P)*P=0; 
    if(!W) W= (strlen(_T)+1)* MCW; But[Tot_B++]=
       CW ( 0, "BUTTON",_T, BS_RIGHT| Chi, M+ C* (W+ M),M+ (J% MaxRows+ 4)*( MCH+ M),W,MCH,Win,(HMENU)(1100+J),PID,0); }
    // Displays a larger main dialog, listing all the reports.
    SetWindowPos(Win,HWND_TOP, Scr[0],Scr[1], 
      max(mwW,(FileCnt+MaxRows-1)/MaxRows* (M+W)+2*M),
      (min(MaxRows,FileCnt)+4)*(M+MCH)+20+ GetSystemMetrics(SM_CYCAPTION),
      SWP_SHOWWINDOW);
    Tot_B++;}}

void Respond() { while( PeekMessage(& Msg,0,0,0, PM_REMOVE )) Call_WinProc }
 
void MASK_Msg(void){DWORD WM=Msg.message,p=(WORD)Msg.wParam;HWND Wnd=Msg.hwnd;char b[4];
  if(WM== WM_PAINT&& WinP&& Wnd== WinP) { 
    if ( PopOut!= 2000) PervPO= PopOut, PopOut= 2000; }
  if(Do_Flip==1){Do_Flip=2;Flip(1);Do_Flip=0;}
  if(Tot_B&&WM==WM_NCLBUTTONDOWN&&p==HTCLOSE){PopOut=(long)GetMenu(*But);return;}
  if(WM==WM_KEYDOWN&&Tot_B&&(p==13||p==9||p==VK_UP||p==VK_DOWN)&&(GetClassName(Wnd,b,4),*b=='E'))
    { Move_Cur( 0, p, Wnd ); return; }   Call_WinProc}

void _Fresh_Page( int Fill ) {  
  if( Paint_Stat== 2) { PopOut= PervPO; return; }  Paint_Stat= 2;  Row=0; int X= PopOut;  SetBkMode(PnDC,OPAQUE);
  if( Cur_File>= FileCnt-1) Hide(But[3]); if( Cur_File<= 0) Hide(But[2]);
  if ( Fill ) FillRect(PnDC,&SubRec,Blue), ValidateRect( WinP,& SubRec);  
  Show(*But); Show(But[1]); if( Cur_File<FileCnt-1)Show(But[3]);  if(Cur_File>0)Show(But[2]);
  Respond();  SetBkMode(PnDC,TRANSPARENT);  Paint_Stat= 0; PopOut= Fill?0:X; }   //   LOOP Respond();

// Pops up a notice window and waits for a key press.
void MessBox( char* P ) {  // MessageBox( Win, P,"BankExec International", MB_TOPMOST);
  float CharW = BigScrCharW, CharH = 81 * BigScrCharW / 49 * 1.3 ;
  int Max= 0, R= 0, L = strlen( P); 
  // allow 50 lines, but only three are expected.
  char *B = (char *)malloc( L + 50 ), *E= B +L, *Y= B, *X; memmove( B, P, L); *E++= '\n'; *E= 0;
  // Count the lines and remember the length of the longest line.
  LOOP { R++; X= Y; Y = strchr( X, '\n' ); if ( !Y ) break; L = Y- X; if ( L > Max ) Max = L;
    memmove( Y+1, Y, E - Y + 1); E++; *Y = '\r'; Y += 2; }
  float W = ( Max + 4 ) * CharW, H = ( R + 3 ) * CharH;
  // Creates a system modal window,  centered on the screen
  HWND O, Txt, _Win= CW ( 0, "WinProcN", "Notice", WS_POPUP|Vis|WS_CAPTION,  max( 0, Scr[0] + (Scr[2] -  W) / 2), 
    Scr[1] + (Scr[3] -  H) / 2,  W,  H, 0,0,PID,0);
  // Creates an editable box with our message, so it can be copied.
  Txt= CW ( 0, "EDIT", B, Chi|ES_MULTILINE|ES_NOHIDESEL|ES_READONLY,
    CharW ,0, W, R * CharH, _Win,0,PID,0);  
    SendMessage( Txt, WM_SETFONT,(int)BigScFnt,1); 
    ShowWindow( Txt, SW_SHOW);
  float _W = CharW * 3.5;
  O = CreateWindow("BUTTON","Ok",BS_PUSHBUTTON|WS_CHILD
      , (W -  _W) / 2, R * CharH, _W, CharH * 1.3,_Win,(HMENU)1,PID,0);
  SendMessage( O, WM_SETFONT,(int)BigScFnt,1); 
  ShowWindow( O, SW_SHOW);  SetForegroundWindow( O );
  while( GetMessage(&Msg,0,0,0)) { DispatchMessage( & Msg );  int M = Msg.message ; uint C = Msg.wParam ;
    // The escape key or the ok button button close this dialog.
    if( M == WM_CLOSE || M == WM_KEYDOWN && C == 27 || Ctr_Close 
      || Msg.message==WM_LBUTTONUP && (int)GetMenu(Msg.hwnd) == 1) break; } 
 DestroyWindow( _Win); free( B); }

// This is the Window routine for a  " Notice "  dialog.
long __stdcall WinProcN(HWND Wnd,uint WM, uint wp,long lp){
  return DefWindowProc(Wnd,WM,wp,lp); }

int __stdcall WinMain(HINSTANCE _P,HINSTANCE _p,LPSTR _C,int _s)
{
	PID=_P;
	Cmd_Ln = strupr(_C);
  _getdcwd( 0, LanDir, 99 );
  SysB=CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	Blue=CreateSolidBrush(_Blu);
	Green=CreateSolidBrush(_Gr);
  {
		WNDCLASS Class={ 0, WinProc,0,0,PID,0,0,SysB,0,"WinProc"};
		RegisterClass(&Class);
	}
  { 
		WNDCLASS Class={ 0, WinProcN,0,0,PID,0,0,SysB,0, "WinProcN"};
		RegisterClass(&Class);
	}
  YPen=CreatePen(PS_SOLID,0,_Ye); 
	BPen= ExtCreatePen(PS_GEOMETRIC|PS_JOIN_BEVEL,3,&Black,0,0);

  SystemParametersInfo( SPI_GETWORKAREA,0, ( RECT *) Scr ,0);
  // Selects the largest resonable scale
  Scr[2]-= Scr[0]; 
	Scr[3]-= Scr[1]; 
	Scale= min( Scr[2] / 640., Scr[3] / 480. );
  strcpy(File_List,!*Cmd_Ln?"*.TXT":Cmd_Ln);   
  { 
		DC= GetDC(0); 
    CF( DC, ScFnt, ScrCharW= Scale* 5); 
    CF( DC, ChipFont, Scale* 6);  
    CF( DC, BigScFnt, BigScrCharW = Scale * 5 * BigSc );
    OF= (HFONT)SetPtr( DC, ChipFont); 
		SIZE s; 
		char *TS= "12345"; 
		int L= strlen(TS); 
    GetTextExtentPoint(DC,TS,L,&s); 
		MCW= Rnd( s.cx/ (float)L);  
		MCH= s.cy; 
		if(OF) SetPtr( DC, OF); 
		ReleaseDC(0,DC);
	}
  // Determins how many rows of files we can list.
  M= MCH/4;
	MaxRows=(Scr[3])/(M+MCH)-6; 
	mwW= 64* MCW, But_H= MCH+ 3;
  // Creates the main dialog.
	// CW is CreateWindow(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)
  Win= CW ( 0, "WinProc","Print",Vis|WS_MINIMIZEBOX|WS_SYSMENU,Scr[0],Scr[1],
    mwW,6*(MCH+M)+ GetSystemMetrics(SM_CYCAPTION),0,0,PID,0);
  PrnRec.lStructSize=sizeof(PrnRec); 
	GetPrinter( PD_RETURNDEFAULT, 1);
  But[Tot_B++]= CW ( 0, "BUTTON","Exit",Chi,M,M,6*MCW,But_H,Win,(HMENU)1002,PID,0);
  But[Tot_B++]= CW ( 0, "BUTTON","Print",Chi,2* M+ 7* MCW,M,6*MCW,But_H,Win,(HMENU)1004,PID,0);
  But[Tot_B++]= CW ( 0, "BUTTON","Stop Printing",Chi,3* M+ 14* MCW,M,15*MCW,But_H,Win,(HMENU)1003,PID,0);
  But[Tot_B++]= CW (  0, "BUTTON", "Change Printers",Chi,4* M+ 30* MCW,M,17*MCW,But_H,Win,(HMENU)1005,PID,0);
  CW ( 0, "STATIC","Files:",Chi,M,2*M+MCH,9*MCW,MCH,Win,0,PID,0);
  But[Tot_B++]= CW ( 0, "EDIT",File_List,WS_BORDER|Chi,2*M+6*MCW,2*M+MCH,34*MCW,MCH,Win,(HMENU)1010,PID,0);
	CW ( 0, "STATIC", "Copies:",Chi,3*M+48*MCW,2*M+MCH,7*MCW,MCH,Win,0,PID,0);
	But[Tot_B++]= CW ( 0, "EDIT",Copies,WS_BORDER|Chi,4*M+55*MCW,2*M+MCH,3*MCW,MCH,Win,(HMENU)1011,PID,0);
  CW ( 0, "STATIC", "Printer:",Chi,M,3*M+2*MCH,8*MCW,MCH,Win,0,PID,0);
	CW ( 0, "STATIC",PrnName,Chi,2*M+8*MCW,3*M+2*MCH,30*MCW,MCH,Win,(HMENU)1006,PID,0);
  CW ( 0, "STATIC","Port:",Chi,M,4*M+3*MCH,7*MCW,MCH,Win,0,PID,0);
  But[Tot_B++]= CW ( 0, "EDIT",PrnPort,WS_BORDER|Chi,2*M+5*MCW,4*M+3*MCH,35*MCW,MCH,Win,(HMENU)1020,PID,0);  Files_B=Tot_B;
  But[Tot_B++]= CW ( 0, "BUTTON","Print Individual Reports", Chi,M,5*M+4*MCH,25*MCW,But_H,Win,(HMENU)1007,PID,0);
  LOOP
	{
		char*fn;
		long opo;
		Get_Msgs();
		if(PopOut==1002) //Exit button
			Bow(0);
		//Update the File_List and Copies if users make the change in the Files textbox
    GetDlgItemText(Win,1010,File_List,290);
		GetDlgItemText(Win,1011,Copies,3);
		int PrintCopies = atoi(Copies);

		//Replace space and '/' with '\\'
		strupr(File_List);
		{
			char*P=File_List;
			LOOP
			{
				while(*P&&*P!='/')P++;
				if(*P)
				{
					*P='\\';
					continue;
				}
				break;
			}
		}
    // Win95: OpenPrinter()  GetPrinter(PRINTER_INFO_5.Attributes &
    // PRINTER_ATTRIBUTE_QUEUED)
    
		//Click the Change Printers button 1005
		if(PopOut==1005)
		{
			GetPrinter(PD_PRINTSETUP, 1); 
			continue;
		}
		
		//Click Print Individual Reports 1007
		if(PopOut==1007)
		{
			List_Files(1);
			continue;
		}

    GetDlgItemText(Win,1020,PrnPort,290);
    if(*File_List)List_Files(0);
		else if(PopOut==1010)MessBox("No Change");

		if(PopOut>=1100)
		{
			long T_LM=LM, LA= Line_Able;
			HDC DC,T_DC=PnDC;
			HPEN OP;
      HFONT TBF= BigPrnFon, TF= PrnFon; 
			BigPrnFon= BigScFnt; 
			PrnFon= ScFnt;
      Cur_File=PopOut-1100;
			opo=PopOut;
      // Creates the window for displaying BankSim's reports and forms.
      WinP=Win= CW ( 0, "WinProc","",WS_POPUP|WS_VISIBLE,Scr[0],Scr[1], mx= Scr[2],my= Scr[3],0,0,PID,0); 
      my-=40;
			GetClientRect( WinP, &SubRec);
      But[Tot_B++]= CW ( 0, "BUTTON","Exit",Chi,FL=mx-24*MCW,my,5*MCW,But_H,Win,(HMENU)1002,PID,0);
      But[Tot_B++]= CW ( 0, "BUTTON","Print",Chi,mx-18*MCW,my,6*MCW,But_H,Win,(HMENU)1003,PID,0);
      But[Tot_B++]= CW ( 0, "BUTTON","Prev",Chi,mx-11*MCW,my,5*MCW,But_H,Win,(HMENU)1004,PID,0);
      But[Tot_B++]= CW ( 0, "BUTTON","Next",Chi,mx-5*MCW,my,5*MCW,But_H,Win,(HMENU)1005,PID,0);
      PnDC= DC= GetDC(Win); 
			SetBkMode(PnDC,OPAQUE);
      OP= (HPEN)SetPtr(DC, WidePen= YPen); 
			Line_Able= 1;
      FnC(_Ye);
			OF= (HFONT)SetPtr(DC,PrnFon);
			SetBkColor(DC,_Blu);
      LOOP
			{
				fn=Files[Cur_File];
				Str(Mess,"%s%s",Files_Dir,fn);
				{ 
					char*z=strchr(fn,0),*T=z-8; 
					Way= Land;
					if(*T=='D'||T[1]=='D'||*T=='W'||T[1]=='W')Way= Portrate; 
				}
				fp=fopen(Mess,"rb");
				if(fp)
				{
					Set_TXT(WinP,Mess);
					Len=_filelength(fileno(fp));
					Buff= ( Line ) realloc(Buff,Len+4);
					if(!fp||!Buff)Bow(MemErr);
					Len=fread(Buff,1,Len,fp);
					fclose(fp);
					*(short*)(Buff+Len)=0; 
					Cur = Buff- 1; 
					Old= Buff;
					LM=8;
					ChW=ScrCharW;
					BigChW=BigScrCharW;
					Margin=-8;
					CharH=(Way==Land?9:6.5)*Scale; 
					int Page= 0, Pages = 0; 
					char* Tops[5]; 
					Tops[Pages++]= Cur;
					while( *(short*)++Cur) if ( *Cur == 12 ) Tops[Pages++]= Cur;  
					if(PopOut==1004) 
						Page = Pages- 1;
					LOOP
					{ 
						Cur = Tops[Page]; 
						Old= Cur+ 1;
						_Fresh_Page( 1 );
						while( !PopOut ) 
						{ 
							++Cur; 
							if ( *Cur== 12 || !*(short*)Cur) 
							{
								Say();
								Row++; 
								Page++; 
								break;
							} 
							
							if((uchar)* Cur>= 32) continue;
							switch(*Cur) 
							{ 
							case 9:
								*Cur=32;continue; 
							case 13:
								Say();
								Row++;
								break; 
							default: 						
								Old= Cur+ 1; 
							} 
						}
						if(Page== Pages) 
							Graph(); 
						_Fresh_Page( 0 ); 
						Get_Msgs();
						if(PopOut== 2000)	
						{
							Page--;
							continue; 
						}
						if(PopOut==1004&& ( Page-=2 ) >= 0 ) continue;  
						if(PopOut==1005&& Page < Pages ) continue;  
						break;
					}
				}  
				if(PopOut==1004) 
				{ 
					if(Cur_File>0)Cur_File--; 
					continue; 
				} 
	      if(PopOut==1005)
				{
					if(Cur_File<FileCnt-1)Cur_File++;
					continue; 
				}   
				break; 	
			} 
			WinP= 0;

      Line_Able= LA; 
			LM=T_LM; 
			PnDC= T_DC; 
			BigPrnFon= TBF; 
			PrnFon= TF; 
			if(OF)SetPtr(DC,OF);
      if(OP)SetPtr(DC,OP);
			ReleaseDC(Win,DC);
			Back_Win();
			if(PopOut!=1003)
				continue;
      {
				long bH=34,ws=bH+4,bW=300,bLM=10,bTM=6;
       // Displays the dialog to print from a selected report.  Centered on the screen.
				int W = bW + 2 * bLM,  H = 3 * ws + 20 + GetSystemMetrics(SM_CYCAPTION);
				Win= CW ( 0, "WinProc","Print",Vis|WS_MINIMIZEBOX|WS_SYSMENU,max(0, ( Scr[2] - W )/2),( Scr[3] - H ) / 2, W, H,0,0,PID,0);
	      But[Tot_B++]= CW ( 0, "BUTTON","Exit",Chi,bLM,bTM,80,bH,Win,(HMENU)2,PID,0);
		    Str(Mess,"Print \"%s\"",Files[Cur_File]);
				But[Tot_B++]= CW ( 0, "BUTTON",Mess,Chi, bLM,ws+bTM,bW,bH,Win,(HMENU)1,PID,0);
				But[Tot_B++]= CW ( 0, "BUTTON","Print to End of List",Chi, bLM,2*ws+bTM,bW,bH,Win,(HMENU)3,PID,0);
				Get_Msgs();
				Back_Win();
				if(PopOut==2)
					continue;
			}		
		}
	
	//Click the Print button 
		if(PopOut==1004||PopOut==1||PopOut==3) 
		{
			Loop(PrintCopies) //This line of code enables to print mulitple copies of reports and forms. 
			{
				if(!PnDC) 
					GetPrinter( PD_RETURNDEFAULT, 0);
				int _Loop,One=PopOut==1;
				char QN[80];
				strcpy(QN,File_List); 
				if(PopOut==3)
					Str(QN,"%s starting at %s",File_List,fn);
				Str(Mess,"Printing %s",One?fn:QN);
				PW_S(Mess);
				{ 
					static DOCINFO di;
					di.cbSize=sizeof(di);
					Queued=1;
					di.lpszDocName=One?fn:QN;
					di.lpszOutput=PrnPort;
					if(StartDoc(PnDC,&di)<=0)
					{
						Str(Mess,"Device (%s) error (%d).",PrnPort,GetLastError());
						Bow(Mess);
					}
				}

				_Loop=One?1:FileCnt;
				WidePen= BPen; 
				if(PopOut==3)
				{
					_Loop=FileCnt-Cur_File;
				}
				else Cur_File=0;
				PopOut=0;
				{
					Loop(_Loop)
					{
						long _Cur_File=J+Cur_File;
						{
							char*z=strchr(One?fn:Files[_Cur_File],0),*T=z-8;
							Way= Land; 
							if(*T=='D'||T[1]=='D'||*T=='W'||T[1]=='W') Way= Portrate; 
						} 
						Str(Mess,"%s%s",Files_Dir,One?fn:Files[_Cur_File]);
						fp=fopen(Mess,"rb");
						Len=_filelength(fileno(fp));
						Buff= ( Line ) realloc(Buff,Len+4);
						if(!fp||!Buff)Bow(MemErr);
						Len=fread(Buff,1,Len,fp);
						fclose(fp); 
						if(!One)
						{
							Str(Mess,"Printing %s",Files[_Cur_File]);
							PW_S(Mess);
							PW_E();
						}
						OF=0;
						FreshPage();
						*(short*)(Buff+Len)=0;
						Cur=Buff-1;
						Old=Buff; 
						SetBkMode(PnDC,TRANSPARENT);
						while(*(short*)++Cur)
						{
							if((uchar)* Cur>= 32)continue;
							if(PopOut)break;
							switch(*Cur)
							{
							case 9:
								*Cur=32;
								break;
							case 13:
								Say();
								Row++;
								break; 
							case 12:
								Say();
								EndPage(PnDC);
								FreshPage();
								break;
							default:
								Old=Cur+1;
							}
						}
						Graph();
						if(OF)SetPtr(PnDC,OF);
						EndPage(PnDC);
						MiddleOfPage=0;
						if(PopOut)break; 
					} 
				}
				if(MiddleOfPage)
					EndPage(PnDC);
				EndDoc(PnDC);
				Queued=0;         
				PW_E();
			}
		}
	}

	Bow(0);
}
