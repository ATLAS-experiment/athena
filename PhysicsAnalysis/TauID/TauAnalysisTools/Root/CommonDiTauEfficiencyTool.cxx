/**
 * @copyright Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Framework include(s):
#include "PathResolver/PathResolver.h"

// local include(s)
#include "TauAnalysisTools/CommonDiTauEfficiencyTool.h"
#include "TauAnalysisTools/DiTauEfficiencyCorrectionsTool.h"

// ROOT include(s)
#include "TH2F.h"
#include "TROOT.h"
#include "TKey.h"
#include "TClass.h"
#include <utility>

using namespace TauAnalysisTools;

//______________________________________________________________________________
CommonDiTauEfficiencyTool::CommonDiTauEfficiencyTool(const std::string& sName)
  : asg::AsgTool( sName )
  , m_fXDiTau(&TruthLeadPt)
  , m_fYDiTau(&TruthSubleadPt)
  , m_fZDiTau(&TruthDeltaR)
  , m_sSystematicSet(nullptr)
  , m_bSFIsAvailableDiTau(false)
  , m_bSFIsAvailableCheckedDiTau(false)
  , m_eCheckTruth(TauAnalysisTools::Unknown)
{
}

CommonDiTauEfficiencyTool::~CommonDiTauEfficiencyTool()
{
  if (m_mSF)
    for (auto mEntry : *m_mSF)
      delete std::get<0>(mEntry.second);	
}


/*
  - Find the root files with scale factor inputs on cvmfs using PathResolver
    (more info here:
    https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/PathResolver)
  - Call further functions to process and define NP strings and so on
  - Configure to provide nominal scale factors by default
*/
StatusCode CommonDiTauEfficiencyTool::initialize()
{
  ATH_MSG_INFO( "Initializing CommonDiTauEfficiencyTool" );
  // only read in histograms once
  if (m_mSF==nullptr)
  {
    std::string sInputFilePath = PathResolverFindCalibFile(m_sInputFilePath);

    m_mSF = std::make_unique< tSFMAP >();
    std::unique_ptr< TFile > fSF( TFile::Open( sInputFilePath.c_str(), "READ" ) );
    if(!fSF)
    {
      ATH_MSG_FATAL("Could not open file " << sInputFilePath.c_str());
      return StatusCode::FAILURE;
    }
    ReadInputs(fSF);
    fSF->Close();
  }

  // needed later on in generateSystematicSets(), maybe move it there
  std::vector<std::string> vInputFilePath;
  split(m_sInputFilePath,'/',vInputFilePath);
  m_sInputFileName = vInputFilePath.back();

  generateSystematicSets();

  if (m_sWP.size()>0)
    m_sSFHistName = "sf_"+m_sWP;

  // load empty systematic variation by default
  if (applySystematicVariation(CP::SystematicSet()) != StatusCode::SUCCESS )
    return StatusCode::FAILURE;

  return StatusCode::SUCCESS;
}



/*
  Retrieve the scale factors and if requested the values for the NP's and add
  this stuff in quadrature. Finally return sf_nom +/- n*uncertainty
*/
//______________________________________________________________________________
CP::CorrectionCode CommonDiTauEfficiencyTool::getEfficiencyScaleFactor(const xAOD::DiTauJet& xDiTau,
    double& dEfficiencyScaleFactor)
{
  // check which true state is requestet
  if (!m_bSkipTruthMatchCheck and getTruthParticleType(xDiTau) != m_eCheckTruth)
  {
    dEfficiencyScaleFactor = 1.;
    return CP::CorrectionCode::Ok;
  }

  CP::CorrectionCode tmpCorrectionCode = getValue(m_sSFHistName,
                                                  xDiTau,
                                                  dEfficiencyScaleFactor);
  // return correction code if histogram is not available
  if (tmpCorrectionCode != CP::CorrectionCode::Ok)
    return tmpCorrectionCode;

  // skip further process if systematic set is empty
  if (m_sSystematicSet->size() == 0)
    return CP::CorrectionCode::Ok;

  // get uncertainties summed in quadrature
  double dTotalSystematic2 = 0.;
  double dDirection = 0.;
  for (const auto & syst : *m_sSystematicSet)
  {

    // check if systematic is available
    auto it = m_mSystematicsHistNames.find(syst.basename());
    if (it == m_mSystematicsHistNames.end())[[unlikely]] continue;
    // get uncertainty value
    double dUncertaintySyst = 0.;

    // needed for up/down decision
    dDirection = syst.parameter();

    // build up histogram name
    std::string sHistName = it->second;
    if (dDirection>0)   sHistName+="_up";
    else                sHistName+="_down";
    if (!m_sWP.empty()) sHistName+="_"+m_sWP;

    // get the uncertainty from the histogram
    tmpCorrectionCode = getValue(sHistName,
                                 xDiTau,
                                 dUncertaintySyst);

    // return correction code if histogram is not available
    if (tmpCorrectionCode != CP::CorrectionCode::Ok)
      return tmpCorrectionCode;

    // scale uncertainty with direction, i.e. +/- n*sigma
    dUncertaintySyst *= dDirection;

    // square uncertainty and add to total uncertainty
    dTotalSystematic2 += dUncertaintySyst * dUncertaintySyst;
  }

  // now use dDirection to use up/down uncertainty
  dDirection = (dDirection > 0.) ? 1. : -1.;

  // finally apply uncertainty (eff * ( 1 +/- \sum  )
  dEfficiencyScaleFactor *= 1. + dDirection * std::sqrt(dTotalSystematic2);

  return CP::CorrectionCode::Ok;
}

