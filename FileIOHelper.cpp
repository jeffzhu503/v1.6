#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <io.h>
#include <sys\stat.h>
#include "FileIOHelper.h"
#include "GlobalDef.h"
#include "OleHelpers.h"
#include "UIHelper.h"
#include "BMSimHelper.h"

#include <Windows.h> 
#include <fstream>
#include <iostream>

using namespace std;

extern int Bank, Bank_On, ComOn, UI_Up, Drive, NumBanks, NumComs, SimQtr, SimYr, YrsQtr, ComI, rvs, len;
WORD OldBuffSize, BuffSize;
extern char FromCom, Community, _Com[10], DatFile[40], FileMode[8];
extern char *BacDir;

extern FILE *fp;
extern FILE *Out;
extern  float Version, *BuffAddress;
extern DWORD NumLen, DotLen, DiskAddress;
extern int _In;
 
extern long rv, EconSet_I;
extern char Mess[400], _T[400], _For[10];

/// <summary> This function returns details about a file I/O error. </summary>
/// <param name="ByteAddr"> is the disk address in the file. </param>
/// <returns> the error message. </returns> 
/// <remarks> It is only used in fatalRW2(). </remarks> 
char *SecAndField(DWORD ByteAddr)
{
  long Field = ((ByteAddr % 1024) / 4);
  static char FourMess[4][20];
  static long i;
  char *Mess;
  i = ++i % 4;
  Mess = FourMess[i];
  Str(Mess, "%3ld", ByteAddr / 1024);
  if(Field)
    Str(strchr(Mess, 0), ":%3d ", Field);
  else
    strcat(Mess, "     ");
  return Mess;
}

/// <summary> This function writes error message if BmSim fails to read or write from the dat file.  </summary> 
/// <param name="*m"> is a string, which can be "Read at", "Write at" or "Open" </param>
void fatalRW1(char *m)
{
  char buff[200];
  Str(buff, "Can NOT %s [%s]", m, DatFile);
  Bow(buff);
}

/// <summary> Write error message if it fails to read or write from the dat file.  </summary> 
/// <param name="*m">is a string, which can be "Read at" or "Write at". </param>
void fatalRW2(char *m)
{
  char M[200];
  Str(M, "%s %s in", m, SecAndField(DiskAddress));
  fatalRW1(M);
}

/// <summary> This function opens a file for sector reads/writes or updates. </summary>
/// <param name="*Name8"> is file Name. </param>
/// <param name="*Ext"> is file Extension Name. </param>
/// <param name="*FMode"> is file Mode. </param>
/// <returns> 0 if success.  Otherwise output error message from _sys_errlist[errno]. </returns> 
/// <remarks> This function decides the file name based on the bank.  For example, bk01 is for the first bank. </remarks>
long ODF(char *Name8, char *Ext, char *FMode)
{
  char FileName[18];
  strcpy(FileName, Name8);
  strupr(FileName);
  if(*FileName == '0')
  {
    char T = FileName[1];
     *FileName += Bank + 1;
    if(T == 'W')
      FileName[1] = 'D';
    if(T == 'C')
      FileName[1] = 'B';
  }
  else if(isalpha(FileName[1]) &&  ! strcmp(FileName + 2, "00"))
	{
    FileName[3] = 49+Bank;  //49 is acsii "0"
	}
  {
    char ComDir[10];
    if( ! Drive)
    {
       *ComDir = Community;
      ComDir[1] = 0;
      if(BacDir)
        strcpy(ComDir, BacDir);
    }
    else
      Str(ComDir, "%c:\\%s", Drive, _Com);
    Str(DatFile, "%s\\%s.%s", ComDir, FileName, Ext);
    strcpy(FileMode, FMode);
    if(stricmp(Ext, "TXT"))
      strcat(FileMode, "b");
    fp = fopen(DatFile, FileMode);
    if( ! fp &&  ! mkdir(ComDir))
      fp = fopen(DatFile, FileMode);
  }
  if( ! fp)
  {
    char Err[40];
    if(_In || BacDir)
      return 1;
    Str(Err, "Open (%s) (%s)", FileMode, _sys_errlist[errno]);
    fatalRW1(Err);
  }
  return 0;
}

