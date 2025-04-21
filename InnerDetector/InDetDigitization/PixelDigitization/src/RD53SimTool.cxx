/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "RD53SimTool.h"
#include "PixelDigitizationUtilities.h"
#include "PixelNoiseFunctions.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "PixelConditionsData/ChargeCalibParameters.h" //for Thresholds
#include "SiDigitization/SiChargedDiodeCollection.h"
#include "InDetRawData/PixelRDO_Collection.h"

#include "SiDigitization/SiHelper.h"
#include "ReadoutGeometryBase/SiReadoutCellId.h"
#include "InDetRawData/Pixel1RawData.h"
#include "CLHEP/Random/RandFlat.h"
#include "PixelNoiseFunctions.h"
#include <cmath>

using namespace PixelDigitization;

RD53SimTool::RD53SimTool(const std::string& type, const std::string& name, const IInterface* parent) :
  FrontEndSimTool(type, name, parent) {
}

RD53SimTool::~RD53SimTool() = default;

StatusCode RD53SimTool::initialize() {
  ATH_CHECK(FrontEndSimTool::initialize());
  ATH_MSG_DEBUG("RD53SimTool::initialize()");
  ATH_CHECK(m_moduleDataKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode RD53SimTool::finalize() {
  ATH_MSG_DEBUG("RD53SimTool::finalize()");
  return StatusCode::SUCCESS;
}

void RD53SimTool::process(SiChargedDiodeCollection& chargedDiodes, PixelRDO_Collection& rdoCollection,
                          CLHEP::HepRandomEngine* rndmEngine) {
  const InDetDD::PixelModuleDesign* p_design =
    static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());

  if (p_design->getReadoutTechnology() != InDetDD::PixelReadoutTechnology::RD53) {
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

  int overflowToT = 14; //for RD53 (aka ITkPixV2) chip, not FEI4

  std::vector<Pixel1RawData*> p_rdo_small_fei4;
  std::vector<int> row, col;
  

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

  for (auto &[mapId,mapDiode]:chargedDiodes) {//cannot be const ref, mapDiode will be altered
    Identifier diodeID = chargedDiodes.getId(mapId);
    double charge = mapDiode.charge();
    unsigned int FE = m_pixelReadout->getFE(diodeID, moduleID);
    InDetDD::PixelDiodeType type = m_pixelReadout->getDiodeType(diodeID);
    if ((FE == InDetDD::invalidFrontEnd) or (type == InDetDD::PixelDiodeType::NONE)){
      SiHelper::disabled(mapDiode, true, true);
      continue;//invalid frontend
    } 
    // Apply analogue threshold, timing simulation
    const auto &thresholds = calibData->getThresholds(type, moduleHash, FE);
    double threshold =  PixelDigitization::randomThreshold(thresholds, rndmEngine); 
    // This noise check is unaffected by digitizationFlags.doInDetNoise in 21.0 - see PixelCellDiscriminator.cxx in that branch

    if (charge > threshold) {
      int bunchSim = 0;
      if (mapDiode.totalCharge().fromTrack()) {
        bunchSim = static_cast<int>(std::floor((getG4Time(mapDiode.totalCharge()) + m_timeOffset) / m_bunchSpace));
        //Timewalk implementation 
        if(m_doTimeWalk){
          if(charge < (threshold + m_overDrive)){
            const int timeWalk = 25; // Here it is assumed that the maximum value of timewalk is one bunch crossing (25ns)
            bunchSim = static_cast<int>(std::floor((getG4Time(mapDiode.totalCharge()) + m_timeOffset + timeWalk) / m_bunchSpace));
          } 
        }
      } else {
        bunchSim = CLHEP::RandFlat::shootInt(rndmEngine, m_numberOfBcid);
      }

      if (bunchSim < 0 || bunchSim > m_numberOfBcid) {
        SiHelper::belowThreshold(mapDiode, true, true);
      } else {
        SiHelper::SetBunch(mapDiode, bunchSim);
      }
    } else {
      SiHelper::belowThreshold(mapDiode, true, true);
    }

    // charge to ToT conversion
    double tot = calibData->getToT(type, moduleHash, FE, charge);
    double totsig = calibData->getTotRes(moduleHash, FE, tot);
    int nToT = generateToT(rndmEngine, tot,totsig, std::make_pair(1,overflowToT));

    if (nToT <= moduleData->getToTThreshold(barrel_ec, layerIndex)) {
      SiHelper::belowThreshold(mapDiode, true, true);
    }

    // Filter events
    if (SiHelper::isMaskOut(mapDiode)) {
      continue;
    }
    if (SiHelper::isDisabled(mapDiode)) {
      continue;
    }

    if (!m_pixelConditionsTool->isActive(moduleHash, diodeID, ctx)) {
      SiHelper::disabled(mapDiode, true, true);
      continue;
    }

    int flag = mapDiode.flag();
    int bunch = (flag >> 8) & 0xff;

    InDetDD::SiReadoutCellId cellId = mapDiode.getReadoutCell();
    const Identifier id_readout = chargedDiodes.element()->identifierFromCellId(cellId);

    // Front-End simulation
    if (bunch >= 0 && bunch < m_numberOfBcid) {
      Pixel1RawData* p_rdo = new Pixel1RawData(id_readout, nToT, bunch, 0, bunch);
      rdoCollection.push_back(p_rdo);
      p_rdo = nullptr;
    }
  }
}
