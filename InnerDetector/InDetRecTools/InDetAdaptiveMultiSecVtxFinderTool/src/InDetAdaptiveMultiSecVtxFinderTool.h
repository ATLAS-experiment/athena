/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
// Author: Neza Ribaric <neza.ribaric@cern.ch>

/***************************************************************************
                         InDetAdaptiveMultiSecVtxFinderTool.cxx  -  Description
                             -------------------
    begin   : 01-12-2022
    authors : Neza Ribaric ( Lancaster University )
    information : Tool for Secondary Vertex Finding using AdaptiveMultivertexFitter and InDetTrackSelection
 ***************************************************************************

 *
 * This class provides an implementation for a secondary
 * vertex finding tool, which uses the Adaptive Multi Vertex
 * Fitter to find and fit multiple secondary vertices.
 *
 ***************************************************************************/

#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "InDetRecToolInterfaces/IInDetAdaptiveMultiSecVtxFinderTool.h"
#include "InDetRecToolInterfaces/IVertexFinder.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkParticleBase/TrackParticleBaseCollection.h"  // type def ...
#include "TrkTrack/TrackCollection.h"                     // type def ...
#include "TrkVertexFitterInterfaces/IImpactPoint3dEstimator.h"
#include "TrkVertexFitterInterfaces/IVertexSeedFinder.h"
#include "TrkVertexFitters/AdaptiveMultiVertexFitter.h"
#include "xAODTracking/TrackParticleContainerFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODTracking/VertexContainerFwd.h"
#include "xAODTracking/VertexFwd.h"

#include <utility>
namespace Trk {

    class Track;
    class TrackParticleBase;
    class ITrackLink;
    class IVertexSeedFinder;
    class IImpactPoint3dEstimator;
    class IVertexLinearizedTrackFactory;
}  // namespace Trk
//
//
namespace InDet {
    class IInDetTrackSelectionTool;

    class ATLAS_NOT_THREAD_SAFE InDetAdaptiveMultiSecVtxFinderTool
        : public extends<AthAlgTool, IAdaptiveMultiSecVertexFinder>  // mutable variables are used without protection.
    {
    public:
        /**
         * Constructor
         */
        using extends::extends;

        InDetAdaptiveMultiSecVtxFinderTool(const std::string& t, const std::string& n, const IInterface* p);

        /**
         * Destructor
         */

        virtual ~InDetAdaptiveMultiSecVtxFinderTool() = default;

        StatusCode initialize() override;
        StatusCode finalize() override;

        std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> findVertex(
            const xAOD::TrackParticleContainer* trackParticles) override;

        void setPrimaryVertexPosition(double, double, double) override;

    private:
        std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> doVertexing(const std::vector<Trk::ITrackLink*>& trackVector);
        float findCompatibleTracks(const EventContext& ctx, Amg::Vector3D& seedVertex, Trk::ITrackLink* trkIter) const;
        void countTracksAndNdf(xAOD::Vertex* myxAODVertex, float& ndf, int& ntracks) const;
        bool checkFit(xAOD::Vertex* actualCandidate) const;
        int removeTracksFromSeeds(xAOD::Vertex* actualCandidate, std::vector<Trk::ITrackLink*>& seedTracks) const;
        void removeClosestTrack(const EventContext& ctx, Amg::Vector3D& seedVertex, std::vector<Trk::ITrackLink*>& seedTracks, int& nFound) const;

        bool V0check(const std::vector<Amg::Vector3D>& momenta, const Amg::Vector3D& posi) const;
        const std::vector<Amg::Vector3D> getVertexMomenta(xAOD::Vertex* myxAODVertex) const;


        ToolHandle<Trk::AdaptiveMultiVertexFitter> m_VertexFitter{this, "VertexFitterTool", "Trk::AdaptiveMultiVertexFitter", "Multi Vertex Fitter"};
        ToolHandle<InDet::IInDetTrackSelectionTool> m_trkFilter{this, "BaseTrackSelector", "InDet::DetailedTrackSelectToolRelax", "Base track selection tool"};
        ToolHandle<InDet::IInDetTrackSelectionTool> m_SVtrkFilter{this, "SecVtxTrackSelector", "InDet::SecVtxTrackSelector", "SV track selection tool"};
        
        ToolHandle<Trk::IVertexSeedFinder> m_SeedFinder{this, "SeedFinder", "Trk::IndexedCrossDistancesSeedFinder", "Seed finder"};
        ToolHandle<Trk::IImpactPoint3dEstimator> m_ImpactPoint3dEstimator{this, "ImpactPoint3dEstimator", "Trk::ImpactPoint3dEstimator", "Impact point estimator"};

        // declareInterface<IAdaptiveMultiSecVertexFinder>(this);
        FloatProperty m_privtxRef{this, "MomentumProjectionOnDirection", -999999.9, "pri vtx ref"};
        DoubleProperty m_significanceCutSeeding{this, "significanceCutSeeding", 10, "significanceCutSeeding"};
        DoubleProperty m_minWghtAtVtx{this, "minTrackWeightAtVtx", 0., "minTrackWeightAtVtx"};
        DoubleProperty m_maxIterations{this, "maxVertices", 25, "max iterations"};
        Amg::Vector3D m_privtx;

    };  // end of class definitions
}  // namespace InDet