/// <summary> This function reads or writes a secctor from/into a data file. </summary>
/// <param name="*Name8"> is the name of data file.</param>
/// <param name="*Mode"> is Read or Write mode. </param>
/// <param name="Sector"> is the number of sector (each has 1024 WORD) in the data file, which may be based on the number of decision form. </param>
/// <param name="*RamAddress"> is the pointer to the data structure. </param>
/// <param name="ByteCnt"> is the size of the data structures </param> 
void DatIO(char *Name8, char *Mode, float Sector, void *RamAddress, DWORD ByteCnt)
{
  OldBuffSize = BuffSize;
  BuffSize = ByteCnt;
  BuffAddress = (FeP)RamAddress;
  ODF(Name8, _In ? "IN" : "DAT", Mode);
  if( ! fp && (_In || BacDir))
  {
    memset(RamAddress, 0, ByteCnt);
    return ;
  }
  fseek(fp, DiskAddress = Sector < 0 ? DiskAddress + OldBuffSize: (DWORD)(Sector *1024), 0);
  if( ! stricmp(Mode, "r"))
  {
    rv = fread(RamAddress, 1, ByteCnt, fp);
    if(rv <= 0 || fclose(fp))
      fatalRW2("Read at");
  }
  else
  {
    rv = fwrite(RamAddress, 1, ByteCnt, fp);
    if(rv <= 0 || fclose(fp))


      fatalRW2("Write at");
  }
}

/// <summary> This function closes the file Out. </summary>
void CloseOut()
{
  if(Out)
  {
    fclose(Out);
    Out = 0;
  }
}

/// <summary> This function copies a file, turning off the Read-only attribute </summary>
/// <param name="*Src"> is the source file. </param>
/// <param name="*Dst"> is the destination file. </param>
void Cop(char *Src, char *Dst)
{
  CopyFile(Src, Dst, 0);
  SetFileAttributes(Dst, FILE_ATTRIBUTE_NORMAL);
}

/// <summary> This function initializes a directory search. </summary>
/// <param name="*F"> defines the criteria to search the file. Wildcard is supported e.g. ? is to search if the directory exists </param>
/// <param name="*S"> has the name of directory where to do the file search </param>
/// <param name="*FF_Rec"> defines WIN32_FIND_DATA structure describing a file found by the FindFirstFile, FindFirstFileEx, or FindNextFile function  </param>
/// <param name="*FF_List"> is window handle to the file list  </param>
/// <returns> The function returns 1, if the file or subfolder is found.  Otherwise it returns 0 </returns> 
/// <exception name="INVALID_HANDLE_VALUE"> INVALID_HANDLE_VALUE if none of file is found based on the search criteria </exception>
long _DirI(char *F, char *S, WIN32_FIND_DATA *FF_Rec, HANDLE *FF_List)
{
  char *p, m[400], _T[400];

  Str(_T, F, S);

  p = _T;

  while(p = strchr(p, '\\'))
    if(*++p == '\\')
      memmove(p - 1, p, strlen(p) + 1);

  strcpy(m, _T);

  if(p = strchr(m, '?'))
     *p = 0;

  if(p = strchr(m, '*'))
     *p = 0;

  if( ! (p = strrchr(m, '\\')) || p-- == m || (*p-- == ':' && toupper(*p) > 'B'))
  {
    Str(Mess, "Tried to Search a Fixed-Disk's Root: \"%s\"", _T);
    Bow(Mess);
  }

  if((*FF_List = FindFirstFile(_T, FF_Rec)) == INVALID_HANDLE_VALUE)
  {

    rv = GetLastError();

    if(rv <= 3)
      return 0;

    Str(m, "Error %d Searching for >%s<", rv, _T);
    MessBox(m);
    return 0;
  }

  while(1)
  {
    if(*(*FF_Rec).cFileName != '.')
    {
      return 1;
    }
    if( ! FindNextFile(*FF_List, FF_Rec))
    {
      FindClose(*FF_List);
      return 0;
    }
  }
}

/// <summary> The function verifies if Handle FF_List reaches the end of the list </summary>
/// <param name="*FF_Rec"> defines WIN32_FIND_DATA structure describing a file found by the FindFirstFile, FindFirstFileEx, or FindNextFile function  </param>
/// <param name="*FF_List"> is window handle to the file list  </param>
/// <returns> The function returns 1, if no more file or subfolder is found.  Otherwise it returns 0 </returns> 
long _DirC(WIN32_FIND_DATA *FF_Rec, HANDLE FF_List)
	{
  while(1)
  {
    if( ! FindNextFile(FF_List, FF_Rec))
    {
      FindClose(FF_List);
      return 1;
    }
    if(*(*FF_Rec).cFileName != '.')
    {
      return 0;
    }
  }
}

extern long FromDrive, ToDrive;

