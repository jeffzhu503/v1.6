#include "UI.h"
#include <sys\stat.h>
#include <sys\timeb.h>
#include <time.h>

/// BMSim's main set of global variable: Communities
extern ComT Com[MaxComm]; 
extern int Bank, Bank_On, ComOn, UI_Up, Drive, NumBanks, NumComs, SimQtr, SimYr, YrsQtr, ComI, rvs, len;
extern char FromCom, Community, _Com[10], DatFile[40], FileMode[8];

extern int _In;
extern char Mess[400], _T[400], _For[10];
extern long rv, EconSet_I;

ListT InstrList, BankList, FormList;

long BXI_Sz, CapSz;
HCURSOR SysC, Wait_C;
HICON ICon;

// General data used on reports:
extern char BankName[50], TimeStamp[88], BankNeu[20], StrA[MaxFields][FldSz], ForQtr[40], ForQtrs[60], AsOf[40], EndOf[40], Date[Qtrs][10], FQ[Qtrs + 1][10], PQ[Qtrs][10];
long FromDrive, ToDrive;
long _NoDumps, L4[5], NewBanks, NumRuns;

//Variables used in reporting
extern int Page, EconPreview;  

// These are the various windows:
HWND But[300],  *_Win, Win, PW_Win;
long FromInstall, PW_Cnt, Cur_B, Tot_B, PopOut;
// Here are the fonts used:
HFONT Font, Font2, Font3;
HDC DC;
HBRUSH SysB, Green, Blue;
MSG Msg;
HINSTANCE PID;
FILE *fp;
//extern RECT r;
long Scr[4];

//extern HWND BT[5];
//extern RECT SubRec, CharRec;
HBRUSH Black, Back;


//MaxFldsPerForm = 128
extern FldsT Flds[128],  *FldP;
//extern POINTS MouseXY;

//variables used in FileIOHelper
extern FILE *Out;
extern float Version; 
float RBanks;  //Between UI and Loans

// Variables are used in UIHelper
int Do_Flip;
HFONT Font_I; // Large monospaced font for input routines and popup messages
float Scale, CharW, CharH; // The size of a character for input and error messages

// Variables are used in BMSimHelper.h
//extern char LngDate[18], _LngDate[18];
int SupressTimeStamp = 0; 
int DisableRandom = 0; //in 2013 we disabled random for the development of the starting point and testing purpose 
int AutoTest = 0;

//Remove: using procedure WL() instead.
//#define WL(L) { Loop( L.Cnt){ if(! L.Act[J]) *P++='-'; memmove( P, L.Nam[J],4); P+=4; *P++ =32; } } P[-1] = 10;
/// <summary> This function copies the list of form names into the string in order to write into Bmsim.txt. </summary>
/// <param name="L"> is the list of form names. </param>
/// <param name="*P"> is the pointer to the string. </param>
LnP WL(ListT L, char *P) { 
	Loop( L.Cnt){ 
		if(! L.Act[J]) 
			*P++='-'; 
		memmove( P, L.Nam[J],4); 
		P+=4; 
		*P++ =32; 
	} 
	P[-1] = 10;
	return P;
}

/// <summary> This function writes the selected instruction report, bank reports, decision forms and economy set number into Bmsim.txt file. </summary>
void WrTXT()
{
  LnP B, P = B = (LnP)malloc(BXI_Sz + 200);
  //WL(InstrList)WL(BankList)WL(FormList) 
  P = WL(InstrList, P);
  P = WL(BankList, P);
  P = WL(FormList, P);
  *P++ = EconSet_I + 48;
  *P++ = 10;
  fp = fopen("BMSim.TXT", "w");
  if( ! fp)
    Bow("Can't write BMSim.TXT.");
  fwrite(B, 1, P - B, fp);
  fclose(fp);
  Fr(B);
}

