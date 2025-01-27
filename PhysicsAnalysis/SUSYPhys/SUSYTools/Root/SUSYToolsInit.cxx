/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "SUSYTools/SUSYObjDef_xAOD.h"

// For the data types to be used in configuring tools
#include "PATCore/PATCoreEnums.h"

// For the tau tool initialization
#include "TauAnalysisTools/Enums.h"

// For the struct needed for OR init
#include "AssociationUtils/OverlapRemovalInit.h"

// Abstract interface classes
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"

#include "JetInterface/IJetSelector.h"
#include "JetCalibTools/IJetCalibrationTool.h"
#include "JetCPInterfaces/ICPJetUncertaintiesTool.h"
#include "JetInterface/IJetUpdateJvt.h"
#include "JetInterface/IJetModifier.h"
#include "JetInterface/IJetDecorator.h"

#include "AsgAnalysisInterfaces/IEfficiencyScaleFactorTool.h"
#include "EgammaAnalysisInterfaces/IEgammaCalibrationAndSmearingTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronEfficiencyCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "EgammaAnalysisInterfaces/IAsgDeadHVCellRemovalTool.h"
#include "EgammaAnalysisInterfaces/IEGammaAmbiguityTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonEfficiencyCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IElectronLRTOverlapRemovalTool.h"

#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "MuonAnalysisInterfaces/IMuonCalibrationAndSmearingTool.h"
#include "MuonAnalysisInterfaces/IMuonEfficiencyScaleFactors.h"
#include "MuonAnalysisInterfaces/IMuonTriggerScaleFactors.h"
#include "MuonAnalysisInterfaces/IMuonLRTOverlapRemovalTool.h"

#include "TauAnalysisTools/ITauSelectionTool.h"
#include "TauAnalysisTools/ITauSmearingTool.h"
#include "TauAnalysisTools/ITauTruthMatchingTool.h"
#include "TauAnalysisTools/ITauEfficiencyCorrectionsTool.h"
#include "tauRecTools/ITauToolBase.h"

#include "IsolationSelection/IIsolationSelectionTool.h"
//disable #include "IsolationSelection/IIsolationLowPtPLVTool.h"
#include "IsolationCorrections/IIsolationCorrectionTool.h"
#include "IsolationSelection/IIsolationCloseByCorrectionTool.h"

#include "METInterface/IMETMaker.h"
#include "METInterface/IMETSystematicsTool.h"
#include "METInterface/IMETSignificance.h"

#include "TrigConfInterfaces/ITrigConfigTool.h"
#include "TriggerMatchingTool/IMatchingTool.h"
#include "TriggerMatchingTool/IMatchScoringTool.h"
#include "TriggerAnalysisInterfaces/ITrigGlobalEfficiencyCorrectionTool.h"
// Can't use the abstract interface for this one (see header comment)
#include "TrigDecisionTool/TrigDecisionTool.h"

#include "PATInterfaces/IWeightTool.h"
#include "AsgAnalysisInterfaces/IPileupReweightingTool.h"
#include "PathResolver/PathResolver.h"
#include "AssociationUtils/IOverlapRemovalTool.h"
#include "BoostedJetTaggers/SmoothedWZTagger.h"
#include "BoostedJetTaggers/JSSWTopTaggerDNN.h"
#include "ParticleJetTools/JetTruthLabelingTool.h"

#include "InDetTrackSystematicsTools/IInclusiveTrackFilterTool.h"

#ifndef XAOD_STANDALONE
#include "AthAnalysisBaseComps/AthAnalysisHelper.h"
#endif

using namespace ST;



#define CONFIG_EG_EFF_TOOL( TOOLHANDLE, TOOLNAME, CORRFILE )                \
  if( !TOOLHANDLE.isUserConfigured() ) {                                \
    TOOLHANDLE.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+TOOLNAME); \
    std::vector< std::string > corrFileNameList = {CORRFILE}; \
    ATH_CHECK( TOOLHANDLE.setProperty("CorrectionFileNameList", corrFileNameList) ); \
    if(!isData())                                                        \
      ATH_CHECK (TOOLHANDLE.setProperty("ForceDataType", static_cast<int>(data_type))); \
    ATH_CHECK( TOOLHANDLE.setProperty("CorrelationModel", m_EG_corrModel) ); \
    ATH_CHECK( TOOLHANDLE.setProperty("OutputLevel", this->msg().level()) ); \
    ATH_CHECK( TOOLHANDLE.initialize() );                                \
  } else ATH_CHECK(TOOLHANDLE.retrieve());

#define CONFIG_EG_EFF_TOOL_KEY( TOOLHANDLE, TOOLNAME, KEYNAME, KEY, MAP )   \
  if( !TOOLHANDLE.isUserConfigured() ) {                                \
    TOOLHANDLE.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+TOOLNAME); \
    ATH_MSG_INFO( "Will now set key \"" << KEYNAME << "\" to value \"" << KEY << "\" when configuring an AsgElectronEfficiencyCorrectionTool" ); \
    ATH_CHECK( TOOLHANDLE.setProperty(KEYNAME, KEY) );                  \
    if(!isData())                                                        \
      ATH_CHECK (TOOLHANDLE.setProperty("ForceDataType", static_cast<int>(data_type))); \
    ATH_CHECK( TOOLHANDLE.setProperty("CorrelationModel", m_EG_corrModel) ); \
    ATH_CHECK( TOOLHANDLE.setProperty("OutputLevel", this->msg().level()) ); \
    ATH_CHECK( TOOLHANDLE.setProperty("MapFilePath", MAP) ); \
    ATH_CHECK( TOOLHANDLE.initialize() );                                \
  } else if (!isData()) ATH_CHECK(TOOLHANDLE.retrieve());

