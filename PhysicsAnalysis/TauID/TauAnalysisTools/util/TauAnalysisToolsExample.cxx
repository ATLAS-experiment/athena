/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
#include "TauAnalysisTools/TauTruthTrackMatchingTool.h"

#include "PileupReweighting/PileupReweightingTool.h"

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

  CP::PileupReweightingTool* m_tPRWTool = new CP::PileupReweightingTool("PileupReweightingTool");
  if (m_doTrigger){//dont initialize tool if not requested
    std::vector<std::string> vLumiCalcFiles = {"/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data15_13TeV/20200803/ilumicalc_histograms_None_282026-282457_OflLumi-13TeV-009.root","/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data16_13TeV/20190708/ilumicalc_histograms_None_297730-311481_OflLumi-13TeV-010.root","/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root","/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data22_13p6TeV/20221025/ilumicalc_histograms_None_428648-436169_OflLumi-Run3-001.root","/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/data22_13p6TeV/20221025/ilumicalc_histograms_None_430536-436169_OflLumi-Run3-001.root","/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/20190609/ilumicalc_histograms_None_276262-284484_OflLumi-13TeV-010.root","/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists/20190609/ilumicalc_histograms_None_325713-340453_OflLumi-13TeV-010.root"};
    CHECK(m_tPRWTool->setProperty("LumiCalcFiles", vLumiCalcFiles));
    // CHECK(m_tPRWTool->setProperty("DefaultChannel", "" ));
    CHECK(m_tPRWTool->initialize());
  }
  ToolHandle<CP::IPileupReweightingTool> m_tPRWToolHandle = m_tPRWTool;
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

  ToolHandle<TauAnalysisTools::ITauSelectionTool> TauSelToolHandle = TauSelTool;

  // ===========================================================================
  // TauSmearingTool
  // ===========================================================================
  TauAnalysisTools::TauSmearingTool TauSmeTool( "TauSmearingTool" );
  TauSmeTool.msg().setLevel( MSG::DEBUG );
  CHECK(TauSmeTool.setProperty("RecommendationTag","2022-prerec"));
  CHECK(TauSmeTool.setProperty("Campaign","mc21"));
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
  CHECK(TauEffCorrTool.setProperty("TauSelectionTool",TauSelToolHandle));
  CHECK(TauEffCorrTool.setProperty("RecommendationTag","2022-prerec"));
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
      CHECK(TauEffTrigTool.setProperty("TriggerName", "HLT_tau25_medium1_tracktwo" ));
      CHECK(TauEffTrigTool.setProperty("JetIDLevel", (int)JETIDRNNTIGHT ));
      CHECK(TauEffTrigTool.setProperty("PileupReweightingTool", m_tPRWToolHandle ));
      CHECK(TauEffTrigTool.setProperty("TriggerSFMeasurement", "combined"));
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

  // ===========================================================================
  // TauTruthTrackMatchingTool
  // ===========================================================================
  TauAnalysisTools::TauTruthTrackMatchingTool T3MT( "TauTruthTrackMatchingTool");
  T3MT.msg().setLevel( MSG::INFO );
  CHECK(T3MT.initialize());

  // Loop over the events:
  for( Long64_t iEntry = 0; iEntry < iEntries; ++iEntry )
  {

    //Check TauJet Container Name
    const char * m_tauJetContainerName = "TauJets";
    
    

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
      // if (xTau->pt() < 25*1000) continue;

      if ((bool)xTau->auxdata<char>("IsTruthMatched"))
      {
        if (xTruthTau->isTau())
        {
          if ((bool)xTruthTau->auxdata<char>("IsHadronicTau"))
            Info( "TauAnalysisToolsExample",
                  "Tau was matched to a truth hadronic tau, which has %i prongs and a charge of %i",
                  int(xTruthTau->auxdata<size_t>("numCharged")),
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

      auto xTruthJetLink = xTau->auxdata< ElementLink< xAOD::JetContainer > >("truthJetLink");
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
        if ((bool)xTau->auxdata<char>("IsTruthMatched") && xTruthTau->isTau() && (bool)xTruthTau->auxdata<char>("IsHadronicTau")){
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
              xTau->auxdata< double >( "TauScaleFactorReconstructionHadTau" ),
              xTau->auxdata< double >( "TauScaleFactorJetIDHadTau" ),
              xTau->auxdata< double >( "TauScaleFactorEleIDHadTau" ),
              xTau->auxdata< double >( "TauScaleFactorEleIDElectron" ));
      }

      for (auto sSystematicSet: vEfficiencyCorrectionsTriggerSystematicSet)
      {
        CHECK( TauEffTrigTool.applySystematicVariation(sSystematicSet));
        CHECK( TauEffTrigTool.applyEfficiencyScaleFactor(*xTau) );
        Info( "TauAnalysisToolsExample",
              "SystType %s: Trigger: %g",
              sSystematicSet.name().c_str(),
              xTau->auxdata< double >( "TauScaleFactorTriggerHadTau" ));
      }
      // print some info about the selected tau:
      Info( "TauAnalysisToolsExample", "Selected tau: pt = %g MeV, eta = %g, phi = %g, prong = %i, charge = %i",
            xTau->pt(), xTau->eta(), xTau->phi(), int(xTau->nTracks()), int(xTau->charge()));


      // for (size_t iTrack = 1; iTrack < xTau->nTracks(); iTrack++)
      // {
      //   const TAUTRACKPARTICLE* xTrack = xTau->track(iTrack);

      //   CHECK(T3MT.classifyTrack(*xTrack));
      //   if (xTrack->auxdata<char>("IsHadronicTrack"))
      //     Info( "TauAnalysisToolsExample", "Track is matched to a hadronic tau decay, with decay depth %i",xTrack->auxdata<int>("IsHadronicTrackDecayDepth"));
      //   else
      //     Info( "TauAnalysisToolsExample", "Track is not matched to a hadronic tau decay");
      //   Info ("TauAnalysisToolsExample", "The track decay history is: %s", (xTrack->auxdata<std::string>("DecayHistory")).c_str());
      //   Info ("TauAnalysisToolsExample", "The spurious type is: %i", xTrack->auxdata<int>("TrackType") );
      // }
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