/// <summary> This function parses the command line for BMSim.exe. </summary>
/// <param name="*_Cmd_Ln"> Pointer to a null-terminated string specifying the command line for the application </param>
/// <returns> Return 2 if the command line is null. Return 0 if it is successful.  </returns>
/// <remarks> bmsim [communityname] -R -C -B -I -E -A -D -S
/// [communityname] Specifies the community name to run the bank simulater.  It is usually a folder name under the bmsim root folder 
/// -A	Bmsim will run AdminMenu() first. This option can not be run with other options. 
/// -B	Set Bank_On to 1. This opition is related to Excel object. 
/// -C	Set ComOn to 1. This opition is related to Excel object.
/// -D	Set _UnDumps to 1.
/// -E	Bmim will run EconMenu first. This option can not be used with other options. 
/// -I	Set SupressTimeStamp to 1.  This option is supposed to control whether the reports will have time stamp or not.  However it is commented out in the code now. 
/// -R	Defines the number of runs by assigning value to NumRun.  It must be between 1 and 8 inclusively. 
/// </remarks> 
int GeSw(char *_Cmd_Ln)  // shall move back out of UI.cpp
{
  char *P = NextBlack(strupr(_Cmd_Ln));
  FromCom = FromDrive = Community =  *_Com = ToDrive = NumRuns = EconPreview = 0;
  NewBanks = 10;
  if( !  *P)
    return 2;
  if(*P == '-')
    goto La2;
  if(P[1] == ':')
  {
    FromDrive =  *P;
    Community =  *_Com = P[3];
    if(P[4])
    {
      MessBox("Invalid command line.");
      return 0;
    }
    goto La;
  }
  if(P[1] == '-')
  {
    if(P[3] == ':')
    {
      Community =  *_Com =  *P;
      ToDrive = P[2];
      if(P[4])
      {
        MessBox("Invalid command line.");
        return 0;
      }
      goto La;
    }
    FromCom =  *P;
    Community =  *_Com = P[2];
    if(P[3] == '-')
      NewBanks = P[4] - '0';
    goto La;
  }
  if(P[1] > ' ')
  {
    Str(Mess, "Community Letter Can't be a _String_: \"%2.2s\"", P);
    MessBox(Mess);
    return 0;
  }
  Community =  *_Com =  *P;
  La:

  if( ! isalnum(Community))
  {
    Str(Mess, "Community Letter Can't be: '%c' (Ascii: %d).", Community, Community);
    MessBox(Mess);
    return 0;
  }
  if( ! FromCom && ActiveCom(Community) < 0)
  if(Community == '0')  //-R0 will update economy data using MakeDats( )
  {
    MakeDats();
    return 0;
  }
  else
  {
    Str(Mess, "Can't Find \"%c\\QTR.TXT\"", Community);
    MessBox(Mess);
    return 0;
  }
  La2:

  while(*(P = NextWord(P)))
    if(*P == '-')
  switch(*++P)
  {
    case 'E':
      EconMenu();
      return 0;
    case 'A':
      AdminMenu();
      return 0;
    case 'B':
      Bank_On = 1;
      break;
    case 'C':
      ComOn = 1;
      break;
    case 'I':
      FromInstall = 1;
      break;
    case 'S':
      SupressTimeStamp = 1;
      break;
    case 'T':
      SupressTimeStamp = 1;
      DisableRandom = 1;
      AutoTest = 1;
      break;
    case 'D':
      _NoDumps = 1;
      break;
    case 'R':
      NumRuns = Bound(atoi(P + 1), 0, 8);  //change for debugging purpose 8/1/2008
      if( ! FromDrive &&  ! ToDrive)
          break;
      MessBox("Can NOT run To Or From another Drive");
      return 0;
  }
  return  ! Community + 1;
}

