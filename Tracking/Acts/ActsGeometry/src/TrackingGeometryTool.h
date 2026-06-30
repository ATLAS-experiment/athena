/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSTRACKINGGEOMETRYTOOL_H
#define ACTSGEOMETRY_ACTSTRACKINGGEOMETRYTOOL_H

// ATHENA
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"

// PACKAGE
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"

// ACTS

namespace Acts {
  class TrackingGeometry;
}



namespace ActsTrk{
class TrackingGeometryTool : public extends<AthAlgTool, ActsTrk::ITrackingGeometryTool> {

    public:
      StatusCode initialize() override;

      using base_class::base_class;
      /** @copydoc ActsTrk::ITrackingGeometryTool::trackingGeometry */
      virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const override;
      /** @copydoc ActsTrk::ITrackingGeometryTool::getGeometryContext */
      virtual const ActsTrk::GeometryContext& getGeometryContext(const EventContext& ctx) const override;
      /** @copydoc ActsTrk::ITrackingGeometryTool::getNominalGeometryContext */
      virtual const ActsTrk::GeometryContext& getNominalGeometryContext() const override;
      /** @copydoc ActsTrk::ITrackingGeometryTool::surfaceIdMap */
      virtual const ActsTrk::DetectorElementToActsGeometryIdMap* surfaceIdMap() const override;
      /** @copydoc ActsTrk::ITrackingGeometryTool::getEnvelope */
      virtual const Acts::TrackingVolume* getEnvelope(const ActsTrk::SystemEnvelope envType) const override;

    private:
      /** @brief Creates and popules the DetectorElement -> Acts::Surface geo identifier map from the geometry service */
      std::unique_ptr<ActsTrk::DetectorElementToActsGeometryIdMap> createDetectorElementToGeoIdMap() const;
     
      ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

      ActsTrk::GeoContextReadKey_t m_rchk{this, "ActsAlignmentKey", "ActsAlignment", "cond read key for the alignment"};

      std::unique_ptr<const ActsTrk::DetectorElementToActsGeometryIdMap> m_detIdMap{};
};
}
#endif
