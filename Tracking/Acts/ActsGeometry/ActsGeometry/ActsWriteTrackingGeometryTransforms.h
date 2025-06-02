/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSWRITETRACKINGGEOMETRYTRANSFORMS_H
#define ACTSGEOMETRY_ACTSWRITETRACKINGGEOMETRYTRANSFORMS_H

// ATHENA
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "Gaudi/Property.h"  /*no forward decl: typedef*/
#include "GaudiKernel/ISvcLocator.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"


// PACKAGE
#include "ActsGeometry/ActsObjWriterTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

// STL
#include <fstream>
#include <memory>
#include <vector>

namespace Acts {
  class TrackingGeometry;
}

class ActsTrackingGeometryTool;

class ActsWriteTrackingGeometryTransforms : public AthAlgorithm {
public:
  ActsWriteTrackingGeometryTransforms (const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

  virtual ~ActsWriteTrackingGeometryTransforms() = default;

private:

  const PixelID *m_pixelID;
  const SCT_ID  *m_SCT_ID;

  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};

  Gaudi::Property<std::string> m_outputName{this, "OutputName", "transforms.csv", "Filename to write the transform output to"};
  Gaudi::Property<bool> m_writeFullTransform{this,"WriteFullTransform",false,"Decide if full transformation needs to be written"};
};

#endif