/// <summary> This function is the core function in the Bmsim, which calls RunForward() to do the calculations and write all reports. </summary> 
/// <param name="*_Cmd_Ln"> is the command line options.  For example, C -R1 -C -B. </param>
// 2013 1) provide ability to save results of quarterly runs to flash drives instead of floppy drive
void BMSim(char *_Cmd_Ln)  // shall move back out of UI.cpp
{
  if(GeSw(_Cmd_Ln) != 1)
    return ;
   *Mess = 0;
  if(UI_Up && NumRuns)
  {
    if(ToDrive || FromDrive)
    {
      MessBox("Invalid command line.");
      return ;
    }
  }
  // The code opens hello.txt to verify if the floppy drive is available in A: or not. 
  else if(ToDrive)
  {
    Str(Mess, "%c:\\HELLO.TXT", ToDrive);
    fp = fopen(Mess, "w");
    if( ! fp)
    {
      MessBox("Can't write to an external drive");
      return ;
    }
    fclose(fp);
    Str(Mess, "Save Q%d/%02d decisions to %c:? (In Community \"%s\")", ConfA, ToDrive, _Com);
  }
  else if(FromDrive)
  {
    Str(Mess, "%c:\\%s\\qtr.txt", FromDrive, _Com);
    fp = fopen(Mess, "r");
    if( ! fp)
    {
      MessBox("Can't find the Community on the external drive");
      return ;
    }
    {
      char *p;
      long Q;
      WORD S;
      S = fread(Mess, 1, 80, fp);
      memset(Mess + S, 0, 4);
      fclose(fp);
      p = NextBlack(Mess);
      Q = atoi(p);
      Str(Mess, "Restore Q%d/%02d decisions from %c:? (In ""Community \"%s\")", Q % 4+1, BaseSimYear - 2000+Q / 4, FromDrive, _Com);
    }
  }
  if(*Mess &&  ! Ok(Mess))
    return ;
  if(FromCom || FromDrive || ToDrive)
  {
    ComT *c = Com + ComI;
    if(FromDrive)
    {
      CopyComDrv(c);
      All_Coms(Read_All_Coms);
      Str(Mess, "Community \"%s\"  Has Been Restored From Drive %c:", _Com, FromDrive);
    }
    else if(ToDrive)
    {
      CopyComDrv(c);
      Str(Mess, "Community \"%s\"  Has Been Backed-Up To Drive %c:", _Com, ToDrive);
    }
    else //When user selects to create a new community. 
    {
      if( ! NewCom())
        Str(Mess, "No Community Created");
      else
      {
        if(NumRuns)
        {
          RunForward();
          return ;
        }
        All_Coms(Update_All_Coms);
        Str(Mess, "Community \"%s\" has been created.  (From \"%c\") ", _Com, FromCom);
      }
    }
    if( ! NumRuns)
    {
      MessBox(Mess);
      return ;
    }
  }
  if(*_Com &&  ! NumRuns)
  {
    MakeDats();
    return ;
  }
  if( ! NumRuns)
  {
    MessBox("Invalid Command Line");
    return ;
  }
  RunForward();
}

/// <summary> This function reads banks information from the file QTR.TXT into *c, a community. </summary> 
/// <param name="Mode">The long integer that read all of available communities </param>
void All_Coms(long Mode)
{
	extern PayIntOnDemDepT PayIntOnDemDep;

  char Buff[1806],  *p;
  WORD S;
  long i =  - 1;
  DirA Version = 1;
  PayIntOnDemDep.Bus = PayIntOnDemDep.Pub = PayIntOnDemDep.Ind = 0;
  GetCurrentDirectory(400, Mess);
  if(DirI("%s\\?", Mess))
  while(1)
  {
    if(IsDir)
    {
      ComT *c = Com + ++i;
      if(Mode == Update_All_Coms)
      {
        if(i != ComI)
          continue;
        if(NumRuns)
          c->Seed = RtnSeed(rand());
        SetTimeStamp();
        if( ! c->NumBanks)
          Bow(" No Banks in Com");
        ODF("QTR", "TXT", "w");
        fprintf(fp, "%d  %d // Start Of Q%d/%02d;%s\n", c->SimQtr, c->Seed, c->SimQtr % 4+1, BaseSimYear + c->SimQtr / 4-2000, TimeStamp);
        {
          Loop(c->NumBanks)fprintf(fp, "   \"%s\"\n", c->BankName[J]);
        }
        fclose(fp);
        return ;
      }
      c->Community = toupper(*Next);
      {
        struct stat fcb;
        char b[12];
        FILE *fp;
        Str(b, "%c\\qtr.txt", c->Community);
        fp = fopen(b, "r");
        if( ! fp)
        {
          --i;
          if(DirC)
            break;
          continue;
        } 
		fstat(fileno(fp), &fcb);
        if(fcb.st_size > 1800)
          Bow("\nqtr.txt is OVER 1800 Bytes\n");
        S = (WORD)fcb.st_size;
        S = fread(Buff, 1, S, fp);
        memset(Buff + S, 0, 4);
        fclose(fp);
        p = NextBlack(Buff);
      }
      c->SimQtr = atoi(p);
      p = NextWord(p);
      {
        char *q = p;
        p = NextDigit(p);
        if( !  *p || p - q > 1)
        {
          Str(Mess, "Community %c Has a Bad Seed# @: (%7.7s)", c->Community, q);
          Bow(Mess);
        }
		if (DisableRandom)
           c->Seed = 1;
        else
        {
          char Seed[6];
          strncpy(Seed, p, 4);
          Seed[4] = 0;
          c->Seed = atoi(Seed);
        }
      }
      p = NextWord(p);
      memset(c->BankName, 0, sizeof(c->BankName));
      {
        long j =  - 1;
        while(*p == '"')
        {
          char *e = strchr(++p, 10);
          if( ! e)
            e = strchr(++p, 0);
          while(*--e &&  *e != '"')
            ;
          {
            long Len = e-p;
            static long NamLen = sizeof(*c->BankName);
            if(*e != '"')
            {
              p = e;
              break;
            }
            j++;
            memmove(c->BankName[j], p, Len < NamLen ? Len : NamLen - 1);
          }
          p = NextWord(++e);
        }
        c->NumBanks = ++j;
      }
    } //end if(IsDir)
    if(DirC)
      break;
  } //end while(1)
  if(Mode == Read_All_Coms)
    NumComs = ++i;
}

