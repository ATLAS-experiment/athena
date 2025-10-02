/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUSAVTXFITTERTOOL_H
#define MUSAVTXFITTERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "TrkParameters/TrackParameters.h"
#include "ITrackToVertex/ITrackToVertex.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"

namespace Rec {
    static const InterfaceID IID_MuSAVtxFitterTool("MuSAVtxFitterTool", 1, 0);

    class MuSAVtxFitterTool:  public AthAlgTool
    {
        public:
        MuSAVtxFitterTool(const std::string& type, const std::string& name, const IInterface*  parent);
        ~MuSAVtxFitterTool();
        virtual StatusCode initialize() override;

        static const InterfaceID& interfaceID() { return IID_MuSAVtxFitterTool;}

        /**
         * @struct WrkVrt
         * @brief Represents a working vertex structure used in VKalVrt fitting.
         *
         * This structure contains information about a fitted vertex candidate, including its
         * position, momentum, associated tracks, and fit quality metrics.
         *
         * It is used to temporarily store vertex information during the finding+fitting process for
         * further selection and manipulation, before a vertex candidate is accepted and saved to the output container in xAOD format
         * 
         * @note Quantities noted by "selected" are those used in the initial vertex-finding "seeding" step, while those with "associated" are tracks unused in the seeding step but used in the final fit via 
         * an additional "inclusive" attachment step. Currently the algorithm performs a simple 2 track fit to "selected" tracks and there are no "associated" tracks, but this will be added in future developments. 
         * 
         */

        struct WrkVrt {
         
            bool isGood = true;                            //! flagged true for good vertex candidates
            std::deque<long int> selectedTrackIndices;      //! list if indices in TrackParticleContainer for selectedBaseTracks -- CURRENTLY UNUSED, LEFT FOR FUTURE DEVELOPMENTS
            std::deque<long int> associatedTrackIndices;    //! list if indices in TrackParticleContainer for associatedTracks -- CURRENTLY UNUSED, LEFT FOR FUTURE DEVELOPMENTS
            std::vector<std::shared_ptr<const xAOD::TrackParticle>> newExtrapolatedTracks; //! list of associated new extrapolated tracks
            std::vector<const xAOD::Muon*> muonCandidates;  //! list of associated muon candidates
            Amg::Vector3D        pos;                    //! VKalVrt fit vertex position
            TLorentzVector       mom;                 //! VKalVrt fit vertex 4-momentum
            std::vector<double>  cov;                 //! VKalVrt fit covariance
            double               chi2 = 0;                  //! VKalVrt fit chi2 result
            double               chi2Core = 0;             //! VKalVrt fit chi2 result
            std::vector<double>  chi2PerTrk;                //! list of VKalVrt fit chi2 for each track
            long int             charge = 0;                //! total charge of the vertex
            std::vector< std::vector<double> > trkAtVrt;    //! list of track parameters wrt the reconstructed vertex
            double               minOpAng = 0;            //! minimum opening angle between tracks
            int                  closestWrkVrtIndex = 0;    //! stores the index of the closest WrkVrt in std::vector<WrkVrt> -- CURRENTLY UNUSED, LEFT FOR FUTURE DEVELOPMENTS
            double               closestWrkVrtValue = 0;    //! stores the value of some observable to the closest WrkVrt ( observable = e.g. significance ) -- CURRENTLY UNUSED, LEFT FOR FUTURE DEVELOPMENTS 

            inline double ndof() const { return 2.0*( selectedTrackIndices.size() + associatedTrackIndices.size() ) - 3.0; } //currently always 1, because only 2trk fit, but left for future developments.
            inline double ndofCore() const { return 2.0*( selectedTrackIndices.size() ) - 3.0; } //currently always 1, because only 2trk fit, but left for future developments. Uses only seeding MSTPs ("core")
            inline unsigned nTracksTotal() const { return selectedTrackIndices.size() + associatedTrackIndices.size(); } 
            inline double fitQuality() const { return chi2 / ndof(); }
    };

        // takes in MSTPs from SA muons and extrapolates them backwards without any constraint, producing a new xAOD::TrackParticle for input to the vertex fitter
        std::unique_ptr<xAOD::TrackParticle> extrapolateMuSA(const xAOD::TrackParticle& trk, const xAOD::EventInfo& eventInfo, const EventContext& ctx) const;

        // performs the MuSA vertex fit routine using the VKalVrtFitter tool
        StatusCode doMuSAVtxFit(std::vector<MuSAVtxFitterTool::WrkVrt>& workVerticesContainer, const xAOD::MuonContainer& muonContainer, const xAOD::EventInfo& eventInfo, const EventContext& ctx) const;

        // does post-fit selections on vertices in the workVerticesContainer
        StatusCode doPostFitSelections(std::vector<MuSAVtxFitterTool::WrkVrt>& workVerticesContainer) const;

        // selects the best remaining vertices from the workVerticesContainer and resolves ambiguities to produce unique track-to-vertex associations
        StatusCode selectBestVertices(std::vector<MuSAVtxFitterTool::WrkVrt>& workVerticesContainer) const;
        
        private:

        ToolHandle<Trk::ITrkVKalVrtFitter> m_vertexFitter{
            this, "VertexFitterTool", "Trk::TrkVKalVrtFitter", "Vertex fitter tool"};
        ToolHandle <Trk::IExtrapolator> m_extrapolator{
            this,"Extrapolator","Trk::Extrapolator/AtlasExtrapolator","ATLAS Extrapolator tool"};
        
        DoubleProperty m_etaCutMSTP{this, "etaCutMSTP", 2.5, "Maximum |eta| of input MSTPs"};
        DoubleProperty m_baseChi2Cut{this, "baseChi2Cut", 50., "Maximum allowed chi2 of saved vertices"};
        Gaudi::Property<bool> m_doValidation{this, "doValidation", false, "Vertex every MSTP in input as part of validation process"};

    };
}

#endif