/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "PATInterfaces/SystematicRegistry.h"
#include "xAODMetaData/FileMetaData.h"

// Local include(s):
#include "TauAnalysisTools/TauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/SharedFilesVersion.h"

namespace TauAnalysisTools
{

//______________________________________________________________________________
TauEfficiencyCorrectionsTool::TauEfficiencyCorrectionsTool( const std::string& sName )
  : asg::AsgMetadataTool( sName )
  , m_vCommonEfficiencyTools()
  , m_vTriggerEfficiencyTools()
  , m_bIsData(false)
  , m_bIsConfigured(false)
  , m_iRunNumber(0)
  , m_iMu(0)
{
  declareProperty( "EfficiencyCorrectionTypes",    m_vEfficiencyCorrectionTypes    = {} );
}


//______________________________________________________________________________
TauEfficiencyCorrectionsTool::~TauEfficiencyCorrectionsTool()
{
  for (auto tTool : m_vCommonEfficiencyTools)
    delete tTool;
  for (auto tTool : m_vTriggerEfficiencyTools)
    delete tTool;
}

//______________________________________________________________________________
StatusCode TauEfficiencyCorrectionsTool::initialize()
{
  ATH_MSG_INFO( "Initializing TauEfficiencyCorrectionsTool" );

  if (m_bSkipTruthMatchCheck)
    ATH_MSG_WARNING("Truth match check will be skipped. This is ONLY FOR TESTING PURPOSE!");

  // check efficiency correction type
  if (m_vEfficiencyCorrectionTypes.empty())
  {
    ATH_MSG_ERROR("Could not retrieve any EfficiencyCorrectionType");
    return StatusCode::FAILURE;
  }
  
  if(m_sRecommendationTag == "2025-prerec") {
    ATH_MSG_WARNING("2025-prerec are available. Tau trigger recommendations for Run2 still missing");
    ATH_CHECK(initializeTools_2025_prerec());
  } else if (m_sRecommendationTag == "2022-prerec") {
    ATH_MSG_WARNING("2022-prerec tag are pre-recommendations and still under development.");
    ATH_CHECK(initializeTools_2022_prerec());
  }
  else {
    ATH_MSG_ERROR("Unknown RecommendationTag " << m_sRecommendationTag);
    return StatusCode::FAILURE;
  }

  // for (auto tCommonEfficiencyTool : m_vCommonEfficiencyTools)
  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
  {
    ATH_CHECK((**it).setProperty("OutputLevel", this->msg().level()));
    ATH_CHECK((**it).initialize());
  }
  for (auto it = m_vTriggerEfficiencyTools.begin(); it != m_vTriggerEfficiencyTools.end(); it++)
  {
    ATH_CHECK((**it).setProperty("OutputLevel", this->msg().level()));
    ATH_CHECK((**it).initialize());
  }

  // Add the affecting systematics to the global registry
  CP::SystematicRegistry& registry = CP::SystematicRegistry::getInstance();
  if (!registry.registerSystematics(*this))
  {
    ATH_MSG_ERROR ("Unable to register the systematics");
    return StatusCode::FAILURE;
  }

  printConfig();

  return StatusCode::SUCCESS;
}

StatusCode TauEfficiencyCorrectionsTool::firstEvent() 
{
  const xAOD::EventInfo* xEventInfo = nullptr;
  ATH_CHECK(evtStore()->retrieve(xEventInfo, "EventInfo"));

  if(m_sRecommendationTag == "2022-prerec"){
    if (xEventInfo->runNumber() != 284500 && xEventInfo->runNumber() != 300000 && xEventInfo->runNumber() != 310000)  // mc21/2022: 410000
      {
        ANA_MSG_WARNING( "Could not determine MC campaign from run number! The mu dependent systematic of the trigger scale factors should not be trusted. Current (" << xEventInfo->runNumber() << "). Will only print this warning once." );
      }
    if (xEventInfo->runNumber() < 410000)  // mc21/2022: 410000
      {
        ANA_MSG_WARNING( "TauEfficiency callibrations from 2022-prerec are not recommended for Run2 MC. Will only print this warning once." );
      }
  }
  return StatusCode::SUCCESS;
}

StatusCode TauEfficiencyCorrectionsTool::beginEvent()
{
  if (!m_bIsConfigured)
  {
    const xAOD::EventInfo* xEventInfo = nullptr;
    ATH_CHECK(evtStore()->retrieve(xEventInfo,"EventInfo"));
    m_bIsData = !(xEventInfo->eventType( xAOD::EventInfo::IS_SIMULATION));
    m_bIsConfigured = true;
  }

  if (!m_firstEvent){
    ATH_CHECK(firstEvent());
    m_firstEvent = true;
      
  }
  if (m_bIsData)
    return StatusCode::SUCCESS;

  const xAOD::EventInfo* xEventInfo = nullptr;
  ATH_CHECK(evtStore()->retrieve(xEventInfo, "EventInfo"));
  m_iMu = xEventInfo->averageInteractionsPerCrossing();

  if (m_bReadRandomRunNumber)
  {
    // Reset the number at the beginning of event
    m_iRunNumber = 0;
  }

  return StatusCode::SUCCESS;
}

//______________________________________________________________________________
void TauEfficiencyCorrectionsTool::printConfig() const
{
  ATH_MSG_DEBUG( "TauEfficiencyCorrectionsTool with name " << name() << " is configured as follows:" );
  for (auto iEfficiencyCorrectionType : m_vEfficiencyCorrectionTypes) {
    ATH_MSG_DEBUG( "  EfficiencyCorrectionTypes " << iEfficiencyCorrectionType );
  }
  ATH_MSG_DEBUG( "  InputFilePathRecoHadTau " << m_sInputFilePathRecoHadTau );
  ATH_MSG_DEBUG( "  InputFilePathEleIDHadTau " << m_sInputFilePathEleIDHadTau );
  ATH_MSG_DEBUG( "  InputFilePathEleIDElectron " << m_sInputFilePathEleIDElectron );
  ATH_MSG_DEBUG( "  InputFilePathJetIDHadTau " << m_sInputFilePathJetIDHadTau );
  ATH_MSG_DEBUG( "  InputFilePathDecayModeHadTau " << m_sInputFilePathDecayModeHadTau );
  ATH_MSG_DEBUG( "  InputFilePathTriggerHadTau " << m_sInputFilePathTriggerHadTau );
  ATH_MSG_DEBUG( "  VarNameRecoHadTau " << m_sVarNameRecoHadTau );
  ATH_MSG_DEBUG( "  VarNameEleIDHadTau " << m_sVarNameEleIDHadTau );
  ATH_MSG_DEBUG( "  VarNameEleIDElectron " << m_sVarNameEleIDElectron );
  ATH_MSG_DEBUG( "  VarNameJetIDHadTau " << m_sVarNameJetIDHadTau );
  ATH_MSG_DEBUG( "  VarNameDecayModeHadTau " << m_sVarNameDecayModeHadTau );
  ATH_MSG_DEBUG( "  VarNameTriggerHadTau " << m_sVarNameTriggerHadTau );
  ATH_MSG_DEBUG( "  RecommendationTag " << m_sRecommendationTag );
  ATH_MSG_DEBUG( "  TriggerName " << m_sTriggerName );
  ATH_MSG_DEBUG( "  UseTauSubstructure " << m_bUseTauSubstructure );
  ATH_MSG_DEBUG( "  JetIDLevel " << m_iJetIDLevel );
  ATH_MSG_DEBUG( "  EleIDLevel " << m_iEleIDLevel );
  ATH_MSG_DEBUG( "  Campaign " << m_sCampaign );
  ATH_MSG_DEBUG( "  useFastSim " << m_useFastSim);
}

//______________________________________________________________________________
CP::CorrectionCode TauEfficiencyCorrectionsTool::getEfficiencyScaleFactor( const xAOD::TauJet& xTau,
    double& eff, unsigned int /*iRunNumber*/, unsigned int /*iMu*/)
{
  eff = 1.;

  if (m_bIsData)
    return CP::CorrectionCode::Ok;

  ANA_CHECK_SET_TYPE (CP::CorrectionCode);
  ANA_CHECK(readRandomRunNumber());

  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
  {
    if ( !(**it)->isSupportedRunNumber(m_iRunNumber) )
      continue;
    double dToolEff = 1.;
    CP::CorrectionCode tmpCorrectionCode = (**it)->getEfficiencyScaleFactor(xTau, dToolEff, m_iRunNumber, m_iMu);
    if (tmpCorrectionCode != CP::CorrectionCode::Ok)
      return tmpCorrectionCode;
    eff *= dToolEff;
  }
  for (auto it = m_vTriggerEfficiencyTools.begin(); it != m_vTriggerEfficiencyTools.end(); it++)
  {
    if ( !(**it)->isSupportedRunNumber(m_iRunNumber) )
      continue;
    double dToolEff = 1.;
    CP::CorrectionCode tmpCorrectionCode = (**it)->getEfficiencyScaleFactor(xTau, dToolEff);
    if (tmpCorrectionCode != CP::CorrectionCode::Ok)
      return tmpCorrectionCode;
    eff *= dToolEff;
  }
  return CP::CorrectionCode::Ok;
}

//______________________________________________________________________________
CP::CorrectionCode TauEfficiencyCorrectionsTool::applyEfficiencyScaleFactor( const xAOD::TauJet& xTau, unsigned int /*iRunNumber*/, unsigned int /*iMu*/)
{
  if (m_bIsData)
    return CP::CorrectionCode::Ok;

  ANA_CHECK_SET_TYPE (CP::CorrectionCode);
  ANA_CHECK(readRandomRunNumber());

  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
  {
    CP::CorrectionCode tmpCorrectionCode = (**it)->applyEfficiencyScaleFactor(xTau, m_iRunNumber, m_iMu);
    if (tmpCorrectionCode != CP::CorrectionCode::Ok)
      return tmpCorrectionCode;
  }
  for (auto it = m_vTriggerEfficiencyTools.begin(); it != m_vTriggerEfficiencyTools.end(); it++)
  {
    if ( !(**it)->isSupportedRunNumber(m_iRunNumber) )
      continue;
    CP::CorrectionCode tmpCorrectionCode = (**it)->applyEfficiencyScaleFactor(xTau, m_iRunNumber, m_iMu);
    if (tmpCorrectionCode != CP::CorrectionCode::Ok)
      return tmpCorrectionCode;
  }
  return CP::CorrectionCode::Ok;
}

/// returns: whether this tool is affected by the given systematics
//______________________________________________________________________________
bool TauEfficiencyCorrectionsTool::isAffectedBySystematic( const CP::SystematicVariation& systematic ) const
{
  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
    if ((**it)->isAffectedBySystematic(systematic))
      return true;
  for (auto it = m_vTriggerEfficiencyTools.begin(); it != m_vTriggerEfficiencyTools.end(); it++)
    if ((**it)->isAffectedBySystematic(systematic))
      return true;
  return false;
}

/// returns: the list of all systematics this tool can be affected by
//______________________________________________________________________________
CP::SystematicSet TauEfficiencyCorrectionsTool::affectingSystematics() const
{
  CP::SystematicSet sAffectingSystematics;
  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
    sAffectingSystematics.insert((**it)->affectingSystematics());
  for (auto it = m_vTriggerEfficiencyTools.begin(); it != m_vTriggerEfficiencyTools.end(); it++)
    sAffectingSystematics.insert((**it)->affectingSystematics());
  return sAffectingSystematics;
}

/// returns: the list of all systematics this tool recommends to use
//______________________________________________________________________________
CP::SystematicSet TauEfficiencyCorrectionsTool::recommendedSystematics() const
{
  CP::SystematicSet sRecommendedSystematics;
  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
    sRecommendedSystematics.insert((**it)->recommendedSystematics());
  for (auto it = m_vTriggerEfficiencyTools.begin(); it != m_vTriggerEfficiencyTools.end(); it++)
    sRecommendedSystematics.insert((**it)->recommendedSystematics());
  return sRecommendedSystematics;
}

//______________________________________________________________________________
StatusCode TauEfficiencyCorrectionsTool::applySystematicVariation ( const CP::SystematicSet& sSystematicSet)
{
  for (auto it = m_vCommonEfficiencyTools.begin(); it != m_vCommonEfficiencyTools.end(); it++)
    if ((**it)->applySystematicVariation(sSystematicSet) == StatusCode::FAILURE)
      return StatusCode::FAILURE;
  for (auto it = m_vTriggerEfficiencyTools.begin(); it != m_vTriggerEfficiencyTools.end(); it++)
    if ((**it)->applySystematicVariation(sSystematicSet) == StatusCode::FAILURE)
      return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

//=================================PRIVATE-PART=================================
StatusCode TauEfficiencyCorrectionsTool::initializeTools_2025_prerec()
{
  std::string sDirectory = "TauAnalysisTools/" + std::string(sSharedFilesVersion) + "/EfficiencyCorrections/";
  for (auto iEfficiencyCorrectionType : m_vEfficiencyCorrectionTypes){

    if (iEfficiencyCorrectionType == SFJetIDHadTau)
    {
      if (m_sInputFilePathJetIDHadTau.empty()) {
        if(m_useFastSim) {
          ATH_MSG_WARNING("No fast-sim recommendation for Tau RNN, using full sim");
        }

	if(m_sCampaign=="mc23"){  
            m_sInputFilePathJetIDHadTau = sDirectory + "RNNID_TrueHadTau_mc23_v1.root";
        } else if (m_sCampaign=="mc20"){
	    m_sInputFilePathJetIDHadTau = sDirectory + "RNNID_TrueHadTau_mc20_v0.root";   	
        }
      }
      if (m_sVarNameJetIDHadTau.empty()) m_sVarNameJetIDHadTau = "TauScaleFactorJetIDHadTau";

      std::string sJetIDWP = ConvertJetIDToString(m_iJetIDLevel);
      if (sJetIDWP.empty()) {
        ATH_MSG_WARNING("Could not find valid ID working point. Skip ID efficiency corrections.");
        continue;
      }

      asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>("TauAnalysisTools::CommonEfficiencyTool/JetIDHadTauTool", this);
      m_vCommonEfficiencyTools.push_back(tTool);
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathJetIDHadTau));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameJetIDHadTau));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
      ATH_CHECK(tTool->setProperty("WP", sJetIDWP));
    }
    else if (iEfficiencyCorrectionType == SFRecoHadTau)
    {
      if (m_sInputFilePathRecoHadTau.empty()) m_sInputFilePathRecoHadTau = sDirectory + "Reco_TrueHadTau_2019-summer_v2.root";
      if (m_sVarNameRecoHadTau.empty()) m_sVarNameRecoHadTau = "TauScaleFactorReconstructionHadTau";

      asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>("TauAnalysisTools::CommonEfficiencyTool/RecoHadTauTool", this);
      m_vCommonEfficiencyTools.push_back(tTool);
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathRecoHadTau));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameRecoHadTau));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
    }
    else if (iEfficiencyCorrectionType == SFEleIDHadTau)
    {
      // the path must be updated once RNN eVeto SFs are available
      if (m_sInputFilePathEleIDHadTau.empty()) m_sInputFilePathEleIDHadTau = sDirectory + "EleOLR_TrueHadTau_2016-ichep.root";
      if (m_sVarNameEleIDHadTau.empty()) m_sVarNameEleIDHadTau = "TauScaleFactorEleIDHadTau";

      asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>("TauAnalysisTools::CommonEfficiencyTool/EleIDHadTauTool", this);
      m_vCommonEfficiencyTools.push_back(tTool);
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathEleIDHadTau));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameEleIDHadTau));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
    }
    else if (iEfficiencyCorrectionType == SFEleIDElectron)
    {
      // the path must be updated once RNN eVeto SFs are available
      if (m_sInputFilePathEleIDElectron.empty()) {
        if(m_useFastSim) {
          ATH_MSG_WARNING("No fast-sim recommendation for tau electron veto, using full sim");
        }

        if(m_sCampaign=="mc23"){
            if( m_iJetIDLevel == (int)JETIDRNNLOOSE){
                m_sInputFilePathEleIDElectron = sDirectory + "EleRNN_SF_2022_looseRNNTauID_1p.root"; 
            } else if( m_iJetIDLevel == (int)JETIDRNNMEDIUM){
                m_sInputFilePathEleIDElectron = sDirectory + "EleRNN_SF_2022_mediumRNNTauID_1p.root";
            }
            else {
                ATH_MSG_ERROR("SFEleIDElectron correction not supported for JetIDLevel="<<m_iJetIDLevel);
                return StatusCode::FAILURE;
            }
        } else if(m_sCampaign=="mc20"){
            if( m_iJetIDLevel == (int)JETIDRNNLOOSE){
                m_sInputFilePathEleIDElectron = sDirectory + "EleRNN_SF_Run2_looseRNNTauID_1p.root";
            } else if( m_iJetIDLevel == (int)JETIDRNNMEDIUM){
                m_sInputFilePathEleIDElectron = sDirectory + "EleRNN_SF_Run2_mediumRNNTauID_1p_v1.root";
            }
            else {
                ATH_MSG_ERROR("SFEleIDElectron correction not supported for JetIDLevel="<<m_iJetIDLevel);
                return StatusCode::FAILURE;
            }
        }
      }
      if (m_sVarNameEleIDElectron.empty()) m_sVarNameEleIDElectron = "TauScaleFactorEleIDElectron";

      asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>("TauAnalysisTools::CommonEfficiencyTool/EleIDElectronTool", this);
      m_vCommonEfficiencyTools.push_back(tTool);
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathEleIDElectron));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameEleIDElectron));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
      ATH_CHECK(tTool->setProperty("WP", ConvertEleIDToString(m_iEleIDLevel)));
      ATH_CHECK(tTool->setProperty("UseTauSubstructure", false));

    } else if (iEfficiencyCorrectionType == SFTriggerHadTau){

      if (m_sTriggerName.empty()) {
        ATH_MSG_ERROR("Property \"Trigger\" was not set, please provide a trigger name.");
        return StatusCode::FAILURE;
      }
      if (m_sInputFilePathTriggerHadTau.empty()) {
        // Determine the input file name from the given trigger name.
	if(m_sCampaign=="mc23a"){
          if (m_sTriggerName.value().find("mediumRNN_tracktwoMVA") != std::string::npos) {
            m_sInputFilePathTriggerHadTau = sDirectory+"Trigger/RNN/Trigger_TrueHadTau_data2022"+GetTriggerSFMeasurementString()+m_sTriggerName+".root";
          }
          else {
            ATH_MSG_ERROR("Trigger " << m_sTriggerName << " is not supported for " << m_sCampaign << " campaign. Please fix \"TriggerName\" property. In case of doubt please consult with TauTrigger coordinators");
            return StatusCode::FAILURE;
          }
	} else if(m_sCampaign=="mc23d"){
          if (m_sTriggerName.value().find("mediumRNN_tracktwoMVA") != std::string::npos) {
            m_sInputFilePathTriggerHadTau = sDirectory+"Trigger/RNN/Trigger_TrueHadTau_data2023"+GetTriggerSFMeasurementString()+m_sTriggerName+".root";
          }
          else {
            ATH_MSG_ERROR("Trigger " << m_sTriggerName << " is not supported for " << m_sCampaign << " campaign. Please fix \"TriggerName\" property. In case of doubt please consult with TauTrigger coordinators");
            return StatusCode::FAILURE;
          }
        } else {
            ATH_MSG_ERROR("SFs are not available for " << m_sCampaign << " campaign.  For Run2, please fallback to the 2022-prerec tag. If the config is correct, then please contact the tau trigger coordinators");
	    return StatusCode::FAILURE;
	}
      }
      if (m_sVarNameTriggerHadTau.empty()) m_sVarNameTriggerHadTau = "TauScaleFactorTriggerHadTau";
	 
      asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>("TauAnalysisTools::TauEfficiencyTriggerTool/TriggerHadTauTool", this);
      m_vTriggerEfficiencyTools.push_back(tTool);
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathTriggerHadTau));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameTriggerHadTau));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
      ATH_CHECK(tTool->setProperty("WP", ConvertTriggerIDToString(m_iJetIDLevel)));
    }
    else {
      ATH_MSG_WARNING("unsupported EfficiencyCorrectionsType with enum " << iEfficiencyCorrectionType);
    }
  }

  return StatusCode::SUCCESS;
}

