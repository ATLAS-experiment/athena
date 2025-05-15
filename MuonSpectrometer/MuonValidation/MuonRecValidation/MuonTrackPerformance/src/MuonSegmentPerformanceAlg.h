/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSEGMENTPERFORMANCEALG_H
#define MUONSEGMENTPERFORMANCEALG_H

#include <fstream>
#include <string>
#include <vector>
#include <array>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "StoreGate/ReadDecorHandleKey.h"

class MuonSegmentPerformanceAlg : public AthAlgorithm {
public:
    // Algorithm Constructor
    MuonSegmentPerformanceAlg(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~MuonSegmentPerformanceAlg() = default;

    // Gaudi algorithm hooks
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

    unsigned int cardinality() const override final { return 1;}
private:
    constexpr static unsigned s_chIdxMax = Muon::MuonStationIndex::toInt(Muon::MuonStationIndex::ChIndex::ChIndexMax);
    using counter_t = std::array<int, s_chIdxMax>;
    std::string printRatio(const std::string& prefix, unsigned int begin, unsigned int end, const counter_t& reco,
                           const counter_t& truth) const;
    std::string printRatio(const std::string& prefix, unsigned int begin, unsigned int end, const counter_t& reco) const;

    /** name of external file to write statistics */
    bool m_writeToFile;
    std::string m_fileName;

    /** output file*/
    std::ofstream m_fileOutput;

    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentLocation", "MuonSegments"};
    SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegmentKey{this, "TruthSegmentLocation", "MuonTruthSegments"};
    SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_truthSegmenLinkKey{this, "truthSegmentsRecoSegmentLinkKey", m_truthSegmentKey, "recoSegmentLink" };

    unsigned int m_nevents;
    std::vector<int> m_nhitCuts;
    std::vector<std::string> m_hitCutString;
    std::vector<counter_t > m_ntruth;
    std::vector<counter_t > m_nfound;
    std::vector<counter_t > m_nfake;
};

#endif  // MUONPERFORMANCEALG