StatusCode SUSYObjDef_xAOD::SUSYToolsInit()
{
  if (m_dataSource < 0) {
    ATH_MSG_FATAL("Data source incorrectly configured!!");
    ATH_MSG_FATAL("You must set the DataSource property to Data, FullSim or AtlfastII !!");
    ATH_MSG_FATAL("Expect segfaults if you're not checking status codes, which you should be !!");
    return StatusCode::FAILURE;
  }

  if (m_subtool_init) {
    ATH_MSG_INFO("SUSYTools subtools already created. Ignoring this call.");
    ATH_MSG_INFO("Note: No longer necessary to explicitly call SUSYToolsInit. Will avoid creating tools again.");
    return StatusCode::SUCCESS;
  }

  // /////////////////////////////////////////////////////////////////////////////////////////
  // Initialise PileupReweighting Tool

  if (!m_prwTool.isUserConfigured()) {
    ATH_MSG_DEBUG("Will now init the PRW tool");
    std::vector<std::string> file_conf;
    for (UInt_t i = 0; i < m_prwConfFiles.size(); i++) {
      ATH_MSG_INFO("Adding PRW file: " << m_prwConfFiles.at(i));
      file_conf.push_back(m_prwConfFiles.at(i));
    }

    std::vector<std::string> file_ilumi;
    for (UInt_t i = 0; i < m_prwLcalcFiles.size(); i++) {
      ATH_MSG_INFO("Adding ilumicalc file: " << m_prwLcalcFiles.at(i));
      file_ilumi.push_back(m_prwLcalcFiles.at(i));
    }

    m_prwTool.setTypeAndName("CP::PileupReweightingTool/PrwTool");
    ATH_CHECK( m_prwTool.setProperty("ConfigFiles", file_conf) );
    ATH_CHECK( m_prwTool.setProperty("LumiCalcFiles", file_ilumi) );
    ATH_CHECK( m_prwTool.setProperty("DataScaleFactor",     m_prwDataSF) ); // 1./1.03 -> default for mc16, see: https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ExtendedPileupReweighting#Tool_Properties
    ATH_CHECK( m_prwTool.setProperty("DataScaleFactorUP",   m_prwDataSF_UP) ); // 1. -> old value (mc15), as the one for mc16 is still missing
    ATH_CHECK( m_prwTool.setProperty("DataScaleFactorDOWN", m_prwDataSF_DW) ); // 1./1.18 -> old value (mc15), as the one for mc16 is still missing
    ATH_CHECK( m_prwTool.setProperty("TrigDecisionTool", m_trigDecTool.getHandle()) );
    ATH_CHECK( m_prwTool.setProperty("UseRunDependentPrescaleWeight", m_runDepPrescaleWeightPRW) );
    ATH_CHECK( m_prwTool.setProperty("OutputLevel", MSG::WARNING) );
    ATH_CHECK( m_prwTool.retrieve() );
  } else {
    ATH_MSG_INFO("Using user-configured PRW tool");
    ATH_CHECK( m_prwTool.retrieve() );
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  // Initialize LRT systematics tool
  if (!m_LRTuncTool.isUserConfigured())
  {
    ATH_MSG_INFO("Initializing LRT uncertainty tool");
    m_LRTuncTool.setTypeAndName("InDet::InclusiveTrackFilterTool/LRTUncTool");    
    ATH_CHECK( m_LRTuncTool.setProperty("Seed", 1) );
    ATH_CHECK(m_LRTuncTool.retrieve());
  } else {
    ATH_MSG_INFO("Using user-configured LRT uncertainty tool");
    ATH_CHECK(m_LRTuncTool.retrieve());
  }

  std::string toolName; // to be used for tool init below, keeping explicit string constants a minimum /CO

  std::string jetname,jetcoll,fatjetcoll;
  if (m_slices["jet"]) {
    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise jet calibration tool

    // pick the right config file for the JES tool : https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ApplyJetCalibrationR22
    jetname = "AntiKt4" + xAOD::JetInput::typeName(xAOD::JetInput::Type(m_jetInputType));
    jetcoll = jetname + "Jets";

    if (!m_jetCalibTool.isUserConfigured()) {
      toolName = "JetCalibTool_" + jetname;
      m_jetCalibTool.setTypeAndName("JetCalibrationTool/"+toolName);
      std::string JES_config_file, calibseq;

      if (m_jetInputType != xAOD::JetInput::EMPFlow) {
        ATH_MSG_ERROR("Unknown (unsupported) jet collection is used, (m_jetInputType = " << m_jetInputType << ")");
        return StatusCode::FAILURE;
      }

      std::string JESconfig = isAtlfast() ? m_jesConfigAFII : m_jesConfig;
      if(isAtlfast() && m_isRun3) {
        ATH_MSG_WARNING("Jet JES/JER recommendations currently not available for fast sim in Run 3, falling back to full sim version");
        JESconfig = m_jesConfig;
      }

      calibseq = m_jesCalibSeq;
      JES_config_file = JESconfig;

      // remove Insitu if it's in the string if not data
      if (!isData()) {
        std::string insitu("_Insitu");
        auto found = calibseq.find(insitu);
        if(found != std::string::npos){
          calibseq.erase(found, insitu.length());
        }
      }

      // JMS calibration (if requested)
      if (m_JMScalib){
          ATH_MSG_ERROR("JMS calibration is not supported yet for R22. Please modify your settings.");
          return StatusCode::FAILURE;
        }

      // now instantiate the tool
      ATH_CHECK( m_jetCalibTool.setProperty("JetCollection", jetname) );
      ATH_CHECK( m_jetCalibTool.setProperty("ConfigFile", JES_config_file) );
      ATH_CHECK( m_jetCalibTool.setProperty("CalibSequence", calibseq) );
      ATH_CHECK( m_jetCalibTool.setProperty("CalibArea", m_jesCalibArea) );
      ATH_CHECK( m_jetCalibTool.setProperty("IsData", isData()) );
      ATH_CHECK( m_jetCalibTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetCalibTool.retrieve() );
    } else ATH_CHECK(m_jetCalibTool.retrieve());
  }

  if (m_slices["fjet"]) {
    //same for fat groomed jets
    fatjetcoll = m_fatJets;

    if(fatjetcoll == "AnalysisLargeRJets") {
      ATH_MSG_DEBUG("Fall back to calibration for AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets, original name for AnalysisLargeRJets");
      fatjetcoll = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets";
    }
    if (fatjetcoll.size()>3)fatjetcoll.erase(fatjetcoll.size()-4,4);
    if (!m_jetFatCalibTool.isUserConfigured() && !m_fatJets.empty()) {
      toolName = "JetFatCalibTool_" + m_fatJets;
      m_jetFatCalibTool.setTypeAndName("JetCalibrationTool/"+toolName);

      std::string jesConfigFat = m_jesConfigFat;
      std::string jesCalibSeqFat = m_jesCalibSeqFat;
      // add Insitu if data
      if(isData()) jesCalibSeqFat += "_Insitu";

      // now instantiate the tool
      ATH_CHECK( m_jetFatCalibTool.setProperty("JetCollection", fatjetcoll) );
      ATH_CHECK( m_jetFatCalibTool.setProperty("ConfigFile", jesConfigFat) );
      ATH_CHECK( m_jetFatCalibTool.setProperty("CalibSequence", jesCalibSeqFat) );
      // always set to false : https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/ApplyJetCalibrationR21
      ATH_CHECK( m_jetFatCalibTool.setProperty("IsData", isData()) );
      ATH_CHECK( m_jetFatCalibTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetFatCalibTool.retrieve() );
    } else if (m_jetFatCalibTool.isUserConfigured()) ATH_CHECK(m_jetFatCalibTool.retrieve());
  }

  if (m_slices["fjet"]) {
    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise Boson taggers: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BoostedJetTaggingRecommendationFullRun2#Higgs_taggers
    if (!m_WTaggerTool.isUserConfigured() && !m_WtagConfig.empty()) {
      m_WTaggerTool.setTypeAndName("SmoothedWZTagger/WTagger");
      ATH_CHECK( m_WTaggerTool.setProperty("ContainerName", m_fatJets) );
      ATH_CHECK( m_WTaggerTool.setProperty("ConfigFile", m_WtagConfig) );
      ATH_CHECK( m_WTaggerTool.setProperty("CalibArea", m_WZTaggerCalibArea) );
      ATH_CHECK( m_WTaggerTool.setProperty("IsMC",!isData()));
      ATH_CHECK( m_WTaggerTool.setProperty("TruthBosonContainerName", "TruthBoson") );  // Set this if you are using a TRUTH3 style truth boson container;
      ATH_CHECK( m_WTaggerTool.setProperty("TruthTopQuarkContainerName", "TruthTop") );  // Set this if you are using a TRUTH3 style truth boson container;
#ifndef XAOD_STANDALONE
      ATH_CHECK( m_WTaggerTool.setProperty("SuppressOutputDependence", true) );
#endif
      ATH_CHECK( m_WTaggerTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_WTaggerTool.retrieve() );

      // Retrieving DecorationName (needed to access tagging results downstream):
      std::string WConfigPath = PathResolverFindCalibFile("BoostedJetTaggers/"+m_WZTaggerCalibArea+"/"+m_WtagConfig);
      if ( m_WconfigReader.ReadFile( WConfigPath.c_str(), EEnvLevel(0) ) ) {
        ATH_MSG_ERROR( "Error while reading large-R config file : " << WConfigPath );
        return StatusCode::FAILURE;
      }
      else ATH_MSG_DEBUG( "Successfully read large-R config file : " << WConfigPath );

      m_WDecorName = m_WconfigReader.GetValue("DecorationName" ,"");
      ANA_MSG_DEBUG( "Found DecorationName in large-R config file : " << m_WDecorName );

    } else if (m_WTaggerTool.isUserConfigured()) ATH_CHECK(m_WTaggerTool.retrieve());

    if (!m_ZTaggerTool.isUserConfigured() && !m_ZtagConfig.empty()) {
      m_ZTaggerTool.setTypeAndName("SmoothedWZTagger/ZTagger");
      ATH_CHECK( m_ZTaggerTool.setProperty("ContainerName", m_fatJets) );
      ATH_CHECK( m_ZTaggerTool.setProperty("ConfigFile", m_ZtagConfig) );
      ATH_CHECK( m_ZTaggerTool.setProperty("CalibArea", m_WZTaggerCalibArea) );
      ATH_CHECK( m_ZTaggerTool.setProperty("IsMC",!isData()));
      ATH_CHECK( m_ZTaggerTool.setProperty("TruthBosonContainerName", "TruthBoson") );  // Set this if you are using a TRUTH3 style truth boson container;
      ATH_CHECK( m_ZTaggerTool.setProperty("TruthTopQuarkContainerName", "TruthTop") );  // Set this if you are using a TRUTH3 style truth boson container;
#ifndef XAOD_STANDALONE
      ATH_CHECK( m_ZTaggerTool.setProperty("SuppressOutputDependence", true) );
#endif
      ATH_CHECK( m_ZTaggerTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_ZTaggerTool.retrieve() );

      // Retrieving DecorationName (needed to access tagging results downstream):
      std::string ZConfigPath = PathResolverFindCalibFile("BoostedJetTaggers/"+m_WZTaggerCalibArea+"/"+m_ZtagConfig);
      if ( m_ZconfigReader.ReadFile( ZConfigPath.c_str(), EEnvLevel(0) ) ) {
        ATH_MSG_ERROR( "Error while reading large-R config file : " << ZConfigPath );
        return StatusCode::FAILURE;
      }
      else ATH_MSG_DEBUG( "Successfully read large-R config file : " << ZConfigPath );

      m_ZDecorName = m_ZconfigReader.GetValue("DecorationName" ,"");
      ANA_MSG_DEBUG( "Found DecorationName in large-R config file : " << m_ZDecorName );

    } else if (m_ZTaggerTool.isUserConfigured()) ATH_CHECK(m_ZTaggerTool.retrieve());

    if (!m_TopTaggerTool.isUserConfigured() && !m_ToptagConfig.empty()) {
      m_TopTaggerTool.setTypeAndName("JSSWTopTaggerDNN/TopTagger");
      ATH_CHECK( m_TopTaggerTool.setProperty("ContainerName", m_fatJets) );
      ATH_CHECK( m_TopTaggerTool.setProperty("ConfigFile", m_ToptagConfig) );
      ATH_CHECK( m_TopTaggerTool.setProperty("CalibArea", m_TopTaggerCalibArea) );
      ATH_CHECK( m_TopTaggerTool.setProperty("IsMC",!isData()));
      ATH_CHECK( m_TopTaggerTool.setProperty("TruthBosonContainerName", "TruthBoson") );  // Set this if you are using a TRUTH3 style truth boson container;
      ATH_CHECK( m_TopTaggerTool.setProperty("TruthTopQuarkContainerName", "TruthTop") );  // Set this if you are using a TRUTH3 style truth boson container;
#ifndef XAOD_STANDALONE
      ATH_CHECK( m_TopTaggerTool.setProperty("SuppressOutputDependence", true) );
#endif
      ATH_CHECK( m_TopTaggerTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_TopTaggerTool.retrieve() );

      // Retrieving DecorationName (needed to access tagging results downstream):
      std::string TopConfigPath = PathResolverFindCalibFile("BoostedJetTaggers/"+m_TopTaggerCalibArea+"/"+m_ToptagConfig);
      if ( m_TopconfigReader.ReadFile( TopConfigPath.c_str(), EEnvLevel(0) ) ) {
        ATH_MSG_ERROR( "Error while reading large-R config file : " << TopConfigPath );
        return StatusCode::FAILURE;
      }
      else ATH_MSG_DEBUG( "Successfully read large-R config file : " << TopConfigPath );

      m_TopDecorName = m_TopconfigReader.GetValue("DecorationName" ,"");
      ANA_MSG_DEBUG( "Found DecorationName in large-R config file : " << m_TopDecorName );

    } else if (m_TopTaggerTool.isUserConfigured()) ATH_CHECK(m_TopTaggerTool.retrieve());

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise JetTruthLabelingTool: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetUncertaintiesRel21Summer2019LargeR#AnalysisBase_21_2_114_and_newer
    if(!isData()) {
      m_label_truthKey = fatjetcoll+"."+m_JetTruthLabelName;
      ATH_CHECK(m_label_truthKey.initialize());

      if (!m_jetTruthLabelingTool.isUserConfigured()) {
	m_jetTruthLabelingTool.setTypeAndName("JetTruthLabelingTool/ST_JetTruthLabelingTool");
	ATH_CHECK( m_jetTruthLabelingTool.setProperty("RecoJetContainer", m_fatJets) );
	ATH_CHECK( m_jetTruthLabelingTool.setProperty("TruthLabelName", m_JetTruthLabelName) );
	ATH_CHECK( m_jetTruthLabelingTool.setProperty("UseTRUTH3", m_useTRUTH3) );                 // Set this to false only if you have the FULL !TruthParticles container in your input file
	ATH_CHECK( m_jetTruthLabelingTool.setProperty("TruthParticleContainerName", "TruthParticles") );
	ATH_CHECK( m_jetTruthLabelingTool.setProperty("TruthBosonContainerName", "TruthBoson") );  // Set this if you are using a TRUTH3 style truth boson container
	ATH_CHECK( m_jetTruthLabelingTool.setProperty("TruthTopQuarkContainerName", "TruthTop") ); // Set this if you are using a TRUTH3 style truth top quark container
	ATH_CHECK( m_jetTruthLabelingTool.setProperty("OutputLevel", this->msg().level()) );
	ATH_CHECK( m_jetTruthLabelingTool.retrieve() );
      } else if (m_jetTruthLabelingTool.isUserConfigured()) ATH_CHECK(m_jetTruthLabelingTool.retrieve());
    }
  }

  if (m_slices["jet"] || m_slices["fjet"]) {


    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise jet uncertainty tool
    // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/JetUncertaintiesRel22
    ATH_MSG_INFO("Set up Jet Uncertainty tool...");

    // if not set, derive the MCType from the simulation type and MC campaign
    if (m_jetUncertaintiesMCType.empty()) m_jetUncertaintiesMCType = m_isRun3 ? "MC23" : (isAtlfast() ? "AF3" : "MC20");

    if (!m_jetUncertaintiesTool.isUserConfigured()) {
      std::string jetdef("AntiKt4" + xAOD::JetInput::typeName(xAOD::JetInput::Type(m_jetInputType)));

      if(jetdef !="AntiKt4EMPFlow"){
        ATH_MSG_WARNING("Jet Uncertaintes recommendations only exist for PFlow jets, falling back to AntiKt4EMPFlow");
        jetdef = "AntiKt4EMPFlow";
      }
      toolName = "JetUncertaintiesTool_" + jetdef;

      if (m_isRun3 && isAtlfast()) {
          ATH_MSG_WARNING("Jet Uncertaintes pre-recommendations for Run3 only exist for full sim");
      }

      m_jetUncertaintiesTool.setTypeAndName("JetUncertaintiesTool/"+toolName);

      ATH_CHECK( m_jetUncertaintiesTool.setProperty("JetDefinition", jetdef) );
      ATH_CHECK( m_jetUncertaintiesTool.setProperty("MCType", m_jetUncertaintiesMCType) );
      ATH_CHECK( m_jetUncertaintiesTool.setProperty("IsData", false) ); // Never use the PDSmearing for the nominal tool.
      ATH_CHECK( m_jetUncertaintiesTool.setProperty("ConfigFile", m_jetUncertaintiesConfig) );
      if(m_jetUncertaintiesAnalysisFile!="default") ATH_CHECK( m_jetUncertaintiesTool.setProperty("AnalysisFile", m_jetUncertaintiesAnalysisFile) );
      if (m_jetUncertaintiesCalibArea != "default") ATH_CHECK( m_jetUncertaintiesTool.setProperty("CalibArea", m_jetUncertaintiesCalibArea) );
      ATH_CHECK( m_jetUncertaintiesTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetUncertaintiesTool.retrieve() );
    } else  ATH_CHECK( m_jetUncertaintiesTool.retrieve() );

    ATH_MSG_INFO("Set up Jet PD Smear Uncertainty tool...");

    if (!m_jetUncertaintiesPDSmearTool.isUserConfigured() && m_jetUncertaintiesPDsmearing == true) {
      std::string jetdef("AntiKt4" + xAOD::JetInput::typeName(xAOD::JetInput::Type(m_jetInputType)));

      if(jetdef !="AntiKt4EMPFlow"){
        ATH_MSG_WARNING("Jet Uncertaintes recommendations only exist for PFlow jets, falling back to AntiKt4EMPFlow");
        jetdef = "AntiKt4EMPFlow";
      }
      toolName = "JetUncertaintiesPDSmearTool_" + jetdef;

      m_jetUncertaintiesPDSmearTool.setTypeAndName("JetUncertaintiesTool/"+toolName);

      // If, for some reason, you're trying to use the PDSmear, with the reduced set return an error (you shouldn't do this, you're just going to duplicate the SimpleJER results.
      bool JERUncPDsmearing = isData() ? isData() : m_jetUncertaintiesPDsmearing;
      if (m_jetUncertaintiesConfig.find("SimpleJER") != std::string::npos && JERUncPDsmearing){
        ATH_MSG_ERROR("You are trying to use the SimpleJER set, with PDsmearing. There is no functionality for this. Please fix your config file. Either run with PDSmear set to false, or run with the AllJER or FullJER sets.");
        return StatusCode::FAILURE;
      }
      ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("JetDefinition", jetdef) );
      ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("MCType", m_jetUncertaintiesMCType) );
      ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("IsData", true) ); // Set to True by default for PDSmear-named tool.
      ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("ConfigFile", m_jetUncertaintiesConfig) );
      if (m_jetUncertaintiesCalibArea != "default") ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("CalibArea", m_jetUncertaintiesCalibArea) );
      ATH_CHECK( m_jetUncertaintiesPDSmearTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetUncertaintiesPDSmearTool.retrieve() );
    } else{
      ATH_MSG_DEBUG("Do not retrieve the jet PD Smearing tool if it is not configured");
      //ATH_CHECK( m_jetUncertaintiesPDSmearTool.retrieve() );
    }
  }

  if (m_slices["fjet"]) {
    ATH_MSG_INFO("Won't initialise jet uncertainty tool for fat jets until we get rec for UFO");
    // Won't initialise jet uncertainty tool for fat jets until we get rec for UFO
    /*
    if (!m_fatjetUncertaintiesTool.isUserConfigured() && !m_fatJets.empty() && !m_fatJetUncConfig.empty()) {

      toolName = "JetUncertaintiesTool_" + m_fatJets;
      m_fatjetUncertaintiesTool.setTypeAndName("JetUncertaintiesTool/"+toolName);

      ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("JetDefinition", fatjetcoll) );
      ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("MCType", "MC16") );
      ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("IsData", isData()) );
      ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("ConfigFile", m_fatJetUncConfig) );
      if (m_jetUncertaintiesCalibArea != "default") ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("CalibArea", m_jetUncertaintiesCalibArea) );

      //Restrict variables to be shifted if (required)
      if( m_fatJetUncVars != "default" ){
        std::vector<std::string> shift_vars = {};

        std::string temp(m_fatJetUncVars);
        do {
          auto pos = temp.find(',');
          shift_vars.push_back(temp.substr(0, pos));
          if (pos == std::string::npos)
            temp = "";
          else
            temp = temp.substr(pos + 1);

        }
        while (!temp.empty() );

        ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("VariablesToShift", shift_vars) );
      }

      ATH_CHECK( m_fatjetUncertaintiesTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_fatjetUncertaintiesTool.retrieve() );
    } else if (m_fatjetUncertaintiesTool.isUserConfigured()) ATH_CHECK(m_fatjetUncertaintiesTool.retrieve());
    */
    ATH_MSG_INFO(" Won't initialise Wtagger uncertainty tool for fat jets until we get rec for UFO");
    // Won't initialise Wtagger uncertainty tool for fat jets until we get rec for UFO
    /*
    if (!m_WTagjetUncertaintiesTool.isUserConfigured() && !m_fatJets.empty() && !m_WtagConfig.empty() && !m_WTagUncConfig.empty()) {

      toolName = "WTagJetUncertaintiesTool_" + m_fatJets;
      m_WTagjetUncertaintiesTool.setTypeAndName("JetUncertaintiesTool/"+toolName);
      ATH_CHECK( m_WTagjetUncertaintiesTool.setProperty("JetDefinition", fatjetcoll) );
      ATH_CHECK( m_WTagjetUncertaintiesTool.setProperty("MCType", "MC16") );
      ATH_CHECK( m_WTagjetUncertaintiesTool.setProperty("IsData", isData()) );
      ATH_CHECK( m_WTagjetUncertaintiesTool.setProperty("ConfigFile", "rel21/Fall2020/"+m_WTagUncConfig) );
      ATH_CHECK( m_WTagjetUncertaintiesTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_WTagjetUncertaintiesTool.retrieve() );
    } else if (m_WTagjetUncertaintiesTool.isUserConfigured()) ATH_CHECK(m_WTagjetUncertaintiesTool.retrieve());
    */
    ATH_MSG_INFO(" Won't initialise Ztagger uncertainty tool for fat jets until we get rec for UFO");
    // Won't initialise Ztagger uncertainty tool for fat jets until we get rec for UFO
    /*
    if (!m_ZTagjetUncertaintiesTool.isUserConfigured() && !m_fatJets.empty() && !m_ZtagConfig.empty() && !m_ZTagUncConfig.empty()) {

      toolName = "ZTagJetUncertaintiesTool_" + m_fatJets;
      m_ZTagjetUncertaintiesTool.setTypeAndName("JetUncertaintiesTool/"+toolName);
      ATH_CHECK( m_ZTagjetUncertaintiesTool.setProperty("JetDefinition", fatjetcoll) );
      ATH_CHECK( m_ZTagjetUncertaintiesTool.setProperty("ConfigFile", "rel21/Fall2020/"+m_ZTagUncConfig) );
      ATH_CHECK( m_ZTagjetUncertaintiesTool.setProperty("MCType", "MC16") );
      ATH_CHECK( m_ZTagjetUncertaintiesTool.setProperty("IsData", isData()) );
      ATH_CHECK( m_ZTagjetUncertaintiesTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_ZTagjetUncertaintiesTool.retrieve() );
    } else if (m_ZTagjetUncertaintiesTool.isUserConfigured()) ATH_CHECK(m_ZTagjetUncertaintiesTool.retrieve());
    */
    ATH_MSG_INFO(" Won't initialise top-tagger uncertainty tool for fat jets until we get rec for UFO");
    // Won't initialise top-tagger uncertainty tool for fat jets until we get rec for UFO
    /*
    if (!m_TopTagjetUncertaintiesTool.isUserConfigured() && !m_fatJets.empty() && !m_ToptagConfig.empty() && !m_TopTagUncConfig.empty()) {

      toolName = "TopTagJetUncertaintiesTool_" + m_fatJets;
      m_TopTagjetUncertaintiesTool.setTypeAndName("JetUncertaintiesTool/"+toolName);
      ATH_CHECK( m_TopTagjetUncertaintiesTool.setProperty("JetDefinition", fatjetcoll) );
      ATH_CHECK( m_TopTagjetUncertaintiesTool.setProperty("MCType", "MC16") );
      ATH_CHECK( m_TopTagjetUncertaintiesTool.setProperty("IsData", isData()) );
      ATH_CHECK( m_TopTagjetUncertaintiesTool.setProperty("ConfigFile", "rel21/Fall2020/"+m_TopTagUncConfig) );
      ATH_CHECK( m_TopTagjetUncertaintiesTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_TopTagjetUncertaintiesTool.retrieve() );
    } else if (m_TopTagjetUncertaintiesTool.isUserConfigured()) ATH_CHECK(m_TopTagjetUncertaintiesTool.retrieve());
    */
  }


  // tagger SF and uncertainties
  // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BoostedJetTaggingRecommendationFullRun2
  // To be implemented here

  if (m_slices["jet"]) {
    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise jet cleaning tools

    // see https://twiki.cern.ch/twiki/bin/view/AtlasProtected/HowToCleanJetsR22
    if (m_badJetCut!="" && !m_jetCleaningTool.isUserConfigured()) {
      toolName = "JetCleaningTool";
      m_jetCleaningTool.setTypeAndName("JetCleaningTool/"+toolName);
      ATH_CHECK( m_jetCleaningTool.setProperty("CutLevel", m_badJetCut) );
      ATH_CHECK( m_jetCleaningTool.setProperty("DoUgly", false) );
      ATH_CHECK( m_jetCleaningTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetCleaningTool.retrieve() );
    } else if (m_jetCleaningTool.isUserConfigured()) ATH_CHECK( m_jetCleaningTool.retrieve() );

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise jet pileup labeling tool (required for labels used by JvtEfficiencyTools)

    if(!m_jetPileupLabelingTool.isUserConfigured() && !isData()) {
      toolName = "PileupLabelingTool";
      m_jetPileupLabelingTool.setTypeAndName("JetPileupLabelingTool/"+toolName);
      ATH_CHECK( m_jetPileupLabelingTool.setProperty("RecoJetContainer", m_defaultJets) );
      ATH_CHECK( m_jetPileupLabelingTool.setProperty("TruthJetContainer", m_defaultTruthJets) );
  #ifndef XAOD_STANDALONE
      ATH_CHECK( m_jetPileupLabelingTool.setProperty("SuppressInputDependence", true) );
      ATH_CHECK( m_jetPileupLabelingTool.setProperty("SuppressOutputDependence", true) );
  #endif
      ATH_CHECK( m_jetPileupLabelingTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetPileupLabelingTool.retrieve() );  
    } else if (m_jetPileupLabelingTool.isUserConfigured()) ATH_CHECK( m_jetPileupLabelingTool.retrieve() );

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise jet Jvt moment tool

    if (!m_jetJvtMomentTool.isUserConfigured()) {
      toolName = "JvtMomentTool";
      m_jetJvtMomentTool.setTypeAndName("JetVertexTaggerTool/"+toolName);
      ATH_CHECK( m_jetJvtMomentTool.setProperty("JetContainer", m_defaultJets) );
  #ifndef XAOD_STANDALONE
      ATH_CHECK( m_jetJvtMomentTool.setProperty("SuppressInputDependence", true) );
      ATH_CHECK( m_jetJvtMomentTool.setProperty("SuppressOutputDependence", true) );
  #endif
      ATH_CHECK( m_jetJvtMomentTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetJvtMomentTool.retrieve() );
    } else if (m_jetJvtMomentTool.isUserConfigured()) ATH_CHECK( m_jetJvtMomentTool.retrieve() );

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise jet NNJvt moment tool

    m_applyJVTCut = !m_JvtWP.empty();
    if (!m_jetNNJvtMomentTool.isUserConfigured() && m_applyJVTCut) {
      toolName = "NNJvtMomentTool";
      m_jetNNJvtMomentTool.setTypeAndName("JetPileupTag::JetVertexNNTagger/"+toolName);
      ATH_CHECK( m_jetNNJvtMomentTool.setProperty("JetContainer", m_defaultJets) );
  #ifndef XAOD_STANDALONE
      ATH_CHECK( m_jetNNJvtMomentTool.setProperty("SuppressInputDependence", true) );
      ATH_CHECK( m_jetNNJvtMomentTool.setProperty("SuppressOutputDependence", true) );
  #endif
      ATH_CHECK( m_jetNNJvtMomentTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetNNJvtMomentTool.retrieve() );
    } else if (m_jetNNJvtMomentTool.isUserConfigured()) ATH_CHECK( m_jetNNJvtMomentTool.retrieve() );

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise jet NNJvt selection tool

    if (!m_jetNNJvtSelectionTool.isUserConfigured() && m_applyJVTCut) {
      toolName = "NNJvtSelectionTool";
      m_jetNNJvtSelectionTool.setTypeAndName("CP::NNJvtSelectionTool/"+toolName);
      ATH_CHECK( m_jetNNJvtSelectionTool.setProperty("JetContainer", m_defaultJets) );      
      ATH_CHECK( m_jetNNJvtSelectionTool.setProperty("WorkingPoint", m_JvtWP) );
      ATH_CHECK( m_jetNNJvtSelectionTool.setProperty("MaxPtForJvt", m_JvtPtMax) );
      ATH_CHECK( m_jetNNJvtSelectionTool.setProperty("JvtMomentName", "NNJvt") );
      ATH_CHECK( m_jetNNJvtSelectionTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetNNJvtSelectionTool.retrieve() );
    } else if (m_jetNNJvtSelectionTool.isUserConfigured()) ATH_CHECK( m_jetNNJvtSelectionTool.retrieve() );

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise jet NNJvt efficiency tool (scale factors)

    if (!m_jetNNJvtEfficiencyTool.isUserConfigured() && m_applyJVTCut) {
      toolName = "NNJvtEfficiencyTool";
      m_jetNNJvtEfficiencyTool.setTypeAndName("CP::NNJvtEfficiencyTool/"+toolName);
      ATH_CHECK( m_jetNNJvtEfficiencyTool.setProperty("JetContainer", m_defaultJets) );  
      ATH_CHECK( m_jetNNJvtEfficiencyTool.setProperty("WorkingPoint", m_JvtWP) );
      ATH_CHECK( m_jetNNJvtEfficiencyTool.setProperty("MaxPtForJvt", m_JvtPtMax) );
      ATH_CHECK( m_jetNNJvtEfficiencyTool.setProperty("SFFile", m_isRun3 ? m_JvtConfigRun3 : m_JvtConfigRun2) );
      ATH_CHECK( m_jetNNJvtEfficiencyTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetNNJvtEfficiencyTool.retrieve() );
    } else if (m_jetNNJvtEfficiencyTool.isUserConfigured()) ATH_CHECK( m_jetNNJvtEfficiencyTool.retrieve() );

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise jet fJvt selection tool

    if (!m_jetfJvtSelectionTool.isUserConfigured() && m_doFwdJVT) {
      toolName = "fJvtSelectionTool";
      m_jetfJvtSelectionTool.setTypeAndName("CP::FJvtSelectionTool/"+toolName);
      ATH_CHECK( m_jetfJvtSelectionTool.setProperty("JetContainer", m_defaultJets) );  
      ATH_CHECK( m_jetfJvtSelectionTool.setProperty("WorkingPoint", m_fJvtWP) );
      ATH_CHECK( m_jetfJvtSelectionTool.setProperty("MaxPtForJvt", m_fJvtPtMax) );
      ATH_CHECK( m_jetfJvtSelectionTool.setProperty("JvtMomentName", "DFCommonJets_fJvt") );
      ATH_CHECK( m_jetfJvtSelectionTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetfJvtSelectionTool.retrieve() );
    } else if (m_jetfJvtSelectionTool.isUserConfigured()) ATH_CHECK( m_jetfJvtSelectionTool.retrieve() );

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise jet fJvt efficiency tool for scale factors (scale factors)

    if (!m_jetfJvtEfficiencyTool.isUserConfigured() && m_doFwdJVT) {
      toolName = "fJvtEfficiencyTool";
      m_jetfJvtEfficiencyTool.setTypeAndName("CP::FJvtEfficiencyTool/"+toolName);
      ATH_CHECK( m_jetfJvtEfficiencyTool.setProperty("JetContainer", m_defaultJets) );  
      ATH_CHECK( m_jetfJvtEfficiencyTool.setProperty("WorkingPoint", m_fJvtWP) );
      ATH_CHECK( m_jetfJvtEfficiencyTool.setProperty("MaxPtForJvt", m_fJvtPtMax) );
      ATH_CHECK( m_jetfJvtEfficiencyTool.setProperty("SFFile", m_isRun3 ? m_fJvtConfigRun3 : m_fJvtConfigRun2) );
      ATH_CHECK( m_jetfJvtEfficiencyTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_jetfJvtEfficiencyTool.retrieve() );
    } else  ATH_CHECK( m_jetfJvtEfficiencyTool.retrieve() );
  }


  if (m_slices["mu"]) {
    ///////////////////////////////////////////////////////////////////////////////////////////
    // Check muon baseline ID

    std::string muQualBaseline = "";
    switch (m_muIdBaseline) {
    case xAOD::Muon::VeryLoose: muQualBaseline = "VeryLoose";
      ATH_MSG_WARNING("No muon scale factors are available for VeryLoose working point.");
      break;
    case xAOD::Muon::Loose:     muQualBaseline = "Loose";  break;
    case xAOD::Muon::Medium:    muQualBaseline = "Medium"; break;
    case xAOD::Muon::Tight:     muQualBaseline = "Tight";  break;
    case 4:                     muQualBaseline = "HighPt";  break;
    case 5:                     muQualBaseline = "LowPt";  break;
    case 6:                     muQualBaseline = "LowPtMVA"; break;
    case 7:                     muQualBaseline = "HighPt3Layers"; break;
    default:
      ATH_MSG_ERROR("Invalid muon working point provided: " << m_muIdBaseline << ". Cannot initialise!");
      return StatusCode::FAILURE;
      break;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise muon calibration tool
    // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MuonMomentumCorrectionsSubgroup#CP_MuonCalibrationAndSmearingToo
    // Please not that for AthAnalysis we need to configure the MuonCalibTool within METSignificance manually and just can't pass the handle
    // Hence, any changes here should also be propagated to the METSignificance setup further below

    if (!m_muonCalibTool.isUserConfigured()) {
      m_muonCalibTool.setTypeAndName("CP::MuonCalibTool/ST_MuonCalibrationTool");
      if (m_isRun3) ATH_CHECK( m_muonCalibTool.setProperty("IsRun3Geo", true ));
      ATH_CHECK( m_muonCalibTool.setProperty("calibMode", m_muCalibrationMode) );
      ATH_CHECK( m_muonCalibTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_muonCalibTool.setProperty("release", "Recs2024_05_06_Run2Run3" ));
      int IdBaselineInt = m_muIdBaseline;
      if (IdBaselineInt == 4) {
        ATH_CHECK( m_muonCalibTool.setProperty("do2StationsHighPt", true) );
      }
      ATH_CHECK( m_muonCalibTool.setProperty("doExtraSmearing", m_muHighPtExtraSmear) );
      ATH_CHECK( m_muonCalibTool.retrieve() );
    } else  ATH_CHECK( m_muonCalibTool.retrieve() );

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise muon selection tool

    if (!m_muonSelectionToolBaseline.isUserConfigured()) {
      toolName = "MuonSelectionTool_Baseline_" + muQualBaseline;
      m_muonSelectionToolBaseline.setTypeAndName("CP::MuonSelectionTool/"+toolName);
      if (m_isRun3) ATH_CHECK( m_muonSelectionToolBaseline.setProperty("IsRun3Geo", true ));

      if (m_muBaselineEta<m_muEta){  // Test for inconsistent configuration
        ATH_MSG_ERROR( "Requested a baseline eta cut for muons (" << m_muBaselineEta <<
                       ") that is tighter than the signal cut (" << m_muEta << ").  Please check your config." );
        return StatusCode::FAILURE;
      }
      int IdBaselineInt = m_muIdBaseline;
      ATH_CHECK( m_muonSelectionToolBaseline.setProperty( "MaxEta", m_muBaselineEta) );
      if (IdBaselineInt == 6){
          ATH_CHECK( m_muonSelectionToolBaseline.setProperty( "MuQuality", 5 ) );
          ATH_CHECK( m_muonSelectionToolBaseline.setProperty( "UseMVALowPt", true));
      } else if (IdBaselineInt == 7){
          ATH_CHECK( m_muonSelectionToolBaseline.setProperty( "MuQuality", 4 ) );
          ATH_CHECK( m_muonSelectionToolBaseline.setProperty( "Use2stationMuonsHighPt", false));
      } else ATH_CHECK(m_muonSelectionToolBaseline.setProperty( "MuQuality", m_muIdBaseline ));
      ATH_CHECK( m_muonSelectionToolBaseline.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_muonSelectionToolBaseline.retrieve() );
    } else ATH_CHECK( m_muonSelectionToolBaseline.retrieve() );


    std::string muQual = "";
    switch (m_muId) {
    case xAOD::Muon::VeryLoose: muQual = "VeryLoose";
      ATH_MSG_WARNING("No muon scale factors are available for VeryLoose working point.");
      break;
    case xAOD::Muon::Loose:     muQual = "Loose";  break;
    case xAOD::Muon::Medium:    muQual = "Medium"; break;
    case xAOD::Muon::Tight:     muQual = "Tight";  break;
    case 4:                     muQual = "HighPt";  break;
    case 5:                     muQual = "LowPt";  break;
    case 6:                     muQual = "LowPtMVA"; break;
    case 7:                     muQual = "HighPt3Layers"; break;
    default:
      ATH_MSG_ERROR("Invalid muon working point provided: " << m_muId << ". Cannot initialise!");
      return StatusCode::FAILURE;
      break;
    }

    if (!m_muonSelectionTool.isUserConfigured()) {
      toolName = "MuonSelectionTool_" + muQual;
      m_muonSelectionTool.setTypeAndName("CP::MuonSelectionTool/"+toolName);
      if (m_isRun3) ATH_CHECK( m_muonSelectionTool.setProperty("IsRun3Geo", true ));
      ATH_CHECK( m_muonSelectionTool.setProperty( "MaxEta", m_muEta) );
      int IdInt = m_muId;
      if (IdInt == 6){
          ATH_CHECK( m_muonSelectionTool.setProperty( "MuQuality", 5 ) );
          ATH_CHECK( m_muonSelectionTool.setProperty( "UseMVALowPt", true));
      } else if (IdInt == 7){
          ATH_CHECK( m_muonSelectionTool.setProperty( "MuQuality", 4 ) );
          ATH_CHECK( m_muonSelectionTool.setProperty( "Use2stationMuonsHighPt", false));
      } else ATH_CHECK(m_muonSelectionTool.setProperty( "MuQuality", m_muId ));
      ATH_CHECK( m_muonSelectionTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_muonSelectionTool.retrieve() );
    } else ATH_CHECK( m_muonSelectionTool.retrieve() );


    if (!m_muonSelectionHighPtTool.isUserConfigured()) { //Fixed to HighPt WP
      toolName = "MuonSelectionHighPtTool_" + muQual;
      m_muonSelectionHighPtTool.setTypeAndName("CP::MuonSelectionTool/"+toolName);
      if (m_isRun3) ATH_CHECK( m_muonSelectionHighPtTool.setProperty("IsRun3Geo", true ));
      ATH_CHECK( m_muonSelectionHighPtTool.setProperty( "MaxEta", m_muEta) );
      ATH_CHECK( m_muonSelectionHighPtTool.setProperty( "MuQuality", 4 ) );
      ATH_CHECK( m_muonSelectionHighPtTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_muonSelectionHighPtTool.retrieve() );
    } else  ATH_CHECK( m_muonSelectionHighPtTool.retrieve() );


    //////////////
    // Initialise prompt/LRT muon OR tool
    if (!m_muonLRTORTool.isUserConfigured()) {
        toolName = "MuonLRTOverlapRemovalTool";
        m_muonLRTORTool.setTypeAndName("CP::MuonLRTOverlapRemovalTool/"+toolName);
        ATH_CHECK( m_muonLRTORTool.setProperty("overlapStrategy", CP::IMuonLRTOverlapRemovalTool::defaultStrategy) );
        if (m_isRun3) ATH_CHECK( m_muonLRTORTool.setProperty("UseRun3WP", true ));
        ATH_CHECK( m_muonLRTORTool.setProperty("OutputLevel", this->msg().level()) );
        ATH_CHECK( m_muonLRTORTool.retrieve() );
    } else ATH_CHECK( m_muonLRTORTool.retrieve() );

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise muon efficiency tools
    if (!m_muonEfficiencySFTool.isUserConfigured() && m_muId != xAOD::Muon::VeryLoose && !isData()) {
      toolName = "MuonEfficiencyScaleFactors_" + muQual;
      m_muonEfficiencySFTool.setTypeAndName("CP::MuonEfficiencyScaleFactors/"+toolName);
      ATH_CHECK( m_muonEfficiencySFTool.setProperty("WorkingPoint", muQual) );
      ATH_CHECK( m_muonEfficiencySFTool.setProperty("CalibrationRelease", m_isRun3? "240711_Preliminary_r24run3":"230213_Preliminary_r22run2") );
      ATH_CHECK( m_muonEfficiencySFTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_muonEfficiencySFTool.retrieve() );
    } else if (m_muonEfficiencySFTool.isUserConfigured()) ATH_CHECK( m_muonEfficiencySFTool.retrieve() );


    if (!m_muonEfficiencyBMHighPtSFTool.isUserConfigured() && !isData()){
      toolName = "MuonEfficiencyScaleFactorsBMHighPt_" + muQual;
      m_muonEfficiencyBMHighPtSFTool.setTypeAndName("CP::MuonEfficiencyScaleFactors/"+toolName);
      ATH_CHECK( m_muonEfficiencyBMHighPtSFTool.setProperty("WorkingPoint", "BadMuonVeto_HighPt") );
      ATH_CHECK( m_muonEfficiencyBMHighPtSFTool.setProperty("CalibrationRelease", m_isRun3? "220817_Preliminary_r22run3":"230213_Preliminary_r22run2") ); //BadMuonVeto_HighPt currently not available for 240711_Preliminary_r24run3
      ATH_CHECK( m_muonEfficiencyBMHighPtSFTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_muonEfficiencyBMHighPtSFTool.retrieve() );
    } else if (m_muonEfficiencyBMHighPtSFTool.isUserConfigured()) ATH_CHECK( m_muonEfficiencyBMHighPtSFTool.retrieve() );


    if (m_doTTVAsf && m_mud0sig<0 && m_muz0<0){
      ATH_MSG_WARNING("Requested TTVA SFs without d0sig and z0 cuts. Disabling scale factors as they will not make sense.");
      m_doTTVAsf=false;
    }

    if (m_doTTVAsf && !m_muonTTVAEfficiencySFTool.isUserConfigured() && !isData()) {
      toolName = "MuonTTVAEfficiencyScaleFactors";
      m_muonTTVAEfficiencySFTool.setTypeAndName("CP::MuonEfficiencyScaleFactors/"+toolName);
      ATH_CHECK( m_muonTTVAEfficiencySFTool.setProperty("WorkingPoint", "TTVA") );
      ATH_CHECK( m_muonTTVAEfficiencySFTool.setProperty("CalibrationRelease", m_isRun3? "240711_Preliminary_r24run3":"230213_Preliminary_r22run2") );
      ATH_CHECK( m_muonTTVAEfficiencySFTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_muonTTVAEfficiencySFTool.retrieve() );
    } else if (m_muonTTVAEfficiencySFTool.isUserConfigured()) ATH_CHECK( m_muonTTVAEfficiencySFTool.retrieve() );



    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise muon isolation tool
    if (!m_muonIsolationSFTool.isUserConfigured() && !m_muIso_WP.empty() && !isData()) {
      toolName = "MuonIsolationScaleFactors_" + m_muIso_WP;

      std::string tmp_muIso_WP = m_muIso_WP;
      if ( !check_isOption(m_muIso_WP, m_mu_iso_support) ) { //check if supported
        ATH_MSG_WARNING("Your selected muon Iso WP ("
          << m_muIso_WP
          << ") does not have SFs defined. Will try to find an appropriate fall-back.");
        if (m_mu_iso_fallback.count(m_muIso_WP) > 0){
          tmp_muIso_WP = m_mu_iso_fallback[m_muIso_WP];
          ATH_MSG_WARNING("Your selected muon Iso WP ("
            << m_muIso_WP
            << " is not supported, and does not have SFs available.  Falling back to "
            << tmp_muIso_WP
            << " for SF determination.");
        } else {
          ATH_MSG_ERROR("***  The muon isolation WP you selected (" << m_muIso_WP << ") is not currentely supported, and no known fall-back option for SFs exists. Sorry! ***");
          return StatusCode::FAILURE;
        }
      }

      m_muonIsolationSFTool.setTypeAndName("CP::MuonEfficiencyScaleFactors/"+toolName);
      ATH_CHECK( m_muonIsolationSFTool.setProperty("WorkingPoint", tmp_muIso_WP + "Iso") );
      ATH_CHECK( m_muonIsolationSFTool.setProperty("CalibrationRelease", m_isRun3? "240711_Preliminary_r24run3":"230213_Preliminary_r22run2") );
      ATH_CHECK( m_muonIsolationSFTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_muonIsolationSFTool.retrieve() );

    } else if (m_muonIsolationSFTool.isUserConfigured()) ATH_CHECK( m_muonIsolationSFTool.retrieve() );


    if (!m_muonHighPtIsolationSFTool.isUserConfigured() && !m_muIsoHighPt_WP.empty() && !isData()) {
      toolName = "MuonHighPtIsolationScaleFactors_" + m_muIsoHighPt_WP;

      std::string tmp_muIsoHighPt_WP = m_muIsoHighPt_WP;
      if ( !check_isOption(m_muIsoHighPt_WP, m_mu_iso_support) ) { //check if supported
        ATH_MSG_WARNING("Your selected muon high-pt Iso WP ("
          << m_muIsoHighPt_WP
          << ") does not have SFs defined. Will try to find an appropriate fall-back.");
        if (m_mu_iso_fallback.count(m_muIsoHighPt_WP) > 0){
          tmp_muIsoHighPt_WP = m_mu_iso_fallback[m_muIsoHighPt_WP];
          ATH_MSG_WARNING("Your selected muon high-pt Iso WP ("
            << m_muIsoHighPt_WP
            << " is not supported, and does not have SFs available.  Falling back to "
            << tmp_muIsoHighPt_WP
            << " for SF determination.");
        } else {
          ATH_MSG_ERROR("***  The muon isolation WP you selected (" << m_muIsoHighPt_WP << ") is not currentely supported, and no known fall-back option for SFs exists. Sorry! ***");
          return StatusCode::FAILURE;
        }
      }

      m_muonHighPtIsolationSFTool.setTypeAndName("CP::MuonEfficiencyScaleFactors/"+toolName);
      // Use for the low-pt WP a dedicated set of isolation scale-factors having an extra uncertainty in place
      ATH_CHECK( m_muonHighPtIsolationSFTool.setProperty("WorkingPoint", tmp_muIsoHighPt_WP + "Iso") );
      ATH_CHECK( m_muonHighPtIsolationSFTool.setProperty("CalibrationRelease", m_isRun3? "240711_Preliminary_r24run3":"230213_Preliminary_r22run2") );
      ATH_CHECK( m_muonHighPtIsolationSFTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_muonHighPtIsolationSFTool.retrieve() );

    } else if (m_muonHighPtIsolationSFTool.isUserConfigured()) ATH_CHECK( m_muonHighPtIsolationSFTool.retrieve() );


    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise muon trigger scale factor tools

    if (!m_muonTriggerSFTool.isUserConfigured() && !isData()) {
      toolName = "MuonTriggerScaleFactors_" + muQual;
      m_muonTriggerSFTool.setTypeAndName("CP::MuonTriggerScaleFactors/"+toolName);
      if ( muQual=="LowPt" ) {
        ATH_MSG_WARNING("You're using the LowPt muon selection, which is not supported yet in terms of muon trigger scale facorts. TEMPORAIRLY configuring the muonTriggerSFTool for Medium muons. Beware!");
        ATH_CHECK( m_muonTriggerSFTool.setProperty("MuonQuality", "Medium" ) );
      }
      else ATH_CHECK( m_muonTriggerSFTool.setProperty("MuonQuality", muQual) );
      //ATH_CHECK( m_muonTriggerSFTool.setProperty("Isolation", m_muIso_WP)); This property has been depreacted long time ago
      ATH_CHECK( m_muonTriggerSFTool.setProperty("AllowZeroSF", true) );
      ATH_CHECK( m_muonTriggerSFTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_muonTriggerSFTool.retrieve() );
      m_muonTrigSFTools.push_back(m_muonTriggerSFTool.getHandle());
    } else if (m_muonTriggerSFTool.isUserConfigured()) {
      ATH_CHECK( m_muonTriggerSFTool.retrieve() );
      m_muonTrigSFTools.push_back(m_muonTriggerSFTool.getHandle());
    }
  }

  if (m_slices["ele"]) {
    // /////////////////////////////////////////////////////////////////////////////////////////
    // Initialise electron selector tools

    // Signal Electrons
    if (!m_elecSelLikelihood.isUserConfigured()) {
      toolName = "EleSelLikelihood_" + m_eleId;

      if (m_eleId.find("DNN") != std::string::npos) {
        m_elecSelLikelihood.setTypeAndName("AsgElectronSelectorTool/"+toolName);
      }
      else {
        m_elecSelLikelihood.setTypeAndName("AsgElectronLikelihoodTool/"+toolName);
      }

      if (! m_eleConfig.empty() ){
        ATH_MSG_INFO("Overriding specified Ele.Id working point in favour of configuration file");
        ATH_CHECK( m_elecSelLikelihood.setProperty("ConfigFile", m_eleConfig) );
      } else if ( !check_isOption(m_eleId, m_el_id_support) ) { //check if supported
        ATH_MSG_ERROR("Invalid electron ID selected: " << m_eleId);
        return StatusCode::FAILURE;
      }
      else if (m_eleId == "VeryLooseLLH" || m_eleId == "LooseLLH") {
        ATH_MSG_WARNING(" ****************************************************************************");
        ATH_MSG_WARNING(" CAUTION: Setting " << m_eleId << " as signal electron ID");
        ATH_MSG_WARNING(" These may be used for loose electron CRs but no scale factors are provided.");
        ATH_MSG_WARNING(" ****************************************************************************");
        ATH_CHECK( m_elecSelLikelihood.setProperty("WorkingPoint", EG_WP(m_eleId) ));
      } else {
        ATH_CHECK( m_elecSelLikelihood.setProperty("WorkingPoint", EG_WP(m_eleId) ));
      }

      ATH_CHECK( m_elecSelLikelihood.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_elecSelLikelihood.retrieve() );
    } else  ATH_CHECK( m_elecSelLikelihood.retrieve() );

    // Baseline Electrons
    if (!m_elecSelLikelihoodBaseline.isUserConfigured()) {
      toolName = "EleSelLikelihoodBaseline_" + m_eleIdBaseline;

      if (m_eleIdBaseline.find("DNN") != std::string::npos) {
        m_elecSelLikelihoodBaseline.setTypeAndName("AsgElectronSelectorTool/"+toolName);
      }
      else {
        m_elecSelLikelihoodBaseline.setTypeAndName("AsgElectronLikelihoodTool/"+toolName);
      }

      if (! m_eleConfigBaseline.empty() ){
        ATH_MSG_INFO("Overriding specified EleBaseline.Id working point in favour of configuration file");
        ATH_CHECK( m_elecSelLikelihoodBaseline.setProperty("ConfigFile", m_eleConfigBaseline ));
      } else if ( !check_isOption(m_eleIdBaseline, m_el_id_support) ) { //check if supported
        ATH_MSG_ERROR("Invalid electron ID selected: " << m_eleIdBaseline);
        return StatusCode::FAILURE;
      } else {
        ATH_CHECK( m_elecSelLikelihoodBaseline.setProperty("WorkingPoint", EG_WP(m_eleIdBaseline)) );
      }

      ATH_CHECK( m_elecSelLikelihoodBaseline.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_elecSelLikelihoodBaseline.retrieve() );
    } else ATH_CHECK( m_elecSelLikelihoodBaseline.retrieve() );
  }

    //////////////
    // Initialise prompt/LRT electron OR tool
    if (!m_elecLRTORTool.isUserConfigured()) {
        toolName = "ElectronLRTOverlapRemovalTool";
        m_elecLRTORTool.setTypeAndName("CP::ElectronLRTOverlapRemovalTool/"+toolName);
        ATH_MSG_DEBUG("Setting ElectronLRTOverlapRemovalTool strategy to "<<m_eleLRT_strat);
        ATH_CHECK( m_elecLRTORTool.setProperty("overlapStrategy", m_eleLRT_strat) );
        ATH_CHECK( m_elecLRTORTool.retrieve() );
    } else ATH_CHECK( m_elecLRTORTool.retrieve() );

  if (m_slices["pho"]) {
    // /////////////////////////////////////////////////////////////////////////////////////////
    // Initialise photon selector tools

    if (!m_photonSelIsEM.isUserConfigured()) {
      toolName = "PhotonSelIsEM_" + m_photonId;
      m_photonSelIsEM.setTypeAndName("AsgPhotonIsEMSelector/"+toolName);

      if (!check_isOption(m_photonId, m_ph_id_support)){ //check if supported
        ATH_MSG_ERROR("Invalid photon ID selected: " << m_photonId);
        return StatusCode::FAILURE;
      }

      ATH_CHECK( m_photonSelIsEM.setProperty("WorkingPoint", m_photonId+"Photon") );
      ATH_CHECK( m_photonSelIsEM.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_photonSelIsEM.retrieve() );
    } else ATH_CHECK( m_photonSelIsEM.retrieve() );

    if (!m_photonSelIsEMBaseline.isUserConfigured()) {
      toolName = "PhotonSelIsEMBaseline_" + m_photonIdBaseline;
      m_photonSelIsEMBaseline.setTypeAndName("AsgPhotonIsEMSelector/"+toolName);

      if(!check_isOption(m_photonIdBaseline, m_ph_id_support)){ //check if supported
        ATH_MSG_ERROR("Invalid photon ID selected: " << m_photonIdBaseline);
        return StatusCode::FAILURE;
      }

      ATH_CHECK( m_photonSelIsEMBaseline.setProperty("WorkingPoint", m_photonIdBaseline+"Photon") );
      ATH_CHECK( m_photonSelIsEMBaseline.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_photonSelIsEMBaseline.retrieve() );
    } else  ATH_CHECK( m_photonSelIsEMBaseline.retrieve() );
  }

  if (m_slices["ele"]||m_slices["pho"]) {
    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise DeadHVCellRemovalTool
    // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/EGammaIdentificationRun2#Removal_of_Electron_Photon_clust

    ATH_MSG_DEBUG("Setup AsgDeadHVCellRemovalTool/deadHVTool");
    m_deadHVTool.setTypeAndName("AsgDeadHVCellRemovalTool/deadHVTool");
    ATH_CHECK(m_deadHVTool.retrieve());
  }

  if (m_slices["ele"]) {
    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise electron efficiency tool

    PATCore::ParticleDataType::DataType data_type(PATCore::ParticleDataType::Data);
    if (!isData()) {
      if (isAtlfast()) data_type = PATCore::ParticleDataType::Fast;
      else data_type = PATCore::ParticleDataType::Full;
      ATH_MSG_DEBUG( "Setting data type to " << data_type);
    }

    m_eleEffMapFilePathRun2 = "ElectronEfficiencyCorrection/2015_2018/rel21.2/Precision_Summer2020_v1/map4.txt";

    if(!m_isRun3) {
      ATH_MSG_INFO( "Running on Run2 samples; Using egamma SF tools from R21 map ");
      ATH_MSG_INFO( m_eleEffMapFilePathRun2);
    }

    toolName = "AsgElectronEfficiencyCorrectionTool_reco";
    CONFIG_EG_EFF_TOOL_KEY(m_elecEfficiencySFTool_reco, toolName, "RecoKey", "Reconstruction", m_isRun3? m_eleEffMapFilePath : m_eleEffMapFilePathRun2);

    //-- get KEYS supported by egamma SF tools
    std::vector<std::string> eSF_keys = getElSFkeys( m_isRun3? m_eleEffMapFilePath : m_eleEffMapFilePathRun2);

    if (m_eleId.find("NoPix") != std::string::npos || m_eleId == "VeryLooseLLH" || m_eleId == "LooseLLH" || m_eleId == "Medium") {
      ATH_MSG_WARNING("Not configuring electron ID and trigger scale factors for " << m_eleId);
    }
    else {
      // This needs to be formatted for the scale factors: no _Rel20, no LH label, etc.
      std::string eleId = TString(m_eleId).ReplaceAll("AndBLayer", "BLayer").ReplaceAll("LLH", "").Data();

      if (m_eleId.find("DNN") != std::string::npos) {
        eleId = TString(eleId).ReplaceAll("DNNnoCF", "").ReplaceAll("DNN", "").ReplaceAll("Loose", "LooseBLayer").Data();
        ATH_MSG_WARNING("Electron DNN ID working point " << m_eleId <<  " doesn't have SFs yet, fall back to " << eleId);
      }

      // electron id
      toolName = "AsgElectronEfficiencyCorrectionTool_id_" + m_eleId;
      CONFIG_EG_EFF_TOOL_KEY(m_elecEfficiencySFTool_id, toolName, "IdKey", eleId, m_isRun3? m_eleEffMapFilePath : m_eleEffMapFilePathRun2);

      // override map file use if correction file list is set for WP
      std::map<std::string,std::string> corrFNList;
      if ( !m_EG_corrFNList.empty() ) {
         for ( const auto& WP_fname : split( m_EG_corrFNList, "," ) ) {
            std::string WP = WP_fname.substr(0,WP_fname.find(":"));
            std::string fname = WP_fname.substr(WP_fname.find(":")+1);
            corrFNList[WP] = fname;
            ATH_MSG_WARNING( "Correction file list defined for WP " << WP << ": " << fname << "." );
            ATH_MSG_WARNING( "Will use correction file rather than central map file." );
         }
      }

      // electron iso
      std::string EleIso("");
      if (std::find(eSF_keys.begin(), eSF_keys.end(), eleId+"_"+m_eleIso_WP) != eSF_keys.end()){
        EleIso   = m_eleIso_WP;
      } else if (std::find(eSF_keys.begin(), eSF_keys.end(), eleId+"_"+m_el_iso_fallback[m_eleIso_WP]) != eSF_keys.end()){
        //--- Check to see if the only issue is an unknown isolation working point
        EleIso = m_el_iso_fallback[m_eleIso_WP];
        ATH_MSG_WARNING("(AsgElectronEfficiencyCorrectionTool_iso_*) Your selected electron Iso WP ("
          << m_eleIso_WP
          << ") does not have iso SFs defined. Falling back to "
          << m_el_iso_fallback[m_eleIso_WP]
          << " for SF calculations");
      }
      else{
        ATH_MSG_ERROR("***  THE ELECTRON ISOLATION SF YOU SELECTED (" << m_eleIso_WP << ") GOT NO SUPPORT ***");
        return StatusCode::FAILURE;
      }

      toolName = "AsgElectronEfficiencyCorrectionTool_iso_" + m_eleId + EleIso;

      // if running with correction file list
      if ( (!m_EG_corrFNList.empty()) && corrFNList.find(EleIso)!=corrFNList.end() ) {                // overriding central map file
        CONFIG_EG_EFF_TOOL( m_elecEfficiencySFTool_iso, toolName, corrFNList[EleIso] );
      }
      // can't do the iso tool via the macro, it needs two properties set
      else {                                                                                          // default: use map file
        if ( !m_elecEfficiencySFTool_iso.isUserConfigured() ) {
          if ( !check_isOption(EleIso, m_el_iso_support) ) { //check if supported
            ATH_MSG_WARNING( "(" << toolName << ") Your electron Iso WP: " << EleIso<< " is no longer supported. This will almost certainly cause a crash now.");
          }

          m_elecEfficiencySFTool_iso.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+toolName);

          if ( m_EG_corrFNList.empty() ) {
            ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("MapFilePath", m_isRun3? m_eleEffMapFilePath : m_eleEffMapFilePathRun2) );
          } else {
            ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("CorrectionFileNameList", corrFNList) );
          }
          ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("IdKey", eleId) );
          ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("IsoKey", EleIso) );
          if (!isData() && ((EleIso.find("TightTrackOnly_VarRad")!=std::string::npos)||
                            (EleIso.find("TightTrackOnly_FixedRad")!=std::string::npos)||
                            (EleIso.find("Tight_VarRad")!=std::string::npos)||
                            (EleIso.find("Loose_VarRad")!=std::string::npos))) {
            if (isAtlfast()) ATH_MSG_WARNING("(AsgElectronEfficiencyCorrectionTool/"+toolName+"). Your selected electron Iso WP (" + EleIso + ") don't have AFII SF. Falling back to FullSim");
            ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("ForceDataType", static_cast<int>(PATCore::ParticleDataType::Full)));
          }
          else if (!isData()){
            ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("ForceDataType", static_cast<int>(data_type)));
          }
          ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("CorrelationModel", m_EG_corrModel) );
          ATH_CHECK( m_elecEfficiencySFTool_iso.setProperty("OutputLevel", this->msg().level()) );
          ATH_CHECK( m_elecEfficiencySFTool_iso.initialize() );
        } else ATH_CHECK( m_elecEfficiencySFTool_iso.initialize() );
      }

      // electron iso high-pt
      std::string EleIsohighPt("");
      if (std::find(eSF_keys.begin(), eSF_keys.end(), eleId+"_"+m_eleIsoHighPt_WP) != eSF_keys.end()){
        EleIsohighPt   = m_eleIsoHighPt_WP;
      } else if (std::find(eSF_keys.begin(), eSF_keys.end(), eleId+"_"+m_el_iso_fallback[m_eleIsoHighPt_WP]) != eSF_keys.end()){
        //--- Check to see if the only issue is an unknown isolation working point
        EleIsohighPt = m_el_iso_fallback[m_eleIsoHighPt_WP];
        ATH_MSG_WARNING("(AsgElectronEfficiencyCorrectionTool_iso_*) Your selected high-pT electron Iso WP ("
          << m_eleIsoHighPt_WP
          << ") does not have iso SFs defined. Falling back to "
          << m_el_iso_fallback[m_eleIsoHighPt_WP]
          << " for SF calculations");
      }
      else{
        ATH_MSG_ERROR("***  THE HIGH PT ELECTRON ISOLATION SF YOU SELECTED (" << m_eleIsoHighPt_WP << ") GOT NO SUPPORT");
        return StatusCode::FAILURE;
      }

      toolName = "AsgElectronEfficiencyCorrectionTool_isoHigPt_" + m_eleId + EleIsohighPt;

      // if running with correction file list
      if ( (!m_EG_corrFNList.empty()) && corrFNList.find(EleIsohighPt)!=corrFNList.end() ) {                // overriding central map file
        CONFIG_EG_EFF_TOOL( m_elecEfficiencySFTool_isoHighPt, toolName, corrFNList[EleIsohighPt] );
      }
      // can't do the iso tool via the macro, it needs two properties set
      else {                                                                                                     // default: use map file
        if ( !m_elecEfficiencySFTool_isoHighPt.isUserConfigured() ) {

          if ( !check_isOption(EleIsohighPt, m_el_iso_support) ) { //check if supported
            ATH_MSG_WARNING( "(" << toolName << ") Your electron high-pt Iso WP: " << EleIsohighPt << " is no longer supported. This will almost certainly cause a crash now.");
          }

          m_elecEfficiencySFTool_isoHighPt.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+toolName);

          ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.setProperty("MapFilePath", m_isRun3? m_eleEffMapFilePath : m_eleEffMapFilePathRun2) );
          ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.setProperty("IdKey", eleId) );
          ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.setProperty("IsoKey", EleIsohighPt) );
          if (!isData()) {
            ATH_CHECK (m_elecEfficiencySFTool_isoHighPt.setProperty("ForceDataType", (int) data_type) );
          }
          ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.setProperty("CorrelationModel", m_EG_corrModel) );
          ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.setProperty("OutputLevel", this->msg().level()) );
          ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.initialize() );
        } else   ATH_CHECK( m_elecEfficiencySFTool_isoHighPt.initialize() );
      }

      // electron triggers - first SFs (but we need to massage the id string since all combinations are not supported)
  
      //single lepton
      
      if(m_isRun3 && eleId.find("LooseBLayer") != std::string::npos) ATH_MSG_WARNING("Replacing 'LooseBLayer' with 'Loose' for Electron ID while configuring single-ele trigger SF using: " << m_eleEffMapFilePath);
      std::string triggerEleID = m_isRun3? TString(eleId).ReplaceAll("LooseBLayer", "Loose").Data() : eleId;
      std::string triggerEleIso= m_eleIso_WP;
      
      // This is an hack to work with ElectronEfficiencyCorrection/2015_2025/rel22.2/2022_Summer_Prerecom_v1/map4.txt to allow 

      if(m_isRun3 && m_eleAllowRun3TrigSFFallback){
        bool pass_isRun3TrigSFFallback = true;
        if (triggerEleID.find("Medium") != std::string::npos && triggerEleIso.find("Loose") != std::string::npos)                {triggerEleID = "Loose";}
        else if (triggerEleID.find("Medium") != std::string::npos && triggerEleIso.find("Tight") != std::string::npos)           {triggerEleID = "Tight"; triggerEleIso = "Tight_VarRad";}
        else if (triggerEleID.find("Medium") != std::string::npos && triggerEleIso.find("HighPtCaloOnly") != std::string::npos)  {triggerEleID = "Tight"; triggerEleIso = "Tight_VarRad";}
        else if (triggerEleID.find("Tight")  != std::string::npos && triggerEleIso.find("Loose") != std::string::npos)           {triggerEleID = "Loose";}
        else if (triggerEleID.find("Tight")  != std::string::npos && triggerEleIso.find("Tight") != std::string::npos)           {triggerEleIso= "Tight_VarRad";}
        else if (triggerEleID.find("Tight")  != std::string::npos && triggerEleIso.find("HighPtCaloOnly") != std::string::npos)  {triggerEleID = "Tight"; triggerEleIso = "Tight_VarRad";}
        else if (triggerEleID.find("Loose")  != std::string::npos && triggerEleIso.find("Tight") != std::string::npos)           {triggerEleID = "Loose"; triggerEleIso = "Loose_VarRad";}
        else if (triggerEleID.find("Loose")  != std::string::npos && triggerEleIso.find("HighPtCaloOnly") != std::string::npos)  {triggerEleID = "Loose"; triggerEleIso = "Loose_VarRad";}
        else {pass_isRun3TrigSFFallback=false;}
        if(pass_isRun3TrigSFFallback){
          ATH_MSG_INFO(" ************** This is only for testing/studying purpose! ************** ");
          ATH_MSG_INFO(" ************** For official recommendation, please get in contact with the SUSY Bkg Forum ************** ");
          ATH_MSG_INFO("In the current map ("<<m_eleEffMapFilePath<<"), the only supported Electron ID working-points supported for Electron Trigger Scale Factor are 'Loose_Loose_VarRad' and 'Tight_Tight_VarRad' ");
          ATH_MSG_INFO("Only for single-lepton trigger scale factor, fall back to Electron ID:  -> "<< triggerEleID << " with Isolation: " << triggerEleIso);
        }
      }
      
      
      ATH_MSG_INFO("eSF_keys: " << m_electronTriggerSFStringSingle<< "_"<<triggerEleID<<"_"<<triggerEleIso);

      if (std::find(eSF_keys.begin(), eSF_keys.end(), m_electronTriggerSFStringSingle+"_"+triggerEleID+"_"+triggerEleIso) != eSF_keys.end()){
        triggerEleIso   = m_eleIso_WP;
      } else if (std::find(eSF_keys.begin(), eSF_keys.end(), m_electronTriggerSFStringSingle+"_"+triggerEleID+"_"+m_el_iso_fallback[triggerEleIso]) != eSF_keys.end()){
        //--- Check to see if the only issue is an unknown isolation working point
        triggerEleIso = m_el_iso_fallback[triggerEleIso];
        ATH_MSG_WARNING("(AsgElectronEfficiencyCorrectionTool_trig_singleLep_*) Your selected electron Iso WP ("
          << m_eleIso_WP
          << ") does not have trigger SFs defined. Falling back to "
          << triggerEleIso
          << " for SF calculations");
      }
      else{
        ATH_MSG_ERROR("***  THE SINGLE ELECTRON TRIGGER SF YOU SELECTED (" << m_electronTriggerSFStringSingle << ") GOT NO SUPPORT FOR YOUR ID+ISO WPs (" << m_eleId << "+" << m_eleIso_WP << ") ***");
        return StatusCode::FAILURE;
      }
  
      toolName = "AsgElectronEfficiencyCorrectionTool_trig_singleLep_" + triggerEleID;
      if ( !m_elecEfficiencySFTool_trig_singleLep.isUserConfigured() ) {
        m_elecEfficiencySFTool_trig_singleLep.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+toolName);
        ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("MapFilePath", m_isRun3? m_eleEffMapFilePath : m_eleEffMapFilePathRun2) );
        ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("TriggerKey", m_electronTriggerSFStringSingle) );
        ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("IdKey", triggerEleID) );
        ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("IsoKey", triggerEleIso) );
        ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("CorrelationModel", m_EG_corrModel) );
        if (!isData()) {
          ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("ForceDataType", (int) (data_type==PATCore::ParticleDataType::Fast)? PATCore::ParticleDataType::Full : data_type) );
        }
        ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.setProperty("OutputLevel", this->msg().level()) );
        ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.initialize() );
      } else ATH_CHECK( m_elecEfficiencySFTool_trig_singleLep.retrieve() );
      
  
      toolName = "AsgElectronEfficiencyCorrectionTool_trigEff_singleLep_" + triggerEleID;
      if ( !m_elecEfficiencySFTool_trigEff_singleLep.isUserConfigured() ) {
        m_elecEfficiencySFTool_trigEff_singleLep.setTypeAndName("AsgElectronEfficiencyCorrectionTool/"+toolName);
        ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("MapFilePath", m_isRun3? m_eleEffMapFilePath : m_eleEffMapFilePathRun2) );
        ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("TriggerKey", "Eff_"+m_electronTriggerSFStringSingle) );
        ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("IdKey", triggerEleID) );
        ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("IsoKey", triggerEleIso) );
        ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("CorrelationModel", m_EG_corrModel) );
        if (!isData()) {
          ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("ForceDataType", (int) (data_type==PATCore::ParticleDataType::Fast)? PATCore::ParticleDataType::Full : data_type) );
        }
        ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.setProperty("OutputLevel", this->msg().level()) );
        ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.initialize() );
      } else  ATH_CHECK( m_elecEfficiencySFTool_trigEff_singleLep.retrieve() );
    
      if (!m_isRun3){
        //mixed-leptons
        std::map<std::string,std::string> electronTriggerSFMapMixedLepton {
          // legs, Trigger keys, 
          {"e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose,e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0", m_electronTriggerSFStringSingle},
          {"e24_lhvloose_nod0_L1EM20VH,e17_lhvloose_nod0,e12_lhloose_L1EM10VH","DI_E_2015_e12_lhloose_L1EM10VH_2016_e17_lhvloose_nod0_2017_2018_e24_lhvloose_nod0_L1EM20VH"},
          {"e26_lhmedium_nod0_L1EM22VHI,e26_lhmedium_nod0","MULTI_L_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_e26_lhmedium_nod0_L1EM22VHI_2017_2018_e26_lhmedium_nod0"},
          {"e17_lhloose,e17_lhloose_nod0","MULTI_L_2015_e17_lhloose_2016_2018_e17_lhloose_nod0"},
          {"e12_lhloose,e12_lhloose_nod0","MULTI_L_2015_e12_lhloose_2016_2018_e12_lhloose_nod0"},
          {"e7_lhmedium,e7_lhmedium_nod0","MULTI_L_2015_e7_lhmedium_2016_2018_e7_lhmedium_nod0"},
          {"e9_lhloose,e9_lhloose_nod0,e12_lhvloose_nod0_L1EM10VH","TRI_E_2015_e9_lhloose_2016_e9_lhloose_nod0_2017_2018_e12_lhvloose_nod0_L1EM10VH"}
          
        };
        
        // 2e17 trigger is used in 2017 or 2018?
        std::string triglist_2017to2018 = m_trig2017combination_diLep + "_" + m_trig2018combination_diLep + "_" + m_trig2017combination_multiLep + "_" + m_trig2018combination_multiLep;
        if (triglist_2017to2018.find("2e17_lhvloose_nod0_L12EM15VHI") != std::string::npos) { 
          electronTriggerSFMapMixedLepton["e17_lhvloose_nod0_L1EM15VHI"] = "DI_E_2015_e12_lhloose_L1EM10VH_2016_e17_lhvloose_nod0_2017_2018_e17_lhvloose_nod0_L1EM15VHI";
        }
    
        std::string triggerMixedEleIso("");
    
        for(auto const& item : electronTriggerSFMapMixedLepton){
    
          if (std::find(eSF_keys.begin(), eSF_keys.end(), item.second+"_"+eleId+"_"+m_eleIso_WP) != eSF_keys.end()){
            triggerMixedEleIso = m_eleIso_WP;
          } else if (std::find(eSF_keys.begin(), eSF_keys.end(), item.second+"_"+eleId+"_"+m_el_iso_fallback[m_eleIso_WP]) != eSF_keys.end()){
            //--- Check to see if the only issue is an unknown isolation working point
            triggerMixedEleIso = m_el_iso_fallback[m_eleIso_WP];
            ATH_MSG_WARNING("(AsgElectronEfficiencyCorrectionTool_trig_mixLep_*) Your selected electron Iso WP ("
              << m_eleIso_WP
              << ") does not have trigger SFs defined. Falling back to "
              << triggerMixedEleIso
              << " for SF calculations");
          } else {
            ATH_MSG_ERROR("***  THE MIXED ELECTRON TRIGGER SF YOU SELECTED (" << item.second << ") GOT NO SUPPORT FOR YOUR ID+ISO WPs (" << m_eleId << "+" << m_eleIso_WP << "). The fallback options failed as well sorry! ***");
            return StatusCode::FAILURE;
          }
    
          ATH_MSG_VERBOSE ("Selected WP: " << item.second << "_" << eleId << "_" << triggerMixedEleIso);
    
          toolName = "AsgElectronEfficiencyCorrectionTool_trig_mixLep_" + (item.first).substr(0,8) + m_eleId;
          auto t_sf = m_elecEfficiencySFTool_trig_mixLep.emplace(m_elecEfficiencySFTool_trig_mixLep.end(), "AsgElectronEfficiencyCorrectionTool/"+toolName);
          ATH_CHECK( t_sf->setProperty("MapFilePath", m_isRun3? m_eleEffMapFilePath : m_eleEffMapFilePathRun2) );
          ATH_CHECK( t_sf->setProperty("TriggerKey", item.second) );
          ATH_CHECK( t_sf->setProperty("IdKey", eleId) );
          ATH_CHECK( t_sf->setProperty("IsoKey", triggerMixedEleIso) );
          ATH_CHECK( t_sf->setProperty("CorrelationModel", m_EG_corrModel) );
          if (!isData()) {
            ATH_CHECK( t_sf->setProperty("ForceDataType", (int) (data_type==PATCore::ParticleDataType::Fast)? PATCore::ParticleDataType::Full : data_type) );
          }
          ATH_CHECK( t_sf->setProperty("OutputLevel", this->msg().level()) );
          ATH_CHECK( t_sf->initialize() );
          m_elecTrigSFTools.push_back(t_sf->getHandle());
          #ifndef XAOD_STANDALONE
            m_legsPerTool[toolName] = item.first;
          #else
            m_legsPerTool["ToolSvc."+toolName] = item.first;
          #endif
    
          toolName = "AsgElectronEfficiencyCorrectionTool_trigEff_mixLep_" + (item.first).substr(0,8) + m_eleId;
          auto t_eff = m_elecEfficiencySFTool_trigEff_mixLep.emplace(m_elecEfficiencySFTool_trigEff_mixLep.end(), "AsgElectronEfficiencyCorrectionTool/"+toolName);
          ATH_CHECK( t_eff->setProperty("MapFilePath", m_isRun3? m_eleEffMapFilePath : m_eleEffMapFilePathRun2) );
          ATH_CHECK( t_eff->setProperty("TriggerKey", "Eff_"+item.second) );
          ATH_CHECK( t_eff->setProperty("IdKey", eleId) );
          ATH_CHECK( t_eff->setProperty("IsoKey", triggerMixedEleIso) );
          ATH_CHECK( t_eff->setProperty("CorrelationModel", m_EG_corrModel) );
          if (!isData()) {
            ATH_CHECK( t_eff->setProperty("ForceDataType", (int) (data_type==PATCore::ParticleDataType::Fast)? PATCore::ParticleDataType::Full : data_type) );
          }
          ATH_CHECK( t_eff->setProperty("OutputLevel", this->msg().level()) );
          ATH_CHECK( t_eff->initialize() );
          m_elecTrigEffTools.push_back(t_eff->getHandle());
          #ifndef XAOD_STANDALONE
            m_legsPerTool[toolName] = item.first;
          #else
            m_legsPerTool["ToolSvc."+toolName] = item.first;
          #endif
    
        }
      }
      else ATH_MSG_WARNING("*** THE ELECTRON TRIGGER SF MIXED AND SF DILEP ARE CURRENTY NOT SUPPORTED IN RUN3 ***");
    
    ATH_MSG_WARNING("*** THE ELECTRON CHARGE FLIP SF ARE CURRENTY NOT SUPPORTED ***");
    // all the iinitialisation of the related tools have been cleaned up

  }

  if (m_slices["pho"]) {
    // /////////////////////////////////////////////////////////////////////////////////////////
    // Initialise photon efficiency tool

    if (!m_photonEfficiencySFTool.isUserConfigured() && !isData()) {
      m_photonEfficiencySFTool.setTypeAndName("AsgPhotonEfficiencyCorrectionTool/AsgPhotonEfficiencyCorrectionTool_" + m_photonId);

      if (m_photonId != "Tight" ) {
        ATH_MSG_WARNING( "No Photon efficiency available for " << m_photonId << ", using Tight instead..." );
      }

      ATH_CHECK( m_photonEfficiencySFTool.setProperty("MapFilePath", "PhotonEfficiencyCorrection/2015_2025/rel22.2/2024_FinalRun2_Recommendation_v1/map1.txt") );
      ATH_CHECK( m_photonEfficiencySFTool.setProperty("ForceDataType", 1) ); // Set data type: 1 for FULLSIM, 3 for AF2
      ATH_CHECK( m_photonEfficiencySFTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_photonEfficiencySFTool.retrieve() );
    } else if (m_photonEfficiencySFTool.isUserConfigured()) ATH_CHECK( m_photonEfficiencySFTool.retrieve() );

   if (!m_photonIsolationSFTool.isUserConfigured() && !isData()) {
     m_photonIsolationSFTool.setTypeAndName("AsgPhotonEfficiencyCorrectionTool/AsgPhotonEfficiencyCorrectionTool_isol" + m_photonIso_WP);

     if (m_photonIso_WP != "FixedCutTight" && m_photonIso_WP != "FixedCutLoose" && m_photonIso_WP != "TightCaloOnly") {
       ATH_MSG_WARNING( "No Photon efficiency available for " << m_photonIso_WP);
     }

     ATH_CHECK( m_photonIsolationSFTool.setProperty("MapFilePath", "PhotonEfficiencyCorrection/2015_2025/rel22.2/2022_Summer_Prerecom_v1/map0.txt") );
     ATH_CHECK( m_photonIsolationSFTool.setProperty("IsoKey", m_photonIso_WP != "TightCaloOnly" ? m_photonIso_WP.substr(8) : m_photonIso_WP ));    // Set isolation WP: Loose,Tight,TightCaloOnly
     ATH_CHECK( m_photonIsolationSFTool.setProperty("ForceDataType", 1) ); // Set data type: 1 for FULLSIM, 3 for AF2
     ATH_CHECK( m_photonIsolationSFTool.setProperty("OutputLevel", this->msg().level()) );
     ATH_CHECK( m_photonIsolationSFTool.retrieve() );
   } else if (m_photonEfficiencySFTool.isUserConfigured()) ATH_CHECK( m_photonIsolationSFTool.retrieve() );

    // trigger scale factors
    if (!m_photonTriggerSFTool.isUserConfigured() && !isData()) {
      m_photonTriggerSFTool.setTypeAndName("AsgPhotonEfficiencyCorrectionTool/AsgPhotonEfficiencyCorrectionTool_trig" + m_photonTriggerName);

      // Fallback to TightCaloOnly if Tight is selected
      std::string photonIso_forTrigSF = m_photonIso_WP;
      if (m_photonIso_WP == "FixedCutTight") {
        ATH_MSG_WARNING( "No Photon trigger SF available for " << m_photonIso_WP << ", using TightCaloOnly instead... Use at your own risk" );
        photonIso_forTrigSF = "TightCaloOnly";
      } else { //  isolation WP supported: Loose or TightCaloOnly, removing "FixedCut" suffix..
        photonIso_forTrigSF = TString(m_photonIso_WP).ReplaceAll("FixedCut","").Data();
      }

      // "symmetric" diphoton triggers (year dependent)
      ATH_CHECK( m_photonTriggerSFTool.setProperty("MapFilePath", "PhotonEfficiencyCorrection/2015_2018/rel21.2/Summer2020_Rec_v1/map3.txt") );
      ATH_CHECK( m_photonTriggerSFTool.setProperty("IsoKey", photonIso_forTrigSF ));    // Set isolation WP: Loose,TightCaloOnly
      ATH_CHECK( m_photonTriggerSFTool.setProperty("TriggerKey", m_photonTriggerName ));
      ATH_CHECK( m_photonTriggerSFTool.setProperty("ForceDataType", 1) ); // Set data type: 1 for FULLSIM, 3 for AF2
      ATH_CHECK( m_photonTriggerSFTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_photonTriggerSFTool.retrieve() );

      // "asymmetric" diphoton triggers
      std::map<std::string,std::string> diphotonTriggerSFMapAsymmetric {
        // legs, Trigger keys,
        {"g25_loose,g25_medium_L1EM20VH", "DI_PH_2015_g25_loose_2016_g25_loose_2017_g25_medium_L1EM20VH_2018_g25_medium_L1EM20VH"},
        {"g35_loose,g35_medium_L1EM20VH", "DI_PH_2015_g35_loose_2016_g35_loose_2017_g35_medium_L1EM20VH_2018_g35_medium_L1EM20VH"},
      };

      for(auto const& item : diphotonTriggerSFMapAsymmetric){

        toolName = "AsgPhotonEfficiencyCorrectionTool_trigSF_asymm_diphoton_" + (item.first).substr(0,9) + photonIso_forTrigSF;
        auto ph_trigSF = m_photonEfficiencySFTool_trigSF_AsymDiphoton.emplace(m_photonEfficiencySFTool_trigSF_AsymDiphoton.end(), "AsgPhotonEfficiencyCorrectionTool/"+toolName);
        ATH_CHECK( ph_trigSF->setProperty("MapFilePath", "PhotonEfficiencyCorrection/2015_2018/rel21.2/Summer2020_Rec_v1/map3.txt") );
        ATH_CHECK( ph_trigSF->setProperty("IsoKey", photonIso_forTrigSF) );
        ATH_CHECK( ph_trigSF->setProperty("TriggerKey", item.second) );
        ATH_CHECK( ph_trigSF->setProperty("ForceDataType", 1) ); // Set DataType: 1 for FullSim and 3 for AFII
        ATH_CHECK( ph_trigSF->setProperty("OutputLevel", this->msg().level()) );
        ATH_CHECK( ph_trigSF->initialize() );
        m_photonTrigSFTools.push_back(ph_trigSF->getHandle());
        #ifndef XAOD_STANDALONE
          m_legsPerTool_ph[toolName] = item.first;
        #else
          m_legsPerTool_ph["ToolSvc."+toolName] = item.first;
        #endif

        toolName = "AsgPhotonEfficiencyCorrectionTool_trigEff_asymm_diphoton_" + (item.first).substr(0,9) + photonIso_forTrigSF;
        auto ph_trigEff = m_photonEfficiencySFTool_trigEff_AsymDiphoton.emplace(m_photonEfficiencySFTool_trigEff_AsymDiphoton.end(), "AsgPhotonEfficiencyCorrectionTool/"+toolName);
        ATH_CHECK( ph_trigEff->setProperty("MapFilePath", "PhotonEfficiencyCorrection/2015_2018/rel21.2/Summer2020_Rec_v1/map3.txt") );
        ATH_CHECK( ph_trigEff->setProperty("IsoKey", photonIso_forTrigSF) );
        ATH_CHECK( ph_trigEff->setProperty("TriggerKey", "Eff_"+item.second) );
        ATH_CHECK( ph_trigEff->setProperty("ForceDataType", 1) ); // Set DataType: 1 for FullSim and 3 for AFII
        ATH_CHECK( ph_trigEff->setProperty("OutputLevel", this->msg().level()) );
        ATH_CHECK( ph_trigEff->initialize() );
        m_photonTrigEffTools.push_back(ph_trigEff->getHandle());
        #ifndef XAOD_STANDALONE
          m_legsPerTool_ph[toolName] = item.first;
        #else
          m_legsPerTool_ph["ToolSvc."+toolName] = item.first;
        #endif

        }
      }
    }

  }
  if (m_slices["ele"] || m_slices["pho"]) {
    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialize the EgammaAmbiguityTool

    if (!m_egammaAmbiguityTool.isUserConfigured()) {
      m_egammaAmbiguityTool.setTypeAndName("EGammaAmbiguityTool/EGammaAmbiguityTool");
      ATH_CHECK( m_egammaAmbiguityTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_egammaAmbiguityTool.retrieve() );
    } else ATH_CHECK( m_egammaAmbiguityTool.retrieve() );
  }


  if (m_slices["ele"]) {
    //disable ///////////////////////////////////////////////////////////////////////////////////////////
    //disable // Initialize the AsgElectronChargeIDSelector

    //disable if (!m_elecChargeIDSelectorTool.isUserConfigured()) {

    //disable   // For the selector, can use the nice function
    //disable   std::string eleId = EG_WP(m_eleId);
    //disable   m_elecChargeIDSelectorTool.setTypeAndName("AsgElectronChargeIDSelectorTool/ElectronChargeIDSelectorTool_"+eleId);
    //disable   //default cut value for https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ElectronChargeFlipTaggerTool
    //disable   float BDTcut = -0.337671; // Loose 97%
    //disable   if (m_eleChID_WP != "Loose" && !m_eleChID_WP.empty()) {
    //disable     ATH_MSG_ERROR("Only Loose WP is supported in R21. Invalid ChargeIDSelector WP selected : " << m_eleChID_WP);
    //disable     return StatusCode::FAILURE;
    //disable   }

    //disable   ATH_CHECK( m_elecChargeIDSelectorTool.setProperty("TrainingFile", "ElectronPhotonSelectorTools/ChargeID/ECIDS_20180731rel21Summer2018.root"));
    //disable   ATH_CHECK( m_elecChargeIDSelectorTool.setProperty("CutOnBDT", BDTcut));
    //disable   ATH_CHECK( m_elecChargeIDSelectorTool.setProperty("OutputLevel", this->msg().level()) );
    //disable   ATH_CHECK( m_elecChargeIDSelectorTool.retrieve() );
    //disable } else  ATH_CHECK( m_elecChargeIDSelectorTool.retrieve() );


    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise egamma calibration tool

    if (!m_egammaCalibTool.isUserConfigured()) {
      m_egammaCalibTool.setTypeAndName("CP::EgammaCalibrationAndSmearingTool/EgammaCalibrationAndSmearingTool");
      ATH_MSG_DEBUG( "Initialising EgcalibTool " );
      ATH_CHECK( m_egammaCalibTool.setProperty("ESModel", m_isRun3 ? "es2022_R22_PRE" : "es2023_R22_Run2_v1") );
      ATH_CHECK( m_egammaCalibTool.setProperty("decorrelationModel", "1NP_v1") );
      // allows to bypass (intended) abort from of egamma calibration tool when configured for fastSim
      if (m_eleForceFullSimCalib) {
        ATH_MSG_WARNING( "Forcing EgcalibTool to use calibrations for full simulation" );
        ATH_CHECK( m_egammaCalibTool.setProperty("useFastSim", false) );
      }
      else {
        ATH_MSG_INFO("is Fast Sim:" << isAtlfast());
        ATH_CHECK( m_egammaCalibTool.setProperty("useFastSim", isAtlfast() ? 1 : 0) );
      }
      ATH_CHECK( m_egammaCalibTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_egammaCalibTool.retrieve() );
    } else ATH_CHECK( m_egammaCalibTool.retrieve() );
  }


  if (m_slices["tau"]) {
  ///////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise path to tau config file and config reader

    std::string inputfile = "";
    if (!m_tauConfigPath.empty() && (m_tauConfigPath!="default")) inputfile = m_tauConfigPath;
    else if (m_tauId == "rnn001") inputfile = "SUSYTools/tau_selection_rnn001.conf";
    else if (m_tauId == "VeryLoose") inputfile = "SUSYTools/tau_selection_veryloose.conf";
    else if (m_tauId == "Loose") inputfile = "SUSYTools/tau_selection_loose.conf";
    else if (m_tauId == "Medium") inputfile = "SUSYTools/tau_selection_medium.conf";
    else if (m_tauId == "Tight") inputfile = "SUSYTools/tau_selection_tight.conf";
    else {
      ATH_MSG_ERROR("Invalid tau ID selected: " << m_tauId);
      return StatusCode::FAILURE;
    }

    // Read in the config file so we can retrieve the fields later when configuring the efficiency tools
    if ( m_tauConfigReader.ReadFile( PathResolverFindCalibFile(inputfile).c_str(), EEnvLevel(0) ) ) {
      ATH_MSG_ERROR( "Error while reading tau config file : " << inputfile );
      return StatusCode::FAILURE;
    }
    else ATH_MSG_DEBUG( "Successfully read tau config file : " << inputfile );


    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise tau selection tools

    if (!m_tauSelTool.isUserConfigured()) {
      toolName = "TauSelectionTool_" + m_tauId;
      m_tauSelTool.setTypeAndName("TauAnalysisTools::TauSelectionTool/"+toolName);
      ATH_CHECK( m_tauSelTool.setProperty("ConfigPath", inputfile) );
      ATH_CHECK( m_tauSelTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_tauSelTool.retrieve() );
    } else  ATH_CHECK( m_tauSelTool.retrieve() );


    if (!m_tauSelToolBaseline.isUserConfigured()) {
      std::string inputfile = "";
      if (!m_tauConfigPathBaseline.empty() && (m_tauConfigPathBaseline!="default")) inputfile = m_tauConfigPathBaseline;
      else if (m_tauIdBaseline == "rnn001") inputfile = "SUSYTools/tau_selection_rnn001.conf";
      else if (m_tauIdBaseline == "VeryLoose") inputfile = "SUSYTools/tau_selection_veryloose.conf";
      else if (m_tauIdBaseline == "Loose") inputfile = "SUSYTools/tau_selection_loose.conf";
      else if (m_tauIdBaseline == "Medium") inputfile = "SUSYTools/tau_selection_medium.conf";
      else if (m_tauIdBaseline == "Tight") inputfile = "SUSYTools/tau_selection_tight.conf";
      else {
        ATH_MSG_ERROR("Invalid baseline tau ID selected: " << m_tauIdBaseline);
        return StatusCode::FAILURE;
      }
      toolName = "TauSelectionToolBaseline_" + m_tauIdBaseline;
      m_tauSelToolBaseline.setTypeAndName("TauAnalysisTools::TauSelectionTool/"+toolName);
      ATH_CHECK( m_tauSelToolBaseline.setProperty("ConfigPath", inputfile) );

      ATH_CHECK( m_tauSelToolBaseline.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_tauSelToolBaseline.retrieve() );
    } else  ATH_CHECK( m_tauSelToolBaseline.retrieve() );


    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise tau efficiency tool

    if (!m_tauEffTool.isUserConfigured() && !isData()) {
      toolName = "TauEffTool_" + m_tauId;
      m_tauEffTool.setTypeAndName("TauAnalysisTools::TauEfficiencyCorrectionsTool/"+toolName);
      ATH_CHECK( m_tauEffTool.setProperty("PileupReweightingTool",m_prwTool.getHandle()) );

      std::vector<int> correction_types;
      // Read out the tau ID from the config file and map into the enum from tau CP
      std::string jetIDWP = m_tauConfigReader.GetValue("JetIDWP" ,"");
      ANA_MSG_DEBUG( "Found JetIDWP in tau config file : " << jetIDWP );
      int jet_id_lvl;
      if (jetIDWP == "JETIDNONE") jet_id_lvl = (int)TauAnalysisTools::JetID::JETIDNONE;
      else if (jetIDWP == "JETIDRNNVERYLOOSE") jet_id_lvl = (int)TauAnalysisTools::JetID::JETIDRNNVERYLOOSE;
      else if (jetIDWP == "JETIDRNNLOOSE") jet_id_lvl = (int)TauAnalysisTools::JetID::JETIDRNNLOOSE;
      else if (jetIDWP == "JETIDRNNMEDIUM") jet_id_lvl = (int)TauAnalysisTools::JetID::JETIDRNNMEDIUM;
      else if (jetIDWP == "JETIDRNNTIGHT") jet_id_lvl = (int)TauAnalysisTools::JetID::JETIDRNNTIGHT;
      else {
        ATH_MSG_ERROR("Invalid Tau ID in tau config file " << jetIDWP);
        return StatusCode::FAILURE; 
      }
      // Add retrieval of reco and ID SFs
      correction_types.insert(correction_types.end(), {TauAnalysisTools::EfficiencyCorrectionType::SFRecoHadTau,
                                                       TauAnalysisTools::EfficiencyCorrectionType::SFJetIDHadTau});

      // Read out the (optional) Ele OR from the config file and map into the enum from tau CP
      std::string eleIDWP = m_tauConfigReader.GetValue("EleIDWP" ,"");
      ANA_MSG_DEBUG( "Found EleIDWP in tau config file : " << eleIDWP );
      int ele_id_lvl = -1;
      if (eleIDWP == "ELEIDRNNLOOSE") ele_id_lvl = (int)TauAnalysisTools::EleID::ELEIDRNNLOOSE;
      else if (eleIDWP == "ELEIDRNNMEDIUM") ele_id_lvl = (int)TauAnalysisTools::EleID::ELEIDRNNMEDIUM;
      else if (eleIDWP == "ELEIDRNNTIGHT") ele_id_lvl = (int)TauAnalysisTools::EleID::ELEIDRNNTIGHT;
      else {
        ATH_MSG_INFO("No or invalid Ele OR in tau config file " << eleIDWP << " will not apply SFs for electro veto" );
      }
      // Add retrieval of electron veto SFs if its applied
      if (ele_id_lvl != -1 )
        correction_types.insert(correction_types.end(), {TauAnalysisTools::EfficiencyCorrectionType::SFEleIDHadTau,
                                                         TauAnalysisTools::EfficiencyCorrectionType::SFEleIDElectron});

      ATH_CHECK( m_tauEffTool.setProperty("JetIDLevel", jet_id_lvl) );
      ATH_CHECK( m_tauEffTool.setProperty("EleIDLevel", ele_id_lvl) );
      ATH_CHECK( m_tauEffTool.setProperty("EfficiencyCorrectionTypes", correction_types) );
      ATH_CHECK( m_tauEffTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_tauEffTool.setProperty("useFastSim", isAtlfast()) );
      ATH_CHECK( m_tauEffTool.retrieve() );
    } else if (m_tauEffTool.isUserConfigured()) ATH_CHECK( m_tauEffTool.retrieve() );


    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise tau trigger efficiency tool(s)

    if (!isData()) {
      int iTauID = (int) TauAnalysisTools::JETIDNONEUNCONFIGURED;
      if (m_tauId == "rnn001")   iTauID = (int) TauAnalysisTools::JETIDNONE;
      else if (m_tauId == "VeryLoose")   iTauID = (int) TauAnalysisTools::JETIDRNNVERYLOOSE;
      else if (m_tauId == "Loose")  iTauID = (int) TauAnalysisTools::JETIDRNNLOOSE;
      else if (m_tauId == "Medium") iTauID = (int) TauAnalysisTools::JETIDRNNMEDIUM;
      else if (m_tauId == "Tight")  iTauID = (int) TauAnalysisTools::JETIDRNNTIGHT;
      else {
        ATH_MSG_ERROR("Invalid tau ID selected: " << m_tauId);
        return StatusCode::FAILURE;
      }

      // map format: SF file name, corresponding single-tau leg (comma-separated in case of OR)
      m_tau_trig_support = {
        {"HLT_tau25_medium1_tracktwo", "HLT_tau25_medium1_tracktwo"},
        {"HLT_tau35_medium1_tracktwo", "HLT_tau35_medium1_tracktwo"},
        {"HLT_tau50L1TAU12_medium1_tracktwo", "HLT_tau50_medium1_tracktwo_L1TAU12"},
        {"HLT_tau60_medium1_tracktwo", "HLT_tau60_medium1_tracktwo"},
        {"HLT_tau80L1TAU60_medium1_tracktwo", "HLT_tau80_medium1_tracktwo_L1TAU60"},
        {"HLT_tau125_medium1_tracktwo", "HLT_tau125_medium1_tracktwo"},
        {"HLT_tau160_medium1_tracktwo", "HLT_tau160_medium1_tracktwo"},
        {"HLT_tau160L1TAU100_medium1_tracktwo", "HLT_tau160_medium1_tracktwo_L1TAU100"},
        // whole 2018 data taking period
        {"HLT_tau25_medium1_tracktwoEF", "HLT_tau25_medium1_tracktwoEF"},
        {"HLT_tau35L1TAU12IM_medium1_tracktwoEF", "HLT_tau35_medium1_tracktwoEF_L1TAU12IM"},
        {"HLT_tau35_medium1_tracktwoEF", "HLT_tau35_medium1_tracktwoEF"},
        {"HLT_tau60_medium1_tracktwoEF", "HLT_tau60_medium1_tracktwoEF"},
        {"HLT_tau80L1TAU60_medium1_tracktwoEF", "HLT_tau80_medium1_tracktwoEF_L1TAU60"},
        {"HLT_tau160L1TAU100_medium1_tracktwoEF", "HLT_tau160_medium1_tracktwoEF_L1TAU100"},
        // 2018 data after TS1, run >= 355261
        {"HLT_tau25_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA", "HLT_tau25_medium1_tracktwoEF,HLT_tau25_mediumRNN_tracktwoMVA"},
        {"HLT_tau35L1TAU12IM_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA", "HLT_tau35_medium1_tracktwoEF_L1TAU12IM,HLT_tau35_mediumRNN_tracktwoMVA_L1TAU12IM"},
        {"HLT_tau35_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA", "HLT_tau35_medium1_tracktwoEF,HLT_tau35_mediumRNN_tracktwoMVA"},
        {"HLT_tau60_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA", "HLT_tau60_medium1_tracktwoEF,HLT_tau60_mediumRNN_tracktwoMVA"},
        {"HLT_tau80L1TAU60_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA", "HLT_tau80_medium1_tracktwoEF_L1TAU60,HLT_tau80_mediumRNN_tracktwoMVA_L1TAU60"},
        {"HLT_tau160L1TAU100_medium1_tracktwoEF_OR_mediumRNN_tracktwoMVA", "HLT_tau160_medium1_tracktwoEF_L1TAU100,HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100"}
      };

      for(auto const& trigger : m_tau_trig_support) {
        toolName = "TauTrigEffTool_" + m_tauId + "_" + trigger.first;
        auto tau_trigSF = m_tauTrigEffTool.emplace(m_tauTrigEffTool.end(), "TauAnalysisTools::TauEfficiencyCorrectionsTool/"+toolName);
        ATH_CHECK( tau_trigSF->setProperty("EfficiencyCorrectionTypes", std::vector<int>({TauAnalysisTools::SFTriggerHadTau})) );
        ATH_CHECK( tau_trigSF->setProperty("TriggerName", trigger.first) );
        ATH_CHECK( tau_trigSF->setProperty("JetIDLevel", iTauID) );
        ATH_CHECK( tau_trigSF->setProperty("PileupReweightingTool", m_prwTool.getHandle()) );
        ATH_CHECK( tau_trigSF->setProperty("OutputLevel", this->msg().level()) );
        ATH_CHECK( tau_trigSF->setProperty("useFastSim", isAtlfast()) );
        ATH_CHECK( tau_trigSF->initialize() );
      }
    }


    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise tau smearing tool

    if (!m_tauSmearingTool.isUserConfigured()) {
      m_tauSmearingTool.setTypeAndName("TauAnalysisTools::TauSmearingTool/TauSmearingTool");
      ATH_CHECK( m_tauSmearingTool.setProperty("RecommendationTag", m_tauSmearingToolRecommendationTag) );
      if (m_isRun3){ ATH_CHECK( m_tauSmearingTool.setProperty("Campaign", "mc21") );}
      else         { ATH_CHECK( m_tauSmearingTool.setProperty("Campaign", "mc20") );}
      ATH_CHECK( m_tauSmearingTool.setProperty("Generator", m_tauSmearingToolGenerator) );
      ATH_CHECK( m_tauSmearingTool.setProperty("useFastSim", isAtlfast()) );
      ATH_CHECK( m_tauSmearingTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_tauSmearingTool.retrieve() );
    } else ATH_CHECK( m_tauSmearingTool.retrieve() );


    ///////////////////////////////////////////////////////////////////////////////////////////
    // Initialise tau truth matching tool

    if (!m_tauTruthMatch.isUserConfigured() && m_tauDoTTM && !isData()) {
      m_tauTruthMatch.setTypeAndName("TauAnalysisTools::TauTruthMatchingTool/TauTruthMatch");
      ATH_CHECK( m_tauTruthMatch.setProperty("TruthJetContainerName", m_defaultTruthJets ) );
      ATH_CHECK( m_tauTruthMatch.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_tauTruthMatch.retrieve() );
    } else if (m_tauTruthMatch.isUserConfigured()) ATH_CHECK( m_tauTruthMatch.retrieve() );


  }


  if (m_slices["bjet"]) {
  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise B-tagging tools

    // btagSelectionTool
    std::string jetcollBTag = jetcoll;
    if (jetcoll == "AntiKt4LCTopoJets") {
      ATH_MSG_WARNING("  *** HACK *** Treating LCTopoJets jets as EMTopo -- use at your own risk!");
      jetcollBTag = "AntiKt4EMTopoJets";
    }

    // print a warning if there is indication that the user supplied a CDI file not matching to the sample being processed
    // see --> https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTagRel22HighLevelSummary
    if( (m_bTaggingCalibrationFilePath.find(!m_isRun3 ? "MC20" : "MC23") == std::string::npos) && (m_bTaggingCalibrationFilePath.find(!m_isRun3 ? "MC20" : "MC21") == std::string::npos) ) {
      ATH_MSG_WARNING( "You are using a "<<(!m_isRun3 ? "Run3":"Run2")<<" CDI file while running on "<<(!m_isRun3 ? "Run2":"Run3")<<" sample; Please updates your CDI file to the correct version for "<<(!m_isRun3 ? "Run2":"Run3"));
    }

    if (m_useBtagging && !m_btagSelTool.isUserConfigured() && !m_BtagWP.empty()) {
      if (jetcollBTag.find("AntiKt4EMTopoJets") == std::string::npos && jetcollBTag.find("AntiKt4EMPFlowJets")==std::string::npos) {
        ATH_MSG_WARNING("** Only AntiKt4EMTopoJets and AntiKt4EMPFlowJets are supported with FTAG scale factors!");
          return StatusCode::FAILURE;
      }

      toolName = "BTagSel_" + jetcollBTag + m_BtagTagger + m_BtagWP;

      m_btagSelTool.setTypeAndName("BTaggingSelectionTool/"+toolName);
      ATH_CHECK( m_btagSelTool.setProperty("TaggerName",     m_BtagTagger ) );
      ATH_CHECK( m_btagSelTool.setProperty("OperatingPoint", m_BtagWP  ) );
      ATH_CHECK( m_btagSelTool.setProperty("JetAuthor",      jetcollBTag   ) );
      ATH_CHECK( m_btagSelTool.setProperty("MinPt",          m_BtagMinPt   ) );
      ATH_CHECK( m_btagSelTool.setProperty("FlvTagCutDefinitionsFileName",  m_bTaggingCalibrationFilePath) );
      ATH_CHECK( m_btagSelTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_btagSelTool.retrieve() );
    } else if (m_btagSelTool.isUserConfigured()) ATH_CHECK( m_btagSelTool.retrieve() );


    if (m_useBtagging && !m_btagSelTool_OR.isUserConfigured() && !m_orBtagWP.empty()) {
      if (jetcoll != "AntiKt4EMTopoJets" && jetcoll != "AntiKt4EMPFlowJets") {
        ATH_MSG_WARNING("** Only AntiKt4EMTopoJets and AntiKt4EMPFlowJets are supported with FTAG scale factors!");
          return StatusCode::FAILURE;
      }

      toolName = "BTagSelOR_" + jetcollBTag + m_orBtagWP;
      m_btagSelTool_OR.setTypeAndName("BTaggingSelectionTool/"+toolName);
      ATH_CHECK( m_btagSelTool_OR.setProperty("TaggerName",     m_BtagTagger  ) );
      ATH_CHECK( m_btagSelTool_OR.setProperty("OperatingPoint", m_orBtagWP  ) );
      ATH_CHECK( m_btagSelTool_OR.setProperty("JetAuthor",      jetcollBTag   ) );
      ATH_CHECK( m_btagSelTool_OR.setProperty("MinPt",          m_BtagMinPt   ) );
      ATH_CHECK( m_btagSelTool_OR.setProperty("FlvTagCutDefinitionsFileName",  m_bTaggingCalibrationFilePath) );
      ATH_CHECK( m_btagSelTool_OR.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_btagSelTool_OR.retrieve() );
    } else if (m_btagSelTool_OR.isUserConfigured()) ATH_CHECK( m_btagSelTool_OR.retrieve() );


    std::string trkjetcoll = m_defaultTrackJets;
    const std::string& BTagColl_TrkJet = trkjetcoll;
    if (m_slices["tjet"]) {
      if ( m_useBtagging_trkJet && m_defaultTrackJets.empty()) {
         m_useBtagging_trkJet = false;
         ATH_MSG_INFO("TrackJet collection set to None: disabling btagging for TrackJets.");
      }

      if (m_useBtagging_trkJet && !m_btagSelTool_trkJet.isUserConfigured() && !m_BtagWP_trkJet.empty()) {
        if (trkjetcoll.find("AntiKt2PV0TrackJets")==std::string::npos && trkjetcoll.find("AntiKtVR30Rmax4Rmin02TrackJets")==std::string::npos) {
          ATH_MSG_WARNING("** Only AntiKt2PV0TrackJets and AntiKtVR30Rmax4Rmin02TrackJets are supported with FTAG scale factors!");
            return StatusCode::FAILURE;
        }

        toolName = "BTagSel_" + trkjetcoll + m_BtagTagger_trkJet + m_BtagWP_trkJet;

        m_btagSelTool_trkJet.setTypeAndName("BTaggingSelectionTool/"+toolName);
        ATH_CHECK( m_btagSelTool_trkJet.setProperty("TaggerName",     m_BtagTagger_trkJet ) );
        ATH_CHECK( m_btagSelTool_trkJet.setProperty("OperatingPoint", m_BtagWP_trkJet  ) );
        ATH_CHECK( m_btagSelTool_trkJet.setProperty("MinPt",          m_BtagMinPt_trkJet  ) );
        ATH_CHECK( m_btagSelTool_trkJet.setProperty("JetAuthor",      BTagColl_TrkJet   ) );
        ATH_CHECK( m_btagSelTool_trkJet.setProperty("MinPt",          m_BtagMinPt_trkJet ) );
        ATH_CHECK( m_btagSelTool_trkJet.setProperty("FlvTagCutDefinitionsFileName",  m_bTaggingCalibrationFilePath) );
        ATH_CHECK( m_btagSelTool_trkJet.setProperty("OutputLevel", this->msg().level()) );
        ATH_CHECK( m_btagSelTool_trkJet.retrieve() );
      } else if (m_btagSelTool_trkJet.isUserConfigured()) ATH_CHECK( m_btagSelTool_trkJet.retrieve() );
    }

   // Set MCshowerType for FTAG MC/MC SFs
    // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/BTagCalibrationRecommendationsRelease21#MC_MC_Scale_Factors_for_Analysis
    std::string MCshowerID;
    if(m_showerType==-1){
        ATH_MSG_ERROR( "Error! m_showerType=-1, which is an error case for FTAG MC/MC SFs." );
        return StatusCode::FAILURE;
    }
    if(m_BtagTagger=="DL1dv01"){
      if(!m_isRun3){
        MCshowerID= "default";                             // Powheg+Pythia8 (410470)  - PhPy8EG_A14
        if (m_showerType == 1)      MCshowerID = "411233"; // PowhegHerwig7 
        else if (m_showerType == 2) MCshowerID = "600666"; // PhH7EG_H7UE - 601414
        else if (m_showerType == 3) MCshowerID = "410250"; // Sherpa_221
        else if (m_showerType == 4) MCshowerID = "700122"; // Sh_2210
        else if (m_showerType == 5) MCshowerID = "700122"; // Sh_2211
        else if (m_showerType == 6) MCshowerID = "700660"; // Sh_2212 - 700660
        else if (m_showerType == 7) MCshowerID = "410464"; // aMcAtNloPy8
        else if (m_showerType == 8) MCshowerID = "412116"; // aMcAtNloHerwig7
        else if (m_showerType != 0){
            ATH_MSG_WARNING("Unknown ShowerID detected! Using default FTAG MC-MC SFs (Powheg+Pythia8). Please check if these are applicable for this sample!");
        }
      }
      else{
        MCshowerID= "default";                              // Powheg+Pythia8 (601229)  - PhPy8EG_A14
        if (m_showerType == 2)      MCshowerID = "601414";  // POWHEG+Herwig721         - PhH7EG_A14
        else if (m_showerType == 6) MCshowerID = "700660";  // Sherpa 2.2.12            - Sh_2212
        else if (m_showerType != 0){
            ATH_MSG_WARNING("Unknown ShowerID detected! Using default FTAG MC-MC SFs (Powheg+Pythia8). Please check if these are applicable for this sample!");
        }
      }
    } else if (m_BtagTagger=="GN2v01"){
      if(!m_isRun3){
        MCshowerID= "default";                              // PowhegPythia8EvtGen (410470)
        if (m_showerType == 1)      MCshowerID = "410480";  // PYTHIA8EVTGEN517 (410480)
        else if (m_showerType == 2) MCshowerID = "411233";  // POWHEGHERWIG7 - 411233
        else if (m_showerType == 3) MCshowerID = "600666";  // PhH7EG_H7UE - 600666
        else if (m_showerType == 4) MCshowerID = "700660"; // Sh_2210 FTAGAnalysisConfig uses this, but docs say only 11-16 can be used
        else if (m_showerType == 5) MCshowerID = "700660"; // Sh_2211
        else if (m_showerType == 6) MCshowerID = "700660"; // Sh_2212
        else if (m_showerType == 7) MCshowerID = "700660"; // Sh_2214
        else if (m_showerType == 8) {
            ATH_MSG_WARNING("Unknown Sherpa version detected! Using FTAG MC-MC SFs for Sherpa 2.2.11-2.2.16. Please check if these are applicable for this sample!"); 
            MCshowerID = "700660"; // Unknown Sherpa Version. This is to handle Sh_blank (e.g. DSID 701050). The examples I've found are all 2.2.16, but that's not guaranteed.
        }
        else if (m_showerType != 0){
            ATH_MSG_WARNING("Unknown ShowerID detected! Using default FTAG MC-MC SFs (Powheg+Pythia8). Please check if these are applicable for this sample!");
        }
      }
      else {
        MCshowerID= "default";                              // PowhegPythia8EvtGen (601229)
        if (m_showerType == 1)      MCshowerID = "601398";  // PYTHIA8EVTGEN517 (601398)
        else if (m_showerType == 3) MCshowerID = "601414";  // PhH7EG_H7UE - 601414 
        else if (m_showerType == 5) MCshowerID = "700808"; // Sh_2211
        else if (m_showerType == 6) MCshowerID = "700808"; // Sh_2212
        else if (m_showerType == 7) MCshowerID = "700808"; // Sh_2214
        else if (m_showerType == 8) {
            ATH_MSG_WARNING("Unknown Sherpa version detected! Using FTAG MC-MC SFs for Sherpa 2.2.11-2.2.16. Please check if these are applicable for this sample!");
            MCshowerID = "700660"; // Unknown Sherpa Version
        }
        else if (m_showerType != 0){
            ATH_MSG_WARNING("Unknown ShowerID detected! Using default FTAG MC-MC SFs (Powheg+Pythia8). Please check if these are applicable for this sample!");
        }
      }
    } else {
        ATH_MSG_WARNING("Unknown b-tagger detected! Only DL1dv01 and GN2v01 are supported. Falling back to default FTAG MC-MC SFs (Powheg+Pythia8). Please check if these are applicable for this sample!");
        MCshowerID= "default";
    }

    // btagEfficiencyTool
    if (m_useBtagging && !m_btagEffTool.isUserConfigured() && !m_BtagWP.empty()) {
      if (jetcoll != "AntiKt4EMTopoJets" && jetcoll != "AntiKt4EMPFlowJets") {
        ATH_MSG_WARNING("** Only AntiKt4EMTopoJets and AntiKt4EMPFlowJets are supported with FTAG scale factors!");
          return StatusCode::FAILURE;
      }

      toolName = "BTagSF_" + jetcollBTag + m_BtagTagger + m_BtagWP;
      m_btagEffTool.setTypeAndName("BTaggingEfficiencyTool/"+toolName);
      ATH_CHECK( m_btagEffTool.setProperty("TaggerName",     m_BtagTagger ) );
      ATH_CHECK( m_btagEffTool.setProperty("ScaleFactorFileName",  m_bTaggingCalibrationFilePath) );
      ATH_CHECK( m_btagEffTool.setProperty("OperatingPoint", m_BtagWP ) );
      ATH_CHECK( m_btagEffTool.setProperty("JetAuthor",      jetcollBTag ) );
      ATH_CHECK( m_btagEffTool.setProperty("MinPt",          m_BtagMinPt ) );
      ATH_CHECK( m_btagEffTool.setProperty("SystematicsStrategy", m_BtagSystStrategy ) );
      ATH_CHECK( m_btagEffTool.setProperty("EfficiencyBCalibrations",     MCshowerID   ));
      ATH_CHECK( m_btagEffTool.setProperty("EfficiencyCCalibrations",     MCshowerID   ));
      ATH_CHECK( m_btagEffTool.setProperty("EfficiencyTCalibrations",     MCshowerID   ));
      ATH_CHECK( m_btagEffTool.setProperty("EfficiencyLightCalibrations", MCshowerID   ));
      ATH_CHECK( m_btagEffTool.setProperty("EigenvectorReductionB", m_EigenvectorReductionB ));
      ATH_CHECK( m_btagEffTool.setProperty("EigenvectorReductionC", m_EigenvectorReductionC ));
      ATH_CHECK( m_btagEffTool.setProperty("EigenvectorReductionLight", m_EigenvectorReductionLight ));
      ATH_CHECK( m_btagEffTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_btagEffTool.retrieve() );
    } else ATH_CHECK( m_btagEffTool.retrieve() );


    if (m_slices["tjet"]) {
      if (m_useBtagging_trkJet && !m_btagEffTool_trkJet.isUserConfigured() && !m_BtagWP_trkJet.empty()) {
        if (trkjetcoll.find("AntiKt2PV0TrackJets")==std::string::npos && trkjetcoll.find("AntiKtVR30Rmax4Rmin02TrackJets")==std::string::npos) {
          ATH_MSG_WARNING("** Only AntiKt2PV0TrackJets and AntiKtVR30Rmax4Rmin02TrackJets are supported with FTAG scale factors!");
            return StatusCode::FAILURE;
        }

        toolName = "BTagSF_" + trkjetcoll;
        m_btagEffTool_trkJet.setTypeAndName("BTaggingEfficiencyTool/"+toolName);
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("TaggerName",     m_BtagTagger_trkJet ) );
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("ScaleFactorFileName",   m_bTaggingCalibrationFilePath) );
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("OperatingPoint", m_BtagWP_trkJet ) );
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("JetAuthor",      BTagColl_TrkJet ) );
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("MinPt",          m_BtagMinPt_trkJet ) );
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("SystematicsStrategy", m_BtagSystStrategy ) );
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("EfficiencyBCalibrations",     MCshowerID   ));
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("EfficiencyCCalibrations",     MCshowerID   ));
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("EfficiencyTCalibrations",     MCshowerID   ));
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("EfficiencyLightCalibrations", MCshowerID   ));
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("EigenvectorReductionB", m_EigenvectorReductionB ));
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("EigenvectorReductionC", m_EigenvectorReductionC ));
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("EigenvectorReductionLight", m_EigenvectorReductionLight ));
        ATH_CHECK( m_btagEffTool_trkJet.setProperty("OutputLevel", this->msg().level()) );
        ATH_CHECK( m_btagEffTool_trkJet.retrieve() );
      } else if (m_btagEffTool_trkJet.isUserConfigured()) ATH_CHECK( m_btagEffTool_trkJet.retrieve() );
    }
  }


  if (m_slices["met"]) {
  ///////////////////////////////////////////////////////////////////////////////////////////
  // Initialise MET tools

    if (!m_metMaker.isUserConfigured()) {
      toolName = m_doFwdJVT ? m_metJetSelection+"_fJVT" : m_metJetSelection+"_NOfJVT";
      m_metMaker.setTypeAndName("met::METMaker/METMaker_ST_"+toolName);

      ATH_CHECK( m_metMaker.setProperty("ORCaloTaggedMuons", m_metRemoveOverlappingCaloTaggedMuons) );
      ATH_CHECK( m_metMaker.setProperty("DoSetMuonJetEMScale", m_metDoSetMuonJetEMScale) );
      ATH_CHECK( m_metMaker.setProperty("DoRemoveMuonJets", m_metDoRemoveMuonJets) );
      ATH_CHECK( m_metMaker.setProperty("UseGhostMuons", m_metUseGhostMuons) );
      ATH_CHECK( m_metMaker.setProperty("DoMuonEloss", m_metDoMuonEloss) );
      ATH_CHECK( m_metMaker.setProperty("GreedyPhotons", m_metGreedyPhotons) );
      ATH_CHECK( m_metMaker.setProperty("VeryGreedyPhotons", m_metVeryGreedyPhotons) );

      // set the jet selection if default empty string is overridden through config file
      if (m_metJetSelection.size()) {
        ATH_CHECK( m_metMaker.setProperty("JetSelection", m_metJetSelection) );
      }
      if (m_jetInputType == xAOD::JetInput::EMPFlow) {
        ATH_CHECK( m_metMaker.setProperty("DoPFlow", true) );
      }

      ATH_CHECK( m_metMaker.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_metMaker.retrieve() );
    } else ATH_CHECK( m_metMaker.retrieve() );


    if (!m_metSystTool.isUserConfigured()) {
      m_metSystTool.setTypeAndName("met::METSystematicsTool/METSystTool");
      ATH_CHECK( m_metSystTool.setProperty("ConfigPrefix", m_metsysConfigPrefix) );

      if (m_trkMETsyst && m_caloMETsyst){
        ATH_MSG_ERROR( "Can only have CST *or* TST configured for MET maker.  Please unset either METDoCaloSyst or METDoTrkSyst in your config file" );
        return StatusCode::FAILURE;
      }

      if (m_trkMETsyst) {
        ATH_CHECK( m_metSystTool.setProperty("ConfigSoftCaloFile", "") );
        ATH_CHECK( m_metSystTool.setProperty("ConfigSoftTrkFile", "TrackSoftTerms-pflow.config") );
      }

      if (m_caloMETsyst) {
        ATH_MSG_WARNING( "CST is no longer recommended by Jet/MET group");
        ATH_CHECK( m_metSystTool.setProperty("ConfigSoftTrkFile", "") );
      }

      if (m_trkJetsyst) {
        ATH_CHECK( m_metSystTool.setProperty("ConfigJetTrkFile", "JetTrackSyst.config") );
      }

      ATH_CHECK( m_metSystTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_metSystTool.retrieve());
    } else ATH_CHECK( m_metSystTool.retrieve());


    if (!m_metSignif.isUserConfigured()) {
      // See https://twiki.cern.ch/twiki/bin/view/AtlasProtected/MetSignificance
      m_metSignif.setTypeAndName("met::METSignificance/metSignificance_"+jetname);
#ifndef XAOD_STANDALONE // Athena and AthAnalysis; need to take into account that MuonCalibTool is private tool
      ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "SoftTermParam", m_softTermParam));
      ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "TreatPUJets", m_treatPUJets));
      ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "DoPhiReso", m_doPhiReso));
      ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "IsAFII", isAtlfast()));
      if(jetname == "AntiKt4EMTopo" || jetname =="AntiKt4EMPFlow"){
        ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "JetCollection", jetname) );
      } else {
          ATH_MSG_WARNING("Object-based METSignificance recommendations only exist for EMTopo and PFlow, falling back to AntiKt4EMTopo");
          ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "JetCollection", "AntiKt4EMTopo") );
      }
      // setup a dedicated new muon calib tool for passing down to METSignificance
      ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "MuonCalibTool", "CP::MuonCalibTool/calibTool"));
      if (m_isRun3)
        ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "calibTool.IsRun3Geo", true));
      ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "calibTool.calibMode", m_muCalibrationMode));
      ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "calibTool.OutputLevel",this->msg().level()));
      ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "calibTool.release", "Recs2024_05_06_Run2Run3"));
      int IdBaselineInt = m_muIdBaseline;
      if (IdBaselineInt == 4)
        ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "calibTool.do2StationsHighPt", true));
      ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "calibTool.doExtraSmearing", m_muHighPtExtraSmear));
      ATH_CHECK( AAH::setProperty(m_metSignif.getHandle(), "OutputLevel", this->msg().level()));
