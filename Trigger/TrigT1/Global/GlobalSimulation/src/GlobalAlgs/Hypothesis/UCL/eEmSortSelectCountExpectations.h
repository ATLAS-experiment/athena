/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMSORTSELECTCOUNTEXPECTATIONS_H
#define GLOBALSIM_EEMSORTSELECTCOUNTEXPECTATIONS_H

#include "AthenaKernel/CLASS_DEF.h"

#include <string>

namespace GlobalSim {
  struct eEmSortSelectCountExpectations {
    eEmSortSelectCountExpectations(const std::string& tobs,
				   const std::string& mults):
      m_expected_tob_bits{tobs}, m_expected_multiplicity_bits{mults}{}

    std::string m_expected_tob_bits;
    std::string m_expected_multiplicity_bits;
  };
}

CLASS_DEF( GlobalSim::eEmSortSelectCountExpectations , 122550346 , 1 )

#endif
