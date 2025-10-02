/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PanTauAlgs/Tool_InformationStore.h"

#define GeV 1000

void PanTau::Tool_InformationStore::ABRDefaultInit(){
#ifdef XAOD_ANALYSIS

  // Boolean values
  MapInt m01 = {
    {"TauConstituents_UsePionMass",1},
    {"FeatureExtractor_UseEmptySeeds",0},
  };
  setMapInt(m01);
  
  // double values
  MapDouble m02 = {
    {"TauConstituents_Types_DeltaRCore",0.2},
    {"TauConstituents_MaxEta",9.9},
    {"TauConstituents_PreselectionMinEnergy",500.},
    // PanTau BDT Cut values --- CellBased
    {"DecayModeDeterminator_BDTCutValue_R10X_CellBased",0.52},
    {"DecayModeDeterminator_BDTCutValue_R11X_CellBased",-0.33},
    {"DecayModeDeterminator_BDTCutValue_R110_CellBased",0.47},
    {"DecayModeDeterminator_BDTCutValue_R1XX_CellBased",-0.21},
    {"DecayModeDeterminator_BDTCutValue_R30X_CellBased",-0.13},
    {"DecayModeDeterminator_BDTCutValue_R3XX_CellBased",-0.08},
  };
  
  setMapDouble(m02);

  // String values
  MapString m03 = {
    {"Name_TauRecContainer","TauJets"},
    {"Name_TrackParticleContainer","TrackParticleCandidate"},
    {"ModeDiscriminator_TMVAMethod","BDTG"},
    {"FeatureExtractor_VarTypeName_varTypeName_Sum", "Sum"},
    {"FeatureExtractor_VarTypeName_varTypeName_Ratio", "Ratio"},
    {"FeatureExtractor_VarTypeName_varTypeName_Isolation", "Isolation"},
    {"FeatureExtractor_VarTypeName_varTypeName_Num", "Num"},
    {"FeatureExtractor_VarTypeName_varTypeName_Mean", "Mean"},
    {"FeatureExtractor_VarTypeName_varTypeName_StdDev", "StdDev"},
    {"FeatureExtractor_VarTypeName_varTypeName_HLV", "HLV"},
    {"FeatureExtractor_VarTypeName_varTypeName_Angle", "Angle"},
    {"FeatureExtractor_VarTypeName_varTypeName_DeltaR", "DeltaR"},
    {"FeatureExtractor_VarTypeName_varTypeName_JetMoment", "JetMoment"},
    {"FeatureExtractor_VarTypeName_varTypeName_Combined", "Combined"},
    {"FeatureExtractor_VarTypeName_varTypeName_Basic", "Basic"},
    {"FeatureExtractor_VarTypeName_varTypeName_PID", "PID"},
    {"FeatureExtractor_VarTypeName_varTypeName_Shots", "Shots"},
  };
  setMapString(m03);

  // vector<double> values
  // In Config_PanTau.py, this was called "vector<float> values". This was changed to "double".
  // The "Units.GeV" (import AthenaCommon.SystemOfUnits as Units) was now replaced by "GeV" (#include "CLHEP/Units/SystemOfUnits.h", using CLHEP::GeV)
  MapVecDouble m04 = {
    {"TauConstituents_BinEdges_Eta",{0.000, 0.800, 1.400, 1.500, 1.900, 9.900}},
    {"TauConstituents_Selection_Neutral_EtaBinned_EtCut",{2.1*GeV, 2.5*GeV, 2.6*GeV, 2.4*GeV, 1.9*GeV}},
    // Eta Binned    P I 0 - B D T   C U T S
    {"CellBased_BinEdges_Eta",{0.000, 0.800, 1.400, 1.500, 1.900, 9.900}},
    {"CellBased_EtaBinned_Pi0MVACut_1prong",{0.46, 0.39, 0.51, 0.47, 0.54}},
    {"CellBased_EtaBinned_Pi0MVACut_3prong",{0.47, 0.52, 0.60, 0.55, 0.50}},
    // P T   B I N S
    {"ModeDiscriminator_BinEdges_Pt",{10*GeV, 100000*GeV}},
    {"ModeDiscriminator_BDTVariableDefaults_CellBased_1p0n_vs_1p1n", {-9.0,     -0.2,    -10.0,     -0.2,     -2.0}},
    {"ModeDiscriminator_BDTVariableDefaults_CellBased_1p1n_vs_1pXn", {-9.0,      -200.0,        -0.2,        -5.0,        -2.0}},
    {"ModeDiscriminator_BDTVariableDefaults_CellBased_3p0n_vs_3pXn", {-0.2,    -9.0,       -0.2,       -2.0,     -200.0}},
  };
  setMapVecDouble(m04);

  // vector<string> values
  MapVecString m05 = {
    {"Names_ModeCases",{"1p0n_vs_1p1n","1p1n_vs_1pXn","3p0n_vs_3pXn"}},
    // ---> CellBased BDT variables
    {"ModeDiscriminator_BDTVariableNames_CellBased_1p0n_vs_1p1n",{"Neutral_PID_BDTValues_BDTSort_1","Neutral_Ratio_1stBDTEtOverEtAllConsts","Combined_DeltaR1stNeutralTo1stCharged","Charged_JetMoment_EtDRxTotalEt","Neutral_Shots_NPhotonsInSeed"}},
    {"ModeDiscriminator_BDTVariableNames_CellBased_1p1n_vs_1pXn",{"Neutral_PID_BDTValues_BDTSort_2","Neutral_HLV_SumM","Neutral_Ratio_EtOverEtAllConsts","Basic_NNeutralConsts","Neutral_Shots_NPhotonsInSeed"}},
    {"ModeDiscriminator_BDTVariableNames_CellBased_3p0n_vs_3pXn",{"Neutral_Ratio_EtOverEtAllConsts","Neutral_PID_BDTValues_BDTSort_1","Charged_StdDev_Et_WrtEtAllConsts","Neutral_Shots_NPhotonsInSeed","Charged_HLV_SumM"}},
    {"ModeDiscriminator_BDTVariableTypes_CellBased_1p0n_vs_1p1n", {"F","F","F","F","F"}},
    {"ModeDiscriminator_BDTVariableTypes_CellBased_1p1n_vs_1pXn", {"F","F","F","F","F"}},
    {"ModeDiscriminator_BDTVariableTypes_CellBased_3p0n_vs_3pXn", {"F","F","F","F","F"}},
  };
  setMapVecString(m05);

#endif
}


