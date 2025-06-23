/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JetRecTools/JetPFlowSelectionAlg.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteHandle.h"
#include "AsgDataHandles/ReadDecorHandle.h"

#include "PFlowUtils/FEElectronHelper.h"
#include "PFlowUtils/FEMuonHelper.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/EgammaxAODHelpers.h" 
#include "xAODMuon/Muon.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementAuxContainer.h"
#include "xAODPFlow/FEHelpers.h"

StatusCode JetPFlowSelectionAlg::initialize() {
  ATH_MSG_DEBUG("Initializing  " );

  ATH_CHECK(m_ChargedPFlowContainerKey.initialize());
  ATH_CHECK(m_NeutralPFlowContainerKey.initialize());
  if (!m_electronContainerKey.empty()) {
    ATH_CHECK(m_electronContainerKey.initialize());
  }
  ATH_CHECK(m_outputChargedPFlowHandleKey.initialize());
  ATH_CHECK(m_outputNeutralPFlowHandleKey.initialize());
  ATH_CHECK(m_chargedFEElectronsReadDecorKey.initialize());
  ATH_CHECK(m_chargedFEMuonsReadDecorKey.initialize());
  ATH_CHECK(m_neutralFEMuons_efrac_match_DecorKey.initialize());
  ATH_CHECK(m_neutralFEElectronsReadDecorKey.initialize());
  ATH_CHECK(m_neutralFEMuonsReadDecorKey.initialize());
  ATH_CHECK(m_chargedFE_energy_match_muonReadHandleKey.initialize());


  return StatusCode::SUCCESS;
}

