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

    // tool
    ATH_CHECK(m_extrapolationTool.retrieve());

    std::array<std::string,8 > caloNames = {"EME1NegZ_Layer", "EME1PosZ_Layer", "EMB1_Layer", "TileBar1_Layer", "HEC1NegZ_Layer", "HEC1PosZ_Layer", "TileExt1NegZ_Layer", "TileExt1PosZ_Layer"};

    m_trackingGeometrySvc->trackingGeometry()->visitVolumes([&](const Acts::TrackingVolume *vol) {
      //get geo identifier
      Acts::GeometryIdentifier geoId = vol->geometryId();
      const auto & name = vol->volumeName();
      ATH_MSG_DEBUG(name << " - " << geoId << " - surfaces: " << vol->surfaces().size());

      if (std::ranges::contains(caloNames, name)){
        ATH_MSG_DEBUG("About to insert caloName " << name << " into map");
        m_caloVolumeLayerMap[std::make_pair(geoId.volume(), geoId.layer())] = vol->volumeName();
      }
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
      }
      tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaEM, etaEM);
      tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiEM, phiEM);
      tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaHad, etaHad);
      tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiHad, phiHad);
    }
    else { 
      // use ACTS Based extrapolation
      bool use_extrap_tool = false;
     
      const Acts::GeometryContext gctx = m_ctxProvider.getGeometryContext(ctx);

      ElementLink<ActsTrk::CaloExtensionContainer> trackCaloExtensionLink;
      if (extensionReadDecorHandle->isAvailable()) {
         trackCaloExtensionLink = (*extensionReadDecorHandle)(*orgTrack);
         use_extrap_tool = true;
      } 

      if (!trackCaloExtensionLink.isValid() && use_extrap_tool) {
         ATH_MSG_DEBUG("Decoration exists, but this track has no valid CaloExtension link (null link), will attempt to do the extrapolation using tool instead of cache");  
         use_extrap_tool = true;
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

          if (m_caloVolumeLayerMap.contains(std::make_pair(thisGeoID.volume(), thisGeoID.layer()))) {
            ATH_MSG_DEBUG("This surface is in the calorimeter");
            Acts::Vector3 pos = pars.position(gctx);
            
	    // match to EM or Had calorimeter
            std::string layer = m_caloVolumeLayerMap.at(std::make_pair(thisGeoID.volume(), thisGeoID.layer()));

            if (not validECal && layer.find("EM") != std::string::npos) {
              validECal = true;
              etaEM = pos.eta();
              phiEM = pos.phi();
              ATH_MSG_DEBUG("Extrapolated to ECal layer. Eta and Phi in caloLayer " << layer << " are " << etaEM << " and " << phiEM);
            }

            if (not validHCal && ((layer.find("Tile") != std::string::npos) || (layer.find("HEC") != std::string::npos))){
              validHCal = true;
              etaHad = pos.eta();
              phiHad = pos.phi();
              ATH_MSG_DEBUG("Extrapolated to HCal layer. Eta and Phi in caloLayer " << layer << " are " << etaHad << " and " << phiHad);
            }

            if( validECal and validHCal ) break;  
          }
        }
	tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaEM, etaEM);
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiEM, phiEM);
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaHad, etaHad);
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiHad, phiHad);
      }

      // backup to extrapolation tool
      if(use_extrap_tool){ 
        unsigned int lastMeasIdx = 0;
        if (!orgTrack->indexOfParameterAtPosition(lastMeasIdx, xAOD::LastMeasurement)) {
            ATH_MSG_ERROR("TrackParticle has no last measurement parameters");
            return StatusCode::FAILURE;
        }

        Acts::Vector3 lastPos{orgTrack->parameterX(lastMeasIdx),
                              orgTrack->parameterY(lastMeasIdx),
                              orgTrack->parameterZ(lastMeasIdx)};
        Acts::Vector3 lastMom{ActsTrk::energyToActs(orgTrack->parameterPX(lastMeasIdx)),
                              ActsTrk::energyToActs(orgTrack->parameterPY(lastMeasIdx)),
                              ActsTrk::energyToActs(orgTrack->parameterPZ(lastMeasIdx))};

        Acts::BoundVector lastBoundParams = Acts::BoundVector::Zero();
        lastBoundParams[Acts::eBoundPhi]    = lastMom.phi();
        lastBoundParams[Acts::eBoundTheta]  = lastMom.theta();
        lastBoundParams[Acts::eBoundQOverP] = orgTrack->charge() / lastMom.norm();

        std::shared_ptr<const Acts::Surface> lastSurface = Acts::CurvilinearSurface(lastPos, lastMom.normalized()).planeSurface()->getSharedPtr();

        Acts::BoundTrackParameters boundPars{
            std::move(lastSurface),
            lastBoundParams,
            std::nullopt,
            Acts::ParticleHypothesis::pion()
        };

        Acts::Result<std::pair<std::vector<Acts::detail::Step>, Acts::RecordedMaterial>> result = m_extrapolationTool.get()->propagationSteps(ctx, boundPars);

        if( !result.ok() ) {
          ATH_MSG_WARNING("Error during extrapolation: " << result.error().message());
          continue;
        }

        const auto &[steps, _] = result.value();

        bool validECal = false;
        bool validHCal = false;

        for(const auto &step : steps) {

          if( step.surface == nullptr || step.surface->geometryId().sensitive() == 0 ) {
            continue;
          }

          Acts::GeometryIdentifier thisGeoID = step.geoID;
          ATH_MSG_DEBUG("Got step with geoID " << thisGeoID);

	  if (m_caloVolumeLayerMap.contains(std::make_pair(thisGeoID.volume(), thisGeoID.layer()))) {

            ATH_MSG_DEBUG("This step is in the calorimeter");
            const auto &p = step.position;

            // match to EM or Had calorimeter
	    std::string layer = m_caloVolumeLayerMap.at(std::make_pair(thisGeoID.volume(), thisGeoID.layer()));

            if (not validECal && layer.find("EM") != std::string::npos) {
              validECal = true; 
	      etaEM = Acts::VectorHelpers::eta(p); 
              phiEM = Acts::VectorHelpers::phi(p);
              ATH_MSG_DEBUG("Extrapolated to ECal layer. Eta and Phi in caloLayer " << layer << " are " << etaEM << " and " << phiEM);
	    }

            if (not validHCal && ((layer.find("Tile") != std::string::npos) || (layer.find("HEC") != std::string::npos))){
              validHCal = true;
              etaHad = Acts::VectorHelpers::eta(p);
              phiHad = Acts::VectorHelpers::phi(p);
	      ATH_MSG_DEBUG("Extrapolated to HCal layer. Eta and Phi in caloLayer " << layer << " are " << etaHad << " and " << phiHad);
            }

            if( validECal and validHCal ) break;
          }
        }

        ATH_MSG_DEBUG("Finished steps loop");

        ATH_MSG_DEBUG( "Extrapolated track with eta=" << orgTrack->eta()
			 << " phi="<<orgTrack->phi()
                         << " to ECal eta=" << etaEM
                         << " phi="<< phiEM
                         << " HCal eta=" << etaHad
                         << " phi="<< phiHad
                       );
  
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaEM, etaEM);
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiEM, phiEM);
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingEtaHad, etaHad);
        tauTrack->setDetail(xAOD::TauJetParameters::CaloSamplingPhiHad, phiHad);
      }
    }	    
  }
 

  return StatusCode::SUCCESS;

}
#endif
