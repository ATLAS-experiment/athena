/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <regex>
#include <string>
#include <utility>
#include <vector>

// ROOT include(s):
#include <TEnv.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TError.h>
#include <TString.h>
#include <TStopwatch.h>
#include <TSystem.h>
#include "TObjArray.h"
#include "TObjString.h"

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TStore.h"
#ifdef ROOTCORE
#include "xAODRootAccess/TEvent.h"
#else
#include "POOLRootAccess/TEvent.h"
#endif // ROOTCORE

// EDM include(s):
#include "xAODEventInfo/EventInfo.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthEvent.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETAuxContainer.h"
#include "xAODBase/IParticleHelpers.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "AsgAnalysisInterfaces/IGoodRunsListSelectionTool.h"
#include "TauAnalysisTools/ITauTruthMatchingTool.h"

// Local include(s):
#include "SUSYTools/SUSYObjDef_xAOD.h"
#include "SUSYTools/SUSYCrossSection.h"

// Other includes
#include "PATInterfaces/SystematicSet.h"
#include "AsgMessaging/StatusCode.h"
#include "AsgMessaging/MessageCheck.h"
#include "PATInterfaces/CorrectionCode.h"
#include "PathResolver/PathResolver.h"
#include "AsgTools/ToolStore.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/StandaloneToolHandle.h"
#include "AsgTools/IAsgTool.h"

#include "xAODCutFlow/CutBookkeeper.h"
#include "xAODCutFlow/CutBookkeeperContainer.h"

#include "TrigDecisionTool/ChainGroup.h"

// ConstAccessors
const static SG::ConstAccessor<size_t> acc_TruthProng("TruthProng");
const static SG::ConstAccessor<int> acc_TruthCharge("TruthCharge");
const static SG::ConstAccessor<char> acc_IsTruthMatched("IsTruthMatched");

const size_t Ncuts = 10;
const char *cut_name[] =
{ "All",
  "GRL+Cleaning",
  "TileTrip",
  "Trigger",
  "Cosmic veto",
  "==1 baseline lepton",
  "==1 signal lepton",
  "lepton pT>20 GeV",
  "trig. matched lepton",
  "Njet(pT>50 GeV)>=2"
};

const unsigned int nSel=8;
enum sel {
  signallep,
  baseline,
  bad,
  cosmic,
  goodpt,
  btagged,
  trgmatch,
  passOR
};

namespace asg{ANA_MSG_HEADER(msgSTT) ANA_MSG_SOURCE(msgSTT,"SUSYToolsTester")}

//====================================================================================================
std::vector<std::string> getTokens(TString line, const TString& delim);
std::map<std::string, std::string> getFileContainers(std::unique_ptr<TFile> &);