/// <summary> This function determines which community is active community and will be run based on the BmSim command options </summary>
/// <param name="Let"> is the community name passed from the command line when running BmSim </param>
/// <returns> the index of the communties if the community is found.  Otherwise it returns -1 </param> 
/// <remarks> This function also sets several important global variables in Bmsim such as NumBanks, Community, SimQtr, SimYr and YrsQtr. </remarks>
long ActiveCom(char Let)
{
  //Loop(NumComs)
  for(int J = 0; J < NumComs; J++)
  {
    ComT *c = Com + J;
    char Let2 = c->Community;
    if(Let != Let2)
      continue;

    Community =  *_Com = Let;
    NumBanks = c->NumBanks;
    RBanks = 1.0 / NumBanks;
    SimQtr = c->SimQtr;
    SimYr = BaseSimYear + SimQtr / 4;
    YrsQtr = SimQtr % 4;
    srand(c->Seed); //Initialize random number generator. The same seed will generate the same succession of results in subsequent calls to rand. 
    return ComI = J;
  }
  return  - 1;
}

/// <summary> This function copies selective files from a source community to a destination community. </summary>
/// <param name="*rootName"> is the partial name of files copied from one community to another. </param>
/// <param name="*extension"> is the file extension name. </param>
/// <param name="i"> is the index. </param>
/// <param name="j"> is the index. </param>
/// <param name="*SrcNam"> is the source file name. </param>
/// <param name="*DestNam"> is the destination name. </param>
/// <param name="ToLet"> is the new community name. </param>
/// <remarks> When users selects to creates a new community, the new community must be created by copying from an existing community. For example community S. </summary> 
void Cpy_Com(char *rootName, char *extension, int i, int j, char *SrcNam, char *DestNam, char ToLet) { 
  
	//int size = strlen(rootName) + 12 + strlen(extension) + 1;
	
	//Review: if use calloc or malloc, causes crash, because possible buffer overrun
	//somewhere else in the code.
	char S1[80]; // = (char *)calloc(1, size);
	char S2[80]; // = (char *)calloc(1, size);
	//TODO calloc success check

	if(i >= 0) {
		sprintf(S1, "%s%d%s", rootName, i+1, extension);
	} 
	else {
		sprintf(S1, "%s%s", rootName, extension);
	}

	if(j >= 0) {
		sprintf(S2, "%s%d%s", rootName, j+1, extension);
	}
	else {
		sprintf(S2, "%s%s", rootName, extension);
	}

	sprintf(SrcNam, "%c\\%s", FromCom, S1);

	sprintf(DestNam, "%c\\%s", ToLet, S2);
	
	Cop(SrcNam,DestNam);

	//free(S1);
	//free(S2);	
}

