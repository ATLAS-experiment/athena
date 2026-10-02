/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// UFOTrackParticleThinning.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkInDet/UFOTrackParticleThinning.h"

#include "StoreGate/ThinningHandle.h"

// Constructor
DerivationFramework::UFOTrackParticleThinning::UFOTrackParticleThinning(const std::string& t,
                                                                        const std::string& n,
                                                                        const IInterface* p ) :
base_class(t,n,p)
{
}

// Destructor
DerivationFramework::UFOTrackParticleThinning::~UFOTrackParticleThinning() = default;

// Athena initialize and finalize
StatusCode DerivationFramework::UFOTrackParticleThinning::initialize()
{
  // Decide which collections need to be checked for ID TrackParticles
  ATH_MSG_VERBOSE("initialize() ...");
  ATH_CHECK( m_inDetSGKey.initialize(m_streamName) );
  ATH_MSG_INFO("Using " << m_inDetSGKey.key() << " as the source collection for inner detector track particles");

  m_PFOChargedSGKey = m_PFOSGKey+"ChargedParticleFlowObjects";
  m_PFONeutralSGKey = m_PFOSGKey+"NeutralParticleFlowObjects";
  ATH_CHECK( m_PFONeutralSGKey.initialize(m_streamName));
  ATH_CHECK( m_PFOChargedSGKey.initialize(m_streamName));
  ATH_MSG_INFO("Using " << m_PFONeutralSGKey.key() << "and " << m_PFOChargedSGKey.key() << " as the source collection for the PFlow collection");

  for(unsigned int i = 0; i < m_addPFOSGKey.size(); i++){
    m_tmpAddPFOChargedSGKey = m_addPFOSGKey[i]+"ChargedParticleFlowObjects";
    ATH_CHECK( m_tmpAddPFOChargedSGKey.initialize(m_streamName) );
    m_addPFOChargedSGKey.push_back(m_tmpAddPFOChargedSGKey);
    m_tmpAddPFONeutralSGKey = m_addPFOSGKey[i]+"NeutralParticleFlowObjects";
    ATH_CHECK( m_tmpAddPFONeutralSGKey.initialize(m_streamName) );
    m_addPFONeutralSGKey.push_back(m_tmpAddPFONeutralSGKey);
  }

  ATH_CHECK( m_ufoSGKey.initialize(m_streamName));
  ATH_MSG_INFO("Using " << m_ufoSGKey.key()<< " as the source collection for UFOs");

  ATH_CHECK( m_jetSGKey.initialize());
  ATH_MSG_INFO("Using " << m_jetSGKey.key() << " as the source collection for UFOs");

  if (!m_selectionString.empty()){
    // order must match enum order EJetTrPThinningParser
    ATH_CHECK( initializeParser( m_selectionString ));
  }

  return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::UFOTrackParticleThinning::finalize()
{
    return StatusCode::SUCCESS;
}

// The thinning itself
StatusCode DerivationFramework::UFOTrackParticleThinning::doThinning(const EventContext& ctx) const
{
    

  // Retrieve main TrackParticle collection
  SG::ThinningHandle<xAOD::TrackParticleContainer> importedTrackParticles(m_inDetSGKey, ctx);
    
  // Retrieve PFO collection if required
  SG::ThinningHandle<xAOD::FlowElementContainer> importedPFONeutral(m_PFONeutralSGKey, ctx);
  SG::ThinningHandle<xAOD::FlowElementContainer> importedPFOCharged(m_PFOChargedSGKey, ctx);

  // Retrieve main jet collection
  SG::ReadHandle<xAOD::JetContainer> importedJets(m_jetSGKey, ctx);
  unsigned int nJets(importedJets->size());
  std::vector<const xAOD::Jet*> jetToCheck; jetToCheck.clear();

  // Check the event contains tracks
  unsigned int nTracks = importedTrackParticles->size();
  // Check the event contains calo clusters
  const size_t nPFONeutral = importedPFONeutral->size();
  const size_t nPFOCharged = importedPFOCharged->size() ;
  unsigned int nPFOs = nPFOCharged + nPFONeutral;
  if (nPFOs==0 && nTracks==0) return StatusCode::SUCCESS;
    
  // Set up a mask with the same entries as the full TrackParticle collection
  std::vector<bool> maskTracks;
  maskTracks.assign(nTracks,false); // default: don't keep any tracks
    
  // Set up a mask with the same entries as the full PFO collection(s)
  std::vector< bool > pfomaskNeutral( nPFONeutral, false );
  std::vector< bool > pfomaskCharged( nPFOCharged, false );
    
  // Retrieve containers
  // ... UFOs
  SG::ThinningHandle<xAOD::FlowElementContainer>  importedUFOs(m_ufoSGKey, ctx);
  unsigned int nUFOs(importedUFOs->size());
    
  // Set up a mask with the same entries as the full CaloCluster collection
  std::vector<bool> maskUFOs;
  maskUFOs.assign(nUFOs,false); // default: don't keep any tracks
    
  // Execute the text parser if requested
  if (m_selectionString!="") {
    std::vector<int> entries =  m_parser->evaluateAsVector();
    unsigned int nEntries = entries.size();
    // check the sizes are compatible
    if (nJets != nEntries ) {
      ATH_MSG_ERROR("Sizes incompatible! Are you sure your selection string used jets??");
      return StatusCode::FAILURE;
    } else {
      // identify which jets to keep for the thinning check
      for (unsigned int i=0; i<nJets; ++i) if (entries[i]==1) jetToCheck.push_back((*importedJets)[i]);
    }
  }

  if (m_selectionString=="") { // check all jets as user didn't provide a selection string
    for(const auto *jet : *importedJets){
      for( size_t j = 0; j < jet->numConstituents(); ++j ) {
        auto ufo = jet->constituentLinks().at(j);
        std::size_t index = ufo.index();
        maskUFOs.at(index) = true;
        const xAOD::FlowElement* ufoO = dynamic_cast<const xAOD::FlowElement*>(*ufo);
        if(!ufoO) continue;

	// Retrieve the track if UFO is charged or combined object
        if(ufoO->signalType()==xAOD::FlowElement::SignalType::Charged || ufoO->signalType()==xAOD::FlowElement::SignalType::Combined){
	  std::size_t index_trk = ufoO->chargedObject(0)->index();
	    maskTracks.at(index_trk) = true;
        }

	// Loop over charged and neutral PFOs
	for (size_t n = 0; n < ufoO->otherObjects().size(); ++n) {
	  std::size_t index_pfo = ufoO->otherObject(n)->index();

	  const xAOD::FlowElement* fe = static_cast<const xAOD::FlowElement*>(ufoO->otherObject(n));

	  if(fe->signalType()==xAOD::FlowElement::SignalType::ChargedPFlow){
	    pfomaskCharged.at( index_pfo ) = true;
	  }
	  else if(fe->signalType()==xAOD::FlowElement::SignalType::NeutralPFlow){
	    pfomaskNeutral.at( index_pfo ) = true; 
	  }
	}
      }
    }
    
  } else {
    
    for (auto & jetIt : jetToCheck) {
      for( size_t j = 0; j < jetIt->numConstituents(); ++j ) {
        auto ufo = jetIt->constituentLinks().at(j);
	std::size_t index = ufo.index();
	maskUFOs.at(index) = true;

	const xAOD::FlowElement* ufoO = dynamic_cast<const xAOD::FlowElement*>(*ufo);
	if(!ufoO) continue;

	if(ufoO->signalType()==xAOD::FlowElement::SignalType::Charged || ufoO->signalType()==xAOD::FlowElement::SignalType::Combined){
          std::size_t index_trk = ufoO->chargedObject(0)->index();
          maskTracks.at(index_trk) = true;
          
        }

        for (size_t n = 0; n < ufoO->otherObjects().size(); ++n) {
          std::size_t index_pfo = ufoO->otherObject(n)->index();

          const xAOD::FlowElement* fe = static_cast<const xAOD::FlowElement*>(ufoO->otherObject(n));

	  if(fe->signalType()==xAOD::FlowElement::SignalType::ChargedPFlow){
            pfomaskCharged.at( index_pfo ) = true;
          }
          else if(fe->signalType()==xAOD::FlowElement::SignalType::NeutralPFlow){
            pfomaskNeutral.at( index_pfo ) = true;
          }
        }
      }
    }
  }

  // Execute the thinning service based on the mask. Finish.
  if(m_thinTracks){
    importedTrackParticles.keep (maskTracks);
  }
  importedPFONeutral.keep (pfomaskNeutral);
  importedPFOCharged.keep (pfomaskCharged);
  importedUFOs.keep (maskUFOs);

  for(unsigned int i = 0; i < m_addPFOChargedSGKey.size(); i++){
    SG::ThinningHandle<xAOD::FlowElementContainer>  tempPFOCharged(m_addPFOChargedSGKey[i], ctx);
    SG::ThinningHandle<xAOD::FlowElementContainer>  tempPFONeutral(m_addPFONeutralSGKey[i], ctx);

    tempPFOCharged.keep(pfomaskCharged);
    tempPFONeutral.keep(pfomaskNeutral);
  }

  return StatusCode::SUCCESS;
}

