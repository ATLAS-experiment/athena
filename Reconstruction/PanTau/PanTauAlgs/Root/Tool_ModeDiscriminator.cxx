/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PanTauAlgs/Tool_ModeDiscriminator.h"
#include "PanTauAlgs/Tool_InformationStore.h"
#include "PanTauAlgs/TauFeature.h"
#include "PanTauAlgs/PanTauSeed.h"
#include "PanTauAlgs/HelperFunctions.h"
#include "PathResolver/PathResolver.h"
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include <memory>

PanTau::Tool_ModeDiscriminator::Tool_ModeDiscriminator(const std::string& name) :
  asg::AsgTool(name),
  m_MVABDT_List()
{
}


PanTau::Tool_ModeDiscriminator::~Tool_ModeDiscriminator() = default;


StatusCode PanTau::Tool_ModeDiscriminator::initialize() {

  ATH_MSG_DEBUG( name() << " initialize()" );
  m_init=true;
    
  ATH_CHECK( HelperFunctions::bindToolHandle( m_Tool_InformationStore, m_Tool_InformationStoreName ) );

  ATH_CHECK(m_Tool_InformationStore.retrieve());
    
  // get the required information from the informationstore tool
  ATH_CHECK( m_Tool_InformationStore->getInfo_VecDouble("ModeDiscriminator_BinEdges_Pt", m_BinEdges_Pt));
  ATH_CHECK( m_Tool_InformationStore->getInfo_String("ModeDiscriminator_TMVAMethod", m_MethodName) );
    
  // build the name of the variable that contains the variable list for this discri tool
  std::string varNameList_Full    = "ModeDiscriminator_BDTVariableNames_CellBased_" + m_Name_ModeCase;
  ATH_CHECK( m_Tool_InformationStore->getInfo_VecString(varNameList_Full, m_List_BDTVariableNames) );
    
  std::string varDefaultValueList_Full    = "ModeDiscriminator_BDTVariableDefaults_CellBased_" + m_Name_ModeCase;
  ATH_CHECK( m_Tool_InformationStore->getInfo_VecDouble(varDefaultValueList_Full, m_List_BDTVariableDefaultValues) );
    
    
  // consistency check:
  // Number of feature names and feature default values has to match
  if ( m_List_BDTVariableDefaultValues.size() != m_List_BDTVariableNames.size() ) {
    ATH_MSG_ERROR("Number of variable names does not match number of default values! Check jobOptions!");
    return StatusCode::FAILURE;
  }
  
  // Create reader for each pT Bin; nBins =  Edges-1    
  for (unsigned int iPtBin=0; iPtBin<(m_BinEdges_Pt.size() - 1); iPtBin++) {
        
    std::string bin_lowerStr    = m_HelperFunctions.convertNumberToString(m_BinEdges_Pt[iPtBin]/1000.);
    std::string bin_upperStr    = m_HelperFunctions.convertNumberToString(m_BinEdges_Pt[iPtBin+1]/1000.);     
    std::string curPtBin        = "ET_" + bin_lowerStr + "_" + bin_upperStr;
    
    // weight files
    std::string curWeightFile = m_calib_path + (!m_calib_path.empty() ? "/" : "");
    curWeightFile += "TrainModes_";
    curWeightFile += "CellBased_";
    curWeightFile += curPtBin + "_";
    curWeightFile += m_Name_ModeCase + "_";
    curWeightFile += m_MethodName + ".weights.root";

    std::string resolvedWeightFileName = PathResolverFindCalibFile(curWeightFile);

    if (resolvedWeightFileName.empty()) {
      ATH_MSG_ERROR("Weight file " << curWeightFile << " not found!");
      return StatusCode::FAILURE;
    }
    
    // MVAUtils BDT       
    std::unique_ptr<TFile> fBDT = std::make_unique<TFile>( resolvedWeightFileName.c_str() );
    TTree* tBDT = dynamic_cast<TTree*> (fBDT->Get("BDT"));
    std::unique_ptr<MVAUtils::BDT> curBDT = std::make_unique<MVAUtils::BDT>(tBDT);
    if (curBDT == nullptr) {
      ATH_MSG_ERROR( "Failed to create MVAUtils::BDT for " << resolvedWeightFileName );
      return StatusCode::FAILURE;
    }

    m_MVABDT_List.push_back(std::move(curBDT));
        
  }//end loop over pt bins to get weight files, reference hists and MVAUtils::BDT objects
    
  return StatusCode::SUCCESS;
}


void PanTau::Tool_ModeDiscriminator::updateReaderVariables(PanTau::PanTauSeed* inSeed, std::vector<float>& list_BDTVariableValues) const {
    
  //update features used in MVA with values from current seed
  // use default value for feature if it is not present in current seed
  //NOTE! This has to be done (even if the seed pt is bad) otherwise problems with details storage
  //      [If this for loop is skipped, it is not guaranteed that all details are set to their proper default value]
  PanTau::TauFeature* seedFeatures = inSeed->getFeatures();

  for (unsigned int iVar=0; iVar<m_List_BDTVariableNames.size(); iVar++) {
    std::string curVar = "CellBased_" + m_List_BDTVariableNames[iVar];
        
    bool isValid;
    double newValue = seedFeatures->value(curVar, isValid);
    if (!isValid) {
      ATH_MSG_DEBUG("\tUse default value as the feature (the one below this line) was not calculated");
      newValue = m_List_BDTVariableDefaultValues[iVar];
      //add this feature with its default value for the details later
      seedFeatures->addFeature(curVar, newValue);
    }
        
    list_BDTVariableValues[iVar] = static_cast<float>(newValue);
  }//end loop over BDT vars
    
  }


double PanTau::Tool_ModeDiscriminator::getResponse(PanTau::PanTauSeed* inSeed, bool& isOK) const {
    
  std::vector<float> list_BDTVariableValues(m_List_BDTVariableNames.size());

  updateReaderVariables(inSeed, list_BDTVariableValues);
    
  if (inSeed->isOfTechnicalQuality(PanTau::PanTauSeed::t_BadPtValue)) {
    ATH_MSG_DEBUG("WARNING Seed has bad pt value! " << inSeed->getTauJet()->pt() << " MeV");
    isOK = false;
    return -2;
  }
    
  //get the pt bin of input Seed
  //NOTE: could be moved to decay mode determinator tool...
  int ptBin = -1;
  for (unsigned int iPtBin=0; iPtBin<m_BinEdges_Pt.size()-1; iPtBin++) {
    if (inSeed->p4().Pt() > m_BinEdges_Pt[iPtBin] && inSeed->p4().Pt() < m_BinEdges_Pt[iPtBin+1]) {
      ptBin = iPtBin;
      break;
    }
  }
  if (ptBin == -1) {
    ATH_MSG_WARNING("Could not find ptBin for tau seed with pt " << inSeed->p4().Pt());
    isOK = false;
    return -2.;
  }
    
  isOK = true;
  
  // return the  mva response
  return m_MVABDT_List[ptBin]->GetGradBoostMVA(list_BDTVariableValues);
}