/// <summary> This function creats a new community when user selects this option from the setup window. </summary> 
long NewCom()
{
  char ToLet = Community;
  char SrcNam[400], DestNam[400], New_Dir[400];
  long FromI = ActiveCom(FromCom);
  long ToI = ActiveCom(ToLet);
  long NamSz = sizeof * Com[0].BankName;
  long Num_Wanted_From_Old = min(NewBanks, Com[FromI].NumBanks);
  
  if(ToI < 0)
    ToI = NumComs++;
  if(FromI < 0)
  {
    MessBox("NewCom: Copy-From Comm was Not Found");
    return 0;
  }
  Str(New_Dir, "%c", ToLet);
  mkdir(New_Dir);
  Str(Mess, "%c\\SF.DAT", ToLet);
  if(UI_Up && (fp = fopen(Mess, "r")))
  {
    fclose(fp);
    Str(Mess, "    === W A R N I N G ===\n\n""This Community Already Exists.\n\n""Continue By Deleteing Community %s?", _Com);
    if( ! Ok(Mess))
      return 0;
    {
      DirA char Sub[400];
      if(DirI("%s\\*.*", New_Dir))
      do
      {
        Str(Sub, "%s\\%s", New_Dir, Next);
        if( ! IsDir)
          unlink(Sub);
        else
        {
          DirA if(DirI("%s\\*.*", Sub))
          do
          {
            Str(_T, "%s\\%s", Sub, Next);
            unlink(_T);
          }
          while( ! DirC);
          rmdir(Sub)
            ;
        }
      }
      while( ! DirC)
        ;
    }
  }
  memmove(Com + ToI, Com + FromI, sizeof(ComT) - ((MaxB - Num_Wanted_From_Old) *NamSz));
  {
    ComT *c = Com + ToI;
    c->Community = ToLet;
    if(NewBanks != 10)
    {
      c->NumBanks = Num_Wanted_From_Old = 1;
    }
    {
	  //Loop(Num_Wanted_From_Old)
	  for(int J = 0; J < Num_Wanted_From_Old; J++)
      {
        //Cpy_Com("BK0%d.DAT", J, J)Cpy_Com("BK0%d.IN", J, J)Cpy_Com("LN0%d.DAT", J, J)
		  
		  //Cpy_Com("BK0%d.DAT", J, J, SrcNam, DestNam, ToLet);
		  Cpy_Com("BK0", ".DAT", J, J, SrcNam, DestNam, ToLet);

		  //Cpy_Com("BK0%d.IN", J, J, SrcNam, DestNam, ToLet);
		  Cpy_Com("BK0", ".IN", J, J, SrcNam, DestNam, ToLet);

		  //Cpy_Com("LN0%d.DAT", J, J, SrcNam, DestNam, ToLet);
		  Cpy_Com("LN0", ".DAT", J, J, SrcNam, DestNam, ToLet);
      }
    }
    if(NewBanks <= MaxB)
    {
      {
        struct timeb MilRec;
        ftime(&MilRec);
        c->Seed = RtnSeed(MilRec.millitm + (WORD)time(0));
      }
      {
        Loop(NewBanks - Num_Wanted_From_Old)
        {
          long i = Num_Wanted_From_Old + J;
          c->NumBanks++;
          Str(c->BankName[i], "%s",  *c->BankName, i + 1);
          //Cpy_Com("BK0%d.DAT", 0, i)Cpy_Com("BK0%d.IN", 0, i)Cpy_Com("LN0%d.DAT", 0, i)

		  //Cpy_Com("BK0%d.DAT", 0, i, SrcNam, DestNam, ToLet);
		  Cpy_Com("BK0", ".DAT", 0, i, SrcNam, DestNam, ToLet);

		  //Cpy_Com("BK0%d.IN", 0, i, SrcNam, DestNam, ToLet);
		  Cpy_Com("BK0", ".IN", 0, i, SrcNam, DestNam, ToLet);

		  //Cpy_Com("LN0%d.DAT", 0, i, SrcNam, DestNam, ToLet);
		  Cpy_Com("LN0", ".DAT", 0, i, SrcNam, DestNam, ToLet);
        }
      }
    }
  }
  //Cpy_Com("SF.DAT", 0, 0)Cpy_Com("EC.DAT", 0, 0)Cpy_Com("QTR.TXT", 0, 0)ActiveCom(ToLet);
  
  //Cpy_Com("SF.DAT", 0, 0, SrcNam, DestNam, ToLet);
  //Cpy_Com("EC.DAT", 0, 0, SrcNam, DestNam, ToLet);
  //Cpy_Com("QTR.TXT", 0, 0, SrcNam, DestNam, ToLet);

  Cpy_Com("SF", ".DAT", -1, -1, SrcNam, DestNam, ToLet);

  Cpy_Com("EC", ".DAT", -1, -1, SrcNam, DestNam, ToLet);
  
  //Inject the value to Prices.Qtr = 0 2013
  {
    ActiveCom(ToLet);
	PricesT Prices; 
	DatIO("EC", "r", 18, &Prices, sizeof Prices);
	Prices.Qtr = 0;
	DatIO("EC", "r+", 18, &Prices, sizeof Prices);
	ActiveCom(FromCom);
  } 

  Cpy_Com("QTR", ".TXT", -1, -1, SrcNam, DestNam, ToLet);

  ActiveCom(ToLet);
  SetStateNames();
  char F[2];
  DirA Str(F, "%c", FromCom);
  Loop(2)if(DirI(J ? "%s\\1????.TXT" : "%s\\Bank_?1*.XLS", F))
  do
  {
    Str(SrcNam, "%c\\%s", FromCom, Next);
    Str(DestNam, "%c\\%s", ToLet, Next);
    Cop(SrcNam, DestNam);
  }
  while( ! DirC);
  if( ! FromInstall)
    CopyForms(PQ[1]);
  Copy_Qtr(*PQ, 0);
  Str(_Com, "%c\\%s", FromCom, PQ[1]);
  _Com[4] = '_';
  Copy_Qtr(PQ[1], 0);
  Str(_Com, "%c", ToLet);
  return 1;
}

