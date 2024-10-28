
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////////////////////////////
// Package : MdtRawDataMonitoring
// Author:   N. Benekos(Illinois) - G. Dedes(MPI) - Orin Harris (University of Washington)
// Author:   M. Biglietti (INFN Roma3)
//
// DESCRIPTION:
// Subject: MDT-->Offline Muon Data Quality
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef MdtRawDataMonAlg_H
#define MdtRawDataMonAlg_H

// Core Include
#include "AsgTools/ToolHandleArray.h"
#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

// Helper Includes
#include "AthenaMonitoring/DQAtlasReadyFilterTool.h"
#include "MDTChamber.h"
#include "MDTNoisyTubes.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "MuonDQAUtils/MuonDQAHistMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkSegment/SegmentCollection.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonSegment.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"
// standard library includes
#include <atomic>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace Muon {
    class MdtPrepData;
}

// stl includes
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// root includes
class TH2;
struct MDTOverviewHistogramStruct;
struct MDTSummaryHistogramStruct;
struct MDTSegmentHistogramStruct;

namespace monAlg {
    enum { L1_UNKNOWN, L1_BARREL, L1_ENDCAP };

    class TubeTraversedBySegment {
    public:
        TubeTraversedBySegment(const std::string& hn, int tb, bool ih, IdentifierHash idh) :
          hardware_name(hn),
          tubeBin(tb),
          isHit(ih),
          idHash(idh)
        {
        }
        std::string hardware_name;
        int tubeBin;
        bool isHit;
        IdentifierHash idHash;
    };

    // Be careful here -- changes to this can break whether insertions to set are unique
    // JG remove expensive hardware_name comparison w/ idHash comparison
    // Hope I was careful ;)
    struct TubeTraversedBySegment_cmp {
        bool operator()(const TubeTraversedBySegment& A, const TubeTraversedBySegment& B) const {
            if (A.idHash != B.idHash) return A.idHash < B.idHash;

            if (A.tubeBin != B.tubeBin) return A.tubeBin < B.tubeBin;
            return A.isHit < B.isHit;
        }
    };
}  // namespace monAlg

// template <class ConcreteAlgorithm> class AlgFactory;
class MdtRawDataMonAlg : public AthMonitorAlgorithm {
public:
    MdtRawDataMonAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual ~MdtRawDataMonAlg();
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

private:
    static constexpr Identifier::value_type s_detectorElementMask = 0xFFFFC00000000000;

    std::unique_ptr<MDTNoisyTubes> m_masked_tubes{nullptr};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    ToolHandle<CP::IMuonSelectionTool> m_muonSelectionTool;

