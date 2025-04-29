/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODBTaggingEfficiency/BTaggingEfficiencyTool.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingUtilities.h"
#include "CalibrationDataInterface/CalibrationDataInterfaceROOT.h"
#include "CalibrationDataInterface/CalibrationDataVariables.h"
#include "CalibrationDataInterface/CalibrationDataContainer.h"
#include "CalibrationDataInterface/CDIReader.h"

#include "xAODBTaggingEfficiency/ToolDefaults.h"

// for the onnxtool
#include "xAODBTaggingEfficiency/SaltModel.h"

#include "PATInterfaces/SystematicRegistry.h"
#include "PathResolver/PathResolver.h"

#include "TSpline.h"

#include <algorithm>

using CP::CorrectionCode;
using CP::SystematicSet;
using CP::SystematicVariation;
using CP::SystematicRegistry;

using Analysis::Uncertainty;
using Analysis::CalibrationDataVariables;
using Analysis::CalibResult;
using Analysis::CalibrationStatus;
using Analysis::Total;
using Analysis::SFEigen;
using Analysis::SFGlobalEigen;
using Analysis::SFNamed;
using Analysis::None;
using Analysis::Extrapolation;
using Analysis::TauExtrapolation;

using xAOD::IParticle;

// The following essentially  duplicates code that already exists in package ParticleJetTools (in JetFlavourInfo.cxx).
// This duplication isn't desirable, but the alternative (to use ParticleJetTools directly) pulls in a lot of dependencies.

namespace {
  // helper methods
  std::string default_flexible(int flavsize, const std::string& suffix = ":default"){
    std::string setting;
    for(int i = 0 ; i < flavsize ; i++){
      setting += std::to_string(i);
      setting += suffix;
      if(i != flavsize - 1){
        setting += ";";
      }
    }
    return setting;
  }

  int GAFinalHadronFlavourLabel (const xAOD::Jet& jet) {

    const std::string labelB = "GhostBHadronsFinal";
    const std::string labelC = "GhostCHadronsFinal";
    const std::string labelTau = "GhostTausFinal";

    std::vector<const IParticle*> ghostB;
    if (jet.getAssociatedObjects<IParticle>(labelB, ghostB) && ghostB.size() > 0) return 5;
    std::vector<const IParticle*> ghostC;
    if (jet.getAssociatedObjects<IParticle>(labelC, ghostC) && ghostC.size() > 0) return 4;
    std::vector<const IParticle*> ghostTau;
    if (jet.getAssociatedObjects<IParticle>(labelTau, ghostTau) && ghostTau.size() > 0) return 15;
    return 0;
  }

  int ExclusiveConeHadronFlavourLabel (const xAOD::Jet& jet, bool doExtended = false) {
    // default label means "invalid"
    int label = -1;

    // We don't check the return value, as we would not be able to handle it gracefully anyway
    if (doExtended) {
      jet.getAttribute("HadronConeExclExtendedTruthLabelID",label);
    } else {
      jet.getAttribute("HadronConeExclTruthLabelID",label);
    }
    return label;
  }

  int jetFlavourLabel (const xAOD::Jet& jet, bool doConeLabelling, bool doExtended) {
    if (doConeLabelling){
      return ExclusiveConeHadronFlavourLabel(jet, doExtended);
    } else {
      return GAFinalHadronFlavourLabel(jet);
    }
  }


  // local utility function: trim leading and trailing whitespace in the property strings
  std::string trim(const std::string& str,
		   const std::string& whitespace = " \t") {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
  }

  // local utility function: split string into a vector of substrings separated by a specified separator
  std::vector<std::string> split(const std::string& str, char token = ';') {
    std::vector<std::string> result;
    if (str.size() > 0) {
      std::string::size_type end;
      std::string tmp(str);
      do {
        end = tmp.find(token);
        std::string entry = trim(tmp.substr(0,end));
        if (entry.size() > 0) result.push_back(entry); 
        if (end != std::string::npos) tmp = tmp.substr(end+1);
      } while (end != std::string::npos);
    }
    return result;
  }
}

BTaggingEfficiencyTool::BTaggingEfficiencyTool( const std::string & name) : asg::AsgTool( name ), m_selectionTool("") {
  namespace def = ftag::defaults;
  declareProperty("TaggerName",                          m_taggerName=def::tagger,               "tagging algorithm name as specified in CDI file");
  declareProperty("OperatingPoint",                      m_OP=def::pcbt_op,                       "operating point as specified in CDI file");
  declareProperty("JetAuthor",                           m_jetAuthor=def::jet_collection,                "jet collection & JVF/JVT specification in CDI file");
  declareProperty("MinPt",                               m_minPt=0,                    "minimum jet pT cut");
  declareProperty("ScaleFactorFileName",                 m_SFFile=def::cdi_path,                 "name of the official scale factor calibration CDI file (uses PathResolver)");
  declareProperty("UseDevelopmentFile",                  m_useDevFile = false,          "specify whether or not to use the (PathResolver) area for temporary scale factor calibration CDI files");
  declareProperty("EfficiencyFileName",                  m_EffFile = "",                "name of optional user-provided MC efficiency CDI file");
  declareProperty("EfficiencyConfig",                    m_EffConfigFile = "",           "name of config file specifying which efficiency map to use with a given samples DSID");
  declareProperty("ScaleFactorBCalibration",             m_SFNames["B"] = "default",    "name of b-jet scale factor calibration object");
  declareProperty("ScaleFactorCCalibration",             m_SFNames["C"] = "default",    "name of c-jet scale factor calibration object");
  declareProperty("ScaleFactorTCalibration",             m_SFNames["T"] = "default",    "name of tau-jet scale factor calibration object");
  declareProperty("ScaleFactorLightCalibration",         m_SFNames["Light"] = "default","name of light-flavour jet scale factor calibration object");
  declareProperty("EigenvectorReductionB",               m_EVReduction["B"] = "Loose",  "b-jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'");
  declareProperty("EigenvectorReductionC",               m_EVReduction["C"] = "Loose",  "c-jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'");
  declareProperty("EigenvectorReductionLight",           m_EVReduction["Light"] = "Loose", "light-flavour jet scale factor Eigenvector reduction strategy; choose between 'Loose', 'Medium', 'Tight'");
  declareProperty("EfficiencyCalibrations",             m_effName = "default",   "default for all flavors");
  declareProperty("EfficiencyBCalibrations",             m_EffNames["B"] = "",   "(semicolon-separated) name(s) of b-jet efficiency object(s)");
  declareProperty("EfficiencyCCalibrations",             m_EffNames["C"] = "",   "(semicolon-separated) name(s) of c-jet efficiency object(s)");
  declareProperty("EfficiencyTCalibrations",             m_EffNames["T"] = "",   "(semicolon-separated) name(s) of tau-jet efficiency object(s)");
  declareProperty("EfficiencyLightCalibrations",         m_EffNames["Light"] = "", "(semicolon-separated) name(s) of light-flavour-jet efficiency object(s)");
  declareProperty("UncertaintyBSuffix",                  m_uncertaintySuffixes["B"] = "","optional suffix for b-jet uncertainty naming");
  declareProperty("UncertaintyCSuffix",                  m_uncertaintySuffixes["C"] = "","optional suffix for c-jet uncertainty naming");
  declareProperty("UncertaintyTSuffix",                  m_uncertaintySuffixes["T"] = "","optional suffix for tau-jet uncertainty naming");
  declareProperty("UncertaintyLightSuffix",              m_uncertaintySuffixes["Light"] = "","optional suffix for light-flavour-jet uncertainty naming");
  declareProperty("ExcludeFromEigenVectorTreatment",     m_excludeFromEV = "",          "(semicolon-separated) names of uncertainties to be excluded from all eigenvector decompositions (if used)");
  declareProperty("ExcludeFromEigenVectorBTreatment",    m_excludeFlvFromEV["B"] = "",  "(semicolon-separated) names of uncertainties to be excluded from b-jet eigenvector decomposition (if used)");
  declareProperty("ExcludeFromEigenVectorCTreatment",    m_excludeFlvFromEV["C"] = "",  "(semicolon-separated) names of uncertainties to be excluded from c-jet eigenvector decomposition (if used)");
  declareProperty("ExcludeFromEigenVectorLightTreatment",m_excludeFlvFromEV["Light"] = "", "(semicolon-separated) names of uncertainties to be excluded from light-flavour-jet eigenvector decomposition (if used)");
  declareProperty("ExcludeRecommendedFromEigenVectorTreatment", m_useRecommendedEVExclusions = false, "specify whether or not to add recommended lists to the user specified eigenvector decomposition exclusion lists");
  // declareProperty("ExcludeJESFromEVTreatment",        m_excludeJESFromEV = true,     "specify whether or not to exclude JES uncertainties from eigenvector decomposition (if used)");
  declareProperty("SystematicsStrategy",                 m_systStrategy=def::strategy,    "name of systematics model; presently choose between 'SFEigen' and 'Envelope'");
  declareProperty("ConeFlavourLabel",                    m_coneFlavourLabel = true,     "specify whether or not to use the cone-based flavour labelling instead of the default ghost association based labelling");
  declareProperty("ExtendedFlavourLabel",                m_extFlavourLabel = false,     "specify whether or not to use an 'extended' flavour labelling (allowing for multiple HF hadrons or perhaps partons)");
  declareProperty("IgnoreOutOfValidityRange",            m_ignoreOutOfValidityRange = false, "ignore out-of-extrapolation-range errors as returned by the underlying tool");
  declareProperty( "useCTagging",                        m_useCTag=false,       "Enabled only for FixedCut or Continuous WPs: define wether the cuts refer to b-tagging or c-tagging");
  declareProperty( "readFromBTaggingObject",             m_readFromBTaggingObject=true,       "Enabled to access btagging scores from xAOD::BTagging object; Can be disabled for GN2v01 to access the scores from the jet itself.");
  // if it is empty, the onnx tool won't be initialised
  declareProperty( "pathToONNX",                         m_pathToONNX = "",             "path to the onnx file that will be used for inference");
  // experimental options
  declareProperty("useFlexibleConfig",                   m_useFlex = false,                "Setup the flexible configuration of the xAODBTaggingEfficiencyTool with alternate labeling");
  declareProperty("FlexibleScaleFactorCalibrations",     m_SFName_flex = "",          "(semicolon-separated) name of scale factor calibration object for (0,1,2..) indexed flavour labels, e.g. '0:default;1:default;2:default;3:default' ");
  declareProperty("FlexibleEfficiencyCalibrations",      m_EffNames_flex = "",          "(semicolon-separated) name(s) of efficiency object(s) names for (0,1,2..) indexed flavour labels, e.g. '0:default;1:default;2:default;3:default' ");
  declareProperty("FlexibleEigenvectorReduction",        m_EVReduction_flex = "",         "(semicolon-separated) list of eigenvector reduction strategy for (0,1,2..) indexed flavour labels; choose between 'Loose', 'Medium', 'Tight' for different labels, e.g. '0:Loose;1:Loose;2:Loose' ");
  declareProperty("FlexibleUncertaintySuffix",           m_uncertaintySuffixes_flex = "",   "optional (semicolon-separated) list of suffixes for (0,1,2..) indexed flavour label uncertainty naming, e.g. '0:;1:;2:;3:' ");
  declareProperty("FlexibleExcludeFromEVTreatment",      m_excludeFlvFromEV_flex = "",        "(semicolon-separated) names of uncertainties to be excluded from (0,1,2..) indexed flavour eigenvector decompositions (if used), e.g. '0:;1:;2:;3:' ");
  
  // initialise some variables needed for caching
  // TODO : add configuration of the mapIndices - rather than just using the default of 0
  //m_mapIndices["Light"] = m_mapIndices["T"] = m_mapIndices["C"] = m_mapIndices["B"] = 0;
  m_initialised = false;
  m_applySyst      = false;
  m_isContinuous   = false;
  m_isContinuous2D = false;
  m_using_conventional_labels = false;
  // declare the selection tool to be private (not absolutely sure this is needed?)
  m_selectionTool.declarePropertyFor(this, "BTaggingSelectionTool", "selection tool to be used internally");
}

