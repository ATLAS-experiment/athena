#include "DerivationFrameworkBPhys/BKllIsoMultiplicityTool.h"

#include <string>
#include <vector>
#include "TLorentzVector.h"

#include "xAODBPhys/BPhysHelper.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"
#include "xAODEgamma/ElectronxAODHelpers.h"

using namespace std;
namespace DerivationFramework {

BKllIsoMultiplicityTool::BKllIsoMultiplicityTool(
  const std::string& t, const std::string& n, const IInterface* p)
    : AthAlgTool(t, n, p),
      m_name("BKllIsoMultTool"),
      m_cones(),
      m_onlyInVertex(false),
      m_vertexPassFlags(),
      m_vertexContainerName("NONE"),
      m_trackContainerName("InDetTrackParticles"),
      m_trkSelector("InDet::TrackSelectorTool"),
      m_trkSelectionCuts(),
      m_trackPtCut(500.),
      m_trackEtaCut(-1.), // < 0 -> No eta cut applied!
      m_elContainerKey("Electrons"),
      m_elTrackContainerKey("GSFTrackParticles"),
      m_elTrkSelectionCuts(),
      m_elTrackPtCut(5000.),
      m_elTrackEtaCut(-1.), // < 0 -> No eta cut applied!
      m_elLHCut("DFCommonElectronsLHVeryLoose"), // Use the name corresponding to the auxdata item on electrons. "None" for no cut.
      m_muContainerKey("Muons"),
      m_muTrackContainerKey("InDetTrackParticles"),
      m_muTrkSelectionCuts(),
      m_muTrackPtCut(5000.),
      m_muTrackEtaCut(-1.), // < 0 -> No eta cut applied!
      m_muQualityCut(-1),   // -1 for no cuts. Tight(0), Medium (1), Loose (2), VeryLoose (3), HighPt (4), LowPt (5): https://twiki.cern.ch/twiki/bin/view/Atlas/MuonSelectionToolR21 
      m_muSelectionTool("CP::MuonSelectionTool/MuonSelectionTool"),
      m_recordTrackMult(true),
      m_recordElMult(true),
      m_recordMuMult(true) {
  ATH_MSG_DEBUG("Constructing...");
  declareInterface<DerivationFramework::IAugmentationTool>(this);

  // Declare tools
  declareProperty("BKllIsoMultiplicityToolName", m_name);
  declareProperty("IsolationCones", m_cones);
  declareProperty("OnlyInVertex", m_onlyInVertex);
  declareProperty("VertexPassFlags", m_vertexPassFlags);
  declareProperty("InputVertexContainer", m_vertexContainerName);
  declareProperty("TrackContainer", m_trackContainerName);
  declareProperty("TrackSelectorTool",m_trkSelector);
  declareProperty("AddTrackSelectionCuts", m_trkSelectionCuts);
  declareProperty("TrackPtCut", m_trackPtCut);
  declareProperty("TrackEtaCut", m_trackEtaCut);
  declareProperty("ElectronContainerKey", m_elContainerKey);
  declareProperty("ElectronTrackContainerKey", m_elTrackContainerKey);
  declareProperty("AddElectronTrackSelectionCuts", m_elTrkSelectionCuts);
  declareProperty("ElectronTrackPtCut", m_elTrackPtCut);
  declareProperty("ElectronTrackEtaCut", m_elTrackEtaCut);
  declareProperty("ElectronLikelihoodCut", m_elLHCut);
  declareProperty("MuonContainerKey", m_muContainerKey);
  declareProperty("MuonTrackContainerKey", m_muTrackContainerKey);
  declareProperty("AddMuonTrackSelectionCuts",m_muTrkSelectionCuts);
  declareProperty("MuonTrackPtCut", m_muTrackPtCut);
  declareProperty("MuonTrackEtaCut", m_muTrackEtaCut);
  declareProperty("MuonQualityCut", m_muQualityCut);
  declareProperty("MuonSelectionTool", m_muSelectionTool);
  declareProperty("RecordTrackMultiplicity", m_recordTrackMult);
  declareProperty("RecordElectronMultiplicity", m_recordElMult);
  declareProperty("RecordMuonMultiplicity", m_recordMuMult);
}

// *******************************************************************

StatusCode BKllIsoMultiplicityTool::initialize() {

  // Get the Track Selector Tool from ToolSvc
  if (m_trkSelector.retrieve().isFailure()) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_trkSelector);
      return StatusCode::FAILURE;
  } else {
      ATH_MSG_DEBUG("Retrieved tool " << m_trkSelector);
  }
  // Get the Muon Selector Tool
  if (m_muSelectionTool.retrieve().isFailure() && m_recordMuMult) {
      ATH_MSG_FATAL("Failed to retrieve tool " << m_muSelectionTool);
      return StatusCode::FAILURE;
  } else {
      ATH_MSG_DEBUG("Retrieved tool " << m_muSelectionTool);
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

  // Get Additional Track Selection Tools 
  if (m_trkSelectionCuts.size() > 0){
    for (unsigned int iSel = 0; iSel < m_trkSelectionCuts.size(); iSel++ ){
        m_addTrkSelTools.push_back( 
          new InDet::InDetTrackSelectionTool( "TrkSel" + m_trkSelectionCuts.at( iSel ), m_trkSelectionCuts.at( iSel ) )
        );
        if ( m_addTrkSelTools.at( iSel )->initialize().isFailure() ){
          ATH_MSG_FATAL("Failed to initialize the tool " << m_addTrkSelTools.at( iSel ) );
        } else {
          ATH_MSG_DEBUG("Initialized the tool " << m_addTrkSelTools.at( iSel ) );
        }
    }
  }
  // Get Additional Electron Track Selection Tools 
  if (m_elTrkSelectionCuts.size() > 0){
    for (unsigned int iSel = 0; iSel < m_elTrkSelectionCuts.size(); iSel++ ){
      m_addElTrkSelTools.push_back( 
        new InDet::InDetTrackSelectionTool( "elTrkSel" + m_elTrkSelectionCuts.at( iSel ), m_elTrkSelectionCuts.at( iSel ) ) 
      );
        if ( m_addElTrkSelTools.at( iSel )->initialize().isFailure() ){
          ATH_MSG_FATAL("Failed to initialize the tool " << m_addElTrkSelTools.at( iSel ) );
        } else {
          ATH_MSG_DEBUG("Initialized the tool " << m_addElTrkSelTools.at( iSel ) );
        }
    }
  }
  // Get Additional Muon Track Selection Tools 
  if (m_muTrkSelectionCuts.size() > 0){
    for (unsigned int iSel = 0; iSel < m_muTrkSelectionCuts.size(); iSel++ ){
      m_addMuTrkSelTools.push_back( 
        new InDet::InDetTrackSelectionTool( "muTrkSel" + m_muTrkSelectionCuts.at( iSel ), m_muTrkSelectionCuts.at( iSel ) ) 
      );
        if ( m_addMuTrkSelTools.at( iSel )->initialize().isFailure() ){
          ATH_MSG_FATAL("Failed to initialize the tool " << m_addMuTrkSelTools.at( iSel ) );
        } else {
          ATH_MSG_DEBUG("Initialized the tool " << m_addMuTrkSelTools.at( iSel ) );
        }
    }
  }
  return StatusCode::SUCCESS;
}

