/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// HITrackParticleThinningTool.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////


#include "HITrackParticleThinningTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/ThinningHandle.h"

// need to find this for the new version
#include "xAODTracking/VertexContainer.h"

#include <vector>


namespace DerivationFramework
{
  HITrackParticleThinningTool::HITrackParticleThinningTool( const std::string& t,
                                                 		const std::string& n,
                                                 		const IInterface* p ) : 
    
    base_class(t,n,p),
    m_ntot(0),
    m_npass(0)
    {
      //declareProperty("InDetTrackParticlesKey", m_TP_key="InDetTrackParticles");
      declareProperty("PrimaryVertexKey", m_vertex_key="PrimaryVertices");
      declareProperty("PrimaryVertexSelection", m_vertex_scheme="sumPt2");
      declareProperty("TrackSelectionTool", m_trkSelTool, "Track selection tool" );
    }
  
  // Destructor
  DerivationFramework::HITrackParticleThinningTool::~HITrackParticleThinningTool() {
  }  

    // Athena initialize and finalize
    StatusCode DerivationFramework::HITrackParticleThinningTool::initialize(){
        if(!m_trkSelTool)
        {
            ATH_MSG_ERROR("InDetTrackSelectionTool not set. Cannot initialize.");
            return StatusCode::FAILURE;
        }

        // Validate m_vertex_scheme
        if (m_vertex_scheme != "sumPt2" && m_vertex_scheme != "nTracks") {
            ATH_MSG_ERROR("Invalid PrimaryVertexSelection: " << m_vertex_scheme);
            return StatusCode::FAILURE;
        }

        ATH_CHECK( m_inDetSGKey.initialize(m_streamName) );
        return StatusCode::SUCCESS;
    }

    StatusCode DerivationFramework::HITrackParticleThinningTool::finalize()
    {
        ATH_CHECK(m_trkSelTool->finalize());
        ATH_MSG_INFO("Processed " << m_ntot << " tracks, " << m_npass << " were retained.");
        return StatusCode::SUCCESS;
    }

  // The thinning itself
  StatusCode DerivationFramework::HITrackParticleThinningTool::doThinning(const EventContext& ctx) const
  {
    // Get the current event context

    // Get the track container
    SG::ThinningHandle<xAOD::TrackParticleContainer> tracks (m_inDetSGKey, ctx);

    //Define a primary vertex 
    const xAOD::Vertex *primary_vertex(nullptr);

    // Retrieve vertex container
    const xAOD::VertexContainer* vtxC(nullptr);
    if (evtStore()->retrieve(vtxC, m_vertex_key).isFailure()) {
        ATH_MSG_ERROR("Failed to retrieve VertexContainer with key: " << m_vertex_key);
        return StatusCode::FAILURE;
    }

    // Check event contains tracks, if none then thinning complete!
    unsigned int nTracks = tracks->size();
    if (nTracks==0) return StatusCode::SUCCESS;

    // Variables to track the best vertex based on the scheme
    float ptmax = 0.;
    size_t ntrkmax = 0;

    // Iterate through the vertices to find the primary vertex
    for (auto vertex : *vtxC) {
        if (!vertex || vertex->vertexType() != xAOD::VxType::PriVtx) {
            continue;
        }

        if (m_vertex_scheme.compare("sumPt2") == 0) {
            // Define the accessor for the sumPt2 variable
            SG::ConstAccessor<float> sumPt2Acc("sumPt2");

            // Check if the sumPt2 variable is available for the vertex
            if (sumPt2Acc.isAvailable(*vertex)) {
                float sumPT = sumPt2Acc(*vertex);  // Access the sumPt2 variable
                if (sumPT > ptmax) {
                    ptmax = sumPT;
                    primary_vertex = vertex;
                }
            }
        } else {
            size_t ntp = vertex->nTrackParticles();
            if (ntp > ntrkmax) {
                ntrkmax = ntp;
                primary_vertex = vertex;
            }
        }
    }

    // Warn if not primary vertex is found
    if (!primary_vertex) {
        ATH_MSG_DEBUG("No primary vertex found.");
    }

    // Loop over tracks, see if they pass, set mask
    std::vector<bool> mask(nTracks,false); // reject all tracks by default
    m_ntot += nTracks;

    for (auto tp : *tracks) {
        if (tp) {
            const xAOD::Vertex* vert_trk = primary_vertex;

            asg::AcceptData acceptData = m_trkSelTool->accept(*tp, vert_trk);
            int index = tp->index();
            mask[index] = static_cast<bool>(acceptData);
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
  