BTaggingEfficiencyTool::~BTaggingEfficiencyTool() {
  //delete m_CDI;
}

StatusCode BTaggingEfficiencyTool::initialize() {


  ATH_MSG_INFO( " Hello BTaggingEfficiencyTool user... initializing");
  ATH_MSG_INFO( " TaggerName = " << m_taggerName);
  ATH_MSG_INFO( " OP = " << m_OP);
  ATH_MSG_INFO( " m_systStrategy is " << m_systStrategy);

  // set default MCMC map if they haven't been specified
  for (auto& [k, v]: m_EffNames) {
    if (v.size() == 0) v = m_effName;
  }

  // Use the PathResolver to find the full pathname (behind the scenes this can also be used to download the file),
  // if the file cannot be found directly.
  // For consistency with the PathResolver code, use the Boost library to check on this first possibility.
  m_SFFile = trim(m_SFFile);
  std::string location = PathResolverFindCalibFile(m_SFFile);
  if (location == "") {
    std::string prepend = "";
    if (m_useDevFile) {
      ATH_MSG_WARNING(" Attempting to retrieve b-tagging scale factor calibration file from development area");
      prepend = "dev/";
    }
    prepend += "xAODBTaggingEfficiency/";
    m_SFFile = prepend + m_SFFile;
    m_SFFileFull = PathResolverFindCalibFile(m_SFFile);
    if (m_SFFileFull == "")
      ATH_MSG_WARNING(" Unable to retrieve b-tagging scale factor calibration file!");
    else
      ATH_MSG_DEBUG(" Retrieving b-tagging scale factor calibration file as " << m_SFFile);
  } else {
    m_SFFileFull = location;
  }

  // The situation for the efficiency file is a bit simpler since it need not reside under "xAODBTaggingEfficiency"
  m_EffFile = trim(m_EffFile);
  if (m_EffFile != "") m_EffFile = PathResolverFindCalibFile(m_EffFile);


  // Strategies for eigenvector reductions (only relevant if eigenvector variations are used, of course).
  // For now, we will assume that the strategy for tau "jets" is identical to that for c jets.
  std::map<std::string, Analysis::EVReductionStrategy> EVRedStrategies, mappings;
  mappings["Loose"] = Analysis::Loose;
  mappings["Medium"] = Analysis::Medium;
  mappings["Tight"] = Analysis::Tight;


  // Now that we know the CDI file location, let's check if the configuration provided is correct
  Analysis::CDIReader Reader(m_SFFileFull);
  if(!Reader.checkConfig(m_taggerName, m_jetAuthor, m_OP, msgLvl(MSG::DEBUG))){
    ATH_MSG_ERROR( "BTaggingEfficiencyTool configuration is invalid - follow the above suggestions to correct your config!");
    return StatusCode::FAILURE;
  };
  if (msgLvl(MSG::INFO)) {
    ATH_MSG_INFO( " --- Calibration file configuration options ---" );
    Reader.printTaggers();
    Reader.printJetCollections();
    Reader.printWorkingPoints();
  }

  std::vector<std::string> config_labels = Reader.getLabels(); // the labels compatible with this configuration
  std::vector<std::string> flavours;
  // "pack" the efficiency map names for each flavour. Note that multiple, semicolon separated, entries may exist; so this needs to be decoded first
  std::map<std::string, std::vector<std::string> > EffNames;
  std::vector<std::string> EVflavours = { "B", "C", "Light" }; // ideally this would also come from the metadata of the CDI file as well...
  // specify which systematic variations are to be excluded from the eigenvector decomposition
  std::map<std::string, std::vector<std::string> > excludeFromEVCov;
  std::vector<std::string> to_exclude = split(m_excludeFromEV); // uncertainties to exclude from all flavours

  if(!m_useFlex){

    //if a configuration file was provided for efficiency maps, overwrite the efficiency map selection with the one provided in the first line of the config
    if(not m_EffConfigFile.empty()){
      m_EffConfigFile = PathResolverFindCalibFile(m_EffConfigFile);
      std::ifstream eff_config_file(m_EffConfigFile);

      std::string str;
      std::getline(eff_config_file, str);

      m_EffNames["B"] = str;
      m_EffNames["C"] = str;
      m_EffNames["T"] = str;
      m_EffNames["Light"] = str;

      while (std::getline(eff_config_file, str)) {
          std::vector<std::string> dsid_and_effmap = split(str,':');
          unsigned int dsid = std::stoul( dsid_and_effmap[0] );
          unsigned int map_index = std::stoul( dsid_and_effmap[1] );
          m_DSID_to_MapIndex[dsid] = map_index;
      }
    }



    flavours = { "B", "C", "Light", "T"};

    for (auto const& flavour : flavours) {
      EffNames[flavour] = split(m_EffNames[flavour]);
    }

    ATH_MSG_INFO( " b-jet     SF/eff calibration = " << m_SFNames["B"] <<     " / " << m_EffNames["B"]);
    ATH_MSG_INFO( " c-jet     SF/eff calibration = " << m_SFNames["C"] <<     " / " << m_EffNames["C"]);
    ATH_MSG_INFO( " tau-jet   SF/eff calibration = " << m_SFNames["T"] <<     " / " << m_EffNames["T"]);
    ATH_MSG_INFO( " light-jet SF/eff calibration = " << m_SFNames["Light"] << " / " << m_EffNames["Light"]);
    ATH_MSG_INFO( " JetAuthor = " << m_jetAuthor);


  } else {

    // Set defaults for the following, based on flavour label
    // metadata if any of the following properties are not set:
    //  - FlexibleScaleFactorCalibrations
    //  - FlexibleEfficiencyCalibrations
    //  - FlexibleEigenvectorReduction
    //  - FlexibleUncertaintySuffix
    //  - FlexibleExcludeFromEVTreatment
    
    flavours = config_labels; // set the flavours vector from CDI metadata
    m_flex_labels = flavours; // set the label scheme, for use elsewhere
    for(const std::string& label : m_flex_labels){
      m_flex_label_integers.push_back(getFlavourID(label, false)); // populate the ordered flavour ID vector
    }
    int size_of_labels = flavours.size();
    if (m_SFName_flex.empty()){
      m_SFName_flex = default_flexible(flavours.size(), ":default");
    } else {ATH_MSG_INFO(" Setting m_SFName_flex : " << m_SFName_flex);}
    if (m_EffNames_flex.empty()){
      m_EffNames_flex = default_flexible(flavours.size(), ":default");
    } else {ATH_MSG_INFO(" Setting m_EffNames_flex : " << m_EffNames_flex);}
    if (m_EVReduction_flex.empty()){
      m_EVReduction_flex = default_flexible(flavours.size(), ":Loose");
    } else {ATH_MSG_INFO(" Setting m_EVReduction_flex : " << m_EVReduction_flex);}
    if (m_uncertaintySuffixes_flex.empty()){
      m_uncertaintySuffixes_flex = default_flexible(flavours.size(), ":");
    } else {ATH_MSG_INFO(" Setting m_uncertaintySuffixes_flex : " << m_uncertaintySuffixes_flex);}
    if (m_excludeFlvFromEV_flex.empty()){
      m_excludeFlvFromEV_flex = default_flexible(flavours.size(), ":");
    } else {ATH_MSG_INFO(" Setting m_excludeFlvFromEV_flex : " << m_excludeFlvFromEV_flex);}

    // empty m_SFNames (filled by the 'default' setting), in the flexible method we want to use the labels found in the CDI file meta-data
    m_SFNames.clear();
    std::vector<std::string> efficiencies_map = split(m_EffNames_flex); // split to {"0:default", "1:47100,47200,default", "2:default", "3:default"}
    for (auto const& effconfig : efficiencies_map) {
      std::vector<std::string> effmap = split(effconfig, ':'); // split to {"0", "default"}
      if(effmap.size() != 2){ // check if first flavour label config is written properly // || std::stoi(effmap.at(0)) > (int)flavours.size()
        ATH_MSG_ERROR( "BTaggingEfficiencyTool configuration is invalid - efficiency files not included properly in the (experimental) flexible configuration!");
        return StatusCode::FAILURE;
      }
      int index = std::stoi(effmap.at(0)); // index of flavour label in 'flavours' vector
      std::vector<std::string> effcalibs; // vector of efficiency calibrations
      if(effmap.at(1) != "default"){
        effcalibs = split(effmap.at(1), ',');
      } else {
        effcalibs.push_back(effmap.at(1));
      }
      std::string flavour_name = flavours.at(index);
      m_EffNames[flavour_name] = effmap.at(1); // set this for posterity
      EffNames[flavour_name] = effcalibs;
    }

    std::vector<std::string> scalefactor_map = split(m_SFName_flex); // split to {"0:default", "1:47100,47200,default", "2:default", "3:default"}
    for (auto const& sfconfig : scalefactor_map) {
      std::vector<std::string> sfmap = split(sfconfig, ':'); // split to {"0", "default"}
      if(sfmap.size() != 2){ // check if first flavour label config is written properly // || std::stoi(sfmap.at(0)) > (int)flavours.size()
        ATH_MSG_ERROR( "BTaggingEfficiencyTool configuration is invalid - efficiency files not included properly in the (experimental) flexible configuration!");
        return StatusCode::FAILURE;
      }
      int index = std::stoi(sfmap.at(0)); // index of flavour label in 'flavours' vector
      std::string flavour_name = flavours.at(index);
      m_SFNames[flavour_name] = sfmap.at(1);
    }
    for(const std::string& flav : flavours){
      ATH_MSG_INFO( flav + " label     SF/eff calibration = " + m_SFNames[flav] +  " / " + m_EffNames[flav] ) ;
    }

    // Now deal with the flexible configurations
    EVflavours.clear(); // ideally this vector would also come from CDI file metadata
    EVflavours = flavours; // for now, just set the vector of EV flavours to the flavours vector
    // construct the flavour m_EVReduction map from m_EVReduction_flex index based configuration
    std::vector<std::string> evreduction_map = split(m_EVReduction_flex);
    for (auto const& evred : evreduction_map) {
      std::vector<std::string> evredmap = split(evred, ':'); // split to {"0", "Loose"} 
      if(evredmap.size() != 2){
        if(std::stoi(evredmap.at(0)) < size_of_labels){
          evredmap.push_back("");
        } else {
          ATH_MSG_ERROR( "BTaggingEfficiencyTool configuration is invalid - reduction strategies not set properly in the (experimental) flexible configuration!");
          return StatusCode::FAILURE;
        }
      }
      int index = std::stoi(evredmap.at(0)); // index of flavour label in 'flavours' vector
      const std::string& flavour_name = flavours.at(index);
      m_EVReduction[flavour_name] = evredmap.at(1);
    }
    
    // construct the flavour m_excludeFlvFromEV map from m_excludeFlvFromEV_flex index based configuration
    std::vector<std::string> exclusion_map = split(m_excludeFlvFromEV_flex);
    for (auto const& excl : exclusion_map) {
      std::vector<std::string> exclmap = split(excl, ':'); // split to {"0", "uncertainty1,uncertainty2,..,uncertaintyN"}
      if(exclmap.size() != 2){
        if(std::stoi(exclmap.at(0)) < size_of_labels){
          exclmap.push_back("");
        } else {
          ATH_MSG_ERROR( "BTaggingEfficiencyTool configuration is invalid - uncertainty exclusion not set properly in the (experimental) flexible configuration!");
          return StatusCode::FAILURE;
        }
      }
      int index = std::stoi(exclmap.at(0)); // index of flavour label in 'flavours' vector
      std::vector<std::string> uncertainties_to_exclude = split(exclmap.at(1), ',');; // vector of uncertainties to exclude for this flavour
      const std::string& flavour_name = flavours.at(index);
      m_excludeFlvFromEV[flavour_name] = exclmap.at(1);
    }
  }

  // set the reduction per EV flavour/label, and exclude uncertainties from EV reduction
  for (auto const& flavour : EVflavours) {
    EVRedStrategies[flavour] = mappings.find(trim(m_EVReduction[flavour])) == mappings.end() ? mappings["Loose"] : mappings[trim(m_EVReduction[flavour])];
    excludeFromEVCov[flavour] = to_exclude; // look for uncertainties to be excluded for all flavours
    std::vector<std::string> further_exclude = split(m_excludeFlvFromEV[flavour]); // now grab the flavour/label specific uncertainties to exclude
    excludeFromEVCov[flavour].insert(excludeFromEVCov[flavour].end(), further_exclude.begin(), further_exclude.end()); // Append to the existing list
  }

  // now, handle the edge-cases of EV reduction and extra uncertainties
  // e.g. C - T calibration duplication in "conventional" flavour labeling
  // check if the labels are "conventional"
  std::vector<std::string> conventional_labels = {"B", "C", "Light", "T"};
  if (flavours == conventional_labels) {
    m_using_conventional_labels = true;
  }
  if (m_using_conventional_labels) {
    // if using conventional label, we can use the same logic as always
    EVRedStrategies["T"] = EVRedStrategies["C"];

    // For the SFEigen strategy, tau "jets" are treated differently from other flavours. 
    // First, copy the charm-jet calibration settings
    excludeFromEVCov["T"] = excludeFromEVCov["C"];

    //high pt extrapolation uncertainties
    if(m_OP.find("Continuous") != std::string::npos){
      excludeFromEVCov["B"].push_back("extrapolation_pt_b_Eigen*");
      excludeFromEVCov["C"].push_back("extrapolation_pt_c_Eigen*");
      excludeFromEVCov["Light"].push_back("extrapolation_pt_l_Eigen*");
      excludeFromEVCov["T"].push_back("extrapolation_pt_c_Eigen*");
    }
  } 

  if (m_OP == "Continuous") {
    // continuous tagging is special in two respects:
    // 1  the tag weight needs to be retrieved
    // 2  the generator dependent scale factor rescaling is done differently, and therefore
    //    CalibrationDataInterfaceROOT::getWeightScaleFactor() instead of
    //    CalibrationDataInterfaceROOT::getScaleFactor() must be used
    m_isContinuous = true;
  }
  else if  (m_OP.find("Continuous2D")  != std::string::npos) {
    m_isContinuous2D = true;
  }
  // Note that the instantiation below does not leave a choice: the Eigenvector variations and generator-specific scale factors are always used
  std::vector<std::string> jetAliases;
  m_CDI = std::shared_ptr<Analysis::CalibrationDataInterfaceROOT>( new Analysis::CalibrationDataInterfaceROOT(
                 m_taggerName,                              // tagger name: always needed
						     m_SFFileFull.c_str(),                          // full pathname of the SF calibration file: always needed
						     (m_EffFile == "") ? 0 : m_EffFile.c_str(), // full pathname of optional efficiency file
						     jetAliases,                                // since we configure the jet "collection name" by hand, we don't need this
						     m_SFNames,                                 // names of the scale factor calibrations to be used
						     EffNames,                                  // names of the efficiency calibrations to be used (can be multiple per flavour)
						     excludeFromEVCov,                          // names of systematic uncertainties to be excluded from the EV decomposition
						     EVRedStrategies,                           // strategies for eigenvector reductions
						     m_systStrategy != "Envelope",              // assume that eigenvector variations will be used unless the "Envelope" model is used
                 (m_systStrategy=="SFEigen") ? Analysis::SFEigen : Analysis::Uncertainty::SFGlobalEigen,
						     true,                                      // use MC/MC scale factors
						     false,                                     // do not use topology rescaling (only relevant for pseudo-continuous tagging)
						     m_useRecommendedEVExclusions,              // if true, add pre-set lists of uncertainties to be excluded from EV decomposition
                 msgLvl(MSG::DEBUG),                         // if false, suppress any non-error/warning messages
                 flavours                                   // vector of flavour labels, conventional or not
                ));                         

  if(m_using_conventional_labels){
    ATH_MSG_INFO("BTEffTool->initialize : setMapIndex(Light)");
    setMapIndex("Light",0);
    ATH_MSG_INFO("BTEffTool->initialize : setMapIndex(C)");
    setMapIndex("C",0);
    ATH_MSG_INFO("BTEffTool->initialize : setMapIndex(B)");
    setMapIndex("B",0);
    ATH_MSG_INFO("BTEffTool->initialize : setMapIndex(T)");
    setMapIndex("T",0);
  } else {
    for(const auto& flavour : flavours){
      // set map index for each flavour (label) index
      ATH_MSG_INFO(" - - -BTEffTool->initialize : setMapIndex(" << flavour << ")");
      setMapIndex(flavour, 0); 
    }
  }

  ATH_MSG_INFO( "Using systematics model " << m_systStrategy);
  if (m_systStrategy != "Envelope" && m_useRecommendedEVExclusions) ATH_MSG_INFO( "excluding pre-set uncertainties from eigenvector decomposition");

  // We have a double loop over flavours here.. not nice but this is to ensure that the suffixes are always well determined before using them.
  // Note that (in the SFEigen model) suffixes are attached to eigenvector variations only (the idea being that named uncertainties, if used, are likely to be correlated).
  std::vector<std::string> suffixes;
  if (m_using_conventional_labels){
    for (int i = 0; i < 4; ++i) { // four flavours in conventional label scheme
      std::string flav = flavours[i]; if (flav == "T") flav = "C";
      // add an underscore to any specified suffix (if specified and if not already starting with a suffix)
      std::string test = trim(m_uncertaintySuffixes[flav]);
      if (test.length() > 0 && test[0] != '_') test.insert(0,"_");
      suffixes.push_back(test);
    }
  } else {
    // Need to handle the suffixes for the generic labels
    std::vector<std::string> suffixvec = split(m_uncertaintySuffixes_flex);
    int size_of_labels = flavours.size();
    for (auto const& suff : suffixvec) {
      std::vector<std::string> suffmap = split(suff, ':'); // split to {"0", "uncertainty1,uncertainty2,..,uncertaintyN"}
      if(suffmap.size() != 2){
        if(std::stoi(suffmap.at(0)) < size_of_labels){
          suffmap.push_back("");
        } else {
          ATH_MSG_ERROR( "BTaggingEfficiencyTool configuration is invalid - suffix configuration not set properly in the (experimental) flexible configuration!");
          return StatusCode::FAILURE;
        }
      }
      int index = std::stoi(suffmap.at(0)); // index of flavour label in 'flavours' vector
      std::string flavour_name = flavours.at(index);
      // add an underscore to any specified suffix (if specified and if not already starting with a suffix)
      std::string suffix = trim(suffmap.at(1));
      if (suffix.length() > 0 && suffix[0] != '_') suffix.insert(0,"_");
      suffixes.push_back(suffix);
    }
  }

  // If the tool has not already been initialised and m_OP and m_jetAuthor have been set - ie via the properties "OperatingPoint" and "JetAuthor"
  // then autmatically set things up to use these by default
  // All this must happen before registerSystematics otherwise that won't work
  for (long unsigned int i = 0; i < flavours.size(); ++i) {
    // For each flavour, we check the validity of the initialization of the CalibrationInterfaceROOT object (m_CDI)
    unsigned int flavourID;
    if (m_using_conventional_labels){
      flavourID = getFlavourID(flavours[i]);
    } else {
      flavourID = getFlavourID(flavours[i], false); // in flexible scheme, need to specify the new labels in getFlavourID
    }
    // std::map<unsigned int, unsigned int>::const_iterator
    auto mapIter = m_SFIndices.find(flavourID);
    if( mapIter==m_SFIndices.end()) { // if the flavour doesn't have an entry need to fail the initialization
      ATH_MSG_ERROR( "No entry for flavour " << flavourID << " which is " << flavours[i] << " in SFIndices map, invalid initialization");
      return StatusCode::FAILURE;
    }
    int id = mapIter->second;
    // Implement the different strategies for dealing with uncertainties here.
    if (m_systStrategy == "SFEigen" || m_systStrategy == "SFGlobalEigen") {
      //
      // Generally recommended model: use eigenvector variations. Notes:
      // -   The list of systematics to be excluded from the eigenvector variation approach is dynamic.
      // -   The tau SF are identical to the c-jet ones, with merely one additional uncertainty assigned due to the extrapolation.
      //
      unsigned int flavourIDRef;
      if (m_using_conventional_labels){
        flavourIDRef = (flavourID == 15) ? 4 : flavourID;
      } else {
        flavourIDRef = flavourID;
      }
      int idRef = m_SFIndices.find(flavourIDRef)->second;
      // First, handle any named variations
      std::vector<std::string> systematics = m_CDI->listScaleFactorUncertainties(idRef, flavours.at(i), true); // flavours[i] should be "B", "C", "Light", or "T"
      // Replace any spaces with underscores (this is to make ROOT browsing happy).
      // Also, remove the "extrapolation" uncertainty from the list (it will be added later under Extrapolation rather than SFNamed).

      auto it = std::remove (systematics.begin(), systematics.end(),
                             "extrapolation");
      bool hasExtrapolation = (it != systematics.end());
      systematics.erase (it, systematics.end());
      // C++20 alternative:
      //bool hasExtrapolation = std::erase (systematics, "extrapolation") > 0;
      
      for (auto& systematic : systematics) {
        std::replace_if(systematic.begin(), systematic.end(), [] (char c) { return c == ' '; }, '_'); // <--- This just replaces spaces with underscores
        // We don't add suffixes here but only for EV variations (see JIRA: AFT-343)
      }
      
      if (!addSystematics(systematics, flavourID, SFNamed)) { // Add the SFNamed to m_systematicsInfo
        ATH_MSG_ERROR("SFEigen model: error adding named systematics for flavour " << getLabel(flavourIDRef) << ", invalid initialization"); // For each uncertainty type, have to add ALL uncertainties pertaining to that type
        return StatusCode::FAILURE;
      }

      // Add here the extrapolation uncertainty (if it exists -- which ought to be the case).
      // "Cosmetic" fix: the outside world wants to see "FT_EFF_" prefixes. On the other hand, like for the above named uncertainties, we don't add suffixes here
      if (hasExtrapolation) {
        std::vector<std::string> extrapSyst; extrapSyst.push_back(std::string("FT_EFF_extrapolation")); // Add the EXTRAPOLATION to m_systematicsInfo
        if (! addSystematics(extrapSyst, flavourID, Extrapolation)) {
          ATH_MSG_ERROR("SFEigen model: error adding extrapolation uncertainty for flavour " << getLabel(flavourIDRef) << ", invalid initialization");
          return StatusCode::FAILURE;
        }
      }

      // And then the eigenvector variations
      std::vector<std::string> eigenSysts;
      if (m_systStrategy == "SFEigen"){
        if (m_using_conventional_labels){
          eigenSysts = makeEigenSyst(getLabel(flavourIDRef),m_CDI->getNumVariations(idRef, SFEigen, getLabel(flavourID)), suffixes[i]); // Add the eigenvariations to m_systematicsInfo
        } else {
          eigenSysts = makeEigenSyst(getLabel(flavourIDRef),m_CDI->getNumVariations(idRef, SFEigen, flavours[i]), suffixes[i]); // Add the eigenvariations to m_systematicsInfo
        }
        if (!addSystematics(eigenSysts, flavourID, SFEigen)) {
          ATH_MSG_ERROR("SFEigen model: error adding eigenvector systematics for flavour " << getLabel(flavourIDRef) << ", invalid initialization");
          return StatusCode::FAILURE;
        }
      } else if (m_systStrategy == "SFGlobalEigen") {
        ////////////////////////////////////////////////

        // we have to add the SFGlobalEigen systematics like this...
        if (m_using_conventional_labels){
          eigenSysts = makeEigenSyst(getLabel(flavourIDRef),m_CDI->getNumVariations(idRef, SFGlobalEigen, getLabel(flavourID)), suffixes[i]); // Add the eigenvariations to m_systematicsInfo
        } else {
          std::vector<std::string> eigenSysts = makeEigenSyst(getLabel(flavourIDRef),m_CDI->getNumVariations(idRef, SFGlobalEigen, flavours[i]), suffixes[i]); // Add the eigenvariations to m_systematicsInfo
        }
        if (!addSystematics(eigenSysts, flavourID, SFGlobalEigen)) {
          ATH_MSG_ERROR("SFGlobalEigen model: error adding eigenvector systematics for flavour " << getLabel(flavourIDRef) << " ie " << getLabel(flavourID) << ", invalid initialization");
          return StatusCode::FAILURE;
        }

      ////////////////////////////////////////////////
      }
      // The above should cover all uncertainties except the charm -> tau extrapolation; so we take care of that here.
      if (flavourID == 15) {
        // First extract the complete list of uncertainties for taus
        std::vector<std::string> all_systematics = m_CDI->listScaleFactorUncertainties(id, getLabel(flavourID));
        // And from this list extract only this particular uncertainty (if it exists)
        const std::string s_tau_extrap = "extrapolation from charm";
        if (std::find(all_systematics.begin(), all_systematics.end(), s_tau_extrap) != all_systematics.end()) {
          // Again, we don't add the suffix here (per JIRA: AFT-343)
          std::string entry = "FT_EFF_extrapolation_from_charm"; // entry.append(suffixes[i]);
          std::vector<std::string> extrapSyst; extrapSyst.push_back(entry);
          if (! addSystematics(extrapSyst, flavourID, TauExtrapolation)) {     // <--- Add the TauExtrapolation to m_systematicsInfo
            ATH_MSG_ERROR("SFEigen model: error adding charm->tau systematics for flavour " << getLabel(flavourID) << ", invalid initialization");
            return StatusCode::FAILURE;
          }
        }
      }
    } else if (m_systStrategy == "Envelope") {
      //
      // Simplified model: use uncertainty envelopes supplemented by a (common) extrapolation uncertainty
      // (since the extrapolation uncertainties aren't included in the Total uncertainty). Notes:
      // -   The tau SF are identical to the c-jet ones, with merely one additional uncertainty assigned due to the extrapolation.
      // -   The "total" uncertainty is always expected to be available; the code will bomb if this is not the case.
      //     Also, the "total" uncertainties for different flavours are assumed to be uncorrelated.
      //
      unsigned int flavourIDRef = (flavourID == 15) ? 4 : flavourID;
      int idRef = m_SFIndices.find(flavourIDRef)->second;
      // First, handle the Total variations; these need different prefixes to reflect them being uncorrelated
      std::vector<std::string> all_ref_systematics = m_CDI->listScaleFactorUncertainties(idRef,getLabel(flavourID),false);
      const std::string s_total = "systematics";
      if (std::find(all_ref_systematics.begin(), all_ref_systematics.end(), s_total) == all_ref_systematics.end()) {
        ATH_MSG_ERROR("Envelope model: required uncertainty " << s_total << " not found for flavour " << getLabel(flavourIDRef)
                << ", invalid initialization");
        return StatusCode::FAILURE;
      }
      std::vector<std::string> totalSyst; totalSyst.push_back("FT_EFF_" + getLabel(flavourIDRef) + "_" + s_total + suffixes[i]);
      if (! addSystematics(totalSyst, flavourID, Total)) {
        ATH_MSG_ERROR("Envelope model: error adding systematics uncertainty for flavour " << getLabel(flavourIDRef)
              << ", invalid initialization");
        return StatusCode::FAILURE;
      }
      // Second, handle the extrapolation variations; these are shared between flavours (unless different suffixes are specified)
      const std::string s_extrap = "extrapolation";
      if (std::find(all_ref_systematics.begin(), all_ref_systematics.end(), s_extrap) != all_ref_systematics.end()) {
        std::vector<std::string> extrapSyst; extrapSyst.push_back("FT_EFF_" + s_extrap + suffixes[i]);
        if (! addSystematics(extrapSyst, flavourID, Extrapolation)) {
          ATH_MSG_ERROR("Envelope model: error adding extrapolation uncertainty for flavour " << getLabel(flavourIDRef)
                << ", invalid initialization");
          return StatusCode::FAILURE;
        }
      }
      // Finally, handle the charm -> tau extrapolation (as in the above)
      if (flavourID == 15) {
        // First extract the complete list of uncertainties for taus
        std::vector<std::string> all_systematics = m_CDI->listScaleFactorUncertainties(id, getLabel(flavourID));
        // And from this list extract only this particular uncertainty (if it exists)
        const std::string s_tau_extrap = "extrapolation from charm";
        if (std::find(all_systematics.begin(), all_systematics.end(), s_tau_extrap) != all_systematics.end()) {
          std::vector<std::string> extrapSyst; extrapSyst.push_back("FT_EFF_extrapolation_from_charm" + suffixes[i]);
          if (! addSystematics(extrapSyst, flavourID, TauExtrapolation)) {
            ATH_MSG_ERROR("Envelope model: error adding charm->tau systematics for flavour " << getLabel(flavourID) << ", invalid initialization");
            return StatusCode::FAILURE;
          }
        }
      }
    }
  } // end flavour loop

  // now fill the SystematicSet
  for( std::map<SystematicVariation,SystInfo>::const_iterator mapIter = m_systematicsInfo.begin(); mapIter != m_systematicsInfo.end();++mapIter) {
    const SystematicVariation & variation = mapIter->first;
    m_systematics.insert(variation);
  }
  // systematics framework
  SystematicRegistry & registry = SystematicRegistry::getInstance();
  if( registry.registerSystematics(*this) != StatusCode::SUCCESS) 
    return StatusCode::FAILURE;

  // Finally, also initialise the selection tool, if needed (for now this is the case only for DL1 tag weight computations,
  // so we do this only when DL1 is specified)
  if (m_taggerName.find("DL1") != std::string::npos || m_taggerName.find("GN1") != std::string::npos || m_taggerName.find("GN2") != std::string::npos) {
    m_selectionTool.setTypeAndName("BTaggingSelectionTool/" + name() + "_selection");
    ATH_CHECK( m_selectionTool.setProperty("FlvTagCutDefinitionsFileName", m_SFFile) );
    ATH_CHECK( m_selectionTool.setProperty("TaggerName",                   m_taggerName) );
    ATH_CHECK( m_selectionTool.setProperty("OperatingPoint",               m_OP) );
    ATH_CHECK( m_selectionTool.setProperty("JetAuthor",                    m_jetAuthor) );
    ATH_CHECK( m_selectionTool.setProperty("MinPt",                        m_minPt) );
    ATH_CHECK( m_selectionTool.setProperty("useCTagging",                  m_useCTag) );
    ATH_CHECK( m_selectionTool.setProperty("readFromBTaggingObject",                  m_readFromBTaggingObject) );
    ATH_CHECK( m_selectionTool.retrieve() );
 }

  // if the user decides to ignore these errors, at least make her/him aware of this
  if (m_ignoreOutOfValidityRange) {
    ATH_MSG_INFO("!!!!! You have chosen to disable out-of-validity return codes -- contact the Flavour Tagging group if such jets comprise a substantial part of the phase space in your analysis !!!!!");
  }
  
  // create and initialise the onnx tool
  if (m_pathToONNX != ""){
    std::string pathtoonnxfile = PathResolverFindCalibFile(m_pathToONNX);
    if (pathtoonnxfile == ""){
      ATH_MSG_ERROR("ONNX error: Model file doesn't exist! Please set the property 'pathToONNX' to a valid ONNX file");
      return StatusCode::FAILURE;
    }
    m_saltModel = std::make_unique<SaltModel> (m_pathToONNX);
    m_saltModel->initialize();
  }

  m_initialised = true;
  return StatusCode::SUCCESS;
}

