#pragma once 
#include <direct.h>
#include <io.h>
#include "GlobalDef.h"
#include "BMSimDataT.h"
#include "ReportingDataT.h"
#include "UIHelper.h"
#include "OleHelpers.h"
#include "BMSim.h"
#include "BMSimHelper.h"
#include "FileIOHelper.h"
#include "Reporting.h" //PrnDecs


// These are the various windows:
extern HWND But[300],  *_Win, Win, PW_Win;
extern long FromInstall, PW_Cnt, Cur_B, Tot_B, PopOut;
// Here are the fonts used:
extern HFONT Font, Font2, Font3;
extern HDC DC;
extern HBRUSH SysB, Green, Blue;
extern MSG Msg;
extern HINSTANCE PID;
extern FILE *fp;

extern long Scr[4];
#define W_H  ER( Scr[ 0 ], ( Scr[ 2 ] - W) / 2 ), ER( Scr[ 1 ], ( Scr[ 3 ] - H) / 2 ), W, H

LnP WL(ListT L, char *P); 
void WrTXT(); 

void BMSim(char *_Cmd_Ln); 
void UI(void);
long ActiveCom(char Let);
void All_Coms(long Mode);
long NewCom(); 

void SelList(ListT *P, char *M); 

void ReDrawScr();
int Edit_Form();
void WrLn();
void Input(LnP Banner);

long __stdcall WinProc(HWND Wnd, uint M, uint C, long C2);
long __stdcall WinProcI(HWND W, uint M, uint C, long C2); 
long __stdcall WinProcN(HWND Wnd, uint WM, uint wp, long lp);

// SetupMenu.cpp
void EconMenu();
void SetupMenu(int SP_I); 

//CommunityManager.cpp
void AdminMenu();
void ManageCom(long cI); 