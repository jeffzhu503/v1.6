#include "UI.h"
#include "UIHelper.h"
#include "Reporting.h"

extern long CapSz; 

/// <summary> This function draws a window for selecting reports from an existing list. </summary> 
/// <param name="*P"> is the list to be selected.  </param>
/// <param name="*M"> is the title of this window. For example "Instructor Reports". </param> 
void SelList(ListT *P, char *M)
{
	extern long Keyed; // True is a key was pressed, used in Button_Event

  int C = P->Cnt;
  enum
  {
    WW = 102, CharH = 25, Rows = 8
  };

  // Creates a centered dialog window for selecting from a list.
  int W = 45+WW * max(3, ((C + Rows - 1) / Rows)), H = CapSz + 70+(CharH + 10) *Rows;
  Win = CW(0, "WinProc", M, MaS, W_H, 0, 0, PID, 0);
  But[Tot_B++] = CW(0, "BUTTON", "Cancel", Chi, 10, 10, 95, 30, Win, (HMENU)2, PID, 0);
  But[Tot_B++] = CW(0, "BUTTON", "OK", Chi, 130, 10, 60, 30, Win, (HMENU)1, PID, 0);
  {
    //Loop(C)
	for(int J = 0; J < C; J++)
    {
      But[Tot_B++] = CW(0, "BUTTON", P->Nam[J], BS_AUTOCHECKBOX|Chi, 35+WW *(J / Rows), 70+(CharH + 10)*(J % Rows), WW, CharH, Win, (HMENU)(100+J), PID, 0);
      CheckDlgButton(Win, 100+J, P->Act[J]);
    }
  }

  while(1)
  {
    Get_Msgs();
    if(PopOut == 2)
      break;

    if(PopOut != 1)
    {
      rv = IsDlgButtonChecked(Win, PopOut);
      if(Keyed && rv < 2)
        CheckDlgButton(Win, PopOut,  ! rv);
      continue;
    }
    {
      Loop(C)P->Act[J] = IsDlgButtonChecked(Win, 100+J);
    }

    WrTXT();
    break;
  }
  Back_Win();
}

/// <summary> This function runs Economy Sets window by clicking "Economy" button on Setup Menu. </summary>
/// <remarks> The user will be able to select the Seconomy Set, choose to print the current or all 3 Economy Set in sevarl quarters back. </remarks>
void EconMenu()  //Move to UI folder
{
	extern EnvT Env, LocEnv; 
	extern SecKT SecK; 
	extern EconT EconList[10]; 
	extern ECNT EconBOQ, EconEOQ; 
	extern EconQtrT EconQtr8[8]; 
	extern PricesT Prices; 

  if(ComI < 0)
    return ;

  int All3, M = 15, HH = 36, WW = 370, MH = M + HH, QtrsAhead, OldSimQtr = SimQtr;

  // Creates a centered dialog window for selecting an economy set.
  int W = 410, H = 2 * M + 6 * MH + 20+CapSz;

  Win = CW(0, "WinProc", "Economy Sets", MaS, W_H, 0, 0, PID, 0);

  But[Tot_B++] = CW(0, "BUTTON", "Exit", Chi, M, M, 85, HH, Win, (HMENU)2, PID, 0);

  CW(0, "STATIC", "Use Economy Set 1, 2, or 3 ?:", Chi, M, M + MH, 340, HH, Win, 0, PID, 0);

  DatIO("EC", "r", 5, &Env, sizeof(Env));

  Str(Mess, "%d", EconSet_I);

  But[Tot_B++] = CW(0, "EDIT", Mess, EdS, 2 *M + 300, M + MH, 55, HH, Win, (HMENU)104, PID, 0);

  But[Tot_B++] = CW(0, "BUTTON", "Change Economy Set", Chi, M, M + 2 * MH, WW, HH, Win, (HMENU)1, PID, 0);

  But[Tot_B++] = CW(0, "BUTTON", "Print all 3 Economy Sets", Chi, M, M + 3 * MH, WW, HH, Win, (HMENU)102, PID, 0);

  But[Tot_B++] = CW(0, "BUTTON", "Print the Current Economy Set", Chi, M, M + 4 * MH, WW, HH, Win, (HMENU)103, PID, 0);

  CW(0, "STATIC", "Print Ahead", Chi, M, M + 5 * MH, 150, HH, Win, 0, PID, 0);

  But[Tot_B++] = CW(0, "EDIT", "4", EdS, 2 *M + 140, M + 5 * MH, 60, HH, Win, (HMENU)101, PID, 0);

  CW(0, "STATIC", "Quarters.", Chi, 4 *M + 180, M + 5 * MH, 110, HH, Win, 0, PID, 0);

  while(1)
  {
    long SimQ;

    Get_Msgs();

    if(PopOut == 2)
    {
      Back_Win();
      return ;
    }

    if(PopOut == 1)
    {
      long EconSet;

      GetDlgItemText(Win, 104, Mess, 2);

      EconSet = atoi(Mess);

      if(EconSet < 1 || EconSet > 3)
      {
        MessBox("Enter a Digit From 1 to 3.");
        continue;
      }

      EconSet_I = EconSet;

      WrTXT();

      Str(Mess, "Economy Set changed to '%d'", EconSet);

      MessBox(Mess);

      continue;
    }

    if(PopOut != 102 && PopOut != 103)
      continue;

    GetDlgItemText(Win, 101, Mess, 4);

    QtrsAhead = atoi(Mess);

    if(QtrsAhead <= 0 || QtrsAhead > 16)
    {
      MessBox("Must be From 1 to 16 Quarters.");
      continue;
    }

    PW_S("Creating the Economy Reports");

    All3 = PopOut == 102;

    {
      DirA if(DirI("%s\\B???.txt", _Com))
      while(1)
      {
        Str(_T, "%s\\%s", _Com, Next);
        unlink(_T);
        if(DirC)
          break;
      }
    }

    DatIO("SF", "r", 15, &SecK, sizeof(SecK));
    DatIO("EC", "r", 0, &EconList, sizeof(EconList));
    {
      Loop(All3 ? 3 : 1)
      {
        YrsQtr = (SimQtr = OldSimQtr) % 4;
        SimYr = BaseSimYear + SimQtr / 4;
        Com[ComI].SimQtr = SimQtr;

        DatIO("EC", "r", 5, &Env, sizeof(Env));
        DatIO("EC", "r", 6, &EconBOQ, sizeof(EconBOQ));
        DatIO("EC", "r", 7, &EconEOQ, sizeof(EconEOQ));
        DatIO("EC", "r", 8, EconQtr8, sizeof(EconQtr8));
        DatIO("EC", "r", 18, &Prices, sizeof(Prices));

        if(All3)
          EconSet_I = J + 1;
        Page = 0;
        {
          //Loop(QtrsAhead)
		  for(int J = 0; J < QtrsAhead; J++)
          {
            SimQ = SimQtr;
            {
              Loop(5)
              {
                L4[J] = SimQ % 4+4 *(SimQ / 4 != SimQtr / 4);
                SimQ--;
              }
            }
            SetStateNames();

            Economy();

            EconPreview = 1;

            Str(EndOf, "End of Quarter %s",  *PQ + 1);

            B900("B900", "Economic Report Summary", 56, EndOf);

            B901("B901", "Economic History and Competitive Rates", 70, EndOf);

            EconPreview = 0;
            YrsQtr = ++SimQtr % 4;
            SimYr = BaseSimYear + SimQtr / 4;

            Com[ComI].SimQtr = SimQtr;
          }
        }
      }
    }
    Com[ComI].SimQtr = OldSimQtr;

    PW_E();
    {
      Str(Mess, "PRINT %s\\B???.txt", _Com);
      Launch(Mess);
    }
  } //while(1)
}