CorrectionCode
BTaggingEfficiencyTool::getScaleFactor( const xAOD::Jet & jet, float & sf)
{
  if (! m_initialised) {
    ATH_MSG_ERROR("BTaggingEfficiencyTool has not been initialised");
    return CorrectionCode::Error;
  }
  
  // get the btag label
  int flavour{0};
  if (m_using_conventional_labels){ // if not using conventional labels, so flavour label will have to be set by some other means...
    flavour = jetFlavourLabel(jet, m_coneFlavourLabel, m_extFlavourLabel);
  } 

  Analysis::CalibrationDataVariables vars;
  //const double pt = jet.pt();
  //const double eta = jet.eta();
  //const double tagwe = 0.7; // temporary testing
  if (! fillVariables(jet, vars)) {
  //if (! fillVariables(pt, eta, tagwe, vars)){
    ATH_MSG_ERROR("unable to fill variables required for scale factor evaluation");
    return CorrectionCode::Error;
  }

  return getScaleFactor(flavour, vars, sf);
}

CorrectionCode
BTaggingEfficiencyTool::getScaleFactor( int flavour, const Analysis::CalibrationDataVariables& v,
					float & sf)
{
  if (! m_initialised) {
    ATH_MSG_ERROR("BTaggingEfficiencyTool has not been initialised");
    return CorrectionCode::Error;
  }
  
  CalibResult result;
  
  unsigned int sfindex = 0;
  unsigned int efindex = 0;
  
  if( !getIndices(flavour,sfindex,efindex)) { //<-------- This method returns true if it can find the sfindex and efindex corresponding to the flavour
    // These indices are used internally in the CalibrationDataInterfaceROOT. They represent the index where
    // the CalibrationDataContainer storing the calibration scalefactors/efficiencies are at in the CDIROOT (in m_objects in both cases, but needs a call to getMCScaleFactor in latter case)
    // Once the container is retrieved by index, the CalibrationDataEigenVariations object is retrieved by container,
    // which actually returns Up/Down variations, else return SF+SFError
    ATH_MSG_ERROR("BTaggingEfficiencyTool::getScaleFactor call to getIndices failed " << flavour << " " << sfindex << " " << efindex);
    return CorrectionCode::Error;
  }
  CalibrationStatus status;
  Uncertainty unc = None;
  unsigned int unc_ind=0; // <----- This is the index of the variation internal to the CDIROOT object
  
  if( m_applySyst) { // indicate that we want to apply a systematic variation, i.e. return an up/down variation pair
    unc = m_applyThisSyst.uncType; // type of systematic strategy...
    if(!m_applyThisSyst.getIndex(flavour,unc_ind)) {
      ATH_MSG_VERBOSE("getScaleFactor: requested variation cannot be applied to flavour " << getLabel(flavour) << ", returning nominal result");
      unc = None;
    }
  }

  // In all likelihood, the "sfindex" and "efindex" need more work in the CDIROOT (or in the CDGEV)
  status = (m_isContinuous || m_isContinuous2D) ? m_CDI->getWeightScaleFactor(v,sfindex,efindex, unc,unc_ind,result) : m_CDI->getScaleFactor(v,sfindex,efindex, unc,unc_ind,result, getLabel(flavour));

  // Interpret what has been retrieved;
  // this depends both on the uncertainty type and on the up/down setting.
  sf = result.first;
  if (m_applySyst && unc != None) {
    if (! (unc == SFEigen || unc == SFNamed || unc == SFGlobalEigen)){ 
      sf += m_applyThisSyst.isUp ? result.second : -result.second ;
    } else if (!m_applyThisSyst.isUp) {
      sf = result.second; // otherwise, set the sf to the down variation (if down is requested)
    } // otherwise, doing nothing will return the result.first value, which SHOULD represent the up variation
  }

  switch (status) {
  case Analysis::kError:
    ATH_MSG_ERROR("BTaggingEfficiencyTool::getScaleFactor call to underlying code returned a kError!");
    return CorrectionCode::Error;
  case Analysis::kRange:
  case Analysis::kExtrapolatedRange:
    return m_ignoreOutOfValidityRange ? CorrectionCode::Ok : CorrectionCode::OutOfValidityRange;
  case Analysis::kSuccess:
  default:
    return CorrectionCode::Ok;
  }
}
  
