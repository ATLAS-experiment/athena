#include "DerivationFrameworkBPhys/BKllIsoMultiplicityTool.h"

#include <string>
#include <vector>

#include "TLorentzVector.h"
#include "xAODBPhys/BPhysHelper.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODEventInfo/EventInfo.h"

// added to convert GSF trackparticles to ID trackparticles
#include "TrkToolInterfaces/ITrackSelectorTool.h"
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
      m_cones(),
      m_trackPtCut(500.),
      m_trackEtaCut(999.),
      m_elContainerKey("Electrons"),
      m_elTrackContainerKey("GSFTrackParticles"),
      m_elTrackPtCut(5000.),
      m_elTrackEtaCut(999.),
      m_elLHCut("VeryLooseNod0"),
      m_recordTrackMult(true),
      m_recordElMult(true),
      m_recordMuMult(true)
      {
  ATH_MSG_DEBUG("Constructing...");
  declareInterface<DerivationFramework::IAugmentationTool>(this);

  // Declare tools
  declareProperty("TrackContainer", m_trackContainerName);
  declareProperty("InputVertexContainer", m_vertexContainerName);
  declareProperty("TrackSelectorTool",m_trkSelector);
  declareProperty("IsolationCones", m_cones);
  declareProperty("TrackPtCut", m_trackPtCut);
  declareProperty("TrackEtaCut", m_trackEtaCut);
  declareProperty("ElectronContainerKey", m_elContainerKey);
  declareProperty("ElectronTrackContainerKey", m_elTrackContainerKey);
  //declareProperty("MuonContainerKey", m_muContainerKey);
  //declareProperty("MuonTrackContainerKey", m_muTrackContainerKey);
  declareProperty("ElectronTrackPtCut", m_elTrackPtCut);
  declareProperty("ElectronTrackEtaCut", m_elTrackEtaCut);
  //declareProperty("MuonTrackPtCut", m_muTrackPtCut);
  //declareProperty("MuonTrackEtaCut", m_muTrackEtaCut);
  declareProperty("ElectronLikelihoodCut", m_elLHCut);
  declareProperty("RecordTrackMultiplicity", m_recordTrackMult);
  declareProperty("RecordElectronMultiplicity", m_recordElMult);
  declareProperty("RecordMuonMultiplicity", m_recordMuMult);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode BKllIsoMultiplicityTool::initialize() {

  // Get the Track Selector Tool from ToolSvc
  if ( m_trkSelector.retrieve().isFailure() ) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_trkSelector);
      return StatusCode::FAILURE;
  } else {
      ATH_MSG_DEBUG("Retrieved tool " << m_trkSelector);
  }

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