/*
  Get scale factor from getEfficiencyScaleFactor and decorate it to the
  tau. Note that this can only be done if the variable name is not already used,
  e.g. if the variable was already decorated on a previous step (enured by the
  m_bSFIsAvailableCheckedDiTau check).

  Technical note: cannot use `static SG::Decorator` as we will have
  multiple instances of this tool with different decoration names.
*/
//______________________________________________________________________________
CP::CorrectionCode CommonDiTauEfficiencyTool::applyEfficiencyScaleFactor(const xAOD::DiTauJet& xDiTau)
{
  double dSf = 0.;

  SG::Decorator< double > decor (m_sVarName);
  if (!m_bSFIsAvailableCheckedDiTau)
  {
    m_bSFIsAvailableDiTau = decor.isAvailable(xDiTau);
    m_bSFIsAvailableCheckedDiTau = true;
    if (m_bSFIsAvailableDiTau)
    {
      ATH_MSG_DEBUG(m_sVarName << " decoration is available on first ditau processed, switched of applyEfficiencyScaleFactor for further ditaus.");
      ATH_MSG_DEBUG("If an application of efficiency scale factors needs to be redone, please pass a shallow copy of the original ditau.");
    }
  }
  if (m_bSFIsAvailableDiTau)
    return CP::CorrectionCode::Ok;

  // retrieve scale factor
  CP::CorrectionCode tmpCorrectionCode = getEfficiencyScaleFactor(xDiTau, dSf);
  // adding scale factor to tau as decoration
  decor(xDiTau) = dSf;

  return tmpCorrectionCode;
}

/*
  standard check if a systematic is available
*/
//______________________________________________________________________________
bool CommonDiTauEfficiencyTool::isAffectedBySystematic( const CP::SystematicVariation& systematic ) const
{
  CP::SystematicSet sys = affectingSystematics();
  return sys.find(systematic) != sys.end();
}

/*
  standard way to return systematics that are available (including recommended
  systematics)
*/
//______________________________________________________________________________
CP::SystematicSet CommonDiTauEfficiencyTool::affectingSystematics() const
{
  return m_sAffectingSystematics;
}

/*
  standard way to return systematics that are recommended
*/
//______________________________________________________________________________
CP::SystematicSet CommonDiTauEfficiencyTool::recommendedSystematics() const
{
  return m_sRecommendedSystematics;
}

