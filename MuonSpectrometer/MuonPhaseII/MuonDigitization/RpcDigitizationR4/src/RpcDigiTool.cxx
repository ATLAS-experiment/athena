/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "RpcDigiTool.h"

#include "CLHEP/Random/RandGaussZiggurat.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "TruthUtils/HepMCHelpers.h"
#include "xAODMuonViews/ChamberViewer.h"

namespace {
constexpr double percentage(unsigned int numerator, unsigned int denom) {
  return 100. * numerator / std::max(denom, 1u);
}
} // namespace
namespace MuonR4 {

StatusCode RpcDigiTool::initialize() {
  ATH_CHECK(MuonDigitizationTool::initialize());
  ATH_CHECK(m_writeKey.initialize());
  ATH_CHECK(m_effiDataKey.initialize(!m_effiDataKey.empty()));
  m_stIdxBIL = m_idHelperSvc->rpcIdHelper().stationNameIndex("BIL");
  m_stIdxBIS = m_idHelperSvc->rpcIdHelper().stationNameIndex("BIS");
  return StatusCode::SUCCESS;
}

StatusCode RpcDigiTool::finalize() {
  ATH_MSG_INFO("Tried to convert "
               << m_allHits[0] << "/" << m_allHits[1] << " hits. In, "
               << percentage(m_acceptedHits[0], m_allHits[0]) << "/"
               << percentage(m_acceptedHits[1], m_allHits[1])
               << "% of the cases, the conversion was successful");
  return StatusCode::SUCCESS;
}

StatusCode
RpcDigiTool::digitize(const EventContext &ctx, const TimedHits &hitsToDigit,
                      xAOD::MuonSimHitContainer *sdoContainer) const {
  const RpcIdHelper &idHelper{m_idHelperSvc->rpcIdHelper()};
  // Prepare the temporary cache
  DigiCache digitCache{};
  /// Fetch the conditions for efficiency calculations
  const Muon::DigitEffiData *efficiencyMap{nullptr};
  ATH_CHECK(SG::get(efficiencyMap, m_effiDataKey, ctx));

  CLHEP::HepRandomEngine *rndEngine = getRandomEngine(ctx);
  xAOD::ChamberViewer viewer{hitsToDigit, m_idHelperSvc.get()};
  do {
    DeadTimeMap deadTimes{};
    for (const TimedHit &simHit : viewer) {
      if (m_digitizeMuonOnly && !MC::isMuon(simHit)) {
        continue;
      }
      const Identifier hitId{simHit->identify()};
      const int stName = m_idHelperSvc->stationName(hitId);
      RpcDigitCollection *digiColl = fetchCollection(hitId, digitCache);
      bool run4_BI = (stName == m_stIdxBIS &&
                      std::abs(m_idHelperSvc->stationEta(hitId)) < 7) ||
                     stName == m_stIdxBIL;
      if (!run4_BI) {
        /// Standard digitization path
        const bool digitizedEta = digitizeHit(simHit, false, efficiencyMap,
                                              *digiColl, rndEngine, deadTimes);

        const bool digitizedPhi = digitizeHit(simHit, true, efficiencyMap,
                                              *digiColl, rndEngine, deadTimes);

        if (digitizedEta || digitizedPhi) {
          xAOD::MuonSimHit *sdo = addSDO(simHit, sdoContainer);
          sdo->setIdentifier(digiColl->at(digiColl->size() - 1)->identify());
        }
      } else if (digitizeHitBI(simHit, efficiencyMap, *digiColl, rndEngine,
                               deadTimes)) {
        xAOD::MuonSimHit *sdo = addSDO(simHit, sdoContainer);
        sdo->setIdentifier(digiColl->at(digiColl->size() - 1)->identify());
      }
    }
  } while (viewer.next());
  /// Write everything at the end into the final digit container
  ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache),
                                idHelper.module_hash_max()));
  return StatusCode::SUCCESS;
}
bool RpcDigiTool::digitizeHit(const TimedHit &hit, const bool measuresPhi,
                              const Muon::DigitEffiData *effiMap,
                              RpcDigitCollection &outContainer,
                              CLHEP::HepRandomEngine *rndEngine,
                              DeadTimeMap &deadTimes) const {

  ++(m_allHits[measuresPhi]);

  const Identifier gasGapId = hit->identify();
  const MuonGMR4::RpcReadoutElement *reEle =
      m_detMgr->getRpcReadoutElement(gasGapId);

  const RpcIdHelper &idHelper{m_idHelperSvc->rpcIdHelper()};

  bool isValid{false};
  const Identifier layerId = idHelper.channelID(
      gasGapId, idHelper.doubletZ(gasGapId), idHelper.doubletPhi(gasGapId),
      idHelper.gasGap(gasGapId), measuresPhi, 1, isValid);

  const MuonGMR4::StripLayerPtr &layerDesign =
      reEle->sensorLayout(reEle->layerHash(layerId));

  const MuonGMR4::StripDesign &design{layerDesign->design(measuresPhi)};

  const double uncert = design.stripPitch() / std::sqrt(12.);
  Amg::Vector3D locHitPos{xAOD::toEigen(hit->localPosition())};
  locHitPos[measuresPhi] = CLHEP::RandGaussZiggurat::shoot(
      rndEngine, locHitPos[measuresPhi], uncert);

  const Amg::Vector2D locPos2D = layerDesign->to2D(locHitPos, measuresPhi);
  if (!design.insideTrapezoid(locPos2D)) {
    ATH_MSG_VERBOSE("The hit " << Amg::toString(locHitPos) << " / "
                               << Amg::toString(locPos2D)
                               << " is outside of the trapezoid bounds for "
                               << m_idHelperSvc->toString(layerId));
    return false;
  }
  const int strip = design.stripNumber(locPos2D);
  if (strip < 0) {
    ATH_MSG_VERBOSE("Hit " << Amg::toString(locHitPos) << " / "
                           << Amg::toString(locPos2D)
                           << " cannot trigger any signal in a strip for "
                           << m_idHelperSvc->toString(layerId) << std::endl
                           << design);
    return false;
  }

  const Identifier digitId{idHelper.channelID(
      gasGapId, idHelper.doubletZ(gasGapId), idHelper.doubletPhi(gasGapId),
      idHelper.gasGap(gasGapId), measuresPhi, strip, isValid)};

  if (!isValid) {
    ATH_MSG_WARNING("Invalid hit identifier obtained for "
                    << m_idHelperSvc->toStringGasGap(gasGapId)
                    << ",  eta strip " << strip << " & hit "
                    << Amg::toString(locHitPos, 2) << " /// " << design);
    return false;
  }
  /// Final check whether the digit is actually efficient
  if (effiMap && effiMap->getEfficiency(digitId) <
                     CLHEP::RandFlat::shoot(rndEngine, 0., 1.)) {
    ATH_MSG_VERBOSE("Hit is marked as inefficient");
    return false;
  }
  if (!passDeadTime(digitId, hitTime(hit), m_deadTime, deadTimes)) {
    ATH_MSG_VERBOSE("Reject hit due to dead map constraint");
    return false;
  }
  /// Correct for the signal propagation time
  const double signalTime =
      hitTime(hit) + reEle->distanceToEdge(reEle->measurementHash(digitId),
                                           locHitPos, EdgeSide::readOut) /
                         m_propagationVelocity;
  const double digitTime = CLHEP::RandGaussZiggurat::shoot(
      rndEngine, signalTime, m_stripTimeResolution);
  ATH_MSG_VERBOSE("Created new digit " << m_idHelperSvc->toString(digitId)
                                       << ", @ " << Amg::toString(locPos2D)
                                       << ", recorded time: " << digitTime);
  outContainer.push_back(std::make_unique<RpcDigit>(
      digitId, digitTime, timeOverThreshold(rndEngine)));
  ++(m_acceptedHits[measuresPhi]);
  return true;
}

