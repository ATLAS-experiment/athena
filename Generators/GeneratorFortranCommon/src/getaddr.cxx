/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////////
// Routine meant to be called from FORTRAN which simply returns the
// address of the (FORTRAN) argument. This should be 64bit-safe.
//
// 1999/01/08 Chris Green (Purdue University)
// 1999/04/29 CG * altered to use void* from long*
////////////////////////////////////////////////////////////////////////

extern "C" {

  void* getaddr_(void* arg);
  void* getaddri_(int* arg);

}

void* getaddr_(void* arg) {

  return(arg);

}

// Version to be used with integer arguments, to prevent LTO warnings
// about inconsistent parameter types.
void* getaddri_(int* arg) {

  return(arg);

}
