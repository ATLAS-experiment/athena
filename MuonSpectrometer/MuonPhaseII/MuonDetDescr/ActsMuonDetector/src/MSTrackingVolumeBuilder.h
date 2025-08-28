/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMUONDETECTOR_MSTRACKINGVOLUMEBUILDER_H
#define ACTSMUONDETECTOR_MSTRACKINGVOLUMEBUILDER_H

#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include "AthenaBaseComps/AthAlgTool.h"

#include "ActsGeometryInterfaces/IActsTrackingVolumeBuilder.h"

#include <MuonIdHelpers/IMuonIdHelperSvc.h>

#include <memory>

namespace Acts {
    class TrackingVolume;
    class VolumeBounds;
    class Surface;
}


class ActsGeometryContext;


namespace ActsTrk{
    class MSTrackingVolumeBuilder : public extends<AthAlgTool, IActsTrackingVolumeBuilder> {
        public:
            StatusCode initialize() override;
            using base_class::base_class;
            std::shared_ptr<Acts::TrackingVolume> trackingVolume(const Acts::GeometryContext& gctx,
                                                            std::shared_ptr<const Acts::TrackingVolume> insideVolume = nullptr,
                                                            std::shared_ptr<const Acts::VolumeBounds> outsideBounds = nullptr) const override;
 
        private:
            Gaudi::Property<unsigned> m_firstLayId{this, "FirstLayId", 1};
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

    };
}
#endif