CorrectionCode
BTaggingEfficiencyTool::getEfficiency( const xAOD::Jet & jet, float & eff)
{
  if (! m_initialised) return CorrectionCode::Error;

  // get the btag label
  int flavour = jetFlavourLabel(jet, m_coneFlavourLabel, m_extFlavourLabel);

  Analysis::CalibrationDataVariables vars;

  if (! fillVariables(jet, vars)) {
    ATH_MSG_ERROR("unable to fill variables required for efficiency evaluation");
    return CorrectionCode::Error;
  }

  return getEfficiency(flavour, vars, eff);
}

CorrectionCode
BTaggingEfficiencyTool::getEfficiency( int flavour, const Analysis::CalibrationDataVariables& v,  
                float & eff)
{
  if (! m_initialised) return CorrectionCode::Error;

  CalibResult result;

  unsigned int sfindex = 0;
  unsigned int efindex = 0;

  if( !getIndices(flavour,sfindex,efindex)) {
    ATH_MSG_ERROR("BTaggingEfficiencyTool::getEfficiency call to getIndices failed " << flavour << " " << sfindex << " " << efindex);
    return CorrectionCode::Error;
  }
  Uncertainty unc = None;
  unsigned int unc_ind = 0;

  if( m_applySyst) {
    
    unc = m_applyThisSyst.uncType;
    // if( m_applyThisSyst.isNamed) {
    //   unc = SFNamed;
    // } else {
    //   unc = SFEigen;
    // }
    
    if(!m_applyThisSyst.getIndex(flavour,unc_ind)) {
      ATH_MSG_VERBOSE("getEfficiency: requested variation cannot be applied to flavour " << getLabel(flavour) << ", returning nominal result");
      unc = None;
    }
  }

  CalibrationStatus status = m_CDI->getEfficiency(v,sfindex,efindex, unc,unc_ind,result, getLabel(flavour));
  // Interpret what has been retrieved;
  // this depends both on the uncertainty type and on the up/down setting.
  eff = result.first; // central value or up variation
  if (m_applySyst && unc != None) {
    if (! (unc == SFEigen || unc == SFNamed)){
      eff += m_applyThisSyst.isUp ? result.second : -result.second ;
    } else if (!m_applyThisSyst.isUp) {
      eff = result.second; // down variation
    }
  }
  
  switch (status) {
  case Analysis::kError:
    ATH_MSG_ERROR("BTaggingEfficiencyTool::getEfficiency call to underlying code returned a kError!");
    return CorrectionCode::Error;
  case Analysis::kRange:
  case Analysis::kExtrapolatedRange:
    return m_ignoreOutOfValidityRange ? CorrectionCode::Ok : CorrectionCode::OutOfValidityRange;
  case Analysis::kSuccess:
  default:
    return CorrectionCode::Ok;
  }
}
  
