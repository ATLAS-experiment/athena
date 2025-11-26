/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 2020 Matthias Schott - Uni Mainz
*/

#ifndef MUON_TRACKALGORITHM_H
#define MUON_TRACKALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"

#include "xAODMuon/MuonContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include <vector>
#include <string>
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"


// AthMonitorAlgorithm
class MuonTrackMonitorAlgorithm : public AthMonitorAlgorithm
{

  public:

    MuonTrackMonitorAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);

    virtual ~MuonTrackMonitorAlgorithm() {};
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

  private:

    SG::ReadHandleKey<xAOD::MuonContainer> m_MuonContainerKey{this, "MuonContainerKey", "Muons", "Key for Muon Containers" };
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_MuonIsoDecorKey{this, "MuonIsoDecorKey", "Muons.ptcone30" };
    SG::ReadHandleKey<xAOD::VertexContainer> m_VertexContainerKey{this, "PrimaryVerticesKey", "PrimaryVertices", "Key for primary VertexContainers"};
    SG::ReadHandleKey<xAOD::EventInfo> m_derEventInfoKey{this, "EventInfo", "EventInfo", ""};
    SG::ReadDecorHandleKeyArray<xAOD::EventInfo> m_beamSpotKey{this, "BeamSpotKeys" , m_derEventInfoKey, {"beamPosSigmaX", "beamPosSigmaY", "beamPosSigmaZ", "beamPosSigmaXY"}, "Add the scheduler dependencies on the beamspot information"};

    Gaudi::Property<bool> m_useBeamSpot{this, "RequireBeamSpot", true, "Ensure that the dependency on the beamspot variables is established."};

    // Tools
    /// Fills data-quality information (e.g. pt, eta, phi..) to histograms for given selection of muons
    /// std::string sIdentifier = "CB","ZBoson","Jpsi": String which is used to match the histogramming
    ///    variables that are defined by the Python script
    /// std::vector<const xAOD::Muon*>	&vecMuons: Vector of muons for which performance plots should be created
    StatusCode	FillMuonInformation(const std::string& sIdentifier, std::vector<const xAOD::Muon*>	&vecMuons, const xAOD::Vertex *pvtx, const xAOD::EventInfo &evt) const;

    /// Function to fill low level Track information
    StatusCode FillTrackInformation(const std::string& sIdentifier, const xAOD::Muon* muon, const xAOD::Vertex *pvtx, const std::string& sTrack, const xAOD::EventInfo &evt) const;

    /// Function to create performance plots for muon standalone tracks with some detailed informatiom
    //StatusCode analyseLowLevelMuonFeatures(const std::string& sIdentifier, const xAOD::MuonContainer& Muons, uint32_t lumiBlockID) const;
    StatusCode analyseLowLevelMuonFeatures(const std::string& sIdentifier, std::vector<const xAOD::Muon*> &Muons, const xAOD::EventInfo &evt) const;

    /// Function to create performance plots for all combined muons
    StatusCode analyseCombinedTracks(const xAOD::MuonContainer& Muons, const xAOD::Vertex *pvtx, const xAOD::EventInfo &evt) const;

    /// Function to get the primary vertex
    const xAOD::Vertex* getPrimaryVertex(const xAOD::VertexContainer& Vertices) const;

    /// Function to get the track category ID for the given identifier
    int getTrackCategoryID(const std::string& sIdentifier) const;

    /// Function to create performance plots for all combined muons that lead to a Z Boson Candidate event
    StatusCode analyseResonanceCandidates(const xAOD::MuonContainer& Muons, const xAOD::Vertex *pvtx, const xAOD::EventInfo &evt) const;

    /// Function to create performance plots for all combined muons that lead to a Jpsi Meson Candidate event
    StatusCode plotResonanceCandidates(const std::string& resonanceName, std::vector<const xAOD::Muon*>& muonCandidates, const xAOD::Vertex *pvtx, const xAOD::EventInfo &evt) const;

    Gaudi::Property< std::vector<std::string> > m_hltchainList{ this, "HLTTriggerList", {"HLT_2mu14_L12MU8F", "HLT_mu24_ivarmedium_L1MU14FCH"}, "High-level triggers used" };

    Gaudi::Property< float > m_CBmuons_minPt{ this, "CBmuons_minPt", 20000., "Minimal muon pt used for CB muons" };

    Gaudi::Property< float > m_ZBosonSelection_minPt{ this, "ZBosonSelection_minPt", 20000., "Minimal muon pt used for Z analysis" };
    Gaudi::Property< float > m_ZBosonSelection_maxEta{ this, "ZBosonSelection_maxEta", 2.5, "Maximal muon eta used for Z analysis" };
    Gaudi::Property< float > m_ZBosonSelection_trkIsolation{ this, "ZBosonSelection_trkIsolation", 0.2, "Track DeltaR isolation criteria" };
    Gaudi::Property< float > m_ZBosonSelection_D0Cut{ this, "ZBosonSelection_D0Cut", 100., "D0 cut applied for Z boson analysis" };
    Gaudi::Property< float > m_ZBosonSelection_Z0Cut{ this, "ZBosonSelection_Z0Cut", 100., "Z0 cut applied for Z boson analysis" };
    Gaudi::Property< float > m_ZBosonSelection_minMass{ this, "ZBosonSelection_minMass", 76000., "Minimal accepted Z  boson mass" };
    Gaudi::Property< float > m_ZBosonSelection_maxMass{ this, "ZBosonSelection_maxMass", 106000., "Maximal accepted Z  boson mass" };

    Gaudi::Property< float > m_JpsiSelection_minPt{ this, "JpsiSelection_minPt", 4000., "Minimal muon pt used for Jpsi analysis" };
    Gaudi::Property< float > m_JpsiSelection_maxEta{ this, "JpsiSelection_maxEta", 2.5, "Maximal muon eta used for Jpsi analysis" };
    Gaudi::Property< float > m_JpsiSelection_trkIsolation{ this, "JpsiSelection_trkIsolation", 1.0, "Jpsi track DeltaR isolation criteria" };
    Gaudi::Property< float > m_JpsiSelection_D0Cut{ this, "JpsiSelection_D0Cut", 100., "D0 cut applied for Jpsi analysis" };
    Gaudi::Property< float > m_JpsiSelection_Z0Cut{ this, "JpsiSelection_Z0Cut", 100., "Z0 cut applied for Jpsi analysis" };
    Gaudi::Property< float > m_JpsiSelection_minMass{ this, "JpsiSelection_minMass", 2600, "Minimal accepted Jpsi mass" };
    Gaudi::Property< float > m_JpsiSelection_maxMass{ this, "JpsiSelection_maxMass", 3600, "Maximal accepted Jpsi mass" };
};


#endif

