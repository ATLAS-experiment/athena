/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class AthPhotonEfficiencyCorrectionTool
   @brief Calculate the photon scale factors in Athena

   @author Michael Pitt <michael.pitt@cern.ch>, Giovanni Marchiori
   @date   February 2018
*/

// Include this class's header
#include "PhotonEfficiencyCorrection/AsgPhotonEfficiencyCorrectionTool.h"

// STL includes
#include <cfloat>
#include <climits>
#include <iostream>
#include <string>

// Include the return object
#include "PATCore/PATCoreEnums.h"

// xAOD includes
#include "xAODEgamma/Egamma.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "PathResolver/PathResolver.h"

// ROOT includes
#include "TSystem.h"
#define MAXETA 2.47
#define MIN_ET 7000.0

using Result = Root::TElectronEfficiencyCorrectionTool::Result;
namespace correlationModel {
enum model
{
  FULL = 0,
  TOTAL = 1
};
}

// =============================================================================
// Standard constructor
// =============================================================================
AsgPhotonEfficiencyCorrectionTool::AsgPhotonEfficiencyCorrectionTool( const std::string& myname ) : 
  AsgTool(myname),
  m_rootTool_unc(nullptr),
  m_rootTool_con(nullptr),
  m_appliedSystematics(nullptr),
  m_correlation_model(correlationModel::FULL),
  m_sysSubstring(""),
  m_corrVarUp{},
  m_corrVarDown{},
  m_uncorrVarUp{},
  m_uncorrVarDown{}
{

  // Create an instances of the underlying ROOT tools
  m_rootTool_unc = new Root::TElectronEfficiencyCorrectionTool();
  m_rootTool_con = new Root::TElectronEfficiencyCorrectionTool();

  // Declare the needed properties
  declareProperty( "CorrectionFileNameConv", m_corrFileNameConv="",
                   "File that stores the correction factors for simulation for converted photons");

  declareProperty( "CorrectionFileNameUnconv", m_corrFileNameUnconv="",
                   "File that stores the correction factors for simulation for unconverted photons");
				   
  //  declareProperty("MapFilePath", m_mapFile = "PhotonEfficiencyCorrection/2015_2025/rel22.2/2024_FinalRun2_Recommendation_v1/map1.txt",
    declareProperty("MapFilePath", m_mapFile = "map_For_test.txt",
                  "Full path to the map file");  
				  
  declareProperty( "ForceDataType", m_dataTypeOverwrite=-1,
                   "Force the DataType of the Photon to specified value");

  declareProperty( "ResultPrefix",       m_resultPrefix="", "The prefix string for the result");
  declareProperty( "ResultName",         m_resultName="",   "The string for the result");
  
  // Properties needed for isolation corrections
  declareProperty( "IsoKey",         m_isoWP="",   "Set isolation WP, if this string is empty the tool will return ID SF");
  
  // Properties needed for trigger SF
  declareProperty( "TriggerKey",         m_trigger="",   "Set trigger, if this string is empty the tool will return ID SF");
  
  // Properties related to the receiving of event run number
  declareProperty("UseRandomRunNumber",  m_useRandomRunNumber = true,
                                        "Set if use RandomRunNumber from eventinfo");
  declareProperty("DefaultRandomRunNumber",  m_defaultRandomRunNumber = 999999,
                                        "Set default run number manually");
  declareProperty("removeTRTConversion", m_removeTRTConversion = true, 
  		  "boolean to treat barrel standalone TRT conversion as unconverted for Run3 ");
  declareProperty("CorrelationModel",
                  m_correlation_model_name = "TOTAL",
                  "Uncertainty correlation model. At the moment TOTAL, FULL,"
		  "default is FULL");
										

}

// =============================================================================
// Standard destructor
// =============================================================================
AsgPhotonEfficiencyCorrectionTool::~AsgPhotonEfficiencyCorrectionTool()
{
  if ( m_rootTool_unc ) delete m_rootTool_unc;
  if ( m_rootTool_con ) delete m_rootTool_con;
}

