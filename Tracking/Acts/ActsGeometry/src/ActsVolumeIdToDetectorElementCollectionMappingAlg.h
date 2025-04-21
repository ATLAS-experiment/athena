/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_ACTSGEOMETRYIDTODETECTORELEMENTMAPPINGALG_H
#define ACTSTRK_ACTSGEOMETRYIDTODETECTORELEMENTMAPPINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "StoreGate/CondHandleKeyArray.h"

#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsGeometry/ActsVolumeIdToDetectorElementCollectionMap.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

namespace ActsTrk {
  class ActsVolumeIdToDetectorElementCollectionMappingAlg : public AthReentrantAlgorithm {
    public:
      ActsVolumeIdToDetectorElementCollectionMappingAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual ~ActsVolumeIdToDetectorElementCollectionMappingAlg();

      StatusCode initialize() override;
      StatusCode execute(const EventContext &ctx) const override;

    private:
     void createDetectorElementToVolumeIdMap(const Acts::TrackingGeometry &acts_tracking_geometry,
                                             std::unordered_map<unsigned long long,
                                                                unsigned int> &detector_element_to_volume_id) const;

     ToolHandle<IActsTrackingGeometryTool> m_trackingGeometryTool
        {this, "TrackingGeometryTool", ""};

    SG::ReadCondHandleKeyArray<InDetDD::SiDetectorElementCollection> m_detEleCollKeys
       {this, "DetectorElementsKeys", {}, "Keys of input SiDetectorElementCollection"};

    SG::WriteCondHandleKey<ActsTrk::ActsVolumeIdToDetectorElementCollectionMap> m_volumeIdToDetectorElementCollMapKey
       {this, "ActsVolumeIdToDetectorElementCollectionMap", "ActsVolumeIdToDetectorElementCollectionMap",
        "Map which associates Acts geometry volume IDs to detector element collections."};
  };
}
#endif
