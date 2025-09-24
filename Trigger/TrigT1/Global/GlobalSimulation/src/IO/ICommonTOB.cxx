/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ICommonTOB.h"

namespace GlobalSim::IOBitwise {
  std::ostream& operator << (std::ostream& os,
			     const ICommonTOB& tob) {

    os << "Et: " << tob.et_bits()
       << " Eta: " << tob.eta_bits()
       << " Phi: " << tob.phi_bits();

    return os;
  }

}