// =============================================================================
// Athena initialize method
// =============================================================================
StatusCode AsgPhotonEfficiencyCorrectionTool::initialize()
{
  // Resolve the paths to the input files
  std::vector < std::string > corrFileNameList;

  // First check if the tool is initialized using the input files or map
  if(!m_mapFile.empty()){ // using map file
         corrFileNameList.push_back(getFileName(m_isoWP,m_trigger,true)); // converted photons input
	 corrFileNameList.push_back(getFileName(m_isoWP,m_trigger,false)); // unconverted photons input
  }
  else if(!m_corrFileNameConv.empty() && !m_corrFileNameUnconv.empty()){ // initialize the tool using input files (old scheme)
  	corrFileNameList.push_back(m_corrFileNameConv);
	corrFileNameList.push_back(m_corrFileNameUnconv);
  }
  else{
      ATH_MSG_ERROR ( "Fail to resolve input file name, check if you set MapFilePath or CorrectionFileName properly" );
      return StatusCode::FAILURE ; 
  }

  // once the input files are retrieved, update the path using PathResolver or TOOL/data folder
  for (auto & i : corrFileNameList){   
    //std::cout<<"i is "<<i<<" "<<std::endl;
    //Using the PathResolver to locate the file
    //std::string filename = PathResolverFindCalibFile( i );//Put it back after local test!
    std::string filename =  "/afs/cern.ch/work/r/rhulsken/private/PhotonId/PhotonEffCorr/Temp_files/"+i ;
    
    if (filename.empty()){
      ATH_MSG_ERROR ( "Could NOT resolve file name " << i );
      return StatusCode::FAILURE ;
    } else{
      ATH_MSG_INFO(" Using path = "<<filename);
    }

    i = filename;

  }
  //std::cout<<"corr[0] "<<corrFileNameList[0]<<std::endl;
  //std::cout<<"prefix ID "<<m_file_prefix_ID<<std::endl;
  // Set prefix for sustematics if this is ISO, Trigger or ID SF
  if( corrFileNameList[0].find(m_file_prefix_Trig) != std::string::npos) m_sysSubstring="TRIGGER_";
  if( corrFileNameList[0].find(m_file_prefix_ID) != std::string::npos) m_sysSubstring="ID_";
  if( corrFileNameList[0].find(m_file_prefix_ISO) != std::string::npos) m_sysSubstring="ISO_";
  if( corrFileNameList[0].find(m_file_prefix_TrigEff) != std::string::npos) m_sysSubstring="TRIGGER_";
  if(m_sysSubstring.empty()) {ATH_MSG_ERROR ( "Invalid input file" ); return StatusCode::FAILURE;}


  if (m_correlation_model_name == "FULL") {
    m_correlation_model = correlationModel::FULL;
  } else if (m_correlation_model_name == "TOTAL") {
    m_correlation_model = correlationModel::TOTAL;
  } else {
    ATH_MSG_ERROR("Unknown correlation model " + m_correlation_model_name);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Correlation model: " + m_correlation_model_name
                << " Enum " << m_correlation_model);
  
  
  
  // Configure the underlying Root tool
  m_rootTool_con->addFileName( corrFileNameList[0] );
  m_rootTool_unc->addFileName( corrFileNameList[1] );

  // Forward the message level
  m_rootTool_con->msg().setLevel(this->msg().level());
  m_rootTool_unc->msg().setLevel(this->msg().level());

  
  // We need to initialize the underlying ROOT TSelectorTool
  if ( (0 == m_rootTool_con->initialize()) || (0 == m_rootTool_unc->initialize()) )
    {
      ATH_MSG_ERROR("Could not initialize the TElectronEfficiencyCorrectionTool!");
      return StatusCode::FAILURE;
    }

  // get Nsyst
  m_nCorrSyst = m_rootTool_con->getNSyst();
  //std::cout<<"Number of Systematics "<<m_nCorrSyst<<std::endl;

  
  // get the map of pt/eta bins
  // let's start with converted 
  m_rootTool_con->getNbins(m_pteta_bins);
  std::map<float, std::vector<float>> pteta_bins_unc;
  // now let's get unconverted
  m_rootTool_unc->getNbins(pteta_bins_unc);
  // let's loop the unconverted map and copy over to the common one
  // in this tool we only ever care about et, so don't care if we overwrite eta information
  for (const auto& [pt_unc, eta_unc]: pteta_bins_unc) {
    m_pteta_bins[pt_unc] = eta_unc;
  }

  // Add the recommended systematics to the registry
  if ( registerSystematics() != StatusCode::SUCCESS) {
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS ;
}


// =============================================================================
// The main accept method: the actual cuts are applied here 
// =============================================================================
CP::CorrectionCode AsgPhotonEfficiencyCorrectionTool::calculate( const xAOD::Egamma* egam, Result& result ) const
{

  if ( !egam ) {
    ATH_MSG_ERROR ( "Did NOT get a valid egamma pointer!" );
    return CP::CorrectionCode::Error;
  }

  // retrieve transverse energy from e/cosh(etaS2)
  const xAOD::CaloCluster* cluster  = egam->caloCluster();  
  if (!cluster){
    ATH_MSG_ERROR("No  cluster associated to the Photon \n"); 
    return  CP::CorrectionCode::Error;
  } 

  // use et from cluster because it is immutable under syst variations of ele energy scale
  const double energy = cluster->e();
  double et = 0.;
  if ( std::abs(egam->eta()) < 999.) {
    const double cosheta = std::cosh(egam->eta());
    et = (cosheta != 0.) ? energy / cosheta : 0.;
  }

  // eta from second layer
  double eta2 = cluster->etaBE(2);

  // allow for a 5% margin at the lowest pT bin boundary (i.e. increase et by 5% 
  // for sub-threshold electrons). This assures that electrons that pass the 
  // threshold only under syst variations of energy get a scale factor assigned.
  std::map<float, std::vector<float>>::const_iterator itr_pt = m_pteta_bins.begin();
  if (itr_pt!=m_pteta_bins.end() && et<itr_pt->first) {
    et=et*1.05;
  }

  // Check if photon in the range to get the SF
  if (std::abs(eta2) > MAXETA) {
    result.SF = 1;
    result.Total = 1;
    ATH_MSG_DEBUG("No correction factor provided for eta "
                  << eta2 << " Returning SF = 1 + / - 1");
    return CP::CorrectionCode::OutOfValidityRange;
  }
  if (et < MIN_ET) {
    result.SF = 1;
    result.Total = 1;
    ATH_MSG_DEBUG("No correction factor provided for eT "
                  << et << " Returning SF = 1 + / - 1");
    return CP::CorrectionCode::OutOfValidityRange;
  }
  if (itr_pt != m_pteta_bins.end() && et < itr_pt->first) {
    result.SF = 1;
    result.Total = 1;
    ATH_MSG_DEBUG("No scale factor uncertainty provided for et "
                  << et / 1e3 << "GeV Returning SF = 1 + / - 1");
    return CP::CorrectionCode::OutOfValidityRange;
  }

  // Get the run number
  const xAOD::EventInfo* eventInfo =
      evtStore()->retrieve<const xAOD::EventInfo>("EventInfo");
  if (!eventInfo) {
    ATH_MSG_ERROR("Could not retrieve EventInfo object!");
    return CP::CorrectionCode::Error;
  }

  
  // Retrieve the proper random Run Number
  unsigned int runnumber = m_defaultRandomRunNumber;
  if (m_useRandomRunNumber) {
    static const SG::AuxElement::Accessor<unsigned int> randomrunnumber(
        "RandomRunNumber");
    if (!randomrunnumber.isAvailable(*eventInfo)) {
      ATH_MSG_WARNING(
          "Pileup tool not run before using PhotonEfficiencyTool! SFs do not "
          "reflect PU distribution in data");
      return CP::CorrectionCode::Error;
    }
    runnumber = randomrunnumber(*(eventInfo));
  }

  //std::cout<<"Run Number"<<runnumber<<std::endl;

  /* For now the dataType must be set by the user. May be added to the IParticle
   * class later.  */
  // probably event info should be able to tell us if it's data, fullsim, AF,..
  PATCore::ParticleDataType::DataType dataType =
      PATCore::ParticleDataType::DataType::Data;
  if (m_dataTypeOverwrite >= 0)
    dataType = (PATCore::ParticleDataType::DataType)m_dataTypeOverwrite;


   //exclude TRT 
   bool excludeTRT = false;
   if(runnumber >= 410000 && m_removeTRTConversion) excludeTRT = true;
  // check if converted
  const bool isConv = xAOD::EgammaHelpers::isConvertedPhoton(egam, excludeTRT);
  //std::cout<<dataType<<std::endl;
  //std::cout<<"arrived Here"<<std::endl;
  // Call the ROOT tool to get an answer (for photons we need just the total)
  const int status = isConv ? m_rootTool_con->calculate(dataType, runnumber,
                                                        eta2, et, result, false)
                            : m_rootTool_unc->calculate(dataType, runnumber,
                                                        eta2, et, result, false);
  //std::cout<<status<<std::endl;
  // if status 0 something went wrong
  if (!status) {
    result.SF = 1;
    result.Total = 1;
    return CP::CorrectionCode::OutOfValidityRange;
  }
  /* if (m_nCorrSyst !=0)
    {
      for (int i = 0 ; i<m_nCorrSyst; i++)
	{
	  std::cout<<"for corr "<<i <<": "<<result.Corr[i]<<std::endl;
	}
	}*/
  
  return CP::CorrectionCode::Ok;
}

CP::CorrectionCode AsgPhotonEfficiencyCorrectionTool::getEfficiencyScaleFactor(const xAOD::Egamma& inputObject, double& efficiencyScaleFactor) const{
  
  Result sfresult;
  CP::CorrectionCode status = calculate(&inputObject, sfresult);

  if ( status != CP::CorrectionCode::Ok ) {
    return status;
  }
  //Get the result + the uncertainty
  float sigma(0);
  
  if(m_appliedSystematics==nullptr){
    efficiencyScaleFactor=sfresult.SF;
    return CP::CorrectionCode::Ok;
  }
  //std::cout<<appliedSystematics().name()<<std::endl;
  if (appliedSystematics().name().size() != 0)
    {
      TString last = appliedSystematics().name().substr(appliedSystematics().name().size()-1,appliedSystematics().name().size());
      std::string uncert_name="";
      if (last == "n")
	{
	  uncert_name = appliedSystematics().name().substr(0,appliedSystematics().name().size()-7);
	  
	}
      else if (last == "p")
	{
	  uncert_name = appliedSystematics().name().substr(0,appliedSystematics().name().size()-5);
	  
	}
      //std::cout<<uncert_name<<std::endl;
      sigma=appliedSystematics().getParameterByBaseName(uncert_name);

      std::string Number_uncert = uncert_name.substr(uncert_name.size()-1,uncert_name.size());
      if (Number_uncert=="y")
	{
	  efficiencyScaleFactor=sfresult.SF+sigma*sfresult.Total;
	}
      else
	{
	  efficiencyScaleFactor=sfresult.SF+sigma*sfresult.Corr[std::stoi(Number_uncert)];

	}
    }
  else {
    sigma=appliedSystematics().getParameterByBaseName("PH_EFF_"+m_sysSubstring+"Uncertainty");
    //std::cout<<"Sigma "<<sigma<<" "<<m_sysSubstring<<std::endl;
    efficiencyScaleFactor=sfresult.SF+sigma*sfresult.Total;
    //std::cout<<"Total "<<sfresult.Total<<std::endl;
    //std::cout<<"SF "<< sfresult.SF<<std::endl;

  }
  //  std::cout<<"Scale Factor "<<efficiencyScaleFactor<<std::endl;

  int m_nCorrSyst = m_rootTool_con->getNSyst();

  //std::cout<<"Corr systematics "<<m_nCorrSyst<<std::endl;

 
  
  return  CP::CorrectionCode::Ok;
}

CP::CorrectionCode AsgPhotonEfficiencyCorrectionTool::getEfficiencyScaleFactorError(const xAOD::Egamma& inputObject, double& efficiencyScaleFactorError) const{   

  Result sfresult;
  CP::CorrectionCode status = calculate(&inputObject, sfresult);

  if ( status != CP::CorrectionCode::Ok ) {
    return status;
  }

  efficiencyScaleFactorError=sfresult.Total;
  return  CP::CorrectionCode::Ok;
}

CP::CorrectionCode AsgPhotonEfficiencyCorrectionTool::applyEfficiencyScaleFactor(xAOD::Egamma& inputObject) const {
  
  double efficiencyScaleFactor = 1.0;
  CP::CorrectionCode result = getEfficiencyScaleFactor(inputObject, efficiencyScaleFactor);
  const static SG::AuxElement::Decorator<float> dec(m_resultPrefix+m_resultName+"SF");
  dec(inputObject) = efficiencyScaleFactor;
  return result;
}

//=======================================================================
//   Systematics Interface
//=======================================================================
bool AsgPhotonEfficiencyCorrectionTool::isAffectedBySystematic( const CP::SystematicVariation& systematic ) const {
	if(!systematic.empty()){
		CP::SystematicSet sys = affectingSystematics();
		return sys.find(systematic) != sys.end();
	}
	return true;
}


/// returns: the list of all systematics this tool can be affected by (for now keep +-1 sigma variation, but ignore it later in applySystematicVariation() )
CP::SystematicSet AsgPhotonEfficiencyCorrectionTool::affectingSystematics() const {
  CP::SystematicSet mySysSet;
 
  mySysSet.insert(CP::SystematicVariation("PH_EFF_"+m_sysSubstring+"Uncertainty", CP::SystematicVariation::CONTINUOUS));
  if (m_correlation_model_name == "TOTAL") {
    mySysSet.insert(CP::SystematicVariation("PH_EFF_"+m_sysSubstring+"Uncertainty", 1));
    mySysSet.insert(CP::SystematicVariation("PH_EFF_"+m_sysSubstring+"Uncertainty", -1));
  }
  else if (m_correlation_model_name == "FULL") {
    for (int i = 0; i < m_nCorrSyst; ++i) {
      mySysSet.insert(CP::SystematicVariation("PH_EFF_"+m_sysSubstring+ Form("Corr_NP%d", i), 1));
      mySysSet.insert(CP::SystematicVariation("PH_EFF_"+m_sysSubstring+ Form("Corr_NP%d", i), -1));
    }
  }
   
  return mySysSet;
}

/// Register the systematics with the registry and add them to the recommended list
StatusCode AsgPhotonEfficiencyCorrectionTool::registerSystematics() {
  CP::SystematicRegistry& registry = CP::SystematicRegistry::getInstance();
  if (registry.registerSystematics(*this) != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("Failed to add systematic to list of recommended systematics.");
	return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

/// returns: the list of all systematics this tool recommends to use
CP::SystematicSet AsgPhotonEfficiencyCorrectionTool::recommendedSystematics() const {
  CP::SystematicSet mySysSet;
  if (m_correlation_model_name == "TOTAL") {
  mySysSet.insert(CP::SystematicVariation("PH_EFF_"+m_sysSubstring+"Uncertainty", 1));
  mySysSet.insert(CP::SystematicVariation("PH_EFF_"+m_sysSubstring+"Uncertainty", -1));
  }
  else if (m_correlation_model_name == "FULL") {
    for (int i = 0; i < m_nCorrSyst; ++i) {
      mySysSet.insert(CP::SystematicVariation("PH_EFF_"+m_sysSubstring+ Form("Corr_NP%d", i), 1));
      mySysSet.insert(CP::SystematicVariation("PH_EFF_"+m_sysSubstring+ Form("Corr_NP%d", i), -1));
    }
    }
   
  return mySysSet;
}


StatusCode AsgPhotonEfficiencyCorrectionTool::
applySystematicVariation ( const CP::SystematicSet& systConfig )
{
  // First, check if we already know this systematic configuration
  auto itr = m_systFilter.find(systConfig);

  // If it's a new input set, we need to filter it
  if( itr == m_systFilter.end() ){

    CP::SystematicSet affectingSys = affectingSystematics();
    CP::SystematicSet filteredSys;   
    if (!CP::SystematicSet::filterForAffectingSystematics(systConfig, affectingSys, filteredSys)){
      ATH_MSG_ERROR("Unsupported combination of systematics passed to the tool!");
      return StatusCode::FAILURE;
    }
    /*   //COPIED FROM ELECTRON CODE
    // Does filtered make sense,  only one per time
    if (filteredSys.size() > 1) {
      ATH_MSG_ERROR(
        "More than one systematic variation passed at the same time");
      return StatusCode::FAILURE;
    }

    if (filteredSys.empty() && !systConfig.empty()) {
      ATH_MSG_DEBUG("systematics : ");
      for (const auto& syst : systConfig) {
        ATH_MSG_DEBUG(syst.name());
      }
      ATH_MSG_DEBUG(" Not supported ");
    }
    //END COPIED FROM ELECTRON CODE*/


    // Insert filtered set into the map
    itr = m_systFilter.insert(std::make_pair(systConfig, filteredSys)).first;
  }
  
  CP::SystematicSet& mySysConf = itr->second;
  m_appliedSystematics = &mySysConf;
  return StatusCode::SUCCESS;
}

//===============================================================================
// Map Key Feature
//===============================================================================
// Gets the correction filename from map
std::string AsgPhotonEfficiencyCorrectionTool::getFileName(const std::string& isoWP, const std::string& trigWP, bool isConv) {  

  // First locate the map file:
  std::string mapFileName2 = PathResolverFindCalibFile( m_mapFile );//Put it back after local test!
  std::cout<<mapFileName2<< " Is the PATH"<<std::endl; 
  
  std::string mapFileName = "/afs/cern.ch/work/r/rhulsken/private/PhotonId/PhotonEffCorr/Temp_files/"+ m_mapFile ;
  if(mapFileName.empty()){
	ATH_MSG_ERROR ( "Can't read map file " << m_mapFile );
	return mapFileName;	// return an empty string
  }
  
  // Construct correction type:
  std::string correction_type = "ID_Tight";
  if(!trigWP.empty()) correction_type = "Trigger_"+trigWP+"_"+isoWP;
  else if(!isoWP.empty()) correction_type = "ISO_"+isoWP;
  
  // trigger SF same for con/unc photons
  if(trigWP.empty()) {correction_type += isConv ? "_Converted" : "_Unconverted";}

  std::string value;
  
  // Read the map file to find the proper correction filename
  std::ifstream is(mapFileName);
  if (!is.is_open()){
    ATH_MSG_ERROR("Couldn't read Map File" + mapFileName);
	return  "";
  }
  while (!is.eof()) {
    std::string strLine;
    getline(is,strLine);
	
	int nPos = strLine.find('=');
	
	if ((signed int)std::string::npos == nPos) continue; // no '=', invalid line;
	std::string strKey = strLine.substr(0,nPos);
	std::string strVal = strLine.substr(nPos + 1, strLine.length() - nPos + 1);
	
	// check if this is the right key, if the right one stop the search
	if(0==correction_type.compare(strKey)) {value=strVal; break;}
  }
  
  return value;

}
