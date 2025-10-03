/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "TauAnalysisTools/DiTauSelectionTool.h"
#include "TauAnalysisTools/SharedFilesVersion.h"
#include "TauAnalysisTools/DiTauSelectionCuts.h"

// Framework include(s):
#include "PathResolver/PathResolver.h"

// ROOT include(s)
#include "TEnv.h"
#include "THashList.h"

// System include(s)
#include <atomic>
#include <cmath>  // for std::isnan

using namespace TauAnalysisTools;

//=================================PUBLIC-PART==================================
//______________________________________________________________________________
DiTauSelectionTool::DiTauSelectionTool( const std::string& name )
  : asg::AsgMetadataTool( name )
  , m_fOutFile(nullptr)
  , m_aAccept( "DiTauSelection" )
{}

//______________________________________________________________________________
DiTauSelectionTool::~DiTauSelectionTool()
{
  m_cMap.clear();
}

//______________________________________________________________________________
StatusCode DiTauSelectionTool::initialize()
{
 
  m_vPtRegion = m_vecPtRegion.value();	
  m_vAbsEtaRegion = m_vecAbsEtaRegion.value();
  m_vNSubjetsRegion = m_vecNSubjetsRegion.value(); 
  m_vAbsCharges = m_vecAbsCharges.value();
  m_vOmniScoreRegion = m_vecOmniScoreRegion.value();

  bool bConfigViaConfigFile = !m_sConfigPath.empty();
  bool bConfigViaProperties = false;
  if (!bConfigViaProperties and !m_vPtRegion.empty())             bConfigViaProperties = true;
  if (!bConfigViaProperties and !std::isnan(m_dPtMin.value()))             bConfigViaProperties = true;
  if (!bConfigViaProperties and !std::isnan(m_dPtMax.value()))             bConfigViaProperties = true;
  if (!bConfigViaProperties and !m_vAbsEtaRegion.empty())         bConfigViaProperties = true;
  if (!bConfigViaProperties and !std::isnan(m_dAbsEtaMin.value()))     bConfigViaProperties = true;
  if (!bConfigViaProperties and !std::isnan(m_dAbsEtaMax.value()))     bConfigViaProperties = true;
  if (!bConfigViaProperties and !m_vNSubjetsRegion.empty())       bConfigViaProperties = true;
  if (!bConfigViaProperties and !std::isnan(m_dNSubjetsMin.value())) bConfigViaProperties = true;
  if (!bConfigViaProperties and !std::isnan(m_dNSubjetsMax.value())) bConfigViaProperties = true;
  if (!bConfigViaProperties and !m_vAbsCharges.empty())       bConfigViaProperties = true;
  if (!bConfigViaProperties and !std::isnan(m_iAbsCharge.value())) bConfigViaProperties = true;
  if (!bConfigViaProperties and !m_vOmniScoreRegion.empty())         bConfigViaProperties = true;
  if (!bConfigViaProperties and !std::isnan(m_dOmniScoreMin.value())) bConfigViaProperties = true;
  if (!bConfigViaProperties and !std::isnan(m_dOmniScoreMax.value())) bConfigViaProperties = true;


  if (bConfigViaConfigFile and bConfigViaProperties)
  {
    ATH_MSG_WARNING("Configured tool via setProperty and configuration file, which may lead to unexpected configuration.");
    ATH_MSG_WARNING("In doubt check the configuration that is printed when the tool is initialized and the message level is set to debug");
    ATH_MSG_WARNING("For further details please refer to the documentation:");
    ATH_MSG_WARNING("https://gitlab.cern.ch/atlas/athena/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/doc/README-DiTauSelectionTool.rst");
  }
  if (!bConfigViaConfigFile and !bConfigViaProperties)
  {
    ATH_MSG_WARNING("No cut configuration provided, the tool will not do anything. For further details please refer to the documentation:");
    ATH_MSG_WARNING("https://gitlab.cern.ch/atlas/athena/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/doc/README-DiTauSelectionTool.rst");
  }

  if (bConfigViaConfigFile)
  {
    TEnv rEnv;
    std::string sInputFilePath = PathResolverFindCalibFile(m_sConfigPath);

    if (!testFileForEOFContainsCharacters(sInputFilePath))
      ATH_MSG_WARNING("Config file for DiTauSelectionTool with path "<<sInputFilePath<<" does not contain an empty last line. The tool might not be properly configured!");

    rEnv.ReadFile(sInputFilePath.c_str(),
                  kEnvAll);

    std::vector<std::string> vCuts;
    // if Cuts are specified in the config file take these ones, if not take all
    // specified in the config
    if (rEnv.Defined("SelectionCuts"))
      TauAnalysisTools::split(rEnv, "SelectionCuts", ' ', vCuts);
    else
    {
      auto lList = rEnv.GetTable();
      for( Int_t i = 0; i < lList->GetEntries(); ++i )
      {
        vCuts.push_back( lList->At( i )->GetName() );
      }
    }

    int iSelectionCuts = 0;

    for (const std::string& sCut : vCuts)
    {
      if (sCut == "PtRegion")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutPt;
        if (m_vPtRegion.empty())
          TauAnalysisTools::split(rEnv,"PtRegion", ';', m_vPtRegion);
      }
      else if (sCut == "PtMin")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutPt;
        if (std::isnan(m_dPtMin.value()))
          m_dPtMin = rEnv.GetValue("PtMin",NAN);
      }
      else if (sCut == "PtMax")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutPt;
        if (std::isnan(m_dPtMax.value()))
          m_dPtMax = rEnv.GetValue("PtMax",NAN);
      }
      else if (sCut == "AbsEtaRegion")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutAbsEta;
        if (m_vAbsEtaRegion.empty())
          TauAnalysisTools::split(rEnv,"AbsEtaRegion", ';', m_vAbsEtaRegion);
      }
      else if (sCut == "AbsEtaMin")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutAbsEta;
        if (std::isnan(m_dAbsEtaMin.value()))
          m_dAbsEtaMin = rEnv.GetValue("AbsEtaMin",NAN);
      }
      else if (sCut == "AbsEtaMax")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutAbsEta;
        if (std::isnan(m_dAbsEtaMax.value()))
          m_dAbsEtaMax = rEnv.GetValue("AbsEtaMax",NAN);
      }
      else if (sCut == "NSubjetsRegion")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutNSubjets;
        if (m_vNSubjetsRegion.empty())
          TauAnalysisTools::split(rEnv,"NSubjetsRegion", ';', m_vNSubjetsRegion);
      }
      else if (sCut == "NSubjetsMin")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutNSubjets;
        if (std::isnan(m_dNSubjetsMin.value()))
          m_dNSubjetsMin = rEnv.GetValue("NSubjetsMin",NAN);
      }
      else if (sCut == "NSubjetsMax")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutNSubjets;
        if (std::isnan(m_dNSubjetsMax.value()))
          m_dNSubjetsMax = rEnv.GetValue("NSubjetsMax",NAN);
      }
      else if (sCut == "AbsCharges")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutAbsCharge;
        if (m_vAbsCharges.empty())
          TauAnalysisTools::split(rEnv,"AbsCharges", ';', m_vAbsCharges);
      }
      else if (sCut == "AbsCharge")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutAbsCharge;
        if (std::isnan(m_iAbsCharge.value()))
          m_iAbsCharge = rEnv.GetValue("AbsCharge",NAN);
      }
      else if (sCut == "OmniScoreRegion")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutOmniScore;
        if (m_vOmniScoreRegion.empty())
          TauAnalysisTools::split(rEnv,"OmniScoreRegion", ';', m_vOmniScoreRegion);

	// check if using OmniScore
        m_useOmniScore = true;
      }
      else if (sCut == "OmniScoreMin")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutOmniScore;
        if (std::isnan(m_dOmniScoreMin.value()))
          m_dOmniScoreMin = rEnv.GetValue("OmniScoreMin",NAN);

        // check for possible mis-config in DiTau selection
        for (const std::string& checkCut : vCuts){
           if (checkCut.find("OmniScoreRegion") != std::string::npos) {
              ATH_MSG_ERROR("Misconfig due to OmniScoreRegion and OmniScoreMin cuts both present in the config file. Please CHECK carefully config file again and choose of the two");
              return StatusCode::FAILURE;
           }
        }

	// check if using OmniScore
	m_useOmniScore = true;
      }
      else if (sCut == "OmniScoreMax")
      {
        iSelectionCuts = iSelectionCuts | DiTauCutOmniScore;
        if (std::isnan(m_dOmniScoreMax.value()))
          m_dOmniScoreMax = rEnv.GetValue("OmniScoreMax",NAN);

        // check for possible mis-config in DiTau selection
        for (const std::string& checkCut : vCuts){
           if (checkCut.find("OmniScoreRegion") != std::string::npos) {
              ATH_MSG_ERROR("Misconfig due to OmniScoreRegion and OmniScoreMax cuts both present in the config file. Please CHECK carefully config file again and choose of the two");
              return StatusCode::FAILURE;
           }
        }

	// check if using OmniScore
	m_useOmniScore = true;
      }
      else ATH_MSG_WARNING("Cut " << sCut << " is not available");
    }

    if (m_iSelectionCuts == NoDiTauCut)
      m_iSelectionCuts = iSelectionCuts;
  }

  // initialise the ReadDecorHandleKey if OmniScore is applied
  if (m_useOmniScore) {
    if(m_OmniScoreDecorKey.empty()) { 	  
        ATH_CHECK( m_OmniScoreDecorKey.assign("DiTauJets.omni_score"));
    }     
  }
  ATH_CHECK( m_OmniScoreDecorKey.initialize( m_useOmniScore ) );

  // specify all available cut descriptions
  using map_type  = std::map<DiTauSelectionCuts, std::unique_ptr<TauAnalysisTools::DiTauSelectionCut>>;
  using pair_type = map_type::value_type;

  pair_type elements[] =
  {
   {DiTauCutPt, std::make_unique<TauAnalysisTools::DiTauSelectionCutPt>(this)},
   {DiTauCutAbsEta, std::make_unique<TauAnalysisTools::DiTauSelectionCutAbsEta>(this)},
   {DiTauCutNSubjets, std::make_unique<TauAnalysisTools::DiTauSelectionCutNSubjets>(this)},
   {DiTauCutAbsCharge, std::make_unique<TauAnalysisTools::DiTauSelectionCutAbsCharge>(this)},
   {DiTauCutOmniScore, std::make_unique<TauAnalysisTools::DiTauSelectionCutOmniScore>(this)}, 
  };
  
  m_cMap = { std::make_move_iterator( begin(elements) ), std::make_move_iterator( end(elements) ) };
  
  ATH_MSG_INFO( "Initializing DiTauSelectionTool" );
  FillRegionVector(m_vPtRegion, m_dPtMin.value(), m_dPtMax.value());
  FillRegionVector(m_vAbsEtaRegion, m_dAbsEtaMin.value(), m_dAbsEtaMax.value());
  FillRegionVector(m_vNSubjetsRegion, m_dNSubjetsMin.value(), m_dNSubjetsMax.value());
  FillValueVector(m_vAbsCharges, m_iAbsCharge.value());
  FillRegionVector(m_vOmniScoreRegion, m_dOmniScoreMin.value(), m_dOmniScoreMax.value() );

  PrintConfigRegion ("Pt",          m_vPtRegion);
  PrintConfigRegion ("AbsEta",      m_vAbsEtaRegion);
  PrintConfigRegion ("NSubjets",    m_vNSubjetsRegion);
  PrintConfigValue  ("AbsCharge",   m_vAbsCharges);
  PrintConfigRegion ("OmniScore",   m_vOmniScoreRegion);

  std::string sCuts = "";
  if (m_iSelectionCuts & DiTauCutPt) sCuts += "Pt ";
  if (m_iSelectionCuts & DiTauCutAbsEta) sCuts += "AbsEta ";
  if (m_iSelectionCuts & DiTauCutNSubjets) sCuts += "NSubjets ";
  if (m_iSelectionCuts & DiTauCutAbsCharge) sCuts += "AbsCharge ";
  if (m_iSelectionCuts & DiTauCutOmniScore) sCuts += "OmniScore ";

  ATH_MSG_DEBUG( "cuts: " << sCuts);

  if (m_bCreateControlPlots)
    setupCutFlowHistogram();

  for ( const auto& entry : m_cMap ) {
     if ( m_iSelectionCuts &entry.first ) {
	entry.second->setAcceptInfo(m_aAccept);
     }
  }
  
  return StatusCode::SUCCESS;
}


