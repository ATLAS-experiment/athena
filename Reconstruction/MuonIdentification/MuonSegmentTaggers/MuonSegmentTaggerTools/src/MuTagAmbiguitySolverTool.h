/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuTagAmbiguitySolverTool_H
#define MuTagAmbiguitySolverTool_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonCombinedEvent/MuonSegmentInfo.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonSegmentMakerToolInterfaces/IMuonSegmentMatchingTool.h"
#include "MuonSegmentTaggerToolInterfaces/IMuTagAmbiguitySolverTool.h"
#include "TrkSegment/SegmentCollection.h"

/** @class MuTagAmbiguitySolverTool */

namespace Muon {
    class MuonSegment;
}

class MuTagAmbiguitySolverTool :  public extends<AthAlgTool, IMuTagAmbiguitySolverTool> {
public:
    MuTagAmbiguitySolverTool(const std::string& t, const std::string& n, const IInterface* p);
    virtual ~MuTagAmbiguitySolverTool() = default;

    virtual StatusCode initialize() override;

    virtual std::vector<MuonCombined::MuonSegmentInfo> solveAmbiguities(const EventContext& ctx, std::vector<MuonCombined::MuonSegmentInfo> mtos) const override;

    virtual std::vector<MuonCombined::MuonSegmentInfo> selectBestMuTaggedSegments(const EventContext& ctx, const std::vector<MuonCombined::MuonSegmentInfo>& mtss) const override;

private:
    ///////////////////////////////////
    int ambiguousSegment(const EventContext& ctx, const Muon::MuonSegment& seg1, const Muon::MuonSegment& seg2) const;
    virtual double Rseg(unsigned int nseg) const override;

    ServiceHandle<Muon::IMuonEDMHelperSvc> m_edmHelperSvc{
        this,
        "edmHelper",
        "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
        "Handle to the service providing the IMuonEDMHelperSvc interface",
    };  //!< Pointer on IMuonEDMHelperSvc
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this,
        "MuonIdHelperSvc",
        "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
    };

    PublicToolHandle<Muon::MuonEDMPrinterTool> m_muonPrinter{this,"Printer", ""};  //!< Pointer on MuonEDMPrinterTool
    ToolHandle<Muon::IMuonSegmentMatchingTool> m_segmentMatchingTool{
        this,
        "MuonSegmentMatchingTool",
        "Muon::MuonSegmentMatchingTool/MuonSegmentMatchingTool",
    };  //!< Pointer on MuonSegmentMatchingTool

    //!< check hit overlap of segments in ambi solving
    Gaudi::Property<bool> m_hitOverlapMatching{this, "DoHitOverlapMatching", true};
    //!< for segments in a SL overlap in the same station layer, check whether from same particle
    Gaudi::Property<bool> m_slOverlapMatching{"ResolveSLOverlaps",  false};
    //!< reject Endcap Outer one station layer tags (without EI or EM)
    Gaudi::Property<bool> m_rejectOuterEndcap{this, "RejectOuterEndcap", true};
    //!< reject one station tags with phi hits and a fabs(MatchPhi = minPullPhi) > 3
    Gaudi::Property<bool> m_rejectMatchPhi{this, "RejectMatchPhi",  true};
};

#endif  // MuTagAmbiguitySolverTool_H