CorrectionCode
BTaggingEfficiencyTool::getInefficiency( const xAOD::Jet & jet, float & eff)
{
  if (! m_initialised) return CorrectionCode::Error;

  // get the btag label
  int flavour = jetFlavourLabel(jet, m_coneFlavourLabel, m_extFlavourLabel);

  Analysis::CalibrationDataVariables vars;
  if (! fillVariables(jet, vars)) {
    ATH_MSG_ERROR("unable to fill variables required for scale factor evaluation");
    return CorrectionCode::Error;
  }

  return getInefficiency (flavour, vars, eff);
}

CorrectionCode
BTaggingEfficiencyTool::getInefficiency( int flavour, const Analysis::CalibrationDataVariables& v, 
					 float & eff)
{
  if (! m_initialised) return CorrectionCode::Error;

  CalibResult result;

  unsigned int sfindex = 0;
  unsigned int efindex = 0;

  if( !getIndices(flavour,sfindex,efindex)) {
    ATH_MSG_ERROR("BTaggingEfficiencyTool::getInefficiency call to getIndices failed " << flavour << " " << sfindex << " " << efindex);
    return CorrectionCode::Error;
  }
  Uncertainty unc = None;
  unsigned int unc_ind = 0;
  if( m_applySyst) {
    
    unc = m_applyThisSyst.uncType;
    // if( m_applyThisSyst.isNamed) {
    //   unc = SFNamed;
    // } else {
    //   unc = SFEigen;
    // }
    
    if(!m_applyThisSyst.getIndex(flavour,unc_ind)) {
      ATH_MSG_VERBOSE("getInefficiency: requested variation cannot be applied to flavour " << getLabel(flavour)
		      << ", returning nominal result");
      unc = None;
    }
  }

  CalibrationStatus status = m_CDI->getInefficiency(v, sfindex, efindex, unc, unc_ind, result, getLabel(flavour));
  // Interpret what has been retrieved;
  // this depends both on the uncertainty type and on the up/down setting.
  // For the Total uncertainty, note also the sign change compared to e.g. getEfficiency().
  eff = result.first; // central value or up variation
  if (m_applySyst && unc != None) {
    if (! (unc == SFEigen || unc == SFNamed))
      eff += m_applyThisSyst.isUp ? -result.second : result.second ;
    else if (!m_applyThisSyst.isUp) {
      eff = result.second; // down variation
    }
  }

  switch (status) {
  case Analysis::kError:
    ATH_MSG_ERROR("BTaggingEfficiencyTool::getInefficiency call to underlying code returned a kError!");
    return CorrectionCode::Error;
  case Analysis::kRange:
  case Analysis::kExtrapolatedRange:
    return m_ignoreOutOfValidityRange ? CorrectionCode::Ok : CorrectionCode::OutOfValidityRange;
  case Analysis::kSuccess:
  default:
    return CorrectionCode::Ok;
  }
}

