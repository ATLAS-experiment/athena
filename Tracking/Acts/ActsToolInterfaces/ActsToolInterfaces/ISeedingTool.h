/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ISEEDINGTOOL_H
#define ACTSTOOLINTERFACES_ISEEDINGTOOL_H

// Athena 
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "Acts/MagneticField/MagneticFieldContext.hpp"

// ACTS EDM
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"
#include "ActsEvent/SpacePointCollector.h"
#include "ActsEvent/Seed.h"

#include "Acts/EventData/SpacePointContainer.hpp"
#include "Acts/Definitions/Algebra.hpp"

namespace ActsTrk {
  class ISeedingTool
    : virtual public IAlgTool {
  public:
    DeclareInterfaceID(ISeedingTool, 1, 0);
    
    virtual
      StatusCode
      createSeeds(const EventContext& ctx,
		  const Acts::SpacePointContainer<ActsTrk::SpacePointCollector, Acts::detail::RefHolder>& spContainer,
                  const Acts::Vector3& beamSpotPos,
                  const Acts::Vector3& bField,
                  ActsTrk::SeedContainer& seedContainer ) const = 0;
  };
  
} // namespace 

#endif 

