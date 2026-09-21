/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "./TotalMETAlg.h"

#include <xAODTrigger/EnergySumRoIAuxInfo.h>

#include <algorithm>
#include <cmath>

TotalMETAlg::TotalMETAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator) {
}


StatusCode TotalMETAlg::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "...");

  ATH_CHECK(m_caloClustersKey.initialize());
  ATH_CHECK(m_gepJetsKey.initialize());

  // Only the enabled flavors get a handle. Initializing a WriteHandleKey whose flavor is
  // off would declare an output nothing ever fills, which the scheduler treats as a
  // missing data dependency for anything downstream that asks for it.
  ATH_CHECK(m_outputTotalMETKey.initialize(m_doTotalMET));
  ATH_CHECK(m_outputJetMETKey.initialize(m_doJetMET));
  ATH_CHECK(m_outputTowerMETKey.initialize(m_doTowerMET));
  ATH_CHECK(m_outputJwoJMETKey.initialize(m_doGEPJwoJMET));
  ATH_CHECK(m_outputJwoJHardMETKey.initialize(m_doGEPJwoJMET));
  ATH_CHECK(m_outputJwoJSoftMETKey.initialize(m_doGEPJwoJMET));

  // An enabled flavor with no key is a configuration error rather than something to work
  // around: the algorithm would run the arithmetic every event and drop it on the floor.
  auto requireKey = [this](bool enabled, const SG::WriteHandleKey<xAOD::EnergySumRoI>& key,
                           const char* flag, const char* keyName) {
    if (enabled && key.key().empty()) {
      ATH_MSG_ERROR(flag << " is set but " << keyName << " is empty; nothing would be written.");
      return false;
    }
    return true;
  };
  if (!requireKey(m_doTotalMET,   m_outputTotalMETKey,    "DoTotalMET",   "outputTotalMETKey"))    return StatusCode::FAILURE;
  if (!requireKey(m_doJetMET,     m_outputJetMETKey,      "DoJetMET",     "outputJetMETKey"))      return StatusCode::FAILURE;
  if (!requireKey(m_doTowerMET,   m_outputTowerMETKey,    "DoTowerMET",   "outputTowerMETKey"))    return StatusCode::FAILURE;
  if (!requireKey(m_doGEPJwoJMET, m_outputJwoJMETKey,     "DoGEPJwoJMET", "outputJwoJMETKey"))     return StatusCode::FAILURE;
  if (!requireKey(m_doGEPJwoJMET, m_outputJwoJHardMETKey, "DoGEPJwoJMET", "outputJwoJHardMETKey")) return StatusCode::FAILURE;
  if (!requireKey(m_doGEPJwoJMET, m_outputJwoJSoftMETKey, "DoGEPJwoJMET", "outputJwoJSoftMETKey")) return StatusCode::FAILURE;

  ATH_CHECK(configureMETMaker());

  return StatusCode::SUCCESS;
}


