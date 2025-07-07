/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <memory>
#include <cstdlib>
#include <iostream>

// ROOT include(s):
#include <TFile.h>
#include <TError.h>
#include <TString.h>

// Infrastructure include(s):
#ifdef ROOTCORE
#   include "xAODRootAccess/Init.h"
#   include "xAODRootAccess/TEvent.h"
#endif // ROOTCORE
#include "xAODCore/ShallowCopy.h"

// EDM include(s):
#include "xAODEventInfo/EventInfo.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODCore/ShallowCopy.h"

#include "AsgTools/ToolHandle.h"

// Local include(s):
#include "TauAnalysisTools/TauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/TauSelectionTool.h"
#include "TauAnalysisTools/TauSmearingTool.h"
#include "TauAnalysisTools/TauTruthMatchingTool.h"

// Smart Slimming include(s):
#include "xAODCore/tools/IOStats.h"
#include "xAODCore/tools/ReadStats.h"

using namespace TauAnalysisTools;

#define CHECK( ARG )					\
  do {                                                  \
    const bool result = ARG;				\
    if( ! result ) {					\
      ::Error( "TauAnalysisToolsExample", "Failed to execute: \"%s\"",	\
	       #ARG );					\
      return 1;						\
    }							\
  } while( false )

#define RETRIEVE( TYPE, CONTAINER , NAME )				\
  do {									\
  if (xEvent.contains<TYPE>(NAME))					\
    CHECK( xEvent.retrieve( CONTAINER, NAME ) );			\
  else									\
    Warning("TauAnalysisToolsExample","%s container is not available", NAME);		\
  } while(false)							\
 
int main( int argc, char* argv[] )
{
  StatusCode::enableFailure();

  // Check if we received a file name:
  if( argc < 2 )
  {
    Error( "TauAnalysisToolsExample", "No file name received!" );
    Error( "TauAnalysisToolsExample", "  Usage: %s [xAOD file name]", "TauAnalysisToolsExample" );
    return 1;
  }

  // Initialise the application:
  CHECK( xAOD::Init( "TauAnalysisToolsExample" ) );

  // Open the input file:
  const TString sInputFileName = argv[ 1 ];
  Info( "TauAnalysisToolsExample", "Opening input file: %s", sInputFileName.Data() );
  std::unique_ptr< TFile > fInputFile( TFile::Open( sInputFileName, "READ" ) );
  CHECK( fInputFile.get() );

  // Create the output file:
  TString sOutputFileName = "output.root";
  if (argc>3)
    sOutputFileName = TString(argv[3]);

  //Do the trigger efficiency tools, requires correct ilumicalc
  bool m_doTrigger =  bool(argc>4);
  
  Info( "TauAnalysisToolsExample", "Opening output file: %s", sOutputFileName.Data() );
  std::unique_ptr< TFile > fOutputFile( TFile::Open( sOutputFileName, "RECREATE" ) );
  CHECK( fOutputFile.get() );

  // Create a TEvent object:
  // xAOD::TEvent xEvent( xAOD::TEvent::kClassAccess );
  xAOD::TEvent xEvent( xAOD::TEvent::kAthenaAccess );
  CHECK( xEvent.readFrom( fInputFile.get() ) );

  // Connect TEvent with output file :
  CHECK( xEvent.writeTo( fOutputFile.get() ) );

  Info( "TauAnalysisToolsExample", "Number of events in the file: %i",
        static_cast< int >( xEvent.getEntries() ) );

  // Decide how many events to run over:
  Long64_t iEntries = xEvent.getEntries();
  if( argc > 2 )
  {
    const Long64_t iMaxEntries = atoll( argv[ 2 ] );
    if( iMaxEntries < iEntries )
    {
      iEntries = iMaxEntries;
    }
  }

  // defining needed Container
  const xAOD::EventInfo* xEventInfo = 0;
  const xAOD::TauJetContainer* xTauJetContainer = 0;

  // ===========================================================================
  // TauSelectionTool
  // ===========================================================================
  TauAnalysisTools::TauSelectionTool* TauSelTool = new TauAnalysisTools::TauSelectionTool( "TauSelectionTool" );
  TauSelTool->msg().setLevel( MSG::DEBUG );
  // preparation for control hisograms
  TauSelTool->setOutFile( fOutputFile.get() );
  CHECK(TauSelTool->setProperty("CreateControlPlots", true ));
  CHECK(TauSelTool->setProperty("JetIDWP", int(JETIDRNNMEDIUM) ));
  CHECK(TauSelTool->setProperty("EleIDWP", int(ELEIDRNNLOOSE) ));
  CHECK(TauSelTool->setProperty("EleIDVersion", 1 ));
  CHECK(TauSelTool->setProperty("PtMin", 20. ));
  CHECK(TauSelTool->setProperty("ConfigPath", "" ));
  CHECK(TauSelTool->setProperty("SelectionCuts", int(CutPt|CutJetIDWP|CutEleIDWP) ));
  CHECK(TauSelTool->initialize());

  // ===========================================================================
  // TauSmearingTool
  // ===========================================================================
  TauAnalysisTools::TauSmearingTool TauSmeTool( "TauSmearingTool" );
  TauSmeTool.msg().setLevel( MSG::DEBUG );
  CHECK(TauSmeTool.setProperty("RecommendationTag","2025-prerec"));
  CHECK(TauSmeTool.setProperty("Campaign","mc23")); // can be also set to mc20 depending on the mc campaign 
  CHECK(TauSmeTool.initialize());

  // restructure all recommended systematic variations for smearing tool
  std::vector<CP::SystematicSet> vSmearingSystematicSet;
  for (auto SystematicsVariation : TauSmeTool.recommendedSystematics())
  {
    vSmearingSystematicSet.push_back(CP::SystematicSet());
    vSmearingSystematicSet.back().insert(SystematicsVariation);
  }

  // ===========================================================================
  // TauEfficiencyCorrectionsTool
  // ===========================================================================
  TauAnalysisTools::TauEfficiencyCorrectionsTool TauEffCorrTool( "TauEfficiencyCorrectionsTool" );
  TauEffCorrTool.msg().setLevel( MSG::VERBOSE );
  CHECK(TauEffCorrTool.setProperty("JetIDLevel", static_cast<int>(TauAnalysisTools::JetID::JETIDRNNMEDIUM)));
  CHECK(TauEffCorrTool.setProperty("EfficiencyCorrectionTypes", static_cast<int>(TauAnalysisTools::EfficiencyCorrectionType::SFJetIDHadTau)));
  CHECK(TauEffCorrTool.setProperty("RecommendationTag","2025-prerec"));
  CHECK(TauEffCorrTool.setProperty("Campaign", "mc23")); // can be also set to mc20 depending on the mc campaign 
  CHECK(TauEffCorrTool.initialize());

  // restructure all recommended systematic variations for efficiency tools
  std::vector<CP::SystematicSet> vEfficiencyCorrectionsSystematicSet;
  vEfficiencyCorrectionsSystematicSet.push_back(CP::SystematicSet());
  for (auto SystematicsVariation : TauEffCorrTool.recommendedSystematics())
  {
    vEfficiencyCorrectionsSystematicSet.push_back(CP::SystematicSet());
    vEfficiencyCorrectionsSystematicSet.back().insert(SystematicsVariation);
  }

  // ===========================================================================
  // TauEfficiencyCorrectionsTriggerTool
  // ===========================================================================
  TauAnalysisTools::TauEfficiencyCorrectionsTool TauEffTrigTool( "TauEfficiencyCorrectionsTriggerTool" );
  // restructure all recommended systematic variations for efficiency tools
  std::vector<CP::SystematicSet> vEfficiencyCorrectionsTriggerSystematicSet;
  if (m_doTrigger){

    TauEffTrigTool.msg().setLevel( MSG::DEBUG );
    CHECK(TauEffTrigTool.setProperty("EfficiencyCorrectionTypes", std::vector<int>({SFTriggerHadTau}) ));
    CHECK(TauEffTrigTool.setProperty("TriggerName", "HLT_tau25_mediumRNN_tracktwoMVA" ));
    CHECK(TauEffTrigTool.setProperty("JetIDLevel", static_cast<int>(JETIDRNNMEDIUM) ));
    CHECK(TauEffTrigTool.setProperty("Campaign", "mc23a")); // can be also set to mc23d depending on the mc campaign 
    CHECK(TauEffTrigTool.initialize());

    vEfficiencyCorrectionsTriggerSystematicSet.push_back(CP::SystematicSet());
    for (auto SystematicsVariation : TauEffTrigTool.recommendedSystematics())
    {
      vEfficiencyCorrectionsTriggerSystematicSet.push_back(CP::SystematicSet());
      vEfficiencyCorrectionsTriggerSystematicSet.back().insert(SystematicsVariation);
    }
  }  
  // ===========================================================================
  // TauTruthMatchingTool
  // ===========================================================================
  TauAnalysisTools::TauTruthMatchingTool T2MT( "TauTruthMatchingTool");
  T2MT.msg().setLevel( MSG::INFO );
  CHECK(T2MT.setProperty("TruthJetContainerName", "AntiKt4TruthDressedWZJets"));
  CHECK(T2MT.initialize());

  static const SG::ConstAccessor<char> acc_IsTruthMatched("IsTruthMatched");
  static const SG::ConstAccessor<char> acc_IsHadronicTau("IsHadronicTau");
  static const SG::ConstAccessor<size_t> acc_numCharged("numCharged");
  static const SG::ConstAccessor<ElementLink< xAOD::JetContainer >> acc_truthJetLink("truthJetLink");
  static const SG::ConstAccessor<double> acc_TauSFRecoHadTau("TauScaleFactorReconstructionHadTau");
  static const SG::ConstAccessor<double> acc_TauSFJetIDHadTau("TauScaleFactorJetIDHadTau");
  static const SG::ConstAccessor<double> acc_TauSFEleIDHadTau("TauScaleFactorEleIDHadTau");
  static const SG::ConstAccessor<double> acc_TauSFEleIDEle("TauScaleFactorEleIDElectron");
  static const SG::ConstAccessor<double> acc_TauSFTrigHadTau("TauScaleFactorTriggerHadTau");

  // Loop over the events:
  for( Long64_t iEntry = 0; iEntry < iEntries; ++iEntry )
  {
    // Tell the object which entry to look at:
    xEvent.getEntry( iEntry );

    // Print some event information for fun:
    RETRIEVE(xAOD::EventInfo, xEventInfo, "EventInfo");
    if (xEventInfo)
      Info( "TauAnalysisToolsExample",
            "===>>>  start processing event #%i, "
            "run #%i %i events processed so far  <<<===",
            static_cast< int >( xEventInfo->eventNumber() ),
            static_cast< int >( xEventInfo->runNumber() ),
            static_cast< int >( iEntry ) );


    //Check TauJet Container Name
    const char * m_tauJetContainerName = "TauJets";
    if (xEvent.contains<xAOD::TauJetContainer>(m_tauJetContainerName)){			  
      RETRIEVE(xAOD::TauJetContainer, xTauJetContainer, m_tauJetContainerName);
    }else{
      m_tauJetContainerName = "AnalysisTauJets";
      RETRIEVE(xAOD::TauJetContainer, xTauJetContainer, m_tauJetContainerName);
    }
    std::pair< xAOD::TauJetContainer*, xAOD::ShallowAuxContainer* >xTauShallowContainer = xAOD::shallowCopyContainer(*xTauJetContainer);
    if(iEntry==0){
      Info( "TauAnalysisToolsExample:: TauJetContainer = ",m_tauJetContainerName);
    }

    // // copy truth particles to get truthparticle link for truth taus to work
    if (xEvent.contains<xAOD::TruthParticleContainer>("TruthParticles"))
      CHECK( xEvent.copy("TruthParticles") );

    // copy taus
    CHECK( xEvent.copy(m_tauJetContainerName) );

    // copy tracks
    CHECK( xEvent.copy("InDetTrackParticles") );

    // Print tau properties, using the tools:
    for ( auto xTau : *xTauShallowContainer.first )
    {
      // perform truth matching
      auto xTruthTau = T2MT.getTruth(*xTau);

      if (static_cast<bool>(acc_IsTruthMatched(*xTau)))
      {
        if (xTruthTau->isTau())
        {
          if (static_cast<bool>(acc_IsHadronicTau(*xTruthTau)))
            Info( "TauAnalysisToolsExample",
                  "Tau was matched to a truth hadronic tau, which has %i prongs and a charge of %i",
                  int(acc_numCharged(*xTruthTau)),
                  int(xTruthTau->charge()));
          else
            Info( "TauAnalysisToolsExample",
                  "Tau was matched to a truth leptonic tau, which has a charge of %i",
                  int(xTruthTau->charge()));
        }
        else if (xTruthTau->isElectron())
          Info( "TauAnalysisToolsExample",
                "Tau was matched to a truth electron");
        else if (xTruthTau->isMuon())
          Info( "TauAnalysisToolsExample",
                "Tau was matched to a truth muon");
      }
      else
        Info( "TauAnalysisToolsExample", "Tau was not matched to truth" );

      auto xTruthJetLink = acc_truthJetLink(*xTau);
      if (xTruthJetLink.isValid())
      {
        const xAOD::Jet* xTruthJet = *xTruthJetLink;
        Info( "TauAnalysisToolsExample",
              "Tau was matched to a truth jet, which has pt=%g, eta=%g, phi=%g, m=%g",
              xTruthJet->p4().Pt(),
              xTruthJet->p4().Eta(),
              xTruthJet->p4().Phi(),
              xTruthJet->p4().M());
      }
      else
        Info( "TauAnalysisToolsExample", "Tau was not matched to truth jet" );

      Info( "TauAnalysisToolsExample",
              "Un-Smeared tau pt: %g ",
              xTau->ptFinalCalib());
      CHECK( TauSmeTool.applyCorrection(*xTau) );
      Info( "TauAnalysisToolsExample",
              "Smeared tau pt: %g ",
	    xTau->pt());

      for (auto sSystematicSet: vSmearingSystematicSet)
      {
        CHECK( TauSmeTool.applySystematicVariation(sSystematicSet)) ;
        CHECK( TauSmeTool.applyCorrection(*xTau) );
        //Skip TES uncertainty print out for non-had taus
        if (static_cast<bool>(acc_IsTruthMatched(*xTau)) && xTruthTau->isTau() && static_cast<bool>(acc_IsHadronicTau(*xTruthTau))){
        Info( "TauAnalysisToolsExample",
              "Smeared tau pt: %g for type %s ",
              xTau->pt(),
              sSystematicSet.name().c_str());
        }
      }
 
      // Select "good" taus:
      if( ! TauSelTool->accept( *xTau ) ){
        Info( "TauAnalysisToolsExample",
              "Tau does not pass selection tool: pt %g ",
              xTau->pt());
	continue;
      }

      for (auto sSystematicSet: vEfficiencyCorrectionsSystematicSet)
      {
        CHECK( TauEffCorrTool.applySystematicVariation(sSystematicSet));
        CHECK( TauEffCorrTool.applyEfficiencyScaleFactor(*xTau) );
        Info( "TauAnalysisToolsExample",
              "SystType %s: RecoSF: %g JetIDSF: %g EleOLRSFHadTau: %g EleRNNSFElectron: %g",
              sSystematicSet.name().c_str(),
              acc_TauSFRecoHadTau(*xTau),
              acc_TauSFJetIDHadTau(*xTau),
              acc_TauSFEleIDHadTau(*xTau),
              acc_TauSFEleIDEle(*xTau));
      }

      for (auto sSystematicSet: vEfficiencyCorrectionsTriggerSystematicSet)
      {
        CHECK( TauEffTrigTool.applySystematicVariation(sSystematicSet));
        CHECK( TauEffTrigTool.applyEfficiencyScaleFactor(*xTau) );
        Info( "TauAnalysisToolsExample",
              "SystType %s: Trigger: %g",
              sSystematicSet.name().c_str(),
              acc_TauSFTrigHadTau(*xTau));
      }
      // print some info about the selected tau:
      Info( "TauAnalysisToolsExample", "Selected tau: pt = %g MeV, eta = %g, phi = %g, prong = %i, charge = %i",
            xTau->pt(), xTau->eta(), xTau->phi(), int(xTau->nTracks()), int(xTau->charge()));
    }
    if (xTauJetContainer->empty())
      CHECK (T2MT.retrieveTruthTaus());
    xEvent.fill();
  }

  TauSelTool->writeControlHistograms();
  delete TauSelTool;
  CHECK( xEvent.finishWritingTo(fOutputFile.get()));
  fOutputFile.get()->Close();
  Info( "TauAnalysisToolsExample", "Finished writing to file: %s", sOutputFileName.Data() );

  // smart slimming
  xAOD::IOStats::instance().stats().printSmartSlimmingBranchList();

  return 0;
}