#else // AnalysisBase; can just pass the muon calib tool configured above
      ATH_CHECK( m_metSignif.setProperty("SoftTermParam", m_softTermParam) );
      ATH_CHECK( m_metSignif.setProperty("TreatPUJets", m_treatPUJets) );
      ATH_CHECK( m_metSignif.setProperty("DoPhiReso", m_doPhiReso) );
      ATH_CHECK( m_metSignif.setProperty("IsAFII", isAtlfast()) );
      if(jetname == "AntiKt4EMTopo" || jetname =="AntiKt4EMPFlow"){
        ATH_CHECK( m_metSignif.setProperty("JetCollection", jetname) );
      } else {
        ATH_MSG_WARNING("Object-based METSignificance recommendations only exist for EMTopo and PFlow, falling back to AntiKt4EMTopo");
        ATH_CHECK( m_metSignif.setProperty("JetCollection", "AntiKt4EMTopo") );
      }
      // just pass the muon calib tool
      ATH_CHECK( m_metSignif.setProperty("MuonCalibTool",m_muonCalibTool.getHandle()));
      ATH_CHECK( m_metSignif.setProperty("OutputLevel", this->msg().level()) );
#endif
      ATH_CHECK( m_metSignif.retrieve() );
    } else ATH_CHECK( m_metSignif.retrieve() );
  }