//______________________________________________________________________________
StatusCode DiTauSelectionTool::beginEvent()
{
  return StatusCode::SUCCESS;
}

//______________________________________________________________________________
const asg::AcceptInfo& DiTauSelectionTool::getAcceptInfo() const
{
  return m_aAccept;
}

//______________________________________________________________________________
asg::AcceptData DiTauSelectionTool::accept( const xAOD::IParticle* xP ) const
{
  // Check if this is a jet:
  if( xP->type() != xAOD::Type::Jet )
  {
    ATH_MSG_ERROR( "accept(...) Function received a non-jet" );
    return asg::AcceptData (&m_aAccept);
  }

  // Cast it to a ditau:
  const xAOD::DiTauJet* xDiTau = dynamic_cast< const xAOD::DiTauJet* >( xP );
  if( ! xDiTau )
  {
    ATH_MSG_FATAL( "accept(...) Failed to cast particle to tau" );
    return asg::AcceptData (&m_aAccept);
  }

  // Let the specific function do the work:
  return accept( *xDiTau );
}

//______________________________________________________________________________
asg::AcceptData DiTauSelectionTool::accept( const xAOD::DiTauJet& xDiTau ) const
{
  asg::AcceptData acceptData (&m_aAccept);

  int iNBin = 0;

  if (m_bCreateControlPlots)
  {
    // fill cutflow 'All' bin
    m_hCutFlow->Fill(iNBin);
    // fill main distributions before all cuts
    for (const auto& entry : m_cMap)
      entry.second->fillHistogramCutPre(xDiTau);
  }
  try
  {
    for (const auto& entry : m_cMap)
    {
      if (m_iSelectionCuts & entry.first)
      {
        if (!entry.second->accept(xDiTau, acceptData))
          return acceptData;
        else
        {
          if (m_bCreateControlPlots)
          {
            // fill cutflow after each passed cut
            iNBin++;
            m_hCutFlow->Fill(iNBin);
          }
        }
      }
    }
  }
  catch (const std::runtime_error& error)
  {
    // LEGACY: In practical terms this should probably just throw, not
    // print a warning/error and then continue on.  However, I leave
    // that to the experts who probably had a reason not to let the
    // exception escape.  For now I just downgraded it from error to
    // warning and limited the number of warnings (04 Jan 22).
    static std::atomic<uint64_t> warning_count (0u);
    auto mycount = ++ warning_count;
    if (mycount < 10u)
    {
      ATH_MSG_WARNING(error.what());
      if (mycount == 9u)
        ATH_MSG_WARNING ("this is your last warning");
    }
  }

  // fill main distributions after all cuts
  if (m_bCreateControlPlots)
  {
    for (const auto& entry : m_cMap)
      entry.second->fillHistogramCut(xDiTau);
  }

  // // Return the result:
  return acceptData;
}

