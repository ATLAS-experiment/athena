/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSMSTRACKINGVOLUMEBUILDER_H
#define ACTSGEOMETRY_ACTSMSTRACKINGVOLUMEBUILDER_H

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaBaseComps/AthService.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadHandleKey.h"

#include "ActsGeometryInterfaces/IActsTrackingVolumeBuilder.h"

#include <MuonIdHelpers/IMuonIdHelperSvc.h>

#include <memory>

namespace Acts {
    class CutoutCylinderVolumeBounds;
    class TrackingVolume;
    class VolumeBounds;
    class Surface;
}

namespace MuonGMR4 {
    class Chamber;
}

class ActsGeometryContext;

using volumePtr = std::unique_ptr<Acts::TrackingVolume>;
using surfacePtr = std::shared_ptr<Acts::Surface>;

class ActsMSTrackingVolumeBuilder : public extends<AthAlgTool, IActsTrackingVolumeBuilder> {
    public:
        StatusCode initialize() override;
        using base_class::base_class;
        std::shared_ptr<Acts::TrackingVolume> trackingVolume(const Acts::GeometryContext& gctx,
                                                            std::shared_ptr<const Acts::TrackingVolume> insideVolume = nullptr,
                                                            std::shared_ptr<const Acts::VolumeBounds> outsideBounds = nullptr) const override;

    private:
    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

    std::pair<std::vector<volumePtr>, std::vector<surfacePtr>> constructInternals(const ActsGeometryContext& gctx,
                                              const MuonGMR4::Chamber& mChamber,
                                              std::pair<unsigned int, unsigned int>& chId) const;

    Gaudi::Property<bool> m_constructInternals{this, "ConstructInternals", true, "Construct the internal volumes of the Muon Spectrometer"};
};

#endif