bool RpcDigiTool::digitizeHitBI(const TimedHit &simHit,
                                const Muon::DigitEffiData *effiMap,
                                RpcDigitCollection &outContainer,
                                CLHEP::HepRandomEngine *rndEngine,
                                DeadTimeMap &deadTimes) const {

  ++(m_allHits[false]);
  const Identifier gasGapId = simHit->identify();
  const MuonGMR4::RpcReadoutElement *reEle =
      m_detMgr->getRpcReadoutElement(gasGapId);
  const Amg::Vector3D locPos = xAOD::toEigen(simHit->localPosition());
  const MuonGMR4::StripDesign &design{*reEle->getParameters().etaDesign};
  const RpcIdHelper &idHelper{m_idHelperSvc->rpcIdHelper()};

  ATH_MSG_DEBUG("----------------->RPCDigiTool");
  /* with RpcReadoutElement reEle you can access infor about the readout like */
  ATH_MSG_DEBUG("RpcDigiTool::digitizeHitBI reEle->nGasGaps "
                << reEle->nGasGaps());
  /* with StripDesign you can access the strip information of the readout
   * element for instance: */
  ATH_MSG_DEBUG("RpcDigiTool::digitizeHitBI design.stripPitch() "
                << design.stripPitch());
  ATH_MSG_DEBUG("RpcDigiTool::digitizeHitBI design.stripWidth() "
                << design.stripWidth());
  ATH_MSG_DEBUG("RpcDigiTool::digitizeHitBI design.numStrips() "
                << design.numStrips());
  ATH_MSG_DEBUG("RpcDigiTool::digitizeHitBI design.halfWidth() "
                << design.halfWidth());

  // Check the correctness of the local hit position
  const Amg::Vector2D locHitPosition{locPos.x(), locPos.y()};
  if (!design.insideTrapezoid(locHitPosition)) {
    ATH_MSG_VERBOSE("The hit " << Amg::toString(locHitPosition)
                               << " is outside of the trapezoid bounds for "
                               << m_idHelperSvc->toStringGasGap(gasGapId));
    return false;
  }

  // Calculate distance to strip edges (mm)
  const IdentifierHash layHash = reEle->layerHash(gasGapId);
  const double DistanceToReadOut =
      reEle->distanceToEdge(layHash, locPos, EdgeSide::readOut); // mm
  const double DistanceToHV =
      reEle->distanceToEdge(layHash, locPos, EdgeSide::highVoltage); // mm

  // Calculate charge deposited
  const double TotalChargeOnStrip =
      calculateChargeOnStrip(simHit, rndEngine, reEle->gasGapPitch());
  ATH_MSG_VERBOSE(" total charge (fC): " << TotalChargeOnStrip);

  // Calculate cluster size (number of strips)
  int clusterSize = determineClusterSizeBI(gasGapId, rndEngine);
  ATH_MSG_VERBOSE(" cluster size: " << clusterSize);

  // Get corresponding strip number and apply checks
  const int strip = design.stripNumber(locHitPosition);
  // std::cout << " strip number: " << strip;
  if (strip < 0) {
    ATH_MSG_VERBOSE("Hit " << Amg::toString(locHitPosition)
                           << " cannot trigger any signal in a strip for "
                           << m_idHelperSvc->toStringGasGap(gasGapId)
                           << std::endl
                           << design);
    return false;
  }

  // Get min and max strips
  int minStrip{strip}, maxStrip{strip}; // case strip number is 1
  if (clusterSize > 1) {
    int halfCluster = clusterSize / 2; // half cluster size (int)
    minStrip = strip - halfCluster;    // min strip number
    if (clusterSize % 2 == 0) { // if clusterSize is even, we have to randomly
                                // assign one strip on left or right side
      int side = (int)(CLHEP::RandFlat::shoot(rndEngine, 0., 1.) + 0.5);
      minStrip += side; // if side==1 move the min strip to right
    }
    maxStrip = minStrip + clusterSize - 1;
    // Check design strip boundaries
    if (minStrip < design.firstStripNumber())
      minStrip = design.firstStripNumber();
    if (maxStrip > design.firstStripNumber() + design.numStrips() - 1)
      maxStrip = design.firstStripNumber() + design.numStrips() - 1;
  }
  // std::cout << " min and max strip number: " << minStrip << " " << maxStrip;

  // Recalculate cluster size with minStrip and maxStrip
  if (minStrip == maxStrip)
    clusterSize = 1;
  else
    clusterSize = (maxStrip - minStrip) + 1;
  // std::cout << " new cluster size: " << clusterSize;

  // Divide charge on N strips
  const std::vector<double> StripCharges =
      divideChargeOnStrips(TotalChargeOnStrip, clusterSize, rndEngine);
  // for (auto &s : StripCharges)
  //   std::cout << " fractions: " << s;

  // Digitize each strip
  for (int aStrip = minStrip; aStrip <= maxStrip; aStrip++) {
    bool isValid{false};
    const Identifier digitId{idHelper.channelID(
        gasGapId, idHelper.doubletZ(gasGapId), idHelper.doubletPhi(gasGapId),
        idHelper.gasGap(gasGapId), false, aStrip, isValid)};

    // Check digitID is valid
    if (!isValid) {
      ATH_MSG_WARNING("Failed to create a valid strip "
                      << m_idHelperSvc->toStringGasGap(gasGapId)
                      << ", strip: " << aStrip);
      return false;
    }
    // Check is not dead time
    if (!passDeadTime(digitId, hitTime(simHit), m_deadTime, deadTimes)) {
      ATH_MSG_VERBOSE("Reject hit due to dead map constraint");
      return false;
    }
    // Check whether the digit is actually efficient
    const bool effiSignal1 =
        !effiMap || effiMap->getEfficiency(gasGapId) >=
                        CLHEP::RandFlat::shoot(rndEngine, 0., 1.);
    const bool effiSignal2 =
        !effiMap || effiMap->getEfficiency(gasGapId) >=
                        CLHEP::RandFlat::shoot(rndEngine, 0., 1.);
    if (effiSignal1) {
      outContainer.push_back(std::make_unique<RpcDigit>(
          digitId,
          hitTime(simHit) + getTOA(StripCharges[aStrip], DistanceToHV / 1000.),
          getTOT(StripCharges[aStrip])));
    }
    if (effiSignal2) {
      outContainer.push_back(std::make_unique<RpcDigit>(
          digitId,
          hitTime(simHit) +
              getTOA(StripCharges[aStrip], DistanceToReadOut / 1000.),
          getTOT(StripCharges[aStrip]), true));
    }
    if (effiSignal1 || effiSignal2) {
      ATH_MSG_VERBOSE("Digitize hit "
                      << m_idHelperSvc->toString(digitId)
                      << " located at: " << Amg::toString(locPos)
                      << ", SDO: " << Amg::toString(locHitPosition));
      ++(m_acceptedHits[false]);
      return true;
    }
  }

  return false;
}

