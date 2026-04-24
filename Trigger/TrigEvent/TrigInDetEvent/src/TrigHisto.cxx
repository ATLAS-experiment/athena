/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigInDetEvent/TrigHisto.h"




//---------------------------------------------------------------

void TrigHisto::clear(void) {
  for (auto& v : m_contents) {
    v = 0.f;
  }
}

//---------------------------------------------------------------

// Require histogram sizes such that it might be called by TrigHisto2D too.
unsigned int TrigHisto::findBin(unsigned int nbins, 
				float h_min, 
				float h_max, 
				float binSize, 
				float value) const {
  unsigned int ibin = 0;
  
  if(value < h_min) { // Underflow
    ibin = 0;
  }
  else if( !(value < h_max)) { // Overflow (catches NaN)
    ibin = nbins+1;
  }
  else {
    while(value > (ibin*binSize+h_min) && ibin <= nbins) { // None under/overflow from 1 to nbins
      ibin++;
    }
  }
  
  return ibin;
}

