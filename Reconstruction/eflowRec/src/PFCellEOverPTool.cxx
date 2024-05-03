/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "eflowRec/eflowEEtaBinnedParameters.h"
#include "eflowRec/PFCellEOverPTool.h"
#include "eflowRec/eflowCaloRegions.h"

#include "GaudiKernel/SystemOfUnits.h"

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

  std::ifstream binBoundariesFile(m_referenceFileLocation+"EOverPBinBoundaries.json");
  nlohmann::json json_binBoundaries;
  binBoundariesFile >> json_binBoundaries;

  auto fillBinBoundaries = [](auto && binBoundaries, const nlohmann::json &json_binBoundaries, const std::string &binName){
    if (json_binBoundaries.find(binName) != json_binBoundaries.end()){
      for (auto binBoundary : json_binBoundaries[binName]) binBoundaries.push_back(binBoundary);
    }
    else std::cout << "Did not find binName:" << binName << std::endl;
  };

  fillBinBoundaries(m_energyBinLowerBoundaries, json_binBoundaries, "energyBinBoundaries");
  fillBinBoundaries(m_etaBinLowerBoundaries, json_binBoundaries, "etaBinBoundaries");
  fillBinBoundaries(m_firstIntBinLowerBoundaries, json_binBoundaries, "firstIntBinBoundaries");
  fillBinBoundaries(m_caloLayerBins, json_binBoundaries, "caloLayerBinBoundaries");

  return StatusCode::SUCCESS;
}

StatusCode PFCellEOverPTool::fillBinnedParameters(eflowEEtaBinnedParameters *binnedParameters) const {

  if (binnedParameters) {

    binnedParameters->initialise(m_energyBinLowerBoundaries, m_etaBinLowerBoundaries);
             
    std::ifstream inputFile_eoverp(m_referenceFileLocation+"EoverP.json");
    nlohmann::json json_eoverp;
    inputFile_eoverp >> json_eoverp;  

    std::ifstream inputFile_cellOrdering(m_referenceFileLocation+"cellOrdering.json");
    nlohmann::json json_cellOrdering;
    inputFile_cellOrdering >> json_cellOrdering;

    //Loop over energy, eta and first int bins
    //For each combination we set the e/p mean and width from the json file values
    for (auto thisEBin : m_energyBinLowerBoundaries){
      std::string currentEBin = std::to_string(thisEBin);
      for (auto thisEtaBin : m_etaBinLowerBoundaries){
        std::string currentEtaBin = std::to_string(thisEtaBin);
        for (auto thisFirstIntRegionBin_Int : m_firstIntBinLowerBoundaries){
          auto thisFirstIntRegionBin = static_cast<eflowFirstIntRegions::J1STLAYER>(thisFirstIntRegionBin_Int);
          std::string currentFirstIntBin = eflowFirstIntRegions::name(thisFirstIntRegionBin);
          std::string eOverPBin = "energyBinLowerBound_"+currentEBin+"_etaBinLowerBound_"+currentEtaBin+"_firstIntBinLowerBound_"+currentFirstIntBin;
          if (json_eoverp.find(eOverPBin+"_mean") != json_eoverp.end()){
            binnedParameters->setFudgeMean(thisEBin,thisEtaBin,thisFirstIntRegionBin,json_eoverp[eOverPBin+"_mean"]);
          }
          if (json_eoverp.find(eOverPBin+"_sigma") != json_eoverp.end()){
            binnedParameters->setFudgeStdDev(thisEBin,thisEtaBin,thisFirstIntRegionBin,json_eoverp[eOverPBin+"_sigma"]);
          }
          for (auto thisLayerBin : m_caloLayerBins){
            eflowCalo::LAYER thisLayerBinEnum = static_cast<eflowCalo::LAYER>(thisLayerBin);   
            std::string currentLayerBin = eflowCalo::name(thisLayerBinEnum);            
            std::string cellOrderingBin = eOverPBin + "_caloLayer_"+currentLayerBin;
            if (json_cellOrdering.find(cellOrderingBin+"_norm1") != json_cellOrdering.end()){
              binnedParameters->setShapeParam(thisEBin,thisEtaBin,thisFirstIntRegionBin,thisLayerBinEnum,NORM1,json_cellOrdering[cellOrderingBin+"_norm1"]);
            }
            if (json_cellOrdering.find(cellOrderingBin+"_sigma1") != json_cellOrdering.end()){
              binnedParameters->setShapeParam(thisEBin,thisEtaBin,thisFirstIntRegionBin,thisLayerBinEnum,WIDTH1,json_cellOrdering[cellOrderingBin+"_sigma1"]);
            }
            if (json_cellOrdering.find(cellOrderingBin+"_norm2") != json_cellOrdering.end()){
              binnedParameters->setShapeParam(thisEBin,thisEtaBin,thisFirstIntRegionBin,thisLayerBinEnum,NORM2,json_cellOrdering[cellOrderingBin+"_norm2"]);
            }
            if (json_cellOrdering.find(cellOrderingBin+"_sigma2") != json_cellOrdering.end()){
              binnedParameters->setShapeParam(thisEBin,thisEtaBin,thisFirstIntRegionBin,thisLayerBinEnum,WIDTH2,json_cellOrdering[cellOrderingBin+"_sigma2"]);
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