/// <summary> Create the Main Menu window </summary>
void UI(void)
{
	char s[300]; // the same variable is also used in ManageCom() in CommunityManagerMenu.cpp

  enum
  {
    bH = 34, ws = bH + 8, bW = 300, Wid = 540
  };

  UI_Up = 1;

  int SP_I = 0, Coms;
  L01: Coms = NumComs > 1;
  {
    Loop(NumComs)if(Com[J].Community == 'S')
      SP_I = J;
  }

  // If there are communities, Create a dimensionless window with Setup button and community buttons to hold them.
  if(Coms)
  {
    Win = CW(0, "WinProc", "BMSim", MaS, 0, 0, 0, 0, 0, 0, PID, 0);

    But[Tot_B++] = CW(0, "BUTTON", "Exit", Chi, bLM, bTM, 85, bH, Win, (HMENU)2, PID, 0);

    But[Tot_B++] = CW(0, "BUTTON", "Setup", Chi, bLM, ws + bTM, bW, bH, Win, (HMENU)200, PID, 0);

    CW(0, "STATIC", "Communities:", Chi, bLM, ws *2+bTM, bW, bH, Win, 0, PID, 0);
  }

  while(1)
  {
    if( ! Coms)
      PopOut = 200;
    else
    {
      {
        Loop(Tot_B - 2)DestroyWindow(But[2+J]);
      }

      // Size the main dialog window to fit all the communities.

      int W = bW + 2 * bLM, H = (NumComs + 2) *ws + 20+CapSz;
      SetWindowPos(Win, HWND_NOTOPMOST, W_H, 0);
      Tot_B = 3;

      {
        //Loop(NumComs)
		for(int J = 0; J < NumComs; J++)
        {
          ComT *c = Com + J;
          char s[50], C = c->Community;
          if(C == 'S')
            continue;

          Str(s, "%c", C);
          But[Tot_B++] = CW(0, "BUTTON", s, Chi, bLM, (Tot_B) *ws + bTM, bW, bH, Win, (HMENU)(100+J), PID, 0);
        }
      }

      Get_Msgs();

      if(PopOut == 2)
      {
        Back_Win();
        Bow(0);
      }
    }

	// Add new button for Economy Management window
	if(PopOut == 200)	
	{
		if(SetupMenu(SP_I) == 2)
		{
			goto L01;
		}
		
		continue;
	}

    if(PopOut < 109)
      ManageCom(PopOut - 100);
  }
} // SetUnhandledExceptionFilter(NoCrash);

