/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ****************************************************************************
// ----------------------------------------------------------------------------
// JpsiFinder header file
//
// James Catmore <James.Catmore@cern.ch>

// ----------------------------------------------------------------------------
// ****************************************************************************
#ifndef JPSIFINDER_H
#define JPSIFINDER_H
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GeneratorModules/GenData.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/Muon.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "JpsiUpsilonTools/ICandidateSearch.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "InDetConversionFinderTools/VertexPointEstimator.h"
#include <memory>

/////////////////////////////////////////////////////////////////////////////

namespace Analysis {
    
    // Struct and enum to associate muon pairs with track pairs
    // and make the program flow more straightforward
    enum PairType{ MUMU=0, MUTRK=1, TRKTRK=2};
    enum MuonTypes{ CC=0, CT=1, TT=2};
    struct JpsiCandidate
    {
        const xAOD::TrackParticle* trackParticle1 = nullptr;
        const xAOD::TrackParticle* trackParticle2 = nullptr;
        const xAOD::Muon* muon1 = nullptr;
        const xAOD::Muon* muon2 = nullptr;
        const xAOD::TrackParticleContainer* collection1 = nullptr;
        const xAOD::TrackParticleContainer* collection2 = nullptr;
        PairType pairType{MUMU};
        MuonTypes muonTypes{CC};
    };
    
    class JpsiFinder:  public extends<AthAlgTool, Analysis::ICandidateSearch>
    {
    public:
        JpsiFinder(const std::string& t, const std::string& n, const IInterface*  p);
        ~JpsiFinder();
        virtual StatusCode initialize() override;
        
        //-------------------------------------------------------------------------------------
        //Doing Calculation and inline functions

        virtual StatusCode performSearch(const EventContext& ctx, xAOD::VertexContainer& vxContainer) const override;
        std::vector<JpsiCandidate> getPairs(const std::vector<const xAOD::TrackParticle*>&) const;
        std::vector<JpsiCandidate> getPairs(const std::vector<const xAOD::Muon*>&) const;
        std::vector<JpsiCandidate> getPairs2Colls(const std::vector<const xAOD::TrackParticle*>&, const std::vector<const xAOD::Muon*>&, bool) const;
        double getInvariantMass(const JpsiCandidate&, std::span<const double> ) const;
        std::vector<JpsiCandidate> selectCharges(const std::vector<JpsiCandidate>&) const;
        std::unique_ptr<xAOD::Vertex> fit(const EventContext& ctx, const std::vector<const xAOD::TrackParticle*>&, const xAOD::TrackParticleContainer* importedTrackCollection) const;
        bool passesMCPCuts(const xAOD::Muon*) const;
        bool isContainedIn(const xAOD::TrackParticle*, const xAOD::TrackParticleContainer*) const;
        //-------------------------------------------------------------------------------------
        
    private:
        bool m_mumu;
        bool m_mutrk;
        bool m_trktrk;
        bool m_allMuons;
        bool m_combOnly;
        bool m_atLeastOneComb;
        bool m_useCombMeasurement;
        bool m_diMuons;
        double m_trk1M;
        double m_trk2M;
        double m_thresholdPt;
        double m_higherPt;
        double m_trkThresholdPt;
        double m_invMassUpper;
        double m_invMassLower;
        double m_collAngleTheta;
        double m_collAnglePhi;
        double m_Chi2Cut;
        bool m_oppChOnly;
        bool m_sameChOnly;
        bool m_allChCombs;
        SG::ReadHandleKey<xAOD::MuonContainer> m_muonCollectionKey{this, "muonCollectionKey", "Muons"};
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrkParticleCollection {this, "TrackParticleCollection", "InDetTrackParticles" };
        SG::ReadHandleKeyArray<xAOD::TrackParticleContainer> m_MuonTrackKeys{this, "MuonTrackKeys", {}};
        PublicToolHandle < Trk::IVertexFitter > m_iVertexFitter{this, "TrkVertexFitterTool", "Trk::TrkVKalVrtFitter"};
        PublicToolHandle < Trk::ITrackSelectorTool > m_trkSelector{this, "TrackSelectorTool", "InDet::TrackSelectorTool"};
        PublicToolHandle < InDet::VertexPointEstimator > m_vertexEstimator{this, "VertexPointEstimator", "InDet::VertexPointEstimator"};
        std::shared_ptr<GenData> m_gendata{nullptr};
        bool m_mcpCuts;
        bool m_doTagAndProbe;
        bool m_forceTagAndProbe;
    };
} // end of namespace
#endif

