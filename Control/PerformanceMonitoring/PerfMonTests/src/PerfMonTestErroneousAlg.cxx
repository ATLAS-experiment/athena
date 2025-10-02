/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PerfMonTestErroneousAlg.cxx 
// Implementation file for class PerfMonTest::ErroneousAlg
// Author: R.Seuster
/////////////////////////////////////////////////////////////////// 


// STL includes

// FrameWork includes
#include "Gaudi/Property.h"

// PerfMonTests includes
#include "PerfMonTestErroneousAlg.h"

using namespace PerfMonTest;


StatusCode ErroneousAlg::execute()
{  
  ATH_MSG_DEBUG ( "Executing " << name() << "..." ) ;
  
  if ( this->jumpOnUninitializedValue() )
    ATH_MSG_INFO ( " jumpOnUninitializedValue() returned false !" ) ;
  
  if ( this->invalidRead() )
    ATH_MSG_INFO ( " jumpOnUninitializedValue() returned false !" ) ;
  
  if ( this->mismatchedFree() )
    ATH_MSG_INFO ( " jumpOnUninitializedValue() returned false !" ) ;
  
  return StatusCode::SUCCESS;
}

bool ErroneousAlg::jumpOnUninitializedValue()
{
  const unsigned int maximum=8192;
  
  bool someConditions[64*maximum];
  
  unsigned int yesNo[] = { 0, 0};
  
  for ( unsigned int i=0; i<64*maximum; ++i )
    // cppcheck-suppress uninitvar; this is meant to be uninitialized
    // cppcheck-suppress legacyUninitvar; this is meant to be uninitialized
    if ( this->shouldIJump(someConditions[i]) )
      yesNo[0]++;
    else
      yesNo[1]++;
  
  //m_msg << "Jump, jump not, jump, jump not : " << yesNo[0] << " to " << yesNo[1] << endmsg;
  
  return (yesNo[1]==0);
}

bool ErroneousAlg::invalidRead()
{
  const unsigned int maximum=8192;
  double *invalidReadPointer = new double[maximum];
  
  // fill with something
  for ( unsigned int i=1; i<=maximum; ++i ){
    //intentional out-of-bounds access
    //cppcheck-suppress arrayIndexOutOfBounds
    invalidReadPointer[i] = double(i);
  }
  //intentional out-of-bounds access
#if __GNUC__ >= 15
# pragma GCC diagnostic ignored "-Warray-bounds"
#endif
  //cppcheck-suppress arrayIndexOutOfBounds
  ATH_MSG_INFO ( "Found, that last element contains " << invalidReadPointer[maximum] ) ;
  
  // and delete
#if __GNUC__ >= 12
# pragma GCC diagnostic ignored "-Wuse-after-free"
#endif  
  delete [] invalidReadPointer;
  
  // and print out element 10 !
  //intentional out-of-bounds access
  //cppcheck-suppress arrayIndexOutOfBounds
  ATH_MSG_INFO ( "Found, that last element contains " << invalidReadPointer[maximum] ) ;
  
  return true;
}

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif
bool ErroneousAlg::mismatchedFree()
{
  const unsigned int maximum=8192;
  double *invalidReadPointer = new double[maximum];
  
  // fill with something
  for ( unsigned int i=1; i<maximum; ++i )
    invalidReadPointer[i] = double(i);
  ATH_MSG_INFO ( "Found, that last element contains " << invalidReadPointer[maximum-1] ) ;
  
  // and delete
  delete [] invalidReadPointer;
  
  return true;
}
