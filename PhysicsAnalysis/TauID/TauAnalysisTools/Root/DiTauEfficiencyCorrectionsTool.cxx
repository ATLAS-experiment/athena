/**
 *
 * @copyright Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 *
 * @file DiTauEfficiencyCorrectionsTool.cxx
 * @brief Class for ditau efficiency correction scale factors and uncertainties
 * @date 2021-02-18
 *
 */

// EDM include(s):
#include "PATInterfaces/SystematicRegistry.h"
#include "xAODEventInfo/EventInfo.h"

// Local include(s):
#include "TauAnalysisTools/DiTauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/SharedFilesVersion.h"

#include <AsgTools/AsgToolConfig.h>

namespace TauAnalysisTools
{

//______________________________________________________________________________
DiTauEfficiencyCorrectionsTool::DiTauEfficiencyCorrectionsTool( const std::string& sName )
  : asg::AsgMetadataTool( sName )
  , m_bIsData(false)
  , m_bIsConfigured(false)
{
}

//______________________________________________________________________________
DiTauEfficiencyCorrectionsTool::~DiTauEfficiencyCorrectionsTool()
{
}


//______________________________________________________________________________
StatusCode DiTauEfficiencyCorrectionsTool::initialize()
{
  // Greet the user:
  ATH_MSG_INFO( "Initializing DiTauEfficiencyCorrectionsTool" );

  if (m_bSkipTruthMatchCheck)
    ATH_MSG_WARNING("Truth match check will be skipped. This is ONLY FOR TESTING PURPOSE!");

  if (m_sRecommendationTag == "2017-moriond")
    ATH_CHECK(initializeTools_2017_moriond());
  else
  {
    ATH_MSG_FATAL("Unknown RecommendationTag: "<<m_sRecommendationTag);
    return StatusCode::FAILURE;
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


//______________________________________________________________________________
StatusCode DiTauEfficiencyCorrectionsTool::beginEvent()
{
  if (!m_bIsConfigured)
  {
    const xAOD::EventInfo* xEventInfo = nullptr;
    ATH_CHECK(evtStore()->retrieve(xEventInfo,"EventInfo"));
    m_bIsData = !(xEventInfo->eventType( xAOD::EventInfo::IS_SIMULATION));
    m_bIsConfigured = true;
  }

  return StatusCode::SUCCESS;
}


//______________________________________________________________________________
void DiTauEfficiencyCorrectionsTool::printConfig() const
{
  ATH_MSG_DEBUG( "DiTauEfficiencyCorrectionsTool with name " << name() << " is configured as follows:" );
  for (auto iEfficiencyCorrectionType : m_vEfficiencyCorrectionTypes) {
    ATH_MSG_DEBUG( "  EfficiencyCorrectionTypes " << iEfficiencyCorrectionType );
  }
  ATH_MSG_DEBUG( "  InputFilePathJetIDHadTau " << m_sInputFilePathJetIDHadTau );
  ATH_MSG_DEBUG( "  VarNameJetIDHadTau " << m_sVarNameJetIDHadTau );
  ATH_MSG_DEBUG( "  RecommendationTag " << m_sRecommendationTag );
}

//______________________________________________________________________________
CP::CorrectionCode DiTauEfficiencyCorrectionsTool::getEfficiencyScaleFactor( const xAOD::DiTauJet& xDiTau,
    double& eff )
{
  eff = 1.;

  if (m_bIsData)
    return CP::CorrectionCode::Ok;

  double dToolEff = 1.;
  CP::CorrectionCode tmpCorrectionCode = m_tTool->getEfficiencyScaleFactor(xDiTau, dToolEff);
  if (tmpCorrectionCode != CP::CorrectionCode::Ok)
    return tmpCorrectionCode;

  eff *= dToolEff;
  return CP::CorrectionCode::Ok;
}

//______________________________________________________________________________
CP::CorrectionCode DiTauEfficiencyCorrectionsTool::applyEfficiencyScaleFactor( const xAOD::DiTauJet& xDiTau )
{
  if (m_bIsData)
    return CP::CorrectionCode::Ok;

  CP::CorrectionCode tmpCorrectionCode = m_tTool->applyEfficiencyScaleFactor(xDiTau);
  if (tmpCorrectionCode != CP::CorrectionCode::Ok)
  {
    return tmpCorrectionCode;
  }

  return CP::CorrectionCode::Ok;
}

/// returns: whether this tool is affected by the given systematis
//______________________________________________________________________________
bool DiTauEfficiencyCorrectionsTool::isAffectedBySystematic( const CP::SystematicVariation& systematic ) const
{
  if (m_tTool->isAffectedBySystematic(systematic))
    return true;

  return false;
}

/// returns: the list of all systematics this tool can be affected by
//______________________________________________________________________________
CP::SystematicSet DiTauEfficiencyCorrectionsTool::affectingSystematics() const
{
  CP::SystematicSet sAffectingSystematics;
  sAffectingSystematics.insert(m_tTool->affectingSystematics());

  return sAffectingSystematics;
}

/// returns: the list of all systematics this tool recommends to use
//______________________________________________________________________________
CP::SystematicSet DiTauEfficiencyCorrectionsTool::recommendedSystematics() const
{
  CP::SystematicSet sRecommendedSystematics;
  sRecommendedSystematics.insert(m_tTool->recommendedSystematics());

  return sRecommendedSystematics;
}

//______________________________________________________________________________
StatusCode DiTauEfficiencyCorrectionsTool::applySystematicVariation ( const CP::SystematicSet& sSystematicSet)
{
  if (m_tTool->applySystematicVariation(sSystematicSet) == StatusCode::FAILURE)
  {
    ATH_MSG_ERROR( "failing in appying systematic uncertainty."); 
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

//=================================PRIVATE-PART=================================

//______________________________________________________________________________
StatusCode DiTauEfficiencyCorrectionsTool::initializeTools_2017_moriond()
{
  std::string sDirectory = "TauAnalysisTools/" + std::string(sSharedFilesVersion) + "/EfficiencyCorrections/";
  for (auto iEfficiencyCorrectionType : m_vEfficiencyCorrectionTypes)
  {
    if (iEfficiencyCorrectionType == SFJetIDHadTau)
    {
      // only set vars if they have been configured by the user
      if (m_sInputFilePathJetIDHadTau.empty()) {
         sDirectory = "TauAnalysisTools/00-04-00/EfficiencyCorrections/"; 	      
         m_sInputFilePathJetIDHadTau = sDirectory+"JetID_TrueHadDiTau_2017-fall.root";
      }
      if (m_sVarNameJetIDHadTau.empty()) m_sVarNameJetIDHadTau = "DiTauScaleFactorJetIDHadTau";

      std::string wp = ConvertJetIDToString(m_iJetIDLevel);

      if (m_tTool.empty()){
        asg::AsgToolConfig config("TauAnalysisTools::CommonDiTauEfficiencyTool/JetIDHadTauTool_"+wp);
	ATH_CHECK(config.setProperty("InputFilePath", m_sInputFilePathJetIDHadTau));
        ATH_CHECK(config.setProperty("VarName", m_sVarNameJetIDHadTau));
        ATH_CHECK(config.setProperty("SkipTruthMatchCheck", m_bSkipTruthMatchCheck));
        ATH_CHECK(config.setProperty("WP", wp));
        ATH_CHECK(config.makePrivateTool(m_tTool));
      }
      ATH_CHECK(m_tTool.retrieve());
    }
    else
    {
      ATH_MSG_WARNING("unsupported EfficiencyCorrectionsType with enum " << iEfficiencyCorrectionType);
    }
  }
  return StatusCode::SUCCESS;
}

//______________________________________________________________________________
std::string DiTauEfficiencyCorrectionsTool::ConvertJetIDToString(const int iLevel) const
{
  switch(iLevel)
  {
  case JETIDNONE:
    return "ditaureconstruction";
    break;
  case JETIDBDTVERYLOOSE:
    return "jetbdtsigveryloose";
    break;
  case JETIDBDTLOOSE:
    return "jetbdtsigloose";
    break;
  case JETIDBDTMEDIUM:
    return "jetbdtsigmedium";
    break;
  case JETIDBDTTIGHT:
    return "jetbdtsigtight";
    break;
  default:
    assert(false && "No valid ID level passed. Breaking up ...");
    break;
  }
  return "";
}


} // namespace TauAnalysisTools
