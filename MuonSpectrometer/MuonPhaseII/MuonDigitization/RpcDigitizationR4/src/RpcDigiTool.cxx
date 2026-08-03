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
    using ChVec_t = std::vector<std::uint16_t>;
    /// @brief Declare the secondary phi and eta channels matched to the SDO
    static const SG::Decorator<ChVec_t> dec_phiChannel{"SDO_phiChannels"};
    static const SG::Decorator<ChVec_t> dec_etaChannel{"SDO_etaChannels"};

} // namespace
namespace MuonR4 {

StatusCode RpcDigiTool::initialize() {
  ATH_CHECK(MuonDigitizationTool::initialize());
  ATH_CHECK(m_writeKey.initialize());
  ATH_CHECK(m_effiDataKey.initialize(!m_effiDataKey.empty()));
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


 double RpcDigiTool::getTOT(const double aCharge) const {
    // This is a parameterization of BIRPC TOT (ns) values corresponding to
    // a charge (fC), it was obtained from a detailed model for
    // RPC signal emulation.
    constexpr std::array<double, 3> coeffs{19.9587, 0.10081, -0.00017};
    using namespace Acts::detail;

    return polynomialSum(aCharge, coeffs); 
  }
  double RpcDigiTool::getTOA(const double aCharge, const double aDistance) const {
    // This is a parameterization of BIRPC TOA (ns) values corresponding to
    // a charge (fC) and a distance (m), it was obtained from a
    // detailed model for RPC signal emulation.
    constexpr std::array<double, 3> distCoeffs{0., 5.00311, 0.00006};
    constexpr std::array<double, 3> chargeCoeffs{2.02843, -0.00641, 0.00001};
    using namespace Acts::detail;

    return polynomialSum(aDistance/1000., distCoeffs) +  // In this parameterization the distance is in m while athena standard is mm
           polynomialSum(aCharge, chargeCoeffs);
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
      RpcDigitCollection *digiColl = fetchCollection(hitId, digitCache);
      const std::size_t beforeDigiSize = digiColl->size();
      xAOD::MuonSimHit* sdo{nullptr};
      if (m_detMgr->getRpcReadoutElement(hitId)->nPhiStrips() > 0) {
        /// Standard digitization path

        const bool digitizedPhi = digitizeHit(simHit, true, efficiencyMap,
                                              *digiColl, rndEngine, deadTimes);
        const bool digitizedEta = digitizeHit(simHit, false, efficiencyMap,
                                              *digiColl, rndEngine, deadTimes);
        if (digitizedEta || digitizedPhi) {
            sdo = addSDO(simHit, sdoContainer);
        }
      } else if (digitizeHitBI(simHit, efficiencyMap, *digiColl, rndEngine,
                               deadTimes)) {
       sdo = addSDO(simHit, sdoContainer);
      }
      if (sdo) {
        sdo->setIdentifier(digiColl->back()->identify());
        dec_etaChannel(*sdo).clear();
        dec_phiChannel(*sdo).clear();
        for (std::size_t newDigit = beforeDigiSize; newDigit< digiColl->size(); ++newDigit) {
            const Identifier id = digiColl->at(newDigit)->identify();
            ChVec_t& ch{idHelper.measuresPhi(id)? dec_phiChannel(*sdo) : dec_etaChannel(*sdo)};
            ch.push_back(idHelper.channel(id));      
        }
      }
    }
  } while (viewer.next());
  /// Write everything at the end into the final digit container
  ATH_CHECK(writeDigitContainer(ctx, m_writeKey, std::move(digitCache),
                                idHelper.module_hash_max()));
  return StatusCode::SUCCESS;
}
bool RpcDigiTool::digitizeHit(const TimedHit &simHit, const bool measuresPhi,
                              const Muon::DigitEffiData *effiMap,
                              RpcDigitCollection &outContainer,
                              CLHEP::HepRandomEngine *rndEngine,
                              DeadTimeMap &deadTimes) const {

  ++(m_allHits[measuresPhi]); // Count all hits separately for eta and phi ([0] and [1], respectively) 

  const Identifier gasGapId = simHit->identify();
  const MuonGMR4::RpcReadoutElement *reEle =
      m_detMgr->getRpcReadoutElement(gasGapId);

  const RpcIdHelper &idHelper{m_idHelperSvc->rpcIdHelper()};

  bool isValid{false};

  const Identifier layerId = idHelper.channelID(
      gasGapId, idHelper.doubletZ(gasGapId), idHelper.doubletPhi(gasGapId),
      idHelper.gasGap(gasGapId), measuresPhi, 1, isValid);
  const IdentifierHash layHash = reEle->layerHash(gasGapId);
  
  const MuonGMR4::StripLayerPtr &layerDesign =
      reEle->sensorLayout(reEle->layerHash(layerId));

  const MuonGMR4::StripDesign &design{layerDesign->design(measuresPhi)};

  Amg::Vector3D locHitPos{xAOD::toEigen(simHit->localPosition())};

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
  
  // Check whether the digit is actually efficient
  const bool effiSignal =
      !effiMap || effiMap->getEfficiency(gasGapId) >=
                        CLHEP::RandFlat::shoot(rndEngine, 0., 1.);
  if (!effiSignal) return false;

  // Calculate distance from readout
  const double DistanceToEdge =
      reEle->distanceToEdge(layHash, locHitPos, EdgeSide::readOut); // mm
  
  // Calculate charge deposited
  const double TotalChargeOnStrip =
      calculateChargeOnStrip(simHit, rndEngine, 2.0);  // 2 mm gap for BM/BO chambers
  ATH_MSG_VERBOSE(" total charge (fC): " << TotalChargeOnStrip);

  // Calculate cluster size (number of strips)
  int clusterSize = determineClusterSize(gasGapId, rndEngine, false);
  ATH_MSG_VERBOSE(" cluster size: " << clusterSize);

  // Get min and max strips
  int minStrip{strip}, maxStrip{strip}; // case strip number is 1
  if (clusterSize > 1) {
    int halfCluster = clusterSize / 2; // half cluster size (int)
    minStrip = strip - halfCluster;    // min strip number
    if (clusterSize % 2 == 0) { // if clusterSize is even, we have to randomly
                                // assign one strip on left or right side
      int side = Acts::copySign(1,CLHEP::RandFlat::shoot(rndEngine, 0., 1.) + 0.5);
      minStrip += side; // if side==1 move the min strip to right
    }
    maxStrip = minStrip + clusterSize - 1;
    // Check design strip boundaries
    minStrip = std::max(minStrip, design.firstStripNumber());
    maxStrip = std::min(design.firstStripNumber() + design.numStrips() - 1, maxStrip);
  }

  // Recalculate cluster size with minStrip and maxStrip
  clusterSize = (maxStrip - minStrip) + 1;
 
  // Divide charge on N strips
  const std::vector<double> StripCharges =
      divideChargeOnStrips(TotalChargeOnStrip, clusterSize, rndEngine);

  // Digitize each strip
  bool hasAcceptedStrip=false;
  for (int aStrip = minStrip; aStrip <= maxStrip; aStrip++) {

    bool isValid{false};
    const Identifier digitId{idHelper.channelID(
        gasGapId, idHelper.doubletZ(gasGapId), idHelper.doubletPhi(gasGapId),
        idHelper.gasGap(gasGapId), measuresPhi, aStrip, isValid)};

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

    outContainer.push_back(std::make_unique<RpcDigit>(
        digitId,
        hitTime(simHit) + getTOA(StripCharges[aStrip-minStrip], DistanceToEdge),
        getTOT(StripCharges[aStrip-minStrip])));

    ATH_MSG_VERBOSE("Digitize hit "
                    << m_idHelperSvc->toString(digitId)
                    << " located at: " << Amg::toString(locHitPos) );
    ++(m_acceptedHits[measuresPhi]); // Count accepted hits for eta ([0]) or phi ([1])
    hasAcceptedStrip=true;
  }

  return hasAcceptedStrip;
}


