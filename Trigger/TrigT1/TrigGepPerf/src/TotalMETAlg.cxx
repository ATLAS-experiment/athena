/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "./TotalMETAlg.h"

#include <xAODTrigger/EnergySumRoIAuxInfo.h>

#include "AthContainers/Accessor.h"

#include <algorithm>
#include <cmath>

TotalMETAlg::TotalMETAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator) {
}


StatusCode TotalMETAlg::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "...");

  ATH_CHECK(m_caloClustersKey.initialize());
  ATH_CHECK(m_gepJetsKey.initialize());

  ATH_CHECK(m_outputTotalMETKey.initialize(m_doTotalMET));
  ATH_CHECK(m_outputJetMETKey.initialize(m_doJetMET));
  ATH_CHECK(m_outputTowerMETKey.initialize(m_doTowerMET));
  ATH_CHECK(m_outputJwoJMETKey.initialize(m_doGEPJwoJMET));
  ATH_CHECK(m_outputJwoJHardMETKey.initialize(m_doGEPJwoJMET));
  ATH_CHECK(m_outputJwoJSoftMETKey.initialize(m_doGEPJwoJMET));

  // An enabled type with no key should give an error:
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
  cfg.sqrt_lut_index_bit_length = m_sqrtLutIndexBitLength;
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

  // Only accept possible JwoJ block sizes (i.e. that divide the 98x64 tower grid nicely)
  if (cfg.doGEPJwoJMET && !cfg.isSupportedJwoJBlockSize(cfg.jwojBlockSize)) {
    ATH_MSG_ERROR("JwoJBlockSize " << cfg.jwojBlockSize << " does not tile the tower grid ("
                  << cfg.phi_range << " phi x " << cfg.eta_range << " eta).");
    return StatusCode::FAILURE;
  }

  // Fill derived constants and build the LUTs once.
  cfg.computeDerived();
  m_metMaker.m_cfg = cfg;

  ATH_MSG_INFO("Configured GEP TotalMET: E_T LSB " << cfg.et_granularity << " GeV"
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
// Both collections are digitized the same way: E_T scaled from MeV to GeV and digitzed
// Eta and phi put on the tower grid
std::vector<Gep::TotalMETMaker::DigiObj>
TotalMETAlg::digitizeTowers(const xAOD::CaloClusterContainer& clusters) const {
  const Gep::TotalMETConfig& cfg = m_metMaker.m_cfg;

  std::vector<Gep::TotalMETMaker::DigiObj> out;
  out.reserve(std::min<size_t>(clusters.size(), cfg.maxTowersConsidered));

  // Towers are taken in container order - no E_T sort: the cap is far above the tower count, so the order
  // does not select which towers are kept, and sorting would only reorder the accumulator
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

  // Taken in container order, which should be already sorted
  std::vector<Gep::TotalMETMaker::DigiObj> out;
  out.reserve(std::min<size_t>(jets.size(), cfg.maxJetsConsidered));
  for (const auto* jet : jets) {
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
  const float metX  = cfg.undigitizeTwosComplementEt(cfg.metTobTwosComplementEt(term.metX, exOverflow));
  const float metY  = cfg.undigitizeTwosComplementEt(cfg.metTobTwosComplementEt(term.metY, eyOverflow));
  const float sumEt = cfg.undigitizeEt(cfg.metTobSumEt(term.sumEt, sumEtOverflow));
  const float met   = cfg.undigitizeEt(term.met);
  const float metPhi = cfg.undigitizeMetPhi(term.phi);

  auto metObj = std::make_unique<xAOD::EnergySumRoI>();
  metObj->setStore(new xAOD::EnergySumRoIAuxInfo());

  // NEGATED on the way in, so that exMiss() / eyMiss() return the MET components.
  //
  // EnergySumRoI splits these two apart: energyX is "the total energy deposited in the X
  // direction", i.e. the SUM, and exMiss() is defined as -energyX(). Our term.metX is
  // already the missing energy 
  //
  // Consumers should therefore use exMiss()/eyMiss(), NOT energyX()/energyY().
  metObj->setEnergyX(-metX);
  metObj->setEnergyY(-metY);
  // SumET is a scalar, so no sign convention applies and energyT is read directly.
  metObj->setEnergyT(sumEt);

  // Accessor and not Decorator because these are set on an object this method just built
  // and still own
  static const SG::Accessor<float>        metAcc             {"met"};
  static const SG::Accessor<float>        metPhiAcc          {"metPhi"};
  static const SG::Accessor<unsigned int> metPhiIndexAcc     {"metPhiIndex"};
  static const SG::Accessor<char>         metOverflowAcc     {"metOverflow"};
  static const SG::Accessor<char>         sumEtOverflowAcc   {"sumEtOverflow"};
  static const SG::Accessor<char>         exOverflowAcc      {"exOverflow"};
  static const SG::Accessor<char>         eyOverflowAcc      {"eyOverflow"};
  static const SG::Accessor<char>         upstreamOverflowAcc{"upstreamOverflow"};

  // The magnitude and azimuth cannot be recovered downstream from the components: the
  // magnitude is a normalized-LUT root (up to 3 LSB off the exact one) and the azimuth a
  // comparator ladder. EnergySumRoI has no field for either, so both ride as aux data.
  metAcc(*metObj)    = met;
  metPhiAcc(*metObj) = metPhi;
  // The raw azimuth index too, since the float above is only its bin center and the
  // index is what the TOB actually carries.
  metPhiIndexAcc(*metObj) = term.phi;

  // The five TOB overflow flags
  metOverflowAcc(*metObj)      = term.metOverflow    ? 1 : 0;
  sumEtOverflowAcc(*metObj)    = sumEtOverflow       ? 1 : 0;
  exOverflowAcc(*metObj)       = exOverflow          ? 1 : 0;
  eyOverflowAcc(*metObj)       = eyOverflow          ? 1 : 0;
  upstreamOverflowAcc(*metObj) = term.inputSaturated ? 1 : 0;

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
