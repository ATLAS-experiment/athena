/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GlobalMETAlgTool.h"

#include "../IO/CommonTOB.h"

#include "StoreGate/ReadHandle.h"

#include <algorithm>
#include <bitset>

namespace GlobalSim {

  // Main constructor
  GlobalMETAlgTool::GlobalMETAlgTool(const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type, name, parent) {
  }


  // Initialize function running before first event
  StatusCode GlobalMETAlgTool::initialize() {

    CHECK(m_gblCellTowersKey.initialize());
    CHECK(m_gblJet1JetsContainerKey.initialize());
    CHECK(m_gblMETContainerKey.initialize());

    // Mirrors GepTotalMETAlgCfg in TrigGepPerf, which configures the same core from
    // floating point input. Kept in step with METAlg::initialize(), which configures the
    // same algorithm for the BitSpec path.
    Gep::TotalMETConfig cfg;

    cfg.maxTowersConsidered      = m_maxTowersConsidered;
    cfg.jetEtThresholdGeV        = m_jetEtThresholdGeV;
    cfg.towerEtThresholdGeV      = m_towerEtThresholdGeV;
    cfg.doJetTowerOverlapRemoval = m_doJetTowerOverlapRemoval;
    cfg.towerScaleFactor         = m_towerScaleFactor;
    cfg.jetScaleFactor           = m_jetScaleFactor;

    // maxJetsConsidered and jetConeR are left at the core's defaults on purpose. Both
    // describe the UPSTREAM jet algorithm -- JET1 emits a fixed 10 jets, and the cone
    // radius is its Jet_dR -- so they are not MET's to choose. Overriding either here
    // would let MET disagree with the jets it was handed.

    // Only total MET is emitted here. The jet and tower terms are computed either way,
    // since total MET is their weighted sum.
    cfg.doTotalMET   = true;
    cfg.doJetMET     = false;
    cfg.doTowerMET   = false;
    cfg.doGEPJwoJMET = false;

    // Field widths: the standard TOB format, matching CommonTOB's s_*_width. The GRID
    // is sized separately by eta_range / phi_range below -- the field widths only have
    // to be wide enough to carry an index, never to define one.
    cfg.et_bit_length        = static_cast<unsigned int>(IOBitwise::CommonTOB::s_et_width);
    cfg.signed_et_bit_length = static_cast<unsigned int>(IOBitwise::METTOB::s_ex_width);
    cfg.eta_bit_length       = static_cast<unsigned int>(IOBitwise::CommonTOB::s_eta_width);
    cfg.phi_bit_length       = static_cast<unsigned int>(IOBitwise::CommonTOB::s_phi_width);
    cfg.sin_bit_length       = 13;

    cfg.eta_range       = 98;
    cfg.phi_range       = 64;
    cfg.eta_min         = -4.85;
    cfg.eta_granularity = 0.1;

    // Digitization range. et_max 2048 GeV over 13 bits gives the 0.25 GeV LSB.
    //
    // NOTE, carried over from GlobalJet_TagAlgTool: nothing on this path actually
    // defines an Et LSB. CommonTOB stores raw bits, and GlobalCellTowerAlgTool packs
    // cell energies in MeV without scaling, so the Et counts arriving here are not on
    // the 0.25 GeV scale assumed below. That matters MORE for MET than it does for the
    // jet tagger: the tagger only uses Et to order and sum objects, whereas the E_T
    // THRESHOLDS below are compared against digitized counts derived from this range.
    // With both thresholds at 0.0 that comparison is a no-op and the scale is harmless,
    // but a non-zero threshold here would cut at the wrong energy until the input scale
    // is settled.
    cfg.et_min = 0.0;
    cfg.et_max = 2048.0;

    // Output azimuth, sized by its own field width rather than by the tower grid: it is
    // arctan(Ey, Ex), so nothing physical caps its resolution. 6 bits is the Table 4.5
    // phi_miss width.
    cfg.met_phi_bit_length           = 6;
    cfg.met_phi_tan_scale_bit_length = 10;

    cfg.computeDerived();
    m_maker.m_cfg = cfg;

    if ((m_jetEtThresholdGeV > 0.f || m_towerEtThresholdGeV > 0.f)) {
      ATH_MSG_WARNING("A non-zero MET E_T threshold is configured (jet "
                      << m_jetEtThresholdGeV << " GeV, tower " << m_towerEtThresholdGeV
                      << " GeV) but the Et scale of the input TOBs on this path is not "
                      "the 0.25 GeV LSB assumed here, so the cut will not land where "
                      "intended. See the note in initialize().");
    }

    ATH_MSG_DEBUG("MET configured: maxTowers=" << cfg.maxTowersConsidered
                  << ", maxJets=" << cfg.maxJetsConsidered
                  << ", towerSF=" << cfg.towerScaleFactor
                  << ", jetSF=" << cfg.jetScaleFactor
                  << ", OR=" << cfg.doJetTowerOverlapRemoval
                  << ", digitized_delta_R2Cut=" << cfg.digitized_delta_R2Cut
                  << ", met_phi_range=" << cfg.met_phi_range);

    return StatusCode::SUCCESS;
  }


