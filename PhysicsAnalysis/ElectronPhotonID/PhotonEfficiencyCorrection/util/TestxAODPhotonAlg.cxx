/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <memory>
#include <cstdlib>

// ROOT include(s):
#include <TFile.h>
#include <TError.h>
#include <TString.h>

// Infrastructure include(s):
#ifdef ROOTCORE
#   include "xAODRootAccess/Init.h"
#   include "xAODRootAccess/TEvent.h"
#   include "xAODRootAccess/TStore.h"
#endif // ROOTCORE

// EDM include(s):
#include "PhotonEfficiencyCorrection/AsgPhotonEfficiencyCorrectionTool.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODCore/ShallowCopy.h"
#include <PATInterfaces/SystematicsUtil.h>

#include "AsgMessaging/MessageCheck.h"
#include "AsgMessaging/MsgStream.h"
// To disable sending data
#include "xAODRootAccess/tools/TFileAccessTracer.h"

#include <iostream>
#include <string>

namespace asg {
ANA_MSG_HEADER(TestxAODPhotonAlg)
ANA_MSG_SOURCE(TestxAODPhotonAlg, "")
}

int main( int argc, char* argv[] ) {

 


  xAOD::TFileAccessTracer::enableDataSubmission(false);
   // The application's name:
   const char* APP_NAME = argv[ 0 ];

  using namespace asg::TestxAODPhotonAlg;
  ANA_CHECK_SET_TYPE(int);
  MSG::Level mylevel = MSG::DEBUG;
  setMsgLevel(mylevel);
  msg().setName(APP_NAME);

   // Check if we received a file name:
   if( argc < 2 ) {
     Error( APP_NAME, "No file name received!" );
     Error( APP_NAME, "  Usage: %s [xAOD file name]", APP_NAME );
      return 1;
   }
   
   // Initialise the application:
   ANA_CHECK( xAOD::Init( APP_NAME ) );

   // Open the input file:
   const TString fileName = argv[ 1 ];
   Info( APP_NAME, "Opening file: %s", fileName.Data() );
   std::unique_ptr< TFile > ifile( TFile::Open( fileName, "READ" ) );
   ANA_CHECK( ifile.get() );

   // Create a TEvent object:
//   xAOD::TEvent event( xAOD::TEvent::kBranchAccess ); //will work for a sample produced in devval
 	xAOD::TEvent event( xAOD::TEvent::kClassAccess );   
   ANA_CHECK( event.readFrom( ifile.get() ) );
   Info( APP_NAME, "Number of events in the file: %i",
         static_cast< int >( event.getEntries() ) );
		 
		 
   // Decide how many events to run over:
   //Long64_t entries = event.getEntries();
   Long64_t entries = 10;//event.getEntries();
   if( argc > 2 ) {
      const Long64_t e = atoll( argv[ 2 ] );
      if( e < entries ) {
         entries = e;
      }
   }

	// Initialize photonFS tool
   AsgPhotonEfficiencyCorrectionTool photonSF_ID("AsgPhotonEfficiencyCorrectionTool_idSF");
   AsgPhotonEfficiencyCorrectionTool photonSF_Iso("AsgPhotonEfficiencyCorrectionTool_isoSF");
   AsgPhotonEfficiencyCorrectionTool photonSF_Trig("AsgPhotonEfficiencyCorrectionTool_TrigSF");

   
   photonSF_ID.msg().setLevel( mylevel );
   photonSF_Iso.msg().setLevel( mylevel );
    photonSF_Trig.msg().setLevel( mylevel );


   //Set Properties for photonID_SF tool
   ANA_CHECK(photonSF_ID.setProperty("ForceDataType",1));

   //Set Properties for photonISO_SF tool
   ANA_CHECK(photonSF_Iso.setProperty("IsoKey","Loose"));  // Set isolation WP: Loose,Tight,TightCaloOnly
   ANA_CHECK(photonSF_Iso.setProperty("ForceDataType",1)); //set data type: 1 for FULLSIM, 3 for AF2

   //Set Properties for PhotonTrig_SF tool
   ANA_CHECK(photonSF_Trig.setProperty("IsoKey","Loose"));  // Set isolation WP: Loose,Tight,TightCaloOnly
   ANA_CHECK(photonSF_Trig.setProperty("TriggerKey","DI_PH_2015_2016_g25_loose_2017_2018_g50_loose_L1EM20VH"));  // Set photon trigger
   ANA_CHECK(photonSF_Trig.setProperty("ForceDataType",1)); //set data type: 1 for FULLSIM, 3 for AF2

   // If the Pileup reweighting tool is not initialized, one can use next properties:
   ANA_CHECK(photonSF_ID.setProperty("UseRandomRunNumber",false));
   ANA_CHECK(photonSF_ID.setProperty("DefaultRandomRunNumber",428648)); // first runnumber of physics in run-3
   ANA_CHECK(photonSF_ID.setProperty("CorrelationModel","FULL")); 
   

   ANA_CHECK(photonSF_Iso.setProperty("UseRandomRunNumber",false));
   ANA_CHECK(photonSF_Iso.setProperty("DefaultRandomRunNumber",428648)); // first runnumber of physics in run-3
   ANA_CHECK(photonSF_Trig.setProperty("UseRandomRunNumber",false));
   ANA_CHECK(photonSF_Trig.setProperty("DefaultRandomRunNumber",349534)); // first runnumber of 2018 data taking

  
   if(!photonSF_ID.initialize()){
     std::cout <<"Failed to initialize the tool, check for errors"<<std::endl;
     return 1;
   }
    if(!photonSF_Iso.initialize()){
     std::cout <<"Failed to initialize the tool, check for errors"<<std::endl;
     return 1;
   }
   if(!photonSF_Trig.initialize()){
     std::cout <<"Failed to initialize the tool, check for errors"<<std::endl;
     return 1;
     }
   
   // Test that recommended systematics properly bieng registered:
   std::vector<CP::SystematicSet> sysList;
   const CP::SystematicRegistry& registry = CP::SystematicRegistry::getInstance();
   const CP::SystematicSet& recommendedSystematics = registry.recommendedSystematics();
   sysList = CP::make_systematics_vector(recommendedSystematics); // replaces continuous systematics with the +/-1 sigma variation
   std::cout << "List of recommended systematics from the registry:"<<std::endl;
   for (auto sysListItr = recommendedSystematics.begin(); sysListItr != recommendedSystematics.end(); ++sysListItr){
     std::cout <<(*sysListItr).name()<<std::endl;
   }


   
   // restructure all recommended systematic variations for the SF tool
   // for +/- nsigma variation see
   // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/PhotonEfficiencyRun2#Systematic_variations

   std::cout << "restructure all recommended systematic variations for the SF tool"<<std::endl;
   std::vector<CP::SystematicSet> syst_PhotonID, syst_PhotonIso, syst_PhotonTrig;
   for (const auto& SystematicsVariation : CP::make_systematics_vector(photonSF_ID.recommendedSystematics()))
   {
     syst_PhotonID.emplace_back();
   	 syst_PhotonID.back().insert(SystematicsVariation);
   }
   for (const auto& SystematicsVariation : CP::make_systematics_vector(photonSF_Iso.recommendedSystematics()))
   {
     syst_PhotonIso.emplace_back();
   	 syst_PhotonIso.back().insert(SystematicsVariation);
   }
   for (const auto& SystematicsVariation : CP::make_systematics_vector(photonSF_Trig.recommendedSystematics()))
   {
     syst_PhotonTrig.emplace_back();
         syst_PhotonTrig.back().insert(SystematicsVariation);
	 }
   //Print all recomended systemtaics
   for (const auto& sSystematicSet: syst_PhotonID){
	  Info(APP_NAME,"PhotonEfficiencyCorrectionTool ID instance has next systematic variation  %s ",sSystematicSet.name().c_str());
   }
   for (const auto& sSystematicSet: syst_PhotonIso){
	  Info(APP_NAME,"PhotonEfficiencyCorrectionTool Iso instance has next systematic variation  %s ",sSystematicSet.name().c_str());
   }   
   for (const auto& sSystematicSet: syst_PhotonTrig){
          Info(APP_NAME,"PhotonEfficiencyCorrectionTool Iso instance has next systematic variation  %s ",sSystematicSet.name().c_str());
   }
    

     
   double efficiencyScaleFactor=0, efficiencyScaleFactorError=0;
   // Loop over the events:
   std::cout << "loop on " << entries << " entries"<<std::endl;
   //for( int entry = 0; entry < entries; ++entry ) {
   for( int entry = 0; entry < 1; ++entry ) {

     // Tell the object which entry to look at:
     event.getEntry( entry );   
		
	// Get the Photon container from the event:
    const xAOD::PhotonContainer *photons = nullptr;
	ANA_CHECK( event.retrieve( photons, "Photons" ) );
	
	//Clone 
	std::pair< xAOD::PhotonContainer*, xAOD::ShallowAuxContainer* > photons_shallowCopy = xAOD::shallowCopyContainer( *photons );

	//Iterate over the shallow copy
    xAOD::PhotonContainer* phsCorr = photons_shallowCopy.first;
    xAOD::PhotonContainer::iterator ph_itr      = phsCorr->begin();
    xAOD::PhotonContainer::iterator ph_end      = phsCorr->end();

    static const SG::ConstAccessor<float> sfAcc("SF");
	
    unsigned int i = 0;
    for( ; ph_itr != ph_end; ++ph_itr, ++i ) {
          xAOD::Photon* ph = *ph_itr;

	  // skip photons with pt outsize the acceptance
	  if(ph->pt()<10000.0) continue;
	  const xAOD::CaloCluster* cluster  = ph->caloCluster();  
	  if (!cluster){
	    ATH_MSG_ERROR("No  cluster associated to the Photon \n"); 
	    return  1;
	  } 
	  if( std::abs(cluster->etaBE(2))>2.37) continue;
	   Info (APP_NAME,"Event #%d, Photon #%d", entry, i); 
	   Info (APP_NAME,"xAOD/raw pt = %f, eta = %f ", ph->pt(), ph->eta() ); 
	   
	   // will set the systematic variation to nominal - no need to do it if applySystematicVariation("UP/DOWN") is not called
	   ANA_CHECK(photonSF_ID.applySystematicVariation(syst_PhotonID.at(0)));
	   ANA_CHECK(photonSF_Iso.applySystematicVariation(syst_PhotonIso.at(0)));
   
	   // Get photon ID SF and the uncertainty
	   ANA_CHECK(photonSF_ID.getEfficiencyScaleFactor(*ph,efficiencyScaleFactor));
	   ANA_CHECK(photonSF_ID.getEfficiencyScaleFactorError(*ph,efficiencyScaleFactorError));
	   ANA_CHECK(photonSF_Trig.applySystematicVariation(syst_PhotonTrig.at(0)));
	   
       Info( APP_NAME,"===>>> Result ID: ScaleFactor %f, TotalUncertainty %f ",efficiencyScaleFactor,efficiencyScaleFactorError);

	   // Get photon SF and the uncertainty
	   ANA_CHECK(photonSF_Iso.getEfficiencyScaleFactor(*ph,efficiencyScaleFactor));
	   ANA_CHECK(photonSF_Iso.getEfficiencyScaleFactorError(*ph,efficiencyScaleFactorError));
	   
       Info( APP_NAME,"===>>> Result Iso: ScaleFactor %f, TotalUncertainty %f ",efficiencyScaleFactor,efficiencyScaleFactorError);

	   // Get photon trigger SF and the uncertainty
       ANA_CHECK(photonSF_Trig.getEfficiencyScaleFactor(*ph,efficiencyScaleFactor));
       ANA_CHECK(photonSF_Trig.getEfficiencyScaleFactorError(*ph,efficiencyScaleFactorError));

       Info( APP_NAME,"===>>> Result Trigger: ScaleFactor %f, TotalUncertainty %f ",efficiencyScaleFactor,efficiencyScaleFactorError);
	   
	   // decorate photon (for different name use photonSF_ID.setProperty("ResultName","ID_"); or photonSF_Iso.setProperty("ResultName","Iso_");)
	   ANA_CHECK(photonSF_ID.applyEfficiencyScaleFactor(*ph));
	   Info( "applyEfficiencyScaleFactor()","===>>> new decoration: \"SF\"=%f", sfAcc(*ph));
	   ANA_CHECK(photonSF_Iso.applyEfficiencyScaleFactor(*ph));
	   Info( "applyEfficiencyScaleFactor()","===>>> new decoration: \"SF\"=%f", sfAcc(*ph));
	   ANA_CHECK(photonSF_Trig.applyEfficiencyScaleFactor(*ph));
	   Info( "applyEfficiencyScaleFactor()","===>>> new decoration: \"SF\"=%f", sfAcc(*ph));
	   
	   // get SF for all recommended systematic variations (nominal is also included):
	   for (const auto& sSystematicSet: syst_PhotonID){
		ANA_CHECK(photonSF_ID.applySystematicVariation(sSystematicSet));
	    ANA_CHECK(photonSF_ID.getEfficiencyScaleFactor(*ph,efficiencyScaleFactor));
		Info( APP_NAME,"===>>> apply %s: ScaleFactor = %f",photonSF_ID.appliedSystematics().name().c_str(),efficiencyScaleFactor);
		}
	   for (const auto& sSystematicSet: syst_PhotonIso){
	     ANA_CHECK(photonSF_Iso.applySystematicVariation(sSystematicSet));
	     ANA_CHECK(photonSF_Iso.getEfficiencyScaleFactor(*ph,efficiencyScaleFactor));
	     Info( APP_NAME,"===>>> apply %s: ScaleFactor = %f",photonSF_Iso.appliedSystematics().name().c_str(),efficiencyScaleFactor);
	   }
	   for (const auto& sSystematicSet: syst_PhotonTrig){
                ANA_CHECK(photonSF_Trig.applySystematicVariation(sSystematicSet));
            ANA_CHECK(photonSF_Trig.getEfficiencyScaleFactor(*ph,efficiencyScaleFactor));
                Info( APP_NAME,"===>>> apply %s: ScaleFactor = %f",photonSF_Trig.appliedSystematics().name().c_str(),efficiencyScaleFactor);
		}	   

	}  // END LOOP ON PHOTONS

    
   } // END LOOP ON EVENTS

   // Return gracefully:
   return 0;
   
} // END PROGRAM

/*
	if(argc!=2){
		printf("input parameters:\nTestxAODPhotonTool [path]\n");
		printf("example: TestxAODPhotonTool /afs/cern.ch/work/k/krasznaa/public/xAOD/19.0.X_rel_4/mc12_8TeV.105200.McAtNloJimmy_CT10_ttbar_LeptonFilter.AOD.19.0.X_rel_4.pool.root\n");
		return 0;
	}	
*/