/// <summary> This function reads the data from the file to the memory structure.  </summary> 
/// <param name="*SrcN"> is the data file with the extension "DAT". </param> 
void Copy(char *SrcN)
{
  if(FromDrive)
    Drive = FromDrive;
  ODF(SrcN, "DAT", "r");
  {
    int bSecs = 30, bSz = bSecs * RECSIZE, Sec = 0;
    LnP b = (LnP)malloc(bSz);
    int sSz = filelength(fileno(fp));
    fclose(fp);
    while(sSz)
    {
      DWORD Chnk = sSz > bSz ? bSz : sSz; //if the file is large than 30, use 30 as the sector unit. 
      sSz -= Chnk;
      if(FromDrive)
        Drive = FromDrive;
      DatIO(SrcN, "r", (WORD)Sec, b, (WORD)Chnk);
      Drive = 0;
      if(ToDrive)
        Drive = ToDrive;
      DatIO(SrcN, Sec ? "r+" : "w", (WORD)Sec, b, (WORD)Chnk);
      Drive = 0;
      Sec += bSecs;
    }
    Fr(b);
  }
}  

/// <summary> This function copies all fires from the source folder in the hard drive to the destination in the flash drive. </summary>
/// <param name="fileName"> takes the regular expression. </param>
void CopyFiles(char* fileName, char* sourceFolder, char* destinationFolder)
{
	WIN32_FIND_DATA FindFileData;
	HRESULT hr; 
	
	char searchFolder[50];
	char sourceFile[50]; 
	char destinationFile[50]; 
		
	strcpy(searchFolder, sourceFolder);
	strcat(searchFolder, fileName); 
	HANDLE hFind = FindFirstFile(searchFolder, &FindFileData); 

	if(hFind == INVALID_HANDLE_VALUE)
	{
		//MessBox("Did not find file."); 
		return;
	}

	do 
	{
		strcpy(sourceFile, sourceFolder); 
		strcat(sourceFile, FindFileData.cFileName); 
		strcpy(destinationFile, destinationFolder);
		strcat(destinationFile, FindFileData.cFileName); 
		CopyFileEx(sourceFile, destinationFile, NULL, NULL, NULL, COPY_FILE_NO_BUFFERING);

		hr = GetLastError(); 
	}
	while(FindNextFile(hFind, &FindFileData) != 0); 
}

void CopyE990(/*char* folder*/)
{
	WIN32_FIND_DATA FindFileData;
	HRESULT hr; 
 
	char searchComFolder[50];
	char searchComFile[50];
	Str(searchComFolder, "%s\\", _Com);
	Str(searchComFile, "%s\\", _Com);
	char searchFile[50] = "*I990.txt"; 
	char sourceFile[50]; 
	char destinationFile[50]; 
		
	strcat(searchComFile, searchFile); 
	HANDLE hFind = FindFirstFile(searchComFile, &FindFileData); 

	if(hFind == INVALID_HANDLE_VALUE)
	{
		//MessBox("Did not find file."); 
		return;
	}

	do 
	{
		strcpy(sourceFile, searchComFolder); 
		strcat(sourceFile, FindFileData.cFileName); 
		strcpy(destinationFile, searchComFolder);
		strcat(destinationFile, FindFileData.cFileName);
		destinationFile[3] = 'B'; 
		CopyFileEx(sourceFile, destinationFile, NULL, NULL, NULL, COPY_FILE_NO_BUFFERING);

		hr = GetLastError(); 
	}
	while(FindNextFile(hFind, &FindFileData) != 0); 
}


