/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
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

  if (m_taggerName=="" || !m_json_config.contains(m_taggerName)){
    ATH_MSG_ERROR( " Tagger " + m_taggerName + " not found in JSON file: " + m_json_config_path );
    return StatusCode::FAILURE;
  }

  if (m_jetAuthor=="" || !m_json_config[m_taggerName].contains(m_jetAuthor)){
    ATH_MSG_ERROR( "Tagger: " +m_taggerName+ " and Jet Collection: " +m_jetAuthor+ " not found in JSON file: " +m_json_config_path );
    return StatusCode::FAILURE;
  }

  if (m_OP=="" || !m_json_config[m_taggerName][m_jetAuthor].contains(m_OP)){
    ATH_MSG_ERROR( "OP " +m_OP+ " not available for " +m_taggerName+ " tagger.");
    return StatusCode::FAILURE;
  }

  m_target = m_json_config[m_taggerName][m_jetAuthor]["meta"]["TaggingTarget"];

  // pre-load fraction values
  m_fractionAccessors.clear();
  for (const auto& outclass : m_json_config[m_taggerName][m_jetAuthor]["meta"]["categories"]) {
    std::string outclassStr = std::string(outclass);
    float fraction = m_json_config[m_taggerName][m_jetAuthor]["meta"]["fraction_" + outclassStr].get<float>();
    SG::AuxElement::ConstAccessor<float> accessor(m_taggerName + "_p" + outclassStr);
    bool isTarget = (outclassStr == m_target);
    m_fractionAccessors.emplace_back(fraction, accessor, isTarget);
  } 

  // pre-load cut values
  m_OPCutValues.clear();
  if ( m_OP == "Continuous" ) {
    for (const auto& ContinuousOP : m_json_config[m_taggerName][m_jetAuthor]["meta"]["OperatingPoints"]) {
      float cutvalue = m_json_config[m_taggerName][m_jetAuthor][std::string(ContinuousOP)]["meta"]["cutvalue"];
      m_OPCutValues.emplace_back(ContinuousOP, cutvalue);
    }
  } else {
    float cutvalue = m_json_config[m_taggerName][m_jetAuthor][m_OP]["meta"]["cutvalue"];
    m_OPCutValues.emplace_back(m_OP, cutvalue);
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

int BTaggingSelectionJsonTool::accept( const xAOD::Jet& jet) const {
  ///////////////////////////////////////////////
  // Cheatsheet:
  // For fix cut WP, return 0 for not tagged, 1 for tagged
  // For Continuous with n WPs (from highest to lowest) A1, A2, A3, ..., An
  // return 0   if not in b-tagging acceptance
  // return 1   if between 100% and A1 (untagged)
  // return 2   if between A1   and A2 (tagged at the A1 WP)
  // return 3   if between A2   and A3 (tagged at the A2 WP)
  // ...
  // return n   if between An-1 and An (tagged at the An-1 WP)
  // return n+1 if between An   and 0% (tagged at the An WP)
  ////////////////////////////////////////////////

  if ( !m_initialised ) {
    throw std::runtime_error("BTaggingSelectionJsonTool has not been initialised.");
  }

  int index = 0;

  if ( std::abs(jet.eta()) > m_maxEta || jet.pt() < m_minPt ) {
    return index;
  }

  double tagger_discriminant = getTaggerDiscriminant(jet);
  for (const auto& [opName, cutvalue] : m_OPCutValues) {
    if (tagger_discriminant > cutvalue) {
      index += 1;
    }
  }
  return index;
}

