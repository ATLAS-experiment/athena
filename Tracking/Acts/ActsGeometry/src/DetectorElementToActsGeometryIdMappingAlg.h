/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_ACTSDETECTORELEMENTTOGEOIDMAPPINGALG_H
#define ACTSTRK_ACTSDETECTORELEMENTTOGEOIDMAPPINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"

namespace ActsTrk {
  class DetectorElementToActsGeometryIdMappingAlg : public AthReentrantAlgorithm {
    public:
      DetectorElementToActsGeometryIdMappingAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual ~DetectorElementToActsGeometryIdMappingAlg();

      StatusCode initialize() override;
      StatusCode execute(const EventContext &ctx) const override;

    private:
     void createDetectorElementToGeoIdMap(const Acts::TrackingGeometry &acts_tracking_geometry,
                                          DetectorElementToActsGeometryIdMap &detector_element_to_geoid) const;

     ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool
        {this, "TrackingGeometryTool", ""};

     SG::WriteCondHandleKey<ActsTrk::DetectorElementToActsGeometryIdMap> m_detectorElementToGeometryIdMapKey
        {this, "DetectorElementToActsGeometryIdMapKey", "DetectorElementToActsGeometryIdMap",
         "Map which associates detector elements to Acts Geometry IDs"};
  };
}
#endif