StatusCode TotalMETAlg::configureMETMaker() {

  Gep::TotalMETConfig cfg;

  // Enables.
  cfg.doTotalMET   = m_doTotalMET;
  cfg.doJetMET     = m_doJetMET;
  cfg.doTowerMET   = m_doTowerMET;
  cfg.doGEPJwoJMET = m_doGEPJwoJMET;

  // Physics thresholds and flow.
  cfg.jetEtThresholdGeV        = m_jetEtThresholdGeV;
  cfg.towerEtThresholdGeV      = m_towerEtThresholdGeV;
  cfg.doJetTowerOverlapRemoval = m_doJetTowerOverlapRemoval;
  cfg.towerScaleFactor         = m_towerScaleFactor;
  cfg.jetScaleFactor           = m_jetScaleFactor;

  // Multiplicities.
  cfg.maxTowersConsidered = m_maxTowersConsidered;

  // jetConeR and maxJetsConsidered are left at the config's defaults on purpose. Both
  // describe the UPSTREAM WTACone jet algorithm -- the radius is its WTAJet_dR and the
  // multiplicity is how many jets it emits -- so they are not MET's to choose.
  // Overriding either here would let MET disagree with the jets it was handed.

  // Digitization field widths.
  cfg.et_bit_length        = m_etBitLength;
  cfg.signed_et_bit_length = m_signedEtBitLength;
  cfg.eta_bit_length       = m_etaBitLength;
  cfg.phi_bit_length       = m_phiBitLength;
  cfg.sin_bit_length       = m_sinBitLength;

  // Tower grid.
  cfg.eta_range       = m_etaRange;
  cfg.phi_range       = m_phiRange;
  cfg.eta_min         = m_etaMin;
  cfg.eta_granularity = m_etaGranularity;

  // E_T range.
  cfg.et_min       = m_etMin;
  cfg.et_max       = m_etMax;
  cfg.inputEtToGeV = m_inputEtToGeV;

  // MET output azimuth.
  cfg.met_phi_bit_length           = m_metPhiBitLength;
  cfg.met_phi_tan_scale_bit_length = m_metPhiTanScaleBitLength;

  // MET magnitude LUT.
  cfg.sqrt_mantissa_bit_length = m_sqrtMantissaBitLength;
  cfg.sqrt_frac_bit_length     = m_sqrtFracBitLength;
  cfg.sqrt_coeff_bit_length    = m_sqrtCoeffBitLength;
  cfg.sqrt_radicand_bit_length = m_sqrtRadicandBitLength;

  // GEP JwoJ.
  cfg.jwojHardEtThresholdGeV = m_jwojHardEtThresholdGeV;
  cfg.jwojBlockSize          = m_jwojBlockSize;
  cfg.jwojHardCoeff          = m_jwojHardCoeff;
  cfg.jwojSoftCoeff          = m_jwojSoftCoeff;

  // A zero in any of these would make computeDerived() produce nonsense granularities,
  // and the grid counts additionally divide.
  if (cfg.et_bit_length == 0 || cfg.eta_bit_length == 0 || cfg.phi_bit_length == 0 ||
      cfg.sin_bit_length == 0 || cfg.met_phi_bit_length == 0) {
    ATH_MSG_ERROR("MET digitization not configured (an E_T/eta/phi/sin/METphi bit length is 0).");
    return StatusCode::FAILURE;
  }
  if (cfg.eta_range == 0 || cfg.phi_range == 0) {
    ATH_MSG_ERROR("MET tower grid not configured (EtaRange or PhiRange is 0).");
    return StatusCode::FAILURE;
  }
  // The comparator ladder needs at least one threshold per octant, and the quadrant fold
  // assumes the range divides by 8.
  if (cfg.met_phi_bit_length < 3) {
    ATH_MSG_ERROR("METPhiBitLength must be at least 3 so the azimuth range divides into octants.");
    return StatusCode::FAILURE;
  }
  if (cfg.et_max <= cfg.et_min) {
    ATH_MSG_ERROR("EtMax must exceed EtMin.");
    return StatusCode::FAILURE;
  }

  // Rejected rather than silently reset to 1: a block size that does not tile the grid
  // would leave a ragged block at the phi seam judged against the same threshold as its
  // full neighbors, i.e. a phi slice pushed systematically into the soft term.
  if (cfg.doGEPJwoJMET && !cfg.isSupportedJwoJBlockSize(cfg.jwojBlockSize)) {
    ATH_MSG_ERROR("JwoJBlockSize " << cfg.jwojBlockSize << " does not tile the tower grid ("
                  << cfg.phi_range << " phi x " << cfg.eta_range << " eta).");
    return StatusCode::FAILURE;
  }

  // Fill derived constants and build the LUTs once.
  cfg.computeDerived();
  m_metMaker.m_cfg = cfg;

  ATH_MSG_INFO("Configured TotalMET: E_T LSB " << cfg.et_granularity << " GeV"
               << ", grid " << cfg.eta_range << " eta x " << cfg.phi_range << " phi"
               << ", MET phi " << cfg.met_phi_range << " bins (" << cfg.met_phi_bit_length << " bits)"
               << ", jetEt > " << cfg.jetEtThresholdGeV << " GeV"
               << ", towerEt > " << cfg.towerEtThresholdGeV << " GeV"
               << ", OR " << (cfg.doJetTowerOverlapRemoval ? "on" : "off")
               << ", twrSF " << cfg.towerScaleFactor << ", jetSF " << cfg.jetScaleFactor);
  if (cfg.doGEPJwoJMET)
    ATH_MSG_INFO("GEP JwoJ enabled: block " << cfg.jwojBlockSize << "x" << cfg.jwojBlockSize
                 << " (" << cfg.jwojBlockSize * cfg.eta_granularity << " x "
                 << cfg.jwojBlockSize * cfg.eta_granularity << ")"
                 << ", hard-term E_T threshold " << cfg.jwojHardEtThresholdGeV << " GeV"
                 << ", hard coefficient " << cfg.jwojHardCoeff
                 << ", soft coefficient " << cfg.jwojSoftCoeff);

  return StatusCode::SUCCESS;
}