/// <summary> This function copies all data files of community files and current quarter folder from/to a backup driver. </summary> 
/// <param name="*c"> is the community name. </param> 
/// 2013 we decide to backup all of reporting txt files as well
void CopyComDrv(ComT *c)
{
	//2013 For debugging purpose, comment the following message to the screen. 
	PW_S(FromDrive ? "Copying Community - From - Diskette" : "Copying Community - To - External Drive");
	{
    //Loop(c->NumBanks)
		for(int J = 0; J < NumBanks; J++)
		{
			char Buf[16];
			long o = J + 1;

			Str(Buf, "BK0%d", o);
			Copy(Buf);
			Str(Buf, "LN0%d", o);
			Copy(Buf);
		}
	}
	Copy("SF");
	Copy("EC");
	if(FromDrive) 
		Drive = FromDrive;
	ODF("QTR", "TXT", "r");
	{
		int S = filelength(fileno(fp));
		LnP b = (LnP)malloc(S);
		S = fread(b, 1, S, fp);
		fclose(fp);
		Drive = 0;
		if(ToDrive)
			Drive = ToDrive;
		ODF("QTR", "TXT", "w");
		fwrite(b, 1, S, fp);
		fclose(fp);
		Drive = 0;
		Fr(b);
	}

	char sourceComFolder[20]; 
	char destinationComFolder[20]; 
	char sourceQtrFolder[20]; // "A\\Q1_17\\";
	char destinationQtrFolder[20];

	//Copy files under the community folder to the flash driver
	if(FromDrive == 0 && ToDrive != 0)
	{
		Str(sourceComFolder, "%s\\", _Com); 
		Str(destinationComFolder, "%c:\\%s\\", ToDrive, _Com);

		Str(sourceQtrFolder, "%s\\Q%d_%02d\\", _Com, YrsQtr + 1, SimYr - 2000); 
		Str(destinationQtrFolder, "%c:\\%s\\Q%d_%02d\\", ToDrive, _Com, YrsQtr + 1, SimYr - 2000); 
	}
	else
	{
		Str(sourceComFolder, "%c:\\%s\\", FromDrive, _Com);
		Str(destinationComFolder, "%s\\", _Com); 

		Str(sourceQtrFolder, "%c:\\%s\\Q%d_%02d\\", FromDrive, _Com, YrsQtr + 1, SimYr - 2000); 
		Str(destinationQtrFolder, "%s\\Q%d_%02d\\", _Com, YrsQtr + 1, SimYr - 2000); 
	}
		
	CopyFiles("*IN", sourceComFolder, destinationComFolder); 
	CopyFiles("*csv", sourceComFolder, destinationComFolder); 
	CopyFiles("*D*.txt", sourceComFolder, destinationComFolder); 
	CopyFiles("L*.txt", sourceComFolder, destinationComFolder); 
	CopyFiles("S.txt", sourceComFolder, destinationComFolder); 

	// Create the new Qtr directory in the extneral drive
		
	HRESULT hr = CreateDirectory(destinationQtrFolder, NULL);
			
	if(hr ==  ERROR_PATH_NOT_FOUND) 
	{
		MessBox("The destination path is not found"); 
		return; 
	}

	if(hr ==  ERROR_ALREADY_EXISTS) 
	{
		MessBox("The current quarter folder already exists in the destination"); 
	}

	CopyFiles("BK*", sourceQtrFolder, destinationQtrFolder); 
	CopyFiles("*DAT", sourceQtrFolder, destinationQtrFolder); 
	CopyFiles("*.TXT", sourceQtrFolder, destinationQtrFolder); 
	CopyFiles("*IN", sourceQtrFolder, destinationQtrFolder); 
	CopyFiles("*csv", sourceQtrFolder, destinationQtrFolder); 
	//CopyFiles("*D*.txt", sourceQtrFolder, destinationQtrFolder); 
	//CopyFiles("L*.txt", sourceQtrFolder, destinationQtrFolder); 
	//CopyFiles("S.txt", sourceQtrFolder, destinationQtrFolder); 
	
	PW_E();
}



/// <summary> This function backup all of input files such as .dat or .in files.   </summary>
/// <param name="*_Bac"> is the array of past 4 quarters.  The first element is the current quarter. </param>
/// <param name="Rest"> copies from community folder to quarter folder if it is 0, otherwise it copies from quarter folder to community folder. </param>
void Copy_Qtr(char *_Bac, int Rest)
{
	extern long FromInstall;

  if( ! UI_Up &&  ! FromInstall)
    return ;
  char __Bac[66], Bac[66], SrcNam[66], DestNam[66];
  strcpy(__Bac, _Bac);
  if(__Bac[2] == '/')
    __Bac[2] = '_';
  Str(Bac, "%c\\%s", Community, __Bac);
  mkdir(Bac);
  char *S = Rest ? Bac : _Com,  *D = Rest ? _Com : Bac;
  Str(SrcNam, "%s\\QTR.TXT", S);
  Str(DestNam, "%s\\QTR.TXT", D);
  Cop(SrcNam, DestNam);
  DirA Loop(2)if(DirI(J ? "%s\\*.DAT" : "%s\\*.IN", S))
  do
  {
    Str(SrcNam, "%s\\%s", S, Next);
    Str(DestNam, "%s\\%s", D, Next);
    Cop(SrcNam, DestNam);
  }
  while( ! DirC);
}

/// <summary> This function writes loan information to the output data file and counts the loan number. </summary>
/// <remarks> It writes pLoans[Bank] to LN00.dat file. </remarks> 
void CloseLoans()
{
	typedef LoanT *pLoansT[MaxL];
	extern LoanGloT LoanGlo[MaxB]; 
	extern pLoansT pLoans[MaxB];
	extern int LnSz; 

  long TotCnt = 0;
  LoanGloT *r = LoanGlo + Bank;
  ODF("LN00", "DAT", "w");
  {
    //Loop(MaxL)
	for(int J = 0; J < MaxL; J++)
    {
      long t = J, c = (long)r->LnCnt[t];
      LoanT *B = pLoans[Bank][t],  *p = B - 1;
      {
        long C = 0;
        Loop(c)if((++p)->Type)
        {
          C++;
          fwrite(p, LnSz, 1, fp);
        }
        r->LnCnt[t] = C;
        TotCnt += C;
      }
      if(B)
        Fr(B);
    }
  }
  fclose(fp);
}