/*
  Configure the tool to use a systematic variation for further usage, until the
  tool is reconfigured with this function. The passed systematic set is checked
  for sanity:
    - unsupported systematics are skipped
    - only combinations of up or down supported systematics is allowed
    - don't mix recommended systematics with other available systematics, cause
      sometimes recommended are a quadratic sum of the other variations,
      e.g. TOTAL=(SYST^2 + STAT^2)^0.5
*/
//______________________________________________________________________________
StatusCode CommonDiTauEfficiencyTool::applySystematicVariation ( const CP::SystematicSet& sSystematicSet)
{

  // first check if we already know this systematic configuration
  auto itSystematicSet = m_mSystematicSets.find(sSystematicSet);
  if (itSystematicSet != m_mSystematicSets.end())
  {
    m_sSystematicSet = &itSystematicSet->first;
    return StatusCode::SUCCESS;
  }

  // sanity checks if systematic set is supported
  double dDirection = 0.;
  CP::SystematicSet sSystematicSetAvailable;
  for (auto sSyst : sSystematicSet)
  {
    // check if systematic is available
    auto it = m_mSystematicsHistNames.find(sSyst.basename());
    if (it == m_mSystematicsHistNames.end())
    {
      ATH_MSG_VERBOSE("unsupported systematic variation: "<< sSyst.basename()<<"; skipping this one");
      continue;
    }


    if (sSyst.parameter() * dDirection < 0)
    {
      ATH_MSG_ERROR("unsupported set of systematic variations, you should either use only \"UP\" or only \"DOWN\" systematics in one set!");
      ATH_MSG_ERROR("systematic set will not be applied");
      return StatusCode::FAILURE;
    }
    dDirection = sSyst.parameter();

    if ((m_sRecommendedSystematics.find(sSyst.basename()) != m_sRecommendedSystematics.end()) and sSystematicSet.size() > 1)
    {
      ATH_MSG_ERROR("unsupported set of systematic variations, you should not combine \"TAUS_{TRUE|FAKE}_EFF_*_TOTAL\" with other systematic variations!");
      ATH_MSG_ERROR("systematic set will not be applied");
      return StatusCode::FAILURE;
    }

    // finally add the systematic to the set of systematics to process
    sSystematicSetAvailable.insert(sSyst);
  }

  // store this calibration for future use, and make it current
  m_sSystematicSet = &m_mSystematicSets.insert(std::pair<CP::SystematicSet,std::string>(sSystematicSetAvailable, sSystematicSet.name())).first->first;

  return StatusCode::SUCCESS;
}

//=================================PRIVATE-PART=================================
void CommonDiTauEfficiencyTool::ReadInputs(std::unique_ptr<TFile> &fFile)
{
  m_mSF->clear();

  // initialize function pointer
  m_fXDiTau = &TruthLeadPt;
  m_fYDiTau = &TruthSubleadPt;
  m_fZDiTau = &TruthDeltaR;

  TKey *kKey;
  TIter itNext(fFile->GetListOfKeys());
  while ((kKey = (TKey*)itNext()))
  {
    // parse file content for objects of type TNamed, check their title for
    // known strings and reset funtion pointer
    std::string sKeyName = kKey->GetName();

    std::vector<std::string> vSplitName = {};
    split(sKeyName,'_',vSplitName);
    if (vSplitName[0] == "sf")
    {
      addHistogramToSFMap(kKey, sKeyName);
    }
    else
    {
      if (sKeyName.find("_up_") != std::string::npos or sKeyName.find("_down_") != std::string::npos)
        addHistogramToSFMap(kKey, sKeyName);
      else
      {
        size_t iPos = sKeyName.find('_');
        addHistogramToSFMap(kKey, sKeyName.substr(0,iPos)+"_up"+sKeyName.substr(iPos));
        addHistogramToSFMap(kKey, sKeyName.substr(0,iPos)+"_down"+sKeyName.substr(iPos));
      }
    }
  }
  ATH_MSG_INFO("data loaded from " << fFile->GetName());
}

/*
  Create the tuple objects for the map
*/
//______________________________________________________________________________
void CommonDiTauEfficiencyTool::addHistogramToSFMap(TKey* kKey, const std::string& sKeyName)
{
  // handling for the 3 different input types TH1F/TH1D/TF1, function pointer
  // handle the access methods for the final scale factor retrieval
  TClass *cClass = gROOT->GetClass(kKey->GetClassName());
  if (cClass->InheritsFrom("TH2"))
  {
    TH2* oObject = static_cast<TH2*>(kKey->ReadObj());
    oObject->SetDirectory(0);
    (*m_mSF)[sKeyName] = tTupleObjectFunc(oObject,&getValueTH2);
    ATH_MSG_DEBUG("added histogram with name "<<sKeyName);
  }
  else if (cClass->InheritsFrom("TH1"))
  {
    TH1* oObject = static_cast<TH1*>(kKey->ReadObj());
    oObject->SetDirectory(0);
    (*m_mSF)[sKeyName] = tTupleObjectFunc(oObject,&getValueTH1);
    ATH_MSG_DEBUG("added histogram with name "<<sKeyName);
  }
  else
  {
    ATH_MSG_DEBUG("ignored object with name "<<sKeyName);
  }
}