//====================================================================================================
int main( int argc, char* argv[] ) {

  using namespace asg::msgSTT;
  ANA_CHECK_SET_TYPE (int);
  setMsgLevel(MSG::INFO);

  CP::CorrectionCode::enableFailure();

  // The application's name:
  const char* APP_NAME = argv[ 0 ];

  // Check if we received a file name:
  if ( argc < 2 ) {
    ANA_MSG_ERROR( "No file name received!" );
    ANA_MSG_ERROR( "  Usage: " << APP_NAME << " [xAOD file name] [maxEvents] [isData=0/1 isAtlfast=0/1] [NoSyst=0/1] [Debug=0/1/2] [ConfigFile=<cfile.conf>] [PRWFile=<prwfile.root>] [autoconfigPRW=0/1] [commonPRWFile=0/1]" );
    return 1;
  }


  /// READ CONFIG ------------

  int autoconfigPRW = 1;
  int commonPRWFile = 1;
  int isData = -1;
  int isAtlfast = -1;
  int NoSyst = 1;
  int debug = 1;
  bool isPHYSLite = false;
  bool hasLRT = false;
  Long64_t entries=-1;

  // Open the input file:
  TString fileName = argv[1];
  ANA_MSG_INFO( "Opening file: " << fileName );
  std::unique_ptr< TFile > ifile( TFile::Open( fileName, "READ" ) );
  ANA_CHECK( ifile.get() );

  bool isRun3 = false;
  if ((fileName.Contains("mc23") || fileName.Contains("mc21") || fileName.Contains("data2")) && fileName.Contains("13p6TeV")) isRun3 = true;
  std::string config_file = (PathResolverFindCalibFile("SUSYTools/SUSYTools_Default.conf")).c_str();
  if (fileName.Contains("LITE")) config_file = (PathResolverFindCalibFile("SUSYTools/SUSYTools_Default_LITE.conf")).c_str();
  if (isRun3) {
    config_file = (PathResolverFindCalibFile("SUSYTools/SUSYTools_Default_Run3.conf")).c_str();
    if (fileName.Contains("LITE")) config_file = (PathResolverFindCalibFile("SUSYTools/SUSYTools_Default_Run3_LITE.conf")).c_str();
    ANA_MSG_INFO("Configuring for Run 3");
  }
  std::string prw_file = "DUMMY";
  std::string ilumicalc_file = "DUMMY";

  for (int i = 1 ; i < argc ; i++) {
    const char* key = strtok(argv[i], "=") ;
    const char* val = strtok(0, " ") ;

    ANA_MSG_INFO( "===== Processing key " << TString(key) << " with value " << TString(val) );
    if (strcmp(key, "autoconfigPRW") == 0) autoconfigPRW = atoi(val);
    if (strcmp(key, "commonPRWFile") == 0) commonPRWFile = atoi(val);
    if (strcmp(key, "isData") == 0) isData = atoi(val);
    if (strcmp(key, "isAtlfast") == 0) isAtlfast = atoi(val);
    if (strcmp(key, "NoSyst") == 0) NoSyst = atoi(val);
    if (strcmp(key, "Debug") == 0) debug = atoi(val);
    if (strcmp(key, "ConfigFile") == 0) config_file = (PathResolverFindCalibFile(val)).c_str();
    if (strcmp(key, "PRWFile") == 0) prw_file = std::string(val);
    if (strcmp(key, "ilumicalcFile") == 0) ilumicalc_file = std::string(val);
    if (strcmp(key, "maxEvents") == 0) entries = atoi(val);

  }

  if (debug>0) setMsgLevel((MSG::Level)(3-debug)); // NIL=0, VERBOSE, DEBUG, INFO, WARNING, ERROR, FATAL, ALWAYS

  if (isData < 0 || isAtlfast < 0) {
    ANA_MSG_ERROR( "One of the flags isData or isAtlfast was not set! Must provide isData or isAtlfast." );
    ANA_MSG_ERROR( "  Usage: " << APP_NAME << " [xAOD file name] [maxEvents] [isData=0/1 isAtlfast=0/1] [NoSyst=0/1] [Debug=0/1/2] [ConfigFile=<cfile.conf>] [PRWFile=<prwfile.root>] [autoconfigPRW=0/1] [commonPRWFile=0/1]");
    return 10;
  }
  ST::ISUSYObjDef_xAODTool::DataSource datasource = (isData ? ST::ISUSYObjDef_xAODTool::Data : (isAtlfast ? ST::ISUSYObjDef_xAODTool::AtlfastII : ST::ISUSYObjDef_xAODTool::FullSim));


  // Open file and check available containers
  std::map<std::string, std::string> containers = getFileContainers(ifile);
  bool hasTrkJets(false), hasFatJets(false);
  for (auto& x : containers) {
    if (x.first.find("AntiKtVR30Rmax4Rmin02TrackJets")!=std::string::npos)           hasTrkJets = true;
    if (x.first.find("AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets")!=std::string::npos) hasFatJets = true;
    if (x.first.find("AnalysisElectrons")!=std::string::npos)                        isPHYSLite = true;
    if (isPHYSLite && x.first.find("AnalysisLargeRJets")!=std::string::npos)         hasFatJets = false;
    if (x.first.find("InDetLargeD0TrackParticles") != std::string::npos)             hasLRT     = true;
  }
  if (debug>0) {
    ANA_MSG_INFO("Checking file contents (containers):");
    for (auto& x : containers) ANA_MSG_INFO("  - found " << x.first.c_str() << " (" << x.second.c_str() << ")");
    ANA_MSG_INFO("hasTrkJets: " << (hasTrkJets?"true":"false"));
    ANA_MSG_INFO("hasFatJets: " << (hasFatJets?"true":"false"));
    ANA_MSG_INFO("isPHYSLite: " << (isPHYSLite?"true":"false"));
    ANA_MSG_INFO("hasLRT    : " << (hasLRT?"true":"false"));
  }

  // Create a TEvent object:

  xAOD::TEvent event( xAOD::TEvent::kClassAccess );
  ANA_CHECK( event.readFrom( ifile.get() ) );
  ANA_MSG_INFO( "Number of events in the file: " << static_cast< int >( event.getEntries() ) );

  xAOD::TStore store;

  // Open up an output file.  Not needed for this example.  If you wish to enable
  // this, please don't forget to add the call to fill towards the end of the code!
  //std::unique_ptr< TFile > ofile( TFile::Open( "out.root", "RECREATE" ) );
  //ANA_CHECK( event.writeTo( ofile.get() ) );

  // If we haven't set the number of events, then run over the whole tree
  if (entries<0){
    entries = event.getEntries();
  }

  // GRL tool
  asg::StandaloneToolHandle<IGoodRunsListSelectionTool> m_grl;
  if (isData) {
    m_grl.setTypeAndName("GoodRunsListSelectionTool/grl");
    std::vector<std::string> myGRLs;

    myGRLs.push_back(PathResolverFindCalibFile("GoodRunsLists/data15_13TeV/20170619/physics_25ns_21.0.19.xml"));
    myGRLs.push_back(PathResolverFindCalibFile("GoodRunsLists/data16_13TeV/20180129/physics_25ns_21.0.19.xml"));
    myGRLs.push_back(PathResolverFindCalibFile("GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.xml"));
    myGRLs.push_back(PathResolverFindCalibFile("GoodRunsLists/data18_13TeV/20190219/physics_25ns_Triggerno17e33prim.xml"));
    myGRLs.push_back(PathResolverFindCalibFile("GoodRunsLists/data22_13p6TeV/20230116/data22_13p6TeV.periodAllYear_DetStatus-v109-pro28-04_MERGED_PHYS_StandardGRL_All_Good_25ns.xml"));
    myGRLs.push_back(PathResolverFindCalibFile("GoodRunsLists/data23_13p6TeV/20230828/data23_13p6TeV.periodAllYear_DetStatus-v110-pro31-06_MERGED_PHYS_StandardGRL_All_Good_25ns.xml"));
    //myGRLs.push_back(PathResolverFindCalibFile("GoodRunsLists/data22_13p6TeV/20230116/data22_13p6TeV.periodAllYear_DetStatus-v109-pro28-04_MERGED_PHYS_StandardGRL_All_Good_25ns_ignore_TRIGMUO_TRIGLAR.xml"));
    ANA_CHECK( m_grl.setProperty("GoodRunsListVec", myGRLs) );
    ANA_CHECK( m_grl.setProperty("PassThrough", false) );
    ANA_CHECK( m_grl.retrieve() );

    ANA_MSG_INFO( "GRL tool retrieve & initialized... " );
  }

  //xsec DB
  SUSY::CrossSectionDB *my_XsecDB = nullptr;
  if (!isData){
    ANA_MSG_INFO(fileName);
    if      (isRun3 && fileName.Contains("mc21")) { my_XsecDB = new SUSY::CrossSectionDB(PathResolverFindCalibFile("dev/PMGTools/PMGxsecDB_mc21.txt"));}
    else if (isRun3 && fileName.Contains("mc23")) { my_XsecDB = new SUSY::CrossSectionDB(PathResolverFindCalibFile("dev/PMGTools/PMGxsecDB_mc23.txt"));}
    else {                                          my_XsecDB = new SUSY::CrossSectionDB(PathResolverFindCalibFile("dev/PMGTools/PMGxsecDB_mc16.txt"));}
    ANA_MSG_INFO( "xsec DB initialized" );
  }

  // Create the tool(s) to test:
  ST::SUSYObjDef_xAOD objTool("SUSYObjDef_xAOD");

  ANA_MSG_INFO(" ABOUT TO INITIALIZE SUSYTOOLS " );

  ///////////////////////////////////////////////////////////////////////////////////////////
  // Configure the SUSYObjDef instance

  ANA_CHECK( objTool.setProperty("DataSource", datasource) ) ;
  if(!config_file.empty())
    ANA_CHECK( objTool.setProperty("ConfigFile", config_file) );

  ///////////////////////////////////////////////////////////////////////////////////////////
  ////                                                                                   ////
  ////       ****     THESE FILES ARE MEANT FOR EXAMPLES OF USAGE ONLY        ****       ////
  ////       ****        AND SHOULD NOT BE USED FOR SERIOUS ANALYSIS          ****       ////
  ////                                                                                   ////
  ///////////////////////////////////////////////////////////////////////////////////////////

  std::vector<std::string> prw_conf;
  if (!isData) {
    if ( autoconfigPRW == 1 ) {
      ANA_CHECK( objTool.setBoolProperty("AutoconfigurePRWTool", true) );
      if ( commonPRWFile == 1 )
        ANA_CHECK( objTool.setBoolProperty("PRWUseCommonMCFiles", true) );
    } else {
      if (prw_file == "DUMMY") {
        prw_conf.push_back("dev/SUSYTools/merged_prw_mc16a_latest.root");
      } else {
        prw_conf = getTokens(prw_file,",");
      }
      ANA_CHECK( objTool.setProperty("PRWConfigFiles", prw_conf) );
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ////
  //// Pick up only relevant ilumicalc files for each MC16 campaign. 
  //// See https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BackgroundStudies
  ////
  //// For mc16a:
  //// GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root
  //// GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root
  ////
  //// For mc16d:
  //// GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root
  ////
  //// For mc16e:
  //// GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root
  ////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////


  std::vector<std::string> prw_lumicalc;
  if (ilumicalc_file == "DUMMY") {
    ANA_CHECK( objTool.setProperty( "mcCampaign", "mc16e" ) );
    prw_lumicalc.push_back(PathResolverFindCalibFile("GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root"));
  } else {
    prw_lumicalc = getTokens(ilumicalc_file,",");
  }
  ANA_CHECK( objTool.setProperty("PRWLumiCalcFiles", prw_lumicalc) );

  // ============================================================================================
  // Guess shower type for btagging MC/MC SFs
  if(!isData){
    int ishower = objTool.getMCShowerType("PhPy8EG","GN2v01"); //get your sample name here!
    ANA_CHECK( objTool.setProperty("ShowerType", (int)ishower) );
  }

  //// Don't touch these unless you know what you are after!!! FOR EFF TESTS ONLY
  // ANA_CHECK( objTool.setBoolProperty("EleForceNoId", false) );
  // ANA_CHECK( objTool.setBoolProperty("MuForceNoId", false) );
  ////

  ANA_CHECK(objTool.setProperty("OutputLevel", msg().level() )); 
  ANA_CHECK(objTool.setBoolProperty("DebugMode", (debug>1))); // debug=0:INFO,1:DEBUG->0, debug=2:VERBOSE->1

  if ( objTool.initialize() != StatusCode::SUCCESS) {
    ANA_MSG_ERROR( "Cannot initialize SUSYObjDef_xAOD, exiting." );
    return 1;
  } else {
    ANA_MSG_INFO( "SUSYObjDef_xAOD initialized..." );
  }

  ANA_MSG_INFO( "Initialized SUSYTools" );

  // ============================================================================================
  // Configuration
  TEnv rEnv;
  if ( rEnv.ReadFile(config_file.c_str(), kEnvAll) != 0 ) {
    ANA_MSG_ERROR( "Cannot open config file, exiting.");
    return 1;
  }
  ANA_MSG_INFO( "Config file opened" );

  std::map<std::string,std::string> configDict = {};
  configDict["Jet.LargeRcollection"] = rEnv.GetValue("Jet.LargeRcollection", "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets" );
  configDict["TrackJet.Collection"]  = rEnv.GetValue("TrackJet.Collection", "AntiKtVR30Rmax4Rmin02TrackJets" );
  configDict["Jet.WtaggerConfig"]    = rEnv.GetValue("Jet.WtaggerConfig", "None");
  configDict["Jet.ZtaggerConfig"]    = rEnv.GetValue("Jet.ZtaggerConfig", "None");
  configDict["Jet.ToptaggerConfig"]  = rEnv.GetValue("Jet.ToptaggerConfig", "None");
  // Trim comments and extra spaces from config entries
  std::regex comment("#.*$");
  std::regex trimspaces("^ +| +$|( ) +");
  for (auto& keyval : configDict) {
     configDict[keyval.first] = regex_replace(configDict[keyval.first], comment, "");
     configDict[keyval.first] = regex_replace(configDict[keyval.first], trimspaces, "$1");
     ANA_MSG_DEBUG("config " << keyval.first << " : " << configDict[keyval.first]);
  }
  std::string FatJetCollection = configDict["Jet.LargeRcollection"];
  std::string TrkJetCollection = configDict["TrackJet.Collection"];
  if (configDict["TrackJet.Collection"].find("None")!=std::string::npos) TrkJetCollection = "";

  std::map<std::string,bool> slices = {};
  slices["ele"]  = bool(rEnv.GetValue("Slices.Ele", true));
  slices["pho"]  = bool(rEnv.GetValue("Slices.Pho", true));
  slices["mu"]   = bool(rEnv.GetValue("Slices.Mu", true));
  slices["tau"]  = bool(rEnv.GetValue("Slices.Tau", true));
  slices["jet"]  = bool(rEnv.GetValue("Slices.Jet", true));
  slices["bjet"] = bool(rEnv.GetValue("Slices.BJet", true));
  slices["fjet"] = bool(rEnv.GetValue("Slices.FJet", true));
  slices["tjet"] = bool(rEnv.GetValue("Slices.TJet", true));
  slices["met"]  = bool(rEnv.GetValue("Slices.MET", true));
  for (auto& x : slices) { ANA_MSG_DEBUG( "Slice " << x.first << ": " << ((x.second)?"true":"false")); }

  // ============================================================================================
  ToolHandle<TauAnalysisTools::ITauTruthMatchingTool> T2MT = 0;
  if (slices["tau"] && !isData) {
    // borrow the T2MT from SUSYTools
    T2MT = asg::ToolStore::get<TauAnalysisTools::ITauTruthMatchingTool>("ToolSvc.TauTruthMatch");
    ANA_CHECK(T2MT.retrieve());
  }

  // ============================================================================================
  // Triggers from https://twiki.cern.ch/twiki/bin/view/Atlas/LowestUnprescaled
  std::vector<std::string> el_triggers,mu_triggers,ph_triggers,tau_triggers,emu_triggers;
  if (isRun3){
    el_triggers = {"HLT_e26_lhtight_ivarloose_L1EM22VHI","HLT_e26_lhtight_ivarloose_L1eEM26M","HLT_e60_lhmedium_L1EM22VHI", "HLT_e140_lhloose_L1EM22VHI"};//,"HLT_e300_etcut_L1EM22VHI"
    mu_triggers = {"HLT_mu24_ivarmedium_L1MU14FCH","HLT_mu50_L1MU14FCH","HLT_mu60_0eta105_msonly_L1MU14FCH","HLT_mu60_L1MU14FCH","HLT_mu80_msonly_3layersEC_L1MU14FCH"};
    ph_triggers = {"HLT_g140_loose_L1EM22VHI","HLT_g300_etcut_L1EM22VHI"};
    tau_triggers = {"HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100", "HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1TAU25IM_2TAU20IM_2J25_3J20"};
    emu_triggers = {"HLT_2e12_lhloose_mu10_L12EM8VH_MU8F", "HLT_e12_lhloose_2mu10_L12MU8F", "HLT_e17_lhloose_mu14_L1EM15VH_MU8F", "HLT_e7_lhmedium_mu24_L1MU14FCH"};
  }
  else
  {
    el_triggers = {"HLT_e24_lhmedium_L1EM20VH","HLT_e26_lhtight_nod0_ivarloose", "HLT_e60_lhmedium_nod0", "HLT_e160_lhloose_nod0"};
    mu_triggers = {"HLT_mu26_ivarmedium","HLT_mu50","HLT_mu20_mu8noL1"};
    ph_triggers = {"HLT_g120_loose"};
    tau_triggers = {"HLT_tau125_medium1_tracktwo", "HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA"};
    emu_triggers = {"HLT_2e12_lhloose_nod0_mu10", "HLT_e12_lhloose_nod0_2mu10", "HLT_e17_lhloose_nod0_mu14", "HLT_e7_lhmedium_nod0_mu24"};
  }
  // ============================================================================================
  // Systematics and counts
  std::vector<ST::SystInfo> systInfoList;
  if (NoSyst) {
    ST::SystInfo infodef;
    infodef.affectsKinematics = false;
    infodef.affectsWeights = false;
    infodef.affectsType = ST::Unknown;
    systInfoList.push_back(infodef);
  } else {
    systInfoList = objTool.getSystInfoList();
  }

  size_t isys = 0;
  const size_t Nsyst = systInfoList.size();
  std::vector<std::vector<int> > elcuts;
  std::vector<std::vector<int> > mucuts;
  for (isys = 0; isys < Nsyst; ++isys) {
    std::vector<int> elcutsCurrentSyst;
    std::vector<int> mucutsCurrentSyst;
    for (size_t icut = 0; icut < Ncuts; ++icut) {
      elcutsCurrentSyst.push_back(0);
      mucutsCurrentSyst.push_back(0);
    }
    elcuts.push_back(elcutsCurrentSyst);
    mucuts.push_back(elcutsCurrentSyst);
  }

  if (objTool.resetSystematics() != StatusCode::SUCCESS) {
    Error(APP_NAME, "Cannot reset SUSYTools systematics" );
    exit(-2);
  }

  // ===============================================================================================
  // Event Loop
  TStopwatch m_clock0;
  m_clock0.Start();
  TStopwatch m_clock1;
  m_clock1.Start();
  TStopwatch m_clock2;

  int period = (debug>0) ? 1 : 100;
  std::string cbkname, stream, ostream, kernel;

  // Loop over the events:
  for ( Long64_t entry = 0; entry < entries; ++entry ) {

    if (entry == 1) {
      m_clock1.Stop();
      m_clock2.Start();
      // CALLGRIND_TOGGLE_COLLECT;
      // ProfilerStart("gperftools.profile.out");
    }


    // Tell the object which entry to look at:
    event.getEntry( entry );

    if(entry == 0){
      // Read the CutBookkeeper container
      const xAOD::CutBookkeeperContainer* completeCBC = nullptr;
      if (!event.retrieveMetaInput(completeCBC, "CutBookkeepers").isSuccess()) {
        ANA_MSG_ERROR("Failed to retrieve CutBookkeepers from MetaData, exiting.");
        return 1;
      }

      // Let's find the right CBK (latest with StreamAOD input before derivations)
      const xAOD::CutBookkeeper* allEventsCBK = nullptr;
      int maxcycle=-1;
      for ( const auto *cbk : *completeCBC ) {
        cbkname = cbk->name();
        stream = cbk->inputStream();
        ostream = (cbk->outputStreams().size() ? cbk->outputStreams()[0] : "");
        ANA_MSG_INFO("== cbkname: " << cbkname << ", description: " << cbk->description() << ", stream: " << stream);
        ANA_MSG_INFO("   inputStream: " << cbk->inputStream()  << ", outputStreams: " << ostream);
        ANA_MSG_INFO("   cycle: " << cbk->cycle() << ",  allEvents = " << cbk->nAcceptedEvents());
        if (cbkname.find("PHYSVAL")!=std::string::npos) kernel="PHYSVAL";
        else if (cbkname.find("PHYSLITE")!=std::string::npos) kernel="PHYSLITE";
        else if (cbkname.find("PHYS")!=std::string::npos) kernel="PHYS";
        else if (stream.find("StreamAOD")!=std::string::npos) kernel=stream.replace(stream.find("Stream"),6,"");
        else kernel="other";
        if (cbkname.find("AllExecutedEvents")!=std::string::npos && ostream.find("StreamAOD")!=std::string::npos && cbk->cycle() > maxcycle) {
           maxcycle = cbk->cycle();
           allEventsCBK = cbk;
        }
      }

      if (allEventsCBK) {
        uint64_t nEventsProcessed  = allEventsCBK->nAcceptedEvents();
        double sumOfWeights        = allEventsCBK->sumOfEventWeights();
        double sumOfWeightsSquared = allEventsCBK->sumOfEventWeightsSquared();
        ANA_MSG_INFO( "CutBookkeepers Accepted " << nEventsProcessed << " SumWei " << sumOfWeights << " sumWei2 " << sumOfWeightsSquared);
      } else { ANA_MSG_INFO( "No relevent CutBookKeepers found" ); }

      ANA_MSG_INFO("Found kernel: " << kernel);
      ANA_MSG_INFO("Found stream: " << stream);

      // No special jets when running on PHYSLITE
      if (isPHYSLite) {
         hasTrkJets = false;
      }
    }

    // Only need to PRW if we aren't running on PHYSLITE
    if (!isPHYSLite) {
      ANA_CHECK( objTool.ApplyPRWTool());
    }

    // Only need LRT uncertainty tool if there are LRT tracks!
    if (hasLRT) {
      ANA_MSG_DEBUG( "Applying LRT uncertainty decorations" );
      ANA_CHECK( objTool.ApplyLRTUncertainty());
    }

    // ============================================================================================
    // Print some event information for fun:
    const xAOD::EventInfo* ei = nullptr;
    ANA_CHECK( event.retrieve( ei, "EventInfo" ) );

    if (entry==0 || entry % period == 999) {
      ANA_MSG_INFO( "===>>>  start processing event #, " << static_cast< int >( ei->eventNumber() ) << 
                    "run #" << static_cast< int >( ei->runNumber() ) << " " << static_cast< int >( entry ) << " events processed so far  <<<===");
    }

    if (!isData && entry % period == 0) {
      float xsectTimesEff =  my_XsecDB->xsectTimesEff(ei->mcChannelNumber());
      ANA_MSG_INFO( "===>>>  process cross section " << xsectTimesEff );
    }

    bool eventPassesGRL(true);
    bool eventPassesCleaning(true);
    bool eventPassesTileTrip(true); // move to xAOD tool!
    bool eventPassesTrigger(true);  // coming soon!

    if (!isData){
      static const SG::ConstAccessor<float> pileupWeightAcc("PileupWeight_NOSYS");
      float PRW_weight = isPHYSLite? pileupWeightAcc(*ei): objTool.GetPileupWeight();
      ANA_MSG_DEBUG( "PRW Weight = " << PRW_weight);
    }

    if (isData) {
      eventPassesGRL = m_grl->passRunLB(ei->runNumber(), ei->lumiBlock());

      eventPassesCleaning = !((ei->errorState(xAOD::EventInfo::LAr)  == xAOD::EventInfo::Error ) ||
                              (ei->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error ) ||
                              (ei->errorState(xAOD::EventInfo::SCT)  == xAOD::EventInfo::Error ) ||
                              (ei->isEventFlagBitSet(xAOD::EventInfo::Core, 18) ));
    }
    else {
      // Check SUSY Proc. ID for signal MC (only for first event for now!)
      if(entry<5){
        int pdgid1 = 0;
        int pdgid2 = 0;
   
        if( objTool.FindSusyHP(pdgid1, pdgid2) != StatusCode::SUCCESS ){
          ANA_MSG_WARNING( "SUSY Proc finding failed. Normal for non-SUSY samples." );
        }
   
        if( pdgid1!=0 && pdgid2!=0){ //(just to avoid warnings)
          // --- Deprecated usage of procID
          ANA_MSG_INFO( "--- SIGNAL ID1     : " << pdgid1 );
          ANA_MSG_INFO( "    SIGNAL ID2     : " << pdgid2 );
          ANA_MSG_INFO( "--- XSECTION DETAILS" );
        }
        ANA_MSG_INFO( "    Xsec (high order)    " << my_XsecDB->xsectTimesEff(ei->mcChannelNumber(),0) );
        ANA_MSG_INFO( "    kfactor (high order) " << my_XsecDB->kfactor(ei->mcChannelNumber(),0) );
        ANA_MSG_INFO( "    filter efficiency    " << my_XsecDB->efficiency(ei->mcChannelNumber(),0) );
      }
    }


    //Check PV in the event
    if (objTool.GetPrimVtx() == nullptr) {
      Warning(APP_NAME , "No PV found for this event! Skipping...");
      store.clear();
      continue;
    }


    // ====================================================================================================
    // Object containers - nominal
    xAOD::ElectronContainer* electrons_nominal(nullptr);
    xAOD::ShallowAuxContainer* electrons_nominal_aux(nullptr);
    xAOD::PhotonContainer* photons_nominal(nullptr);
    xAOD::ShallowAuxContainer* photons_nominal_aux(nullptr);
    xAOD::MuonContainer* muons_nominal(nullptr);
    xAOD::ShallowAuxContainer* muons_nominal_aux(nullptr);
    xAOD::JetContainer* jets_nominal(nullptr);
    xAOD::ShallowAuxContainer* jets_nominal_aux(nullptr);
    xAOD::JetContainer* trkjets_nominal(nullptr);
    xAOD::ShallowAuxContainer* trkjets_nominal_aux(nullptr);
    xAOD::JetContainer* fatjets_nominal(nullptr);
    xAOD::ShallowAuxContainer* fatjets_nominal_aux(nullptr);
    xAOD::TauJetContainer* taus_nominal(nullptr);
    xAOD::ShallowAuxContainer* taus_nominal_aux(nullptr);
    xAOD::MissingETContainer* metcst_nominal = new xAOD::MissingETContainer;
    xAOD::MissingETAuxContainer* metcst_nominal_aux = new xAOD::MissingETAuxContainer;
    xAOD::MissingETContainer* mettst_nominal = new xAOD::MissingETContainer;
    xAOD::MissingETAuxContainer* mettst_nominal_aux = new xAOD::MissingETAuxContainer;

    // ====================================================================================================
    // Get the nominal object containers from the event
    // Electrons
    if (slices["ele"]) {
      ANA_MSG_DEBUG( "Nominal electron step" );
      ANA_CHECK( objTool.GetElectrons(electrons_nominal, electrons_nominal_aux, true, isPHYSLite?"AnalysisElectrons":"Electrons") );
      ANA_MSG_DEBUG( electrons_nominal->size() << " electrons");

      for (const auto& electron : *electrons_nominal){
        if (debug>0 && entry<10) {
          bool accepted=false;
          bool idok = electron->passSelection(accepted, "passBaseID");
          if(idok) ANA_MSG_DEBUG("Electron Baseline ID Decision : " << accepted );
          else     ANA_MSG_DEBUG("Electron Baseline ID Decision not available " );
        }
      }
    }

    // Photons
    if( slices["pho"] && stream.find("SUSY12")==std::string::npos) {
      ANA_MSG_DEBUG( "Nominal photon step" );
      ANA_CHECK( objTool.GetPhotons(photons_nominal,photons_nominal_aux, true, isPHYSLite?"AnalysisPhotons":"Photons") );
      ANA_MSG_DEBUG( photons_nominal->size() << " photons");
    }

    // Muons
    if (slices["mu"]) {
      ANA_MSG_DEBUG( "Nominal muon step" );
      ANA_CHECK( objTool.GetMuons(muons_nominal, muons_nominal_aux, true, isPHYSLite?"AnalysisMuons":"Muons") );
      ANA_MSG_DEBUG( muons_nominal->size() << " muons");

      // check HighPt muons (if required)
      for (const auto& muon : *muons_nominal){
        if (debug>0 && entry<10){
          ANA_MSG_DEBUG("--------------------------------------" );
          ANA_MSG_DEBUG("Muon pt = " << muon->pt()*0.001 << " , "
			<< "baseline = " << (int)ST::acc_baseline(*muon) << " ,"
			<< "bad = " << (int)ST::acc_bad(*muon) << " ,"
			<< "IsHighPt(deco) = " << (int)ST::acc_passedHighPtCuts(*muon) << " , "
			<< "IsHighPt(only) = " << (int)objTool.IsHighPtMuon(*muon) );
        }
      }
    }

    // Jets
    if (slices["jet"]) {
      ANA_MSG_DEBUG( "Nominal jet step" );
      ANA_CHECK( objTool.GetJets(jets_nominal, jets_nominal_aux, true, isPHYSLite?"AnalysisJets":"") ); 
      ANA_MSG_DEBUG( jets_nominal->size() << " jets");
    }

    // TrackJets
    const xAOD::JetContainer* TJC = nullptr;
    if (slices["tjet"] && hasTrkJets) {
      ANA_MSG_DEBUG( "Nominal track jet step" );
      if( event.retrieve(TJC, TrkJetCollection).isSuccess() ){
        ANA_CHECK(objTool.GetTrackJets(trkjets_nominal, trkjets_nominal_aux));
        ANA_MSG_DEBUG( trkjets_nominal->size() << " track jets");
      } else {
        ANA_MSG_ERROR("TrackJet collection " << TrkJetCollection.c_str() << " not available in input file. Please check!");
        return 1;
      }
    }

    // FatJets
    const xAOD::JetContainer* FJC = nullptr;
    if(slices["fjet"] && hasFatJets) { // stream.Contains("SUSY10") || stream.Contains("PHYSVAL")){
      ANA_MSG_DEBUG( "Nominal fat jet step, collection: " <<  FatJetCollection );
      if( event.retrieve(FJC, FatJetCollection).isSuccess() ){
        ANA_CHECK( objTool.GetFatJets(fatjets_nominal, fatjets_nominal_aux, true, "", true) );
        ANA_MSG_DEBUG( fatjets_nominal->size() << " large R jets");
        if (debug>0 && entry < 10) {
          for (const auto& fatjet : *fatjets_nominal) {
            ANA_MSG_INFO( " FatJet pt = " << fatjet->pt()*0.001 << ", Wtag = " << ST::acc_wtagged(*fatjet) << ", Ztag = " << ST::acc_ztagged(*fatjet) );
          }
        }
      } else {
        ANA_MSG_ERROR("LargeR jet collection " << FatJetCollection.c_str() << " not available in input file. Please check!");
        return 1;
      }
    }

    // Taus
    if(slices["tau"]) {
      ANA_MSG_DEBUG( "Nominal tau step" );
      ANA_CHECK( objTool.GetTaus(taus_nominal,taus_nominal_aux, true, isPHYSLite?"AnalysisTauJets":"TauJets") );
      ANA_MSG_DEBUG( taus_nominal->size() << " taus");
    }
    
    // MET Trigger
    bool passMETtrig = objTool.IsMETTrigPassed();
    ANA_MSG_DEBUG( "Pass MET trigger " << passMETtrig);

    // MET
    metcst_nominal->setStore(metcst_nominal_aux);
    metcst_nominal->reserve(10);
    double metsig_cst (0.);
    mettst_nominal->setStore(mettst_nominal_aux);
    mettst_nominal->reserve(10);
    double metsig_tst (0.);


    // ============================================================================================
    // Set up the event weights
    // Base should include all weights that do not depend on individual objects
    double base_event_weight(1.);
    if (!isData){
      base_event_weight *= ei->mcEventWeight();
      //Check for the new Sherpa 2.2 Reweighting function
      if(debug>0 && entry < 5){
        float RW_sh22 = objTool.getSherpaVjetsNjetsWeight("AntiKt4TruthWZDressedJets");
        Info(APP_NAME , "--- SHERPA2.2 REWEIGHTING : %f", RW_sh22);
      }
    }

    // ============================================================================================
    // Nominal weights
    // Additionally define a nominal weight for each object type
    double elecSF_nominal(1.);
    double muonSF_nominal(1.);
    //double tauSF_nominal(1.);
    double btagSF_nominal(1.);
    double btagSF_trkJet_nominal(1.);

    // ====================
    bool isNominal(true);
    isys = 0;
    // Now loop over all the systematic variations
    for (const auto& sysInfo : systInfoList) {
      const CP::SystematicSet& sys = sysInfo.systset;
      ANA_MSG_DEBUG(">>>> Working on variation: \"" <<(sys.name()).c_str() << "\" <<<<<<" );

      size_t icut = 0;
      // log all events
      elcuts[isys][icut] += 1;
      mucuts[isys][icut] += 1;
      ++icut;

      // does the event pass the GRL + Cleaning?
      if (!(eventPassesGRL && eventPassesCleaning)) {++isys; continue;}
      elcuts[isys][icut] += 1;
      mucuts[isys][icut] += 1;
      ++icut;

      // Apply TileTrip
      if (!eventPassesTileTrip) {++isys; continue;}
      elcuts[isys][icut] += 1;
      mucuts[isys][icut] += 1;
      ++icut;

      if (debug>0 && entry==0) {
        // Testing trigger
        std::vector<std::string> trigItem = {"HLT_e26_lhtight_nod0_ivarloose","HLT_mu26_ivarmedium","HLT_mu50","HLT_xe100","HLT_noalg_.*"}; // Trigger for Run 2
        if (isRun3) trigItem = {"HLT_e26_lhtight_ivarloose_L1EM22VHI","HLT_e26_lhtight_ivarloose_L1eEM26M","HLT_mu24_ivarmedium_L1MU14FCH","HLT_mu50_L1MU14FCH","HLT_xe100","HLT_noalg_.*"}; // Trigger for Run 3
        for (int it = 0; it < (int) trigItem.size(); it++) {
          bool passed = objTool.IsTrigPassed(trigItem[it]);
          float prescale = objTool.GetTrigPrescale(trigItem[it]);
          ANA_MSG_DEBUG( "Pass " << trigItem[it].c_str() << " trigger? " << (int)passed << ", prescale " << prescale );
          //
          // example of more sophisticated trigger access
          const Trig::ChainGroup* cg = objTool.GetTrigChainGroup(trigItem[it]);
          bool cg_passed = cg->isPassed();
          float cg_prescale = cg->getPrescale();
          ANA_MSG_DEBUG( "ChainGroup " << trigItem[it].c_str() << ": passing trigger? " << (int)cg_passed << ", prescale " << cg_prescale );
          for (const auto& cg_trig : cg->getListOfTriggers()) {
            ANA_MSG_DEBUG("\t\t includes trigger " << cg_trig.c_str() );
          }
        }
      }

      // Trigger (coming soon...)
      if (!eventPassesTrigger)  {++isys; continue;}
      elcuts[isys][icut] += 1;
      mucuts[isys][icut] += 1;
      ++icut;

      // ====================================================================================================
      // Object containers - systematics

      // Generic pointers for either nominal or systematics copy
      xAOD::ElectronContainer* electrons(electrons_nominal);
      xAOD::PhotonContainer* photons(photons_nominal);
      xAOD::MuonContainer* muons(muons_nominal);
      xAOD::JetContainer* jets(jets_nominal);
      xAOD::JetContainer* trkjets(trkjets_nominal);
      xAOD::TauJetContainer* taus(taus_nominal);
      xAOD::MissingETContainer* metcst(metcst_nominal);
      xAOD::MissingETContainer* mettst(mettst_nominal);
      xAOD::MissingETAuxContainer* metcst_aux(metcst_nominal_aux);
      xAOD::MissingETAuxContainer* mettst_aux(mettst_nominal_aux);

      xAOD::JetContainer* goodJets = new xAOD::JetContainer(SG::VIEW_ELEMENTS);


      // Tell the SUSYObjDef_xAOD which variation to apply
      if (objTool.applySystematicVariation(sys) != StatusCode::SUCCESS) {
        ANA_MSG_INFO( "Cannot configure SUSYTools for systematic var. " << (sys.name()).c_str() );
      } else {
        ANA_MSG_DEBUG( "Variation " << (sys.name()).c_str() << " configured...");
      }
      if (sysInfo.affectsKinematics || sysInfo.affectsWeights) isNominal = false;

      // If nominal, compute the nominal weight, otherwise recompute the weight
      double event_weight = base_event_weight;

      // If necessary (kinematics affected), make a shallow copy with the variation applied
      bool syst_affectsElectrons = ST::testAffectsObject(xAOD::Type::Electron, sysInfo.affectsType);
      bool syst_affectsMuons = ST::testAffectsObject(xAOD::Type::Muon, sysInfo.affectsType);
      bool syst_affectsTaus = ST::testAffectsObject(xAOD::Type::Tau, sysInfo.affectsType);
      bool syst_affectsPhotons = ST::testAffectsObject(xAOD::Type::Photon, sysInfo.affectsType);
      bool syst_affectsJets = ST::testAffectsObject(xAOD::Type::Jet, sysInfo.affectsType);
      bool syst_affectsBTag = ST::testAffectsObject(xAOD::Type::BTag, sysInfo.affectsType);
      //bool syst_affectsMET = ST::testAffectsObject(xAOD::Type::MissingET, sysInfo.affectsType);


      if (sysInfo.affectsKinematics) {
        if (slices["ele"] && syst_affectsElectrons) {
          xAOD::ElectronContainer* electrons_syst(nullptr);
          xAOD::ShallowAuxContainer* electrons_syst_aux(nullptr);
          ANA_CHECK( objTool.GetElectrons(electrons_syst, electrons_syst_aux, true, isPHYSLite?"AnalysisElectrons":"Electrons") );
          electrons = electrons_syst;
        }

        if (slices["mu"] && syst_affectsMuons) {
          xAOD::MuonContainer* muons_syst(nullptr);
          xAOD::ShallowAuxContainer* muons_syst_aux(nullptr);
          ANA_CHECK( objTool.GetMuons(muons_syst, muons_syst_aux, true, isPHYSLite?"AnalysisMuons":"Muons") );
          muons = muons_syst;
        }

        if(slices["tau"] && syst_affectsTaus) {
          xAOD::TauJetContainer* taus_syst(nullptr);
          xAOD::ShallowAuxContainer* taus_syst_aux(nullptr);
          ANA_CHECK( objTool.GetTaus(taus_syst,taus_syst_aux, true, isPHYSLite?"AnalysisTauJets":"TauJets") );
          taus = taus_syst;
        }

        if(slices["pho"] && syst_affectsPhotons) {
          xAOD::PhotonContainer* photons_syst(nullptr);
          xAOD::ShallowAuxContainer* photons_syst_aux(nullptr);
          ANA_CHECK( objTool.GetPhotons(photons_syst,photons_syst_aux, true, isPHYSLite?"AnalysisPhotons":"Photons") );
          photons = photons_syst;
        }

        if (slices["jet"] && syst_affectsJets) {
          xAOD::JetContainer* jets_syst(nullptr);
          xAOD::ShallowAuxContainer* jets_syst_aux(nullptr);
          ANA_CHECK( objTool.GetJetsSyst(*jets_nominal, jets_syst, jets_syst_aux, true, isPHYSLite?"AnalysisJets":"") );
          jets = jets_syst;
        }

        if (slices["btag"] && syst_affectsBTag) {
          xAOD::JetContainer* trkjets_syst(nullptr);
          xAOD::ShallowAuxContainer* trkjets_syst_aux(nullptr);
          ANA_CHECK( objTool.GetTrackJets(trkjets_syst, trkjets_syst_aux) );
          trkjets = trkjets_syst;
        }

        xAOD::MissingETContainer* metcst_syst = new xAOD::MissingETContainer;
        xAOD::MissingETAuxContainer* metcst_syst_aux = new xAOD::MissingETAuxContainer;
        xAOD::MissingETContainer* mettst_syst = new xAOD::MissingETContainer;
        xAOD::MissingETAuxContainer* mettst_syst_aux = new xAOD::MissingETAuxContainer;
        if (slices["met"]) {
          metcst_syst->setStore(metcst_syst_aux);
          mettst_syst->setStore(mettst_syst_aux);
          metcst_nominal->reserve(10);
          metcst_nominal->reserve(10);

          metcst = metcst_syst;
          mettst = mettst_syst;
          metcst_aux = metcst_syst_aux;
          mettst_aux = mettst_syst_aux;
        }
      }


      // ***************    ************************    *****************
      // *************** Now start processing the event *****************
      // ***************    ************************    *****************

      // Electrons
      if (slices["ele"] && (isNominal || (sysInfo.affectsKinematics && syst_affectsElectrons))) {
        for (const auto& el : *electrons) {
          //objTool.IsSignalElectron( *el ) ; // TODO: check that this should be removed /CO

          if (!isData) {
            const xAOD::TruthParticle* truthEle = xAOD::TruthHelpers::getTruthParticle(*el);
            if (truthEle) {
              ANA_MSG_DEBUG( " Truth Electron pt " << truthEle->pt() << " eta " << truthEle->eta() );
              ANA_MSG_DEBUG( " IsPrompt() = " << objTool.isPrompt(el) );
            } else { ANA_MSG_DEBUG( " Truth Electron not found" ); }
          }
        }
      }

      // Muons
      if (slices["mu"] && (isNominal || (sysInfo.affectsKinematics && syst_affectsMuons))) {
        for (const auto& mu : *muons) {
          // TODO: check that these should be removed too /CO
          //objTool.IsSignalMuon( *mu ) ;
          //objTool.IsCosmicMuon( *mu ) ;

          if (!isData) {
            // Example to access MC type/origin
            int muonTruthType = 0;
            int muonTruthOrigin = 0;
            const xAOD::TrackParticle* trackParticle = mu->primaryTrackParticle();
            if (trackParticle) {
              if (ST::acc_truthType.isAvailable(*trackParticle)) muonTruthType = ST::acc_truthType(*trackParticle);
              if (ST::acc_truthOrigin.isAvailable(*trackParticle)) muonTruthOrigin = ST::acc_truthOrigin(*trackParticle);
              const xAOD::TruthParticle* truthMu = xAOD::TruthHelpers::getTruthParticle(*trackParticle);
              if (truthMu) {
                ANA_MSG_DEBUG( " Truth Muon pt " << truthMu->pt() << " eta " << truthMu->eta() );
                ANA_MSG_DEBUG( "            type " << muonTruthType << " origin " <<  muonTruthOrigin );
              } else { ANA_MSG_DEBUG( "Truth Muon not found" ); }
            }
          }
        }
      }

      // Jets
      if (slices["jet"] && (isNominal || (sysInfo.affectsKinematics && syst_affectsJets))) {
        for (const auto& jet : *jets) {
          objTool.IsBJet( *jet) ;
        }
      }

      // Taus
      if (slices["tau"] && (isNominal || (sysInfo.affectsKinematics && syst_affectsTaus))) {
        for(const auto& tau : *taus){
          if (!isData){
            const xAOD::TruthParticle* truthTau = T2MT->getTruth(*tau) ;
            if (acc_IsTruthMatched(*tau) || !truthTau){
              ANA_MSG_DEBUG("Tau was matched to a truth tau, which has "
                            << int(acc_TruthProng(*tau))
                            << " prongs and a charge of "
                            << acc_TruthCharge(*tau));
            } else { ANA_MSG_DEBUG( "Tau was not matched to truth" ); }
          }
        }
      }

      // Overlap Removal
      ANA_MSG_DEBUG( "Overlap removal" );
      if (isNominal || (sysInfo.affectsKinematics && (syst_affectsElectrons || syst_affectsMuons || syst_affectsJets))) {
        if(stream.find("SUSY3")!=std::string::npos) {       ANA_CHECK( objTool.OverlapRemoval(electrons, muons, jets, 0, taus) ); }
        else if(stream.find("SUSY10")!=std::string::npos) { ANA_CHECK( objTool.OverlapRemoval(electrons, muons, jets, 0, 0, fatjets_nominal) ); }
        else{                                               ANA_CHECK( objTool.OverlapRemoval(electrons, muons, jets, photons) ); }
      }

      // Jets - get goodjets
      if (slices["jet"]) {
        ANA_MSG_DEBUG("GoodJets?");
        for (const auto& jet : *jets) {
          if (ST::acc_baseline(*jet) == 1  &&
              ST::acc_passOR(*jet) == 1  &&
              ST::acc_signal(*jet) == 1  &&
              jet->pt() > 20000.  && ( std::abs(jet->eta()) < 2.5) ) {
            goodJets->push_back(jet);
          }
        }
      }

      if (slices["met"] && (isNominal || sysInfo.affectsKinematics)) {
        ANA_MSG_DEBUG( "MET step" );
        if(stream.find("SUSY3")!=std::string::npos){
          if (debug>0) Info(APP_NAME, "METCST?");
          ANA_CHECK( objTool.GetMET(*metcst, jets, electrons, muons, photons, taus, false, false) ); // CST=false, JVT=false (No JVT if you use CST)
          if (debug>0) Info(APP_NAME, "METSignificance CST?");
          ANA_CHECK( objTool.GetMETSig(*metcst, metsig_cst, false, false) );

          if (debug>0) Info(APP_NAME, "METTST?");
          ANA_CHECK( objTool.GetMET(*mettst, jets, electrons, muons, photons, taus, true, true) ); 
          if (debug>0) Info(APP_NAME, "METSignificance TST?");
          ANA_CHECK( objTool.GetMETSig(*mettst, metsig_tst, true, true) );
        }
        else{
          if (debug>0) Info(APP_NAME, "METCST?");
          ANA_CHECK( objTool.GetMET(*metcst, jets, electrons, muons, photons, 0, false, false) ); // CST=false, JVT=false (No JVT if you use CST)
          if (debug>0) Info(APP_NAME, "METSignificance CST?");
          ANA_CHECK( objTool.GetMETSig(*metcst, metsig_cst, false, false) );

          if (debug>0) Info(APP_NAME, "METTST?");
          ANA_CHECK( objTool.GetMET(*mettst, jets, electrons, muons, photons, 0, true, true) ); 
          if (debug>0) Info(APP_NAME, "METSignificance TST?");
          ANA_CHECK( objTool.GetMETSig(*mettst, metsig_tst, true, true) );
        }
        if (debug>0) Info(APP_NAME, "MET done");
      }

      // ========================================
      // Electrons
      float elecSF = 1.0;
      int el_idx[nSel] = {0};
      if (slices["ele"]) {
        ANA_MSG_DEBUG( "Electron step - selection" );
        for (const auto& el : *electrons) {
          if ( ST::acc_passOR(*el) == 0  ) {
            el_idx[passOR]++;
            continue;
          }
          if ( ST::acc_baseline(*el) == 1  ) {
            el_idx[baseline]++;
          }
          if ( ST::acc_signal(*el) == 1  ) {
            el_idx[signallep]++;
            if ( el->pt() > 20000. ) {
              el_idx[goodpt]++;

              bool passTM=false;
              for(const auto& t : el_triggers){
                passTM |= (objTool.IsTrigPassed(t) && objTool.IsTrigMatched(el, t));
              }
              if(passTM)
                el_idx[trgmatch]++;

              //check ChID BDT
	      // static const SG::ConstAccessor<char> passChIDAcc("passChID");
	      // static const SG::ConstAccessor<double> ecisBDTAcc("ecisBDT");
              //Info(APP_NAME, "electron passChID : %d ,  BDT : %.3f", passChIDAcc(*el), ecisBDTAcc(*el));
            }
          }
        }
        if (isNominal || syst_affectsElectrons) {
          if(!isData) elecSF = objTool.GetTotalElectronSF(*electrons);
        }

        if (isNominal) {elecSF_nominal = elecSF;}
        else if (!syst_affectsElectrons) {elecSF = elecSF_nominal;}
        event_weight *= elecSF;
      }

      // ====================
      // Muons
      float muonSF = 1.0;
      int mu_idx[nSel] = {0};
      bool passTMtest = false;

      TString muTrig2015 = "HLT_mu20_iloose_L1MU15_OR_HLT_mu50"; //"HLT_mu18_mu8noL1"; //"HLT_mu20_iloose_L1MU15_OR_HLT_mu50";
      TString muTrig2016 = "HLT_mu26_ivarmedium_OR_HLT_mu50";
      TString muTrig2017 = "HLT_mu26_ivarmedium_OR_HLT_mu50";
      TString muTrig2022 = "HLT_mu24_ivarmedium_L1MU14FCH_OR_HLT_mu50_L1MU14FCH";

      std::vector<std::string> muTrigs2015 = {"HLT_mu20_iloose_L1MU15","HLT_mu50"}; //"HLT_mu18_mu8noL1"; //"HLT_mu20_iloose_L1MU15_OR_HLT_mu50";
      std::vector<std::string> muTrigs2016 = {"HLT_mu26_ivarmedium","HLT_mu50"};
      std::vector<std::string> muTrigs2017 = {"HLT_mu26_ivarmedium","HLT_mu50"};
      std::vector<std::string> muTrigs2022 = {"HLT_mu24_ivarmedium_L1MU14FCH","HLT_mu50_L1MU14FCH"};
      
      if (slices["mu"]) {
        ANA_MSG_DEBUG( "Muon step - selection" );
        for (const auto& mu : *muons) {
          if ( ST::acc_passOR(*mu) == 0  ) {
            mu_idx[passOR]++;
            continue;
          }
          if ( ST::acc_baseline(*mu) == 1  ) {
            mu_idx[baseline]++;
            if ( ST::acc_cosmic(*mu) == 1  ) {
              mu_idx[cosmic]++;
            }
          }
          if ( ST::acc_signal(*mu) == 1  ) {
            mu_idx[signallep]++;
            if ( mu->pt() > 20000. )  {
              mu_idx[goodpt]++;

              bool passTM=false;
              for(const auto& t : mu_triggers){
                if (objTool.IsTrigPassed(t)) ANA_MSG_DEBUG("Pass " << t << " : " << (int)objTool.IsTrigMatched(mu, t));
                else ANA_MSG_DEBUG("Pass " << t << " : " << 0);
                passTM |= (objTool.IsTrigPassed(t) && objTool.IsTrigMatched(mu, t));
              }
              if(passTM)
                mu_idx[trgmatch]++;

            }

            std::vector<std::string> my_mu_trigs;
            if(!isData){
              if(objTool.treatAsYear()==2015)
                my_mu_trigs=muTrigs2015;
              else if(objTool.treatAsYear()==2016)
                my_mu_trigs=muTrigs2016;
              else if(objTool.treatAsYear()==2017 || objTool.treatAsYear()==2018)
                my_mu_trigs=muTrigs2017;
              else
                my_mu_trigs=muTrigs2022;
            }
            else{
              my_mu_trigs=muTrigs2016;
            }
            for (auto& t : my_mu_trigs) passTMtest |= (objTool.IsTrigPassed(t) && objTool.IsTrigMatched(mu,t));
          }
        }
      }


      // ========================================
      // Photons
      if (slices["pho"]) {
        ANA_MSG_DEBUG( "Photon step - selection" );
        int n_SignalPhotons = 0;
        for (const auto& y : *photons) {
          if ( ST::acc_passOR(*y) && ST::acc_signal(*y)) {
            n_SignalPhotons++;
            ANA_MSG_DEBUG( "Photon pt = "<< y->pt()*1e-3
                          << ", idSF = "   << objTool.GetSignalPhotonSF(*y,true,false,false)
                          << ", isoSF = " << objTool.GetSignalPhotonSF(*y,false,true,false)
                          << ", trigSF = "  << objTool.GetSignalPhotonSF(*y,false,false,true)
                        );
          }
        }
        if(n_SignalPhotons>0) ANA_MSG_DEBUG( "Total Event photon SF = " << objTool.GetTotalPhotonSF(*photons, true, true, true));
      }

      // ====================
      // Check for combined e-mu triggers
      bool comb_trig_check = false;

      if (slices["ele"] && slices["mu"] && comb_trig_check) {
        ANA_MSG_DEBUG( "Electron/Muon step - trigger" );
        if (objTool.IsTrigPassed("HLT_2e12_lhloose_L12EM10VH"))
          ANA_MSG_DEBUG(" 2e12_lhloose_L12EM10VH SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "diLepton"));
        if (objTool.IsTrigPassed("HLT_e17_lhloose_mu14"))
          ANA_MSG_DEBUG("e17_lhloose_mu14 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "diLepton") );
        if (objTool.IsTrigPassed("HLT_2e17_lhvloose_nod0"))
          ANA_MSG_DEBUG(" 2e17_lhvloose_nod0 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "diLepton") );
        if (objTool.IsTrigPassed("HLT_2e17_lhvloose_nod0_L12EM15VHI"))
          ANA_MSG_DEBUG(" 2e17_lhvloose_nod0_L12EM15VHI SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "diLepton") );
        if (objTool.IsTrigPassed("HLT_e17_lhloose_nod0_mu14"))
          ANA_MSG_DEBUG(" e17_lhloose_nod0_mu14 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "diLepton") );
        if (objTool.IsTrigPassed("HLT_e7_lhmedium_mu24"))
          ANA_MSG_DEBUG(" e7_lhmedium_mu24 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "diLepton") );
        if (objTool.IsTrigPassed("HLT_e7_lhmedium_nod0_mu24"))
          ANA_MSG_DEBUG(" e7_lhmedium_nod0_mu24 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "diLepton") );
        if (objTool.IsTrigPassed("HLT_mu18_mu8noL1"))
          ANA_MSG_DEBUG(" mu18_mu8noL1 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "diLepton") );
        if (objTool.IsTrigPassed("HLT_mu20_mu8noL1"))
          ANA_MSG_DEBUG(" mu20_mu8noL1 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "diLepton") );
        if (objTool.IsTrigPassed("HLT_2mu10"))
          ANA_MSG_DEBUG(" 2mu10 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "diLepton") );
        if (objTool.IsTrigPassed("HLT_2mu14"))
          ANA_MSG_DEBUG(" 2mu14 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "diLepton") );
        if (objTool.IsTrigPassed("HLT_2e12_lhloose_mu10"))
          ANA_MSG_DEBUG(" 2e12_lhloose_mu10 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "multiLepton") );
        if (objTool.IsTrigPassed("HLT_2e12_lhloose_nod0_mu10"))
          ANA_MSG_DEBUG(" 2e12_lhloose_nod0_mu10 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "multiLepton") );
        if (objTool.IsTrigPassed("HLT_e12_lhloose_2mu10"))
          ANA_MSG_DEBUG(" e12_lhloose_2mu10 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "multiLepton") );
        if (objTool.IsTrigPassed("HLT_e12_lhloose_nod0_2mu10"))
          ANA_MSG_DEBUG(" e12_lhloose_nod0_2mu10 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "multiLepton") );
        if (objTool.IsTrigPassed("HLT_3mu6"))
          ANA_MSG_DEBUG(" 3mu6 SF:  " << objTool.GetTriggerGlobalEfficiencySF(*electrons_nominal, *muons_nominal, "multiLepton") );
      }

      if (slices["mu"] && ((!isData && isNominal) || syst_affectsMuons)) {
        ANA_MSG_DEBUG( "Muon step - trigger" );
        if(passTMtest || 1){ //objTool.IsTrigPassed(muTrig.ReplaceAll("_OR_",",").Data())){
          ANA_MSG_DEBUG("MUON BEFORE SF = " << muonSF << "   " << objTool.treatAsYear() << "   "  << objTool.GetRandomRunNumber() << "    " <<  objTool.GetPileupWeight() );
          if(objTool.treatAsYear()==2015)        muonSF = objTool.GetTotalMuonSF(*muons, true, true, muTrig2015.Data());
          else if(objTool.treatAsYear()==2016)   muonSF = objTool.GetTotalMuonSF(*muons, true, true, muTrig2016.Data());
          else if(objTool.treatAsYear()==2017)   muonSF = objTool.GetTotalMuonSF(*muons, true, true, muTrig2017.Data());
          else if(objTool.treatAsYear()==2018)   muonSF = objTool.GetTotalMuonSF(*muons, true, true, muTrig2017.Data());
          else if(objTool.treatAsYear()==2022)   muonSF = objTool.GetTotalMuonSF(*muons, true, true, muTrig2022.Data());
          ANA_MSG_DEBUG("MUON AFTER SF =  " << muonSF << "   " << objTool.treatAsYear() << "   "  << objTool.GetRandomRunNumber() << "    " <<  objTool.GetPileupWeight() );
        }
      }

      if (slices["mu"]) {
        if (isNominal) { muonSF_nominal = muonSF; }
        else if (!syst_affectsMuons) { muonSF = muonSF_nominal; }
        event_weight *= muonSF;
      }

      // ====================
      // Taus
      if (slices["tau"] && isNominal && !isData && stream.find("SUSY3")!=std::string::npos) {
        ANA_MSG_DEBUG( "Tau step - SF" );
        if (entry<10){
          // CP::SystematicSet testSet("TAUS_TRUEHADTAU_EFF_TRIGGER_TOTAL2016__1up");
          // testSet.insert( CP::SystematicVariation("TAUS_TRUEHADTAU_EFF_TRIGGER_TOTAL2016", 1) );

          // CP::SystematicSet testSet("TAUS_TRUEHADTAU_EFF_TRIGGER_SYST2015__1up");
          // testSet.insert( CP::SystematicVariation("TAUS_TRUEHADTAU_EFF_TRIGGER_SYST2015", 1) );
          // if(objTool.applySystematicVariation(testSet) != StatusCode::SUCCESS){
          //   Error( APP_NAME, "Problems with tau trig eff systematic settings!");
          // }

          for (const auto& tau : *taus) {
            ANA_MSG_INFO( "Tau pt = "<< tau->pt()*1e-3
                          << ", idSF = "   << objTool.GetSignalTauSF(*tau,true,false,"tau125_medium1_tracktwo")
                          << ", trigSF = " << objTool.GetSignalTauSF(*tau,false,true,"tau125_medium1_tracktwo")
                          << ", totSF = "  << objTool.GetSignalTauSF(*tau,true,true,"tau125_medium1_tracktwo")
                        );
          }
          ANA_MSG_INFO( "Total Event Tau SF = " << objTool.GetTotalTauSF(*taus, true, true, "tau125_medium1_tracktwo") );
        }
      }

      ANA_MSG_DEBUG( "Lepton SF done" );

      // ====================
      // Jets
      int jet_idx[nSel] = {0};
      if (slices["jet"]) {
        ANA_MSG_DEBUG( "Jet step - selection" );
        for (const auto& jet : *goodJets) {
          if ( ST::acc_bad(*jet) == 1 )
            jet_idx[bad]++;
          if ( ST::acc_passOR(*jet) == 0 ) {
            jet_idx[passOR]++;
            continue;
          }
          if ( ST::acc_baseline(*jet) == 1 ) {
            jet_idx[baseline]++;
            if ( jet->pt() > 50000. )
              jet_idx[goodpt]++;
          }
          if ( ST::acc_bjet(*jet) == 1 )
            jet_idx[btagged]++;
        }
      }

      // compute b-tagging SF
      float btagSF(1.);
      if (slices["jet"] && slices["btag"]) {
        ANA_MSG_DEBUG( "(b)Jet step - selection" );
        if (!isData) {
          if (isNominal) {btagSF = btagSF_nominal = objTool.BtagSF(jets);}
          else if (syst_affectsBTag || (sysInfo.affectsKinematics && syst_affectsJets)) {btagSF = objTool.BtagSF(jets);}
          else {btagSF = btagSF_nominal;}
        }
        event_weight *= btagSF;

        // checking BtagSF 
        if ( stream.find("SUSY1")!=std::string::npos ) { // PHYSVAL doesn't contain truthlabel for VR jets
          float btagSF_trkJet(1.);
          if (!isData) {
            if (isNominal) {btagSF_trkJet = btagSF_trkJet_nominal = objTool.BtagSF_trkJet(trkjets);}
            else if (syst_affectsBTag || (sysInfo.affectsKinematics && syst_affectsJets)) {btagSF_trkJet = objTool.BtagSF_trkJet(trkjets);}
            else {btagSF_trkJet = btagSF_trkJet_nominal;}
          }
          event_weight *= btagSF_trkJet;
        }
      }

      ANA_MSG_DEBUG("Jet SF done");

      // ====================
      // Cosmics
      if (slices["ele"] && slices["mu"] && mu_idx[cosmic] == 0 ) {
        elcuts[isys][icut] += 1;
        mucuts[isys][icut] += 1;
        ++icut;

        bool passlep = (el_idx[baseline] + mu_idx[baseline]) == 1;
        if (passlep) {
          bool passel = el_idx[baseline] == 1;
          bool passmu = mu_idx[baseline] == 1;
          if (passel) elcuts[isys][icut] += 1;
          if (passmu) mucuts[isys][icut] += 1;
          ++icut;

          passel = el_idx[signallep] == 1;
          passmu = mu_idx[signallep] == 1;
          if (passel) elcuts[isys][icut] += 1;
          if (passmu) mucuts[isys][icut] += 1;
          ++icut;

          passel = el_idx[goodpt] == 1;
          passmu = mu_idx[goodpt] == 1;
          if (passel) elcuts[isys][icut] += 1;
          if (passmu) mucuts[isys][icut] += 1;
          ++icut;

          passel = el_idx[trgmatch] == 1;
          passmu = mu_idx[trgmatch] == 1;
          if (passel) elcuts[isys][icut] += 1;
          if (passmu) mucuts[isys][icut] += 1;
          ++icut;

          if (jet_idx[goodpt] >= 2) {
            if (passel) elcuts[isys][icut] += 1;
            if (passmu) mucuts[isys][icut] += 1;
            ++icut;
          } // good jets
        } // passlep
      } // cosmics

      ANA_MSG_DEBUG("Lepton SF done");
      ANA_MSG_DEBUG("Final event weight = " << event_weight);

      // Clean up the systematics copies
      if (sysInfo.affectsKinematics) {
        delete metcst;
        delete metcst_aux;
        delete mettst;
        delete mettst_aux;
      }

      isNominal = false;
      ANA_MSG_DEBUG(">>>> Finished with variation: \"" <<(sys.name()).c_str() << "\" <<<<<<" );


      ++isys;
    }


    //Reset systematics settings for next event
    ANA_CHECK( objTool.resetSystematics());


    // The containers created by the shallow copy are owned by you. Remember to delete them.
    // In our case, all of these were put into the store

    // store.print();
    store.clear();

    // Close with a message:
    if (entry==0 || entry % period == 99) {
      Info( APP_NAME,
            "===>>>  done processing event #%i, "
            "run #%i %i events processed so far  <<<===",
            static_cast< int >( ei->eventNumber() ),
            static_cast< int >( ei->runNumber() ),
            static_cast< int >( entry + 1 ) );
    }


  }

  m_clock0.Stop();
  m_clock2.Stop();

  // CALLGRIND_TOGGLE_COLLECT;
  // CALLGRIND_DUMP_STATS;
  // ProfilerStop();

  Info( APP_NAME,
        "Time per event:          (CPU / WALL)\n"
        "  Entire loop           - %f / %f ms\n"
        "  First event           - %f / %f ms\n"
        "  Excluding first event - %f / %f ms",
        m_clock0.CpuTime() * 1e3 / entries, m_clock0.RealTime() * 1e3 / entries,
        m_clock1.CpuTime() * 1e3, m_clock1.RealTime() * 1e3,
        m_clock2.CpuTime() * 1e3 / (entries - 1), m_clock2.RealTime() * 1e3 / (entries - 1));


  // Dump the cutflow:
  isys = 0;
  for (const auto& sysInfo : systInfoList) {
    const CP::SystematicSet& sys = sysInfo.systset;
    ANA_MSG_INFO( "-- Dummy Cutflow -- >>>> Variation " << sys.name().c_str() << " <<<<" );
    for (size_t icut = 0; icut < Ncuts; ++icut) {
      ANA_MSG_INFO( "Cut " << icut << "(" << cut_name[icut] << ")" 
                    << "-> el: " << elcuts[isys][icut] << ", mu: " << mucuts[isys][icut] );
    }
    ++isys;
  }

  // Return gracefully:
  return 0;
}

//====================================================================================================
//====================================================================================================
std::vector<std::string> getTokens(TString line, const TString& delim) {
  std::vector<std::string> vtokens;
  TObjArray* tokens = TString(std::move(line)).Tokenize(delim); //delimiters
  if(tokens->GetEntriesFast()) {
    TIter iString(tokens);
    TObjString* os = nullptr;
    while ((os=(TObjString*)iString())) {
      vtokens.push_back( os->GetString().Data() );
    }
  }
  delete tokens;

  return vtokens;
}

//====================================================================================================
// get list of containers in file ~ checkxAOD.py style
std::map<std::string, std::string> getFileContainers(std::unique_ptr<TFile> &f) {
  std::map< std::string, std::string > containers;
  std::unique_ptr< TTree > ctree( dynamic_cast<TTree*>( f->Get("CollectionTree") ) );
  TObjArray *blist = dynamic_cast<TObjArray*>( ctree->GetListOfBranches() );
  std::smatch match;
  std::string result, bname;
  for (int ib=0; ib<blist->GetEntries(); ++ib) {
     TBranch *b = dynamic_cast<TBranch*>( blist->At(ib) );
     bname = b->GetName();
     if (bname.find("Aux")==std::string::npos) continue;
     //
     if (std::regex_search(bname, match, std::regex("(.*)Aux\\..*")) && match.size() > 1) { result = match.str(1); }            // static
     else if (std::regex_search(bname, match, std::regex("(.*)AuxDyn\\..*")) && match.size() > 1) { result = match.str(1); }    // dynamic
     //
     if ((!result.empty()) && (containers.find(result)==containers.end()) && (ctree->GetBranch(result.c_str()))) {
        containers[result] = ctree->GetBranch(result.c_str())->GetClassName();
     }
     result = "";
  }
  return containers;
}
