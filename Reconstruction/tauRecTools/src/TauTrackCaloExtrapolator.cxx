/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "TrkParametersIdentificationHelpers/TrackParametersIdHelper.h"

#include "TauTrackCaloExtrapolator.h"

#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Material/MaterialInteraction.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/Surfaces/CurvilinearSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "ActsEvent/CaloExtension.h"

#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "ActsInterop/UnitConverters.h"

#include "StoreGate/ReadDecorHandle.h"

TauTrackCaloExtrapolator::TauTrackCaloExtrapolator(const std::string& name) :
  TauRecToolBase(name),
  m_EMSamplings {CaloSampling::EME1, CaloSampling::EMB1},
  m_HadSamplings {CaloSampling::TileBar1, CaloSampling::HEC1, CaloSampling::TileExt1}
{}

TauTrackCaloExtrapolator::~TauTrackCaloExtrapolator() {}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode TauTrackCaloExtrapolator::initialize() {

  ATH_CHECK( m_ParticleCacheKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_extensionReadDecorKey.initialize(SG::AllowEmpty));
  ATH_CHECK( m_ctxProvider.initialize(SG::AllowEmpty));
  if(!m_useACTSExtrapolator){
    if(!m_caloExtensionTool.empty()) {
    ATH_CHECK( m_caloExtensionTool.retrieve() );
    }
  } else {

    ATH_CHECK(m_trackingGeometrySvc.retrieve());

    std::array<std::string, 8 > caloNames = {"PreSamplerB_Layer",
	                                     "EMB1_Layer", "EMB2_Layer", "EMB3_Layer",
                                             "TileBar0_Layer", "TileBar1_Layer", 
                                             "CaloLayerDiscNegativeZ", "CaloLayerDiscPositiveZ"};

    m_trackingGeometrySvc->trackingGeometry()->visitVolumes([&](const Acts::TrackingVolume *vol) {
      //get geo identifier
      Acts::GeometryIdentifier geoId = vol->geometryId();
      const auto & name = vol->volumeName();
      ATH_MSG_DEBUG(name << " - " << geoId << " - surfaces: " << vol->surfaces().size());

      if (std::ranges::contains(caloNames, name)){
        vol->visitSurfaces([&](const Acts::Surface *surface){
        ATH_MSG_DEBUG("This surface has ID" << surface->geometryId());
        m_geoIdCaloSampleMap[surface->geometryId()] = name; 
        });
      };
    });
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


  std::optional <SG::ReadHandle<CaloExtensionCollection>> particleCache;
  if(!m_ParticleCacheKey.empty()){
    particleCache.emplace(m_ParticleCacheKey, ctx);
  }
  
  std::optional < SG::ReadDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::CaloExtensionContainer>> > extensionReadDecorHandle;
  if(!m_extensionReadDecorKey.empty()){
    extensionReadDecorHandle.emplace(m_extensionReadDecorKey, ctx);
  }
  
  // Calculate eta, phi impact point of leading track at calorimeter layers EM 0,1,2,3
  constexpr size_t numberOfEM_Layers{4};
  constexpr double invalidCoordinate{-11111.};
  std::array<double, numberOfEM_Layers> extrapolatedEta{};
  std::array<double, numberOfEM_Layers> extrapolatedPhi{};
  extrapolatedEta.fill(invalidCoordinate);
  extrapolatedPhi.fill(invalidCoordinate);
  int i = 0;

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

    if(!m_useACTSExtrapolator){
      if(!m_ParticleCacheKey.key().empty()){
        /*get the CaloExtension object*/
        ATH_MSG_VERBOSE("Using the CaloExtensionBuilder Cache");
        caloExtension = (*(particleCache->cptr()))[trackIndex];
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
         
        // for the leading track we need to save additional information used later in TauElectronVetoVariables 
        if(i == 0 && tauTrack->flag(xAOD::TauJetParameters::classifiedCharged) ){
          for( const Trk::CurvilinearParameters& cur : clParametersVector ){
            if( !parsIdHelper.isEntryToVolume(cur.cIdentifier()) ) continue;
            CaloSampling::CaloSample sample = parsIdHelper.caloSample(cur.cIdentifier());  	     
            int index = -1;
            if( sample == CaloSampling::PreSamplerE || sample == CaloSampling::PreSamplerB ) index = 0;
            else if( sample == CaloSampling::EME1 || sample == CaloSampling::EMB1 )          index = 1;
	    else if( sample == CaloSampling::EME2 || sample == CaloSampling::EMB2 )          index = 2;
            else if( sample == CaloSampling::EME3 || sample == CaloSampling::EMB3 )          index = 3;

	    if( index >= 0 ){
              extrapolatedEta[index] = cur.position().eta();
              extrapolatedPhi[index] = cur.position().phi();
	    }
          }
        }
      }
      tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaEM, etaEM);
      tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiEM, phiEM);
      tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaHad, etaHad);
      tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiHad, phiHad);

    }
    else { 
      // use ACTS Based extrapolation
      const Acts::GeometryContext gctx = m_ctxProvider.getGeometryContext(ctx);

      ElementLink<ActsTrk::CaloExtensionContainer> trackCaloExtensionLink;
      if (extensionReadDecorHandle->isAvailable()) {
         trackCaloExtensionLink = (*extensionReadDecorHandle)(*orgTrack);
      } 

      if (!trackCaloExtensionLink.isValid()) {
         ATH_MSG_DEBUG("Decoration exists, but this track has no valid CaloExtension link (null link)");  
      } else {
	// do extrapolation from cache      
        const ActsTrk::CaloExtension* trackCaloExtension = *trackCaloExtensionLink;
        ATH_MSG_DEBUG("Have CaloExtensions with size " << trackCaloExtension->size() << " for this track");
        ATH_MSG_DEBUG("Are the parameters empty?: " << trackCaloExtension->parameters().empty());

	bool validECal = false;
        bool validHCal = false;

        for (const Acts::BoundTrackParameters& pars : trackCaloExtension->parameters()) {
          const Acts::Surface& surface = pars.referenceSurface();
          ATH_MSG_DEBUG("Geometry ID of surface is " << surface.geometryId());

          const auto thisGeoID = surface.geometryId();

          if (m_geoIdCaloSampleMap.contains(thisGeoID)) {

            ATH_MSG_DEBUG("This surface is in the calorimeter");

	    // match to EM or Had calorimeter
            std::string layer = m_geoIdCaloSampleMap.at(thisGeoID);   

            if (not validECal && layer.find("EM") != std::string::npos) {
              validECal = true;
              etaEM = pars.position(gctx).eta();
              phiEM = pars.position(gctx).phi();
              ATH_MSG_DEBUG("Extrapolated to ECal layer. Eta and Phi in caloLayer " << layer << " are " << etaEM << " and " << phiEM);
	    }

            if (not validHCal && ((layer.find("Tile") != std::string::npos) || (layer.find("Calo") != std::string::npos))){
              validHCal = true;
              etaHad = pars.position(gctx).eta(); 
              phiHad = pars.position(gctx).phi();
              ATH_MSG_DEBUG("Extrapolated to HCal layer. Eta and Phi in caloLayer " << layer << " are " << etaHad << " and " << phiHad);
            }

            if( validECal and validHCal ) break;  
          }
        }
	tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaEM, etaEM);
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiEM, phiEM);
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaHad, etaHad);
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiHad, phiHad);

        // for the leading track we need to save additional information used later in TauElectronVetoVariables 
        if(i == 0 && tauTrack->flag(xAOD::TauJetParameters::classifiedCharged) ){
          for (const Acts::BoundTrackParameters& pars : trackCaloExtension->parameters()) {
            const Acts::Surface& surface = pars.referenceSurface();
            ATH_MSG_DEBUG("Geometry ID of surface is " << surface.geometryId());

            const auto thisGeoID = surface.geometryId();

            if (m_geoIdCaloSampleMap.contains(thisGeoID)) {
              std::string layer = m_geoIdCaloSampleMap.at(thisGeoID);
              int index = -1;
	      if(layer.find("PreSampler") != std::string::npos)  index = 0;
	      else if(layer.find("EMB1") != std::string::npos)   index = 1;
	      else if(layer.find("EMB2") != std::string::npos)   index = 2;
	      else if(layer.find("EMB3") != std::string::npos)   index = 3;

	      if( index >= 0 ){
                extrapolatedEta[index] = pars.position(gctx).eta();
                extrapolatedPhi[index] = pars.position(gctx).phi();
              }
	    }
	  }
	}
      }
    }	

    // increse the counter 
    if(tauTrack->flag(xAOD::TauJetParameters::classifiedCharged)){
      i++;    
    }
  }

  // save the information we needed ater in TauElectronVetoVariables  
  static const SG::Accessor<std::array<double,numberOfEM_Layers>> acc_extrapolLeadTrackEta("extrapolLeadTrackEta");
  acc_extrapolLeadTrackEta(pTau) = std::move(extrapolatedEta);

  static const SG::Accessor<std::array<double,numberOfEM_Layers>> acc_extrapolLeadTrackPhi("extrapolLeadTrackPhi");
  acc_extrapolLeadTrackPhi(pTau) = std::move(extrapolatedPhi);

  return StatusCode::SUCCESS;

}
#endif
