/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/


#include <AsgTools/StandaloneToolHandle.h>
#include <AsgTools/ToolHandle.h>
#include "AsgMessaging/MessageCheck.h"
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"
#include "FTagAnalysisInterfaces/IBTaggingEigenVectorRecompositionTool.h"

#include <string>

ANA_MSG_HEADER(testBTagEigenVecRec)
ANA_MSG_SOURCE(testBTagEigenVecRec, "BTaggingEigenVectorRecompositionToolTester")
using namespace testBTagEigenVecRec;

int main ATLAS_NOT_THREAD_SAFE (int argc, char* argv[]) {

  const char* TEST_NAME = argv[0];
  if (argc < 3) {
    ANA_MSG_ERROR ( "No right inputs received!" );
    ANA_MSG_ERROR ( "Usage: " << TEST_NAME << "[CDI path] [b-tagger name] [WP name]" );
    return 1;
  }

  std::string CDIPath    = argv[1];
  std::string taggerName = argv[2];
  std::string workingPointName = argv[3];
  
  asg::StandaloneToolHandle<IBTaggingEfficiencyTool> btag_eff_tool("BTaggingEfficiencyTool/BTagEffTest");
  StatusCode code1 = btag_eff_tool.setProperty("ScaleFactorFileName", CDIPath);
  StatusCode code2 = btag_eff_tool.setProperty("TaggerName",    taggerName);
  StatusCode code3 = btag_eff_tool.setProperty("OperatingPoint", workingPointName);
  StatusCode code4 = btag_eff_tool.setProperty("JetAuthor", "AntiKt4EMPFlowJets" );
  StatusCode code5 = btag_eff_tool.setProperty("MinPt", 20. );
  // Exclude certain original uncertainties from Eigenvector scheme so that
  // it these uncertainties will be exclude from eigen vector recomposition. 
  // The original uncertainty names are separated by semicolon.
  // Here exclude two uncertainties as an example.
  // StatusCode code0 = btag_eff_tool.setProperty("ExcludeFromEigenVectorBTreatment","FT_EFF_PDF4LHC_np_19;JET_EffectiveNP_Mixed3");
  StatusCode code6 = btag_eff_tool.initialize();
  std::vector<StatusCode> codes = {code1, code2, code3, code4, code5, code6};
  for (const auto& code : codes) {
    if (code != StatusCode::SUCCESS) {
      ANA_MSG_ERROR("Initialization of tool " << btag_eff_tool->name() << " failed! ");
      return 1;
    }else {
      ANA_MSG_INFO("Initialization of tool " << btag_eff_tool->name() << " finished! ");
    }
  } 

  ANA_MSG_INFO("----------------------------------");
  asg::StandaloneToolHandle<IBTaggingEigenVectorRecompositionTool> evr_tool("BTaggingEigenVectorRecompositionTool/BTagEVRTest");
  StatusCode code7 = evr_tool.setProperty("BTaggingEfficiencyTool", btag_eff_tool);
  StatusCode code8 = evr_tool.initialize();
  if (code7 != StatusCode::SUCCESS || code8 != StatusCode::SUCCESS ) {
    ANA_MSG_ERROR("Initialization of tool " << evr_tool->name() << " failed! ");
    return 1;
  } else {
    ANA_MSG_INFO("Initialization of tool " << evr_tool->name() << " finished! ");
  }
  
  ANA_MSG_INFO("----------------------------------");
  const std::string flavour_labels[4] = {"B","C","T","Light"};

  for (const std::string& label : flavour_labels) {
    ANA_MSG_INFO("--- Eigenvector decomposition for flavour: " << label << " ---");

    /**
       getNumEigenVectors(label)
       
       input value:
       1. label: flavour label in std::string format, could be one of B, C, T, Light
       return value:
       number of eigen vectors used for chosen label. Return 0 if error occured. 
     */

    int nEigen = evr_tool->getNumEigenVectors(label);
    ANA_MSG_DEBUG("----------------------------------");
    ANA_MSG_DEBUG("  Number of eigenvectors: " << nEigen);
    ANA_MSG_DEBUG("----------------------------------");
    /**
       getCoefficients(label, evIdx)
       
       input value:
       1. label: flavour label in std::string format, could be one of B, C, T, Light
       2. evIdx: The index of eigenvector user interested in.
       output value:
       vector of coefficient values. The order is the same as output given by
       getListOfOriginalNuisanceParameters()
     */
    const unsigned int evIdx = 0;
    std::vector<float> coeffs = evr_tool->getCoefficients(label, evIdx);
    ANA_MSG_DEBUG("Eigenvector " << evIdx << ":");
    for (float c : coeffs)
      ANA_MSG_DEBUG(" " << std::fixed << c);
    ANA_MSG_DEBUG("----------------------------------");

    float norm = std::sqrt(std::inner_product(coeffs.begin(), coeffs.end(), coeffs.begin(), 0.0f));
    ANA_MSG_DEBUG("Norm of Eigenvector " << evIdx << " is " << norm);
    std::vector<float> coeffs1 = evr_tool->getCoefficients(label, evIdx+1);
    float norm1 = std::sqrt(std::inner_product(coeffs1.begin(), coeffs1.end(), coeffs1.begin(), 0.0f));
    ANA_MSG_DEBUG("Norm of Eigenvector " << (evIdx + 1)<< " is " << norm1);
    std::vector<float> coeffs2 = evr_tool->getCoefficients(label, evIdx+2);
    float norm2 = std::sqrt(std::inner_product(coeffs2.begin(), coeffs2.end(), coeffs2.begin(), 0.0f));
    ANA_MSG_DEBUG("Norm of Eigenvector " << (evIdx + 2)<< " is " << norm2);
    ANA_MSG_DEBUG("----------------------------------");

    /**
       getListOfOriginalNuisanceParameters(label)
       
       input value:
       1. label: flavour label in std::string format, could be one of B, C, T, Light
       output value:
       List of original nuisance parameter names.
     */
    std::vector<std::string> orig_nps = evr_tool->getListOfOriginalNuisanceParameters(label);
    ANA_MSG_DEBUG("Original nuisance parameters (" << orig_nps.size() << "):");
    for (const auto& np : orig_nps)
      ANA_MSG_DEBUG(np << " ");
    ANA_MSG_DEBUG("----------------------------------");
    /**
       getCoefficientMap(label, EigenIdxList)
        input value:
       1. label: flavour label in std::string format, could be one of B, C, T, Light
       2. EigenIdxList is user defined vector containing all eigenvector index
       that user interested in.
       output:
       Map of format map<string, map<string, float>> containing decomposition
       coefficient of the list of eigenvectors defined by EigenIdxList.
    */
    std::vector<unsigned int> eigenIdxList = {0,1,2,3,4,5};
    std::map<std::string, std::map<std::string, float>> coefficientMap = evr_tool->getCoefficientMap(label, eigenIdxList);
    ANA_MSG_DEBUG("Coefficient map summary (first key):");
    ANA_MSG_DEBUG("----------------------------------");
    if (!coefficientMap.empty()) {
      const auto& firstEntry = *coefficientMap.begin();
      const std::string& npName = firstEntry.first;
      const auto& evMap = firstEntry.second;
      ANA_MSG_DEBUG("NP " << npName);
      for (const auto& [evName, coeff] : evMap)
        ANA_MSG_DEBUG(evName << ":" << std::fixed << coeff << " ");
    } else {
      ANA_MSG_ERROR("Coefficient map is empty!");
      return 1;
    }

  }
  return 0;

}