bool BKllIsoMultiplicityTool::isTrackInVertex(const xAOD::Vertex* theVtx, const xAOD::TrackParticle* thePart) const {
  for (unsigned int i = 0; i < theVtx->nTrackParticles(); i++){
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

bool BKllIsoMultiplicityTool::setIsoVar(const xAOD::Vertex* theVtx, std::vector<std::vector<float>> isolationsAllLegsAllCones, std::string isoName ) const {
      for ( unsigned int iCone = 0; iCone < m_cones.size(); iCone ++ ){
        std::vector<float> perConeIsolation;
        for ( unsigned int iLeg = 0; iLeg < theVtx->nTrackParticles(); iLeg++ ){
          perConeIsolation.push_back( isolationsAllLegsAllCones.at( iLeg ).at( iCone ) );
        }
        std::string fullIsoName = isoName + "_c" + m_cones.at( iCone );
        SG::AuxElement::Decorator< std::vector<float> > isoDecorator( fullIsoName );
        //std::cout<< "Decorating Isolation w/ Cone Size: " << m_cones.at( iCone ) << " w/ Name: " << fullIsoName << " | Isolation Values: " << std::endl;
        //std::cout << "    0th Track: " << perConeIsolation.at(0) << std::endl;
        //std::cout << "    1st Track: " << perConeIsolation.at(1) << std::endl;
        //std::cout << "    2nd Track: " << perConeIsolation.at(2) << std::endl;
        //std::cout << "    3rd Track: " << perConeIsolation.at(3) << std::endl;
        isoDecorator( *theVtx ) = perConeIsolation;
      }
      return true;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode BKllIsoMultiplicityTool::addBranches() const {

  const xAOD::TrackParticleContainer* idTrackParticleContainer = NULL;
  const xAOD::VertexContainer* vertexContainer = NULL;
  const xAOD::ElectronContainer* elContainer = NULL;
  //const xAOD::MuonContainer* muContainer = NULL;
  const xAOD::TrackParticleContainer* elTrackContainer = NULL;
  //const xAOD::TrackParticleContainer* muTrackContainer = NULL;

  // Load the TrackParticles for Isolation 
  if (evtStore()->contains<xAOD::TrackParticleContainer>(m_trackContainerName)) {
    CHECK(evtStore()->retrieve(idTrackParticleContainer, m_trackContainerName));
  } else {
    ATH_MSG_ERROR("Failed loading TrackParticleContainer container!");
    return StatusCode::FAILURE;
  }

  // Load the Electrons & Electron Track Particles
  if ( m_recordElMult ){
    if (evtStore()->contains<xAOD::ElectronContainer>(m_elContainerKey)) {
      CHECK(evtStore()->retrieve(elContainer, m_elContainerKey));
    } else {
      ATH_MSG_ERROR("Failed loading Electron container!");
      return StatusCode::FAILURE;
    }
    if (evtStore()->contains<xAOD::TrackParticleContainer>(m_elTrackContainerKey)) {
      CHECK(evtStore()->retrieve(elTrackContainer, m_elTrackContainerKey));
    } else {
      ATH_MSG_ERROR("Failed loading Electron Track container!");
      return StatusCode::FAILURE;
    }
  }
  
  //	Load the Vertices
  if (evtStore()->contains<xAOD::VertexContainer>(m_vertexContainerName)) {
    CHECK(evtStore()->retrieve(vertexContainer, m_vertexContainerName));
  } else {
    ATH_MSG_ERROR("Failed loading vertex container!");
    return StatusCode::FAILURE;
  }

  // Load EventInfo
  const xAOD::EventInfo* eventInfo = evtStore()->retrieve<const xAOD::EventInfo>("EventInfo");
  if (!eventInfo && (m_recordTrackMult || m_recordElMult || m_recordMuMult )){
    ATH_MSG_ERROR("Failed loading event info!");
    return StatusCode::FAILURE;
  }

  // Prepare TrackBags!
  std::vector<const xAOD::TrackParticle*> trackBag;
  for ( auto track : *idTrackParticleContainer ) {
      if ( !m_trkSelector->decision(*track, 0) ) continue;  
      if ( track->p4().Pt() < m_trackPtCut ) continue;
      if ( abs( track->p4().Eta() ) > m_trackEtaCut ) continue;
      trackBag.push_back( track );
  }
  if ( m_recordTrackMult ) {
    SG::AuxElement::Decorator<unsigned int> trackMultDecorator("trackMultiplicity");
    trackMultDecorator( *eventInfo ) = trackBag.size();
  }

  // Electron Multiplicity
  if ( m_recordElMult ){
    unsigned int elMult = 0;
    for ( auto el : *elContainer ) {
      SG::AuxElement::Accessor<char> elLikelihoodAcc( m_elLHCut );
      if (! elLikelihoodAcc( *el ) ) continue;
      const xAOD::TrackParticle* elTrack;
      if ( m_elTrackContainerKey == "GSFCaloContainer" ) {
        static const SG::AuxElement::Accessor<ElementLink<xAOD::TrackParticleContainer> > refittedTrackParticleLink("gsfCaloTrackParticleLink");
        if ( ! refittedTrackParticleLink.isAvailable(*el) || ! refittedTrackParticleLink(*el).isValid() ) continue;
        elTrack = *refittedTrackParticleLink(*el);
        if (!elTrack) continue;
      }
      if ( m_elTrackContainerKey == "GSFTrackParticles" ) {
        elTrack = el->trackParticle();
        if (!elTrack) continue;
      }
      if ( m_elTrackContainerKey == "InDetTrackParticles" ) {
        auto _elTrack =  el->trackParticle();
        if (!_elTrack ) continue;
        elTrack =  xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF( _elTrack ); 
        if (!elTrack) continue;
      }
      if ( elTrack->p4().Pt() < m_elTrackPtCut ) continue;
      if ( abs( elTrack->p4().Eta() ) > m_elTrackEtaCut ) continue;
      elMult += 1;
    }
    SG::AuxElement::Decorator<unsigned int> elMultDecorator("electronMultiplicity");
    elMultDecorator( *eventInfo ) = elMult;
  }

  // Muon Multiplicity 

  // Loop Over Vertices
  for (auto vertex : *vertexContainer) {
    std::vector<float> refTracksPx = vertex->auxdata< std::vector< float, std::allocator< float > > > ("RefTrackPx");
    std::vector<float> refTracksPy = vertex->auxdata< std::vector< float, std::allocator< float > > > ("RefTrackPy");
    std::vector<float> refTracksPz = vertex->auxdata< std::vector< float, std::allocator< float > > > ("RefTrackPz");

    // Loop Over Tracks to Create Temporarily Updated TrackBag
    std::vector<const xAOD::TrackParticle*> vTrackBag;
    for (unsigned int iTrack = 0; iTrack < trackBag.size(); iTrack++ ) {
      auto track = trackBag.at( iTrack ); 
      if ( isTrackInVertex( vertex, track ) ) continue;
      vTrackBag.push_back( track );
    }
    // Loop Over Legs 
    std::vector<std::vector<float>> isolationsOrigAllLegs;
    std::vector<std::vector<float>> isolationsRefitAllLegs;
    std::vector<std::vector<float>> isolationsOrigInDetAllLegs;
    for (unsigned int iLeg = 0; iLeg < vertex->nTrackParticles(); iLeg++ ){
        auto vertexTrack = vertex->trackParticle( iLeg );

        auto legMomentumOrig      = vertexTrack->p4();
        auto legMomentumRefit     = TLorentzVector();
        legMomentumRefit.SetXYZM( refTracksPx.at(iLeg), refTracksPy.at(iLeg), refTracksPz.at(iLeg), legMomentumOrig.M() ); // Mass doesn't matter!
        auto legMomentumOrigInDet = TLorentzVector();
        auto origInDetLeg =  xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF( vertexTrack ); 
        if ( origInDetLeg == NULL ){
          origInDetLeg    = vertexTrack; // TODO: Check if not InDetTrackParticles for Muons!!
        }
        legMomentumOrigInDet      = origInDetLeg->p4();
        std::vector<float> isolationsOrig      (m_cones.size(), 0.);
        std::vector<float> isolationsRefit     (m_cones.size(), 0.);
        std::vector<float> isolationsOrigInDet (m_cones.size(), 0.);
        // Loop Over Cones
        for (unsigned int iCone = 0; iCone < m_cones.size(); iCone++){
          float thrDeltaR = std::stof( m_cones.at( iCone ) ) * 0.01;
          // Loop Over Tracks 
          for ( unsigned int iTrack = 0; iTrack < vTrackBag.size(); iTrack++ ){
              auto trackMomentum = vTrackBag.at( iTrack )->p4();
              if ( legMomentumOrig.DeltaR( trackMomentum ) <= thrDeltaR ) isolationsOrig.at( iCone ) += trackMomentum.Pt(); 
              if ( legMomentumRefit.DeltaR( trackMomentum ) <= thrDeltaR ) isolationsRefit.at( iCone ) += trackMomentum.Pt();
              if ( legMomentumOrigInDet.DeltaR( trackMomentum ) <= thrDeltaR ) isolationsOrigInDet.at( iCone ) += trackMomentum.Pt();
          }
        }
        isolationsOrigAllLegs.push_back( isolationsOrig );
        isolationsRefitAllLegs.push_back( isolationsRefit );
        isolationsOrigInDetAllLegs.push_back( isolationsOrigInDet );
      }
      setIsoVar( vertex, isolationsOrigAllLegs, "trackIsoOrig" );
      setIsoVar( vertex, isolationsRefitAllLegs, "trackIsoRefit" );
      setIsoVar( vertex, isolationsOrigInDetAllLegs, "trackIsoOrigInDet" );
    }
  return StatusCode::SUCCESS;
}

}  // End of namespace DerivationFramework