CorrectionCode
BTaggingEfficiencyTool::getInefficiencyScaleFactor( const xAOD::Jet & jet, float & sf)
{
  if (! m_initialised) return CorrectionCode::Error;
  if (m_isContinuous || m_isContinuous2D){
      ATH_MSG_ERROR("Inefficiency SF should NOT be included for the Continous operation point");
      return CorrectionCode::Error;
  }

  // get the btag label
  int flavour = jetFlavourLabel(jet, m_coneFlavourLabel, m_extFlavourLabel);

  Analysis::CalibrationDataVariables vars;
  if (! fillVariables(jet, vars)) {
    ATH_MSG_ERROR("unable to fill variables required for scale factor evaluation");
    return CorrectionCode::Error;
  }
  
  return getInefficiencyScaleFactor( flavour, vars, sf);
}

CorrectionCode
BTaggingEfficiencyTool::getInefficiencyScaleFactor( int flavour, const Analysis::CalibrationDataVariables& v, 
						    float & sf)
{
  if (! m_initialised) return CorrectionCode::Error;
  if (m_isContinuous || m_isContinuous2D){
      ATH_MSG_ERROR("Inefficiency SF should NOT be included for the Continous operation point");
      return CorrectionCode::Error;
  }

  CalibResult result;

  unsigned int sfindex = 0;
  unsigned int efindex = 0;
  
  if( !getIndices(flavour,sfindex,efindex)) {
    ATH_MSG_ERROR("BTaggingEfficiencyTool::getInefficiencyScaleFactor call to getIndices failed " << flavour << " " << sfindex << " " << efindex);
    return CorrectionCode::Error;
  }
  Uncertainty unc=None;
  unsigned int unc_ind=0;
  if( m_applySyst) {
    
    unc = m_applyThisSyst.uncType;
    // if( m_applyThisSyst.isNamed) {
    //   unc = SFNamed;
    // } else {
    //   unc = SFEigen;
    // }
    
    if(!m_applyThisSyst.getIndex(flavour,unc_ind)) {
      ATH_MSG_VERBOSE("getInefficiencyScaleFactor: requested variation cannot be applied to flavour " << getLabel(flavour)
		      << ", returning nominal result");
      unc = None;
    }
  }

  CalibrationStatus status = m_CDI->getInefficiencyScaleFactor(v,sfindex,efindex, unc, unc_ind, result, getLabel(flavour));
  // Interpret what has been retrieved;
  // this depends both on the uncertainty type and on the up/down setting.
  // For the Total uncertainty, note also the sign change compared to e.g. getScaleFactor().
  sf = result.first; // central value or up variation
  if (m_applySyst && unc != None) {
    if (! (unc == SFEigen || unc == SFNamed))
      sf += m_applyThisSyst.isUp ? -result.second : result.second ;
    else if (!m_applyThisSyst.isUp) {
      sf = result.second; // down variation
    }
  }

  switch (status) {
  case Analysis::kError:
    ATH_MSG_ERROR("BTaggingEfficiencyTool::getInefficiencyScaleFactor call to underlying code returned a kError!");
    return CorrectionCode::Error;
  case Analysis::kRange:
  case Analysis::kExtrapolatedRange:
    return m_ignoreOutOfValidityRange ? CorrectionCode::Ok : CorrectionCode::OutOfValidityRange;
  case Analysis::kSuccess:
  default:
    return CorrectionCode::Ok;
  }
}
  
