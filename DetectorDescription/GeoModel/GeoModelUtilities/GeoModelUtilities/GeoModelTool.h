/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELUTILITIES_GEOMODELTOOL_H
#define GEOMODELUTILITIES_GEOMODELTOOL_H

#ifndef BUILDVP1LIGHT

#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoModelKernel/GeoVDetectorManager.h"
#include "GeoModelInterfaces/IGeoModelTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"

class GeoModelTool : public extends<AthAlgTool, IGeoModelTool> {

public:
  using base_class::base_class;
  virtual ~GeoModelTool() = default;

  virtual GeoVDetectorManager* manager() {return m_detector;}
  virtual const GeoVDetectorManager* manager() const {return m_detector;}

  virtual StatusCode clear() override {return StatusCode::SUCCESS;}
  virtual StatusCode registerCallback ATLAS_NOT_THREAD_SAFE () override {return StatusCode::FAILURE;}
  virtual StatusCode align(IOVSVC_CALLBACK_ARGS) override {return StatusCode::SUCCESS;}

protected:
  GeoVDetectorManager*   m_detector{nullptr};
};

#endif  // BUILDVP1LIGHT

#endif // GEOMODELSVC_DETDESCRTOOL_H