StatusCode JetPFlowSelectionAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG(" execute() ... ");

  //Check for invalid combinations of flags
  if (m_excludeChargedMuonFE && m_includeChargedMuonFE) {
    ATH_MSG_ERROR("excludeChargedElectronFE and includeChargedMuonFE are mutually exclusive!");
    return StatusCode::FAILURE;
  }

  if (m_excludeChargedElectronFE && m_includeChargedElectronFE) {
    ATH_MSG_ERROR("excludeChargedElectronFE and includeChargedElectronFE are mutually exclusive!");
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<xAOD::FlowElementContainer> ChargedPFlowObjects(m_ChargedPFlowContainerKey, ctx);
  if (! ChargedPFlowObjects.isValid()){
    ATH_MSG_ERROR("Can't retrieve input container "<< m_ChargedPFlowContainerKey);                  
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<xAOD::FlowElementContainer> NeutralPFlowObjects(m_NeutralPFlowContainerKey, ctx);
  if (! NeutralPFlowObjects.isValid()){
    ATH_MSG_ERROR("Can't retrieve input container "<< m_NeutralPFlowContainerKey);                  
    return StatusCode::FAILURE;
  }

  SG::ReadDecorHandle<xAOD::FlowElementContainer, std::vector< ElementLink<xAOD::ElectronContainer> > > chargedFE_ElectronLinks(m_chargedFEElectronsReadDecorKey,ctx);
  if (!chargedFE_ElectronLinks.isValid()){
    ATH_MSG_ERROR("Can't retrieve input decoration " << chargedFE_ElectronLinks.key());                  
    return StatusCode::FAILURE;
  }

  SG::ReadDecorHandle<xAOD::FlowElementContainer, std::vector< ElementLink<xAOD::MuonContainer> > > chargedFE_MuonLinks(m_chargedFEMuonsReadDecorKey,ctx);
  if (!chargedFE_MuonLinks.isValid()){
    ATH_MSG_ERROR("Can't retrieve input decoration "<< chargedFE_MuonLinks.key());                  
    return StatusCode::FAILURE;
  }

  SG::ReadDecorHandle<xAOD::FlowElementContainer, std::vector< ElementLink<xAOD::ElectronContainer> > > neutralFE_ElectronLinks(m_neutralFEElectronsReadDecorKey,ctx);
  if (!neutralFE_ElectronLinks.isValid()){
    ATH_MSG_ERROR("Can't retrieve input decoration "<< neutralFE_ElectronLinks.key());                  
    return StatusCode::FAILURE;
  }

  SG::ReadDecorHandle<xAOD::FlowElementContainer, std::vector< ElementLink<xAOD::MuonContainer> > > neutralFE_MuonLinks(m_neutralFEMuonsReadDecorKey,ctx);
  if (!neutralFE_MuonLinks.isValid()){
    ATH_MSG_ERROR("Can't retrieve input decoration "<< neutralFE_MuonLinks.key());                  
    return StatusCode::FAILURE;
  }

  auto selectedChargedPFlowObjects = std::make_unique<xAOD::FlowElementContainer>(); // SG::VIEW_ELEMENTS
  auto selectedChargedPFlowObjectsAux = std::make_unique<xAOD::FlowElementAuxContainer>();
  selectedChargedPFlowObjects->setStore(selectedChargedPFlowObjectsAux.get());

  auto selectedNeutralPFlowObjects = std::make_unique<xAOD::FlowElementContainer>();
  auto selectedNeutralPFlowObjectsAux = std::make_unique<xAOD::FlowElementAuxContainer>();
  selectedNeutralPFlowObjects->setStore(selectedNeutralPFlowObjectsAux.get());

  // To store the charged FE objects matched to an electron/muon
  std::vector< const xAOD::FlowElement* > ChargedPFlowObjects_matched;

  FEMuonHelper muonHelper;
  FEElectronHelper electronHelper;

  // Loop over Charged FE objects
  for ( const xAOD::FlowElement* fe : *ChargedPFlowObjects ) {

    bool isMuonToExclude = muonHelper.checkMuonLinks(chargedFE_MuonLinks(*fe), m_muonIDToExclude);
    bool isMuonToInclude = muonHelper.checkMuonLinks(chargedFE_MuonLinks(*fe), m_muonIDToInclude);
    bool isElectronToInclude = electronHelper.checkElectronLinks(chargedFE_ElectronLinks(*fe), m_electronIDToInclude);
    bool isElectronToExclude = electronHelper.checkElectronLinks(chargedFE_ElectronLinks(*fe), m_electronIDToExclude);

    bool excludeChargedFE = false;

    //If we have only an exclude list we only need to do:
    if (m_excludeChargedElectronFE && isElectronToExclude) excludeChargedFE = true;
    if (m_excludeChargedMuonFE && isMuonToExclude) excludeChargedFE = true;

    //If we have an include list and the include ID is passed, we keep the FE object
    //Whilst if it otherwise passes excludeID we exclude it
    //e.g if we want medium  we should only include leptons that pass medium ID,
    //whilst excluding those that pass loose and NOT medium.
    if (m_includeChargedElectronFE && isElectronToInclude) excludeChargedFE = false;
    else if (m_includeChargedElectronFE && isElectronToExclude) excludeChargedFE = true;

    if (m_includeChargedMuonFE && isMuonToInclude) excludeChargedFE = false;
    else if (m_includeChargedMuonFE && isMuonToExclude) excludeChargedFE = true;
    
    // Select FE object if not matched to an electron or muon, to exclude, via links
    if ( !excludeChargedFE) {
      xAOD::FlowElement* selectedFE = new xAOD::FlowElement();
      selectedChargedPFlowObjects->push_back(selectedFE);
      *selectedFE = *fe; // copies auxdata
    }
    else { // Use the matched object to put back its energy later
      ChargedPFlowObjects_matched.push_back(fe);
    }

  } // End loop over Charged FE Objects

  // Loop over Neutral FE objects
  for ( const xAOD::FlowElement* fe : *NeutralPFlowObjects ) {

    bool isMuonToExclude = muonHelper.checkMuonLinks(neutralFE_MuonLinks(*fe), m_muonIDToExclude);
    bool isMuonToInclude = muonHelper.checkMuonLinks(neutralFE_MuonLinks(*fe), m_muonIDToInclude);
    bool isElectronToInclude = electronHelper.checkElectronLinks(neutralFE_ElectronLinks(*fe), m_electronIDToInclude);
    bool isElectronToExclude = electronHelper.checkElectronLinks(neutralFE_ElectronLinks(*fe), m_electronIDToExclude);

    //If we specify to exclude, then we exclude if exclude ID is passed
    //If we specify to include, then we include if include ID is passed - in this case
    //we might want to e.g include medium electrons, whilst excluding that poass loose ID
    if (m_excludeNeutralElectronFE && isElectronToExclude) continue;
    bool vetoElectron_IncludeMode = false;
    if (m_includeNeutralElectronFE && isElectronToInclude) vetoElectron_IncludeMode = false;
    else if (m_includeNeutralElectronFE && isElectronToExclude) vetoElectron_IncludeMode = true;
    if (vetoElectron_IncludeMode) continue;

    xAOD::FlowElement* selectedFE = new xAOD::FlowElement();
    selectedNeutralPFlowObjects->push_back(selectedFE);
    *selectedFE = *fe;

    bool excludeNeutralMuon = false;
    //Muons can have both exclude and include lists
    //Exclude case is the same as electron exclude case
    if (m_excludeNeutralMuonFE && isMuonToExclude) excludeNeutralMuon = true;
    //Include case is more complex - if muon passes includeID we keep it
    //Whilst if it otherwise passes excludeID we exclude it
    //e.g if we want medium muons we should only include muons that pass medium ID,
    //whilst excluding those that pass loose and NOT medium.
    if (m_includeNeutralMuonFE && isMuonToInclude) excludeNeutralMuon = false;
    else if (m_includeNeutralMuonFE && isMuonToExclude) excludeNeutralMuon = true;

    if (excludeNeutralMuon) {
      // If we are excluding muons, we need to adjust the energy of the neutral FE object
      // by subtracting the muon energy in this calorimeter cluster
      SG::ReadDecorHandle<xAOD::FlowElementContainer, std::vector<double> > clusterMuonEnergyFracs(m_neutralFEMuons_efrac_match_DecorKey,ctx); 
      selectedFE->setP4(muonHelper.adjustNeutralCaloEnergy(clusterMuonEnergyFracs(*fe),*fe));
    }

  } // End loop over Neutral FE Objects

  // Add the energy from removed charged FE clusters to neutral FE object 
  // if shared clusters exist, create the new neutral FE object otherwise
  for ( const xAOD::FlowElement* chargedFE : ChargedPFlowObjects_matched ){
      
    // Get charged FE topoclusters and weights
    std::vector<std::pair<const xAOD::IParticle*,float> > theOtherPairs_charged = chargedFE->otherObjectsAndWeights();
    std::vector<ElementLink<xAOD::IParticleContainer>> theOtherLinks_charged = chargedFE->otherObjectLinks();

    // Loop over charged FE topoclusters
    for (unsigned int iCluster = 0; iCluster < chargedFE->nOtherObjects(); ++iCluster){

      bool thisCluster_matched = false;

      std::pair<const xAOD::IParticle*,float> theOtherPair_charged = theOtherPairs_charged[iCluster];
      const xAOD::IParticle* theCluster_charged = theOtherPair_charged.first;
      float theClusterWeight_charged = theOtherPair_charged.second;

      // Loop over neutral FE objects
      for ( xAOD::FlowElement* neutralFE : *selectedNeutralPFlowObjects ) {
        if (thisCluster_matched) continue;

        // Loop over neutral FE topoclusters
        std::vector<std::pair<const xAOD::IParticle*,float> > theOtherPairs_neutral = neutralFE->otherObjectsAndWeights();
        for (auto& [theCluster_neutral, theClusterWeight_neutral] : theOtherPairs_neutral){

          // If topoclusters are matched, add the energy to the neutral FE object
          if (theCluster_charged == theCluster_neutral){

            // Add the energy to the neutral FE object
            float newEnergy = neutralFE->e() + theClusterWeight_charged;
            neutralFE->setP4(newEnergy/cosh(neutralFE->eta()), 
                            neutralFE->eta(),
                            neutralFE->phi(),
                            neutralFE->m());            

            ATH_MSG_DEBUG("Updated neutral FlowElement with E, pt, eta and phi: "
                    << neutralFE->e() << ", " << neutralFE->pt() << ", "
                    << neutralFE->eta() << " and " << neutralFE->phi());

            thisCluster_matched = true;
          }

        } // End loop over neutral FE clusters
      } // End loop over neutral FE objects

      // If a topocluster is left unmatched, create a neutral FE object.
      // Ignore topoclusters with nullptr
      if ( !thisCluster_matched && theCluster_charged ){

        //check if charged cluster belongs to an electron, before we put it back as neutral        
        bool belongsToElectron = false;
        if (m_excludeNeutralElectronFE){

          //get container index of charged cluster and compare to indices of electron topoclusters
          unsigned int chargedClusterIndex = theCluster_charged->index();

          SG::ReadHandle<xAOD::ElectronContainer> electronReadHandle(m_electronContainerKey,ctx);
          if (!electronReadHandle.isValid()){
            ATH_MSG_ERROR("Can't retrieve electron container "<< m_electronContainerKey.key());                  
            return StatusCode::FAILURE;
          }

          for (auto thisElectron : *electronReadHandle){
            const std::vector<const xAOD::CaloCluster*> electronTopoClusters = xAOD::EgammaHelpers::getAssociatedTopoClusters(thisElectron->caloCluster());
            for (auto thisElectronTopoCluster : electronTopoClusters){
              if (thisElectronTopoCluster->index() == chargedClusterIndex){
                belongsToElectron = true;                
                break;
              }
            }
          }
        }

        if (belongsToElectron) continue;

        bool belongsToMuon = false;
        double muonCaloEnergy = 0.0;
        if (m_excludeNeutralMuonFE){          
          SG::ReadDecorHandle<xAOD::FlowElementContainer, std::vector<double> > chargedFE_energy_match_muonReadHandle(m_chargedFE_energy_match_muonReadHandleKey,ctx);
          std::vector<double> muonCaloEnergies = chargedFE_energy_match_muonReadHandle(*chargedFE);
          muonCaloEnergy = muonCaloEnergies[iCluster];
        }

        if (belongsToMuon) continue;

        xAOD::FlowElement* newFE = new xAOD::FlowElement();
        selectedNeutralPFlowObjects->push_back(newFE);

        newFE->setP4((theClusterWeight_charged - muonCaloEnergy) / cosh(theCluster_charged->eta()),  // using energy from charged FE weight, not cluster->e()
                    theCluster_charged->eta(),
                    theCluster_charged->phi(),
                    theCluster_charged->m());
        newFE->setCharge(0);
        newFE->setSignalType(xAOD::FlowElement::SignalType::NeutralPFlow);

        ATH_MSG_DEBUG("Created neutral FlowElement with E, pt, eta and phi: "
                  << newFE->e() << ", " << newFE->pt() << ", "
                  << newFE->eta() << " and " << newFE->phi());

        std::vector<ElementLink<xAOD::IParticleContainer>> theClusters;
        ElementLink< xAOD::IParticleContainer > theIParticleLink;
        theIParticleLink.resetWithKeyAndIndex(theOtherLinks_charged[iCluster].persKey(), theOtherLinks_charged[iCluster].persIndex()); 

        theClusters.push_back(theIParticleLink);
        newFE->setOtherObjectLinks(theClusters);

        //Add Standard data to these new FlowElements
        FEHelpers::FillNeutralFlowElements FEFiller;
        const xAOD::CaloCluster* castCluster_charged = dynamic_cast<const xAOD::CaloCluster*>(theCluster_charged);
        FEFiller.addStandardMoments(*newFE,*castCluster_charged);        
        FEFiller.addStandardSamplingEnergies(*newFE,*castCluster_charged);    

        float layerEnergy_TileBar0 = castCluster_charged->eSample(xAOD::CaloCluster::CaloSample::TileBar0);
        float layerEnergy_TileExt0 = castCluster_charged->eSample(xAOD::CaloCluster::CaloSample::TileExt0);
        const static SG::AuxElement::Accessor<float> accFloatTIle0E("LAYERENERGY_TILE0");
        accFloatTIle0E(*newFE) = layerEnergy_TileBar0 + layerEnergy_TileExt0;

        const static SG::AuxElement::Accessor<float> accFloatTiming("TIMING");
        accFloatTiming(*newFE) = castCluster_charged->time();
      }
        
    } // End loop over topoclusters of removed charged FE objects
  } // End loop over removed charged FE objects


  auto handle_ChargedPFlow_out =  SG::makeHandle(m_outputChargedPFlowHandleKey, ctx);
  if (!handle_ChargedPFlow_out.record(std::move(selectedChargedPFlowObjects), std::move(selectedChargedPFlowObjectsAux)) ){
    ATH_MSG_ERROR("Can't record output PFlow container "<< m_outputChargedPFlowHandleKey);
    return StatusCode::FAILURE;
  }

  auto handle_NeutralPFlow_out =  SG::makeHandle(m_outputNeutralPFlowHandleKey, ctx);
  if (!handle_NeutralPFlow_out.record(std::move(selectedNeutralPFlowObjects), std::move(selectedNeutralPFlowObjectsAux)) ){
    ATH_MSG_ERROR("Can't record output PFlow container "<< m_outputNeutralPFlowHandleKey);
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
