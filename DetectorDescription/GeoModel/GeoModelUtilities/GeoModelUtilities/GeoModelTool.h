/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELUTILITIES_GEOMODELTOOL_H
#define GEOMODELUTILITIES_GEOMODELTOOL_H

#ifndef BUILDVP1LIGHT

#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoModelKernel/GeoVDetectorManager.h"
#include "GeoModelInterfaces/IGeoModelTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

class GeoModelTool : public extends<AthAlgTool, IGeoModelTool> {

public:
  using base_class::base_class;
  virtual ~GeoModelTool() = default;

  virtual GeoVDetectorManager* manager() {return m_detector;}
  virtual const GeoVDetectorManager* manager() const {return m_detector;}

  virtual StatusCode clear() override {return StatusCode::SUCCESS;}
  virtual StatusCode align() override {return StatusCode::SUCCESS;}

protected:
  GeoVDetectorManager*   m_detector{nullptr};
};

#endif  // BUILDVP1LIGHT

#endif // GEOMODELSVC_DETDESCRTOOL_H