//______________________________________________________________________________
void DiTauSelectionTool::setOutFile( TFile* fOutFile )
{
  m_fOutFile = fOutFile;
}

//______________________________________________________________________________
void DiTauSelectionTool::writeControlHistograms()
{
  if (m_bCreateControlPlots and !m_fOutFile)
    ATH_MSG_WARNING("CreateControlPlots was set to true, but no valid file pointer was provided");
  if (m_bCreateControlPlots and m_fOutFile)
  {
    /// create output directory
    m_fOutFile->mkdir((this->name()+"_control").c_str());
    m_fOutFile->cd((this->name()+"_control").c_str());
    /// write cutflow histogram
    m_hCutFlow->Write();

    for (const auto& entry : m_cMap)
      entry.second->writeControlHistograms();
  }
}


//=================================PRIVATE-PART=================================
void DiTauSelectionTool::setupCutFlowHistogram()
{
  // count number of cuts
  int iNBins = 0;
  for (const auto& entry : m_cMap)
    if (m_iSelectionCuts & entry.first)
      iNBins++;
  // create cutflow histogram with iNBins+1 bins, where first bin is 'All' bin
  m_hCutFlow = std::make_shared<TH1F>("hCutFlow","CutFlow;; events",iNBins+1,0,iNBins+1);
  m_hCutFlow->GetXaxis()->SetBinLabel(1,"All");

  // reusing this variable to reduce overhead
  iNBins = 2;
  // set bin labels
  for (const auto& entry : m_cMap)
    if (m_iSelectionCuts & entry.first)
    {
      m_hCutFlow->GetXaxis()->SetBinLabel(iNBins, entry.second->getName().c_str());
      iNBins++;
    }
}

