/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSTRACKINGGEOMETRYTOOL_H
#define ACTSGEOMETRY_ACTSTRACKINGGEOMETRYTOOL_H

// ATHENA
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"

// PACKAGE
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometrySvc.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"

// ACTS

namespace Acts {
  class TrackingGeometry;
}




class ActsTrackingGeometryTool : public extends<AthAlgTool, IActsTrackingGeometryTool> {

    public:
      StatusCode initialize() override;

      using base_class::base_class;

      virtual std::shared_ptr<const Acts::TrackingGeometry> trackingGeometry() const override;

      virtual const ActsGeometryContext& getGeometryContext(const EventContext& ctx) const override;

      virtual const ActsGeometryContext& getNominalGeometryContext() const override;

      virtual const ActsTrk::DetectorElementToActsGeometryIdMap* surfaceIdMap() const override;
    private:
      /** @brief Creates and popules the DetectorElement -> Acts::Surface geo identifier map from the geometry service */
      std::unique_ptr<ActsTrk::DetectorElementToActsGeometryIdMap> createDetectorElementToGeoIdMap() const;
     
      ServiceHandle<IActsTrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};

      SG::ReadHandleKey<ActsGeometryContext> m_rchk{this, "ActsAlignmentKey", "ActsAlignment", "cond read key for the alignment"};

      std::unique_ptr<const ActsTrk::DetectorElementToActsGeometryIdMap> m_detIdMap{};
};

#endif
