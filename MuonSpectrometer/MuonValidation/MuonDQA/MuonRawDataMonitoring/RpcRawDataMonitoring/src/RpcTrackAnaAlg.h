/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RPCRAWDATAMONITORING_RPCTRACKANAALG_H
#define RPCRAWDATAMONITORING_RPCTRACKANAALG_H

#include <time.h>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

// Athena/Gaudi
#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

// ATLAS
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/RpcPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonTrigCoinData/RpcCoinDataContainer.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkParameters/TrackParameters.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"

// local
#include "RPCDQUtils.h"

class RpcTrackAnaAlg : public AthMonitorAlgorithm {
   public:
    RpcTrackAnaAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~RpcTrackAnaAlg();

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

    typedef std::map<Identifier, std::shared_ptr<RpcPanel>> RpcPanelMap;
    typedef std::pair<ExResult, const std::shared_ptr<GasGapData>> GasGapResult;

   private:
    enum BarrelDL {
        BI = 1,
        BM1,
        BM2,
        BO1,
        BO2,
        OUT
    };  // Barrel doublet: BM_dbR
    enum MuonSource { AllMuon = 0, ZCand };

    StatusCode readElIndexFromXML();
    StatusCode initRpcPanel();
    StatusCode setPanelIndex(std::shared_ptr<RpcPanel> panel);

    StatusCode initTrigTag();
    StatusCode initArrayHistosMap();

    StatusCode fillMuonExtrapolateEff(const EventContext& ctx) const;
    StatusCode fillHistPRD(const EventContext& ctx) const;

    StatusCode triggerMatching(const xAOD::Muon*,
                               const std::vector<TagDef>&) const;

    StatusCode extrapolate2RPC(const xAOD::TrackParticle* track,
                               const Trk::PropDirection direction,
                               std::vector<GasGapResult>& results,
                               BarrelDL barrelDL) const;
    std::unique_ptr<Trk::TrackParameters> computeTrackIntersectionWithGasGap(
        ExResult& result, const xAOD::TrackParticle* track_particle,
        const std::shared_ptr<GasGapData>& gap) const;

    StatusCode extrapolate2RPC(std::unique_ptr<Trk::TrackParameters> trackParam,
                               const Trk::PropDirection direction,
                               std::vector<GasGapResult>& results,
                               BarrelDL barrelDL) const;
    std::unique_ptr<Trk::TrackParameters> computeTrackIntersectionWithGasGap(
        ExResult& result, const Trk::TrackParameters* trackParam,
        const std::shared_ptr<GasGapData>& gap) const;

    StatusCode readHitsPerGasgap(const EventContext& ctx,
                                 std::vector<GasGapResult>& results,
                                 MuonSource muon_source) const;
    StatusCode fillClusterSize(std::vector<const Muon::RpcPrepData*>& view_hits,
                               const int panel_index, int LB, int phiSector,
                               int isPhi) const;
    bool IsNearbyHit(const std::vector<const Muon::RpcPrepData*>& cluster_hits,
                     const Muon::RpcPrepData* hit) const;

   private:
    BooleanProperty m_plotMuonEff{
        this, "plotMuonEff", false,
        "switch to plot histograms for Muon Efficiency"};
    BooleanProperty m_plotPRD{
        this, "plotPRD", false,
        "switch to plot histograms for Prepare Data objects"};
    BooleanProperty m_useAODParticle{this, "useAODParticle", false,
                                     "use AOD Particle"};

    FloatProperty m_lbDuraThr{this, "lbDuraThr", 10.,
                              "Thrshold of luminosity block deruation"};
    StringProperty m_packageName{this, "PackageName", "RpcTrackAnaAlg",
                                 "group name for histograming"};

    StringProperty m_elementsFileName{this, "ElementsFileName", "RpcRawDataMonitoring/Element.xml",
                                      "Elements xml file"};

    StringProperty m_trigTagList{
        this, "TagTrigList", "HLT_mu_ivarmedium;HLT_mu50",
        "list of triggers to be used for trigger matching"};
    FloatProperty m_trigMatchWindow{this, "TrigMatchingWindow", 0.005,
                                    "Window size in R for trigger matching"};
    BooleanProperty m_TagAndProbe{this, "TagAndProbe", false,
                                  "switch to perform tag-and-probe method"};
    BooleanProperty m_TagAndProbeZmumu{
        this, "TagAndProbeZmumu", false,
        "switch to perform tag-and-probe method Z->mumu"};