double RpcDigiTool::timeOverThreshold(CLHEP::HepRandomEngine *rndmEngine) {
  // mn Time-over-threshold modeled as a narrow and a wide gaussian
  // mn based on the fit documented in
  // https://its.cern.ch/jira/browse/ATLASRECTS-7820
  constexpr double tot_mean_narrow = 16.;
  constexpr double tot_sigma_narrow = 2.;
  constexpr double tot_mean_wide = 15.;
  constexpr double tot_sigma_wide = 4.5;

  double thetot = 0.;

  if (CLHEP::RandFlat::shoot(rndmEngine) < 0.75) {
    thetot = CLHEP::RandGaussZiggurat::shoot(rndmEngine, tot_mean_narrow,
                                             tot_sigma_narrow);
  } else {
    thetot = CLHEP::RandGaussZiggurat::shoot(rndmEngine, tot_mean_wide,
                                             tot_sigma_wide);
  }

  return std::max(thetot, 0.);
}

std::vector<double>
RpcDigiTool::divideChargeOnStrips(double totalCharge, int n_strips,
                                  CLHEP::HepRandomEngine *rndmEngine) const {

  std::vector<double> charges;

  switch (n_strips) {
  case 1: {
    // Trivial case, all charge on a single strip
    charges.push_back(totalCharge);
    break;
  }
  case 2: {
    // We use a Gaussian distribution centered over 0.5 to simulate
    // a charge sharing that is on average 50/50 but includes fluctuations
    double f = CLHEP::RandGaussZiggurat::shoot(rndmEngine, 0.5, 0.15);

    // Make sure fraction is bounded between 0 and 1
    f = std::clamp(f, 0., 1.);

    charges.push_back(f * totalCharge);
    charges.push_back((1.0 - f) * totalCharge);
    break;
  }
  case 3: {
    // These fractions are guesses on a reasonable charge
    // sharing when three strips are activated.
    // These fractions must be updated once the final
    // distributions of TOT from Phase-II BI RPCs
    // are available.
    charges.push_back(0.20 * totalCharge); // left strip
    charges.push_back(0.60 * totalCharge); // center strip
    charges.push_back(0.20 * totalCharge); // right strip
    break;
  }
  case 4: {
    // These fractions are guesses on a reasonable charge
    // sharing when four strips are activated.
    // These fractions must be updated once the final
    // distributions of TOT from Phase-II BI RPCs
    // are available.
    charges.push_back(0.15 * totalCharge); // left external strip
    charges.push_back(0.35 * totalCharge); // left internal strip
    charges.push_back(0.35 * totalCharge); // right internal strip
    charges.push_back(0.15 * totalCharge); // right external strip
    break;
  }
  default: {
    // return empy vector
    break;
  }
  }
  return charges;
}

