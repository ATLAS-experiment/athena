/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRY_ACTSDETALIGNCONDALG_H
#define ACTSGEOMETRY_ACTSDETALIGNCONDALG_H

#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometryInterfaces/DetectorAlignStore.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

/**
 *  The DetectorAlignCondAlg loads the rigid alignment corrections and pipes them through the 
 *  readout geometry to cache the final transformations of the sensor surfaces associated to one
 *  particular detector technology (Pixel, Sct, etc.). The transformations are cached in the 
 *  DetectorAlignmentStore which is later propagated to the GeometryContext.
 * 
 */
namespace ActsTrk{
  class DetectorAlignCondAlg : public AthReentrantAlgorithm {
  public:
      /// Standard constructor
      using AthReentrantAlgorithm::AthReentrantAlgorithm;
 
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
      ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeoSvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
      /// Flag determining the subdetector. Needs to be static castable to DetectorType
      Gaudi::Property<int> m_detType{this, "DetectorType", static_cast<int>(DetectorType::UnDefined)};
      /// Flag toggling whether the alignment store shall be filled with the transforms or not
      Gaudi::Property<bool> m_fillAlignStoreCache{this, "FillAlignCache", true};
      /// Static cast of >DetectorType< property
      DetectorType m_Type{DetectorType::UnDefined};

  };
}
#endif