/// <summary> This function copies all of forms community folder to the quarter folder. </summary>
/// <param name="*_Bac"> is the quarter name.  For exampe "Q4/09". </param>
void CopyForms(char *_Bac)
{
	extern long FromInstall; //from UI

  if( ! UI_Up &&  ! FromInstall)
    return ;
  char Bac[66], SrcNam[66], DestNam[66];
  strcpy(Bac, _Bac), Bac[2] = '_';
  Str(DestNam, "%s\\%s", _Com, Bac);
  mkdir(DestNam);
  DirA Loop(2)if(DirI(J ? "%s\\?????.TXT" : "%s\\*.XLS", _Com))
  do
  {
    if( ! stricmp(Next, "QTR.TXT") && DirC)
      break;
    Str(SrcNam, "%s\\%s", _Com, Next);
    Str(DestNam, "%s\\%s\\%s", _Com, Bac, Next);
    Cop(SrcNam, DestNam);
  }
  while( ! DirC);
}

///////////////////////////////////////////////////////////////
// File Input/Output help functions used in RunForward()
///////////////////////////////////////////////////////////////
extern SecPurDecT SecPurDec[MaxB];
extern SecSaleDecT SecSaleDec[MaxB];
extern LoanDecT LoanDec[MaxB];
extern SellLnsDecT SellLnsDec[MaxB];
extern CredDecT CredDec[MaxB];
extern DeposDecT DeposDec[MaxB];
extern TreasDecT TreasDec[MaxB];
extern AdminDecT AdminDec[MaxB];
/// <summary> Returns the location of a bank's input fields given an decision form number and the bank number. </summary>
/// <param name="inJ"> is a decision form number from 0 to 7. </param>
/// <returns> the pointer to a bank's decsion form structure. </returns>
// DecP and DecLoc have two copies, one in DecisionForm.cpp
float *DecP(short inJ) 
{
	extern int DecSz[];
	LnP _DecP[]= { (LnP) SecPurDec, (LnP) SecSaleDec, (LnP) LoanDec,
	(LnP) SellLnsDec, (LnP) CredDec, (LnP) DeposDec, (LnP) TreasDec,
	(LnP) AdminDec };
	return (FeP) ( _DecP[inJ] + DecSz[inJ] * Bank );
}

/// <summary> This function returns the pointer to a bank's decision form structure and the size of structure given an decision form number and a bank number. </summary>
/// <param name="ScrI"> is the index of a form.  </param>
/// <param name="**p"> is pointer to the address of bank's decision form structure.</param>
/// <returns> the size of bank's decision structure. </returns>
short DecLoc(short ScrI, float **p)
{
	extern int DecSz[];

   *p = DecP(ScrI);
  return DecSz[ScrI];
}

typedef LoanT *pLoansT[MaxL]; 
//int LnSz = sizeof(LoanT);
extern pLoansT pLoans[MaxB]; // The actual loan portfolio. MaxB is the max bank number = 6. 

void SzM(DWORD *FirstI, int J, DWORD *LnT, LoanT **FirstLoan, DWORD *LnI, LoanT **CurLoan) 
{ 
	extern LoanGloT LoanGlo[MaxB]; 
	int LnSz = sizeof(LoanT);
	
	long Cnt; 
	LoanGlo[Bank].LnCnt[*LnT] = Cnt = J - *FirstI;

	if (Cnt) { 
		DWORD Sz = Cnt * LnSz; 
		LoanT *p = (LoanT *)malloc(Sz);
		memmove( pLoans[Bank][*LnT] = p, *FirstLoan, Sz ); 
	} 

	*FirstI = J; 
	*LnT = *LnI; 
	*FirstLoan = *CurLoan;  
}

/// <summary> This function opens and reads LN0x.dat, the data file containing the loan information. </summary>
/// <remarks> It is only called from Load_Bank().  It calls SzM(DWORD *FirstI, int J, DWORD *LnT, 
/// LoanT **FirstLoan, DWORD *LnI, LoanT **CurLoan) to load loan information. </remarks>
void OpenLoans()
{
	extern LoanGloT LoanGlo[MaxB]; 
	int LnSz = sizeof(LoanT);
	
	long TotCnt;
	LoanT *B;
	ODF("LN00", "DAT", "r");  // open LN0x.dat file from the quarter folder.  
	{
		struct stat fcb;
		fstat(fileno(fp), &fcb);   // fp is pointer to the LN0x.dat file.  
		TotCnt = (fcb.st_size / LnSz);
	}
	B = (LoanT*)malloc((1+TotCnt) *LnSz);
	rv = fread(B, LnSz, TotCnt, fp);
	fclose(fp);
	memset(LoanGlo[Bank].LnCnt, 0, MaxL *4);
	{
		LoanT *FirstLoan = B,  *CurLoan = B - 1;
		DWORD FirstI = 0, LnI, LnT = 0;
		int J; 
		for(J = 0; J < TotCnt; J++)
		{
			if((LnI = (++CurLoan)->Type - 1) == LnT)
				continue;	
			else if(LnI < LnT || LnI >= MaxL)
			{
				memset(CurLoan, 0, LnSz);
				continue;
			}  
  			SzM(&FirstI, J, &LnT, &FirstLoan, &LnI, &CurLoan);
		}
    //SzM
		SzM(&FirstI, J, &LnT, &FirstLoan, &LnI, &CurLoan);
		Fr(B);
	}
}