// *******************************************************************

StatusCode BKllIsoMultiplicityTool::finalize() {
  // everything all right
  return StatusCode::SUCCESS;
}

// *******************************************************************

bool BKllIsoMultiplicityTool::isTrackInVertex( const xAOD::Vertex* theVtx, const xAOD::TrackParticle* thePart) const {
  for (unsigned int i = 0; i < theVtx->nTrackParticles(); i++){
    auto vertexTrack   = theVtx->trackParticle( i );
    auto originalTrack = xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF( vertexTrack );
    if ( originalTrack == NULL ){
        if ( vertexTrack   == thePart ) return true;
    } else {
        if ( originalTrack == thePart ) return true;
    }
  }
  return false;
}

bool BKllIsoMultiplicityTool::setIsoVar( const xAOD::Vertex* theVtx, std::vector<std::vector<float>> isolationsAllLegsAllCones, std::string isoName ) const {
      for ( unsigned int iCone = 0; iCone < m_cones.size(); iCone ++ ){
        std::vector<float> perConeIsolation;
        for ( unsigned int iLeg = 0; iLeg < theVtx->nTrackParticles(); iLeg++ ){
          perConeIsolation.push_back( isolationsAllLegsAllCones.at( iLeg ).at( iCone ) );
        }
        std::string fullIsoName = isoName + "_c" + m_cones.at( iCone );
        SG::AuxElement::Decorator< std::vector<float> > isoDecorator( fullIsoName );
        isoDecorator( *theVtx ) = perConeIsolation;
      }
      return true;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode BKllIsoMultiplicityTool::addBranches() const {

  const xAOD::VertexContainer* vertexContainer = NULL;
  const xAOD::TrackParticleContainer* idTrackParticleContainer = NULL;
  const xAOD::ElectronContainer* elContainer = NULL;
  const xAOD::MuonContainer* muContainer = NULL;
  const xAOD::TrackParticleContainer* elTrackContainer = NULL;
  const xAOD::TrackParticleContainer* muTrackContainer = NULL;

  //	Load the Vertices
  if (evtStore()->contains<xAOD::VertexContainer>(m_vertexContainerName)) {
    CHECK(evtStore()->retrieve(vertexContainer, m_vertexContainerName));
  } else {
    ATH_MSG_ERROR("Failed loading vertex container!");
    return StatusCode::FAILURE;
  }

  // Load the TrackParticles for Isolation 
  if (evtStore()->contains<xAOD::TrackParticleContainer>(m_trackContainerName)) {
    CHECK(evtStore()->retrieve(idTrackParticleContainer, m_trackContainerName));
  } else {
    ATH_MSG_ERROR("Failed loading TrackParticleContainer!");
    return StatusCode::FAILURE;
  }

  // Load the Electrons & Electron Track Particles
  if ( m_recordElMult ){
    if (evtStore()->contains<xAOD::ElectronContainer>(m_elContainerKey)) {
      CHECK(evtStore()->retrieve(elContainer, m_elContainerKey));
    } else {
      ATH_MSG_ERROR("Failed loading ElectronContainer!");
      return StatusCode::FAILURE;
    }
    if (evtStore()->contains<xAOD::TrackParticleContainer>(m_elTrackContainerKey)) {
      CHECK(evtStore()->retrieve(elTrackContainer, m_elTrackContainerKey));
    } else {
      ATH_MSG_ERROR("Failed loading TrackParticleContainer for Electrons!");
      return StatusCode::FAILURE;
    }
  }

  // Load the Muons and Muon Track Particles
  if ( m_recordMuMult ){
    if (evtStore()->contains<xAOD::MuonContainer>(m_muContainerKey)) {
      CHECK(evtStore()->retrieve(muContainer, m_muContainerKey));
    } else {
      ATH_MSG_ERROR("Failed loading MuonContainer!");
      return StatusCode::FAILURE;
    }
    if (evtStore()->contains<xAOD::TrackParticleContainer>(m_muTrackContainerKey)) {
      CHECK(evtStore()->retrieve(muTrackContainer, m_muTrackContainerKey));
    } else {
      ATH_MSG_ERROR("Failed loading TrackParticleContainer for Muons!");
      return StatusCode::FAILURE;
    }
  }
 
  // Load EventInfo
  const xAOD::EventInfo* eventInfo = evtStore()->retrieve<const xAOD::EventInfo>("EventInfo");
  if (!eventInfo && (m_recordTrackMult || m_recordElMult || m_recordMuMult )){
      ATH_MSG_ERROR("Failed loading event info!");
      return StatusCode::FAILURE;
  }

  // Prepare TrackBags!
  std::vector<const xAOD::TrackParticle*> trackBag;
  std::vector<bool> isSelectedTrackBag;
  unsigned int selectedTrackMult = 0;
  for ( auto track : *idTrackParticleContainer ) {
    bool isSelected = true;
    bool vertexCheck = m_onlyInVertex ? false : true;
    if ( m_onlyInVertex ){
      for ( auto vertex : *vertexContainer ){
        bool goodVtx = m_vertexPassFlags.size() == 0 ? true : false;
        for ( unsigned int flagItr = 0; flagItr < m_vertexPassFlags.size(); flagItr++ ) {
            SG::AuxElement::Accessor<Char_t> flagAcc(m_vertexPassFlags.at(flagItr));
            if(flagAcc.isAvailable(*vertex) && flagAcc(*vertex) != 0) {
              goodVtx = true;
              break;
            }
        }
        if (!goodVtx) continue;
        for ( unsigned int iVtxTrack = 0; iVtxTrack < vertex->nTrackParticles(); iVtxTrack++  ){
          if ( track == vertex->trackParticle( iVtxTrack ) ){
            vertexCheck = true;
            break;
          } else {
            auto origVtxTrack = xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF( vertex->trackParticle( iVtxTrack ) ); 
            if ( origVtxTrack == track ){
              vertexCheck = true;
              break;
            }
          }
        }
        if ( vertexCheck ) break;
      }
    }
    if ( m_onlyInVertex && !vertexCheck ) continue;
    if ( !m_trkSelector->decision(*track, 0) ) continue;  
    for (unsigned int iSel = 0; iSel < m_addTrkSelTools.size(); iSel++ ){
      if (!m_addTrkSelTools.at( iSel )->accept( track ) ) {
        isSelected = false;
        break;
      }
    }
    if ( track->p4().Pt() < m_trackPtCut ) continue;
    if ( ( m_trackEtaCut > 0 ) && abs( track->p4().Eta() ) > m_trackEtaCut ) continue;
    trackBag.push_back( track );
    if ( isSelected ) selectedTrackMult += 1;
    isSelectedTrackBag.push_back( isSelected );
  }
  if ( m_recordTrackMult ) {
    SG::AuxElement::Decorator<unsigned int> trackMultDecorator( m_name + "TrackMultiplicity" );
    SG::AuxElement::Decorator<unsigned int> selectedTrackMultDecorator( m_name + "SelectedTrackMultiplicity" );
    trackMultDecorator( *eventInfo ) = trackBag.size();
    selectedTrackMultDecorator( *eventInfo ) = selectedTrackMult;
  }

  // Electron Multiplicity
  if ( m_recordElMult ){
    unsigned int elMult = 0;
    unsigned int selectedElMult = 0;
    for ( auto el : *elContainer ) {
      bool isSelected = true;
      if ( m_elLHCut != "None" ){
          SG::AuxElement::Accessor<char> elLikelihoodAcc( m_elLHCut );
          if (! elLikelihoodAcc( *el ) ) continue;
      }
      const xAOD::TrackParticle* elTrack;
      if ( m_elTrackContainerKey == "GSFCaloContainer" ) {
        static const SG::AuxElement::Accessor<ElementLink<xAOD::TrackParticleContainer> > refittedTrackParticleLink("gsfCaloTrackParticleLink");
        if ( ! refittedTrackParticleLink.isAvailable(*el) || ! refittedTrackParticleLink(*el).isValid() ) continue;
        elTrack = *refittedTrackParticleLink(*el);
        if (!elTrack) continue;
      }
      else if ( m_elTrackContainerKey == "GSFTrackParticles" ) {
        elTrack = el->trackParticle();
        if (!elTrack) continue;
      }
      else if ( m_elTrackContainerKey == "InDetTrackParticles" ) {
        auto _elTrack =  el->trackParticle();
        if (!_elTrack ) continue;
        elTrack =  xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF( _elTrack ); 
        if (!elTrack) continue;
      }
      else{
        ATH_MSG_ERROR("ElectronTrackContainerKey must be one of: GSFCaloContainer, GSFTrackParticles, InDetTrackParticles..." );
        return StatusCode::FAILURE;
      }
      if ( !m_trkSelector->decision(*elTrack, 0) ) continue; 
      for (unsigned int iSel = 0; iSel < m_addElTrkSelTools.size(); iSel++ ){
        if (!m_addElTrkSelTools.at( iSel )->accept( elTrack ) ) {
          isSelected = false;
          break;
        }
      }
      if ( elTrack->p4().Pt() < m_elTrackPtCut ) continue;
      if ( ( m_elTrackEtaCut > 0 ) && abs( elTrack->p4().Eta() ) > m_elTrackEtaCut ) continue;
      elMult += 1;
      if ( isSelected ) selectedElMult += 1;
    }
    SG::AuxElement::Decorator<unsigned int> elMultDecorator( m_name + "ElectronMultiplicity" );
    SG::AuxElement::Decorator<unsigned int> selectedElMultDecorator( m_name + "SelectedElectronMultiplicity" );
    elMultDecorator( *eventInfo ) = elMult;
    selectedElMultDecorator( *eventInfo ) = selectedElMult;
  }

  // Muon Multiplicity 
  if ( m_recordMuMult ){
    unsigned int muMult = 0;
    unsigned int selectedMuMult = 0;
    for ( auto mu : *muContainer ) {
      bool isSelected = 0;
      if( m_muQualityCut >= 0 ){
          if (!(m_muSelectionTool->getQuality(*mu) <= m_muQualityCut) ) continue;
      }
      const xAOD::TrackParticle* muTrack;
      if ( m_muTrackContainerKey == "InDetTrackParticles" ) {
        muTrack = mu->trackParticle( xAOD::Muon::InnerDetectorTrackParticle );
        if (!muTrack) continue;
      }
      else if ( m_muTrackContainerKey == "CombinedMuonTrackParticles" ) {
        muTrack = mu->trackParticle( xAOD::Muon::CombinedTrackParticle );
        if (!muTrack) continue;
      }
      else{
        ATH_MSG_ERROR("MuonTrackContainerKey must be one of: InDetTrackParticles, CombinedMuonTrackParticles..." );
        return StatusCode::FAILURE;
      }
      if ( !m_trkSelector->decision(*muTrack, 0) ) continue; 
      for (unsigned int iSel = 0; iSel < m_addMuTrkSelTools.size(); iSel++ ){
        if (!m_addMuTrkSelTools.at( iSel )->accept( muTrack ) ) {
          isSelected = false;
          break;
        }
      }
      if ( muTrack->p4().Pt() < m_muTrackPtCut ) continue;
      if ( ( m_muTrackEtaCut > 0 ) && abs( muTrack->p4().Eta() ) > m_muTrackEtaCut ) continue;
      muMult += 1;
      if ( isSelected ) selectedMuMult += 1;
    }
    SG::AuxElement::Decorator<unsigned int> muMultDecorator( m_name + "MuonMultiplicity");
    SG::AuxElement::Decorator<unsigned int> selectedMuMultDecorator( m_name + "SelectedMonMultiplicity");
    muMultDecorator( *eventInfo ) = muMult;
    selectedMuMultDecorator( *eventInfo ) = selectedMuMult;
  }

  // Loop Over Vertices
  for (auto vertex : *vertexContainer) {
    std::vector<float> refTracksPx = vertex->auxdata< std::vector< float, std::allocator< float > > > ("RefTrackPx");
    std::vector<float> refTracksPy = vertex->auxdata< std::vector< float, std::allocator< float > > > ("RefTrackPy");
    std::vector<float> refTracksPz = vertex->auxdata< std::vector< float, std::allocator< float > > > ("RefTrackPz");

    // Loop Over Tracks to Create Temporarily Updated TrackBag
    std::vector<const xAOD::TrackParticle*> vTrackBag;
    std::vector<bool> isSelectedVTrackBag;
    for (unsigned int iTrack = 0; iTrack < trackBag.size(); iTrack++ ) {
      auto track = trackBag.at( iTrack ); 
      if ( isTrackInVertex( vertex, track ) ) continue;
      vTrackBag.push_back( track );
      isSelectedVTrackBag.push_back( isSelectedTrackBag.at( iTrack ) );
    }
    // Loop Over Legs 
    std::vector<std::vector<float>> isolationsOrigAllLegs;
    std::vector<std::vector<float>> isolationsRefitAllLegs;
    std::vector<std::vector<float>> isolationsOrigInDetAllLegs;
    std::vector<std::vector<float>> selectedIsolationsOrigAllLegs;
    std::vector<std::vector<float>> selectedIsolationsRefitAllLegs;
    std::vector<std::vector<float>> selectedIsolationsOrigInDetAllLegs;
    for (unsigned int iLeg = 0; iLeg < vertex->nTrackParticles(); iLeg++ ){
        auto vertexTrack = vertex->trackParticle( iLeg );

        auto legMomentumOrig      = vertexTrack->p4();
        auto legMomentumRefit     = TLorentzVector();
        legMomentumRefit.SetXYZM( refTracksPx.at(iLeg), refTracksPy.at(iLeg), refTracksPz.at(iLeg), legMomentumOrig.M() ); // Mass doesn't matter!
        auto legMomentumOrigInDet = TLorentzVector();
        auto origInDetLeg =  xAOD::EgammaHelpers::getOriginalTrackParticleFromGSF( vertexTrack ); 
        if ( origInDetLeg == NULL ){
          origInDetLeg    = vertexTrack; // TODO: Check if not InDetTrackParticle for Muon! Usually, should be since vertexing is done w/ InDetTrackParticles for Muons.
        }
        legMomentumOrigInDet      = origInDetLeg->p4();
        std::vector<float> isolationsOrig      (m_cones.size(), 0.);
        std::vector<float> isolationsRefit     (m_cones.size(), 0.);
        std::vector<float> isolationsOrigInDet (m_cones.size(), 0.);
        std::vector<float> selectedIsolationsOrig      (m_cones.size(), 0.);
        std::vector<float> selectedIsolationsRefit     (m_cones.size(), 0.);
        std::vector<float> selectedIsolationsOrigInDet (m_cones.size(), 0.);
        // Loop Over Cones
        for (unsigned int iCone = 0; iCone < m_cones.size(); iCone++){
          float thrDeltaR = std::stof( m_cones.at( iCone ) ) * 0.01;
          // Loop Over Tracks 
          for ( unsigned int iTrack = 0; iTrack < vTrackBag.size(); iTrack++ ){
            bool isSelected = isSelectedVTrackBag.at( iTrack );
              auto trackMomentum = vTrackBag.at( iTrack )->p4();
              if ( legMomentumOrig.DeltaR( trackMomentum ) <= thrDeltaR ) {
                isolationsOrig.at( iCone ) += trackMomentum.Pt(); 
                if ( isSelected ) selectedIsolationsOrig.at( iCone ) += trackMomentum.Pt();
              }
              if ( legMomentumRefit.DeltaR( trackMomentum ) <= thrDeltaR ) {
                isolationsRefit.at( iCone ) += trackMomentum.Pt();
                if ( isSelected ) selectedIsolationsRefit.at( iCone ) += trackMomentum.Pt();
              }
              if ( legMomentumOrigInDet.DeltaR( trackMomentum ) <= thrDeltaR ) {
                isolationsOrigInDet.at( iCone ) += trackMomentum.Pt();
                if ( isSelected ) selectedIsolationsOrigInDet.at( iCone ) += trackMomentum.Pt();
              }
          }
        }
        isolationsOrigAllLegs.push_back( isolationsOrig );
        isolationsRefitAllLegs.push_back( isolationsRefit );
        isolationsOrigInDetAllLegs.push_back( isolationsOrigInDet );
        selectedIsolationsOrigAllLegs.push_back( selectedIsolationsOrig );
        selectedIsolationsRefitAllLegs.push_back( selectedIsolationsRefit );
        selectedIsolationsOrigInDetAllLegs.push_back( selectedIsolationsOrigInDet );
      }
      setIsoVar( vertex, isolationsOrigAllLegs, m_name + "TrackIsoOrig" );
      setIsoVar( vertex, isolationsRefitAllLegs, m_name + "TrackIsoRefit" );
      setIsoVar( vertex, isolationsOrigInDetAllLegs, m_name + "TrackIsoOrigInDet" );
      setIsoVar( vertex, selectedIsolationsOrigAllLegs, m_name + "SelectedTrackIsoOrig" );
      setIsoVar( vertex, selectedIsolationsRefitAllLegs, m_name + "SelectedTrackIsoRefit" );
      setIsoVar( vertex, selectedIsolationsOrigInDetAllLegs, m_name + "SelectedTrackIsoOrigInDet" );
    }
  return StatusCode::SUCCESS;
}

}  // End of namespace DerivationFramework
