/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// ****************************************************************************
// ----------------------------------------------------------------------------
// JpsiPlus2Tracks header file
//
// James Catmore <James.Catmore@cern.ch>

// ----------------------------------------------------------------------------
// ****************************************************************************
#ifndef JPSIPLUS2TRACKS_H
#define JPSIPLUS2TRACKS_H
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODTracking/VertexFwd.h"
#include "xAODTracking/VertexContainerFwd.h"
#include "StoreGate/ReadHandleKey.h"
#include <bitset>
#include <vector>
#include <string>
#include "xAODMuon/MuonContainer.h"
#include "JpsiUpsilonTools/ICandidateSearch.h"
/////////////////////////////////////////////////////////////////////////////

namespace Trk {
    class IVertexFitter;
    class ITrackSelectorTool;
    class TrkVKalVrtFitter;
}
namespace InDet { class VertexPointEstimator; }

namespace xAOD{
   class BPhysHelper;
}

namespace Analysis {
    
    static const InterfaceID IID_JpsiPlus2Tracks("JpsiPlus2Tracks", 1, 0);
    
    class JpsiPlus2Tracks:  public Analysis::ICandidateSearch, public AthAlgTool
    {
    public:
        JpsiPlus2Tracks(const std::string& t, const std::string& n, const IInterface*  p);
        ~JpsiPlus2Tracks();
        virtual StatusCode initialize() override;
        
        static const InterfaceID& interfaceID() { return IID_JpsiPlus2Tracks;};
        
        //-------------------------------------------------------------------------------------
        //Doing Calculation and inline functions
        virtual StatusCode performSearch(const EventContext& ctx, xAOD::VertexContainer&) const override;

        static bool   oppositeCharges(const xAOD::TrackParticle*, const xAOD::TrackParticle*);

        bool  passCuts(xAOD::BPhysHelper &bHelper, std::span<const double> masses, std::string_view str) const;
        bool  vertexCuts(xAOD::BPhysHelper &bHelper) const;
        xAOD::Vertex* fit(const std::vector<const xAOD::TrackParticle*>&,
                          const xAOD::TrackParticleContainer*, const xAOD::TrackParticleContainer* GSL) const;
        //-------------------------------------------------------------------------------------
        
    private:
        bool m_pipiMassHyp{};
        bool m_kkMassHyp{};
        bool m_kpiMassHyp{};
        bool m_kpMassHyp{};
        bool m_oppChargesOnly{};
        bool m_sameChargesOnly{};
        double m_trkThresholdPt{};
        double m_trkMaxEta{};
        double m_BThresholdPt{};
        double m_BMassUpper{};
        double m_BMassLower{};
        SG::ReadHandleKey<xAOD::VertexContainer> m_jpsiCollectionKey;
        double m_jpsiMassUpper{};
        double m_jpsiMassLower{};
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrkParticleCollection;
        SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrkParticleGSFCollection;
        SG::ReadHandleKey<xAOD::MuonContainer> m_MuonsUsedInJpsi;
        bool m_excludeJpsiMuonsOnly{}; //Add by Matt Klein
        bool m_excludeCrossJpsiTracks{}; //Added by Matteo Bedognetti
        ToolHandle < Trk::IVertexFitter > m_iVertexFitter;
        ToolHandle < Trk::ITrackSelectorTool > m_trkSelector;
        Trk::TrkVKalVrtFitter* m_VKVFitter{};
        bool m_useMassConst{};
        double m_altMassConst{};
        double m_diTrackMassUpper{};
        double m_diTrackMassLower{};
        
        // additional cuts (Daniel Scheirich)
        double m_chi2cut{};                 // chi2/Ndof of the final veretx
        double m_diTrackPt{};               // pT of the hadronic track pair before fit
        double m_trkQuadrupletMassUpper{};  // invariant mass of all the 4 tracks before fit
        double m_trkQuadrupletMassLower{};
        double m_trkQuadrupletPt{};         // combined pT of all 4 tracks  before fit
        double m_finalDiTrackMassUpper{};   // inveriant mass of the hadronic track pair after the fit
        double m_finalDiTrackMassLower{};
        double m_finalDiTrackPt{};          // pT of the hadronic track after fit
        double m_trkDeltaZ{};               // DeltaZ between the JPsi vertex and hadronic tracks Z0
        // (to reduce the pileup contribution before vertexing)
        std::vector<double> m_manualMassHypo;
        int m_requiredNMuons{};
        std::vector<double> m_altMassMuonTracks;
        std::vector<double>  m_mumukkMasses;
        std::vector<double>  m_mumupipiMasses;
        std::vector<double>  m_mumukpiMasses;
        std::vector<double>  m_mumupikMasses;
        std::vector<double>  m_mumukpMasses;
        std::vector<double>  m_mumupkMasses;
        std::vector<int>     m_useGSFTrackIndices;
        std::bitset<4>       m_useGSFTrack;
        size_t m_candidateLimit{};

    };
} // end of namespace
#endif