// The function opens the .csv file that holds the data from Excel
// It returns the _ID from the first line of .csv file
int GetIdFromCsv(int idBank) 
{
	_int64 csv_ID = 0;

	char FileName[12] = "BK00_DI.csv"; 
	FileName[3] = 49 + idBank;
	char CsvPath[14]; 
	Str(CsvPath, "%s\\%s", _Com, FileName);
	ifstream is(CsvPath);

	if(is == NULL) 
		return 0;

	// Skip the first line, which shall have the _ID that matches to .DAT and .IN files.
	is >> csv_ID;
	is.close();  

	return csv_ID; 
}

// This function reads the csv file to load the new decision entry data
bool LoadDecisionCsv(float* pData, int idForm, int idBank) 
{
	extern int DecSz[]; 

	char FileName[12] = "BK00_DI.csv"; 
	FileName[3] = 49 + idBank;
	char CsvPath[14]; 
	Str(CsvPath, "%s\\%s", _Com, FileName);
	ifstream is(CsvPath);

	if(is == NULL) 
		return false;

	// Skip the first line, which shall have the _ID that matches to .DAT and .IN files.
	//_int64 csv_ID = 0; 
	//is >> csv_ID;
	//is.get(); 
	
	for(int i = 0; i < idForm; i++)
	{
		for(int j = 0; j < DecSz[i]/4; j++)
		{
			is >> *pData;
			is.get();
		}
	}

	int count = 0; 
	while(count < DecSz[idForm]/4 && !is.eof())
	{
		is >> *pData;
		is.get();
		*pData++;
		count++; 
	}
	is.close();
	return true;
}

/// <summary> This function loads bank decision entry from BK00.IN. The original function name is Load_Bank </summary>
/// <param name="Prn"> is a flag.  It can be either 1 or 0.  If it is 0, it will also open loan information. </param>
/// <returns> true if it is ready to write all reports, or false if there is no new decision entered. </returns>
int LoadBankDecisions(int Prn)
{ 
	extern BOAT BOA[MaxB]; 
	extern YTDT YTD[MaxB]; 
	extern LnPkgT LnPkg[MaxB][MaxLnPkgs]; 
	extern LnSalesT LnSales[MaxB][MaxLnSales]; 
	extern SecT Sec[MaxB][MaxSecs]; 
	extern FundsGloT FundsGlo[MaxB]; 
	extern LoanGloT LoanGlo[MaxB]; 
	extern ResGloT ResGlo[MaxB];
	extern CostsGloT CostsGlo[MaxB]; 
	extern DepT Dep[MaxB];
	extern TimeDepT TimeDep[MaxB][MaxTimeDep]; 

	extern long FromInstall; 

	int QtrSz = sizeof(AcT), IncmSz = sizeof(IncomeOutGoT);

	DatIO("BK00", "r", 0, BOA[Bank], 8 *QtrSz);
	if( ! fp)
		return 0;
	DatIO("BK00", "r", 8, YTD[Bank], 8 *IncmSz);
	DatIO("BK00", "r", 15, LnPkg + Bank, sizeof(*LnPkg));
	DatIO("BK00", "r", 16, LnSales + Bank, sizeof(*LnSales));
	DatIO("BK00", "r", 17, Sec + Bank, sizeof(*Sec));
	DatIO("BK00", "r", 21, FundsGlo + Bank, sizeof(*FundsGlo));
	FundsGloT *F = FundsGlo + Bank;
	DatIO("BK00", "r", 21, F, sizeof *F);
	DatIO("BK00", "r", 24, LoanGlo + Bank, sizeof(*LoanGlo));
	DatIO("BK00", "r", 28, ResGlo + Bank, sizeof(*ResGlo));
	DatIO("BK00", "r", 30, CostsGlo + Bank, sizeof(*CostsGlo));
	DatIO("BK00", "r", 38, Dep + Bank, sizeof(*Dep));
	DatIO("BK00", "r", 40, TimeDep + Bank, sizeof(*TimeDep));
  
	if( ! Prn)   /// Prn = 0 to read LN0x.dat files
		OpenLoans();
  //Loop(MaxForms)
	for(int J = 0; J < MaxForms; J++)
	{
		float *P;
		long Sz = DecLoc(J, &P), To = Sz / 4;

		// 10/5/2011 testing BK01_DI.csv
		int hasCsv = LoadDecisionCsv(P, J, Bank);
		//ToDo: test this logic here
		if(hasCsv == true)
		{ //write the data from csv to BK00
			DatIO("BK00", "r+", DecSec(J), P, Sz);

			//This flag determines whether to display the warning message of "No New Decision". 
			F->Dirty = true; 
		}
		else //read the data from BK00
		{
			DatIO("BK00", "r", DecSec(J), P, Sz);
		}
	
		if(J == depos)
		{
			if(SimQtr == 12 && P[6] == 1 && P[7] == 10)
				P[6] = .01, P[7] = .1;

			TimeDecT *d = ((DeposDecT*)P)->Time.TimeDec - 1;
			Loop(MaxTimeDec)if((++d)->Mat <= 0 || d->Rate <= 0 || d->Fixed == UnSet)
			d->Mat = d->Rate = d->Fixed =  ! Prn ? 0 : UnSet;
		}
		if( ! Prn)
		{
			Loop(To)if(P[J] == UnSet)
			P[J] = 0;
		}
	}

	return Prn || FromInstall || F->Dirty ||  ! UI_Up;
}