//_____________________________________________________________________________
// this whole block is a place holder until we get R22 Run2 recommendations
// none of these SFs are valid for R22 MC, except possibly RNN ID to very coarse approximation
StatusCode TauEfficiencyCorrectionsTool::initializeTools_2022_prerec()
{
  std::string sDirectory = "TauAnalysisTools/" + std::string(sSharedFilesVersion) + "/EfficiencyCorrections/";

  // initialise paths and SF names unless they have been configured by the user
  for (auto iEfficiencyCorrectionType : m_vEfficiencyCorrectionTypes)
  {
    if (iEfficiencyCorrectionType == SFJetIDHadTau)
    {
      if (m_sInputFilePathJetIDHadTau.empty()) {
        if(m_useFastSim) {
          ATH_MSG_WARNING("No fast-sim recommendation for Tau RNN, using full sim");
        }

	m_sInputFilePathJetIDHadTau = sDirectory + "RNNID_TrueHadTau_2022-prerecommendation_v2.root";
      }
      if (m_sVarNameJetIDHadTau.empty()) m_sVarNameJetIDHadTau = "TauScaleFactorJetIDHadTau";

      std::string sJetIDWP = ConvertJetIDToString(m_iJetIDLevel);
      if (sJetIDWP.empty()) {
        ATH_MSG_WARNING("Could not find valid ID working point. Skip ID efficiency corrections.");
        continue;
      }

      asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>("TauAnalysisTools::CommonEfficiencyTool/JetIDHadTauTool", this);
      m_vCommonEfficiencyTools.push_back(tTool);
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathJetIDHadTau));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameJetIDHadTau));
      //ATH_CHECK(tTool->setProperty("UseTauSubstructure", m_bUseTauSubstructure));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
      ATH_CHECK(tTool->setProperty("WP", sJetIDWP));
    }
    else if (iEfficiencyCorrectionType == SFEleIDHadTau)
    {
      // the path must be updated once RNN eVeto SFs are available
      if (m_sInputFilePathEleIDHadTau.empty()) m_sInputFilePathEleIDHadTau = sDirectory + "EleOLR_TrueHadTau_2016-ichep.root";
      if (m_sVarNameEleIDHadTau.empty()) m_sVarNameEleIDHadTau = "TauScaleFactorEleIDHadTau";

      asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>("TauAnalysisTools::CommonEfficiencyTool/EleIDHadTauTool", this);
      m_vCommonEfficiencyTools.push_back(tTool);
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathEleIDHadTau));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameEleIDHadTau));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
    }
    else if (iEfficiencyCorrectionType == SFEleIDElectron)
    {
      // the path must be updated once RNN eVeto SFs are available
      if (m_sInputFilePathEleIDElectron.empty()) {
 	if(m_useFastSim) {
	  ATH_MSG_WARNING("No fast-sim recommendation for tau electron veto, using full sim");
	}	

	m_sInputFilePathEleIDElectron = sDirectory+ "EleRNN_TrueElectron_2022-mc20-prerec-v2.root";
      }
      if (m_sVarNameEleIDElectron.empty()) m_sVarNameEleIDElectron = "TauScaleFactorEleIDElectron";

      asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>("TauAnalysisTools::CommonEfficiencyTool/EleIDElectronTool", this);
      m_vCommonEfficiencyTools.push_back(tTool);
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathEleIDElectron));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameEleIDElectron));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
      ATH_CHECK(tTool->setProperty("WP", ConvertEleIDToString(m_iEleIDLevel)));
      ATH_CHECK(tTool->setProperty("UseTauSubstructure", false));
    }
    else if (iEfficiencyCorrectionType == SFRecoHadTau)
    {
      if (m_sInputFilePathRecoHadTau.empty()) m_sInputFilePathRecoHadTau = sDirectory + "Reco_TrueHadTau_2019-summer_v2.root";
      if (m_sVarNameRecoHadTau.empty()) m_sVarNameRecoHadTau = "TauScaleFactorReconstructionHadTau";

      asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>("TauAnalysisTools::CommonEfficiencyTool/RecoHadTauTool", this);
      m_vCommonEfficiencyTools.push_back(tTool);
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathRecoHadTau));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameRecoHadTau));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
    }
    else if (iEfficiencyCorrectionType == SFDecayModeHadTau)
    {
      if (m_sInputFilePathDecayModeHadTau.empty()) m_sInputFilePathDecayModeHadTau = sDirectory + "DecayModeSubstructure_TrueHadTau_2019-summer.root";
      if (m_sVarNameDecayModeHadTau.empty()) m_sVarNameDecayModeHadTau = "TauScaleFactorDecayModeHadTau";

      std::string sJetIDWP = ConvertJetIDToString(m_iJetIDLevel);
      if (sJetIDWP.empty()) {
        ATH_MSG_WARNING("Could not find valid ID working point. Skip ID efficiency corrections.");
        continue;
      }

      asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>("DecayModeHadTauTool", this);
      m_vCommonEfficiencyTools.push_back(tTool);
      ATH_CHECK(ASG_MAKE_ANA_TOOL(*tTool, TauAnalysisTools::CommonEfficiencyTool));
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathDecayModeHadTau));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameDecayModeHadTau));
      ATH_CHECK(tTool->setProperty("UseTauSubstructure", true));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
      ATH_CHECK(tTool->setProperty("WP", sJetIDWP));
    }
    else if (iEfficiencyCorrectionType == SFTriggerHadTau)
    {
      if (m_sTriggerName.empty()) {
	ATH_MSG_ERROR("Property \"Trigger\" was not set, please provide a trigger name.");
	return StatusCode::FAILURE;  
      }
      if (m_sInputFilePathTriggerHadTau.empty()) {
	// Determine the input file name from the given trigger name.
	// Triggers having "mediumRNN_tracktwoMVA are only part of 2018aftTS1.
	// Every other trigger having "tracktwoEF" is only part of 2018.
	// Every other trigger having "tau160_medium1" is only part of 2016.
	// Every other trigger having "tau160" is only part of 2017/2018.
	// Lastly check for other possible triggers, if this is not fulfilled the passed trigger is not supported.
	if (m_sTriggerName.value().find("mediumRNN_tracktwoMVA") != std::string::npos) {
	  m_sInputFilePathTriggerHadTau = sDirectory+"Trigger/RNN/Trigger_TrueHadTau_2019-summer_data2018aftTS1"+GetTriggerSFMeasurementString()+m_sTriggerName+".root";
	}
	else if (m_sTriggerName.value().find("tracktwoEF") != std::string::npos) {
	  m_sInputFilePathTriggerHadTau = sDirectory+"Trigger/RNN/Trigger_TrueHadTau_2019-summer_data2018"+GetTriggerSFMeasurementString()+m_sTriggerName+".root";
	}
	else if (m_sTriggerName.value().find("tau160_medium1") != std::string::npos) {
	  m_sInputFilePathTriggerHadTau = sDirectory+"Trigger/RNN/Trigger_TrueHadTau_2019-summer_data2016"+GetTriggerSFMeasurementString()+m_sTriggerName+".root";
	}
	else if ((m_sTriggerName.value().find("tau160") != std::string::npos) || (m_sTriggerName.value().find("tau60") != std::string::npos)) {
	  m_sInputFilePathTriggerHadTau = sDirectory+"Trigger/RNN/Trigger_TrueHadTau_2019-summer_data1718"+GetTriggerSFMeasurementString()+m_sTriggerName+".root";
	}
	else if ((m_sTriggerName.value().find("tau125") != std::string::npos) || (m_sTriggerName.value().find("tau25") != std::string::npos) || (m_sTriggerName.value().find("tau35") != std::string::npos) || (m_sTriggerName.value().find("tau50") != std::string::npos) || (m_sTriggerName.value().find("tau80") != std::string::npos) ) {
	  m_sInputFilePathTriggerHadTau = sDirectory+"Trigger/RNN/Trigger_TrueHadTau_2019-summer_data161718"+GetTriggerSFMeasurementString()+m_sTriggerName+".root";
	}
	else {
	  ATH_MSG_ERROR("Trigger " << m_sTriggerName << " is not supported. Please fix \"TriggerName\" property.");        
	  return StatusCode::FAILURE;
	}
      }
      if (m_sVarNameTriggerHadTau.empty()) m_sVarNameTriggerHadTau = "TauScaleFactorTriggerHadTau";

      asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* tTool = new asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>("TauAnalysisTools::TauEfficiencyTriggerTool/TriggerHadTauTool", this);
      m_vTriggerEfficiencyTools.push_back(tTool);
      ATH_CHECK(tTool->setProperty("InputFilePath", m_sInputFilePathTriggerHadTau));
      ATH_CHECK(tTool->setProperty("VarName", m_sVarNameTriggerHadTau));
      ATH_CHECK(tTool->setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
      ATH_CHECK(tTool->setProperty("WP", ConvertTriggerIDToString(m_iJetIDLevel)));
    }
    else {
      ATH_MSG_WARNING("unsupported EfficiencyCorrectionsType with enum " << iEfficiencyCorrectionType);
    }
  }

  return StatusCode::SUCCESS;
}

