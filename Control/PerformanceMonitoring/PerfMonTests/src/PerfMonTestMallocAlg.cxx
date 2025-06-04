/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PerfMonTestMallocAlg.cxx 
// Implementation file for class PerfMonTest::MallocAlg
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes
#include <vector>

// FrameWork includes
#include "Gaudi/Property.h"

// CLHEP includes
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Random/RandGauss.h"

// PerfMonTests includes
#include "PerfMonTestMallocAlg.h"

using namespace PerfMonTest;


StatusCode MallocAlg::execute()
{  
  ATH_MSG_DEBUG ( "Executing " << name() << "..." ) ;

  m_currentEvtNbr++;

  //typedef int ElemType;
  typedef float ElemType;

  if ( m_currentEvtNbr >= m_evtNbr ) {
    const std::size_t nmax = 1024*1024;

    if (m_useStdVector) {
      std::vector<ElemType> c1_array(nmax);
      for ( std::size_t i = 0; i!=nmax; ++i ) {
        c1_array[i] =  i;
      }
      
      if ( m_currentEvtNbr >= 2*m_evtNbr ) {
        std::vector<ElemType> c2_array(nmax);
        for ( std::size_t i = 0; i!=nmax; ++i ) {
          c2_array[i] =  -1*static_cast<int>(i);
        }
      }
      
    } else {
      ElemType c1_array[nmax];
      for ( std::size_t i = 0; i!=nmax; ++i ) {
        c1_array[i] =  i;
      }
      isUsed (&c1_array);

      if ( m_currentEvtNbr >= 2*m_evtNbr ) {
        ElemType c2_array[nmax];
        for ( std::size_t i = 0; i!=nmax; ++i ) {
          c2_array[i] =  -1*static_cast<int>(i);
        }
        // dummy stuff to silence gcc-warning.
        if (c2_array[0] > c1_array[0]) { 
          c2_array[0] = c1_array[0];
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}
