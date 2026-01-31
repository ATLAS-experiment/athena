/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSVOLUMEMAPPINGTOOL_H
#define ACTSGEOMETRY_ACTSVOLUMEMAPPINGTOOL_H

// ATHENA
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/EventContext.h"

// PACKAGE
#include "ActsGeometryInterfaces/IActsVolumeMappingTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

// ACTS
#include "Acts/Material/VolumeMaterialMapper.hpp"

// BOOST

#include <cmath>

class ActsVolumeMappingTool : public extends<AthAlgTool, IActsVolumeMappingTool>
{

public:
  virtual StatusCode initialize() override;

  ActsVolumeMappingTool(const std::string& type, const std::string& name,
	           const IInterface* parent);

  std::shared_ptr<Acts::VolumeMaterialMapper>
  mapper() const override
  {
    return m_mapper;
  };

  virtual
  Acts::VolumeMaterialMapper::State
  mappingState() const override;

private:
  // Straight line stepper
  using SlStepper  = Acts::StraightLineStepper;
  using StraightLinePropagator = Acts::Propagator<SlStepper, Acts::Navigator>;
  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};
  std::shared_ptr<Acts::VolumeMaterialMapper> m_mapper;
  std::shared_ptr<const Acts::TrackingGeometry> m_trackingGeometry;
};



#endif