CorrectionCode
BTaggingEfficiencyTool::getMCEfficiency( const xAOD::Jet & jet, float & eff)
{
  if (! m_initialised) return CorrectionCode::Error;

  // get the btag label
  int flavour = jetFlavourLabel(jet, m_coneFlavourLabel, m_extFlavourLabel);

  Analysis::CalibrationDataVariables vars;
  if (! fillVariables(jet, vars)) {
    ATH_MSG_ERROR("unable to fill variables required for scale factor evaluation");
    return CorrectionCode::Error;
  }
  
  return getMCEfficiency( flavour, vars, eff);
}

CorrectionCode
BTaggingEfficiencyTool::getMCEfficiency( int flavour, const Analysis::CalibrationDataVariables& v, 
					 float & eff)
{
  if (! m_initialised) return CorrectionCode::Error;
  CalibResult result;

  unsigned int sfindex = 0;
  unsigned int efindex = 0;
  
  if( !getIndices(flavour,sfindex,efindex)) {
    ATH_MSG_ERROR("BTaggingEfficiencyTool::getMCEfficiency call to getIndices failed " << flavour << " " << sfindex << " " << efindex);
    return CorrectionCode::Error;
  }
  Uncertainty unc = None;
  // no uncertainty index here as there aren't any uncertainties associated with the MC efficiencies
  CalibrationStatus status = m_CDI->getMCEfficiency(v,efindex, unc,result);
  eff = result.first;
  if( m_applySyst && !m_applyThisSyst.isUp) {
    eff = result.second; // down variation
  }
  
  switch (status) {
  case Analysis::kError:
    ATH_MSG_ERROR("BTaggingEfficiencyTool::getMCEfficiency call to underlying code returned a kError!");
    return CorrectionCode::Error;
  case Analysis::kRange:
  case Analysis::kExtrapolatedRange:
    return m_ignoreOutOfValidityRange ? CorrectionCode::Ok : CorrectionCode::OutOfValidityRange;
  case Analysis::kSuccess:
  default:
    return CorrectionCode::Ok;
  }
}

// get efficiencies with the onnx model (fixed cut wp)
CorrectionCode
BTaggingEfficiencyTool::getMCEfficiencyONNX( const std::vector<std::vector<float>>& node_feat, std::vector<float>& effAllJet)
{
  m_saltModel->runInference(node_feat, effAllJet);
  return CorrectionCode::Ok;
}

// get efficiencies with the onnx model (continuous wp)
CorrectionCode
BTaggingEfficiencyTool::getMCEfficiencyONNX( const std::vector<std::vector<float>>& node_feat, std::vector<std::vector<float>>& effAllJetAllWp)
{
  m_saltModel->runInference(node_feat, effAllJetAllWp);
  return CorrectionCode::Ok;
}

// Systematics framework - modelled on PhysicsAnalysis/AnalysisCommon/CPAnalysisExamples/Root/JetCalibrationToolExample3.cxx
// returns true if the argument systematic is supported by this tool
bool BTaggingEfficiencyTool::isAffectedBySystematic( const SystematicVariation & systematic ) const
{
  SystematicSet sys = affectingSystematics();
  return sys.find( systematic) != sys.end();
}

// this returns a list of systematics supported by this tool
SystematicSet BTaggingEfficiencyTool::affectingSystematics() const {
  return m_systematics;
}

// subset of systematics that are recommended
SystematicSet BTaggingEfficiencyTool::recommendedSystematics() const {
  return affectingSystematics();
}

const std::map<SystematicVariation, std::vector<std::string> >
BTaggingEfficiencyTool::listSystematics() const {
  std::map<SystematicVariation, std::vector<std::string> > results;

  if (! m_initialised) {
    ATH_MSG_ERROR("listSystematics() cannot be called before initialisation is finished");
    return results;
  }


  std::vector<unsigned int> all_flavours{5, 4, 15, 0};
  if(!m_using_conventional_labels){
    all_flavours = m_flex_label_integers;
  }

  for (const auto& info : m_systematicsInfo) {
    // The map key is easy...
    const SystematicVariation& variation = info.first;
    // Then see for which flavours this particular key is relevant
    std::vector<std::string> flavours;
    for(const unsigned int flavour : all_flavours){ // Grab the number 5,4,15,0 for B,C,T,Light respectively (or other custom labelings)
      unsigned int idx;
      if (info.second.getIndex(flavour, idx)){ // If the flavour is mapped to an index internally in the SystInfo.indexMap, then return true
        flavours.push_back(getLabel(int(flavour)));
      }
    }
    results[variation] = flavours; // <------ Map the list of flavours that a systematic applies to, to the SystematicVariation object that was retrieved from m_systematicsInfo
  }
  return results;
}


///
/// This method retrieves all systematic uncertainties known to the relevant calibration objects.
/// Since the expected use of this method is in the context of the SFEigen model, we will assume (and not check) that this model is being used.
/// Note: the uncertainties returned are in the format of the underlying CDI, and do not have the rewriting applied to them that one would use in analysis.
///
std::map<std::string, std::vector<std::string> >
BTaggingEfficiencyTool::listScaleFactorSystematics(bool named) const {
  std::map<std::string, std::vector<std::string> > uncertainties;

  std::vector<unsigned int> all_flavours;
  if(m_using_conventional_labels){
    all_flavours = { 5, 4, 15, 0 };
  } else {
    all_flavours = m_flex_label_integers;
  }

  for (const unsigned int flavourID : all_flavours){
    // Assumed model: use eigenvector variations. In this model, the tau SF are identical to the c-jet ones,
    // with merely one additional uncertainty assigned due to the extrapolation.
    unsigned int flavourIDRef = flavourID;
    if (m_using_conventional_labels and flavourID == 15){
      flavourIDRef = 4; // make C - T relationship
    }
    auto mapIter = m_SFIndices.find(flavourIDRef);
    if( mapIter==m_SFIndices.end()) { // if the flavour doesn't have an entry need to fail the initialization
      ATH_MSG_ERROR( "No entry for flavour " << flavourIDRef << " in SFIndices map, invalid initialization");
      continue;
    }
    int idRef = mapIter->second;
    // Retrieve the actual list
    std::vector<std::string> systematics = m_CDI->listScaleFactorUncertainties(idRef, getLabel(flavourID), named);
    // For the special case of tau SF, add the extrapolation from charm.
    // Since this comes on top of the charm uncertainties, it would always be a "named" uncertainty,
    // so there is no need to check for the "named" argument.
    if (m_using_conventional_labels and flavourID == 15) systematics.push_back("extrapolation from charm");
    uncertainties[getLabel(int(flavourID))] = systematics;
  }
  return uncertainties;
}

CorrectionCode
BTaggingEfficiencyTool::getEigenRecompositionCoefficientMap(const std::string &label, std::map<std::string, std::map<std::string, float>> & coefficientMap){
  // Calling EigenVectorRecomposition method in CDI and retrieve recomposition map.
  // If success, coefficientMap would be filled and return ok.
  // If failed, return error.
  // label  :  flavour label
  // coefficientMap: store returned coefficient map.
  if (! m_initialised) {
    ATH_MSG_ERROR("BTaggingEfficiencyTool has not been initialised");
    return CorrectionCode::Error;
  }
  if(label.compare("B") != 0 &&
     label.compare("C") != 0 &&
     label.compare("T") != 0 &&
     label.compare("Light") != 0){
    ATH_MSG_ERROR("Flavour label is illegal! Label need to be B,C,T or Light.");
    return CorrectionCode::Error;
  }
  CalibrationStatus status = m_CDI->runEigenVectorRecomposition(m_jetAuthor, label, m_OP);
  if (status != Analysis::kSuccess){
    ATH_MSG_ERROR("Failure running EigenVectorRecomposition Method.");
    return CorrectionCode::Error;
  }
  coefficientMap = m_CDI->getEigenVectorRecompositionCoefficientMap();
  return CorrectionCode::Ok;
}

