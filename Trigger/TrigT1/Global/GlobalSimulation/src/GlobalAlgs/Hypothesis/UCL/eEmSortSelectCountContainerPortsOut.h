/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMSORTSELECTCOUNTCONTAINERPORTSOUT_H
#define GLOBALSIM_EEMSORTSELECTCOUNTCONTAINERPORTSOUT_H

#include "AlgoConstants.h"
#include "AlgoDataTypes.h"

#include <ostream>
#include <memory>
#include <vector>


#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim {

  struct eEmSortSelectCountContainerPortsOut {

    using GenTobPtr = std::shared_ptr<GenericTob>;
    using BSPtrNumTotalCountWidth =
      std::shared_ptr<std::bitset<AlgoConstants::eEmNumTotalCountWidth>>;

    // Output GenericTobs. VHDL variable is an array od eEMGenericTobs
    std::vector<GenTobPtr> m_O_eEmGenTob {AlgoConstants::eEmNumTotalTobWidth,
					  GenTobPtr()};

    // Output counts. VHDL variable is a bit array
    BSPtrNumTotalCountWidth
    m_O_Multiplicity{std::make_shared<std::bitset<AlgoConstants::eEmNumTotalCountWidth>>()};
  
  };

}

std::ostream&
operator<< (std::ostream&,
	    const GlobalSim::eEmSortSelectCountContainerPortsOut&);

CLASS_DEF( GlobalSim::eEmSortSelectCountContainerPortsOut , 1289475565 , 1 )

#endif 
  
