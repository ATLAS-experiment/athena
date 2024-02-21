/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SpacePointAnalysisAlg.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "xAODInDetMeasurement/ContainerAccessor.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

namespace ActsTrk {
  SpacePointAnalysisAlg::SpacePointAnalysisAlg(const std::string& name, ISvcLocator *pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) 
  {}

  StatusCode SpacePointAnalysisAlg::initialize() {
    ATH_MSG_DEBUG( "Initializing " << name() << " ... " );
    
    ATH_CHECK( m_spacePointContainerKey.initialize() );

    if (m_usePixel and m_useOverlap)
      ATH_MSG_INFO("No overlap collection when enabled for pixel space points! Check your configuration if needed.");

    ATH_MSG_DEBUG("Monitoring settings ...");
    ATH_MSG_DEBUG(m_monGroupName);
    
    return AthMonitorAlgorithm::initialize();
  }
  
  StatusCode SpacePointAnalysisAlg::fillHistograms(const EventContext& ctx) const {
    ATH_MSG_DEBUG(" In " << name() << "::fillHistograms()" );
    
    const PixelID *pixelID = nullptr;
    const SCT_ID *stripID = nullptr;
    
    if (m_usePixel)
      ATH_CHECK(detStore()->retrieve(pixelID, "PixelID"));
    else
      ATH_CHECK(detStore()->retrieve(stripID, "SCT_ID"));
    
    SG::ReadHandle< xAOD::SpacePointContainer > inputSpacePointContainer( m_spacePointContainerKey, ctx );
    if (!inputSpacePointContainer.isValid()){
      ATH_MSG_FATAL("xAOD::SpacePointContainer with key " << m_spacePointContainerKey.key() << " is not available...");
      return StatusCode::FAILURE;
    }

    auto monitor_nsp = Monitored::Scalar<int>("Nsp", inputSpacePointContainer->size());
    fill(m_monGroupName.value(), monitor_nsp);

    const xAOD::SpacePointContainer* inputSpacePointCollection = inputSpacePointContainer.cptr();
    // Check we can have access to clusters
    for (const xAOD::SpacePoint* sp : *inputSpacePointCollection) {
      if (sp->isAvailable< std::vector< const xAOD::UncalibratedMeasurement* > >("measurements")) {
	const auto& els = sp->measurements();
	for (const auto* el : els) {
	  [[maybe_unused]] const auto idHash = el->identifierHash();
	}
      } else if (sp->isAvailable< std::vector< ElementLink<xAOD::UncalibratedMeasurementContainer> > >("measurementLink")) {
	// if we are here, that means the bare pointers are not available
	// This should not happen
	ATH_MSG_ERROR("Space point has Element links but not bare pointers to cluster. This should not happen!");
	return StatusCode::FAILURE;
      } else {
	// This should never happen
	ATH_MSG_ERROR("There are no decorations that link the space point to the original clusters");
	return StatusCode::FAILURE;
      }
    }

    auto monitor_barrelEndcap = Monitored::Collection("barrelEndcap", *inputSpacePointCollection,
						      [this, &pixelID, &stripID] (const auto* spacePoint) -> int
						      {
							const auto idHash = spacePoint->elementIdList()[0];
							const Identifier id = this->m_usePixel ? 
							  pixelID->wafer_id(idHash) : stripID->wafer_id(idHash);
							return this->m_usePixel ? pixelID->barrel_ec(id) : stripID->barrel_ec(id);
						      });
    auto monitor_layerDisk = Monitored::Collection("layerDisk", *inputSpacePointCollection, 
						   [this, &pixelID, &stripID] (const auto* spacePoint) -> int 
						   {
						     const auto idHash = spacePoint->elementIdList()[0];
						     const Identifier id = this->m_usePixel ?
						       pixelID->wafer_id(idHash) : stripID->wafer_id(idHash);
						     return this->m_usePixel ? pixelID->layer_disk(id) : stripID->layer_disk(id);
						   });

    auto monitor_phiModule = Monitored::Collection("phiModule", *inputSpacePointCollection, 
						   [this, &pixelID, &stripID] (const auto* spacePoint) -> int
						   {
						     const auto idHash = spacePoint->elementIdList()[0];
						     const Identifier id = this->m_usePixel ?
						       pixelID->wafer_id(idHash) : stripID->wafer_id(idHash);
						     return this->m_usePixel ? pixelID->phi_module(id) : stripID->phi_module(id);
						   });
    auto monitor_etaModule = Monitored::Collection("etaModule", *inputSpacePointCollection,
						   [this, &pixelID, &stripID] (const auto* spacePoint) -> int
						   {
						     const auto idHash = spacePoint->elementIdList()[0];
						     const Identifier id = this->m_usePixel ?
						       pixelID->wafer_id(idHash) : stripID->wafer_id(idHash);
						     return this->m_usePixel ? pixelID->eta_module(id) : stripID->eta_module(id);
						   });
    auto monitor_sideModule = Monitored::Collection("sideModule", *inputSpacePointCollection, 
						    [this, &stripID] (const auto* spacePoint) -> int
						    {
						      if (this->m_usePixel) return 0;

						      const auto idHash = spacePoint->elementIdList()[0];
						      const Identifier id = stripID->wafer_id(idHash);
						      return stripID->side(id);
						    });

    auto monitor_isInnermost = Monitored::Collection("isInnermost", *inputSpacePointCollection,
						     [this, &pixelID] (const auto* spacePoint) -> int
						     {
						       if (not this->m_usePixel) return 0;

						       const auto idHash = spacePoint->elementIdList()[0];
						       const Identifier id = pixelID->wafer_id(idHash);
						       const int layerDisk = pixelID->layer_disk(id);
						       return int(layerDisk==0);
						     });
    auto monitor_isNextToInnermost = Monitored::Collection("isNextToInnermost", *inputSpacePointCollection,
							   [this, &pixelID] (const auto* spacePoint) -> int
							   {
							     if (not this->m_usePixel) return 0;
							     
							     const auto idHash = spacePoint->elementIdList()[0];
							     const Identifier id = pixelID->wafer_id(idHash);

							     const int brlEc = pixelID->layer_disk(id);
							     const int layerDisk = pixelID->layer_disk(id);
							     return int((layerDisk==1) or (brlEc!=0 and layerDisk==2));
							   });
    auto monitor_isOverlap = Monitored::Collection("isOverlap", *inputSpacePointCollection, 
						   [this] (const auto*) -> int
						   {
						     return this->m_usePixel ? 0 : int(this->m_useOverlap);
						   });

    auto monitor_eta = Monitored::Collection("eta", *inputSpacePointCollection,
					     [] (const auto* spacePoint) -> double
					     {
					       Amg::Vector3D globalPosition(spacePoint->x(), spacePoint->y(), spacePoint->z());
					       return globalPosition.eta();
					     });
    auto monitor_perp = Monitored::Collection("perp", *inputSpacePointCollection,
					      [] (const auto* spacePoint) -> double
					     {
					       Amg::Vector3D globalPosition(spacePoint->x(), spacePoint->y(), spacePoint->z());
					       return globalPosition.perp();
					     });

    auto monitor_globalX = Monitored::Collection("globalX", *inputSpacePointCollection,
						 [] (const auto* spacePoint) -> double
					         { return spacePoint->x(); }); 
    auto monitor_globalY = Monitored::Collection("globalY", *inputSpacePointCollection,
						 [] (const auto* spacePoint) -> double
						 { return spacePoint->y(); }); 
    auto monitor_globalZ = Monitored::Collection("globalZ", *inputSpacePointCollection,
						 [] (const auto* spacePoint) -> double
						 { return spacePoint->z(); });

    auto monitor_cov_r = Monitored::Collection("globalCovR", *inputSpacePointCollection,
					       [] (const auto* spacePoint) -> double
					       { return spacePoint->varianceR(); }); 
    auto monitor_cov_z = Monitored::Collection("globalCovZ", *inputSpacePointCollection,
					       [] (const auto* spacePoint) -> double
					       { return spacePoint->varianceZ(); }); 

    fill(m_monGroupName.value(),
	 monitor_barrelEndcap, monitor_layerDisk,
	 monitor_phiModule, monitor_etaModule, monitor_sideModule,
	 monitor_isInnermost, monitor_isNextToInnermost,
	 monitor_isOverlap,
	 monitor_eta, monitor_perp,
	 monitor_globalX, monitor_globalY, monitor_globalZ,
	 monitor_cov_r, monitor_cov_z);
    
    return StatusCode::SUCCESS;
  }
  
}
