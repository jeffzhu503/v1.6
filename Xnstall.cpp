#pragma warning(disable: 4003 4005 4010 4028 4033 4047 4051 4100 4057 4101 4127 4133 4136 4189 4201 4204 4214 4220 \
 4244 4305 4390 4508 4514 4554 4611 4701 4702 4706 4709 4710 4715 4761)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning( disable: 4035 4214 )
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <direct.h>
#include <dde.h>
#include <lzexpand.h>
#include <time.h>
#include <ole2.h>
#include <shlobj.h>

#define LOOP while(1)
#define Loop(n)short J=-1;while(++J<(n))
#define Str sprintf
#define Rel( X) (! X? 0:(short) X== -1? X= 0:( X->Release(), X= 0))
#define DirA WIN32_FIND_DATA FF_Rec;HANDLE FF_List;
#define DirI(F,S)_DirI(F,S,&FF_Rec,&FF_List)
#define Dir (FF_Rec.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
#define Next FF_Rec.cFileName
#define DirC _DirC(&FF_Rec,FF_List)

typedef unsigned char uchar; typedef unsigned short ushort; typedef unsigned long ulong; typedef unsigned int uint;

char Mess[300],_T[200];long rv;HCURSOR SysC,Wait_C;HICON ICon;
void MessBox(char*m);void Bow(char*m);

