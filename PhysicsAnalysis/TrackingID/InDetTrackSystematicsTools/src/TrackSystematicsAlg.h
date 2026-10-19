/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKSYSTEMATICSALG_HH
#define TRACKSYSTEMATICSALG_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "AthContainers/ConstDataVector.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ShallowCopyDecorDeps.h"

#include <AsgTools/ToolHandle.h>

#include "InDetTrackSystematicsTools/InclusiveTrackFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthFilterTool.h"
#include "xAODTracking/TrackParticleContainer.h"

namespace InDet {
  /// Writes a view container of the input tracks that survive the track filter
  /// systematics: standard tracks are filtered with SystematicVariationSTD,
  /// LRT tracks with SystematicVariationLRT (at most one variation per track type)
  class TrackSystematicsAlg: public AthReentrantAlgorithm {
  
    public:
  
      TrackSystematicsAlg(const std::string& name,
                          ISvcLocator* pSvcLocator );
  
      StatusCode  initialize() override;
      StatusCode  execute(const EventContext& ctx) const override;
  
    private:

      ToolHandle<InDet::InclusiveTrackFilterTool> m_trackFilterToolLRT{this, "TrackFilterToolLRT", "", "LRT Track filter tool"};

      ToolHandle<InDet::InDetTrackTruthFilterTool> m_trackFilterToolSTD{this, "TrackFilterToolSTD", "", "STD Track filter tool"};

      // LLP secondary vertexing (LLP1, IDTR2) uses TRK_EFF_LOOSE_COMBINED (STD) + TRK_EFF_LARGED0_GLOBAL (LRT)
      // in one container -- prevents proliferation of bulky SV containers in DAOD, but may not capture
      // individual systematic effects!!! Individual variations need one instance each, e.g. "TRK_EFF_LOOSE_GLOBAL"
      Gaudi::Property<std::string> m_systematicVariationSTD{this, "SystematicVariationSTD", "",
        "Track filter systematic for standard tracks (empty: none)"};
      Gaudi::Property<std::string> m_systematicVariationLRT{this, "SystematicVariationLRT", "",
        "Track filter systematic for LRT tracks (empty: none)"};

      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inTrackKey{this, "InputTrackContainer", "InDetLargeD0TrackParticles", "Input track particle container"};
      SG::WriteHandleKey<xAOD::TrackParticleContainer> m_outTrackKey{this, "OutputTrackContainer", "InDetLargeD0TrackParticles_TRK_EFF_LARGED0_GLOBAL__1down", "Output track particle container"};
      SG::ShallowCopyDecorDeps<xAOD::TrackParticleContainer> m_decorDeps { this, "DecorDeps", {}, "List of decorations to propagate through the view container" };
  
  };
}
#endif