/// <summary> 
/// This is the main function in this Window 32 application
/// </summary>
/// <param name="_P"> HINSTANCE - Handle to the current instance of the application. </param>
/// <Param name = "_p"> HINSTANCE - Handle to the previous instance of the application. </param>
/// <Param name="_C"> LPSTR - Pointer to a null-terminated string specifying the command line for the application, excluding the program name. </param>
/// <Param name="_s"> int - Specifies how the window is to be shown. </param> 
/// <returns> If the function succeeds, terminating when it receives a WM_QUIT message, it should return the exit value contained in that message's wParam parameter. 
/// If the function terminates before entering the message loop, it should return zero. </returns> 
int __stdcall WinMain(HINSTANCE _P, HINSTANCE _p, LPSTR _C, int _s)
{
	PID = _P;
   
	__int64 Secnd;
	// Get's the number of tics per second.
	#define G_Secnd QueryPerformanceFrequency( ( LARGE_INTEGER * ) & Secnd );

	G_Secnd CreateMutex(0, 0, "Bank_Sim");
	if(GetLastError() == ERROR_ALREADY_EXISTS)
		Bow("Please Kill All BMSim processes,\nEven \"Hidden\" ones.");
	SystemParametersInfo(SPI_GETWORKAREA, 0, (RECT*)Scr, 0);
	// Selects the largest resonable scale
	Scr[2] -= Scr[0];
	Scr[3] -= Scr[1];
	Scale = min(Scr[2] / 640., Scr[3] / 480.);
	SysB = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	Black = CreateSolidBrush(_Blac);
	Blue = CreateSolidBrush(_Blu);
	Green = CreateSolidBrush(_Gr);
	// Sets the character dimentionsions for the Input routines
	CharW = 8 * Scale;
	CharH = Round(15 *Scale);
	Font_I = CreateFont(CharH, 0, 0, 0, 700, 0, 0, 0, 0, 0, 0, 0, FIXED_PITCH|FF_MODERN, 0);
	Font = CFont(25)Font2 = CFont(21)Font3 = CFont(18)ICon = LoadIcon(PID, "0");
	SysC = LoadCursor(0, IDC_ARROW);
	Wait_C = LoadCursor(0, IDC_APPSTARTING);
	{
		WNDCLASS Class =
		{
			0, WinProc, 0, 0, PID, ICon, SysC, SysB, 0, "WinProc"
		};
		RegisterClass(&Class);
	}
	{
		WNDCLASS Class =
		{
			CS_HREDRAW | CS_VREDRAW, WinProcI, 0, 0, PID, ICon, SysC, Blue, 0, "WinProcI"
		};
		RegisterClass(&Class);
	}
	{	
		WNDCLASS Class =
		{
			0, WinProcN, 0, 0, PID, ICon, SysC, SysB, 0, "WinProcN"
		};	
		RegisterClass(&Class);
	}
  
	fp = fopen("BMSim.TXT", "r");
	if(_In =  ! fp)
	{
		FundsGloT F;
		//Loop(MaxB)
		for(int J = 0; J < MaxB; J++)
		{
			sprintf(_T, "BK0%d.IN", (Bank = J) + 1);
			if(fp = fopen(_T, "r+"))
			break;
		}
		if( ! fp)
		{
			sprintf(_T, "To enter decisions,\n""  You need to put the IN file  ( e.g. BK01.IN )\n""  in this directory ( %s\\ ).", _getdcwd(0, Mess, 200));
			Bow(_T);
		}
		*_Com = Community = '.';
		fclose(fp);
		
		DatIO("BK00", "r", 21, &F, sizeof F);
		SimQtr = F.SimQtr;
		YrsQtr = SimQtr % 4;
		Input(F.Banner);
		exit(1);
	}
  
	CapSz = GetSystemMetrics(SM_CYCAPTION);
	{
		int I = 0;
		LnP B, P = B = (LnP)malloc(BXI_Sz = filelength(fileno(fp)));
		BXI_Sz = fread(B, 1, BXI_Sz, fp);
		fclose(fp);
		B[BXI_Sz] = 0;

//#define Ld(L) do { if(!( L.Act[I]= *P != '-')) P++; memmove( L.Nam[I],P,4); P += 5; L.Nam[I][4] = 0; I++; } while( P[-1] > 10);  L.Cnt = I; I = 0;
    
		//Ld(InstrList)
		do 
		{ 
			if(!( InstrList.Act[I]= *P != '-')) P++;
			memmove( InstrList.Nam[I],P,4);
			P += 5; 
			InstrList.Nam[I][4] = 0; 
			I++; 
		} while( P[-1] > 10);  
		InstrList.Cnt = I; 
		I = 0;
    
		//Ld(BankList)
		do 
		{ 
			if(!( BankList.Act[I]= *P != '-')) P++;
			memmove( BankList.Nam[I],P,4);
			P += 5; 
			BankList.Nam[I][4] = 0; 
			I++; 
		} while( P[-1] > 10);  
		BankList.Cnt = I; 
		I = 0;
    
		//Ld(FormList)
		do 
		{ 
			if(!( FormList.Act[I]= *P != '-')) P++;
			memmove( FormList.Nam[I],P,4);
			P += 5; 
			FormList.Nam[I][4] = 0; 
			I++; 
		} while( P[-1] > 10);  
		FormList.Cnt = I; 
		I = 0;
    
		EconSet_I =  *P++ - 48;
		Fr(B);
	}
	All_Coms(Read_All_Coms);
	if(GeSw(_C) == 2)
    UI();
	BMSim(_C);
	Bow(0);
}