///////////////////////////////////////////////////////////////////////////////////////////
// Initialise trigger tools

  if (!m_trigDecTool.isUserConfigured()) {

    // The decision tool
    m_trigDecTool.setTypeAndName("Trig::TrigDecisionTool/TrigDecisionTool");
    ATH_CHECK( m_trigDecTool.setProperty("TrigDecisionKey", "xTrigDecision") );
    if (m_isRun3) {
      ATH_CHECK( m_trigDecTool.setProperty("NavigationFormat", "TrigComposite") );
      ATH_CHECK( m_trigDecTool.setProperty("HLTSummary", "HLTNav_Summary_DAODSlimmed") );
    }
    ATH_CHECK( m_trigDecTool.setProperty("OutputLevel", this->msg().level()) );

#ifndef XAOD_STANDALONE // Athena and AthAnalysis
    // Clear the default tool handle to cause a fallback on the config service
    ATH_CHECK( m_trigDecTool.setProperty("ConfigTool", ""));
#else // AnalysisBase
    m_trigConfTool.setTypeAndName("TrigConf::xAODConfigTool/xAODConfigTool");
    ATH_CHECK(m_trigConfTool.retrieve() );
    ATH_CHECK( m_trigDecTool.setProperty("ConfigTool", m_trigConfTool.getHandle()) );
#endif

    ATH_CHECK( m_trigDecTool.retrieve() );

  } else  ATH_CHECK( m_trigDecTool.retrieve() );

  if (m_isRun3) {
    if (!m_trigDRScoringTool.isUserConfigured()) {
      m_trigDRScoringTool.setTypeAndName("Trig::DRScoringTool/TrigDRScoringTool");
      ATH_CHECK( m_trigDRScoringTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_trigDRScoringTool.retrieve() );
    } else  ATH_CHECK( m_trigDRScoringTool.retrieve() );
  }

  if (!m_trigMatchingTool.isUserConfigured()) {
    if (m_upstreamTriggerMatching){
      m_trigMatchingTool.setTypeAndName("Trig::MatchFromCompositeTool/TrigMatchFromCompositeTool");
      ATH_CHECK( m_trigMatchingTool.setProperty("InputPrefix", m_trigMatchingPrefix) );
      ATH_CHECK( m_trigMatchingTool.setProperty("RemapBrokenLinks", true) );
    }
    else {
      if (m_isRun3){
        m_trigMatchingTool.setTypeAndName("Trig::R3MatchingTool/TrigR3MatchingTool");
        ATH_CHECK( m_trigMatchingTool.setProperty("ScoringTool", m_trigDRScoringTool.getHandle()) );
        ATH_CHECK( m_trigMatchingTool.setProperty("TrigDecisionTool", m_trigDecTool.getHandle()) );
      }
      else {
        m_trigMatchingTool.setTypeAndName("Trig::MatchingTool/TrigMatchingTool");
        ATH_CHECK( m_trigMatchingTool.setProperty("TrigDecisionTool", m_trigDecTool.getHandle()) );
      }
    }
    ATH_CHECK( m_trigMatchingTool.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_trigMatchingTool.retrieve() );
  } else  ATH_CHECK( m_trigMatchingTool.retrieve() );

