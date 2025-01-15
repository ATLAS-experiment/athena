/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/DetectorVolumeSvc.h"
#include <Acts/Utilities/AxisDefinitions.hpp>

#include "ActsGeoUtils/NoDeletePtr.h"
// ATHENA
#include "GaudiKernel/EventContext.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "PathResolver/PathResolver.h"

// ACTS
#include "Acts/Detector/Detector.hpp"
#include "Acts/Detector/DetectorVolume.hpp"
#include "Acts/Detector/CylindricalContainerBuilder.hpp"
#include "Acts/Detector/DetectorBuilder.hpp"

using namespace ActsTrk;
DetectorVolumeSvc::DetectorVolumeSvc(const std::string &name,
                                                 ISvcLocator *svc)
    : base_class(name, svc) {}

StatusCode DetectorVolumeSvc::initialize() {
  ATH_CHECK(m_builderTools.retrieve());
  if (m_builderTools.empty()) {
      ATH_MSG_FATAL("No subdetectors were defined ");
      return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

std::shared_ptr<const Acts::Experimental::Detector> DetectorVolumeSvc::detector() const {
  if (!m_detector.isValid()) {
      ATH_MSG_INFO("Build the Acts tracking detector");
      m_detector.set(buildDetector());
   }
  return *m_detector.ptr();
}

unsigned int DetectorVolumeSvc::populateAlignmentStore(AlignmentStore& /*store*/) const {
    return 0;
}

const ActsGeometryContext& DetectorVolumeSvc::getNominalContext() const {
    return m_nomContext;
}

std::shared_ptr<const Acts::Experimental::Detector> DetectorVolumeSvc::buildDetector() const {
    ActsGeometryContext gctx{};
    std::vector<std::shared_ptr<const Acts::Experimental::IDetectorComponentBuilder> > builders;
    for (const auto &builder : m_builderTools) {
        builders.push_back(NoDeletePtr<const Acts::Experimental::IDetectorComponentBuilder>(builder.get()));
    }
    using enum Acts::AxisDirection;

    //Define config for cylindrical container builder
    Acts::Experimental::CylindricalContainerBuilder::Config cylindricalCfg;
    cylindricalCfg.builders = builders;
    cylindricalCfg.binning = std::vector<Acts::AxisDirection>{AxisZ, AxisR};
    auto cylindricalBuilder = std::make_shared<Acts::Experimental::CylindricalContainerBuilder>(cylindricalCfg);

    //Define config for detector builder
    Acts::Experimental::DetectorBuilder::Config detectorCfg;
    detectorCfg.builder = cylindricalBuilder;
    return Acts::Experimental::DetectorBuilder(detectorCfg).construct(gctx.context());
}
