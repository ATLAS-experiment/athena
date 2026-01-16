/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "AthViews/ViewHelper.h"
#include "CxxUtils/phihelper.h"
#include "ViewCreatorMuonSuperROITool.h"

#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"


ViewCreatorMuonSuperROITool::ViewCreatorMuonSuperROITool( const std::string& type, 
							  const std::string& name, 
							  const IInterface* parent)
  : base_class(type, name, parent)
{}


StatusCode ViewCreatorMuonSuperROITool::initialize()  {
  ATH_CHECK( m_roisWriteHandleKey.initialize() );
  
  return StatusCode::SUCCESS;
}


StatusCode ViewCreatorMuonSuperROITool::attachROILinks( TrigCompositeUtils::DecisionContainer& decisions, 
							const EventContext& ctx ) const {

  // ===================================================================================== // 
  // Create output RoI collection

  SG::WriteHandle<TrigRoiDescriptorCollection> roisWriteHandle =  TrigCompositeUtils::createAndStoreNoAux(m_roisWriteHandleKey, ctx);
  // ===================================================================================== // 

  // Only expect one object in container
  ATH_MSG_DEBUG("In MuonSuperROITool - got decisions, size: " << decisions.size());


  // Create SuperRoI to merge the RoIs from each muon
  std::unique_ptr<TrigRoiDescriptor> superRoI = std::make_unique<TrigRoiDescriptor>();
  superRoI->setComposite(true);
  superRoI->manageConstituents(false);

  // loop over decision objects
  for (TrigCompositeUtils::Decision* decision : decisions) {
    ATH_MSG_DEBUG(" Check decisions object ");

    ATH_MSG_DEBUG("PRINTING DECISION");
    ATH_MSG_DEBUG( *decision );


    // find the iParticle for this decision
    const std::vector<TrigCompositeUtils::LinkInfo<xAOD::IParticleContainer>> myFeature = TrigCompositeUtils::findLinks<xAOD::IParticleContainer>(decision, m_iParticleLinkName, TrigDefs::lastFeatureOfType);

    // there should be only one
    if (myFeature.size() != 1) {
      ATH_MSG_ERROR("Did not find exactly one most-recent xAOD::IParticle '" << m_iParticleLinkName << "' for Decision object index " << decision->index()
        << ", found " << myFeature.size());
      return StatusCode::FAILURE;
    }

   ATH_CHECK(myFeature.at(0).isValid());

   // find the muon
   const ElementLink<xAOD::IParticleContainer> p4EL = myFeature.at(0).link;
   const xAOD::Muon* muon = dynamic_cast< const xAOD::Muon*>(*p4EL); //get muon of this found object

   if (!( muon && muon->primaryTrackParticle()) ) {
     ATH_MSG_ERROR("NO PRIMARY  muon  from decision object! " << myFeature.at(0).link);
     return StatusCode::FAILURE;
   }

   ATH_MSG_DEBUG("MUON  -- pt=" << muon->pt() <<
        " eta=" << muon->eta() <<
        " phi=" << muon->phi() );


   double muonEta{muon->eta()}, muonPhi{muon->phi()};
   if (m_isBhh) {
      muonEta = 0.;
      muonPhi = CxxUtils::wrapToPi( muon->phi() + M_PI );
   }

   double etaMinus = muonEta - m_roiEtaWidth;
   double etaPlus  = muonEta + m_roiEtaWidth;
      
   double phiMinus = CxxUtils::wrapToPi( muonPhi - m_roiPhiWidth );
   double phiPlus  = CxxUtils::wrapToPi( muonPhi + m_roiPhiWidth );

   double muonZed=0.;

   std::unique_ptr<TrigRoiDescriptor> newROI = nullptr;

   if ( muon->primaryTrackParticle() ) {
      muonZed = muon->primaryTrackParticle()->z0() + muon->primaryTrackParticle()->vz();
	   
      // create ROIs
      ATH_MSG_DEBUG("Adding RoI to RoI container");
      ATH_MSG_DEBUG( "eta " << muonEta << " +/-" << m_roiEtaWidth << "     phi " << muonPhi << " +/- " << m_roiPhiWidth << "    zed " << muonZed << " +/- " << m_roiZedWidth);

      
      double zMinus = muonZed - m_roiZedWidth;
      double zPlus  = muonZed + m_roiZedWidth;

      zMinus = zMinus < -225. ? -225. : zMinus;      
      zPlus = zPlus > 225. ? 225. : zPlus; 

      ATH_MSG_DEBUG( "eta- " << etaMinus << " eta+ " << etaPlus << "     phi- " << phiMinus << " phi+ " << phiPlus << "    zed- " << zMinus << " zed+ " << zPlus);

     superRoI->push_back( new TrigRoiDescriptor( muonEta, etaMinus, etaPlus,
						 muonPhi, phiMinus, phiPlus,
						 muonZed, zMinus, zPlus ) );

     superRoI->manageConstituents(true);

   } else {
      ATH_MSG_DEBUG("Adding RoI to RoI container");
      ATH_MSG_DEBUG( "eta " << muonEta << " +/-" << m_roiEtaWidth << "     phi " << muonPhi << " +/- " << m_roiPhiWidth);
      ATH_MSG_DEBUG( "eta- " << etaMinus << " eta+ " << etaPlus << "     phi- " << phiMinus << " phi+ " << phiPlus);      
 
      superRoI->push_back( new TrigRoiDescriptor( muonEta, etaMinus, etaPlus,
						  muonPhi, phiMinus, phiPlus) );

      superRoI->manageConstituents(true);

   }


  }  //end loop over decisions

  

  roisWriteHandle->push_back(superRoI.release());
  const ElementLink< TrigRoiDescriptorCollection > roiEL = ElementLink< TrigRoiDescriptorCollection >( *roisWriteHandle, 0, ctx );


  for (TrigCompositeUtils::Decision* decision : decisions) {
    decision->setObjectLink( TrigCompositeUtils::roiString(), roiEL );

    ATH_MSG_DEBUG("PRINTING DECISION");
    ATH_MSG_DEBUG( *decision );
  }
  return StatusCode::SUCCESS;
}
