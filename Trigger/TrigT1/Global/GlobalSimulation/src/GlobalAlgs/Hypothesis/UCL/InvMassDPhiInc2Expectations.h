/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_INVMASSDPHIINC2EXPECTATIONS_H
#define GLOBALSIM_INVMASSDPHIINC2EXPECTATIONS_H

#include "AthenaKernel/CLASS_DEF.h"

#include <string>

namespace GlobalSim {
  struct InvMassDPhiInc2Expectations {
    InvMassDPhiInc2Expectations(const std::string& results):
      m_expected_results{results}{}

      std::string m_expected_results;  // bit string
  };
}

CLASS_DEF( GlobalSim::InvMassDPhiInc2Expectations , 236371547 , 1 )

#endif
