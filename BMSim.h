//#ifndef BMSIM_H
//#define BMSIM_H
#include "GlobalDef.h"
#include "ReportingDataT.h"
#include "BMSimDataT.h"
#include "DecisionFormDataT.h" 
//#include "FileIOHelper.h"
#include "BMSimHelper.h" 

//Calculation
void Economy(); 
void Securities(); 
void Resources(); 
void Loans();
void Treasury();
void Deposits(); //TBC
void Expenses();  
void Books(); 
void Stock();
void AvailableFunds();

//Keep in BMSim.h, and called in ui.cpp
void RunForward(); 
void MakeDats(); 
//#endif