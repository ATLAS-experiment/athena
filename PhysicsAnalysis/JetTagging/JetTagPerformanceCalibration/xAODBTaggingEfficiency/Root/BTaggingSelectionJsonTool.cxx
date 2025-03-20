/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include "xAODBTaggingEfficiency/BTaggingSelectionJsonTool.h"
#include <fstream>

BTaggingSelectionJsonTool::BTaggingSelectionJsonTool( const std::string & name)
  : asg::AsgTool( name )
{
  m_initialised = false;
  declareProperty( "MaxEta", m_maxEta = 2.5 );
  declareProperty( "MinPt", m_minPt = -1 /*MeV*/);
  declareProperty( "TaggerName",                    m_taggerName="",       "tagging algorithm name");
  declareProperty( "JetAuthor",                     m_jetAuthor="",        "jet collection");
  declareProperty( "OperatingPoint",                m_OP="",               "operating point");
  declareProperty( "JsonConfigFile",                m_json_config_path="", "Path to JSON config file");
}

StatusCode BTaggingSelectionJsonTool::initialize() {
  m_initialised = true;
  
  std::ifstream jsonFile(m_json_config_path);
  if (!jsonFile.is_open()) {
    ATH_MSG_ERROR( "JSON file " + m_json_config_path + " do not exist. Please put the correct path of the file." );
    return StatusCode::FAILURE;
  }
  m_json_config = json::parse(jsonFile);
  jsonFile.close();

  if (m_taggerName.empty() || !m_json_config.contains(m_taggerName)){
    ATH_MSG_ERROR( "Tagger " + m_taggerName + " not found in JSON file: " + m_json_config_path );
    return StatusCode::FAILURE;
  }

  if (m_jetAuthor.empty() || !m_json_config[m_taggerName].contains(m_jetAuthor)){
    ATH_MSG_ERROR( "Tagger: " +m_taggerName+ " and Jet Collection: " +m_jetAuthor+ " not found in JSON file: " +m_json_config_path );
    return StatusCode::FAILURE;
  }

  if (m_OP.empty() || !m_json_config[m_taggerName][m_jetAuthor].contains(m_OP)){
    ATH_MSG_ERROR( "OP " +m_OP+ " not available for " +m_taggerName+ " tagger.");
    return StatusCode::FAILURE;
  }

  const auto& meta = m_json_config[m_taggerName][m_jetAuthor]["meta"];
  m_target = meta["TaggingTarget"];

  // pre-load fraction values
  for (const auto& outclass : meta["categories"]) {
    std::string outclassStr = std::string(outclass);
    float fraction = meta["fraction_" + outclassStr].get<float>();
    SG::AuxElement::ConstAccessor<float> accessor(m_taggerName + "_p" + outclassStr);
    bool isTarget = (outclassStr == m_target);
    m_fractionAccessors.emplace_back(fraction, accessor, isTarget);
  }

  // pre-load cut values
  auto& pT_mass_2d_cutvalue = m_json_config[m_taggerName][m_jetAuthor][m_OP]["pT_mass_2d_cutvalue"];

  // Loop over the pT bins values 
  // pTbins is a list of floats or "inf" for the highest bin value 
  for (unsigned int ipT = 0; ipT < pT_mass_2d_cutvalue["pTbins"].size(); ++ipT){
    // Get pT information 
    const auto &pt = pT_mass_2d_cutvalue["pTbins"][ipT];
    // retrieve pT value as a float 
    m_pTbins.push_back(BTaggingToolUtil::getExtendedFloat(pt));

    // Retrieve corresponding pT bin i.e. "pT_lowerEdge_upperEdge"
    // and related mass bin values and efficiency cut values 
    if (ipT != pT_mass_2d_cutvalue["pTbins"].size()-1){
      // Retrieve upper edge pT value 
      const auto &ptUp = pT_mass_2d_cutvalue["pTbins"][ipT+1];

      // here get pT values as strings 
      std::string pT_key = "pT_" + BTaggingToolUtil::getExtendedString(pt) + "_" + BTaggingToolUtil::getExtendedString(ptUp); 

      // Make sure that the pT bin information can be found in the json file 
      auto itr = pT_mass_2d_cutvalue.find(pT_key);
      if (itr == pT_mass_2d_cutvalue.end()){
        ATH_MSG_ERROR( "pT_key=" + pT_key + " not found in JSON file: " +m_json_config_path );
        return StatusCode::FAILURE;
      }

      // Retrieve mass values and cut values 
      std::vector<float> mass_values = itr->at("mass").get<std::vector<float>>();
      std::vector<float> cut_values = itr->at("cutvalues").get<std::vector<float>>();

      // Add the corresponding mass bins and OP cut values information 
      m_massbins.push_back(mass_values);
      m_OPCutValues.push_back(cut_values);
    }
  }

  return StatusCode::SUCCESS;
}

double BTaggingSelectionJsonTool::getTaggerDiscriminant ( const xAOD::Jet& jet) const{

  float numerator = 0.;
  float denominator = 0.;
  for ( const auto& frac : m_fractionAccessors ) {
    float p_output = frac.accessor( jet ); 
    if ( frac.isTarget ) {
      numerator += frac.fraction * p_output;
    } else {
      denominator += frac.fraction * p_output;
    }
  }

  double tagger_discriminant = log(numerator / denominator); 

  return tagger_discriminant;
}

int BTaggingSelectionJsonTool::accept( const xAOD::Jet& jet ) const {
  ///////////////////////////////////////////////
  // Cheatsheet:
  // For fix cut WP, return 0 for not tagged, 1 for tagged
  ////////////////////////////////////////////////
  return accept( jet.pt(), jet.eta(), jet.m(), getTaggerDiscriminant(jet) );
}

int BTaggingSelectionJsonTool::accept( double pt, double eta, double mass, double tagger_discriminant ) const {

  if ( !m_initialised ) {
    throw std::runtime_error("BTaggingSelectionJsonTool has not been initialised.");
  }

  int index = 0;

  if ( std::abs(eta) > m_maxEta || pt < m_minPt ) {
    return index;
  }

  int pt_bin_index = findBin(m_pTbins, pt/1000.);
  if (pt_bin_index == -1) {
    return index;
  }

  int mass_bin_index = findBin(m_massbins[pt_bin_index], mass/1000.);
  if (mass_bin_index == -1) {
    return index;
  }

  float cutvalue = m_OPCutValues[pt_bin_index][mass_bin_index];
  index = (tagger_discriminant > cutvalue) ? 1 : 0;
  return index;

}

int BTaggingSelectionJsonTool::findBin(const std::vector<float>& bins, float value) const {
  for (size_t i = 0; i < bins.size()-1; i++) {
    if ((std::min(bins[i], bins[i+1]) <= value && value < std::max(bins[i], bins[i+1]))) {
      return i;
    }
  }
  return -1;
}
