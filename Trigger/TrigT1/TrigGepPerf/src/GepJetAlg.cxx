
/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "./GepJetAlg.h"

// Interface to jet reconstruction objects
#include "./IJetMaker.h"

// concrete jet reconstruction classes.
#include "./ModAntikTJetMaker.h"
#include "./ConeJetMaker.h"
#include "./WTAConeJetMaker.h"

// input and output types
#include "./Cluster.h"
#include "./Jet.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetAuxContainer.h"

GepJetAlg::GepJetAlg( const std::string& name, ISvcLocator* pSvcLocator ) :
    AthReentrantAlgorithm( name, pSvcLocator ){

}


StatusCode GepJetAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_MSG_INFO ("Jet alg " << m_jetAlgName);

  // Initialize data access keys
  CHECK(m_caloClustersKey.initialize());
  CHECK(m_jFexSRJetsKey.initialize());
  CHECK(m_outputGepJetsKey.initialize());

  return StatusCode::SUCCESS;
}


StatusCode GepJetAlg::execute(const EventContext& context) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  
  
  SG::WriteHandle<xAOD::JetContainer>
    h_outputJets = SG::makeHandle(m_outputGepJetsKey, context);


  CHECK(h_outputJets.record(std::make_unique<xAOD::JetContainer>(),
			    std::make_unique<xAOD::JetAuxContainer>()));
	
  // read in clusters
  auto h_caloClusters = SG::makeHandle(m_caloClustersKey, context);
  CHECK(h_caloClusters.isValid());
  ATH_MSG_DEBUG("Read in " << h_caloClusters->size() << " clusters");

  const auto& clusters = *h_caloClusters; 



  std::vector<Gep::Cluster> gepClusters;
  std::transform(clusters.cbegin(),
		 clusters.cend(),
		 std::back_inserter(gepClusters),
		 [](const auto& cluster){
		   return Gep::Cluster(cluster->p4());});
		 


  // create a  jet maker
  std::unique_ptr<Gep::IJetMaker> jetMaker{};
  

  if ( m_jetAlgName=="ModAntikT" ) {
    jetMaker.reset(new Gep::ModAntikTJetMaker());
  }
  
  else if ( m_jetAlgName=="Cone" ) {

    // Use jJFexSR RoIs as seeds
    auto h_seeds = SG::makeHandle(m_jFexSRJetsKey, context);
    CHECK(h_seeds.isValid());
    ATH_MSG_DEBUG("No of seeds "<< h_seeds->size());
    jetMaker.reset(new Gep::ConeJetMaker(0.4, *h_seeds));
    
  } else if(m_jetAlgName=="WTACone"){ // Large block for the WTACone

    auto WTAConeJetMaker = std::make_unique<Gep::WTAConeJetMaker>(); // Default parameters for now

    WTAConeJetMaker->m_GEPWTAParameters.SetConstEtCut(m_WTAConstEtCut * Athena::Units::GeV); // Set ConstEtCut to 2GeV
    WTAConeJetMaker->m_GEPWTAParameters.SetSeedEtCut(m_WTASeedEtCut * Athena::Units::GeV); // Set SeedEtCut to 5GeV by default
    WTAConeJetMaker->m_GEPWTAParameters.SetJet_dR2(m_WTAJet_dR2);
    WTAConeJetMaker->m_GEPWTAParameters.SetIso_dR2(m_WTAJet_dR2); // Default is Jet_dR2 = Iso_dR2
    WTAConeJetMaker->m_GEPWTAParameters.SetMaxConstN(m_WTAMaxConstN);
    WTAConeJetMaker->m_GEPWTAParameters.SetMaxSeedSortingN(m_WTAMaxSeedSortingN);
    WTAConeJetMaker->SetBlockN(m_WTABlockN);

    WTAConeJetMaker->SetSeedCleaningAlgo(0); // 0 = Baseline
    if(m_WTASeedCleaningName=="TwoPass")WTAConeJetMaker->SetSeedCleaningAlgo(1); // 1 = TwoPass

    jetMaker = std::move(WTAConeJetMaker);
  } // WTACone loop, will be updated as the WTAConeJets
  else {
    ATH_MSG_ERROR( "Unknown JetMaker " <<  m_jetAlgName);
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG( "jet maker: " << jetMaker->toString());
  
  std::vector<Gep::Jet> gepJets = jetMaker->makeJets( gepClusters );
  
  ATH_MSG_DEBUG("Number of jets found for " <<
		 m_jetAlgName << " " <<gepJets.size());

  // if no jets were found, skip event
  if( gepJets.empty() ){
    return StatusCode::SUCCESS;
  }
  
  // store gep jets in athena format
  for(const auto& gjet: gepJets){
    
    std::unique_ptr<xAOD::Jet> xAODJet{new xAOD::Jet()};
    xAOD::Jet* p_xAODJet = xAODJet.get();

    // store the xAOD::Jet in the output container to prepare the Aux container
    // The move invalids the unique_ptr, but we still have the bare pointer
    // which allows the updating of the xAODJet from the gep jet data.
    h_outputJets->push_back(std::move(xAODJet));
    
    xAOD::JetFourMom_t p4;
    p4.SetPt(gjet.vec.Pt());
    p4.SetEta(gjet.vec.Eta());
    p4.SetPhi(gjet.vec.Phi());
    p4.SetM(gjet.vec.M());

    p_xAODJet->setJetP4(p4);
    
    p_xAODJet->setAttribute("RCut", gjet.radius);
    p_xAODJet->setAttribute("SeedEta", gjet.seedEta); // < gep attributes
    p_xAODJet->setAttribute("SeedPhi", gjet.seedPhi); //
    p_xAODJet->setAttribute("SeedEt", gjet.seedEt); //

    for (const auto& i: gjet.constituentsIndices) {
      p_xAODJet->addConstituent(clusters.at(i));
    }
    
  }
	
  return StatusCode::SUCCESS;
}
