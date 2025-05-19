/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#include "FEI3SimTool.h"
#include "PixelDigitizationUtilities.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelConditionsData/ChargeCalibParameters.h" //for Thresholds
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "SiDigitization/SiChargedDiodeCollection.h"
#include "InDetRawData/PixelRDO_Collection.h"
#include "InDetRawData/Pixel1RawData.h"
#include "InDetIdentifier/PixelID.h"
//
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"

#include <cmath>
#include "PixelNoiseFunctions.h"

using namespace PixelDigitization;


FEI3SimTool::FEI3SimTool(const std::string& type, const std::string& name, const IInterface* parent) :
  FrontEndSimTool(type, name, parent) {
}

FEI3SimTool::~FEI3SimTool() = default;

StatusCode FEI3SimTool::initialize() {
  ATH_CHECK(FrontEndSimTool::initialize());
  ATH_MSG_DEBUG("FEI3SimTool::initialize()");
  ATH_CHECK(m_moduleDataKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode FEI3SimTool::finalize() {
  ATH_MSG_DEBUG("FEI3SimTool::finalize()");
  return StatusCode::SUCCESS;
}

void FEI3SimTool::process(SiChargedDiodeCollection& chargedDiodes, PixelRDO_Collection& rdoCollection,
                          CLHEP::HepRandomEngine* rndmEngine) const {
  const InDetDD::PixelModuleDesign* p_design =
    static_cast<const InDetDD::PixelModuleDesign*>(&(chargedDiodes.element())->design());

  if (p_design->getReadoutTechnology() != InDetDD::PixelReadoutTechnology::FEI3) {
    return;
  }

  const PixelID* pixelId = static_cast<const PixelID*>(chargedDiodes.element()->getIdHelper());
  const IdentifierHash moduleHash = pixelId->wafer_hash(chargedDiodes.identify()); // wafer hash
  Identifier moduleID = pixelId->wafer_id(chargedDiodes.element()->identify());

  int barrel_ec = pixelId->barrel_ec(chargedDiodes.element()->identify());
  int layerIndex = pixelId->layer_disk(chargedDiodes.element()->identify());
  int moduleIndex = pixelId->eta_module(chargedDiodes.element()->identify());

  if (std::abs(barrel_ec) != m_BarrelEC) {
    return;
  }

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  SG::ReadCondHandle<PixelModuleData> moduleDataHandle(m_moduleDataKey, ctx);
  const PixelModuleData *moduleData = *moduleDataHandle;
  SG::ReadCondHandle<PixelChargeCalibCondData> calibDataHandle(m_chargeDataKey, ctx);
  const PixelChargeCalibCondData *calibData = *calibDataHandle;
  const auto selectedTuneYear = moduleData->getFEI3TimingSimTune(barrel_ec, layerIndex);
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
  const InDetDD::SiDetectorElement * siDetEl = static_cast<const InDetDD::SiDetectorElement *>(chargedDiodes.element());
  for (auto &[mapId,mapDiode]:chargedDiodes) {
    // Merge ganged pixel
    InDetDD::SiCellId cellID = chargedDiodes.element()->cellIdFromIdentifier(chargedDiodes.getId( mapId));
    InDetDD::SiCellId gangedCell = siDetEl->gangedCell(cellID);
    Identifier gangedID = chargedDiodes.element()->identifierFromCellId(gangedCell);
    if (gangedCell.isValid()) {
      SiChargedDiode* gangedChargeDiode = chargedDiodes.find(gangedID);
      int phiGanged = pixelId->phi_index(gangedID);
      int phiThis = pixelId->phi_index(chargedDiodes.getId( mapId));

      if (gangedChargeDiode) { // merge charges
        bool maskGanged = ((phiGanged > 159) && (phiGanged < 168));
        bool maskThis = ((phiThis > 159) && (phiThis < 168));
        // mask the one ganged pixel that does not correspond to the readout electronics.
        // not really sure this is needed
        if (maskGanged && maskThis) {
          ATH_MSG_ERROR("FEI3SimTool: both ganged pixels are in the mask out region -> BUG!");
        }
        if (maskGanged) {
          mapDiode.add(gangedChargeDiode->totalCharge()); // merged org pixel
          SiHelper::maskOut(*gangedChargeDiode, true);
        } else {
          gangedChargeDiode->add(mapDiode.totalCharge()); // merged org pixel
          SiHelper::maskOut(mapDiode, true);
        }
      }
    }
  }

  for (SiChargedDiodeOrderedIterator i_chargedDiode = chargedDiodes.orderedBegin();
       i_chargedDiode != chargedDiodes.orderedEnd(); ++i_chargedDiode) {
    SiChargedDiode& diode = **i_chargedDiode;

    Identifier diodeID = chargedDiodes.getId(diode.diode());
    double charge = diode.charge();

    unsigned int FE = m_pixelReadout->getFE(diodeID, moduleID);
    InDetDD::PixelDiodeType type = m_pixelReadout->getDiodeType(diodeID);
    if ((FE == InDetDD::invalidFrontEnd) or (type == InDetDD::PixelDiodeType::NONE)) continue;//invalid frontend

    // charge to ToT conversion
    double tot = calibData->getToT(type, moduleHash, FE, charge);
    const auto thresholds = calibData->getThresholds(type, moduleHash, FE);
    // Apply analog threshold, timing simulation
    double th0 = thresholds.value;
    double ith0 = thresholds.inTimeValue;
    double threshold = PixelDigitization::randomThreshold(thresholds, rndmEngine);
    // This noise check is unaffected by digitizationFlags.doInDetNoise in
    // 21.0 - see PixelCellDiscriminator.cxx in that branch

    if (charge > threshold) {
      int bunchSim = 0;
      if (diode.totalCharge().fromTrack()) {
        const std::vector<float> & totCharges = moduleData->getTimingIndex(barrel_ec, layerIndex);
        const std::vector<float> & probArray  = moduleData->getTimingProbability(barrel_ec, layerIndex, moduleIndex);

        double prob = 0.0;
        if (selectedTuneYear==2023) { prob = getProbability(totCharges, probArray, tot); }
        if (selectedTuneYear==2022) { prob = getProbability(totCharges, probArray, tot); }
        if (selectedTuneYear==2018) { prob = getProbability(totCharges, probArray, diode.totalCharge().charge()); }
        if (selectedTuneYear==2015) { prob = getProbability(totCharges, probArray, diode.totalCharge().charge()); }

        double G4Time = getG4Time(diode.totalCharge());
        double rnd = CLHEP::RandFlat::shoot(rndmEngine, 0.0, 1.0);

        double timeWalk = 0.0;
        if (rnd<prob) { timeWalk = 25.0; }
        bunchSim = static_cast<int>(std::floor((G4Time+m_timeOffset+timeWalk)/m_bunchSpace));

        if (selectedTuneYear == 2009) {   // RUN1 procedure (based on 2007 cosmic data)
          double intimethreshold = (ith0 / th0) * threshold;
          bunchSim = relativeBunch2009(threshold, intimethreshold, diode.totalCharge(), rndmEngine);
        }
      } 
      else {
        if (moduleData->getFEI3TimingSimTune(barrel_ec, layerIndex) > 0) {
          bunchSim = CLHEP::RandFlat::shootInt(rndmEngine, m_numberOfBcid);
        }
      }

      if (bunchSim < 0 || bunchSim > m_numberOfBcid) {
        SiHelper::belowThreshold(diode, true, true);
      } else {
        SiHelper::SetBunch(diode, bunchSim);
      }
    } else {
      SiHelper::belowThreshold(diode, true, true);
    }

    double totsig = calibData->getTotRes(moduleHash, FE, tot);
    int nToT = static_cast<int>(CLHEP::RandGaussZiggurat::shoot(rndmEngine, tot, totsig));

    if (nToT < 1) {
      nToT = 1;
    }

    if (nToT <= moduleData->getToTThreshold(barrel_ec, layerIndex)) {
      SiHelper::belowThreshold(diode, true, true);
    }

    if (nToT >= moduleData->getFEI3Latency(barrel_ec, layerIndex)) {
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

    // Front-End simulation
    if (bunch >= 0 && bunch < m_numberOfBcid) {
      rdoCollection.push_back(new Pixel1RawData(id_readout, nToT, bunch, 0, bunch));
    }
    
    // Duplication mechanism for FEI3 small hits :
    if (m_duplication) {//is true for run1 only
      static constexpr int smallHitThreshold{7}; //constant for both barrel and endcap, never changes
      bool smallHitChk = false;
      if (nToT <= smallHitThreshold) {
        smallHitChk = true;
      }
      if (smallHitChk && bunch > 0 && bunch <= m_numberOfBcid) {
        rdoCollection.push_back(new Pixel1RawData(id_readout, nToT, bunch - 1, 0, bunch - 1));
      }
    }
  }
  }

int FEI3SimTool::relativeBunch2009(const double threshold, const double intimethreshold,
                                   const SiTotalCharge& totalCharge,
                                   CLHEP::HepRandomEngine* rndmEngine) const {
  int BCID = 0;
  double myTimeWalkEff = 0.;
  double overdrive = intimethreshold - threshold;

  //my TimeWalk computation through PARAMETRIZATION (by Francesco De Lorenzi - Milan)
  //double curvature  =  7.6e7*overdrive-2.64e10;
  //double divergence = -1.6*overdrive+942 ;
  //double myTimeWalk    = curvature/(pow((totalCharge.charge()-divergence),2.5));

  //my TimeWalk computation through PARAMETRIZATION from 2009 cosmic data (by I. Ibragimov and D. Miller)
  double p1 = 20. / std::log(intimethreshold / overdrive);
  double p0 = p1 * std::log(1. - threshold / 100000.);

  double myTimeWalk = -p0 - p1 * std::log(1. - threshold / totalCharge.charge());

  myTimeWalkEff = myTimeWalk + myTimeWalk * 0.2 * CLHEP::RandGaussZiggurat::shoot(rndmEngine);
  const double limit = m_timeJitter * 0.5;
  double randomJitter = CLHEP::RandFlat::shoot(rndmEngine, - limit, limit);

  //double G4Time	 = totalCharge.time();

  double G4Time = getG4Time(totalCharge);
  double timing = m_timeOffset + myTimeWalkEff + randomJitter + G4Time;
  BCID = static_cast<int>(std::floor(timing / m_bunchSpace));
  //ATH_MSG_DEBUG (  CTW << " , " << myTimeWalkEff << " , " << G4Time << " , " << timing << " , " << BCID );

  return BCID;
}

double FEI3SimTool::getProbability(const std::vector<float> &bounds, const std::vector<float> &probs, const double &val) const {
  auto pCategory = std::upper_bound(bounds.begin(), bounds.end(),val);
  if (pCategory == bounds.end()) return 0.0;
  auto idx = std::distance(bounds.begin(), pCategory);
  return probs[idx];
}