/// 2013 This function shall only be used when user click bank number to enter the input
/// <summary> The function loads the data from Csv file if avaiable in the community directory </summary>
/// <returns> 1 if Csv is succsually loaded, 0 if not </returns> 
bool loadDecisionFromCsvToDat(int Prn)
{
	extern BOAT BOA[MaxB]; 
	extern YTDT YTD[MaxB]; 
	extern LnPkgT LnPkg[MaxB][MaxLnPkgs]; 
	extern LnSalesT LnSales[MaxB][MaxLnSales]; 
	extern SecT Sec[MaxB][MaxSecs]; 
	extern FundsGloT FundsGlo[MaxB]; 
	extern LoanGloT LoanGlo[MaxB]; 
	extern ResGloT ResGlo[MaxB];
	extern CostsGloT CostsGlo[MaxB]; 
	extern DepT Dep[MaxB];
	extern TimeDepT TimeDep[MaxB][MaxTimeDep]; 

	extern long FromInstall; 

	int QtrSz = sizeof(AcT), IncmSz = sizeof(IncomeOutGoT);

	DatIO("BK00", "r", 0, BOA[Bank], 8 *QtrSz);
	if( ! fp)
		return 0;
	DatIO("BK00", "r", 8, YTD[Bank], 8 *IncmSz);
	DatIO("BK00", "r", 15, LnPkg + Bank, sizeof(*LnPkg));
	DatIO("BK00", "r", 16, LnSales + Bank, sizeof(*LnSales));
	DatIO("BK00", "r", 17, Sec + Bank, sizeof(*Sec));
	DatIO("BK00", "r", 21, FundsGlo + Bank, sizeof(*FundsGlo));
	FundsGloT *F = FundsGlo + Bank;
	DatIO("BK00", "r", 21, F, sizeof *F);
	DatIO("BK00", "r", 24, LoanGlo + Bank, sizeof(*LoanGlo));
	DatIO("BK00", "r", 28, ResGlo + Bank, sizeof(*ResGlo));
	DatIO("BK00", "r", 30, CostsGlo + Bank, sizeof(*CostsGlo));
	DatIO("BK00", "r", 38, Dep + Bank, sizeof(*Dep));
	DatIO("BK00", "r", 40, TimeDep + Bank, sizeof(*TimeDep));

	int hasCsv;
	  
	if( ! Prn)   /// Prn = 0 to read LN0x.dat files
		OpenLoans();

	for(int J = 0; J < MaxForms; J++)
	{
		float *P;
		long Sz = DecLoc(J, &P), To = Sz / 4;

		// 10/5/2011 testing BK01_DI.csv
		hasCsv = LoadDecisionCsv(P, J, Bank);
		//ToDo: test this logic here
		if(hasCsv == true)
		{ //write the data from csv to BK00
			DatIO("BK00", "r+", DecSec(J), P, Sz);

			//This flag determines whether to display the warning message of "No New Decision". 
			//F->Dirty = true; 

			//Don't know what it does here
			if(J == depos)
			{
				if(SimQtr == 12 && P[6] == 1 && P[7] == 10)
					P[6] = .01, P[7] = .1;

				TimeDecT *d = ((DeposDecT*)P)->Time.TimeDec - 1;
				Loop(MaxTimeDec)if((++d)->Mat <= 0 || d->Rate <= 0 || d->Fixed == UnSet)
				d->Mat = d->Rate = d->Fixed =  ! Prn ? 0 : UnSet;
			}
			if( ! Prn)
			{
				Loop(To)if(P[J] == UnSet)
				P[J] = 0;
			}
		}

	}

	return hasCsv; 
}

