/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSCALOTRACKINGVOLUMEBUILDER_H
#define ACTSGEOMETRY_ACTSCALOTRACKINGVOLUMEBUILDER_H

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaBaseComps/AthService.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadHandleKey.h"

#include "ActsGeometryInterfaces/IActsTrackingVolumeBuilder.h"

#include <memory>

// ACTS
#include "Acts/Geometry/Volume.hpp"
#include "Acts/Geometry/GeometryContext.hpp"

class CaloDetDescrManager;

namespace Acts {
class TrackingVolume;
class VolumeBounds;
class CutoutCylinderVolumeBounds;


}

class ActsCaloTrackingVolumeBuilder : public extends<AthAlgTool, IActsTrackingVolumeBuilder>
{
public:
  StatusCode initialize() override;
  ActsCaloTrackingVolumeBuilder(const std::string& type,
                                const std::string& name,
                                const IInterface* parent);

  std::shared_ptr<Acts::TrackingVolume>
  trackingVolume(const Acts::GeometryContext& gctx,
                 std::shared_ptr<const Acts::TrackingVolume> insideVolume = nullptr,
                 std::shared_ptr<const Acts::VolumeBounds> outsideBounds = nullptr) const override;

private:

  Acts::Volume
  build_endcap(double z, double dz, double eta, double deta, double phi, double dphi) const;

  Acts::Volume
  build_barrel(double r, double dr, double eta, double deta, double phi, double dphi) const;

  Acts::Volume
  build_box(double x, double dx, double y, double dy, double z, double dz) const;

  std::vector<std::unique_ptr<Acts::Volume>>
  cellFactory() const;

  std::shared_ptr<Acts::CutoutCylinderVolumeBounds>
  makeCaloVolumeBounds(const std::vector<std::unique_ptr<Acts::Volume::BoundingBox>>& boxStore,
                       std::shared_ptr<const Acts::TrackingVolume> insideVolume) const;


  const CaloDetDescrManager* m_caloMgr{nullptr};
};

#endif
