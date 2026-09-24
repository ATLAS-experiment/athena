/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelNoiseFunctions.h"

#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandPoisson.h"

#include "SiDigitization/SiChargedDiodeCollection.h"
#include "InDetSimEvent/SiTotalCharge.h"

#include "SiDigitization/SiHelper.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "PixelConditionsData/PixelModuleData.h"  
#include "PixelConditionsData/PixelChargeCalibCondData.h"  
#include "PixelConditionsData/ITkPixSimulationParameters.h" 
#include "InDetIdentifier/PixelID.h"

#include <limits>


namespace PixelDigitization{
  void 
  crossTalk(double crossTalk, SiChargedDiodeCollection& chargedDiodes){
    const InDetDD::PixelModuleDesign* p_design =
      static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());
    SiChargedDiodeMap oldChargedDiodes = chargedDiodes.chargedDiodes();

    for (SiChargedDiodeIterator i_chargedDiode = oldChargedDiodes.begin(); i_chargedDiode != oldChargedDiodes.end();
         ++i_chargedDiode) {
      InDetDD::SiCellId diode = (*i_chargedDiode).second.diode();
      std::vector<InDetDD::SiCellId> neighbours;
      p_design->neighboursOfCell(diode, neighbours);
      for (std::vector<InDetDD::SiCellId>::const_iterator p_neighbour = neighbours.begin();
           p_neighbour != neighbours.end(); ++p_neighbour) {
        const double intersection = p_design->intersectionLength(diode, *p_neighbour);
        // add cross talk only if the intersection is non-zero
        // if the original pixel is at (col,row) then the intersection length is
        // (col+-1, row+-1) : 0 -> diagonal
        // (col   , row+-1) : 0.4 mm (or 0.6 if long pixel) pixel width  = 400um or 600um
        // (col+-1, row   ) : 0.05 mm , pixel height = 50um
        // intersection length is just the length of the contact surface between the two pixels
        if (intersection > 0) {
          // create a new charge:
          // Q(new) = Q*L*X
          //   Q = charge of source pixel
          //   L = intersection length [mm]
          //   X = crosstalk factor    [mm-1]
          const SiChargedDiode& siCharge = (*i_chargedDiode).second;
          SiCharge charge(siCharge.charge() * intersection* crossTalk,
                          siCharge.totalCharge().time(), SiCharge::diodeX_Talk, siCharge.totalCharge().particleLink());
          chargedDiodes.add(*p_neighbour, charge);
        }
      }
    }
    return;
  }

  void 
  thermalNoise(double thermalNoise, SiChargedDiodeCollection& chargedDiodes,
    CLHEP::HepRandomEngine* rndmEngine) {
    for (SiChargedDiodeOrderedIterator i_chargedDiode = chargedDiodes.orderedBegin();
         i_chargedDiode != chargedDiodes.orderedEnd(); ++i_chargedDiode) {
      SiCharge charge(thermalNoise * CLHEP::RandGaussZiggurat::shoot(rndmEngine), 0, SiCharge::noise);
      (*i_chargedDiode)->add(charge);
    }
    return;
  }


  void 
  randomNoise(SiChargedDiodeCollection& chargedDiodes, const PixelModuleData *moduleData,
    int nBcid,
    const PixelChargeCalibCondData *chargeCalibData, CLHEP::HepRandomEngine* rndmEngine, 
    InDetDD::IPixelReadoutManager * pixelReadout) {
    const InDetDD::PixelModuleDesign* p_design =
      static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());

    const PixelID* pixelId = static_cast<const PixelID*>(chargedDiodes.element()->getIdHelper());
    int barrel_ec = pixelId->barrel_ec(chargedDiodes.element()->identify());
    int layerIndex = pixelId->layer_disk(chargedDiodes.element()->identify());
    const double totalNoiseOccupancy = moduleData->getNoiseOccupancy(barrel_ec,layerIndex) * nBcid;
    //prepare to enter loop
    const std::vector<float> &noiseShape = moduleData->getNoiseShape(barrel_ec, layerIndex);
    const auto technology = p_design->getReadoutTechnology();
    // protection to the overflow ToT, that depends on the sensor technology
    float overflowToT = std::numeric_limits<float>::max();
    if (technology == InDetDD::PixelReadoutTechnology::FEI4) {
      overflowToT = chargeCalibData->getFEI4OverflowToT();
    } else if (technology == InDetDD::PixelReadoutTechnology::FEI3) {
      overflowToT = moduleData->getFEI3Latency(barrel_ec, layerIndex);
    }
    return randomNoise(chargedDiodes, totalNoiseOccupancy, noiseShape, overflowToT, chargeCalibData, rndmEngine, pixelReadout);
  }
  
  void 
  ITkRandomNoise(SiChargedDiodeCollection& chargedDiodes, const ITkPixSimulationParameters & chipData,
    int nBcid,
    const PixelChargeCalibCondData *chargeCalibData, CLHEP::HepRandomEngine* rndmEngine, 
    InDetDD::IPixelReadoutManager * pixelReadout) {
    const double totalNoiseOccupancy = chipData.noiseOccupancy() * nBcid;
    //prepare to enter loop
    const std::vector<float> &noiseShape = chipData.noiseShape();
    // protection to the overflow ToT, that depends on the sensor technology
    float overflowToT = std::numeric_limits<float>::max();
    return ITkRandomNoise(chargedDiodes, totalNoiseOccupancy, noiseShape, overflowToT, chargeCalibData, rndmEngine, pixelReadout);
  }



  void 
  randomNoise(SiChargedDiodeCollection& chargedDiodes, const double totalNoiseOccupancy, const std::vector<float> &noiseShape, float overflowToT,
              const PixelChargeCalibCondData *chargeCalibData, CLHEP::HepRandomEngine* rndmEngine, InDetDD::IPixelReadoutManager * pixelReadout){
   const InDetDD::PixelModuleDesign* p_design =
      static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());

    const PixelID* pixelId = static_cast<const PixelID*>(chargedDiodes.element()->getIdHelper());
    const IdentifierHash moduleHash = pixelId->wafer_hash(chargedDiodes.identify()); // wafer hash
    const auto nCircuits = p_design->numberOfCircuits();
    const auto nColumns = p_design->columnsPerCircuit();
    const auto nRows = p_design->rowsPerCircuit();
    const auto totalCells = nCircuits * nColumns * nRows;
    int nNoise = CLHEP::RandPoisson::shoot(rndmEngine, totalCells * totalNoiseOccupancy);
    //prepare to enter loop
    const auto technology = p_design->getReadoutTechnology();
    for (int i = 0; i < nNoise; i++) {
      int circuit = CLHEP::RandFlat::shootInt(rndmEngine, nCircuits);
      int column = CLHEP::RandFlat::shootInt(rndmEngine, nColumns);
      int row = CLHEP::RandFlat::shootInt(rndmEngine, nRows);
      if (row > 159 && technology == InDetDD::PixelReadoutTechnology::FEI3) {
        row += 8;
      } // jump over ganged pixels - rowsPerCircuit == 320 above

      InDetDD::SiReadoutCellId roCell(row, nColumns * circuit + column);
      Identifier noisyID = chargedDiodes.element()->identifierFromCellId(roCell);

      if (roCell.isValid()) {
        InDetDD::SiCellId diodeNoise = roCell;
        float x = static_cast<float>(CLHEP::RandFlat::shoot(rndmEngine, 0., 1.));  // returns double
        size_t bin{};
        for (size_t j = 1; j < noiseShape.size(); j++) {
          if (x > noiseShape[j - 1] && x <= noiseShape[j]) {
            bin = j - 1;
            break;
          }
        }
        float noiseToTm = bin + 1.5f;
        float noiseToT = CLHEP::RandGaussZiggurat::shoot(rndmEngine, noiseToTm, 1.f);
        if (noiseToT < 1.f) { continue; }  // throw away unphysical noise
        noiseToT = std::min(noiseToT, overflowToT);
        InDetDD::PixelDiodeType type = pixelReadout->getDiodeType(noisyID);
        if (type == InDetDD::PixelDiodeType::NONE) continue;
        float chargeShape = chargeCalibData->getCharge(type, moduleHash, circuit, noiseToT);
        chargedDiodes.add(diodeNoise, SiCharge(chargeShape, 0, SiCharge::noise));
      }
    }
    return;
  }



  void 
  ITkRandomNoise(SiChargedDiodeCollection& chargedDiodes, const double totalNoiseOccupancy, const std::vector<float> &noiseShape, float overflowToT,
              const PixelChargeCalibCondData *chargeCalibData, CLHEP::HepRandomEngine* rndmEngine, InDetDD::IPixelReadoutManager * pixelReadout){
   const InDetDD::PixelModuleDesign* p_design =
      static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());

    const PixelID* pixelId = static_cast<const PixelID*>(chargedDiodes.element()->getIdHelper());
    const IdentifierHash moduleHash = pixelId->wafer_hash(chargedDiodes.identify()); // wafer hash
    const auto nCircuits = p_design->numberOfCircuits();
    const auto nColumns = p_design->columnsPerCircuit();
    const auto nRows = p_design->rowsPerCircuit();
    const auto totalCells = nCircuits * nColumns * nRows;
    int nNoise = CLHEP::RandPoisson::shoot(rndmEngine, totalCells * totalNoiseOccupancy);

    //my tests
    const auto nCircuitsPerCol = p_design->numberOfCircuitsPerColumn();
    const auto nCircuitsPerRow = p_design->numberOfCircuitsPerRow();
    //std::cout << "CircuitsPerCol = " << CircuitsPerCol << std::endl;
    //std::cout << "CircuitsPerRow = " << CircuitsPerRow << std::endl;

    //prepare to enter loop
    const auto technology = p_design->getReadoutTechnology();
    for (int i = 0; i < nNoise; i++) {
      int circuit = CLHEP::RandFlat::shootInt(rndmEngine, nCircuits);
      int column = CLHEP::RandFlat::shootInt(rndmEngine, nColumns);
      int CircuitsPerCol = CLHEP::RandFlat::shootInt(rndmEngine, nCircuitsPerCol);
      int CircuitsPerRow = CLHEP::RandFlat::shootInt(rndmEngine, nCircuitsPerRow); 
      int row = CLHEP::RandFlat::shootInt(rndmEngine, nRows);
      if (row > 159 && technology == InDetDD::PixelReadoutTechnology::FEI3) {
        row += 8;
      } // jump over ganged pixels - rowsPerCircuit == 320 above
      

      //InDetDD::SiReadoutCellId roCell =
      //  (circuit > 1)
      //    ? InDetDD::SiReadoutCellId(nRows + row, nColumns * (circuit - 2) + column)
      //    : InDetDD::SiReadoutCellId(row, nColumns * circuit + column);

      //InDetDD::SiReadoutCellId roCell = InDetDD::SiReadoutCellId(row + nRows*(CircuitsPerCol), column + nColumns*(CircuitsPerRow));
      InDetDD::SiReadoutCellId roCell = InDetDD::SiReadoutCellId(row, column + nColumns*(CircuitsPerRow));
     


      //InDetDD::SiReadoutCellId roCell(row, nColumns * circuit + column);
      Identifier noisyID = chargedDiodes.element()->identifierFromCellId(roCell);
      
      if (roCell.isValid()) {
        InDetDD::SiCellId diodeNoise = roCell;
        float x = static_cast<float>(CLHEP::RandFlat::shoot(rndmEngine, 0., 1.));  // returns double
        size_t bin{};
        for (size_t j = 1; j < noiseShape.size(); j++) {
          if (x > noiseShape[j - 1] && x <= noiseShape[j]) {
            bin = j - 1;
            break;
          }
        }
        float noiseToTm = bin + 1.5f;
        float noiseToT = CLHEP::RandGaussZiggurat::shoot(rndmEngine, noiseToTm, 1.f);

        if (noiseToT < 1.f) { continue; }  // throw away unphysical noise
        noiseToT = std::min(noiseToT, overflowToT);
        InDetDD::PixelDiodeType type = pixelReadout->getDiodeType(noisyID);
        if (type == InDetDD::PixelDiodeType::NONE) continue;
        float chargeShape = chargeCalibData->getCharge(type, moduleHash, circuit, noiseToT);
        chargedDiodes.add(diodeNoise, SiCharge(chargeShape, 0, SiCharge::noise));
      }
    }
    return;
  }

  void 
  randomDisable(SiChargedDiodeCollection& chargedDiodes,const PixelModuleData *moduleData,
    CLHEP::HepRandomEngine* rndmEngine) {
    const PixelID* pixelId = static_cast<const PixelID*>(chargedDiodes.element()->getIdHelper());
    const int barrel_ec = pixelId->barrel_ec(chargedDiodes.element()->identify());
    const int layerIndex = pixelId->layer_disk(chargedDiodes.element()->identify());
    const double disableProbability = moduleData->getDisableProbability(barrel_ec, layerIndex);
    return randomDisable(chargedDiodes, disableProbability, rndmEngine);
  }
  void 
  randomDisable(SiChargedDiodeCollection& chargedDiodes,const ITkPixSimulationParameters & chipData,
    CLHEP::HepRandomEngine* rndmEngine) {
    const double disableProbability = chipData.disableProbability();
    return randomDisable(chargedDiodes, disableProbability, rndmEngine);
  }

  void 
  randomDisable(SiChargedDiodeCollection& chargedDiodes, double disableProbability, 
    CLHEP::HepRandomEngine* rndmEngine){
    for (SiChargedDiodeOrderedIterator i_chargedDiode = chargedDiodes.orderedBegin();
         i_chargedDiode != chargedDiodes.orderedEnd(); ++i_chargedDiode) {
      if (CLHEP::RandFlat::shoot(rndmEngine) < disableProbability) {
        SiHelper::disabled(**i_chargedDiode, true, false);
      }
    }
    return;
  }
  
  int 
  generateToT(CLHEP::HepRandomEngine* rndmEngine, double mean, double sd, const std::pair<int, int>& range){
    int nToT = static_cast<int>(CLHEP::RandGaussZiggurat::shoot(rndmEngine, mean, sd));
    const auto &[lo,hi] = range;
    //
    if (nToT < lo) {
      nToT = lo;
    }
    if (nToT >= hi) {
      nToT = hi;
    }
    return nToT;
  }

  double 
  getG4Time(const SiTotalCharge& totalCharge) {
    // If there is one single charge, return its time:
    if (totalCharge.chargeComposition().empty()) {
      return totalCharge.time();
    }
    auto p_charge = totalCharge.chargeComposition().begin();
    int findfirst = 0;
    SiCharge first = *p_charge;
    // Look for first charge which is not noise
    for (; p_charge != totalCharge.chargeComposition().end(); p_charge++) {
      if (p_charge->processType() != SiCharge::noise) {
        findfirst = 1;
        break;
      }
    }
    // if all charges were noise, return the time of the highest charge
    if (findfirst == 0) {
      return totalCharge.time();
    }
    // look for the earliest charge among the remaining non-noise charges:
    first = *p_charge;
    p_charge++;
    for (; p_charge != totalCharge.chargeComposition().end(); p_charge++) {
      if (p_charge->time() < first.time() && p_charge->processType() != SiCharge::noise) {
        first = *p_charge;
      }
    }
    return first.time();
  }
}
