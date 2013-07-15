#include "UI.h"
#include "UIHelper.h"

//#include "Reporting.h"

//extern int Bank, Bank_On, ComOn, UI_Up, Drive, NumBanks, NumComs, SimQtr, SimYr, YrsQtr, ComI, rvs, len;
extern int Bank, NumBanks, ComI, YrsQtr, SimQtr;
extern ComT Com[MaxComm]; 
extern long CapSz;

char *BacDir;
/// <summary> This function creates window for Managing a community after user selects The Starting Point from the setup window. </summary> 
/// <param name="cI"> is the community number. </param>
void ManageCom(long cI)
{
	extern char PQ[Qtrs][10];
	extern char FromCom, Community, _Com[10], DatFile[40], FileMode[8];
	extern char Mess[400], _T[400], _For[10];

	char s[300];

  long Run_But;
  char S[100];
  enum
  {
    bH = 25, ws = bH + 6, bW = 260
  };
  ComT *c = Com + cI;
  ActiveCom(c->Community);
  SetStateNames();

  // Creates a centered dialog window for managing a community.
  int W = 2 * bW + 3 * bLM, H = 12 * ws + 20+CapSz;
  Win = CW(0, "WinProc", "", MaS, W_H, 0, 0, PID, 0);
  But[Tot_B++] = CW(0, "BUTTON", "Exit", Chi, bLM, bTM, 85, bH, Win, (HMENU)1002, PID, 0);
  CW(0, "STATIC", "Banks:", Chi, bLM, bTM + ws, bW, bH, Win, 0, PID, 0);
  LB
  {
    Str(s, "%d", Bank + 1);
    But[Tot_B++] = CW(0, "BUTTON", s, Chi, bLM, (Bank + 2) *ws + bTM, bW, bH, Win, (HMENU)(1101+Bank), PID, 0);
  }
  But[Tot_B++] = CW(0, "BUTTON", "Administrative Input", Chi, bLM, 11 *ws + bTM, bW, bH, Win, (HMENU)1203, PID, 0);
  But[Tot_B++] = CW(0, "BUTTON", "Restoration Tools", Chi, 2 *bLM + bW, bTM, bW, bH, Win, (HMENU)1224, PID, 0);
  But[Tot_B++] = CW(0, "BUTTON", "Backup to a Flash Drive", Chi, 2 *bLM + bW, 2 *ws + bTM, bW, bH, Win, (HMENU)1207, PID, 0);
  Str(s, "Run   %s",  *PQ);
  But[Run_But = Tot_B++] = CW(0, "BUTTON", s, Chi, 2 *bLM + bW, 4 *ws + bTM, bW, 2 *bH, Win, (HMENU)204, PID, 0);
  But[Tot_B++] = CW(0, "BUTTON", "Print Instructor Reports", Chi, 2 *bLM + bW, 7 *ws + bTM, bW, bH, Win, (HMENU)1220, PID, 0);
  But[Tot_B++] = CW(0, "BUTTON", "Print Bank Reports", Chi, 2 *bLM + bW, 9 *ws + bTM, bW, bH, Win, (HMENU)1221, PID, 0);
  But[Tot_B++] = CW(0, "BUTTON", "Print Decision Forms", Chi, 2 *bLM + bW, 11 *ws + bTM, bW, bH, Win, (HMENU)1222, PID, 0);

  while(1)
  {
    ActiveCom(c->Community);
    SetStateNames();
    Str(S, "Start of %s,  Community %s",  *PQ, _Com);

    SendMessage(Win, WM_SETTEXT, 0, (DWORD)S);
    Str(s, "Run   %s",  *PQ);
    SendMessage(But[Run_But], WM_SETTEXT, 0, (DWORD)s);

    Get_Msgs();

    if(PopOut == 1002)
    {
      Back_Win();
      break;
    }

    if(PopOut == 1224)
    {
      long bH = 34, ws = bH + 8, bW = 340-2 * bLM;
      Str(s, "Start of %s, Com \"%s\"",  *PQ, _Com);

      // Creates a centered dialog window for accessing backups.
      int W = 340, H = 4 * ws + 20+CapSz;
      Win = CW(0, "WinProc", s, MaS, W_H, 0, 0, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "Exit", Chi, bLM, bTM, 85, bH, Win, (HMENU)2, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "Restore From Hard Disk", Chi, bLM, 1 *ws + bTM, bW, bH, Win, (HMENU)205, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "Restore From a Flash Drive", Chi, bLM, 2 *ws + bTM, bW, bH, Win, (HMENU)208, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "Print Other Quarters", Chi, bLM, 3 *ws + bTM, bW, bH, Win, (HMENU)206, PID, 0);

      while(1)
      {
        ActiveCom(c->Community);
        SetStateNames();
        Str(s, "Start of %s,  Com %s",  *PQ, _Com);

        SendMessage(Win, WM_SETTEXT, 0, (DWORD)s);
        Get_Msgs();
        if(PopOut == 2)
        {
          Back_Win();
          break;
        }
        if(PopOut == 208)
        {
          int bH = 34, ws = bH + 6, W = 420, H = 5 * ws + 20+CapSz;
          // Creates a centered dialog window for restoring from a diskette.
          Win = CW(0, "WinProc", "Restore From Flash Drive", MaS, W_H, 0, 0, PID, 0);
          But[Tot_B++] = CW(0, "BUTTON", "Exit", Chi, 2 *bLM + 260, 4 *ws + bTM, 85, bH, Win, (HMENU)2, PID, 0);
          CW(0, "STATIC", "     --- W A R N I N G ---\n""This will overwrite\n""current decisions and data.", Chi, bLM, bTM, 440, 3 *bH, Win, 0, PID, 0);
          CW(0, "STATIC", "Drive Letter:", Chi, bLM, 3 *ws + bTM, 150, bH, Win, 0, PID, 0);
          But[Tot_B++] = CW(0, "EDIT", "E", EdS, 2 *bLM + 150, 3 *ws + bTM, 60, bH, Win, (HMENU)100, PID, 0);
          But[Tot_B++] = CW(0, "BUTTON", "Restore From Flash Drive", Chi, bLM, 4 *ws + bTM, 275, bH, Win, (HMENU)1, PID, 0);
          Get_Msgs();
          GetDlgItemText(Win, 100, s, 3);

          if(PopOut == 1)
          {
            Str(Mess, "%s:-%s", s, _Com);
            BMSim(Mess);
          }

          Back_Win();
          continue;
        }

        if(PopOut == 205)
        {
          int bH = 25, ws = bH + 6, bW = 260;
          //Get_Qtrs
		  char Qtr[64][10];
		  long Q_I=0;
		  {
			DirA 
			if(DirI("%s\\Q?_??.",_Com))
				while(1)
				{
					if(IsDir)
						strcpy(Qtr[Q_I++],Next);
					if(Q_I==64||DirC)
						break;
				}
		  }
		  while(1)
		  {
			  long Done=1;
			  char Q[10];
			  Loop(Q_I-1)
			  {
				  if(atoi(Qtr[J]+3)*4+Qtr[J][1]<atoi(Qtr[J+1]+3)*4+Qtr[J+1][1])
				  {
					Done=0;
					strcpy(Q,Qtr[J]);
					strcpy(Qtr[J],Qtr[J+1]);
					strcpy(Qtr[J+1],Q);
				  }
			  }
				if(Done)
				break;
		  }
		  if( ! Q_I)
          {
            MessBox("No Backups are available");
            continue;
          }

          int Lns = 1, MaxBxs = 12 / Lns, Cols = (Q_I + MaxBxs - 1) / MaxBxs;
          char _Qtr[10];
          strcpy(_Qtr,  *PQ);
          _Qtr[2] = '_';
          Str(s, "Start of %s",  *PQ);
          // Creates a centered dialog window for restoring from a hard disk.
          int W = Cols * bW + (Cols + 1) *bLM, H = ((Q_I > MaxBxs ? MaxBxs : Q_I) *Lns + 2) *ws + 20+CapSz;
          Win = CW(0, "WinProc", s, MaS, W_H, 0, 0, PID, 0);
          But[Tot_B++] = CW(0, "BUTTON", "Exit", Chi, bLM, bTM, 85, bH, Win, (HMENU)1300, PID, 0);
          CW(0, "STATIC", "Click and drag the caption\n if this extends off screen.", Chi|BS_LEFT, bW + 2 * bLM, bTM, bW + 20, 2 *bH, Win, (HMENU)1301, PID, 0);
          CW(0, "STATIC", "Restore to Start of:", Chi, bLM, bTM + ws, bW, bH, Win, 0, PID, 0);

          {
            //Loop(Q_I)
			for(int J = 0; J < Q_I; J++)
            {
              long x = J / MaxBxs * bW + (J / MaxBxs + 1) *bLM, y = ws *(Lns *(J % MaxBxs) + 2) + bTM;							
			  But[Tot_B++] = CW(0, "BUTTON", Qtr[J], Chi, x, y, bW, bH, Win, (HMENU)(1000+J), PID, 0);
            }
          }

          while(1)
          {
            Get_Msgs();

            if(PopOut != 1301)
              break;
          }

          if(PopOut == 1300)
          {
            Back_Win();
            continue;
          }
		  //Copy_Qtr(Qtr[Q_I + 999 - PopOut], 1);
		  Copy_Qtr(Qtr[PopOut - 1000], 1);
		  All_Coms(Read_All_Coms);

		  //Str(s, "The Files Are Now Restored to the Start of %s", Qtr[Q_I + 999 - PopOut]);
		  Str(s, "The Files Are Now Restored to the Start of %s", Qtr[PopOut - 1000]);
          MessBox(s);
          Back_Win();

          continue;
        }

        if(PopOut == 206)
        {
          int bLM = 10, bTM = 6, bH = 17, ws = bH + 4, bW = 306, Lns = 5, MaxBxs = 21 / Lns, Cols;

          //Get_Qtrs 
		  char Qtr[64][10];
		  long Q_I=0;
		  {
			DirA 
			if(DirI("%s\\Q?_??.",_Com))
				while(1)
				{
					if(IsDir)
						strcpy(Qtr[Q_I++],Next);
					if(Q_I==64||DirC)
						break;
				}
		  }
		  while(1)
		  {
			  long Done=1;
			  char Q[10];
			  Loop(Q_I-1)
			  {
				  if(atoi(Qtr[J]+3)*4+Qtr[J][1]<atoi(Qtr[J+1]+3)*4+Qtr[J+1][1])
				  {
					Done=0;
					strcpy(Q,Qtr[J]);
					strcpy(Qtr[J],Qtr[J+1]);
					strcpy(Qtr[J+1],Q);
				  }
			  }
				if(Done)
				break;
		  }  
		  Cols = (Q_I + MaxBxs - 1) / MaxBxs;

          if(!Q_I)
		  {
            MessBox("No Reports are available");
            continue;
          }

          // Creates a centered dialog window for printing any quarter.
          int W = Cols * bW + (Cols + 1) *bLM, H = ((Q_I > MaxBxs ? MaxBxs : Q_I) *Lns + 1) *ws + 20+CapSz;

          Win = CW(0, "WinProc", "Print", MaS, W_H, 0, 0, PID, 0);

          But[Tot_B++] = CW(0, "BUTTON", "Exit", Chi, bLM, bTM, 45, bH, Win, (HMENU)2300, PID, 0);

          CW(0, "STATIC", "If you can't view this whole window, you\n   may click and drag the caption.", Chi|BS_LEFT, bW + 2 * bLM, bTM, bW, 2 *bH, Win, (HMENU)2301, PID, 0);

          {
            //Loop(Q_I)
			for(int J = 0; J < Q_I; J++)
            {
              long x = J / MaxBxs * bW + (J / MaxBxs + 1) *bLM, y = Lns *(J % MaxBxs) + 2;

#define C_B(I,M)Str(s,"%s: "M,Qtr[J]);But[Tot_B++]=  CW ( 0, "BUTTON",s,Chi|BS_LEFT,x, y++*ws+bTM,bW,bH,Win,(HMENU)(2000+J*10+I),PID,0);

              C_B(0, "Decisions")

              if(J)
              {
                C_B(1, "Instructor Reports")C_B(2, "Bank Reports")C_B(3, "End of Quarter Decision Forms")
              }
            }
          }

          while(1)
          {
            Get_Msgs();
            if(PopOut == 2301)
              continue;
            if(PopOut == 2300)
            {
              Back_Win();
              break;
            }

            Str(Mess, "%s\\%s", _Com, Qtr[(PopOut - 2000) / 10]);
            switch((PopOut - 2000) % 10)
            {
              case 0:
                {
                  char Dir[10];

                  strcpy(BacDir = Dir, Mess);

                  strcpy(s,  *PQ);
                  s[2] = '_';

                  if( ! strcmp(s, Mess + 2))
                    Dir[1] = 0;

                  int X = SimQtr;

                  SimQtr = 4 *(2000+atoi(Dir + 5) - BaseSimYear) + Dir[3] - '1';

                  YrsQtr = SimQtr % 4;

                  LB if(LoadBankDecisions(1))
                    PrnDecs(1);

                  SimQtr = X;

                  YrsQtr = SimQtr % 4;

                  BacDir = 0;

                  Str(s, "PRINT %s\\?DB??.TXT", Dir);
                  break;
                }

              case 1:
                /*InstrList*/
                PrintList(s, 0);
                break;

              case 2:
                /*BankList*/
                PrintList(s, 1);
                break;

              case 3:
                /*FormList*/
                PrintList(s, 2);
            }

            Launch(s);
          }
          continue;
        }

        if(PopOut == 211)
        {
          Copy_Qtr(*PQ, 0);
          MessBox("Decisions Saved");
          continue;
        }
      }
      continue;
    }

    if(PopOut > 1100 && PopOut < 1101+MaxB)
    {
      Bank = PopOut - 1101;

      Str(s, "%s,  Bank %d,  %s", S, Bank + 1, c->BankName[Bank]);

      Input(s);

      continue;
    }

    if(PopOut == 1203)
    {
      Str(s, "%s -A", _Com);
      BMSim(s);
      continue;
    }

	//Change -R1 to -R0 to test if the program will pick MakeDat( ). Jeff 8/1/2008
    if(PopOut == 204)
    {
      Str(s, "%s -R1 -C -B", _Com);
      BMSim(s);
      continue;
    }

    Str(Mess, "%s\\Q%c_%s", _Com, PQ[1][1], PQ[1] + 3);

    if(PopOut == 1220)
     /*InstrList*/
    {
      PrintList(s, 0);
      Launch(s);
      continue;
    }

    if(PopOut == 1221)
     /*BankList*/
    {
      PrintList(s, 1);
      Launch(s);
      continue;
    }

    if(PopOut == 1222)
     /*FormList*/
    {
      PrintList(s, 2);
      Launch(s);
      continue;
    }

    if(PopOut == 1207)
    {
      int bH = 34, ws = bH + 6;

      // Creates a centered dialog window for saving to a diskette.
      int W = 360, H = 2 * ws + 20+CapSz;

      Win = CW(0, "WinProc", "Save To Flash Drive", MaS, W_H, 0, 0, PID, 0);

      But[Tot_B++] = CW(0, "BUTTON", "Exit", Chi, 2 *bLM + 210, ws + bTM, 85, bH, Win, (HMENU)2, PID, 0);

      CW(0, "STATIC", "Drive Letter:", Chi, bLM, bTM, 150, bH, Win, 0, PID, 0);

      But[Tot_B++] = CW(0, "EDIT", "E", EdS, 2 *bLM + 150, bTM, 60, bH, Win, (HMENU)100, PID, 0);

      But[Tot_B++] = CW(0, "BUTTON", "Save To Flash Drive", Chi, bLM, ws + bTM, 210, bH, Win, (HMENU)1, PID, 0);

      Get_Msgs();

      GetDlgItemText(Win, 100, s, 3);

      if(PopOut == 1)
      {
        Str(Mess, "%s-%s:", _Com, s);
        BMSim(Mess);
      }

      Back_Win();

      continue;
    }
  }
}