  // Main functional block running for each event
  StatusCode GlobalMETAlgTool::run(const std::unique_ptr<IDataCollector>& dc,
				   const EventContext& ctx) const {

    ATH_MSG_DEBUG("Building total MET");
    if (dc){dc->collect(*this, "start");}

    // Read the cell towers
    SG::ReadHandle<IOBitwise::CommonTOBContainer> h_towerTOBs =
      SG::makeHandle(m_gblCellTowersKey, ctx);
    CHECK(h_towerTOBs.isValid());
    const IOBitwise::CommonTOBContainer& inTowers = *h_towerTOBs;

    // Read the Jet1 (WTA cone) small-R jets
    SG::ReadHandle<IOBitwise::Jet1TOBContainer> h_Jet1TOBs =
      SG::makeHandle(m_gblJet1JetsContainerKey, ctx);
    CHECK(h_Jet1TOBs.isValid());
    const IOBitwise::Jet1TOBContainer& inJets = *h_Jet1TOBs;

    ATH_MSG_DEBUG("Reading " << inTowers.size() << " cell towers as CommonTOBs and "
                  << inJets.size() << " Jet1Jets as Jet1TOBs");

    // Both collections arrive already digitized, so the bits drop straight into the
    // core's input objects. Only the CommonTOB kinematics are used from either.
    std::vector<Gep::TotalMETMaker::DigiObj> towers;
    towers.reserve(inTowers.size());
    for (unsigned int i = 0; i < inTowers.size(); i++) {
      const IOBitwise::CommonTOB* inTower = inTowers[i];
      Gep::TotalMETMaker::DigiObj obj;
      obj.et  = inTower->et_bits().to_ulong();
      obj.eta = inTower->eta_bits().to_ulong();
      obj.phi = inTower->phi_bits().to_ulong();
      towers.push_back(obj);
    }

    std::vector<Gep::TotalMETMaker::DigiObj> jets;
    jets.reserve(inJets.size());
    for (unsigned int i = 0; i < inJets.size(); i++) {
      const IOBitwise::Jet1TOB* inJet = inJets[i];
      Gep::TotalMETMaker::DigiObj obj;
      obj.et  = inJet->et_bits().to_ulong();
      obj.eta = inJet->eta_bits().to_ulong();
      obj.phi = inJet->phi_bits().to_ulong();
      jets.push_back(obj);
    }

    // E_T thresholds, overlap removal and the multiplicity clamps are all applied inside
    // the core, because the firmware applies them there.
    const auto result = m_maker.makeMETDigitized(towers, jets);
    const auto& total = result.total;

    // Pack into the Table 4.5 fields. The core keeps full-width accumulators, so the TOB
    // encoders are what saturate the fields and raise the flags.
    bool exOverflow = false, eyOverflow = false, sumEtOverflow = false;
    const unsigned int exBits    = m_maker.m_cfg.metTobSignedEt(total.metX, exOverflow);
    const unsigned int eyBits    = m_maker.m_cfg.metTobSignedEt(total.metY, eyOverflow);
    const unsigned int sumEtBits = m_maker.m_cfg.metTobSumEt(total.sumEt, sumEtOverflow);

    // The inherited CommonTOB fields carry ET_miss and phi_miss; eta is meaningless for
    // a whole-event quantity and is driven to zero.
    const IOBitwise::CommonTOB commonBits(
        std::bitset<IOBitwise::CommonTOB::s_et_width>(total.met),
        std::bitset<IOBitwise::CommonTOB::s_eta_width>(0),
        std::bitset<IOBitwise::CommonTOB::s_phi_width>(total.phi));

    auto h_METTOBs = SG::makeHandle(m_gblMETContainerKey, ctx);
    auto metTOBs = std::make_unique<IOBitwise::METTOBContainer>();

    metTOBs->emplace_back(new IOBitwise::METTOB(
        commonBits,
        std::bitset<IOBitwise::METTOB::s_ex_width>(exBits),
        std::bitset<IOBitwise::METTOB::s_ey_width>(eyBits),
        std::bitset<IOBitwise::METTOB::s_sum_et_width>(sumEtBits),
        std::bitset<IOBitwise::METTOB::s_ex_overflow_width>(exOverflow),
        std::bitset<IOBitwise::METTOB::s_ey_overflow_width>(eyOverflow),
        std::bitset<IOBitwise::METTOB::s_sum_et_overflow_width>(sumEtOverflow),
        std::bitset<IOBitwise::METTOB::s_met_overflow_width>(total.metOverflow),
        std::bitset<IOBitwise::METTOB::s_upstream_overflow_width>(total.inputSaturated)));

    ATH_MSG_DEBUG("Returning MET TOB: " << metTOBs->back()->to_string());

    CHECK(h_METTOBs.record(std::move(metTOBs)));

    if (dc){dc->collect(*this, "end");}

    return StatusCode::SUCCESS;
  }

  // Overrides toString() function from base class, unused here
  std::string GlobalMETAlgTool::toString() const {
      return {};
  }

} //namespace GlobalSim