/*  
  This function parses the names of the objects from the input file and
  generates the systematic sets and defines which ones are recommended or only
  available. It also checks, based on the root file name, on which tau it needs
  to be applied, e.g. only on reco taus coming from true taus or on those faked
  by true electrons...
  Examples:
  filename: JetID_TrueHadDiTau_2017-fall.root -> apply only to true ditaus
  histname: sf_* -> nominal scale factor
  histname: TOTAL_* -> "total" NP, recommended
  histname: afii_* -> "total" NP, not recommended, but available
*/
//______________________________________________________________________________
void CommonDiTauEfficiencyTool::generateSystematicSets()
{
  // creation of basic string for all NPs, e.g. "TAUS_TRUEHADTAU_EFF_RECO_"
  std::vector<std::string> vSplitInputFilePath = {};
  split(m_sInputFileName,'_',vSplitInputFilePath);
  std::string sEfficiencyType = vSplitInputFilePath.at(0);
  std::string sTruthType = vSplitInputFilePath.at(1);
  std::transform(sEfficiencyType.begin(), sEfficiencyType.end(), sEfficiencyType.begin(), toupper);
  std::transform(sTruthType.begin(), sTruthType.end(), sTruthType.begin(), toupper);
  std::string sSystematicBaseString = "TAUS_"+sTruthType+"_EFF_"+sEfficiencyType+"_";
  // set truth type to check for in truth matching
  if (sTruthType=="TRUEHADTAU") m_eCheckTruth = TauAnalysisTools::TruthHadronicTau;
  if (sTruthType=="TRUEHADDITAU") m_eCheckTruth = TauAnalysisTools::TruthHadronicDiTau;

  for (const auto & mSF : *m_mSF)
  {
    // parse for nuisance parameter in histogram name
    std::vector<std::string> vSplitNP = {};
    split(mSF.first,'_',vSplitNP);
    std::string sNP = vSplitNP.at(0);
    std::string sNPUppercase = vSplitNP.at(0);
    // skip nominal scale factors
    if (sNP == "sf") continue;
    // test if NP starts with a capital letter indicating that this should be recommended
    bool bIsRecommended = false;
    if (isupper(sNP.at(0)))
      bIsRecommended = true;
    // make sNP uppercase and build final NP entry name
    std::transform(sNPUppercase.begin(), sNPUppercase.end(), sNPUppercase.begin(), toupper);
    std::string sSystematicString = sSystematicBaseString+sNPUppercase;
    // add all found systematics to the AffectingSystematics
    m_sAffectingSystematics.insert(CP::SystematicVariation (sSystematicString, 1));
    m_sAffectingSystematics.insert(CP::SystematicVariation (sSystematicString, -1));
    // only add found uppercase systematics to the RecommendedSystematics
    if (bIsRecommended)
    {
      m_sRecommendedSystematics.insert(CP::SystematicVariation (sSystematicString, 1));
      m_sRecommendedSystematics.insert(CP::SystematicVariation (sSystematicString, -1));
    }
    ATH_MSG_DEBUG("connected base name " << sNP << " with systematic " <<sSystematicString);
    m_mSystematicsHistNames.insert({sSystematicString,sNP});
  }
}

