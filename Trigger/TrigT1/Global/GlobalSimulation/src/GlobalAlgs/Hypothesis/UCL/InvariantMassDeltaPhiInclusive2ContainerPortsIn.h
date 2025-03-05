/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMSORTSELECTCOUNTCONTAINERPORTSIN_H
#define GLOBALSIM_EEMSORTSELECTCOUNTCONTAINERPORTSIN_H

#include "AlgoConstants.h"
#include "AlgoDataTypes.h"

#include <ostream>
#include <memory>
#include <vector>


#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim {

  struct InvariantMassDeltaPhiInclusive2ContainerPortsIn {

    using GenTobPtr = std::shared_ptr<GenericTob>;

    // Output GenericTobs. VHDL variable is an array of GenericTobs
    std::array<GenTobPtr, AlgoConstants::maxNTob>  m_I_Tobs1;
    std::array<GenTobPtr, AlgoConstants::maxNTob>  m_I_Tobs2;

  };

}

std::ostream&
operator<< (std::ostream&,
	    const GlobalSim::InvariantMassDeltaPhiInclusive2ContainerPortsIn&);

CLASS_DEF( GlobalSim::InvariantMassDeltaPhiInclusive2ContainerPortsIn , 1091889833 , 1 )

#endif 
  
