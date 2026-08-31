/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSWRITETRACKINGGEOMETRYTRANSFORMS_H
#define ACTSGEOMETRY_ACTSWRITETRACKINGGEOMETRYTRANSFORMS_H

// ATHENA
#include "AthenaBaseComps/AthAlgorithm.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"


// PACKAGE
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsEvent/ContextUtility.h"

// STL
#include <fstream>
#include <memory>
#include <vector>

namespace Acts {
  class TrackingGeometry;
}


class ActsWriteTrackingGeometryTransforms : public AthAlgorithm {
public:
  using AthAlgorithm::AthAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;

  virtual ~ActsWriteTrackingGeometryTransforms() = default;

private:

  const PixelID *m_pixelID{nullptr};
  const SCT_ID  *m_SCT_ID{nullptr};

 ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

  /** @brief Context provider for geometry, magnetic field and calibration contexts */
  ActsTrk::ContextUtility m_ctxProvider{this};

  Gaudi::Property<std::string> m_outputName{this, "OutputName", "transforms.csv", "Filename to write the transform output to"};
  Gaudi::Property<bool> m_writeFullTransform{this,"WriteFullTransform",false,"Decide if full transformation needs to be written"};
};

#endif