/*
  return value from the tuple map object based on the pt/eta values (or the
  corresponding value in case of configuration)
*/
//______________________________________________________________________________
CP::CorrectionCode CommonDiTauEfficiencyTool::getValue(const std::string& sHistName,
    const xAOD::DiTauJet& xDiTau,
    double& dEfficiencyScaleFactor) const
{
  const tSFMAP& mSF = *m_mSF;
  auto it = mSF.find (sHistName);
  if (it == mSF.end())
  {
    ATH_MSG_ERROR("Object with name "<<sHistName<<" was not found in input file.");
    ATH_MSG_DEBUG("Content of input file");
    for (auto eEntry : mSF)
      ATH_MSG_DEBUG("  Entry: "<<eEntry.first);
    return CP::CorrectionCode::Error;
  }

  // get a tuple (TObject*,functionPointer) from the scale factor map
  tTupleObjectFunc tTuple = it->second;

  // get pt and eta (for x and y axis respectively)
  double dX = m_fXDiTau(xDiTau);
  double dY = m_fYDiTau(xDiTau);
  double dZ = m_fZDiTau(xDiTau);

  double dVars[3] = {dX, dY, dZ};
  // finally obtain efficiency scale factor from TH1F/TH1D/TF1, by calling the
  // function pointer stored in the tuple from the scale factor map
  return  (std::get<1>(tTuple))(std::get<0>(tTuple), dEfficiencyScaleFactor, dVars);
}

//______________________________________________________________________________
double TauAnalysisTools::TruthLeadPt(const xAOD::DiTauJet& xDiTau)
{
  // return leading truth tau pt in GeV
  static const SG::ConstAccessor< float > acc( "TruthVisLeadPt" );
  return acc( xDiTau ) * 0.001;
}

//______________________________________________________________________________
double TauAnalysisTools::TruthSubleadPt(const xAOD::DiTauJet& xDiTau)
{
  // return subleading truth tau pt in GeV
  static const SG::ConstAccessor< float > acc( "TruthVisSubleadPt" );
  return acc( xDiTau ) * 0.001;
}

//______________________________________________________________________________
double TauAnalysisTools::TruthDeltaR(const xAOD::DiTauJet& xDiTau)
{
  // return truth taus distance delta R
  static const SG::ConstAccessor< float > acc( "TruthVisDeltaR" );
  return acc( xDiTau );
}

/*
  find the particular value in TH1 depending on pt (or the
  corresponding value in case of configuration)
  Note: In case values are outside of bin ranges, the closest bin value is used
*/
//______________________________________________________________________________
CP::CorrectionCode CommonDiTauEfficiencyTool::getValueTH1(const TObject* oObject,
    double& dEfficiencyScaleFactor, double dVars[])
{
  double dPt = dVars[0];

  const TH1* hHist = dynamic_cast<const TH1*>(oObject);

  if (!hHist)
  {
    // ATH_MSG_ERROR("Problem with casting TObject of type "<<oObject->ClassName()<<" to TH2F");
    return CP::CorrectionCode::Error;
  }

  // protect values from underflow bins
  dPt = std::max(dPt,hHist->GetXaxis()->GetXmin());
  // protect values from overflow bins (times .999 to keep it inside last bin)
  dPt = std::min(dPt,hHist->GetXaxis()->GetXmax() * .999);

  // get bin from TH2 depending on x and y values; finally set the scale factor
  int iBin = hHist->FindFixBin(dPt);
  dEfficiencyScaleFactor = hHist->GetBinContent(iBin);
  return CP::CorrectionCode::Ok;
}

/*
  find the particular value in TH2 depending on pt and eta (or the
  corresponding value in case of configuration)
  Note: In case values are outside of bin ranges, the closest bin value is used
*/
//______________________________________________________________________________
CP::CorrectionCode CommonDiTauEfficiencyTool::getValueTH2(const TObject* oObject,
    double& dEfficiencyScaleFactor, double dVars[])
{
  double dPt = dVars[0];
  double dEta = dVars[1];

  const TH2* hHist = dynamic_cast<const TH2*>(oObject);

  if (!hHist)
  {
    // ATH_MSG_ERROR("Problem with casting TObject of type "<<oObject->ClassName()<<" to TH2F");
    return CP::CorrectionCode::Error;
  }

  // protect values from underflow bins
  dPt = std::max(dPt,hHist->GetXaxis()->GetXmin());
  dEta = std::max(dEta,hHist->GetYaxis()->GetXmin());
  // protect values from overflow bins (times .999 to keep it inside last bin)
  dPt = std::min(dPt,hHist->GetXaxis()->GetXmax() * .999);
  dEta = std::min(dEta,hHist->GetYaxis()->GetXmax() * .999);

  // get bin from TH2 depending on x and y values; finally set the scale factor
  int iBin = hHist->FindFixBin(dPt,dEta);
  dEfficiencyScaleFactor = hHist->GetBinContent(iBin);
  return CP::CorrectionCode::Ok;
}