bool RpcDigiTool::digitizeHitBI(const TimedHit &simHit,
                                const Muon::DigitEffiData *effiMap,
                                RpcDigitCollection &outContainer,
                                CLHEP::HepRandomEngine *rndEngine,
                                DeadTimeMap &deadTimes) const {

  ++(m_allHits[false]); // Count all hits for eta ([0]) since there are no phi strips in BI chambers 
  const Identifier gasGapId = simHit->identify();
  const MuonGMR4::RpcReadoutElement *reEle =
      m_detMgr->getRpcReadoutElement(gasGapId);
  const Amg::Vector3D locHitPos = xAOD::toEigen(simHit->localPosition());
  const MuonGMR4::StripDesign &design{*reEle->getParameters().etaDesign};
  const RpcIdHelper &idHelper{m_idHelperSvc->rpcIdHelper()};

  /* with RpcReadoutElement reEle you can access infor about the readout like */
  ATH_MSG_VERBOSE("RpcDigiTool::digitizeHitBI reEle->nGasGaps "<< reEle->nGasGaps());
  /* with StripDesign you can access the strip information of the readout
   * element for instance: */
  ATH_MSG_VERBOSE("RpcDigiTool::digitizeHitBI design: "<< design);
 
  // Check the correctness of the local hit position
  const Amg::Vector2D locHitPosition{locHitPos.x(), locHitPos.y()};
  if (!design.insideTrapezoid(locHitPosition)) {
    ATH_MSG_VERBOSE("The hit " << Amg::toString(locHitPosition)
                               << " is outside of the trapezoid bounds for "
                               << m_idHelperSvc->toStringGasGap(gasGapId));
    return false;
  }

  // Calculate distance to strip edges (mm)
  const IdentifierHash layHash = reEle->layerHash(gasGapId);
  const double DistanceToReadOut =
      reEle->distanceToEdge(layHash, locHitPos, EdgeSide::readOut); // mm
  const double DistanceToHV =
      reEle->distanceToEdge(layHash, locHitPos, EdgeSide::highVoltage); // mm

  // Calculate charge deposited
  const double TotalChargeOnStrip =
      calculateChargeOnStrip(simHit, rndEngine, 1.0);  // 1 mm gap for BI chambers
//mn      calculateChargeOnStrip(simHit, rndEngine, reEle->thickness());
  ATH_MSG_VERBOSE(" total charge (fC): " << TotalChargeOnStrip);

  // Calculate cluster size (number of strips)
  int clusterSize = determineClusterSize(gasGapId, rndEngine, true);
  ATH_MSG_VERBOSE(" cluster size: " << clusterSize);

  // Get corresponding strip number and apply checks
  const int strip = design.stripNumber(locHitPosition);
  if (strip < 0) {
    ATH_MSG_VERBOSE("Hit " << Amg::toString(locHitPosition)
                           << " cannot trigger any signal in a strip for "
                           << m_idHelperSvc->toStringGasGap(gasGapId)
                           << std::endl
                           << design);
    return false;
  }

  // Check whether the digit is actually efficient
  const bool effiSignal1 =
      !effiMap || effiMap->getEfficiency(gasGapId) >=
                      CLHEP::RandFlat::shoot(rndEngine, 0., 1.);
  const bool effiSignal2 =
      !effiMap || effiMap->getEfficiency(gasGapId) >=
                      CLHEP::RandFlat::shoot(rndEngine, 0., 1.);
  if (!effiSignal1 && !effiSignal2) return false;

  // Get min and max strips
  int minStrip{strip}, maxStrip{strip}; // case strip number is 1
  if (clusterSize > 1) {
    int halfCluster = clusterSize / 2; // half cluster size (int)
    minStrip = strip - halfCluster;    // min strip number
    if (clusterSize % 2 == 0) { // if clusterSize is even, we have to randomly
                                // assign one strip on left or right side
      int side = Acts::copySign(1,CLHEP::RandFlat::shoot(rndEngine, 0., 1.) + 0.5);
      minStrip += side; // if side==1 move the min strip to right
    }
    maxStrip = minStrip + clusterSize - 1;
    // Check design strip boundaries
    minStrip = std::max(minStrip, design.firstStripNumber());
    maxStrip = std::min(design.firstStripNumber() + design.numStrips() - 1, maxStrip);
  }
 
  // Recalculate cluster size with minStrip and maxStrip
  clusterSize = (maxStrip - minStrip) + 1;
 
  // Divide charge on N strips
  const std::vector<double> StripCharges =
      divideChargeOnStrips(TotalChargeOnStrip, clusterSize, rndEngine);
 
  // Digitize each strip
  bool hasAcceptedStrip=false;
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

    if (effiSignal1) {
      outContainer.push_back(std::make_unique<RpcDigit>(
          digitId,
          hitTime(simHit) + getTOA(StripCharges[aStrip-minStrip], DistanceToHV),
          getTOT(StripCharges[aStrip-minStrip])));
    }
    if (effiSignal2) {
      outContainer.push_back(std::make_unique<RpcDigit>(
          digitId,
          hitTime(simHit) +
              getTOA(StripCharges[aStrip-minStrip], DistanceToReadOut),
          getTOT(StripCharges[aStrip-minStrip]), true));
    }
    if (effiSignal1 || effiSignal2) {
      ATH_MSG_VERBOSE("Digitize hit "
                      << m_idHelperSvc->toString(digitId)
                      << " located at: " << Amg::toString(locHitPos)
                      << ", SDO: " << Amg::toString(locHitPosition));
      ++(m_acceptedHits[false]); // Count accepted hits for eta ([0]) since there are no phi strips in BI chambers 
      hasAcceptedStrip=true;
    }
  }

  return hasAcceptedStrip;
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
  const double GAP_THICKNESS_MM = gasGapSize; // Unit: [mm] (1 mm for Phase-II BI RPCs, 2 mm for BM/BO RPCs))

  // Townsend coefficient for gas mixture and operational voltage
  constexpr double ALPHA_PER_MM = 5.5; // Unit: [1/mm]

  // Energy deposited by Geant4
  const double energy_deposit_ev = simHit->energyDeposit() / Gaudi::Units::eV;
 
  // Number of electron-ion pairs created
  const double N0 = energy_deposit_ev / W_VALUE_EV;

  // Primary ionization poistion inside gas gap
  const double z_hit_mm =
      CLHEP::RandFlat::shoot(rndmEngine, 0.0, GAP_THICKNESS_MM); // Unit: [mm]

  // Distance to anode
  const double z_drift_mm = std::abs(GAP_THICKNESS_MM - z_hit_mm); // Unit: [mm]

  // Avalanche gain
  const double gas_gain = std::exp(ALPHA_PER_MM * z_drift_mm);

  // Total charge
  const double total_charge_c = N0 * gas_gain * Gaudi::Units::e_SI; // Unit: [C]

  ATH_MSG_DEBUG(__func__<<"() - "<<__LINE__<<" GAP_THICKNESS_MM: "<<GAP_THICKNESS_MM<<
      ", "<<energy_deposit_ev<<", z_hit_mm: "<<z_hit_mm<<", N0: "<<N0<<", z_drift_mm: "<<z_drift_mm
    <<", gas_gain: "<<gas_gain<< "---> Charge on strip (fC): " << total_charge_c * 1e15);

  return total_charge_c * 1e15; // charge in fC
}