long _DirI(char*F,char*S,WIN32_FIND_DATA*FF_Rec,HANDLE*FF_List)
{
	char *p,m[200];
	sprintf(_T,F,S);
	p=_T;
	while(p=strchr(p,'\\')) if(*++p=='\\')memmove(p-1,p,strlen(p)+1);
	strcpy(m,_T);
	if(p=strchr(m,'?'))*p=0;
	if(p=strchr(m,'*'))*p=0;
	if(!(p=strrchr(m,'\\'))||p--==m||(*p--==':'&&toupper(*p)>'B'))
	{
		sprintf(Mess,"Tried to Search a Fixed-Disk's Root: \"%s\"",_T);
		Bow(Mess);
	}
	if((*FF_List=FindFirstFile(_T,FF_Rec))==INVALID_HANDLE_VALUE)
	{
		rv=GetLastError();
		if(rv<=3) return 0;
		sprintf(m,"Error %d Searching for >%s<",rv,_T);
		MessBox(m);
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

long _DirC(WIN32_FIND_DATA*FF_Rec,HANDLE FF_List){
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

HWND But[300],*_Win,Win,PW_Win;
long PW_Cnt,Cur_B,Tot_B,PopOut;
HFONT Font,Font2;
HDC DC;
HBRUSH SysB,Green,Blue;
MSG Msg;
HINSTANCE PID;
FILE *fp;
char *Cmd_Ln;RECT r;

void Send_To_Win_Proc(void){TranslateMessage(&Msg);DispatchMessage(&Msg);}

void MASK_Msg(void);

void FLUSH_Msgs(void){while(PeekMessage(&Msg,0,0,0,0)&&GetMessage(&Msg,0,
  0,0))MASK_Msg();}

#define Hide(W){ShowWindow(W,SW_HIDE);FLUSH_Msgs();\
  if(W==But[Cur_B])Move_Cur(4,0,0);}
#define Show(W){ShowWindow(W,SW_HIDE);ShowWindow(W,SW_SHOW);}

void Flip(long On)
{
	char b[4];
	HWND B=But[Cur_B];
	if(!Tot_B||(GetClassName(B,b,4),*b=='E'))return;
	FLUSH_Msgs();
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

void MessBox(char*m){MessageBox(0,m,0,MB_TOPMOST);}

void PW_E(void){if(--PW_Cnt<=0){PW_Cnt=0;if(PW_Win)
  DestroyWindow(PW_Win),PW_Win=0;}}

void PW_S(char*M){SIZE s;long w,h;static HWND _PW_Win;
  if(!PW_Cnt++){PW_Win=CreateWindow("WinProc","Busy: ",WS_POPUP|WS_VISIBLE|
    WS_CAPTION,0,0,0,0,0,0,PID,0);_PW_Win=CreateWindow("STATIC","",WS_CHILD,
      16,4,1640,480,PW_Win,0,PID,0);}
  DC=GetDC(_PW_Win);SelectObject(DC,Font);
  GetTextExtentPoint(DC,M,strlen(M),&s);w=s.cx+40;
  h=s.cy+20+GetSystemMetrics(SM_CYCAPTION);ReleaseDC(_PW_Win,DC);
  SendMessage(_PW_Win,WM_SETTEXT,0,(DWORD)M);
  SetWindowPos(PW_Win,HWND_TOPMOST,max(0,(640-w)/2),100,
    min(640,w),h,0);FLUSH_Msgs();}

#define Eg 4

void Back_Win(void){if(!_Win)return; DestroyWindow(Win);Win=_Win[1];Tot_B=(long)_Win[2];
  Cur_B=(long)_Win[3];memmove(But,_Win+4,Tot_B*4);Flip(1); {HWND *p=_Win; _Win=(HWND *)*_Win;free(p);} Show(Win)}

long Keyed;
void Move_Cur(long WP,long p,HWND UB){HWND T,B=But[Cur_B];char b[6];
  if(!Tot_B)return;Keyed=WP==1;
  if(p==27){PopOut=(long)GetMenu(*But);return;}
  if(p==13&&(WP||UB==B)){PopOut=(long)GetMenu(But[Cur_B]);return;}
  if(WP==3){GetWindowText(UB,b,4);if(!*b){RECT c,r;GetWindowRect(UB,&c);
    {Loop(Tot_B){GetWindowRect(But[J],&r);if(r.top-Eg==c.top&&
      r.left-Eg==c.left){UB=But[J];break;}}}}}
  if(WP!=4){GetClassName(B,b,4);if(*b=='E'){if(WP!=5)SetFocus(Win);}else 
    Flip(0);}
  if(WP!=4&&(WP==3||(WP==5||!WP)&&UB!=B))
    {Loop(Tot_B)if(But[J]==UB){Cur_B=J;if(WP==5)return;break;}}
  else {Loop(Tot_B){Cur_B=(p==VK_UP?Cur_B-1+Tot_B:Cur_B+1)%Tot_B;
     if(IsWindowVisible(But[Cur_B]))break;}}
  GetClassName(B=But[Cur_B],b,4);
  if(*b!='E')Flip(1);else SetFocus(B),SendMessage(B,EM_SETSEL,0,-1);
  if(p==13&&!WP){PopOut=(long)GetMenu(UB);return;}}    

long Do_Flip;

void MASK_Msg(void){DWORD m=Msg.message,p=(WORD)Msg.wParam;HWND W=Msg.hwnd;
  char b[4];
  if(Do_Flip==1)Do_Flip=2,Flip(1),Do_Flip=0;
  if(Tot_B&&m==WM_NCLBUTTONDOWN&&Msg.wParam==HTCLOSE)
    {PopOut=(long)GetMenu(*But);return;}
  // Looking for a carriage return, tab, up arrow or down arrow.
  if( m== WM_KEYDOWN && Tot_B&&(p==13||p==9||p==VK_UP||p==VK_DOWN)&&
    (GetClassName(W,b,4),*b=='E'))
    {Move_Cur(0,p,W);return;}
  Send_To_Win_Proc();
}

void Get_Msgs(void){char b[4];PopOut=0;SetForegroundWindow(Win);FLUSH_Msgs();
  if(Tot_B&&(GetClassName(But[Cur_B],b,4),*b=='E'))SetFocus(But[Cur_B]);
  while(!PopOut&&GetMessage(&Msg,0,0,0))MASK_Msg();}

/// It will launch BmSim.exe after the installation completes.
void Launch(char*CL)
{
	char Cmd_Ln[400],Path[400];
	static STARTUPINFO s;
	static PROCESS_INFORMATION p;
	SetCursor(Wait_C);
	FLUSH_Msgs();	
	Hide(Win) FLUSH_Msgs(); 
	GetCurrentDirectory(100,Path); 
	{
		char*p=strchr(Path,0);
		if(p&&*--p=='\\')*p=0;
	}
	strcat(Path,"\\");
	{ 
		Loop(1)  ///change from 2 to 1 by Jeff
		{
			sprintf( Cmd_Ln, "%s%s%s",Path,J?"EXE\\":"",CL);
			rv=CreateProcess(0,Cmd_Ln,0,0,0,0,0,0,&s,&p);
			if(rv) break;
		}
	}
	if(!rv){char m[400];
	sprintf(m,"Launch: (%s) Failed: %s",Cmd_Ln,
    GetLastError()>1?"Executable Not Found":"File is NOT an Executable");
    MessBox(m);
}
Sleep(300);
{ 
	DWORD r;
	while(GetExitCodeProcess(p.hProcess,&r)&&r==STILL_ACTIVE)
	{
		FLUSH_Msgs();Sleep(300);}}Sleep(300);Show(Win) SetCursor(SysC);
	}

long __stdcall WinProc(HWND W,uint m, uint p,long P){
  if(m==WM_CLOSE){PopOut=(long)GetMenu(*But);return 1;}
  if(m==WM_PAINT&&W==Win&&IsIconic(Win)){GetClientRect(Win,&r);
    DC=GetDC(Win);DrawIcon(DC,0,0,ICon);ReleaseDC(Win,DC);
    ValidateRect(Win,&r);return 0;} 
  if(m==WM_COMMAND&&p>0&&p<2000){HWND W=GetFocus();PopOut=p;
    if(W!=Win){SetFocus(Win),FLUSH_Msgs();Move_Cur(3,0,W);}}
  if(m==WM_COMMAND&&HIWORD(p)==EN_SETFOCUS&&(HWND)P!=But[Cur_B])
    Move_Cur(5,0,(HWND)P);
  if(m==WM_CREATE&&strncmp(((CREATESTRUCT*)P)->lpszName,"Busy: ",6)){
    if(Win){HWND *p=(HWND *)malloc((4+Tot_B)*4);*p=(HWND)_Win;_Win=p;p[1]=Win;Hide(Win)
      p[2]=(HWND)Tot_B;p[3]=(HWND)Cur_B;memmove(p+4,But,Tot_B*4);
      Win=0;Tot_B=0;}Cur_B=0;}
  if(m==WM_PARENTNOTIFY&&(WORD)p==WM_CREATE){
    SendMessage((HWND)P,WM_SETFONT,(long)(HIWORD(p)>=1000?Font2:Font),1);  
    Show((HWND)P)}
  if(Tot_B&&m==WM_KEYDOWN){Move_Cur(1,p,W);return 1;}
  if(m==WM_CTLCOLORSTATIC||m==WM_CTLCOLORBTN){// 2 Win_32_S Fixs: 
    if(Tot_B&&(HWND)P==But[Cur_B])Do_Flip=1;
    SetBkColor((HDC)p,GetSysColor(COLOR_3DFACE));
    SetTextColor((HDC)p,GetSysColor(COLOR_BTNTEXT));return(LRESULT)SysB;}
  return DefWindowProc(W,m,p,P);} 

#define MaxClient 46
#define MaxExpires 20
typedef struct { char Client[ MaxClient + 2 ], Expires[ MaxExpires + 2 ]; } LicenseT;
LicenseT far License;
char DestDir[10], DST[200], DestDrive;
long NumC; 

void Bow(char*m){PW_E(); if(Win)DestroyWindow(Win); if(m)MessBox(m);exit(1);}

long Approves(char*Q){return IDYES==MessageBox(0,Q,"Question",MB_TOPMOST|MB_YESNO);}

void  Expand ( char  *Src, char  *Dst ) {
  OFSTRUCT SrcRec, DestRec; HFILE SrcFp, DestFp;
  SrcFp = LZOpenFile( Src, &SrcRec, OF_READ );  
  DestFp = LZOpenFile( Dst, &DestRec, OF_CREATE );
  LZCopy( SrcFp, DestFp ); LZClose(SrcFp); LZClose(DestFp);
}

void Cop( char  *Src, char  *Dst ) { CopyFile(Src,Dst,0); SetFileAttributes(Dst,FILE_ATTRIBUTE_NORMAL);}

void cp(char*N){char Dst[180];sprintf(Dst,"%s\\%s",DST,N);Cop(N,Dst);}

char SRC[70],s[200],b[4];long H=34,M=6,MH=40;

#define DsQ "C:\\BmSim"
#define DsQ2 "%s\\BmSim.exe"

int __stdcall WinMain(HINSTANCE _P,HINSTANCE _p,LPSTR _C,int _s){ 
	PID=_P;
	Cmd_Ln = strupr(_C);
	SysB=CreateSolidBrush(GetSysColor(COLOR_3DFACE)); 
	Blue=CreateSolidBrush(RGB(0,0,255));
	Green=CreateSolidBrush(RGB(0,255,0)); 
	Font2=Font=CreateFont(27,0,0,0,0,0,0,0,0,0,0,0,FIXED_PITCH|FF_MODERN,0);
	ICon=LoadIcon(PID,"0"); 
	SysC=LoadCursor(0,IDC_ARROW);
	Wait_C=LoadCursor(0,IDC_APPSTARTING);
	{ 
		WNDCLASS Class=	{CS_VREDRAW|CS_HREDRAW,WinProc,0,0,PID,0,SysC,SysB,0, "WinProc"	};
		RegisterClass(&Class);
	}

	if(!(fp=fopen("S\\SF.DAT","rb")))
	{
		MessBox("'Files Not Found");
		return 1;
	}
 	fread(&License,1,sizeof(License),fp);
	fclose(fp);
	FLUSH_Msgs();
  
	if(!*License.Expires)
	{
		MessBox("This Floppy is NOT Licensed");
		unlink("INSTALL.EXE");
		return 1;
	}
  
	{ 
		short*S=(short*)&License-1;
		Loop(sizeof(License)/2)*S=~*++S;
	}
	{ 
		char*s=License.Expires,*o=s,*Exp=s;
		short m,d,y;    
		while(*s&&*s>='0') s++;
		*s=0;
		m=atoi(o);
		o=++s;
		while(*s&&*s>='0')s++;
		*s=0;
		d=atoi(o);
		y=atoi(++s);
		{ 
			static char *Months[]={"Jan","Feb","March","April","May","June","July","Aug","Sept","Oct","Nov","Dec"};
			sprintf(Exp,"%s %d, %d",Months[max(m-1,0)],d,y);
		}
    
		{ 
			time_t Seconds=time(0);
			struct tm*TimeRec=localtime(&Seconds);
			short D=TimeRec->tm_mday,M=TimeRec->tm_mon+1,Y=TimeRec->tm_year+1900;
			if(Y>y||(Y==y&&(M>m||M==m&&D>d)))
			{
				sprintf(Mess, "license expired _%s_",Exp);
				MessBox(Mess);
				unlink("INSTALL.EXE");
				return 1;
			}
		}
	}
  
	Win=CreateWindow("WinProc","Install",WS_POPUP|WS_VISIBLE|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU,
    0,120,680,M+3*MH+20+GetSystemMetrics(SM_CYCAPTION),0,0,PID,0);

	But[Tot_B++]=CreateWindow("BUTTON","Exit",BS_PUSHBUTTON|WS_CHILD,M,M,80,H,Win,(HMENU)2,PID,0);
	But[Tot_B++]=CreateWindow("BUTTON","Install",BS_PUSHBUTTON|WS_CHILD, M+90,M,120,H,Win,(HMENU)1,PID,0);
	But[Tot_B++]=CreateWindow("BUTTON","README.TXT",WS_CHILD, M+220,M,200,H,Win,(HMENU)3,PID,0);
	CreateWindow("STATIC","Install to:",WS_CHILD, M,60,180,H,Win,0,PID,0);
	But[Tot_B++]=CreateWindow("EDIT",DsQ,WS_BORDER|WS_CHILD, M,90,680-3*M,H,Win,(HMENU)4,PID,0);
	LOOP{Get_Msgs();
	if(PopOut==2)Bow(0);
    if(PopOut==3)
	{
		char*p;
		HWND _Win;
		fp=fopen("README.TXT","rb");
		rv=_filelength(fileno(fp));
		rv=fread(p=(char*)malloc(rv+1),1,rv,fp);
		p[rv]=0;
		fclose(fp);
		Win=CreateWindow("WinProc","Install Help",WS_POPUP|WS_VISIBLE|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU,0,0,640,480,0,0,PID,0);    
		But[Tot_B++]=CreateWindow("BUTTON","Exit",BS_PUSHBUTTON|WS_CHILD,M,M,60,H,Win,(HMENU)2,PID,0);
		_Win=CreateWindow("EDIT",p,ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL|ES_NOHIDESEL|ES_READONLY|WS_CHILD,
        M,M+MH,640-2*M-4,480-30-M-MH,Win,0,PID,0);
		SendMessage(_Win,WM_SETFONT,(DWORD)Font2,1);
		Show(_Win)Get_Msgs();
		free(p);
		Back_Win();
		continue;
	}
    if(PopOut!=1) continue;/*Clicked "Install"*/

	//When the user click Install button. PopOut=1
	//Fix a bug, change the maxium path from 70 to 300
	_getdcwd(0,SRC,70);  //SRC = the current directory where install.exe is.  
	GetDlgItemText(Win,4,DST,100);  //set DST = the installation path
    if(!_chdir(DST))
	{
		sprintf(Mess, "The \"%s\\\" Directory Already Exists.\n\n""Continue by deleting the prior contents of \"%s\\\"?", DST, DST );
		if(!Approves(Mess))Bow(0); 
		{
			DirA 
			char ComA[190],Q1_17[190];
			PW_S("Deleting");
			if(DirI("%s\\*.*",DST))LOOP{sprintf(ComA,"%s\\%s",DST,Next);
			if(!Dir) unlink(ComA);
			else
			{
				DirA if(DirI("%s\\*.*",ComA))LOOP
				{
					sprintf(Q1_17,"%s\\%s",ComA,Next);
					if(!Dir)unlink(Q1_17);
					else
					{
						DirA if(DirI("%s\\*.*",Q1_17))LOOP
						{
							sprintf(_T,"%s\\%s",Q1_17,Next);
							unlink(_T);
							if(DirC)
								break;
						}
						rmdir(Q1_17);
					}
					if(DirC) break;
				}
				rmdir(ComA);
			}
			if(DirC)break;
			}
			PW_E();
		}
	}
    PW_S("Copying.");

	/// Copy all of files from the root installation directory.
	mkdir(DST);
	chdir(SRC);
    {
		DirA  
		//Jeff: Change to use the function directly.
		//if(DirI("%s\\*.*",SRC))
		if(_DirI("%s\\*.*",SRC, &FF_Rec, &FF_List))
		{
			LOOP
			{
				if(!Dir)
				{
					if(stricmp(Next,"XX.CMP")) cp(Next);
					else
					{
						sprintf(_T,DsQ2,DST);
						Expand("xx.cmp",_T);
					}
				}
				if(DirC)break;
			}
		}
	}

	sprintf(Mess,"%s\\ReportTemplates",DST);
	mkdir(Mess);
    {
		DirA
		if(DirI("%s\\ReportTemplates\\*.*",SRC))
		{
			LOOP
			{
				if(!Dir)
				{
					char Dest[180];
					sprintf(Dest,"ReportTemplates\\%s",Next);
					cp(Dest);
				}
				if(DirC)break;
			}
		}
	}

	/// Copy files under S subdirectory
    sprintf(Mess,"%s\\S",DST);
	mkdir(Mess);
    {
		DirA
		if(DirI("%s\\S\\*.*",SRC))
		{
			LOOP
			{
				if(!Dir)
				{
					char Dest[180];
					sprintf(Dest,"S\\%s",Next);
					cp(Dest);
				}
				if(DirC)break;
			}
		}
	}

    _chdrive(toupper(DestDrive)-'A'+1);
    if(chdir(DST))Bow("Can't change to BankExec's directory.");

    CoInitialize(0); 
	IShellLink *Shell; // Shell->SetArguments(""); 
    if( CoCreateInstance( CLSID_ShellLink, 0, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&Shell)>= 0) 
	{
		IPersistFile *File; 
		if( Shell->QueryInterface(IID_IPersistFile, (void **)&File)>= 0) 
		{
			char Start_M[400],Desk_T[400],*SS= getenv("USERPROFILE"); 
			if(SS)strcpy(Start_M,SS); 
			else GetWindowsDirectory(Start_M,400); 
			Str(Desk_T,"%s\\Desktop",Start_M); 
			strcat(Start_M,"\\Start Menu\\Programs");
			chdir(Start_M); unlink("BmSim\\BmSim.lnk"); 
			rmdir("BmSim");
			Str(Mess,"%s\\BmSim.LNK",Desk_T); 
			unlink(Mess); 
			Str(Mess,"%s\\BmSim.EXE",DST); 
			Shell->SetPath(Mess); 
			Shell->SetWorkingDirectory(DST); 
			//ushort S[400];
			wchar_t S[400];
			Str( Mess, "%s\\BmSim.LNK",Start_M); 
			mbstowcs( S, Mess, 400 ); 
			File->Save(S, 1); 
			Rel(File);
		} 
		Rel(Shell); 
	}
	CoUninitialize(); 
	PW_E(); 
	break; 
}
// Per George request, the program should start at the Q3. Disable these three lines.			
// Str(Mess,"BmSim S -R1 -C -B -I");  
// chdir(DST); 
// Launch(Mess); 
}
