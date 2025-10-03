/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <string>

// ROOT include(s):
#include <TFile.h>
#include <TString.h>
#include <TTree.h>
#include <TChain.h>

// Infrastructure include(s):
#ifdef ROOTCORE
#   include "xAODRootAccess/Init.h"
#   include "xAODRootAccess/TEvent.h"
#endif // ROOTCORE
#include "AsgTools/StandaloneToolHandle.h"

// EDM include(s):
#include "xAODCore/ShallowAuxContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODCore/tools/IOStats.h"

// Tool testing include(s):
#include "BoostedJetTaggers/SmoothedTopTagger.h"

#include "AsgMessaging/MessageCheck.h"

// messaging 
ANA_MSG_HEADER(Test)
ANA_MSG_SOURCE(Test, "BoostedJetTaggers")
using namespace Test;

int main( int argc, char* argv[] ) {

  ANA_CHECK_SET_TYPE (int); // makes ANA_CHECK return ints if exiting function

  // The application's name:
  char* APP_NAME = argv[ 0 ];

  // arguments
  TString fileName = "/eos/atlas/atlascerngroupdisk/perf-jets/ReferenceFiles/mc16_13TeV.361028.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ8W.deriv.DAOD_FTAG1.e3569_s3126_r9364_r9315_p3260/DAOD_FTAG1.12133096._000074.pool.root.1";
  int  ievent=-1;
  int  nevents=-1;
  bool verbose=false;
  bool useConfigFile = true; // use config file from CVMFS for tagger (alternately setup cut functions manually)

  Info( APP_NAME, "==============================================" );
  Info( APP_NAME, "Usage: $> %s [xAOD file name]", APP_NAME );
  Info( APP_NAME, " $> %s       | Run on default file", APP_NAME );
  Info( APP_NAME, " $> %s -f X  | Run on xAOD file X", APP_NAME );
  Info( APP_NAME, " $> %s -n X  | X = number of events you want to run on", APP_NAME );
  Info( APP_NAME, " $> %s -e X  | X = specific number of the event to run on - for debugging", APP_NAME );
  Info( APP_NAME, " $> %s -v    | run in verbose mode   ", APP_NAME );
  Info( APP_NAME, " $> %s -wp   | working point to test (either 50 or 80 [signal efficiency])   ", APP_NAME );
  Info( APP_NAME, " $> %s -m    | manually setup taggers using cut function and tagger mode properties", APP_NAME );
  Info( APP_NAME, "==============================================" );

  // Check if we received a file name:
  if( argc < 2 ) {
    Info( APP_NAME, "No arguments - using default file" );
    Info( APP_NAME, "Executing on : %s", fileName.Data() );
  }

  ////////////////////////////////////////////////////
  //:::  parse the options
  ////////////////////////////////////////////////////
  std::string options;
  for( int i=0; i<argc; i++){
    options+=(argv[i]);
  }

  if(options.find("-f")!=std::string::npos){
    for( int ipos=0; ipos<argc ; ipos++ ) {
      if(std::string(argv[ipos]).compare("-f")==0){
        fileName = argv[ipos+1];
        Info( APP_NAME, "Argument (-f) : Running on file # %s", fileName.Data() );
        break;
      }
    }
  }

  // check both -event and -e in case the argument is shortened as in the description
  if(options.find("-event")!=std::string::npos || options.find("-e")!=std::string::npos) {
    for( int ipos=0; ipos<argc ; ipos++ ) {
      if(std::string(argv[ipos]).compare("-event")==0 || std::string(argv[ipos]).compare("-e")==0){
        ievent = atoi(argv[ipos+1]);
        Info( APP_NAME, "Argument (-event) : Running only on event # %i", ievent );
        break;
      }
    }
  }

  if(options.find("-n")!=std::string::npos){
    for( int ipos=0; ipos<argc ; ipos++ ) {
      if(std::string(argv[ipos]).compare("-n")==0){
        nevents = atoi(argv[ipos+1]);
        Info( APP_NAME, "Argument (-n) : Running on NEvents = %i", nevents );
        break;
      }
    }
  }

  if(options.find("-v")!=std::string::npos){
    verbose=true;
    Info( APP_NAME, "Argument (-v) : Setting verbose");
  }

  if(options.find("-m")!=std::string::npos){
    useConfigFile=false;
    Info( APP_NAME, "Argument (-m) : Setting manual tagger setup");
  }

  // default to 50% efficiency WP
  std::string taggerEfficiency = "50";
  // check if efficiency option is provided
  if (options.find("-wp") != std::string::npos) {
    for( int ipos=0; ipos<argc ; ipos++ ) {
      if(std::string(argv[ipos]).compare("-wp")==0) {
        if (std::string(argv[ipos+1]) == "50" || std::string(argv[ipos+1]) == "80") {
          taggerEfficiency = argv[ipos+1];
          if (taggerEfficiency == "50") {
            Info( APP_NAME, "Argument (-wp) : Setting is 50pc signal efficiency");
          } else if (taggerEfficiency == "80") {
            Info( APP_NAME, "Argument (-wp) : Setting is 80pc signal efficiency");
          }
        } else {
          Error( APP_NAME, "Argument (-wp) : Invalid value (should be 50 or 80), using default 50 value");
        }
      }
    }
  }


  ////////////////////////////////////////////////////
  //:::  initialize the application and get the event
  ////////////////////////////////////////////////////
  ANA_CHECK( xAOD::Init( APP_NAME ) );
  StatusCode::enableFailure();

  // Open the input file:
  TFile* ifile( TFile::Open( fileName, "READ" ) );
  if( !ifile ) Error( APP_NAME, "Cannot find file %s",fileName.Data() );

  TChain *chain = new TChain ("CollectionTree","CollectionTree");
  chain->Add(fileName);

  // Create a TEvent object:
  xAOD::TEvent event( chain, xAOD::TEvent::kAthenaAccess );
  Info( APP_NAME, "Number of events in the file: %i", static_cast< int >( event.getEntries() ) );

  // Create a transient object store. Needed for the tools.
  xAOD::TStore store;

  // Decide how many events to run over:
  Long64_t entries = event.getEntries();

  // create ROOT filename with tagger efficiency
  std::string ROOTFileName = "output_HLLHCSmoothedTopTaggers_" + taggerEfficiency + "SignalEff.root";

  // Fill a validation true with the tag return value
  TFile* outputFile = TFile::Open( ROOTFileName.c_str(), "recreate" );
  int pass, passMass, passSphericity, validJet, validKinRange;
  float sphericityCut, massCut;
  TTree* Tree = new TTree( "tree", "test_tree" );
  Tree->Branch( "pass", &pass, "pass/I" );
  Tree->Branch( "passMass", &passMass, "passMass/I" );
  Tree->Branch( "passSphericity", &passSphericity, "passSphericity/I" );
  Tree->Branch( "massCut", &massCut, "massCut/F" );
  Tree->Branch( "sphericityCut", &sphericityCut, "sphericityCut/F" );
  Tree->Branch( "validJetContent", &validJet, "validJetContent/I" );
  Tree->Branch( "validKinRange", &validKinRange, "validKinRange/I" );

  // extra tree variables for jets
  float jetPt, jetEta, jetPhi, jetM, jetE, jetSphericity;
  Tree->Branch("jetEta", &jetEta, "jetEta/F");
  Tree->Branch("jetPt", &jetPt, "jetPt/F");
  Tree->Branch("jetPhi", &jetPhi, "jetPhi/F");
  Tree->Branch("jetM", &jetM, "jetM/F");
  Tree->Branch("jetE", &jetE, "jetE/F");
  Tree->Branch("jetSphericity", &jetSphericity, "jetSphericity/F");

  ////////////////////////////////////////////
  /////////// START TOOL SPECIFIC ////////////
  ////////////////////////////////////////////

  // flags used for tagger setup
  bool useLocalCalibArea = true; // retrieve config files from local directory
  
  // variables to define different config related variables
  std::string configFile, decorationName;

  // 50% efficiency setup 
  if (taggerEfficiency == "50") {
    configFile = "HLLHCSmoothedContainedTopTagger_AntiKt10LCTopoTrimmed_MassSphericityFixedSignalEfficiency50_20220413.dat";
    decorationName = "HLLHCSmoothedTop50MassSphericity";
  }
  // 80% efficiency setup
  else if (taggerEfficiency == "80") {
    configFile = "HLLHCSmoothedContainedTopTagger_AntiKt10LCTopoTrimmed_MassSphericityFixedSignalEfficiency80_20220413.dat";
    decorationName = "HLLHCSmoothedTop80MassSphericity";
  }

  ////////////////////////////////////////////////////
  //::: Tool setup
  // setup the tool handle as per the
  // recommendation by ASG - https://twiki.cern.ch/twiki/bin/view/AtlasProtected/AthAnalysisBase#How_to_use_AnaToolHandle
  ////////////////////////////////////////////////////

  /*
  How to setup the HL-LHC tagger:

  Config file provided:
  set ConfigFile to required config location + file
  for local running set CalibArea to Local

  No config file (user specifying own selections):
  for local running set CalibArea to Local
  instead of providing ConfigFile containing information
  provide:
  TaggerMode = MassSphericity
  Var1CutFunc = mass cut function (C++ string formatted with x to indicate where mass should be inserted)
  Var2CutFunc = sphericity cut function (C++ string with same formatting as Var1CutFunc)

  */
  std::string localConfigPath = "HLLHCSmoothedTopTaggers/";

  std::cout<<"Initializing HLLHC Top Tagger"<<std::endl;
  asg::StandaloneToolHandle<SmoothedTopTagger> m_Tagger; //!
  m_Tagger.setTypeAndName("SmoothedTopTagger/MyTagger");
  if(verbose) ANA_CHECK( m_Tagger.setProperty("OutputLevel", MSG::DEBUG) );
  if (useLocalCalibArea) {ANA_CHECK( m_Tagger.setProperty("CalibArea", "Local") );} 
  else {ANA_CHECK( m_Tagger.setProperty("CalibArea", "SmoothedTopTaggers/HLLHC/July2022/") );}
  
  if (useConfigFile) { 
    if (!useLocalCalibArea) {
      ANA_CHECK( m_Tagger.setProperty( "ConfigFile", configFile) ); 
    } else {
      ANA_CHECK( m_Tagger.setProperty( "ConfigFile", localConfigPath + configFile) )
    }
  }
  else {
    // use an empty CalibArea 
    ANA_CHECK( m_Tagger.setProperty("CalibArea", "") );
    // working point dependent settings for tagger cuts
    std::vector<std::string> variableNames = {"Mass", "Sphericity"};
    std::vector<std::string> cutExpressions;
    if (taggerEfficiency == "50") {
      ANA_CHECK( m_Tagger.setProperty("Decoration", "HLLHCSmoothedTop50MassSphericity") );
      cutExpressions.push_back("(x > 2250.0) ? 163.5 : (x < 575.0) ? 121.5 : (107.08208957157846) * pow(x, 0.0) + (0.025074626862387987) * pow(x, 1.0)");
      cutExpressions.push_back("(x > 2250.0) ? 0.2665 : (x < 575.0) ? 0.2115 : (0.21589532367374106) * pow(x, 0.0) + (-4.630614969502935e-05) * pow(x, 1.0) + (7.982389622058105e-08) * pow(x, 2.0) + (-2.188773324349366e-11) * pow(x, 3.0)");
    } else if (taggerEfficiency == "80") {
      ANA_CHECK( m_Tagger.setProperty("Decoration", "HLLHCSmoothedTop80MassSphericity") );
      cutExpressions.push_back("(x > 2250.0) ? 142.5 : (x < 575.0) ? 96.5 : (80.7089552147599) * pow(x, 0.0) + (0.027462686570187158) * pow(x, 1.0)");
      cutExpressions.push_back("(x > 2250.0) ? 0.0965 : (x < 575.0) ? 0.0445 : (-0.015521973683231065) * pow(x, 0.0) + (0.0001160203413917445) * pow(x, 1.0) + (-1.7074246660926898e-08) * pow(x, 2.0) + (-5.494467663191552e-12) * pow(x, 3.0)");
    } else {
      Error( APP_NAME, "Invalid tagger efficiency working point" );
    }

    ANA_CHECK( m_Tagger.setProperty("VarCutFuncs", cutExpressions) );
    ANA_CHECK( m_Tagger.setProperty("VarCutNames", variableNames) );
  }
  ANA_CHECK( m_Tagger.retrieve() );

  ////////////////////////////////////////////////////
  // Loop over the events
  ////////////////////////////////////////////////////

  // setup the decoration names
  std::string massDecor = decorationName + "_Cut_m";
  std::string sphericityDecor = decorationName + "_Cut_Sphericity";

  std::string sphericityPassDecor = decorationName + "_PassSphericity";
  std::string massPassDecor = decorationName + "_PassMass";

  std::string validJetDecor = decorationName + "_ValidJetContent";  
  std::string validKinRangeDecor = decorationName + "_ValidKinRange";

  static const SG::ConstAccessor<float> acc_massDecor(massDecor);
  static const SG::ConstAccessor<float> acc_sphericityDecor(sphericityDecor);
  static const SG::ConstAccessor<bool> acc_sphericityPassDecor(sphericityPassDecor);
  static const SG::ConstAccessor<bool> acc_massPassDecor(massPassDecor);
  static const SG::ConstAccessor<bool> acc_validJetDecor(validJetDecor);
  static const SG::ConstAccessor<bool> acc_validKinRangeDecor(validKinRangeDecor);

  for( Long64_t entry = 0; entry < entries; ++entry ) {

    if( nevents!=-1 && entry > nevents ) break;
    // Tell the object which entry to look at:
    event.getEntry( entry );

    // Print some event information
    const xAOD::EventInfo* evtInfo = 0;
    if(event.retrieve( evtInfo, "EventInfo" ) != StatusCode::SUCCESS){
      continue;
    }
    if(ievent!=-1 && static_cast <int> (evtInfo->eventNumber())!=ievent) {
      continue;
    }

    // Get the jets
    const xAOD::JetContainer* myJets = 0;
    if( event.retrieve( myJets, "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets" ) != StatusCode::SUCCESS)
      continue ;

    // Loop over jet container
    std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > jets_shallowCopy = xAOD::shallowCopyContainer( *myJets );
    std::unique_ptr<xAOD::JetContainer> shallowJets(jets_shallowCopy.first);
    std::unique_ptr<xAOD::ShallowAuxContainer> shallowAux(jets_shallowCopy.second);
    for(const xAOD::Jet* jet : * shallowJets ){

      if(verbose) std::cout<<"Testing Top Tagger"<< std::endl;

      ANA_CHECK( m_Tagger->tag( *jet ) );

      // get tagger results and store in tree variables
      // if (res) {pass = 1;} else {pass = 0;}
      // pass = (res) ? 1 : 0; // tagger pass result (evaluates to true if all TAccept attributes are true)
      passMass = acc_massPassDecor(*jet);
      passSphericity = acc_sphericityPassDecor(*jet);
      massCut = acc_massDecor(*jet);
      sphericityCut = acc_sphericityDecor(*jet);
      jetEta = jet->eta();
      jetPhi = jet->phi();
      // store jet E, m, pT in GeV
      jetM = jet->m()*0.001;
      jetE = jet->e()*0.001;
      jetPt = jet->pt()*0.001;
      validJet = acc_validJetDecor(*jet);
      validKinRange = acc_validKinRangeDecor(*jet);
      // ints implicitly converted to bool in this approach
      pass = validJet && passMass && passSphericity && validKinRange;
      if (!(jet->getAttribute("Sphericity",jetSphericity))) {
        std::cout << "WARNING sphericity is not decorated on this jet!" << std::endl;
      }

      if (verbose) {
        std::cout<<"Total tagger result   : "<<pass<<std::endl;
        std::cout<<"Valid jet content     = "<<validJet<<std::endl;
        std::cout<<"result massPass       = "<<passMass<<std::endl;
        std::cout<<"result sphericityPass = "<<passSphericity<<std::endl;
        std::cout<<"result massCut        = "<<massCut<<std::endl;
        std::cout<<"result sphericityCut  = "<<sphericityCut<<std::endl;
        std::cout << "Jet pT (GeV) = " << jetPt << ", eta = " << jetEta << " ; mass (GeV) = " << jetM << ", Sphericity = " << jetSphericity << std::endl;
      }

      Tree->Fill();
    }

    Info( APP_NAME, "===>>>  done processing event #%i, run #%i %i events processed so far  <<<===", static_cast< int >( evtInfo->eventNumber() ), static_cast< int >( evtInfo->runNumber() ), static_cast< int >( entry + 1 ) );
  }

  ////////////////////////////////////////////
  /////////// END TOOL SPECIFIC //////////////
  ////////////////////////////////////////////

  // write the tree to the output file
  outputFile->cd();
  Tree->Write();
  outputFile->Close();

  // cleanup
  delete chain;  

  // print the branches that were used for help with smart slimming
  std::cout<<std::endl<<std::endl;
  std::cout<<"Smart Slimming Checker :"<<std::endl;
  xAOD::IOStats::instance().stats().printSmartSlimmingBranchList();
  std::cout<<std::endl<<std::endl;

  return 0;

}
