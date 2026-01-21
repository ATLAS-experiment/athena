/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#ifndef JPSIUPSILONCOMMON
#define JPSIUPSILONCOMMON

#include <vector>
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODMuon/MuonContainer.h"
#include <algorithm>
#include "xAODTracking/VertexContainerFwd.h"
#include "xAODTracking/Vertex.h"
#include <array>
#include <span>

namespace xAOD{
   class BPhysHelper;
}

namespace Analysis {
   class PrimaryVertexRefitter;
   class CleanUpVertex{
       const xAOD::Vertex* m_vtx;
       bool m_cleanup;
    public:
       const xAOD::Vertex* get() const { return m_vtx; }
       ~CleanUpVertex(){ if (m_cleanup) delete  m_vtx; }
       CleanUpVertex(const xAOD::Vertex* vtx, bool cleanup) : m_vtx(vtx), m_cleanup(cleanup) {}
       CleanUpVertex(const CleanUpVertex&) = delete;
       CleanUpVertex(CleanUpVertex&& vtx) noexcept {
           m_vtx = vtx.m_vtx;
           m_cleanup = vtx.m_cleanup;
           vtx.m_cleanup = false;
           vtx.m_vtx = nullptr;
       }
       CleanUpVertex & operator=(const CleanUpVertex&) = delete;
   };

   class JpsiUpsilonCommon{
    public:

        static double getInvariantMass(const xAOD::TrackParticle* trk1, double mass1, const xAOD::TrackParticle* trk2, double mass2);
        static double getInvariantMass(std::span<const xAOD::TrackParticle*> trk,
                                             std::span<const double> masses);
        static double getPt(std::span<const xAOD::TrackParticle* const> tracks);
        static bool   isContainedIn(const xAOD::TrackParticle*, std::span<const xAOD::TrackParticle* const>) noexcept;
        static bool   isContainedIn(const xAOD::TrackParticle*, const xAOD::MuonContainer*);
        static bool   cutRangeOR(std::span<double const> values, double min, double max) noexcept;
        static bool   cutRange(double value, double min, double max) noexcept;
        static bool   cutAcceptGreaterOR(std::span<double const> values, double min) noexcept;
        static bool   cutAcceptGreater(double value, double min) noexcept;
        static Analysis::CleanUpVertex ClosestRefPV(xAOD::BPhysHelper&, const xAOD::VertexContainer*,const Analysis::PrimaryVertexRefitter*);
        static void RelinkVertexTracks(std::span<const xAOD::TrackParticleContainer* const> trkcols, xAOD::Vertex* vtx);
        static void RelinkVertexMuons(std::span<const xAOD::MuonContainer* const> muoncols, xAOD::Vertex* vtx);
        template <typename... Tracks>
        static double getPt(const xAOD::TrackParticle* first, const Tracks*... rest)
        {
            // Start with the four-momentum of the first track
            auto momentum = first->genvecP4();
        
            // Fold the rest of the tracks into the sum using += 
            // (The expansion applies += to each subsequent genvecP4())
            ( (momentum += rest->genvecP4()), ... );
        
            return std::sqrt(momentum.Perp2());
        }
   };


}

#endif

