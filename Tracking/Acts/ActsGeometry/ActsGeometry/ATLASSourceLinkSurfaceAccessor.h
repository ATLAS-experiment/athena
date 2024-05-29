/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ATLASSOURCELINKSURFACEACCESSOR_H
#define ATLASSOURCELINKSURFACEACCESSOR_H

#include "ATLASSourceLink.h"
#include "TrackingSurfaceHelper.h"

namespace Acts {
   class TrackingGeometry;
   class Surface;
}

namespace ActsTrk {
class IActsToTrkConverterTool;

struct ATLASSourceLinkSurfaceAccessor {
  const ActsTrk::IActsToTrkConverterTool *m_converterTool = nullptr;

  const Acts::Surface* operator()(const Acts::SourceLink& sourceLink) const;
};

struct ATLASUncalibSourceLinkSurfaceAccessor {
  const ActsTrk::IActsToTrkConverterTool *m_converterTool = nullptr;
  const TrackingSurfaceHelper *m_surfaceHelper = nullptr;

  const Acts::Surface* operator()(const Acts::SourceLink& sourceLink) const;
};
}

#include "ActsEventCnv/IActsToTrkConverterTool.h"

namespace ActsTrk {
// surface accessor implementation for ATLASUncalibSourceLink i.e. xAOD::UncalibratedMeasurement
inline const Acts::Surface* ATLASUncalibSourceLinkSurfaceAccessor::operator()(const Acts::SourceLink& sourceLink) const {
   const auto atlas_uncalib_source_link = sourceLink.get<ATLASUncalibSourceLink>();
   const xAOD::UncalibratedMeasurement &uncalibMeas = getUncalibratedMeasurement(atlas_uncalib_source_link);

   return &this->m_converterTool->trkSurfaceToActsSurface(m_surfaceHelper->associatedSurface( uncalibMeas ));
}

// surface accessor implementation for ATLASSourceLink i.e. Trk::MeasurementBase
inline const Acts::Surface* ATLASSourceLinkSurfaceAccessor::operator()(const Acts::SourceLink& sourceLink) const {
   const auto atlas_source_link = sourceLink.get<ATLASSourceLink>();
   return &this->m_converterTool->trkSurfaceToActsSurface( atlas_source_link->associatedSurface() );
}
}
#endif
