/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "TrkParametersIdentificationHelpers/TrackParametersIdHelper.h"

#include "TauTrackCaloExtrapolator.h"

TauTrackCaloExtrapolator::TauTrackCaloExtrapolator(const std::string& name) :
  TauRecToolBase(name),
  m_EMSamplings {CaloSampling::EME1, CaloSampling::EMB1},
  m_HadSamplings {CaloSampling::TileBar1, CaloSampling::HEC1, CaloSampling::TileExt1}
{}

TauTrackCaloExtrapolator::~TauTrackCaloExtrapolator() {}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode TauTrackCaloExtrapolator::initialize() {

  ATH_CHECK( m_ParticleCacheKey.initialize(SG::AllowEmpty) );
  if(!m_caloExtensionTool.empty()) {
     ATH_CHECK( m_caloExtensionTool.retrieve() );
  } 

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode TauTrackCaloExtrapolator::executeTool(xAOD::TauJet& pTau,
				                const EventContext& ctx,
				                xAOD::TauTrackContainer& tauTrackCon) const 
{
  Trk::TrackParametersIdHelper parsIdHelper;

  int trackIndex = -1;
  const Trk::CaloExtension * caloExtension = nullptr;
  std::unique_ptr<Trk::CaloExtension> uniqueExtension;



  for(const ElementLink<xAOD::TauTrackContainer>& trackLink : pTau.allTauTrackLinks())
  {
    assert (trackLink.getStorableObjectPointer() == &tauTrackCon);
    xAOD::TauTrack* tauTrack = tauTrackCon[trackLink.index()];

    // if requested, run extrapolation only for charged tau-tracks
    if(m_extrapOnlyChargedTracks && (!tauTrack->flag(xAOD::TauJetParameters::classifiedCharged))){
        continue;
    }

    const xAOD::TrackParticle *orgTrack = tauTrack->track();
    if( !orgTrack ) continue;
    trackIndex = orgTrack->index();   

    // set default values
    float etaEM = -10.0;
    float phiEM = -10.0;
    float etaHad = -10.0;
    float phiHad = -10.0;

    // get the extrapolation into the calo
    ATH_MSG_DEBUG( "Try extrapolation of track with pt = " << orgTrack->pt() 
		   << ", eta " << orgTrack->eta() 
		   << ", phi" << orgTrack->phi() );

    if(!m_ParticleCacheKey.key().empty()){
      /*get the CaloExtension object*/
      ATH_MSG_VERBOSE("Using the CaloExtensionBuilder Cache");
      SG::ReadHandle<CaloExtensionCollection>  particleCache {m_ParticleCacheKey, ctx};
      caloExtension = (*particleCache)[trackIndex];
      ATH_MSG_VERBOSE("Getting element " << trackIndex << " from the particleCache");
      if( not caloExtension ){
        ATH_MSG_VERBOSE("Cache does not contain a calo extension -> "
                        "Calculating with the a CaloExtensionTool");
        uniqueExtension = m_caloExtensionTool->caloExtension(ctx, *orgTrack);
        caloExtension = uniqueExtension.get();
      }
    }
    else {
      /* If CaloExtensionBuilder is unavailable, use the calo extension tool */
      ATH_MSG_VERBOSE("Using the CaloExtensionTool");
      uniqueExtension = m_caloExtensionTool->caloExtension(ctx, *orgTrack);
      caloExtension = uniqueExtension.get();
    }

    if (!caloExtension)
      { 
	ATH_MSG_DEBUG("Track extrapolation failed");
      }
    else {
      const std::vector<Trk::CurvilinearParameters>& clParametersVector = caloExtension->caloLayerIntersections();
      if (clParametersVector.empty()) {
	ATH_MSG_DEBUG("Track extrapolation failed");
      }

      ATH_MSG_DEBUG("Scanning samplings");
      bool validECal = false;
      bool validHCal = false;
      for( const Trk::CurvilinearParameters& cur : clParametersVector ){
	ATH_MSG_DEBUG("Sampling " << parsIdHelper.caloSample(cur.cIdentifier()) );
                
	// only use entry layer
	if( not parsIdHelper.isEntryToVolume(cur.cIdentifier()) ) continue;

	CaloSampling::CaloSample sample = parsIdHelper.caloSample(cur.cIdentifier());

	// ECal
	if( not validECal and m_EMSamplings.count(sample))
	  {
	    validECal = true;
	    etaEM = cur.position().eta();
	    phiEM = cur.position().phi();
	    ATH_MSG_DEBUG("Extrapolated to ECal layer " << sample);
	  }

	// HCal
	if( not validHCal and m_HadSamplings.count(sample))
	  {
	    validHCal = true;
	    etaHad = cur.position().eta();
	    phiHad = cur.position().phi();
	    ATH_MSG_DEBUG("Extrapolated to HCal layer " << sample);
	  }
	if( validECal and validHCal ) break;
      }
      // EM failure warn if within acceptance 
      if( not validECal and std::abs(orgTrack->pt()) < 2.48 ){
	ATH_MSG_DEBUG("Failed extrapolation to ECal");
      }
      // Had failure warn if enough pt to reach HCal
      if( not validHCal and orgTrack->pt() > 2000. ){
	ATH_MSG_DEBUG("Failed extrapolation to HCal");
      }

      ATH_MSG_DEBUG( "Extrapolated track with eta=" << orgTrack->eta()
		     << " phi="<<orgTrack->phi()
		     << " to ECal eta=" << etaEM 
		     << " phi="<< phiEM
		     << " HCal eta=" << etaHad 
		     << " phi="<< phiHad
		     );
    }
    tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaEM, etaEM);
    tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiEM, phiEM);
    tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaHad, etaHad);
    tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiHad, phiHad);
  }

  return StatusCode::SUCCESS;

}
#endif