// WARNING the behaviour of future calls to getEfficiency and friends are modified by this
// method - it indicates which systematic shifts are to be applied for all future calls
StatusCode BTaggingEfficiencyTool::applySystematicVariation( const SystematicSet & systConfig) {
  // If the user is doing the right thing, no need to use the costly filterForAffectingSystematics
  // i.e if only 1 variation passed and this variation is in the map. Else, resort to full logic.
  if (systConfig.size() == 1 ) {
    auto mapIter = m_systematicsInfo.find(*(systConfig.begin()));
    if (mapIter != m_systematicsInfo.end()) {
      m_applySyst = true;
      m_applyThisSyst = mapIter->second;
      ATH_MSG_VERBOSE("variation '" << systConfig.begin()->name() << "' applied successfully");
      return StatusCode::SUCCESS;
    }
  }

  // First filter out any systematics that do not apply to us
  SystematicSet filteredSysts;
  if (SystematicSet::filterForAffectingSystematics(systConfig, affectingSystematics(), filteredSysts) != StatusCode::SUCCESS) {
    ATH_MSG_ERROR("received unsupported systematics: " << systConfig.name());
    return StatusCode::FAILURE;
  }
  // check the size of the remaining (filtered) SystematicSet
  if (filteredSysts.size() == 0) {
    // If it is 0 then turn off systematics
    ATH_MSG_VERBOSE("empty systematics set; nothing to be done");
    m_applySyst = false;
    return StatusCode::SUCCESS;
  } else if (filteredSysts.size() > 1) {
    // Restriction: we allow only a single systematic variation affecting b-tagging
    ATH_MSG_WARNING("more than a single b-tagging systematic variation requested but not (yet) supported");
    return StatusCode::FAILURE;
  } else {
    // Interpret the (single) remaining variation
    SystematicVariation var = *(filteredSysts.begin());
    auto mapIter = m_systematicsInfo.find(var);
    if (mapIter == m_systematicsInfo.end()) {
      ATH_MSG_WARNING("variation '" << var.name() << "' not found! Cannot apply");
      return StatusCode::FAILURE;
    }
    m_applySyst = true;
    m_applyThisSyst = mapIter->second;
    ATH_MSG_VERBOSE("variation '" << var.name() << "' applied successfully");
  }
  return StatusCode::SUCCESS;
}
//

bool
BTaggingEfficiencyTool::fillVariables( const xAOD::Jet & jet, CalibrationDataVariables& x) const
{
  x.jetPt = jet.pt();
  x.jetEta = jet.eta();
  x.jetTagWeight = 0.;
  x.jetAuthor = m_jetAuthor;
  //bool weightOK = true;

  if (m_isContinuous2D){
    x.jetTagWeight = m_selectionTool->getQuantile(jet)+0.5;
  }
  else if (m_isContinuous) {
    if(m_readFromBTaggingObject){
        const xAOD::BTagging* tagInfo = xAOD::BTaggingUtilities::getBTagging( jet );
        if (!tagInfo) return false;
    }
    // For now, we defer the tag weight computation to the selection tool only in the case of DL1* (this is likely to be revisited)
    if (m_taggerName.find("DL1") != std::string::npos || m_taggerName.find("GN1") != std::string::npos || m_taggerName.find("GN2") != std::string::npos) {
      return (m_selectionTool->getTaggerWeight(jet, x.jetTagWeight, m_useCTag) == CP::CorrectionCode::Ok);
    } else {
      ATH_MSG_ERROR("BTaggingEfficiencyTool doesn't support tagger: "+m_taggerName);
      return CorrectionCode::Error;
    }
  }

  return true;
}

bool
BTaggingEfficiencyTool::fillVariables( const double jetPt, const double jetEta, const double jetTagWeight, CalibrationDataVariables& x) const
{
  x.jetPt = jetPt;
  x.jetEta = jetEta;
  x.jetTagWeight = jetTagWeight;
  x.jetAuthor = m_jetAuthor;

  return true;
}

// FIXME - if this method is kept - then need additional version that lets one pick the MapIndex by name - but this 
//         would also mean a change in the CDI tool to retrieve the list of names
// FIXME - this method might screw up the systematics framework by changing the list of valid systematics??? needs checking
bool
BTaggingEfficiencyTool::setMapIndex(const std::string& label, unsigned int index)
{
  // do nothing unless it's needed!
  if (m_initialised && index == m_mapIndices[label]) return true;
  // convert to integer index
  unsigned int flavourID = -1; // set default, error if ever seen
  if (m_using_conventional_labels){
    flavourID = getFlavourID(label);
  } else {
    auto iter = std::find(m_flex_labels.begin(), m_flex_labels.end(), label);
    if (iter != m_flex_labels.end()){
      flavourID = getFlavourID(label, false);
    }
  }
  // retrieve the new calibration index
  unsigned int effIndex;
  if (m_CDI->retrieveCalibrationIndex(label, m_OP, m_jetAuthor, false, effIndex, index)) {
    // replace cached information
    m_mapIndices[label] = index; 
    m_EffIndices[flavourID] = effIndex; // This shortcuts you from flavourID to Eff container
    unsigned int sfIndex;
    if( m_CDI->retrieveCalibrationIndex(label, m_OP, m_jetAuthor, true, sfIndex, index)) {
      m_SFIndices[flavourID] = sfIndex; // This shortcuts you from flavourID to SF container
      return true;
    } else {
      ATH_MSG_ERROR("setMapIndex failed to find a SF calibration object" << label << " " << index);
    }
  } else {
    // flag non-existent calibration object & do nothing
    ATH_MSG_ERROR("setMapIndex failed to find an Eff calibration object" << label << " " << index);
  }
  return false;
}
bool BTaggingEfficiencyTool::setMapIndex(unsigned int dsid){

  if(m_DSID_to_MapIndex.find(dsid) == m_DSID_to_MapIndex.end() ){
    ATH_MSG_WARNING("setMapIndex DSID "  << dsid << "not found in config file");

  }else{
      unsigned int map_index = m_DSID_to_MapIndex[dsid];

      bool set_b = setMapIndex("B",map_index);
      bool set_c = setMapIndex("C",map_index);
      bool set_light = setMapIndex("Light",map_index);
      bool set_t = setMapIndex("T",map_index);

      return set_b && set_c && set_light && set_t;
  }


  return false;

}

// private method to generate the list of eigenvector variations
// internally these are not named, they are just numbered
// but the systematics framework needs names
std::vector<std::string> BTaggingEfficiencyTool::makeEigenSyst(const std::string & flav, int number, const std::string& suffix) {
  std::vector<std::string> systStrings;
  for(int i=0;i<number;++i) {
    std::ostringstream ost;
    ost << flav << "_" << i << suffix;
    std::string basename="FT_EFF_Eigen_"+ost.str();
    systStrings.push_back(basename);
  }
  return systStrings;
}

bool
BTaggingEfficiencyTool::getIndices(unsigned int flavour, unsigned int & sf, unsigned int & ef) const {
  auto mapIter = m_SFIndices.find(flavour);
  if(mapIter != m_SFIndices.end()) {
    sf = mapIter->second;
  } else {
    return false;
  }
    
  mapIter = m_EffIndices.find(flavour);
  if(mapIter != m_EffIndices.end()) {
    ef = mapIter->second;
  } else {
    return false;
  }
  return true;
}

bool
BTaggingEfficiencyTool::SystInfo::getIndex( unsigned int flavourID, unsigned int & index) const {
  auto mapIter = indexMap.find(flavourID); // if this systematic applies to flavour (i.e. if it's found) then return the index of the variation (numVariation) internal to CDIROOT
  if (mapIter==indexMap.end()) {
    return false;
  } else {
    index = mapIter->second;
    return true;
  }
}

// helper method to take a list of systematic names and a flavour and add them to the map of SystematicVariation to SystInfo
// this map is used to do the lookup of which systematic to apply.
// ie it is used to map the systematics framework on the systematics approach of the CDI

bool BTaggingEfficiencyTool::addSystematics(const std::vector<std::string> & systematicNames, unsigned int flavourID, Uncertainty uncType) {
  for (int i=0, n=systematicNames.size(); i<n; ++i) {
    const std::string systName = systematicNames[i];
    SystematicVariation up(systName,1);
    SystematicVariation down(systName,-1);
    std::map<SystematicVariation,SystInfo>::iterator iter = m_systematicsInfo.find(up);
    if (iter == m_systematicsInfo.end()) {
      // First case: new variation
      SystInfo info;
      info.isUp = true;
      info.uncType = uncType;
      info.indexMap[flavourID] = i;
      m_systematicsInfo[up] = info;
      ATH_MSG_VERBOSE("addSystematics: adding " << systName << " for flavour " << getLabel(flavourID));
      info.isUp = false;
      m_systematicsInfo[down]=info;
    } else {
      // Second case: already known variation. This can happen if a variation applies to more than one
      // jet flavour. Check that indeed it's not registered yet for the requested flavour.
      SystInfo info = iter->second; // make a copy
      std::map<unsigned int, unsigned int>::const_iterator indIter = info.indexMap.find(flavourID);
      if (indIter != info.indexMap.end()) {
        ATH_MSG_ERROR("addSystematics : flavourID " << flavourID << " is already in the map for uncertainty '" << systName << "', ignoring");
        continue;
      } else {
        info.indexMap[flavourID] = i;
        m_systematicsInfo[up] = info;
        ATH_MSG_VERBOSE("addSystematics: adding " << systName << " for flavour " << getLabel(flavourID));
        info.isUp = false;
        m_systematicsInfo[down] = info;
      }
    }
  }
  return true;
}
