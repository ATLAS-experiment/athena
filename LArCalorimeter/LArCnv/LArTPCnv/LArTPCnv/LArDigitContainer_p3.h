/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDIGITCONTAINER_P3_H
#define LARDIGITCONTAINER_P3_H

#include <vector>

class LArDigitContainer_p3{
public:
  bool m_this_is_slar = false;        //false for standard cells, true for supercells
  unsigned char m_nSamples = 0;       //identical for all readout channels
  std::vector<unsigned char> m_gain;  //2 bits per online-hash. 0:no data 1,2,3:gain+1
  std::vector<short> m_samples;
};


#endif
