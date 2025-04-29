/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "Identifier/Identifier.h"
#include <vector>
#include <tuple>

namespace ActsTrk::detail {
  
  std::tuple<std::vector<int>,
    std::vector< std::vector< int > >,
    std::vector< std::vector< float > >
    >
    getSDOInformation( const std::vector<Identifier>& rdoList,
		       const InDetSimDataCollection& sdoCollection );


  std::tuple<std::vector<float>,
    std::vector<float>,
    std::vector<int>,
    std::vector<int>,
    std::vector<float>,
    std::vector<float>,
    std::vector<float>,
    std::vector<float>,
    std::vector<float>,
    std::vector<float>
    >
    getSiHitInformation( const InDetDD::SiDetectorElement& element,
			 const std::vector<SiHit>& matchingHits );
  
}