double RpcDigiTool::calculateChargeOnStrip(const TimedHit &simHit,
                                           CLHEP::HepRandomEngine *rndmEngine,
                                           const double gasGapSize) const {

  // Average energy to create an electron-ion pair inside RPC gas
  constexpr double W_VALUE_EV = 30.0; // Unit: [eV/pair]

  // RPC BI gas gap thickness
  const double GAP_THICKNESS_M =
      gasGapSize; // Unit: [m] (2 mm for Phase-II BI RPCs))

  // Townsend coefficient for gas mixture and operational voltage
  constexpr double ALPHA_PER_M = 5500.0; // Unit: [1/m]

  // Energy deposited by Geant4
  const double energy_deposit_ev =
      simHit->energyDeposit() * 1.0e6; // [MeV] -> [eV]

  // Number of electron-ion pairs created
  const double N0 = energy_deposit_ev / W_VALUE_EV;

  // Primary ionization poistion inside gas gap
  const double z_hit_m =
      CLHEP::RandFlat::shoot(rndmEngine, 0.0, GAP_THICKNESS_M); // Unit: [m]

  // Distance to anode
  const double z_drift_m = std::abs(GAP_THICKNESS_M - z_hit_m); // Unit: [m]

  // Avalanche gain
  const double gas_gain = std::exp(ALPHA_PER_M * z_drift_m);

  // Total charge
  const double total_charge_c = N0 * gas_gain * Gaudi::Units::e_SI; // Unit: [C]

  ATH_MSG_DEBUG("Charge on strip (fC): " << total_charge_c * 1e15);

  return total_charge_c * 1e15; // charge in fC
}