//______________________________________________________________________________
template<typename T, typename U>
void DiTauSelectionTool::FillRegionVector(std::vector<T>& vRegion, U tMin, U tMax) const
{
  if (!vRegion.empty())
    return;
  if (!std::isnan(tMin)) 		// if tMin is NAN, then this assumption fails and -inf is added to the vector
    vRegion.push_back(tMin);
  else
    vRegion.push_back(-std::numeric_limits<T>::infinity());

  if (!std::isnan(tMax))		// if tMax is NAN, then this assumption fails and inf is added to the vector
    vRegion.push_back(tMax);
  else
    vRegion.push_back(std::numeric_limits<T>::infinity());
}

//______________________________________________________________________________
template<typename T, typename U>
void DiTauSelectionTool::FillValueVector(std::vector<T>& vRegion, U tVal) const
{
  if (!vRegion.empty())
    return;
  if (tVal == tVal)		// if tMax is NAN, then this assumption fails and nothing is added to the vector
    vRegion.push_back(tVal);
}

//______________________________________________________________________________
template<typename T>
void DiTauSelectionTool::PrintConfigRegion(const std::string& sCutName, std::vector<T>& vRegion) const
{
  unsigned int iNumRegion = vRegion.size()/2;
  for( unsigned int iRegion = 0; iRegion < iNumRegion; iRegion++ )
  {
    ATH_MSG_DEBUG( sCutName<<": " << vRegion.at(iRegion*2) << " to " << vRegion.at(iRegion*2+1) );
  }
}

//______________________________________________________________________________
template<typename T>
void DiTauSelectionTool::PrintConfigValue(const std::string& sCutName, std::vector<T>& vRegion) const
{
  for (auto tVal : vRegion)
    ATH_MSG_DEBUG( sCutName<<": " << tVal );
}

//______________________________________________________________________________
template<typename T>
void DiTauSelectionTool::PrintConfigValue(const std::string& sCutName, T& tVal) const
{
  ATH_MSG_DEBUG( sCutName<<": " << tVal );
}