    // MuonDetectorManager from the conditions store
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                            "Key of input MuonDetectorManager condition data"};

    virtual void fillMDTOverviewVects(const Muon::MdtPrepData*, bool& isNoiseBurstCandidate, MDTOverviewHistogramStruct& vects) const;
    virtual void fillMDTOverviewHistograms(const MDTOverviewHistogramStruct& vects) const;
    virtual StatusCode fillMDTSummaryVects(const Muon::MdtPrepData*, const std::set<std::string>&, bool& isNoiseBurstCandidate,
                                           bool trig_barrel, bool trig_endcap, std::array<MDTSummaryHistogramStruct, 4096>*) const;
    virtual StatusCode fillMDTSummaryHistograms(std::array<MDTSummaryHistogramStruct, 4096>* vects, int lb) const;
    virtual StatusCode fillMDTHistograms(const Muon::MdtPrepData*) const;  // fill chamber by chamber histos

    StatusCode handleEvent_effCalc_fillVects(const Trk::SegmentCollection* segms, MDTSegmentHistogramStruct (&vects)[4][4][16]) const;
    virtual StatusCode fillMDTSegmentHistograms(const MDTSegmentHistogramStruct (&vects)[4][4][16]) const;

    // MDTRawDataUtils_cxx
    static bool AinB(int A, std::vector<int>& B);
    virtual StatusCode binMdtGlobal(TH2*, char ecap);
    virtual StatusCode binMdtRegional(TH2*, std::string_view xAxis);
    virtual StatusCode binMdtGlobal_byLayer(TH2*, TH2*, TH2*);
    virtual StatusCode binMdtOccVsLB(TH2*& h, int region, int layer);
    virtual StatusCode binMdtOccVsLB_Crate(TH2*& h, int region, int crate);
    static void ChamberTubeNumberCorrection(int& tubeNum, std::string_view hardware_name, int tubePos, int numLayers);
    static void CorrectTubeMax(const std::string& hardware_name, int& numTubes);
    static void CorrectLayerMax(const std::string& hardware_name, int& numLayers);
    int get_bin_for_LB_hist(int region, int layer, int phi, int eta, bool isBIM) const;
    int get_bin_for_LB_crate_hist(int region, int layer, int phi, int eta, std::string_view chamber) const;
    // private function to initialize the selection of a certain region
    void mdtchamberId();
    // private function to find mdt mezz cards
    int mezzmdt(const Identifier& id) const;
    int GetTubeMax(const Identifier& digcoll_id, std::string_view hardware_name);

    StatusCode GetTimingInfo();  // here
    void initDeadChannels(const MuonGM::MdtReadoutElement* mydetEl);

    SG::ReadHandleKeyArray<Trk::SegmentCollection> m_segm_type{
        this, "SegmentKey", {"TrkMuonSegments", "UnAssocMuonTrkSegments"}, "muon segments"};
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_muon_type{this, "Muon_type", "ExtrapolatedMuonTrackParticles", "extrapolated muons"};

    std::vector<Identifier> m_chambersId;
    std::vector<IdentifierHash> m_chambersIdHash;
    //  std::map<std::string,float> m_hitsperchamber_map;//DEV to be put back?
    std::map<std::string, int> m_tubesperchamber_map;

    SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_key_mdt{this, "MdtPrepDataContainer", "MDT_DriftCircles", "MDT PRDs"};
    SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_key_rpc{this, "RpcPrepDataContainer", "RPC_Measurements", "RPC PRDs"};
    SG::ReadHandleKey<xAOD::MuonRoIContainer> m_l1RoiKey{this, "L1RoiKey", "LVL1MuonRoIs", "L1 muon ROIs"};
    SG::ReadHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonKey", "Muons", "muons"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfo", "EventInfo", "event info"};

    // Chamber by Chamber Plots
    std::vector<std::unique_ptr<MDTChamber>> m_hist_hash_list{};

    std::string getChamberName(const Muon::MdtPrepData*) const;
    std::string getChamberName(const Identifier&) const;
    StatusCode getChamber(const IdentifierHash& id, MDTChamber*& chamber) const;

    Gaudi::Property<bool> m_doMdtESD{this, "DoMdtEsd", false};
    Gaudi::Property<bool> m_doChamberHists{this, "DoChamberHist", true};
    Gaudi::Property<bool> m_maskNoisyTubes{this, "maskNoisyTubes", true};
    Gaudi::Property<size_t> m_ADCCut{this, "ADCCut", 50};
    Gaudi::Property<size_t> m_nb_hits{this, "Eff_nHits", 5};
    Gaudi::Property<bool> m_chi2_cut{this, "Eff_chi2Cut", 10};
    Gaudi::Property<bool> m_do_mdtChamberHits{this, "do_mdtChamberHits", true};
    Gaudi::Property<bool> m_do_mdttdccut_sector{this, "do_mdttdccut_sector", true};
    Gaudi::Property<bool> m_do_mdtchamberstatphislice{this, "do_mdtchamberstatphislice", true};
    Gaudi::Property<bool> m_do_run3Geometry{this, "do_Run3Geometry", false};
    Gaudi::Property<size_t> m_HighOccThreshold{this, "nHits_NoiseThreshold", 16000};
    Gaudi::Property<size_t> m_adcScale{this, "ADCScale", 1};

    bool m_BMGpresent{false};
    int m_BMGid{-1};
    std::map<Identifier, std::set<Identifier> > m_DeadChannels{};
    mutable std::atomic<int> m_firstEvent{-1};

    bool m_atlas_ready{false};
    bool isATLASReady() const { return m_atlas_ready; }
    void setIsATLASReady();

    int cachedTubeMax(const Identifier& id) const;
    int cachedTubeLayerMax(const Identifier& id) const;
    const MuonGM::MuonDetectorManager* m_detMgr{nullptr};
};

#endif