// auto detection of simulation flavour, used to cross check configuration of tool
//______________________________________________________________________________
StatusCode TauEfficiencyCorrectionsTool::beginInputFile()
{
  if (inputMetaStore()->contains<xAOD::FileMetaData>("FileMetaData")) {
    const xAOD::FileMetaData* fmd = nullptr;
    ATH_CHECK( inputMetaStore()->retrieve( fmd, "FileMetaData" ) );
    std::string simType("");
    bool result = fmd->value( xAOD::FileMetaData::simFlavour , simType );
    // if no result -> no simFlavor metadata, so must be data
    if(result)  std::transform(simType.begin(), simType.end(), simType.begin(), ::toupper);
    
    if( simType.find("ATLFAST3") != std::string::npos && !m_useFastSim){
      ATH_MSG_WARNING("Input file is AF3 sample but you are _not_ using AF3 corrections and uncertainties, you should set \"useFastSim\" to \"true\"");
    } else if (simType.find("FULLG4")!=std::string::npos && m_useFastSim){
      ATH_MSG_WARNING("Input file is full simulation but you are using AF3 corrections and uncertainties, you should set \"useFastSim\" to \"false\"");
    }
  }

  return StatusCode::SUCCESS;
}

//______________________________________________________________________________
std::string TauEfficiencyCorrectionsTool::ConvertJetIDToString(const int iLevel) const
{
  switch(iLevel)
    {
    case JETIDNONE:
      return "none";
    case JETIDRNNVERYLOOSE: 
      ATH_MSG_WARNING("Efficiency corrections for JETIDRNNVERYLOOSE working point are not supported.");
      return "";
    case JETIDRNNLOOSE: 
      return "jetrnnsigloose";
    case JETIDRNNMEDIUM: 
      return "jetrnnsigmedium";
    case JETIDRNNTIGHT: 
      return "jetrnnsigtight";
    default:
      ATH_MSG_WARNING("No valid JetID level passed.");
      return "";
    }
}