// ------------------------------------------------------------------
// Float front end
// ------------------------------------------------------------------
// Both collections are digitized the same way: E_T scaled from MeV to GeV and put on the
// E_T grid, eta and phi put on the tower grid. The cast to float before digitizing is
// deliberate -- see the note on the declaration.
std::vector<Gep::TotalMETMaker::DigiObj>
TotalMETAlg::digitizeTowers(const xAOD::CaloClusterContainer& clusters) const {
  const Gep::TotalMETConfig& cfg = m_metMaker.m_cfg;

  std::vector<Gep::TotalMETMaker::DigiObj> out;
  out.reserve(std::min<size_t>(clusters.size(), cfg.maxTowersConsidered));

  // Towers are taken in container order, matching the standalone emulation, which reads
  // them in ntuple order. No E_T sort: the cap is far above the tower count, so the order
  // does not select which towers are kept, and sorting would only reorder the accumulator
  // -- which is not free, since integer division truncates at every term.
  for (const auto* cluster : clusters) {
    if (out.size() >= cfg.maxTowersConsidered) break;
    Gep::TotalMETMaker::DigiObj obj;
    obj.et  = cfg.digitizeEt(cluster->et() * cfg.inputEtToGeV);
    obj.eta = cfg.digitizeEta(static_cast<float>(cluster->eta()));
    obj.phi = cfg.digitizePhi(static_cast<float>(cluster->phi()));
    out.push_back(obj);
  }
  return out;
}


std::vector<Gep::TotalMETMaker::DigiObj>
TotalMETAlg::digitizeJets(const xAOD::JetContainer& jets) const {
  const Gep::TotalMETConfig& cfg = m_metMaker.m_cfg;

  // Sorted E_T-descending before the cap bites, so MaxJetsConsidered keeps the LEADING
  // jets. The standalone reads an already-sorted jet tree, so this is what reproduces it;
  // without the sort the cap would keep whichever jets the container happened to list
  // first.
  std::vector<const xAOD::Jet*> sorted;
  sorted.reserve(jets.size());
  for (const auto* jet : jets) sorted.push_back(jet);
  std::sort(sorted.begin(), sorted.end(),
            [](const xAOD::Jet* a, const xAOD::Jet* b) { return a->pt() > b->pt(); });

  std::vector<Gep::TotalMETMaker::DigiObj> out;
  out.reserve(std::min<size_t>(sorted.size(), cfg.maxJetsConsidered));
  for (const auto* jet : sorted) {
    if (out.size() >= cfg.maxJetsConsidered) break;
    Gep::TotalMETMaker::DigiObj obj;
    obj.et  = cfg.digitizeEt(jet->pt() * cfg.inputEtToGeV);
    obj.eta = cfg.digitizeEta(static_cast<float>(jet->eta()));
    obj.phi = cfg.digitizePhi(static_cast<float>(jet->phi()));
    out.push_back(obj);
  }
  return out;
}