    // cuts for muons
    FloatProperty m_minPt{this, "minPt", 25.0e3, "minmum pT of muon"};
    FloatProperty m_maxEta{this, "maxEta", 2.5,
                           "max eta absolute value of muon"};

    // cuts for barrel muons
    FloatProperty m_barrelMinPt{this, "barrelMinPt", 2.0e3};
    FloatProperty m_barrelMinEta{this, "barrelMinEta", 0.1};
    FloatProperty m_barrelMaxEta{this, "barrelMaxEta", 1.05};

    FloatProperty m_muonMass{this, "MuonMass", 105.6583755,
                             "muon invariant mass in MeV"};
    FloatProperty m_zMass_lowLimit{
        this, "zMass_lowLimit", 50000.,
        "2 muon invariant mass low limit in Zmumu event"};
    FloatProperty m_zMass_upLimit{
        this, "zMass_upLimit", 150000.,
        "2 muon invariant mass up limit in Zmumu event"};

    FloatProperty m_isolationWindow{
        this, "IsolationWindow", 0.1,
        "Window size in R for isolation with other muons"};
    FloatProperty m_l1trigMatchWindow{
        this, "L1TrigMatchingWindow", 0.3,
        "Window size in R for L1 trigger matching"};
    // StringProperty
    // m_MuonEFContainerName{this,"MuonEFContainerName","HLT_MuonsCBOutsideIn","HLT
    // RoI-based muon track container"};
    FloatProperty m_minDRTrackToGasGap{
        this, "minDRTrackToGasGap", 0.02,
        "minimum of DR between track and gasgap"};

    FloatProperty m_boundsToleranceReadoutElement{
        this, "boundsToleranceReadoutElement", 100.0,
        "boundsToleranceReadoutElement"};
    FloatProperty m_boundsToleranceReadoutElementTight{
        this, "boundsToleranceReadoutElementTight", 20.0,
        "boundsToleranceReadoutElementTight"};

    FloatProperty m_diffHitTrackPostion{
        this, "diffHitTrackPostion", 30.0,
        "the largest distance between hit and tracks local postion"};
    FloatProperty m_outtime{this, "outtime", 12.5, "the out-time time"};

    ///////////////////////////////////////////////////////////////////
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    const RpcIdHelper* m_rpcIdHelper{nullptr};

    ToolHandle<Trk::IExtrapolator> m_extrapolator{
        this, "TrackExtrapolator", "Trk::Extrapolator/AtlasExtrapolator",
        "Track extrapolator"};
    SG::ReadHandleKey<xAOD::MuonRoIContainer> m_MuonRoIContainerKey{
        this, "MuonRoIContainerName", "LVL1MuonRoIs", "Key for L1 ROIs"};
    SG::ReadHandleKey<xAOD::MuonContainer> m_MuonContainerKey{
        this, "MuonContainerKey", "Muons",
        "Key for Offline muon track Containers"};
    SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_rpcPrdKey{
        this, "RpcPrepDataContainer", "RPC_Measurements", "RPC PRDs"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_PrimaryVertexContainerKey{
        this, "PrimaryVertexContainerName", "PrimaryVertices",
        "Primary Vertex Container"};

    // accessors for beam spot uncertainty
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_beamSigmaX{
        this, "beamPosSigmaX", "EventInfo.beamPosSigmaX",
        "Beam spot position sigma in X"};
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_beamSigmaY{
        this, "beamPosSigmaY", "EventInfo.beamPosSigmaY",
        "Beam spot position sigma in Y"};
    // note that this last entry is a covariance: the units are mm^2,
    // whereas the above have units of mm
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_beamSigmaXY{
        this, "beamPosSigmaXY", "EventInfo.beamPosSigmaXY",
        "Beam spot covariance in XY"};

    RpcPanelMap m_rpcPanelMap{};

    std::vector<TagDef> m_trigTagDefs{};

    std::map<std::pair<int, int>, std::vector<std::shared_ptr<GasGapData>>>
        m_gasGapData{};

    // 2=BML,3=BMS,4=BOL,5=BOS,8=BMF,9=BOF,10=BOG,53=BME
    std::map<BarrelDL, std::vector<int>> m_StationNames{};
    std::map<std::string, int> m_elementIndex{};

    std::map<std::string, int> m_SectorGroup{};
    std::map<std::string, int> m_TriggerThrGroup{};
};

#endif
