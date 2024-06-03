/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "eflowRec/eflowEEtaBinnedParameters.h"
#include "eflowRec/PFCellEOverPTool.h"
#include "eflowRec/eflowCaloRegions.h"

#include "GaudiKernel/SystemOfUnits.h"

#include <PathResolver/PathResolver.h>
#include <nlohmann/json.hpp>

#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>

PFCellEOverPTool::PFCellEOverPTool(const std::string& type,
                                   const std::string& name,
                                   const IInterface* parent)
  : IEFlowCellEOverPTool(type, name, parent)
{

  declareInterface<IEFlowCellEOverPTool>(this);

}

StatusCode PFCellEOverPTool::initialize(){

  std::string path;
  path = PathResolverFindCalibDirectory(m_referenceFileLocation);

  ATH_MSG_INFO("Using reference file location " + path);

  std::ifstream binBoundariesFile(path+"EOverPBinBoundaries.json");
  nlohmann::json json_binBoundaries;
  binBoundariesFile >> json_binBoundaries;

  auto fillBinBoundaries = [](auto && binBoundaries, const nlohmann::json &json_binBoundaries, const std::string &binName){
    if (json_binBoundaries.contains(binName)){
      for (auto binBoundary : json_binBoundaries[binName]) binBoundaries.push_back(binBoundary);
      return true;
    }
    else return false;
  };
  
  bool filledBins = false;
  std::string energyBinBoundaries = "energyBinBoundaries";
  std::string etaBinBoundaries = "etaBinBoundaries";
  std::string firstIntBinBoundaries = "firstIntBinBoundaries";
  std::string caloLayerBinBoundaries = "caloLayerBinBoundaries";

  //here we verify we can load in the bin boundaries - if not we return a FAILURE code
  //because particle flow cannot run correctly without these being loaded
  filledBins = fillBinBoundaries(m_energyBinLowerBoundaries, json_binBoundaries, energyBinBoundaries);
  if (!filledBins) {
    ATH_MSG_ERROR("Could not bin boundaries in json file: " << energyBinBoundaries);
    return StatusCode::FAILURE;
  }
  filledBins = fillBinBoundaries(m_etaBinLowerBoundaries, json_binBoundaries, etaBinBoundaries);
  if (!filledBins) {
    ATH_MSG_ERROR("Could not bin boundaries in json file: " << etaBinBoundaries);
    return StatusCode::FAILURE;
  }
  filledBins = fillBinBoundaries(m_firstIntBinLowerBoundaries, json_binBoundaries, firstIntBinBoundaries);
  if (!filledBins) {
    ATH_MSG_ERROR("Could not bin boundaries in json file: " << firstIntBinBoundaries);
    return StatusCode::FAILURE;
  }
  filledBins = fillBinBoundaries(m_caloLayerBins, json_binBoundaries, caloLayerBinBoundaries);
  if (!filledBins) {
    ATH_MSG_ERROR("Could not bin boundaries in json file: " << caloLayerBinBoundaries);
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode PFCellEOverPTool::fillBinnedParameters(eflowEEtaBinnedParameters *binnedParameters) const {

  if (binnedParameters) {

    binnedParameters->initialise(m_energyBinLowerBoundaries, m_etaBinLowerBoundaries);

    std::string path;
    path = PathResolverFindCalibDirectory(m_referenceFileLocation);  

    std::ifstream inputFile_eoverp(path+"EoverP.json");
    nlohmann::json json_eoverp;
    inputFile_eoverp >> json_eoverp;  

    std::ifstream inputFile_cellOrdering(path+"cellOrdering.json");
    nlohmann::json json_cellOrdering;
    inputFile_cellOrdering >> json_cellOrdering;

    //Loop over energy, eta and first int bins
    //For each combination we set the e/p mean and width from the json file values
    int energyBinCounter = -1;
    for (auto thisEBin : m_energyBinLowerBoundaries){
      energyBinCounter++;
      std::stringstream currentEBinStream;
      currentEBinStream << std::fixed << std::setprecision(0) << thisEBin;
      std::string currentEBin = currentEBinStream.str();
      int etaBinCounter = -1;
      for (auto thisEtaBin : m_etaBinLowerBoundaries){
        etaBinCounter++;
        //check if the eta value is divisible by exactly 1
        unsigned int etaPrecision = 0;
        if (std::fmod(thisEtaBin,1) != 0) etaPrecision = 1;
        std::stringstream currentEtaBinStream;
        currentEtaBinStream << std::fixed << std::setprecision(etaPrecision) << thisEtaBin;
        std::string currentEtaBin = currentEtaBinStream.str();
        for (auto thisFirstIntRegionBin_Int : m_firstIntBinLowerBoundaries){
          //enum version is used for the calls to binnedParameters APIs
      	  auto thisFirstIntRegionBin = static_cast<eflowFirstIntRegions::J1STLAYER>(thisFirstIntRegionBin_Int);
          //and string version is used to lookup values from the json file
      	  std::string currentFirstIntBin = std::to_string(thisFirstIntRegionBin_Int);
          std::string eOverPBin = "energyBinLowerBound_"+currentEBin+"_etaBinLowerBound_"+currentEtaBin+"_firstIntBinLowerBound_"+currentFirstIntBin;
          if (json_eoverp.contains(eOverPBin+"_mean")){
            binnedParameters->setFudgeMean(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,json_eoverp[eOverPBin+"_mean"]);
          }
          if (json_eoverp.contains(eOverPBin+"_sigma")){
            binnedParameters->setFudgeStdDev(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,json_eoverp[eOverPBin+"_sigma"]);
          }
          for (auto thisLayerBin : m_caloLayerBins){
            eflowCalo::LAYER thisLayerBinEnum = static_cast<eflowCalo::LAYER>(thisLayerBin);   
            std::string currentLayerBin = eflowCalo::name(thisLayerBinEnum);            
            std::string cellOrderingBin = eOverPBin + "_caloLayer_"+std::to_string(thisLayerBin);
            if (json_cellOrdering.contains(cellOrderingBin+"_norm1")){
              binnedParameters->setShapeParam(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,thisLayerBinEnum,NORM1,json_cellOrdering[cellOrderingBin+"_norm1"]);
            }
            if (json_cellOrdering.contains(cellOrderingBin+"_sigma1")){
              binnedParameters->setShapeParam(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,thisLayerBinEnum,WIDTH1,json_cellOrdering[cellOrderingBin+"_sigma1"]);
            }
            if (json_cellOrdering.contains(cellOrderingBin+"_norm2")){
              binnedParameters->setShapeParam(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,thisLayerBinEnum,NORM2,json_cellOrdering[cellOrderingBin+"_norm2"]);
            }
            if (json_cellOrdering.contains(cellOrderingBin+"_sigma2")){
              binnedParameters->setShapeParam(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,thisLayerBinEnum,WIDTH2,json_cellOrdering[cellOrderingBin+"_sigma2"]);
            }            
          }
        }
      }
    }    
  }

  return StatusCode::SUCCESS;

}

StatusCode PFCellEOverPTool::finalize(){
  return StatusCode::SUCCESS;
}