///////////////////////////////////////////////////////////////////////////////////////////
// Initialise trigGlobalEfficiencyCorrection tool

  if (!m_trigGlobalEffCorrTool_diLep.isUserConfigured() && !isData()) {

    std::string no2e17("");
    if (m_trig2017combination_diLep.find("||2e17_lhvloose_nod0_L12EM15VHI") != std::string::npos) {
      auto pos_2e17 = m_trig2017combination_diLep.find("||2e17_lhvloose_nod0_L12EM15VHI");
      no2e17 = m_trig2017combination_diLep.substr(0, pos_2e17) + m_trig2017combination_diLep.substr(pos_2e17+31, m_trig2017combination_diLep.size());
    } else if (m_trig2017combination_diLep.find("2e17_lhvloose_nod0_L12EM15VHI||") != std::string::npos) {
      auto pos_2e17 = m_trig2017combination_diLep.find("2e17_lhvloose_nod0_L12EM15VHI||");
      no2e17 = m_trig2017combination_diLep.substr(0, pos_2e17) + m_trig2017combination_diLep.substr(pos_2e17+31, m_trig2017combination_diLep.size());
    } else {
      no2e17 = m_trig2017combination_diLep;
    }
    ATH_MSG_DEBUG( "TrigGlobalEfficiencyCorrectionTool/TrigGlobal_diLep: no2e17 trigger string: " << no2e17 );

    std::map<std::string,std::string> triggers_diLep;
    triggers_diLep["2015"] = m_trig2015combination_diLep;
    triggers_diLep["2016"] = m_trig2016combination_diLep;
    triggers_diLep["324320-326695"] = m_trig2017combination_diLep; // 2017 before accidental prescale of L12EM15VHI
    triggers_diLep["326834-328393"] = no2e17;                      // 2017 during accidental prescale
    triggers_diLep["329385-340453"] = m_trig2017combination_diLep; // 2017 after accidental prescale
    triggers_diLep["2018"] = m_trig2018combination_diLep;

    m_trigGlobalEffCorrTool_diLep.setTypeAndName("TrigGlobalEfficiencyCorrectionTool/TrigGlobal_diLep");
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("ElectronEfficiencyTools", m_elecTrigEffTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("ElectronScaleFactorTools", m_elecTrigSFTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("MuonTools", m_muonTrigSFTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("TriggerCombination", triggers_diLep) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("TriggerMatchingTool", m_trigMatchingTool.getHandle()) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("ListOfLegsPerTool", m_legsPerTool) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("NumberOfToys", m_trigNToys_diLep) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diLep.initialize() );
  } else if (m_trigGlobalEffCorrTool_diLep.isUserConfigured()) ATH_CHECK( m_trigGlobalEffCorrTool_diLep.retrieve() );


  if (!m_trigGlobalEffCorrTool_multiLep.isUserConfigured() && !isData()) {

    std::string no2e17("");
    if (m_trig2017combination_multiLep.find("||2e17_lhvloose_nod0_L12EM15VHI") != std::string::npos) {
      auto pos_2e17 = m_trig2017combination_multiLep.find("||2e17_lhvloose_nod0_L12EM15VHI");
      no2e17 = m_trig2017combination_multiLep.substr(0, pos_2e17) + m_trig2017combination_multiLep.substr(pos_2e17+31, m_trig2017combination_multiLep.size());
    } else if (m_trig2017combination_multiLep.find("2e17_lhvloose_nod0_L12EM15VHI||") != std::string::npos) {
      auto pos_2e17 = m_trig2017combination_multiLep.find("2e17_lhvloose_nod0_L12EM15VHI||");
      no2e17 = m_trig2017combination_multiLep.substr(0, pos_2e17) + m_trig2017combination_multiLep.substr(pos_2e17+31, m_trig2017combination_multiLep.size());
    } else {
      no2e17 = m_trig2017combination_multiLep;
    }
    ATH_MSG_DEBUG( "TrigGlobalEfficiencyCorrectionTool/TrigGlobal_multiLep: no2e17 trigger string: " << no2e17 );

    std::map<std::string,std::string> triggers_multiLep;
    triggers_multiLep["2015"] = m_trig2015combination_multiLep;
    triggers_multiLep["2016"] = m_trig2016combination_multiLep;
    triggers_multiLep["324320-326695"] = m_trig2017combination_multiLep; // 2017 before accidental prescale of L12EM15VHI
    triggers_multiLep["326834-328393"] = no2e17;                         // 2017 during accidental prescale
    triggers_multiLep["329385-340453"] = m_trig2017combination_multiLep; // 2017 after accidental prescale
    triggers_multiLep["2018"] = m_trig2018combination_multiLep;

    m_trigGlobalEffCorrTool_multiLep.setTypeAndName("TrigGlobalEfficiencyCorrectionTool/TrigGlobal_multiLep");
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("ElectronEfficiencyTools", m_elecTrigEffTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("ElectronScaleFactorTools", m_elecTrigSFTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("MuonTools", m_muonTrigSFTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("TriggerCombination", triggers_multiLep) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("TriggerMatchingTool", m_trigMatchingTool.getHandle()) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("ListOfLegsPerTool", m_legsPerTool) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("NumberOfToys", m_trigNToys_multiLep) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.initialize() );
  } else if (m_trigGlobalEffCorrTool_multiLep.isUserConfigured()) ATH_CHECK( m_trigGlobalEffCorrTool_multiLep.retrieve() );


  if (!m_trigGlobalEffCorrTool_diPhoton.isUserConfigured() && !isData()) {
    m_trigGlobalEffCorrTool_diPhoton.setTypeAndName("TrigGlobalEfficiencyCorrectionTool/TrigGlobal_diPhoton");
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("PhotonEfficiencyTools", m_photonTrigEffTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("PhotonScaleFactorTools", m_photonTrigSFTools) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("TriggerCombination2015", m_trig2015combination_diPhoton) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("TriggerCombination2016", m_trig2016combination_diPhoton) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("TriggerCombination2017", m_trig2017combination_diPhoton) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("TriggerCombination2018", m_trig2018combination_diPhoton) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("ListOfLegsPerTool", m_legsPerTool_ph) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("NumberOfToys", m_trigNToys_diPhoton) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.initialize() );
  } else if (m_trigGlobalEffCorrTool_diPhoton.isUserConfigured()) ATH_CHECK( m_trigGlobalEffCorrTool_diPhoton.retrieve() );


  if (m_slices["ele"] || m_slices["pho"] || m_slices["mu"]) {
  // /////////////////////////////////////////////////////////////////////////////////////////
  // Initialise Isolation Correction Tool

    if ( !m_isoCorrTool.isUserConfigured() ) {
      m_isoCorrTool.setTypeAndName("CP::IsolationCorrectionTool/IsoCorrTool");
      ATH_CHECK( m_isoCorrTool.setProperty( "ToolVer", "REL22") );
      ATH_CHECK( m_isoCorrTool.setProperty( "IsMC", !isData()) );
      ATH_CHECK( m_isoCorrTool.setProperty( "AFII_corr", isAtlfast()) );
      ATH_CHECK( m_isoCorrTool.setProperty( "Apply_SC_leakcorr", false) );
      ATH_CHECK( m_isoCorrTool.setProperty( "CorrFile", "IsolationCorrections/v6/isolation_ptcorrections_rel22_mc20.root") );
      ATH_CHECK( m_isoCorrTool.setProperty( "OutputLevel", this->msg().level()) );
      ATH_CHECK( m_isoCorrTool.retrieve() );
    } else  ATH_CHECK( m_isoCorrTool.retrieve() );


  // /////////////////////////////////////////////////////////////////////////////////////////
  // Initialise Isolation Tool
    if (!m_isoTool.isUserConfigured()) {
      m_isoTool.setTypeAndName("CP::IsolationSelectionTool/IsoTool");
      ATH_CHECK( m_isoTool.setProperty("ElectronWP", m_eleIso_WP.empty()    ? "Loose_VarRad" : m_eleIso_WP) );
      ATH_CHECK( m_isoTool.setProperty("MuonWP",     m_muIso_WP.empty()     ? "Loose_VarRad" : m_muIso_WP) );
      ATH_CHECK( m_isoTool.setProperty("PhotonWP",   m_photonIso_WP.empty() ? "FixedCutTight" : m_photonIso_WP ) );
      ATH_CHECK( m_isoTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_isoTool.retrieve() );
    } else  ATH_CHECK( m_isoTool.retrieve() );

    //disable if (!m_isoToolLowPtPLV.isUserConfigured()) {
    //disable   m_isoToolLowPtPLV.setTypeAndName("CP::IsolationLowPtPLVTool/IsoToolLowPtPLV");
    //disable   ATH_CHECK( m_isoToolLowPtPLV.setProperty("OutputLevel", this->msg().level()) );
    //disable   ATH_CHECK( m_isoToolLowPtPLV.retrieve() );
    //disable } else  ATH_CHECK( m_isoToolLowPtPLV.retrieve() );


    if (!m_isoBaselineTool.isUserConfigured()) {
      m_isoBaselineTool.setTypeAndName("CP::IsolationSelectionTool/IsoBaselineTool");
      ATH_CHECK( m_isoBaselineTool.setProperty("ElectronWP", m_eleBaselineIso_WP.empty()    ? "Loose_VarRad" : m_eleBaselineIso_WP    ) );
      ATH_CHECK( m_isoBaselineTool.setProperty("MuonWP",     m_muBaselineIso_WP.empty()     ? "Loose_VarRad" : m_muBaselineIso_WP     ) );
      ATH_CHECK( m_isoBaselineTool.setProperty("PhotonWP",   m_photonBaselineIso_WP.empty() ? "FixedCutTight" : m_photonBaselineIso_WP ) );
      ATH_CHECK( m_isoBaselineTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_isoBaselineTool.retrieve() );
    } else ATH_CHECK( m_isoBaselineTool.retrieve() );


    if (!m_isoHighPtTool.isUserConfigured()) {
      m_isoHighPtTool.setTypeAndName("CP::IsolationSelectionTool/IsoHighPtTool");
      ATH_CHECK( m_isoHighPtTool.setProperty("ElectronWP", m_eleIsoHighPt_WP.empty() ? "Loose_VarRad" : m_eleIsoHighPt_WP) );
      ATH_CHECK( m_isoHighPtTool.setProperty("MuonWP",     m_muIsoHighPt_WP.empty()  ? "Loose_VarRad" : m_muIsoHighPt_WP ) );
      ATH_CHECK( m_isoHighPtTool.setProperty("PhotonWP",   m_photonIso_WP.empty()    ? "FixedCutTight" : m_photonIso_WP ) );
      ATH_CHECK( m_isoHighPtTool.setProperty("OutputLevel", this->msg().level()) );
      ATH_CHECK( m_isoHighPtTool.retrieve() );
    } else ATH_CHECK( m_isoHighPtTool.retrieve() );


  // /////////////////////////////////////////////////////////////////////////////////////////
  // Initialise IsolationCloseByCorrectionTool Tool
    if (!m_isoCloseByTool.isUserConfigured()) {
      m_isoCloseByTool.setTypeAndName("CP::IsolationCloseByCorrectionTool/IsoCloseByTool");
      // Actually we could debate about what is the proper tool to choose if the users have different baseline & signal islation WP's
      ATH_CHECK( m_isoCloseByTool.setProperty("IsolationSelectionTool", m_useSigLepForIsoCloseByOR ? m_isoTool : m_isoBaselineTool));
      ATH_CHECK( m_isoCloseByTool.setProperty("PassoverlapDecorator", m_IsoCloseByORpassLabel) );
      ATH_CHECK( m_isoCloseByTool.setProperty("SelectionDecorator", m_useSigLepForIsoCloseByOR ? "signal" : "baseline") );
      // Make this propery configurable as well?
      ATH_CHECK( m_isoCloseByTool.setProperty("BackupPrefix", "ORIG") );
      // The isolation selection decorator is updated as well by the tool
      ATH_CHECK( m_isoCloseByTool.setProperty("IsolationSelectionDecorator", "isol") );

      ATH_CHECK( m_isoCloseByTool.setProperty("OutputLevel", this->msg().level()) );
#ifndef XAOD_ANALYSIS
      ATH_CHECK( m_isoCloseByTool.setProperty("ParticleCaloExtensionTool", "") );
#endif
      ATH_CHECK( m_isoCloseByTool.retrieve() );
    } else  ATH_CHECK( m_isoCloseByTool.retrieve() );
  }


// /////////////////////////////////////////////////////////////////////////////////////////
// Initialise Overlap Removal Tool
  if ( m_orToolbox.masterTool.empty() ){

    // set up the master tool
    std::string suffix = "";
    if (m_orDoTau) suffix += "Tau";
    if (m_orDoPhoton) suffix += "Gamma";
    if (m_orDoBjet) suffix += "Bjet";
    std::string toolName = "ORTool" + suffix;
    ATH_MSG_INFO("SUSYTools: Autoconfiguring " << toolName);

    std::string bJetLabel = "";
    //overwrite lepton flags if the global is false (yes?)
    if (!m_orDoBjet || !m_useBtagging) {
      m_orDoElBjet = false;
      m_orDoMuBjet = false;
      m_orDoTauBjet = false;
    }
    if (m_orDoElBjet || m_orDoMuBjet || m_orDoTauBjet) {
      bJetLabel = "bjet_loose";
    }

    // Set the generic flags
    ORUtils::ORFlags orFlags(toolName, m_orInputLabel, "passOR");
    orFlags.bJetLabel      = bJetLabel;
    orFlags.boostedLeptons = (m_orDoBoostedElectron || m_orDoBoostedMuon);
    orFlags.outputPassValue = true;
    orFlags.linkOverlapObjects = m_orLinkOverlapObjects;
    if (m_orDoElEl) {
      orFlags.doEleEleOR = true;
    } else orFlags.doEleEleOR = false;
    orFlags.doElectrons = true;
    orFlags.doMuons = true;
    orFlags.doJets = true;
    orFlags.doTaus = m_orDoTau;
    orFlags.doPhotons = m_orDoPhoton;
    orFlags.doFatJets = m_orDoFatjets;

    //set up all recommended tools
    ATH_CHECK( ORUtils::recommendedTools(orFlags, m_orToolbox));

    // We don't currently have a good way to determine here which object
    // definitions are disabled, so we currently just configure all overlap
    // tools and disable the pointer safety checks
    ATH_CHECK( m_orToolbox.setGlobalProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_orToolbox.masterTool.setProperty("RequireExpectedPointers", false) );

    // Override boosted OR sliding cone options
    ATH_CHECK( m_orToolbox.eleJetORT.setProperty("UseSlidingDR", m_orDoBoostedElectron) );
    ATH_CHECK( m_orToolbox.muJetORT.setProperty("UseSlidingDR", m_orDoBoostedMuon) );

    //add custom tau-jet OR tool
    if(m_orDoTau){
      m_orToolbox.tauJetORT.setTypeAndName("ORUtils::TauJetOverlapTool/" + orFlags.masterName + ".TauJetORT");
      ATH_CHECK( m_orToolbox.tauJetORT.setProperty("BJetLabel", m_orDoTauBjet?bJetLabel:"") );
    }

    // override sliding cone params if sliding dR is on and the user-provided parameter values are non-negative
    if (m_orDoBoostedElectron) {
      if (m_orBoostedElectronC1 > 0) ATH_CHECK( m_orToolbox.eleJetORT.setProperty("SlidingDRC1", m_orBoostedElectronC1) );
      if (m_orBoostedElectronC2 > 0) ATH_CHECK( m_orToolbox.eleJetORT.setProperty("SlidingDRC2", m_orBoostedElectronC2) );
      if (m_orBoostedElectronMaxConeSize > 0) ATH_CHECK( m_orToolbox.eleJetORT.setProperty("SlidingDRMaxCone", m_orBoostedElectronMaxConeSize) );
    }
    if (m_orDoBoostedMuon) {
      if (m_orBoostedMuonC1 > 0) ATH_CHECK( m_orToolbox.muJetORT.setProperty("SlidingDRC1", m_orBoostedMuonC1) );
      if (m_orBoostedMuonC2 > 0) ATH_CHECK( m_orToolbox.muJetORT.setProperty("SlidingDRC2", m_orBoostedMuonC2) );
      if (m_orBoostedMuonMaxConeSize > 0) ATH_CHECK( m_orToolbox.muJetORT.setProperty("SlidingDRMaxCone", m_orBoostedMuonMaxConeSize) );
    }

    // and switch off lep-bjet check if not requested
    if (!m_orDoElBjet) {
      ATH_CHECK(m_orToolbox.eleJetORT.setProperty("BJetLabel", ""));
    }
    if (!m_orDoMuBjet) {
      ATH_CHECK(m_orToolbox.muJetORT.setProperty("BJetLabel", ""));
    }

    // propagate the mu-jet ghost-association option which might be set by the user (default is true)
    ATH_CHECK(m_orToolbox.muJetORT.setProperty("UseGhostAssociation", m_orDoMuonJetGhostAssociation));

    // propagate mu-jet OR settings if requested
    ATH_CHECK(m_orToolbox.muJetORT.setProperty("ApplyRelPt", m_orApplyRelPt) );
    if(m_orApplyRelPt){
      if (m_orMuJetPtRatio > 0)    ATH_CHECK(m_orToolbox.muJetORT.setProperty("MuJetPtRatio", m_orMuJetPtRatio) );
      if (m_orMuJetTrkPtRatio > 0) ATH_CHECK(m_orToolbox.muJetORT.setProperty("MuJetTrkPtRatio", m_orMuJetTrkPtRatio) );
    }
    if (m_orMuJetInnerDR > 0)    ATH_CHECK(m_orToolbox.muJetORT.setProperty("InnerDR", m_orMuJetInnerDR) );

    // propagate the calo muon setting for EleMuORT
    ATH_CHECK(m_orToolbox.eleMuORT.setProperty("RemoveCaloMuons", m_orRemoveCaloMuons) );

    // Use electron-muon DR matching to remove electrons within DR <  0.01 of Muons.
    if (m_orDoElMu){
      ATH_CHECK(m_orToolbox.eleMuORT.setProperty("UseDRMatching", m_orDoElMu) );
    }

    // propagate the fatjets OR settings
    if(m_orDoFatjets){
      if(m_EleFatJetDR>0) ATH_CHECK(m_orToolbox.eleFatJetORT.setProperty("DR", m_EleFatJetDR));
      if(m_JetFatJetDR>0) ATH_CHECK(m_orToolbox.jetFatJetORT.setProperty("DR", m_JetFatJetDR));
    }

    // Make sure that we deal with prorities correctly
    ATH_CHECK(m_orToolbox.eleJetORT.setProperty("EnableUserPriority", true));
    ATH_CHECK(m_orToolbox.muJetORT.setProperty("EnableUserPriority", true));
    if (m_orDoTau) ATH_CHECK(m_orToolbox.tauJetORT.setProperty("EnableUserPriority", true));
    if (m_orDoPhoton) ATH_CHECK(m_orToolbox.phoJetORT.setProperty("EnableUserPriority", true));

    if ( m_orPhotonFavoured ) {
       ATH_CHECK(m_orToolbox.phoEleORT.setProperty("SwapContainerPrecedence", true));
       ATH_CHECK(m_orToolbox.phoMuORT.setProperty("SwapContainerPrecedence", true));
    }

    if (!m_orDoEleJet){
      // Disable the electron removal part of e-j overlap removal
      ATH_CHECK( m_orToolbox.eleJetORT.setProperty("OuterDR",-1.) );
      ATH_CHECK( m_orToolbox.eleJetORT.setProperty("SlidingDRMaxCone",-1.) );
    }
    if (!m_orDoMuonJet){
      // Disable the muon removal part of m-j overlap removal
      ATH_CHECK( m_orToolbox.muJetORT.setProperty("OuterDR",-1.) );
      ATH_CHECK( m_orToolbox.muJetORT.setProperty("SlidingDRMaxCone",-1.) );
    }

    ATH_CHECK( m_orToolbox.initialize() );

  }
  // Done with the OR toolbox setup!

// /////////////////////////////////////////////////////////////////////////////////////////
// Initialise PMG Tools
  if (!m_pmgSHnjetWeighter.isUserConfigured() && !isData()) {
    m_pmgSHnjetWeighter.setTypeAndName("PMGTools::PMGSherpa22VJetsWeightTool/PMGSHVjetReweighter");
    ATH_CHECK( m_pmgSHnjetWeighter.setProperty( "TruthJetContainer", "AntiKt4TruthJets"));
    ATH_CHECK( m_pmgSHnjetWeighter.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_pmgSHnjetWeighter.retrieve());
  } else if (m_pmgSHnjetWeighter.isUserConfigured()) ATH_CHECK( m_pmgSHnjetWeighter.retrieve());


  if (!m_pmgSHnjetWeighterWZ.isUserConfigured() && !isData()) {
    m_pmgSHnjetWeighterWZ.setTypeAndName("PMGTools::PMGSherpa22VJetsWeightTool/PMGSHVjetReweighterWZ");
    ATH_CHECK( m_pmgSHnjetWeighterWZ.setProperty( "TruthJetContainer", "AntiKt4TruthWZJets"));
    ATH_CHECK( m_pmgSHnjetWeighterWZ.setProperty("OutputLevel", this->msg().level()) );
    ATH_CHECK( m_pmgSHnjetWeighterWZ.retrieve() );
  } else if (m_pmgSHnjetWeighterWZ.isUserConfigured()) ATH_CHECK( m_pmgSHnjetWeighterWZ.retrieve() );


  // prevent these initialiation snippets from being run again
  m_subtool_init = true;

  ATH_MSG_INFO("Done initialising SUSYTools");

  return StatusCode::SUCCESS;
}
