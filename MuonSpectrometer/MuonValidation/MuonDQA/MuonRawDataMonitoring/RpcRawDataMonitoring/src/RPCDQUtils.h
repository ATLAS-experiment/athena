/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// -*- c++ -*-
#ifndef RPCRAWDATAMONITORING_RPCDQUtils_H
#define RPCRAWDATAMONITORING_RPCDQUtils_H

// ROOT
#include "TH1.h"
#include "TH1F.h"

// Athena
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "xAODMuon/MuonContainer.h"

//================================================================================================
struct RpcPanel {
    RpcPanel() = default;
    RpcPanel(Identifier id, const RpcIdHelper &rpcIdHelper);
    RpcPanel(const Muon::IMuonIdHelperSvc &idHelperSvc,
             const MuonGM::RpcReadoutElement *readoutEl_, const int doubletZ_,
             const int doubletPhi_, const int gasgap_, const int measPhi_);

    const MuonGM::RpcReadoutElement *readoutEl{nullptr};

    Identifier panelId{0};
    bool panel_valid{false};
    std::string panel_str{};
    std::string panel_name{};
    std::string stationNameStr{};

    void FillRpcId(Identifier id, const RpcIdHelper &rpcIdHelper);
    void SetPanelIndex(int index);

    int stationName{0};
    int stationEta{0};
    int stationPhi{0};
    int doubletR{0};
    int doubletZ{0};
    int doubletPhi{0};
    int gasGap{0};
    int measPhi{-1};
    int panel_index{-1};
    unsigned NHit_perEvt{0};

    std::string getOnlineConvention() const;
    std::pair<int, int> getSectorLayer() const;
    std::string getElementStr() const;

    bool operator==(const RpcPanel &rhs) const;

    bool operator<(const RpcPanel &rhs) const;

    bool operator>(const RpcPanel &rhs) const;
};

//================================================================================================
struct TagDef {
    TString eventTrig;
    TString tagTrig;
};

//================================================================================================
struct MyMuon {
    const xAOD::Muon *muon;
    TLorentzVector fourvec;

    bool tagged{false};
    bool isolated{false};
    bool tagProbeOK{false};
    bool tagProbeAndZmumuOK{false};
    bool isZmumu{false};

    std::set<int> matchedL1ThrExclusive;
    std::set<int> matchedL1ThrInclusive;
};

//================================================================================================
struct ExResult {
    ExResult(const Identifier gasgap_id_, const Trk::PropDirection direction_);

    const Identifier gasgap_id;
    const Trk::PropDirection direction;

    bool localTrackPosInBounds{false};
    bool localTrackPosInBoundsTight{false};
    Amg::Vector3D localPos;
    Amg::Vector3D globalPos;
    float minTrackGasGapDEta{10.0};
    float minTrackGasGapDPhi{10.0};
    float minTrackGasGapDR{10.0};
};

//================================================================================================
struct GasGapData {
    GasGapData(const Muon::IMuonIdHelperSvc &idHelperSvc,
               const MuonGM::RpcReadoutElement *readoutEl_, const int doubletZ_,
               const int doubletPhi_, const unsigned gasgap_);

    void computeTrackDistanceToGasGap(ExResult &result,
                                      const xAOD::TrackParticle &track) const;
    void computeTrackDistanceToGasGap(
        ExResult &result, const Trk::TrackParameters *trackParam) const;

    const MuonGM::RpcReadoutElement *readoutEl;

    Identifier gapid{0};
    bool gapid_valid{false};
    std::string gapid_str;

    int stationName;
    int stationEta;
    int stationPhi;
    int doubletR;
    int doubletZ;
    int doubletPhi;
    unsigned gasgap;

    unsigned NetaStrips{0};
    unsigned NphiStrips{0};
    double minStripEta{10.0};
    double maxStripEta{-10.0};
    double minStripPhi{10.0};
    double maxStripPhi{-10.0};
    double localTrackPosY{0.0};
    double localTrackPosZ{0.0};

    std::pair<std::shared_ptr<RpcPanel>, std::shared_ptr<RpcPanel>>
        RpcPanel_eta_phi;
};

#endif