// This function draw Setup Menu after user clicks the button of Setup from the Main Menu
// SP_I is the index of S community in the folder
void SetupMenu(int SP_I)
{
	extern ListT InstrList, BankList, FormList;
	int bH = 34, ws = bH + 8, bW = 300, Wid = 540; 
	char s[300];
	extern int Coms;

      int W = bW + 2 * bLM, H = 9 * ws + 20+CapSz;

      // Creates a centered dialog window for Setup
      Win = CW(0, "WinProc", "Setup", MaS, W_H, 0, 0, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", NumComs > 1 ? "Main Menu" : "Exit", Chi, bLM, bTM, bW, bH, Win, (HMENU)2, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "The Starting Point", Chi, bLM, 1 *ws + bTM, bW, bH, Win, (HMENU)210, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "The Economy", Chi, bLM, 2 *ws + bTM, bW, bH, Win, (HMENU)209, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "Select Instructor Reports", Chi, bLM, 3 *ws + bTM, bW, bH, Win, (HMENU)220, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "Select Bank Reports", Chi, bLM, 4 *ws + bTM, bW, bH, Win, (HMENU)221, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "Select Forms && Worksheets", Chi, bLM, 5 *ws + bTM, bW, bH, Win, (HMENU)222, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "Create a New Community", Chi, bLM, 6 *ws + bTM, bW, bH, Win, (HMENU)200, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "Delete a Community", Chi, bLM, 7 *ws + bTM, bW, bH, Win, (HMENU)201, PID, 0);
      But[Tot_B++] = CW(0, "BUTTON", "Economy Management System", Chi, bLM, 8 *ws + bTM, bW, bH, Win, (HMENU)230, PID, 0);

      while(1)
      {
        SeTxt(*But, NumComs > 1 ? "Main Menu" : "Exit");

        Get_Msgs();
		if(PopOut == 230)
		{
			//DrawEconManagementMain(); To Do: add this function back
		}
        if(PopOut == 2)
        {
          if(NumComs <= 1)
            exit(1);
		  //The following condition shall not be true. 
         //if( ! Coms)
            //goto L01;
          Back_Win();
          break;
        }
        if(PopOut == 210)
        {
          ManageCom(SP_I);
          continue;
        }
        if(PopOut == 209)
        {
          BMSim("S -E");
          continue;
        }
        if(PopOut == 220)
        {
          SelList(&InstrList, "Instructor Reports");
          continue;
        }
        if(PopOut == 221)
        {
          SelList(&BankList, "Bank Reports");
          continue;
        }
        if(PopOut == 222)
        {
          SelList(&FormList, "Decision Forms");
          continue;
        }
        if(PopOut == 200)
        {
          int W = 564, H = 5 * ws + 20+CapSz;

          // Creates a centered dialog window for adding communities
          Win = CW(0, "WinProc", "Create a New Community", MaS, W_H, 0, 0, PID, 0);
          But[Tot_B++] = CW(0, "BUTTON", "Exit", Chi, bLM, bTM, 85, bH, Win, (HMENU)2, PID, 0);
          But[Tot_B++] = CW(0, "BUTTON", "Create", Chi, bLM + 100, bTM, 80, bH, Win, (HMENU)1, PID, 0);
          CW(0, "STATIC", "Source Community Letter:\n  Note: Community \"S\" is the \"Starting Point\".", Chi, bLM, ws + bTM, 600, bH *2, Win, 0, PID, 0);
          But[Tot_B++] = CW(0, "EDIT", "S", EdS, 2 *bLM + 270,  - 10+ws + bTM, 60, bH, Win, (HMENU)100, PID, 0);
          CW(0, "STATIC", "Destination Community Letter:", Chi, bLM, 3 *ws + bTM, 350, bH, Win, 0, PID, 0);
          But[Tot_B++] = CW(0, "EDIT", "", EdS, 2 *bLM + 330, 3 *ws + bTM, 60, bH, Win, (HMENU)101, PID, 0);
          CW(0, "STATIC", "Number of Banks:", Chi, bLM, 4 *ws + bTM, 520, 10+3 * bH, Win, 0, PID, 0);
          But[Tot_B++] = CW(0, "EDIT", "", EdS, 2 *bLM + 200,  - 10+4 * ws + bTM, 60, bH, Win, (HMENU)102, PID, 0);

          while(1)
          {
            Get_Msgs();
            GetDlgItemText(Win, 100, s + 9, 3);
            GetDlgItemText(Win, 101, s, 3);
            GetDlgItemText(Win, 102, Mess, 3);

            if(PopOut == 2)
            {
              Back_Win();
              break;
            }

            rv = atoi(Mess);
            if( ! isalnum(*s) ||  ! isalnum(s[9]) ||  *Mess != '*' &&  ! (rv > 0 && rv <= MaxB))
            {
              MessBox("The Number of Banks must be either an  *  or\n     a Number From 1 to 6.\n\n""The Destination Community Must be an Alpha or Numeric.");
              continue;
            }
            if(rv)
              Str(Mess, "%c-%c-%d", s[9],  *s, rv);
            else
              Str(Mess, "%c-%c", s[9],  *s);

            BMSim(Mess);

            All_Coms(Read_All_Coms);
            Back_Win();
            break;
          }
          continue;
        }

        if(PopOut == 201)
        {
          ListT Comms;
          Comms.Cnt = NumComs - 1;
          {
            long I =  - 1;
            //Loop(NumComs)
			for(int J = 0; J < NumComs; J++)
            {
              char C = Com[J].Community;
              if(C == 'S')
                continue;
              I++;

              Comms.Act[I] = Comms.Nam[I][1] = 0;

              Comms.Nam[I][0] = C;

              {
                //Loop(I)
				for(int J = 0; J < I; J++)
                {
                  if(Comms.Nam[J][0] > C)
                  {
                    long K = J, L = I;
                    {
                      Loop(I - K)Comms.Nam[L--][0] = Comms.Nam[L - 1][0];
                    }

                    Comms.Nam[K][0] = C;
                    break;
                  }
                }
              }
            }
          }

          SelList(&Comms, "Delete Communities");
          {
            //Loop(Comms.Cnt)
			for(int J = 0; J < Comms.Cnt; J++)
            {
              if( ! Comms.Act[J] || Comms.Nam[J][0] < '0')
                continue;

              strcpy(s, Comms.Nam[J]);

              Str(Mess, "%s\\SF.DAT", s);

              if( ! (fp = fopen(Mess, "r")))
              {
                Str(Mess, "Community %s Doesn't Exist.", s);

                MessBox(Mess);
                continue;
              }
              else
              {
                DirA fclose(fp);
                PW_S("Deleting");

                if(DirI("%s\\*.*", s))
                while(1)
                {
                  char Sub[400];
                  Str(Sub, "%s\\%s", s, Next);
                  if( ! IsDir)
                    unlink(Sub);

                  else
                  {
                    DirA if(DirI("%s\\*.*", Sub))
                    while(1)
                    {
                      Str(_T, "%s\\%s", Sub, Next);
                      unlink(_T);
                      if(DirC)
                        break;
                    }
                    rmdir(Sub);
                  }

                  if(DirC)
                    break;
                }

                rmdir(s);
                All_Coms(Read_All_Coms);
                PW_E();
                continue;
              }
            }
          }
          continue;
        }
      }
}

