/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// HIJetTrackParticleThinningTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "HIJetTrackParticleThinningTool.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/ThinningHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"

// need to find this for the new version
#include "xAODTracking/VertexContainer.h"

#include <vector>
#include <string>

namespace DerivationFramework
{
    HIJetTrackParticleThinningTool::HIJetTrackParticleThinningTool(
        const std::string& t,
        const std::string& n,
        const IInterface* p) :

        base_class(t,n,p) {}

    // Destructor
    DerivationFramework::HIJetTrackParticleThinningTool::~HIJetTrackParticleThinningTool() {
    }

    // Athena initialize and finalize
    StatusCode DerivationFramework::HIJetTrackParticleThinningTool::initialize() {

        // Validate m_vertexScheme
        if (m_vertexScheme != "sumPt2" && m_vertexScheme != "nTracks" ) {
            ATH_MSG_ERROR("Invalid PrimaryVertexSelection: " << m_vertexScheme);
            return StatusCode::FAILURE;
        }

        ATH_CHECK(m_vertexKey.initialize());
        ATH_CHECK(m_jetKey.initialize());
        ATH_CHECK(m_sumPt2Key.initialize(m_vertexScheme == "sumPt2"));
        ATH_CHECK(m_trkSelTool.retrieve());
        ATH_CHECK(m_inDetSGKey.initialize(m_streamName));
        return StatusCode::SUCCESS;
    }

    StatusCode DerivationFramework::HIJetTrackParticleThinningTool::finalize() {
        ATH_CHECK(m_trkSelTool->finalize());
        ATH_MSG_INFO("Processed "<< m_ntot <<" tracks, "<< m_npass << " were retained.");
        return StatusCode::SUCCESS;
    }

    // The thinning itself
    StatusCode DerivationFramework::HIJetTrackParticleThinningTool::doThinning() const {
        // Get current event context
        const EventContext& ctx = Gaudi::Hive::currentContext();

        // Get TrackParticle collection
        SG::ThinningHandle<xAOD::TrackParticleContainer> tracks (m_inDetSGKey, ctx);

        // Check event contains tracks, if none then thinning complete!
        unsigned int nTracks = tracks->size();
        if (nTracks==0) return StatusCode::SUCCESS;

        // Set up a mask with same entires as full TrackParticle collection
        std::vector<bool> mask(nTracks,false); // reject all tracks by default
        m_ntot += nTracks;

        // Define a primary vertex
        const xAOD::Vertex *primary_vertex(nullptr);
        
        // Retrieve vertex container
        SG::ReadHandle<xAOD::VertexContainer> vtxC(m_vertexKey,ctx);
        ATH_CHECK(vtxC.isValid());

        // Retrieve jet containers
        SG::ReadHandle<xAOD::JetContainer> importedJets(m_jetKey,ctx);
        ATH_CHECK(importedJets.isValid());

        // Variables for tracking best vertex based on scheme
        float ptmax = 0.;
        std::size_t ntrkmax = 0;

        // Retrieve sumPt2 decoration
        SG::ReadDecorHandle<xAOD::VertexContainer, float> sumPt2Handle(m_sumPt2Key, ctx);
        ATH_CHECK(sumPt2Handle.isValid());

        // Iterate through vertices to find the primary vertex
        for (const xAOD::Vertex* vertex : *vtxC) {
            if (!vertex || vertex->vertexType() != xAOD::VxType::PriVtx) {
                continue;
            }

            if (m_vertexScheme == "sumPt2") {
                float sumPt = sumPt2Handle(*vertex);
                if (sumPt > ptmax) {
                    ptmax = sumPt;
                    primary_vertex = vertex;
                }
            }
            else {
                std::size_t ntp = vertex->nTrackParticles();
                if (ntp > ntrkmax) {
                    ntrkmax = ntp;
                    primary_vertex = vertex;
                }
            }
        }

        if (!primary_vertex) {
            ATH_MSG_DEBUG("No primary vertex found.");
        }

        // Iterate through jets, get associated tracks, set mask if passing thinning
        for (const auto  *jet : *importedJets) {
            std::vector<const xAOD::TrackParticle*> jetTracks;
            bool haveJetTracks = jet->getAssociatedObjects(xAOD::JetAttribute::GhostTrack, jetTracks);
            if ( !haveJetTracks ) { ATH_MSG_WARNING("Associated jet tracks not found!"); }
            else {
                for (auto & jetTrack : jetTracks) {
                    const xAOD::Vertex* vert_trk = primary_vertex;

                    asg::AcceptData acceptData = m_trkSelTool->accept(*jetTrack, vert_trk);
                    int index = jetTrack->index();
                    mask[index] = static_cast<bool>(acceptData);
                }
            }
        }

        // Count up mask contents
        unsigned int n_pass=0;
        for (unsigned int i=0; i<nTracks; ++i) {
            if (mask[i]) ++n_pass;
        }
        m_npass += n_pass;

        tracks.keep (mask);

        return StatusCode::SUCCESS;
    }
}