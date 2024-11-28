#include "DerivationFrameworkBPhys/BKllIsoMultiplicityTool.h"

#include <string>
#include <vector>

#include "TLorentzVector.h"
#include "xAODBPhys/BPhysHelper.h"
#include "xAODTracking/VertexContainer.h"

// added to convert GSF trackparticles to ID trackparticles
#include "RecoToolInterfaces/ITrackIsolationTool.h"
#include "xAODEgamma/ElectronxAODHelpers.h"
#include "xAODPrimitives/IsolationHelpers.h"  //For the definition of Iso::conesize

using namespace std;
namespace DerivationFramework {

BKllIsoMultiplicityTool::BKllIsoMultiplicityTool(const std::string& t, const std::string& n, const IInterface* p)
    : AthAlgTool(t, n, p),
      m_trackContainerName("InDetTrackParticles"),
      m_vertexContainerName("NONE"),
      m_trkSelector("InDet::TrackSelectorTool"),
      m_cones()
      {
  ATH_MSG_DEBUG("Constructing...");
  declareInterface<DerivationFramework::IAugmentationTool>(this);

  // Declare tools
  declareProperty("TrackContainer", m_trackContainerName);
  declareProperty("InputVertexContainer", m_vertexContainerName);
  declareProperty("TrackSelectorTool",m_trkSelector);
  declareProperty("IsolationTypes", m_cones);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode BKllIsoMultiplicityTool::initialize() {

  CHECK(m_trackIsoTool.retrieve());
  // Check that flags were given to tag the correct vertices

  // Control the IsolationType sequence
  if (m_cones.empty()) {
    ATH_MSG_INFO("Setting ptcones to default");
      m_cones.push_back("10");
      m_cones.push_back("20");
      m_cones.push_back("30");
      m_cones.push_back("40");
      m_cones.push_back("50");
  }

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode BKllIsoMultiplicityTool::finalize() {
  // everything all right
  return StatusCode::SUCCESS;
}

bool BKllIsoMultiplicityTool::isTrackInVertex(const xAOD::Vertex* theVtx,
                                         const <const xAOD::TrackParticle*> thePart) const {
  for (unsigned int i = 0; i < theVtx->size(); i++){
    auto vertexTrack   = theVtx->trackParticle( i );
    auto originalTrack = xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF( vertexTrack );
    if ( originalTrack == NULL ){
        if ( vertexTrack == thePart ) return true;
    } else {
        if ( originalTrack == thePart ) return true;
    }
  }
  return false;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode BKllIsoMultiplicityTool::addBranches() const {

  const xAOD::TrackParticleContainer* idTrackParticleContainer = NULL;
  const xAOD::VertexContainer* vertexContainer = NULL;

  // Load the TrackParticles for Isolation 
  if (evtStore()->contains<xAOD::TrackParticleContainer>(m_trackContainerName)) {
    CHECK(evtStore()->retrieve(idTrackParticleContainer, m_trackContainerName));
  } else {
    ATH_MSG_ERROR("Failed loading TrackParticleContainer container!");
    return StatusCode::FAILURE;
  }
  
  // Get the Track Selector Tool from ToolSvc
  if ( m_trkSelector.retrieve().isFailure() ) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_trkSelector);
      return StatusCode::FAILURE;
  } else {
      ATH_MSG_DEBUG("Retrieved tool " << m_trkSelector);
  }

  //	Load the Vertices
  if (evtStore()->contains<xAOD::VertexContainer>(m_vertexContainerName)) {
    CHECK(evtStore()->retrieve(vertexContainer, m_vertexContainerName));
  } else {
    ATH_MSG_ERROR("Failed loading vertex container!");
    return StatusCode::FAILURE;
  }

  // Prepare TrackBags!
  std::vector<xAOD::TrackParticle*> trackBag;
  for ( auto track : *idTrackParticleContainer ) {
      if ( !m_trkSelector->decision(*TP, vx) ) continue;  
      trackBag.push_back( track );
  }

  // Loop Over Vertices
  for (auto vertex : *vertexContainer) {
    //static SG::AuxElement::Decorator< std::vector<float> > refIsolation("");
    //static SG::AuxElement::Decorator< std::vector<float> > origIsolation("");
    //static SG::AuxElement::Decorator< std::vector<float> > origInDetIsolation("");
    //std::vector<float>
    std::vector<float> refTracksPx = vertex->auxdata< std::vector< float, std::allocator< float > > > ("RefTrackPx");
    std::vector<float> refTracksPy = vertex->auxdata< std::vector< float, std::allocator< float > > > ("RefTrackPy");
    std::vector<float> refTracksPz = vertex->auxdata< std::vector< float, std::allocator< float > > > ("RefTrackPz");
    for (auto track : *trackBag ) {
      if ( isTrackInVertex( vertex, track ) ) continue;
      auto trackMomentum = track->p4();
      for (unsinged int iLeg = 0; iLeg < vertex->size(); iLeg++ ){
        auto legMomentumOrig  = vertex->trackParticle( iLeg )->p4();
        auto legMomentumRefit = TLorentzVector();
        auto legMomentumRefit.SetXYZM( refTrackPx.at(iLeg), refTracksPy.at(iLeg), refTrackPz.at(iLeg), legMomentumOrig.M() ); // Mass doesn't matter!
        auto legMomentumOrigInDet = TLorentzVector();
        auto origInDetLeg =  xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF( vertexTrack ); 
        if ( origInDetLeg == NULL ){
          origInDetLeg = vertexTrack; // TODO: Make it also InDet for Muons!!!
        }
        legMomentumOrigInDet = origInDetLeg->p4();
      }
    }
  }

  return StatusCode::SUCCESS;
}

}  // End of namespace DerivationFramework