PanTau::Tool_InformationStore::Tool_InformationStore(const std::string& name) :
  asg::AsgTool(name)
{
  declareProperty("Infos_String",     m_Infos_String, "Map with string type infos");
  declareProperty("Infos_VecString",  m_Infos_VecString, "Map with vector<string> type infos");
  declareProperty("Infos_Int",        m_Infos_Int,    "Map with int type infos");
  declareProperty("Infos_Double",     m_Infos_Double, "Map with double type infos");
  declareProperty("Infos_VecDouble",  m_Infos_VecDouble, "Map with double type infos");
}

PanTau::Tool_InformationStore::~Tool_InformationStore() = default;

StatusCode PanTau::Tool_InformationStore::initialize() {
  ATH_MSG_INFO( name() << " initialize()" );
  m_init=true;

  //This function does nothing in athena
  ABRDefaultInit();
    
  return StatusCode::SUCCESS;
}

StatusCode PanTau::Tool_InformationStore::getInfo_Int(const std::string& varName, int& value) const {
  MapInt::const_iterator it = m_Infos_Int.find(varName);
  if(it == m_Infos_Int.end()) {
    ATH_MSG_ERROR("getInfo_Int: No integer information called " << varName << " present in InformationStore");
    return StatusCode::FAILURE;
  }
  value = it->second;
  return StatusCode::SUCCESS;
}

StatusCode PanTau::Tool_InformationStore::getInfo_Double(const std::string& varName, double& value) const {
  MapDouble::const_iterator it = m_Infos_Double.find(varName);
  if(it == m_Infos_Double.end()) {
    ATH_MSG_ERROR("getInfo_Double: No double information called " << varName << " present in InformationStore");
    return StatusCode::FAILURE;
  }
  value = it->second;
  return StatusCode::SUCCESS;
}

StatusCode PanTau::Tool_InformationStore::getInfo_VecDouble(const std::string& varName, std::vector<double>& value) const {
  MapVecDouble::const_iterator it = m_Infos_VecDouble.find(varName);
  if(it == m_Infos_VecDouble.end()) {
    ATH_MSG_ERROR("getInfo_VecDouble: No double information called " << varName << " present in InformationStore");
    return StatusCode::FAILURE;
  }
  value = it->second;
  return StatusCode::SUCCESS;
}

StatusCode PanTau::Tool_InformationStore::getInfo_String(const std::string& varName, std::string& value) const {
  MapString::const_iterator it = m_Infos_String.find(varName);
  if(it == m_Infos_String.end()) {
    ATH_MSG_ERROR("getInfo_String: No string information called " << varName << " present in InformationStore");
    return StatusCode::FAILURE;
  }
  value = it->second;
  return StatusCode::SUCCESS;
}

StatusCode PanTau::Tool_InformationStore::getInfo_VecString(const std::string& varName, std::vector<std::string>& value) const {
  MapVecString::const_iterator it = m_Infos_VecString.find(varName);
  if(it == m_Infos_VecString.end()) {
    ATH_MSG_ERROR("getInfo_VecString: No std::string information called " << varName << " present in InformationStore");
    return StatusCode::FAILURE;
  }
  value = it->second;
  return StatusCode::SUCCESS;
}