/// <summary> This function runs Administration Input window from Manage Community Window.</summary>
/// <remarks> The user will be able to set Tax Rate, Bank Name, Income Less Expenses and Fines in the Administration Decision window. </remarks>
void AdminMenu()
{
	
	extern BOAT BOA[MaxB]; 
	extern EnvT Env;
	int QtrSz = sizeof(AcT);
	

  if(ComI < 0)
    return ;

  ComT *c = Com + ComI;

  char N[] = "Bank 1", TaxRate[8];

  int M = 5, HH = 36, MH = M + HH;

  DatIO("EC", "r", 5, &Env, sizeof(Env));

  Str(TaxRate, "%d", (long)Env.TaxRate);

  // Creates a centered dialog window for Administration Decisions
  int W = 720, H = 40 *(NumBanks + 1) + 80+CapSz;

  Win = CW(0, "WinProc", "Administration Decisions", MaS, W_H, 0, 0, PID, 0);

  But[Tot_B++] = CW(0, "BUTTON", "Exit", Chi, M, M, 85, HH, Win, (HMENU)2, PID, 0);

  But[Tot_B++] = CW(0, "BUTTON", "OK", Chi, 3 *M + 85, M, 58, HH, Win, (HMENU)1, PID, 0);

  CW(0, "STATIC", "Tax Rate(%):", Chi, 6 *M + 85+58, M, 140, HH, Win, 0, PID, 0);

  But[Tot_B++] = CW(0, "EDIT", TaxRate, WS_THICKFRAME|Chi|ES_NUMBER, 8 *M + 85+58+140, M, 60, HH, Win, (HMENU)101, PID, 0);

  CW(0, "STATIC", "Bank Names", Chi, 200, 70, 150, 30, Win, 0, PID, 0);

  CW(0, "STATIC", "Income Less Expenses and Fines (Thousands-$)", Chi, 480, M, 240, 90, Win, 0, PID, 0);

  LB
  {
    long id = 111+Bank * 10;

    long Y = 105+40 * Bank, y = Y;

    N[5] = Bank + '1';

    CW(0, "STATIC", N, Chi, 8, Y, 90, 30, Win, 0, PID, 0);

    But[Tot_B++] = CW(0, "EDIT", c->BankName[Bank], EdS, 95, y, 370, 34, Win, (HMENU)id, PID, 0);
    {
      AcT *Ac = BOA[Bank] + YrsQtr;

      IncomeOutGoT *Incm = &Ac->IncomeOutGo;

      long n;

      DatIO("BK00", "r", YrsQtr, Ac, QtrSz);

      n = Round((Incm->Fees.Other - Incm->Expenses.OtherOpEx) *1000.);
      {
        char b[8];
        Str(b, "%d", n);

        But[Tot_B++] = CW(0, "EDIT", b, EdS, 520, y, 83, 34, Win, (HMENU)(id + 1), PID, 0);
      }
    }
  }

  while(1)
  {
    char b[40];

    long Tax, All_Coms_Dirty = 0;

    Get_Msgs();

    if(PopOut == 2)
    {
      Back_Win();
      return ;
    }

    if(PopOut != 1)
      continue;

    GetDlgItemText(Win, 101, b, 4);

    Tax = abs(atoi(b)) % 100;

    if(strlen(b) > 2 ||  ! isdigit(*b))
    {
      MessBox("Tax Must be From: 0 to 99");
      continue;
    }

    if(Tax != Env.TaxRate)
    {
      Env.TaxRate = Tax;
      DatIO("EC", "r+", 5, &Env, sizeof(Env));
    }

    LB
    {
      long id = 111+Bank * 10;

      AcT *Ac = BOA[Bank] + YrsQtr;

      IncomeOutGoT *Incm = &Ac->IncomeOutGo;

      GetDlgItemText(Win, id, b, 40);

      b[39] = 0;

      if(strcmp(b, c->BankName[Bank]))

      {
        All_Coms_Dirty = 1;
        strcpy(c->BankName[Bank], b);
      }
      {
        char b[8];

        float t;

        GetDlgItemText(Win, id + 1, b, 8);

        t = (float)atoi(b) / 1000;

        Incm->Expenses.OtherOpEx = Incm->Fees.Other = 0;

        if(t < 0)
          Incm->Expenses.OtherOpEx =  - t;

        if(t > 0)
          Incm->Fees.Other = t;

        DatIO("BK00", "r+", YrsQtr, Ac, QtrSz);
      }
    }

    if(All_Coms_Dirty)
      All_Coms(Update_All_Coms);

    Back_Win();

    return ;
  } //end while(1)
}