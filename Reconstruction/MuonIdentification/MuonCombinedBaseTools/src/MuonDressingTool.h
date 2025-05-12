/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCOMBINEDBASETOOLS_MUONDRESSINGTOOL_H
#define MUONCOMBINEDBASETOOLS_MUONDRESSINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecToolInterfaces/IMuonHitSummaryTool.h"
#include "xAODMuonCnv/IMuonDressingTool.h"

namespace Trk {
    class TrackSummary;
}

namespace MuonCombined {

    class MuonDressingTool : public extends<AthAlgTool, xAOD::IMuonDressingTool> {
    public:
        using base_class::base_class;
        ~MuonDressingTool() = default;

        StatusCode initialize() override;

        void addMuonHitSummary(xAOD::Muon& muon, const Trk::TrackSummary* summary = 0) const override ;

    private:
        ToolHandle<Muon::IMuonHitSummaryTool> m_hitSummaryTool{this, "MuonHitSummaryTool", "Muon::MuonHitSummaryTool/MuonHitSummaryTool"};
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    };

}  // namespace MuonCombined

#endif