// This is the Window routine for BMSim's main dialogs.
// To Debug:
//   #include "C:\BXD_Src\Debug.CPP"     // WriteWM  WriteDB  ReSet= 1;  LLL
/// <summary> This is an application-defined function that processes messages sent to a window. </summary> 
/// <returns> The return value is the result of the message processing and depends on the message sent. </returns>
long __stdcall WinProc(HWND Wnd, uint M, uint C, long C2)
{
  // WriteWM;
  if(M == WM_CLOSE)
  {
    PopOut = (long)GetMenu(*But);
    return 1;
  }
  // if(M==WM_PAINT&&Wnd==Win&&IsIconic(Win)){GetClientRect(Win,&r);
  //  DC=GetDC(Win);DrawIcon(DC,0,0,ICon);ReleaseDC(Win,DC); ValidateRect(Win,&r);return 0;}
  if(M == WM_TIMER)
  {
    static int B;
    SeTxt(PW_Win, (B^=1) ? "Busy:" : "");
  }
  if(M == WM_COMMAND && C > 0 && C < 3000)
  {
    HWND Wnd = GetFocus();
    PopOut = C;
    if(Wnd != Win)
    {
      SetFocus(Win), FLUSH_Msgs();
      Button_Event(3, 0, Wnd);
    }
  }
  if(M == WM_COMMAND && HIWORD(C) == EN_SETFOCUS && (HWND)C2 != But[Cur_B])
    Button_Event(5, 0, (HWND)C2);

  // When a new window is created, push the old one on the stack.
  if(M == WM_CREATE)
  {

    if(Win)
    {
      HWND *C = (HWND*)malloc((4+Tot_B) *4);
       *C = (HWND)_Win;
      _Win = C;
      C[1] = Win;
      Hide(Win);
      C[2] = (HWND)Tot_B;
      C[3] = (HWND)Cur_B;
      memmove(C + 4, But, Tot_B *4);
      Win = 0;
      Tot_B = 0;
    }
    Cur_B = 0;
  }
  if(M == WM_PARENTNOTIFY && (WORD)C == WM_CREATE)
  {
    int f = HIWORD(C);
    SendMessage((HWND)C2, WM_SETFONT, (long)(
    // This decides which font to use for a field.

    f < 1000 ? Font : (f < 2000 ? Font2 : Font3)), 1);
    Show((HWND)C2);
  }

  if(M == WM_KEYDOWN && C == 27 || (M == WM_KEYDOWN && C == VK_F4 && GetAsyncKeyState(VK_CONTROL)))
    C = 27;

  if(Tot_B && M == WM_KEYDOWN)
  {
    Button_Event(1, C, Wnd);
    return 1;
  }

  if(M == WM_CTLCOLORSTATIC || M == WM_CTLCOLORBTN)
  {
    // 2 Win_32_S Fixs:
    if(Tot_B && (HWND)C2 == But[Cur_B])
      Do_Flip = 1;
    SetBkColor((HDC)C, GetSysColor(COLOR_3DFACE));
    SetTextColor((HDC)C, GetSysColor(COLOR_BTNTEXT));
    return (LRESULT)SysB;
  }
  return DefWindowProc(Wnd, M, C, C2);
}


/// <summary> This is the Window routine for a  " Notice "  dialog. </summary>
/// <remarks> 
/// The function calls DefWindowProc to provide default processing for any window messages that an application does not process. 
/// This function ensures that every message is processed.
/// </remarks>
long __stdcall WinProcN(HWND Wnd, uint WM, uint wp, long lp)
{
  return DefWindowProc(Wnd, WM, wp, lp);
}