int RpcDigiTool::determineClusterSizeBI(
    const Identifier &idGasGap, CLHEP::HepRandomEngine *rndmEngine) const {

  const RpcIdHelper &id_helper{m_idHelperSvc->rpcIdHelper()};

  ATH_MSG_DEBUG("RpcDigitizationTool::in determineClusterSize");

  ATH_MSG_DEBUG("Digit Id = " << id_helper.show_to_string(idGasGap));

  // These cluster size probabilities were taken from preliminary
  // results from the BI RPC Upgrade work in 2025 at BB5.
  // They could be updated if new results for BI RPCs become available.
  static constexpr std::array<double, 4> ClusterSizeProbabilities{0.642, 0.316,
                                                                  0.032, 0.010};
  // Compile-time calculation of the cumulative array
  // Used empty capture list [] since variables are static constexpr
  static constexpr std::array<double, 4> cumulative = []() {
    std::array<double, 4> acumulative{};
    acumulative[0] = ClusterSizeProbabilities[0];
    for (size_t i = 1; i < ClusterSizeProbabilities.size(); ++i) {
      acumulative[i] = acumulative[i - 1] + ClusterSizeProbabilities[i];
    }
    return acumulative;
  }();

  float rndmCS = CLHEP::RandFlat::shoot(rndmEngine, 1.);

  unsigned ClusterSize{0};
  while (ClusterSize < ClusterSizeProbabilities.size() &&
         rndmCS > cumulative[ClusterSize])
    ++ClusterSize;

  if (ClusterSize >= ClusterSizeProbabilities.size())
    ClusterSize = ClusterSizeProbabilities.size() - 1;
  return ClusterSize + 1;
}

} // namespace MuonR4
