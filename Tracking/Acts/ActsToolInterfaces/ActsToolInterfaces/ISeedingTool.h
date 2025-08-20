/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ISEEDINGTOOL_H
#define ACTSTOOLINTERFACES_ISEEDINGTOOL_H

// Athena
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

// ACTS EDM
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/EventData/SeedContainer2.hpp"
#include "Acts/EventData/SpacePointContainer.hpp"
#include "Acts/EventData/SpacePointContainer2.hpp"
#include "ActsEvent/Seed.h"
#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/SpacePointCollector.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

namespace ActsTrk {

class ISeedingTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ISeedingTool, 1, 0);

  virtual StatusCode createSeeds(
      const EventContext& ctx,
      const Acts::SpacePointContainer<ActsTrk::SpacePointCollector,
                                      Acts::detail::RefHolder>& spContainer,
      const Acts::Vector3& beamSpotPos, const Acts::Vector3& bField,
      ActsTrk::SeedContainer& seedContainer) const {
    (void)ctx;
    (void)spContainer;
    (void)beamSpotPos;
    (void)bField;
    (void)seedContainer;
    return StatusCode::FAILURE;
  }

  virtual StatusCode createSeeds2(
      const EventContext& ctx,
      const std::vector<const xAOD::SpacePointContainer*>&
          spacePointCollections,
      const Eigen::Vector3f& beamSpotPos, float bFieldInZ,
      ActsTrk::SeedContainer& seedContainer) const {
    (void)ctx;
    (void)spacePointCollections;
    (void)beamSpotPos;
    (void)bFieldInZ;
    (void)seedContainer;
    return StatusCode::FAILURE;
  }
};

}  // namespace ActsTrk

#endif
