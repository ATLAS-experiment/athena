/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_INVARIANTMASSDELTAPHIINCLUSIVE2ContainerPORTSOUT_H
#define GLOBALSIM_INVARIANTMASSDELTAPHIINCLUSIVE2ContainerPORTSOUT_H

#include "AlgoConstants.h"
#include "GenericTob.h"

#include <ostream>
#include <memory>
#include <vector>


#include "AthenaKernel/CLASS_DEF.h"

namespace GlobalSim {

  struct InvariantMassDeltaPhiInclusive2ContainerPortsOut {

    using GenTobPtr = std::shared_ptr<GenericTob>;


    
    // output bits

    constexpr static std::size_t MaxResultsBits{4};

    using BSPtrMaxResultsBits = std::shared_ptr<std::bitset<MaxResultsBits>>;
    
    BSPtrMaxResultsBits
    m_Results{std::make_shared<std::bitset<MaxResultsBits>>()};

  };

}

std::ostream&
operator<< (std::ostream&,
	    const GlobalSim::InvariantMassDeltaPhiInclusive2ContainerPortsOut&);

CLASS_DEF( GlobalSim::InvariantMassDeltaPhiInclusive2ContainerPortsOut , 1330059925 , 1 )

#endif 
  