/// <summary> 2013, BMSim write the decision data from csv to the BK0N.DAT. </summary>
/// <param name="discard"> is the flat.  If 0, the decision entries are saved the data file. If -1, discard </param>
int saveDecision(long CurDecForm)
{
	int DecSz[]= {sizeof* SecPurDec, sizeof* SecSaleDec,sizeof* LoanDec, sizeof* SellLnsDec, sizeof* CredDec, sizeof* DeposDec, sizeof* TreasDec, sizeof* AdminDec };
	FldsT Flds[128];
  //Fr(ScrBuff);
  //ScrBuff = 0;
  //if( ! discard)
  
    FILE *fp = fopen(DatFile, "r+b");
    if(!fp)
    {
		Str(_T, "Can't W-Open: (%s)", DatFile);
		return 0;
    }

    fseek(fp, (long)(DecSec(rv = CurDecForm) *1024), 0);
    {
		int Sz = DecSz[rv];
		FeP B = (FeP)malloc(Sz);
		{
			Loop(Sz / 4)B[J] = Flds[J].Float;
		}
		fwrite(B, 1, Sz, fp);
		Fr(B);
      /*{
        ulong Dirty = 0xDDCCEEAB;
        FundsGloT *F = 0;
        fseek(fp, 21 *1024+(long) &F->Dirty, 0);
        fwrite(&Dirty, 1, 4, fp);
        fclose(fp);
      }*/
	}
  
	return 1;
}

/// <summary> This function clears some decisions for the current quarter and the next quarter before starting the decision forms</summary>
/// <remarks> This function writes the data into BK0x file. </remarks>
void StartingDecs()
{
	for(int intIndexOfCount = 0; intIndexOfCount < MaxForms; intIndexOfCount++)
	{
		float *P;
		int Form = intIndexOfCount, Sz = DecLoc(Form, &P), To = Sz / 4;

		// 11/27/2011, Bill starts using CSV files for decision entry
		// If CSV files are not available in the directory, switch back to BK0N.dat
		int hasCsv = LoadDecisionCsv(P, Form, Bank); 
		if(hasCsv == false)
		{
			DatIO("BK00", "r", DecSec(Form), P, Sz);
		}
    
		switch(Form)
		{
			case depos:
				if(SimQtr == 12 && P[6] == 1 && P[7] == 10)
					P[6] = .01, P[7] = .1;

				{
					TimeDecT *d = ((DeposDecT*)P)->Time.TimeDec - 1;
					for(int intIndexOfCount = 0; intIndexOfCount < MaxTimeDec; intIndexOfCount++)
					{
						if((++d)->Mat <= 0 || d->Rate <= 0 || d->Fixed == UnSet)
							d->Mat = d->Rate = d->Fixed = UnSet;
					}
				}
				break;
	
			case secpur:
			case secsal:
			case lnprsl:
				{
					for(int intIndexOfCount = 0; intIndexOfCount < To; intIndexOfCount++)
						P[intIndexOfCount] = UnSet;
				}
				break;
			
			case treas:
				{
					TreasDecT *q = (TreasDecT*)P;
					float *r = &q->Div,  *z = &q->Forecast,  *pp = P - 1;
					{
						for(int intIndexOfCount = 0; intIndexOfCount < To; intIndexOfCount++)
						{
							pp++;
							if(pp != r && pp != z)
								*pp = UnSet;
						}
					}
				}
				break;
			
			case genadm:
				{
					FeP *q = (FeP*) &(((AdminDecT*)P)->Premises);
					int To = sizeof(DecPremT) / 4;
					for(int intIndexOfCount = 0; intIndexOfCount < To; intIndexOfCount++)
						*((float*)q + intIndexOfCount) = UnSet;
					// REVIEW / TODO resolve type problem should be q[intIndexOfCount] = UnSet;
				}
			break;
			case credadm:
			{
				CredDecT *q = (CredDecT*)P;
				q->Nat.DoRev = q->MidM.DoRev = q->SmallB.DoRev = q->ImpExp.DoRev = q->Cons.DoRev = q->CommRe.DoRev = q->MulFam.DoRev = q->SingFam.DoRev = q->HomeEqu.DoRev = q->Personal.DoRev = q->CredC.DoRev = q->Instal.DoRev = UnSet;
			}
			break;
		}
		// Make a copy of the end of quarter decisions:
		DatIO("BK00", "r+", DecSec(Form), P, Sz);
		DatIO("BK00", "r+", DecSec2(Form), P, Sz);
	}
}