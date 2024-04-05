/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRY_ACTSDETALIGNCONDALG_H
#define ACTSGEOMETRY_ACTSDETALIGNCONDALG_H

#include "ActsGeometryInterfaces/IActsTrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/IDetectorVolumeSvc.h"
#include "ActsGeometryInterfaces/DetectorAlignStore.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

/**
 *  The DetectorAlignCondAlg loads the rigid alignment corrections and pipes them through the 
 *  readout geometry to cache the final transformations of the sensor surfaces associated to one
 *  particular detector technology (Pixel, Sct, etc.). The transformations are cached in the 
 *  DetectorAlignmentStore which is later propagated to the ActsGeometryContext.
 * 
 */
namespace ActsTrk{
  class DetectorAlignCondAlg : public AthReentrantAlgorithm {
  public:
      /// Standard constructor
      DetectorAlignCondAlg(const std::string& name, ISvcLocator* pSvcLocator);

      virtual ~DetectorAlignCondAlg();

      StatusCode initialize() override final;

      StatusCode execute(const EventContext& ctx) const override final;
      /// Switch off reentrancy to avoid condition clashes
      bool isReEntrant() const override final { return false; }

  private:
      /// Key to the alignment transformations for the detector volumes
      SG::ReadCondHandleKey<GeoAlignmentStore> m_inputKey{this, "InputTransforms", ""};
      /// Key to the alignment transformations written by the alg
      SG::WriteCondHandleKey<DetectorAlignStore> m_outputKey{this, "ActsTransforms", ""};
      /// ServiceHandle to the ActsTrackingGeometry
      ServiceHandle<IActsTrackingGeometrySvc> m_trackingGeoSvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
      /// ServiceHandle to the IDetectorVolumeSvc
      ServiceHandle<ActsTrk::IDetectorVolumeSvc> m_detVolSvc{this,"DetectorVolumeSvc", "DetectorVolumeSvc"};
      /// Flag determining the subdetector. Needs to be static castable to DetectorType
      Gaudi::Property<int> m_detType{this, "DetectorType", static_cast<int>(DetectorType::UnDefined)};
      /// Flag toggling whether the alignment store shall be filled with the transforms or not
      Gaudi::Property<bool> m_fillAlignStoreCache{this, "FillAlignCache", true};
      /// Static cast of >DetectorType< property
      DetectorType m_Type{DetectorType::UnDefined};

      Gaudi::Property<bool> m_loadTrkGeoSvc{this, "LoadTrackingGeoSvc", true, 
                                            "Toggle whether the tracking geometry svc shall be retrieved"};

      Gaudi::Property<bool> m_loadDetVolSvc{this, "LoadDetectorVolumeSvc", false, 
                                            "Toggle whether the detector volume svc shall be retrieved"};

  };
}
#endif