std::string TauEfficiencyCorrectionsTool::ConvertEleIDToString(const int iLevel) const
{
  switch(iLevel)
    {
    case ELEIDRNNTIGHT:
      return "eleRNNTight";
    case ELEIDRNNMEDIUM:
      return "eleRNNMedium";
    case ELEIDRNNLOOSE:
      return "eleRNNLoose";
    default:
      ATH_MSG_WARNING("No valid EleID level passed.");
      return "";
    }
}

//______________________________________________________________________________
std::string TauEfficiencyCorrectionsTool::ConvertTriggerIDToString(const int iLevel) const
{
  switch(iLevel)
    {
    case JETIDRNNLOOSE:
      return "loose";
    case JETIDRNNMEDIUM:
      return "medium";
    case JETIDRNNTIGHT:
      return "tight";
    default:
      ATH_MSG_WARNING("No valid TriggerID level passed.");
      return "";
    }
}

//______________________________________________________________________________
std::string TauEfficiencyCorrectionsTool::GetTriggerSFMeasurementString() const
{
  std::string sMeasurement = "_comb_";

  if (m_sTriggerSFMeasurement == "Ztautau")
    sMeasurement = "_Ztt_";
  else if (m_sTriggerSFMeasurement == "ttbar")
    sMeasurement = "_ttbar_";
  else if (m_sTriggerSFMeasurement != "combined")
    ATH_MSG_WARNING("Trigger scale factor measurement \'" << m_sTriggerSFMeasurement << "\' is not supported. \'combined\' is used instead.");

  return sMeasurement;
}

StatusCode TauEfficiencyCorrectionsTool::readRandomRunNumber()
{
  // read the random run rumber from the EventInfo 	
  if (m_bReadRandomRunNumber && m_iRunNumber == 0)
  {
    static const SG::ConstAccessor<unsigned int> acc_rnd("RandomRunNumber");
    const xAOD::EventInfo* eventInfo = nullptr;
    if (!evtStore()->contains<xAOD::EventInfo>("EventInfo") || !evtStore()->retrieve(eventInfo, "EventInfo").isSuccess())
    {
      ANA_MSG_ERROR("Could not retrieve EventInfo");
      return StatusCode::FAILURE;
    }

    if (!acc_rnd.isAvailable(*eventInfo))
    {
      ANA_MSG_ERROR("Failed to find the RandomRunNumber decoration. Call the apply() method from the PileupReweightingTool beforehand to get period dependent SFs.");
      return StatusCode::FAILURE;
    }

    m_iRunNumber = acc_rnd(*eventInfo);

    ANA_MSG_VERBOSE("Read RandomRunNumber as " << m_iRunNumber);
  }

  return StatusCode::SUCCESS;
}

} // namespace TauAnalysisTools