int RpcDigiTool::determineClusterSize(
    const Identifier &idGasGap, CLHEP::HepRandomEngine *rndmEngine, bool isBIRPC) const {

  const RpcIdHelper &id_helper{m_idHelperSvc->rpcIdHelper()};

  ATH_MSG_DEBUG("RpcDigitizationTool::in determineClusterSize");

  ATH_MSG_DEBUG("Digit Id = " << id_helper.show_to_string(idGasGap));

  // These cluster size probabilities were taken from the legacy RPC code.
  // MuonSpectrometer/MuonConfig/python/RPC_DigitizationConfig.py
  static constexpr std::array<double, 4> ClusterSizeProbabilities{0.610, 0.260,
                                                                  0.083, 0.047};
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

  // These cluster size probabilities were taken from preliminary
  // results from the BI RPC Upgrade work in 2025 at BB5.
  // They could be updated if new results for BI RPCs become available.
  static constexpr std::array<double, 4> ClusterSizeProbabilitiesBI{0.642, 0.316,
                                                                  0.032, 0.010};
  // Compile-time calculation of the cumulative array
  // Used empty capture list [] since variables are static constexpr
  static constexpr std::array<double, 4> cumulativeBI = []() {
    std::array<double, 4> acumulative{};
    acumulative[0] = ClusterSizeProbabilitiesBI[0];
    for (size_t i = 1; i < ClusterSizeProbabilitiesBI.size(); ++i) {
      acumulative[i] = acumulative[i - 1] + ClusterSizeProbabilitiesBI[i];
    }
    return acumulative;
  }();

  std::array<double, 4> theCumulative{};
  if (isBIRPC) {
    theCumulative=cumulativeBI;
  } else {
    theCumulative=cumulative;
  }
  
  float rndmCS = CLHEP::RandFlat::shoot(rndmEngine, 1.);

  unsigned ClusterSize{1};
  while (ClusterSize < theCumulative.size() &&
         rndmCS > theCumulative[ClusterSize-1])
    ++ClusterSize;

  if (ClusterSize > theCumulative.size())
    ClusterSize = theCumulative.size();
  return ClusterSize;
}


} // namespace MuonR4
