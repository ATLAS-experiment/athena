#ifndef MUONCSVDUMP_TruthSegmentCsvDumperAlg_H
#define MUONCSVDUMP_TruthSegmentCsvDumperAlg_H
/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <AthenaBaseComps/AthAlgorithm.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonRecHelperTools/IMuonEDMHelperSvc.h>
#include <StoreGate/ReadHandleKeyArray.h>

#include <xAODMuon/MuonSegmentContainer.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>


namespace MuonR4{
class TruthSegmentCsvDumperAlg: public AthAlgorithm {

  public:
      using AthAlgorithm::AthAlgorithm;
      ~TruthSegmentCsvDumperAlg() = default;
      StatusCode initialize() override;
      StatusCode execute() override;
  
  private:
    const MuonGMR4::SpectrometerSector* msSector(const xAOD::MuonSegment& segment) const;
    SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_inSegmentKey{this, "MuonTruthSegmentsKey", "MuonTruthSegments"};
    
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    ServiceHandle<Muon::IMuonEDMHelperSvc> m_edmHelperSvc{this, "edmHelperSvc", "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc"};

    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
    size_t m_event{0};

};
}
#endif
