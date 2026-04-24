/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "eflowEEtaBinnedParameters.h"
#include "PFCellEOverPTool.h"
#include "eflowCaloRegions.h"

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

  std::ifstream binBoundariesFile(path+"binBoundaries.json");
  nlohmann::json json_binBoundaries;
  binBoundariesFile >> json_binBoundaries;

  //whilst for calo layers and first interaction regions we want to lookup the full set of bin values
  auto fillBinValues = [](auto && binBoundaries, const nlohmann::json &json_binBoundaries, const std::string &binName){
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
  filledBins = fillBinValues(m_energyBinLowerBoundaries, json_binBoundaries, energyBinBoundaries);
  if (!filledBins) {
    ATH_MSG_ERROR("Could not bin boundaries in json file: " << energyBinBoundaries);
    return StatusCode::FAILURE;
  }

  //The e/p (mean, sigma) parameters are binned in track energy, track eta, calorimeter LHED and are derived using:
  //https://gitlab.cern.ch/atlas-jetetmiss/pflow/commontools/EOverPTools/-/blob/main/EoverpNtupleAnalysis/Run_EoverP_Comparison.py?ref_type=heads


  //The cell ordering parameters (norm1, sigma1, norm2, sigma2) are binned in the same way, and also in calo layer and are derived using:
  //https://gitlab.cern.ch/atlas-jetetmiss/pflow/commontools/EOverPTools/-/blob/main/EoverpNtupleAnalysis/CellOrdering_Cpp.cxx?ref_type=heads

  //The energy bins have a special treatment - if e.g if the last pair if boundaries are 40 and 100 GeV we measure e/p between those values, but we 
  //want to allow any track with e > 40 GeV to find an e/p reference value. Thus we remove the last bin boundary for the energy bins.

  m_energyBinLowerBoundaries.pop_back(); 

  //We have to be careful with the eta bins. In this tool we don't want to use the last boundary value, because in this tool
  //each eta bin value corresponds to the lower bin boundary and clearly you cannot move beyond the edge of the ID/ITK acceptance, 
  //so there is nothing to look up for the final bin. However in eflowEEtaBinnedParameters the same array is used to check the range
  //in which an eta value falls. Thus we keep the final eta bin boundary in the array, but we have to be careful when looping over eta 
  //bins to not try to look up values for the final bin boundary value.

  filledBins = fillBinValues(m_etaBinLowerBoundaries, json_binBoundaries, etaBinBoundaries);
  if (!filledBins) {
    ATH_MSG_ERROR("Could not bin boundaries in json file: " << etaBinBoundaries);
    return StatusCode::FAILURE;
  }

  //The final two arrays correspond to specific calorimeter layes to look up parameters for, so this is like a key to look up
  //rather than comparing a value to bin boundaries as is done for track energy and track eta.

  filledBins = fillBinValues(m_firstIntBinLowerBoundaries, json_binBoundaries, firstIntBinBoundaries);
  if (!filledBins) {
    ATH_MSG_ERROR("Could not bin boundaries in json file: " << firstIntBinBoundaries);
    return StatusCode::FAILURE;
  }
  filledBins = fillBinValues(m_caloLayerBins, json_binBoundaries, caloLayerBinBoundaries);
  if (!filledBins) {
    ATH_MSG_ERROR("Could not bin boundaries in json file: " << caloLayerBinBoundaries);
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("Energy bin boundaries: " << m_energyBinLowerBoundaries);
  ATH_MSG_DEBUG("Eta bin boundaries: " << m_etaBinLowerBoundaries);
  ATH_MSG_DEBUG("First interaction region bin boundaries: " << m_firstIntBinLowerBoundaries);
  ATH_MSG_DEBUG("Calo layer bin boundaries: " << m_caloLayerBins);

  return StatusCode::SUCCESS;
}

StatusCode PFCellEOverPTool::fillBinnedParameters(eflowEEtaBinnedParameters *binnedParameters) const {

  if (binnedParameters) {

    //The energy values in the json files are in GeV, but in athena we work in MeV
    //so the energy values used by binnedParameters are converted to MeV
    std::vector<double>  energyBinLowerBoundaries_GeV;
    for (auto energyBin : m_energyBinLowerBoundaries) energyBinLowerBoundaries_GeV.push_back(energyBin*Gaudi::Units::GeV);
    binnedParameters->initialise(energyBinLowerBoundaries_GeV, m_etaBinLowerBoundaries);

    std::string path;
    path = PathResolverFindCalibDirectory(m_referenceFileLocation);  

    std::ifstream inputFile_eoverp(path+"eOverP.json");
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
        if (thisEtaBin == m_etaBinLowerBoundaries.back()) continue; //the last eta bin boundary is just used to define the upper edge of the final eta bin, so we don't need to fill parameters for it
        etaBinCounter++;
        std::stringstream currentEtaBinStream;
        currentEtaBinStream << std::fixed << std::setprecision(1) << thisEtaBin;
        std::string currentEtaBin = currentEtaBinStream.str();
        for (auto thisFirstIntRegionBin_Int : m_firstIntBinLowerBoundaries){
          //enum version is used for the calls to binnedParameters APIs
      	  auto thisFirstIntRegionBin = static_cast<eflowFirstIntRegions::J1STLAYER>(thisFirstIntRegionBin_Int);
          //and string version is used to lookup values from the json file
      	  std::string currentFirstIntBin = std::to_string(thisFirstIntRegionBin_Int);
          std::string eOverPBin = "energyBinLowerBound_"+currentEBin+"_etaBinLowerBound_"+currentEtaBin+"_firstIntBinLowerBound_"+currentFirstIntBin;
          if (json_eoverp.contains(eOverPBin+"_mean")){     
            binnedParameters->setFudgeMean(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,json_eoverp[eOverPBin+"_mean"]);
            ATH_MSG_DEBUG("Setting mean for bin " << eOverPBin << " to " << json_eoverp[eOverPBin+"_mean"]);
          }
          //DEBUG because this is expected to happen for some combinations (e.g HEC calo layers for tracks in the barrel)
          else ATH_MSG_DEBUG("No mean found for bin " << eOverPBin);
          if (json_eoverp.contains(eOverPBin+"_sigma")){
            binnedParameters->setFudgeStdDev(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,json_eoverp[eOverPBin+"_sigma"]);
            ATH_MSG_DEBUG("Setting sigma for bin " << eOverPBin << " to " << json_eoverp[eOverPBin+"_sigma"]);
          }
          else ATH_MSG_DEBUG("No sigma found for bin " << eOverPBin);
          for (auto thisLayerBin : m_caloLayerBins){
            eflowCalo::LAYER thisLayerBinEnum = static_cast<eflowCalo::LAYER>(thisLayerBin);   
            std::string currentLayerBin = eflowCalo::name(thisLayerBinEnum);            
            std::string cellOrderingBin = eOverPBin + "_caloLayer_"+std::to_string(thisLayerBin);
            if (json_cellOrdering.contains(cellOrderingBin+"_norm1")){
              binnedParameters->setShapeParam(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,thisLayerBinEnum,NORM1,json_cellOrdering[cellOrderingBin+"_norm1"]);
              ATH_MSG_DEBUG("Setting norm1 for bin " << cellOrderingBin << " to " << json_cellOrdering[cellOrderingBin+"_norm1"]);
            }
            else ATH_MSG_DEBUG("No norm1 found for bin " << cellOrderingBin);
            if (json_cellOrdering.contains(cellOrderingBin+"_sigma1")){
              binnedParameters->setShapeParam(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,thisLayerBinEnum,WIDTH1,json_cellOrdering[cellOrderingBin+"_sigma1"]);
              ATH_MSG_DEBUG("Setting sigma1 for bin " << cellOrderingBin << " to " << json_cellOrdering[cellOrderingBin+"_sigma1"]);
            }
            else ATH_MSG_DEBUG("No sigma1 found for bin " << cellOrderingBin);
            if (json_cellOrdering.contains(cellOrderingBin+"_norm2")){
              binnedParameters->setShapeParam(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,thisLayerBinEnum,NORM2,json_cellOrdering[cellOrderingBin+"_norm2"]);
              ATH_MSG_DEBUG("Setting norm2 for bin " << cellOrderingBin << " to " << json_cellOrdering[cellOrderingBin+"_norm2"]);
            }
            else ATH_MSG_DEBUG("No norm2 found for bin " << cellOrderingBin);
            if (json_cellOrdering.contains(cellOrderingBin+"_sigma2")){
              binnedParameters->setShapeParam(energyBinCounter,etaBinCounter,thisFirstIntRegionBin,thisLayerBinEnum,WIDTH2,json_cellOrdering[cellOrderingBin+"_sigma2"]);
              ATH_MSG_DEBUG("Setting sigma2 for bin " << cellOrderingBin << " to " << json_cellOrdering[cellOrderingBin+"_sigma2"]);
            }            
            else ATH_MSG_DEBUG("No sigma2 found for bin " << cellOrderingBin);
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
