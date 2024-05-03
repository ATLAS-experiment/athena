/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AmdcStand/GetAmdcsimrecVariable.h"
// Fortran routines -------------------------------------------------------------
#include "f1get.h"

// Get Amdcsimrec REAL (double) Variable -------------------------------------------

double
GetAmdcRealVar(const std::string & VarName, int I1, int I2, int I3){
  std::string NAMEVAR = VarName.substr(0,40);
  int Long = NAMEVAR.size();
  double ToBeReturned=0.;
  f1getramdcvar_( Long, NAMEVAR.data(), I1, I2, I3, ToBeReturned, Long );
  return ToBeReturned;
}

// Get Amdcsimrec INTEGER (int) Variable -------------------------------------------

int
GetAmdcIntVar(const std::string & VarName, int I1, int I2, int I3){
  std::string NAMEVAR = VarName.substr(0,40);
  int Long = NAMEVAR.size();
  int ToBeReturned=0;
  f1getiamdcvar_( Long, NAMEVAR.data(), I1, I2, I3, ToBeReturned, Long );
  return ToBeReturned;
}

// Get Amdcsimrec CHARACTER (std::string) Variable ---------------------------------

std::string
GetAmdcCharVar(const std::string & VarName, int I1, int I2, int I3){
  std::string NAMEVAR = VarName.substr(0,40);
  int Long = NAMEVAR.size();
  char CVAR[40];
  int Lvar{};
  f1getcamdcvar_( Long , NAMEVAR.data(), I1, I2, I3, Lvar, CVAR, Long, 40 );
  std::string ToBeReturned(CVAR, CVAR+Lvar);
  return ToBeReturned;
}
