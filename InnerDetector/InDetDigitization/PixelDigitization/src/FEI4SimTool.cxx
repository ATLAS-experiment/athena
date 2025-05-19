/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#include "FEI4SimTool.h"
#include "PixelDigitizationUtilities.h"

#include "PixelConditionsData/ChargeCalibParameters.h" //for Thresholds
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "InDetRawData/Pixel1RawData.h"
#include "InDetRawData/PixelRDO_Collection.h"

#include "SiDigitization/SiChargedDiodeCollection.h"
#include "Identifier/IdentifierHash.h"
#include "InDetIdentifier/PixelID.h"
//
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"
#include "PixelNoiseFunctions.h"
#include <array>
#include <algorithm>


using namespace PixelDigitization;



namespace{
  double
  getSigma(int tot){
    constexpr std::array<double,17> sigmas{0.0,0.50,0.50,0.50,0.50,0.50,0.60,0.60,0.60,0.60,0.65,0.70,0.75,0.80,0.80,0.80,0.80};
    return sigmas.at(tot);
  }

}



FEI4SimTool::FEI4SimTool(const std::string& type, const std::string& name, const IInterface* parent) :
  FrontEndSimTool(type, name, parent) {
}

FEI4SimTool::~FEI4SimTool() = default;

StatusCode FEI4SimTool::initialize() {
  ATH_CHECK(FrontEndSimTool::initialize());
  ATH_MSG_DEBUG("FEI4SimTool::initialize()");
  ATH_CHECK(m_moduleDataKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode FEI4SimTool::finalize() {
  ATH_MSG_DEBUG("FEI4SimTool::finalize()");
  return StatusCode::SUCCESS;
}

void FEI4SimTool::process(SiChargedDiodeCollection& chargedDiodes, PixelRDO_Collection& rdoCollection,
                          CLHEP::HepRandomEngine* rndmEngine) const {
  const InDetDD::PixelModuleDesign* p_design =
    static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());

  if (p_design->getReadoutTechnology() != InDetDD::PixelReadoutTechnology::FEI4) {
    return;
  }

  const PixelID* pixelId = static_cast<const PixelID*>(chargedDiodes.element()->getIdHelper());
  const IdentifierHash moduleHash = pixelId->wafer_hash(chargedDiodes.identify()); // wafer hash
  Identifier moduleID = pixelId->wafer_id(chargedDiodes.element()->identify());

  int barrel_ec = pixelId->barrel_ec(chargedDiodes.element()->identify());
  int layerIndex = pixelId->layer_disk(chargedDiodes.element()->identify());

  if (std::abs(barrel_ec) != m_BarrelEC) {
    return;
  }

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  SG::ReadCondHandle<PixelModuleData> moduleDataHandle(m_moduleDataKey, ctx);
  const PixelModuleData *moduleData = *moduleDataHandle;
  SG::ReadCondHandle<PixelChargeCalibCondData> calibDataHandle(m_chargeDataKey, ctx);
  const PixelChargeCalibCondData *calibData = *calibDataHandle;

  int maxFEI4SmallHit = 2;
  int overflowToT = calibData->getFEI4OverflowToT();

  std::vector<std::unique_ptr<Pixel1RawData>> p_rdo_small_fei4;
  int nSmallHitsFEI4 = 0;
  std::vector<int> row, col;
  const int maxRow = p_design->rowsPerCircuit();
  const int maxCol = p_design->columnsPerCircuit();
  std::vector<std::vector<int> > FEI4Map(maxRow + 16, std::vector<int>(maxCol + 16));

  // Add cross-talk
  crossTalk(moduleData->getCrossTalk(barrel_ec, layerIndex), chargedDiodes);

  if (m_doNoise) {
    // Add thermal noise
    thermalNoise(m_thermalNoise, chargedDiodes, rndmEngine);

    // Add random noise
    randomNoise(chargedDiodes, moduleData, m_numberOfBcid, calibData, rndmEngine, m_pixelReadout.get());
  }

  // Add random diabled pixels
  randomDisable(chargedDiodes, moduleData, rndmEngine); // FIXME How should we handle disabling pixels in Overlay jobs?

  for (SiChargedDiodeOrderedIterator i_chargedDiode = chargedDiodes.orderedBegin();
       i_chargedDiode != chargedDiodes.orderedEnd(); ++i_chargedDiode) {
    SiChargedDiode& diode = **i_chargedDiode;

    Identifier diodeID = chargedDiodes.getId(diode.diode());
    double charge = diode.charge();

    // charge scaling function applied. (Reference: ATL-COM-INDET-2018-052)
 
    double corrQ = 1.11 *
                   (1.0 - (-7.09 * 1000.0) / (23.72 * 1000.0 + charge) + (-0.22 * 1000.0) /
                    (-0.42 * 1000.0 + charge));
    if (corrQ < 1.0) {
      corrQ = 1.0;
    }
    charge *= 1.0 / corrQ;
  
    //could scale if necessary

    unsigned int FE = m_pixelReadout->getFE(diodeID, moduleID);
    InDetDD::PixelDiodeType type = m_pixelReadout->getDiodeType(diodeID);
    if ((FE == InDetDD::invalidFrontEnd) or (type == InDetDD::PixelDiodeType::NONE)) continue;//invalid frontend

    // Apply analog threshold, timing simulation
    const auto & thresholds = calibData->getThresholds(type, moduleHash, FE);
    double threshold =  PixelDigitization::randomThreshold(thresholds, rndmEngine);
                       // This noise check is unaffected by digitizationFlags.doInDetNoise in
                       // 21.0 - see PixelCellDiscriminator.cxx in that branch

    if (charge > threshold) {
      int bunchSim;
      if (diode.totalCharge().fromTrack()) {
        bunchSim =
          static_cast<int>(std::floor((getG4Time(diode.totalCharge()) +
                                  m_timeOffset) / m_bunchSpace));
      } else {
        bunchSim = CLHEP::RandFlat::shootInt(rndmEngine, m_numberOfBcid);
      }

      if (bunchSim < 0 || bunchSim > m_numberOfBcid) {
        SiHelper::belowThreshold(diode, true, true);
      } else {
        SiHelper::SetBunch(diode, bunchSim);
      }
    } else {
      SiHelper::belowThreshold(diode, true, true);
    }

    // charge to ToT conversion
    double tot = calibData->getToT(type, moduleHash, FE, charge);
    double totsig = calibData->getTotRes(moduleHash, FE, tot);
    int nToT = static_cast<int>(CLHEP::RandGaussZiggurat::shoot(rndmEngine, tot, totsig));

    // This is for new IBL calibration, since above method (stat_cast) is not effective.
    if (totsig==0.0) {
      double totIBLsig = getSigma(nToT);
      if (totIBLsig) {
        if (CLHEP::RandFlat::shoot(rndmEngine,0.0,1.0)<std::exp(-0.5/totIBLsig/totIBLsig)) {
          if (CLHEP::RandFlat::shoot(rndmEngine,0.0,1.0)<0.5) { nToT--; }
          else                                                { nToT++; }
        }
      }
    } 

   

    // FEI4 HitDiscConfig
    if (nToT == 2 && maxFEI4SmallHit == 2) {
      nToT = 1;
    }
    
    nToT=std::clamp(nToT, 1, overflowToT);

    if (nToT <= moduleData->getToTThreshold(barrel_ec, layerIndex)) {
      SiHelper::belowThreshold(diode, true, true);
    }

    // Filter events
    if (SiHelper::isMaskOut(diode)) {
      continue;
    }
    if (SiHelper::isDisabled(diode)) {
      continue;
    }

    if (!m_pixelConditionsTool->isActive(moduleHash, diodeID, ctx)) {
      SiHelper::disabled(diode, true, true);
      continue;
    }

    int flag = diode.flag();
    int bunch = (flag >> 8) & 0xff;

    InDetDD::SiReadoutCellId cellId = diode.getReadoutCell();
    const Identifier id_readout = chargedDiodes.element()->identifierFromCellId(cellId);

    int iirow = cellId.phiIndex();
    int iicol = cellId.etaIndex();
    if (iicol >= maxCol) {
      iicol = iicol - maxCol;
    } // FEI4 copy mechanism works per FE.

    // Front-End simulation
    if (bunch >= 0 && bunch < m_numberOfBcid) {
      auto p_rdo = std::make_unique<Pixel1RawData>(id_readout, nToT, bunch, 0, bunch);
      if (nToT > maxFEI4SmallHit) {
        rdoCollection.push_back(p_rdo.release());
        FEI4Map[iirow][iicol] = 2; //Flag for "big hits"
      } else {
        p_rdo_small_fei4.push_back(std::move(p_rdo));
        row.push_back(iirow);
        col.push_back(iicol);
        FEI4Map[iirow][iicol] = 1; //Flag for low hits
        nSmallHitsFEI4++;
      }
    }
  }

  // Copy mechanism for IBL small hits:
  if (nSmallHitsFEI4 > 0) {
    bool recorded = false;

    //First case: Record small hits which are in the same Pixel Digital Region than a big hit:
    for (int ismall = 0; ismall < nSmallHitsFEI4; ismall++) {
      int rowPDR = row[ismall] / 2;
      int colPDR = col[ismall] / 2;
      for (int rowBigHit = 2 * rowPDR; rowBigHit != 2 * rowPDR + 2 && rowBigHit < maxRow; ++rowBigHit) {
        for (int colBigHit = 2 * colPDR; colBigHit != 2 * colPDR + 2 && colBigHit < maxCol; ++colBigHit) {
          ATH_MSG_DEBUG(
            "rowBig = " << rowBigHit << " colBig = " << colBigHit << " Map Content = " <<
              FEI4Map[rowBigHit][colBigHit]);
          if (FEI4Map[rowBigHit][colBigHit] == 2 && !recorded) {
            rdoCollection.push_back(p_rdo_small_fei4[ismall].release());
            recorded = true;
          }
        }
      }

      // Second case: Record small hits which are phi-neighbours with a big hit:
      if (!recorded && row[ismall] < maxRow - 1) {
        if (FEI4Map[row[ismall] + 1][col[ismall]] == 2) {
          rdoCollection.push_back(p_rdo_small_fei4[ismall].release());
          recorded = true;
        }
      }
      if (!recorded && row[ismall] != 0) {
        if (FEI4Map[row[ismall] - 1][col[ismall]] == 2) {
          rdoCollection.push_back(p_rdo_small_fei4[ismall].release());
          recorded = true;
        }
      }
    }
  }
  }