// ------------------------------------------------------------------
// Output
// ------------------------------------------------------------------
StatusCode TotalMETAlg::recordMET(const SG::WriteHandleKey<xAOD::EnergySumRoI>& key,
                                  const EventContext& ctx,
                                  const Gep::TotalMETMaker::DigiMETTerm& term) const {
  const Gep::TotalMETConfig& cfg = m_metMaker.m_cfg;

  // Through the TOB encoders first: the accumulator is full width, the TOB fields are
  // not, and the firmware SATURATES where a mask would wrap. Writing the accumulator
  // directly would disagree with the hardware exactly on the events where it matters.
  bool exOverflow = false, eyOverflow = false, sumEtOverflow = false;
  const float metX  = cfg.undigitizeSignedEt(cfg.metTobSignedEt(term.metX, exOverflow));
  const float metY  = cfg.undigitizeSignedEt(cfg.metTobSignedEt(term.metY, eyOverflow));
  const float sumEt = cfg.undigitizeEt(cfg.metTobSumEt(term.sumEt, sumEtOverflow));
  const float met   = cfg.undigitizeEt(term.met);
  const float metPhi = cfg.undigitizeMetPhi(term.phi);

  auto metObj = std::make_unique<xAOD::EnergySumRoI>();
  metObj->setStore(new xAOD::EnergySumRoIAuxInfo());
  metObj->setEnergyX(metX);
  metObj->setEnergyY(metY);
  metObj->setEnergyT(sumEt);

  // The magnitude and azimuth cannot be recovered downstream from the components: the
  // magnitude is a normalized-LUT root (up to 3 LSB off the exact one) and the azimuth a
  // comparator ladder. Both have to travel with the object.
  metObj->auxdata<float>("met")    = met;
  metObj->auxdata<float>("metPhi") = metPhi;
  // The raw azimuth index too, since the float above is only its bin center and the
  // index is what the TOB actually carries.
  metObj->auxdata<unsigned int>("metPhiIndex") = term.phi;

  // The five TOB overflow flags. char rather than bool: std::vector<bool> aside, the aux
  // store handles char cleanly and ROOT shows it as a small integer.
  metObj->auxdata<char>("metOverflow")      = term.metOverflow    ? 1 : 0;
  metObj->auxdata<char>("sumEtOverflow")    = sumEtOverflow       ? 1 : 0;
  metObj->auxdata<char>("exOverflow")       = exOverflow          ? 1 : 0;
  metObj->auxdata<char>("eyOverflow")       = eyOverflow          ? 1 : 0;
  metObj->auxdata<char>("upstreamOverflow") = term.inputSaturated ? 1 : 0;

  auto handle = SG::makeHandle(key, ctx);
  ATH_CHECK(handle.record(std::move(metObj)));

  return StatusCode::SUCCESS;
}


StatusCode TotalMETAlg::execute(const EventContext& context) const {
  ATH_MSG_DEBUG("Executing " << name() << "...");
  setFilterPassed(false, context);

  auto h_caloClusters = SG::makeHandle(m_caloClustersKey, context);
  ATH_CHECK(h_caloClusters.isValid());

  auto h_gepJets = SG::makeHandle(m_gepJetsKey, context);
  ATH_CHECK(h_gepJets.isValid());

  ATH_MSG_DEBUG("Read " << h_caloClusters->size() << " towers and "
                << h_gepJets->size() << " jets");

  // Float front end, then the bitwise core.
  const std::vector<Gep::TotalMETMaker::DigiObj> towers = digitizeTowers(*h_caloClusters);
  const std::vector<Gep::TotalMETMaker::DigiObj> jets   = digitizeJets(*h_gepJets);

  const Gep::TotalMETMaker::DigiMETResult result = m_metMaker.makeMETDigitized(towers, jets);

  if (m_doTotalMET) ATH_CHECK(recordMET(m_outputTotalMETKey, context, result.total));
  if (m_doJetMET)   ATH_CHECK(recordMET(m_outputJetMETKey,   context, result.jet));
  if (m_doTowerMET) ATH_CHECK(recordMET(m_outputTowerMETKey, context, result.tower));
  if (m_doGEPJwoJMET) {
    ATH_CHECK(recordMET(m_outputJwoJMETKey,     context, result.jwoj));
    ATH_CHECK(recordMET(m_outputJwoJHardMETKey, context, result.jwojHard));
    ATH_CHECK(recordMET(m_outputJwoJSoftMETKey, context, result.jwojSoft));
  }

  ATH_MSG_DEBUG("Total MET " << m_metMaker.m_cfg.undigitizeEt(result.total.met) << " GeV"
                << " at phi bin " << result.total.phi);

  setFilterPassed(true, context);
  return StatusCode::SUCCESS;
}
