/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GLOBALSIM_PU1SUPPEXPECTATIONS_H
#define GLOBALSIM_PU1SUPPEXPECTATIONS_H

#include "AthenaKernel/CLASS_DEF.h"
#include <string>

namespace GlobalSim{
	
	struct PU1SuppExpectations {
	  PU1SuppExpectations(const std::string& tobs, const std::string& mults): 
		m_expected_tob_bits{tobs}, m_expected_multiplicity_bits{mults}{}

	  std::string m_expected_tob_bits;
	  std::string m_expected_multiplicity_bits;
	};
}

CLASS_DEF( GlobalSim::PU1SuppExpectations, 394339223, 1)

#endif	
