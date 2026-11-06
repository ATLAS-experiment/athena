/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "METAlg.h"
#include "GlobalSimulation/JET1Jet.h"
#include "GlobalSimulation/Tower.h"
#include "GlobalSimulation/met_output.h"
#include "Utilities/TowerLUTs.h"
#include "xAODCore/AuxContainerBase.h"

#include <algorithm>
#include <utility>

namespace GlobalSim {

    StatusCode METAlg::initialize() {

        CHECK( m_inputTowersKey.initialize() );
        CHECK( m_inputJetsKey.initialize() );
        CHECK( m_outputKey.initialize() );

        // Mirrors GepTotalMETAlgCfg in TrigGepPerf, which configures the same core from
        // floating point input. Kept in step with GlobalMETAlgTool::initialize(), which
        // configures it for the TOB path.
        Gep::TotalMETConfig cfg;

        cfg.maxTowersConsidered      = m_maxTowersConsidered;
        cfg.jetEtThresholdGeV        = m_jetEtThresholdGeV;
        cfg.towerEtThresholdGeV      = m_towerEtThresholdGeV;
        cfg.doJetTowerOverlapRemoval = m_doJetTowerOverlapRemoval;
        cfg.towerScaleFactor         = m_towerScaleFactor;
        cfg.jetScaleFactor           = m_jetScaleFactor;

        // Only total MET is configured here
        cfg.doTotalMET   = true;
        cfg.doJetMET     = false;
        cfg.doTowerMET   = false;
        cfg.doGEPJwoJMET = false;

        // Field widths from the v3 tower encoding, Ex/Ey width from met_output_type. The tower
        // grid (98 x 64) is the TotalMETConfig default, not the field widths.
        static_assert(met_output_type::et_miss.width == v3::Tower::ptt.width,
                      "MET E_T width must equal the tower E_T width");
        cfg.et_bit_length        = v3::Tower::ptt.width;
        cfg.signed_et_bit_length = met_output_type::ex_miss.width;
        cfg.eta_bit_length       = v3::Tower::eta.width;
        cfg.phi_bit_length       = v3::Tower::phi.width;

        // Et LSB from the v3 tower ptt decoder (0.25 GeV), over the full field (2048 GeV)
        cfg.et_max = v3::Tower::ptt.decode(1u) * (1u << v3::Tower::ptt.width);

        // The sqrt LUT's radicand is Ex^2 + Ey^2, twice the MET E_T width
        cfg.sqrt_radicand_bit_length = 2 * met_output_type::et_miss.width;

        // sin LUT and phi_miss widths are the TotalMETConfig defaults (fixed by the firmware)

        cfg.computeDerived();
        // sin LUTs from the tower BitSpec's phi codes, replacing the ones computeDerived() built
        auto [sinLUT, sinLUTHalf] = makeSinLUTs(v3::Tower::phi, cfg.sin_bit_length);
        if (not cfg.setSinLUTs(std::move(sinLUT), std::move(sinLUTHalf))) {
            ATH_MSG_ERROR("Tower phi BitSpec gives " << nGridCodes(v3::Tower::phi)
                          << " phi codes, the MET tower grid has " << cfg.phi_range);
            return StatusCode::FAILURE;
        }
        m_maker.m_cfg = cfg;

        ATH_MSG_DEBUG("MET configured: maxTowers=" << cfg.maxTowersConsidered
                      << ", maxJets=" << cfg.maxJetsConsidered
                      << ", towerSF=" << cfg.towerScaleFactor
                      << ", jetSF=" << cfg.jetScaleFactor
                      << ", OR=" << cfg.doJetTowerOverlapRemoval
                      << ", digitized_delta_R2Cut=" << cfg.digitized_delta_R2Cut
                      << ", met_phi_range=" << cfg.met_phi_range);

        return StatusCode::SUCCESS;
    }

    StatusCode METAlg::execute(const EventContext& ctx) const {

        ATH_MSG_DEBUG("Building total MET");

        // Read the pileup-suppressed cell towers
        auto inTowers = SG::makeHandle(m_inputTowersKey, ctx);
        CHECK(inTowers.isValid());
        const auto nTowers = inTowers->size();

        // Read the JET1 (WTA cone) small-R jets
        auto inJets = SG::makeHandle(m_inputJetsKey, ctx);
        CHECK(inJets.isValid());
        const auto nJets = inJets->size();

        ATH_MSG_DEBUG("Reading " << nTowers << " cell towers and "
                      << nJets << " JET1 jets from xAOD::BaseContainer");

        // Both collections arrive already digitized, so the bits drop straight into the
        // core's input objects with no conversion.
        std::vector<Gep::TotalMETMaker::DigiObj> towers;
        towers.reserve(nTowers);
        for (size_t i = 0; i < nTowers; i++) {
            Object<v3::Tower> inTower( *inTowers->at(i) );
            Gep::TotalMETMaker::DigiObj obj;
            obj.et  = inTower.ptt.bits().to_ulong();
            obj.eta = inTower.eta.bits().to_ulong();
            obj.phi = inTower.phi.bits().to_ulong();
            towers.push_back(obj);
        }

        std::vector<Gep::TotalMETMaker::DigiObj> jets;
        jets.reserve(nJets);
        for (size_t i = 0; i < nJets; i++) {
            Object<JET1Jet> inJet( *inJets->at(i) );
            Gep::TotalMETMaker::DigiObj obj;
            obj.et  = inJet.ptt.bits().to_ulong();
            obj.eta = inJet.eta.bits().to_ulong();
            obj.phi = inJet.phi.bits().to_ulong();
            jets.push_back(obj);
        }

        // E_T thresholds, overlap removal and the multiplicity clamps are all applied
        // inside the core, because the firmware applies them there.
        const auto result = m_maker.makeMETDigitized(towers, jets);
        const auto& total = result.total;

        bool exOverflow = false, eyOverflow = false, sumEtOverflow = false;
        const unsigned int exBits    = m_maker.m_cfg.metTobTwosComplementEt(total.metX, exOverflow);
        const unsigned int eyBits    = m_maker.m_cfg.metTobTwosComplementEt(total.metY, eyOverflow);
        const unsigned int sumEtBits = m_maker.m_cfg.metTobSumEt(total.sumEt, sumEtOverflow);

        // Write out as an xAOD::BaseContainer of SG::AuxElements. One TOB per event.
        auto outMET = SG::makeHandle(m_outputKey, ctx);
        CHECK( outMET.record( std::make_unique<xAOD::BaseContainer>(),
                              std::make_unique<xAOD::AuxContainerBase>()) );

        outMET->push_back( std::make_unique<SG::AuxElement>() );
        auto outTob = Object<met_output_type>( *outMET->back() );

        // The fields are declared with encoders that take physical units, so the counts
        // are undigitized on the way in and re-encoded to the same bits on the way out.
        // Going through the encoders rather than assigning raw counts is what keeps this
        // word identical to one built by any other client of met_output_type.
        outTob.et_miss  = m_maker.m_cfg.undigitizeEt(total.met);
        // phi_miss is {6-bit index, zero padding}: shift the index up by the field's spare bits
        outTob.phi_miss = static_cast<uint16_t>(total.phi << (met_output_type::phi_miss.width - m_maker.m_cfg.met_phi_bit_length));
        outTob.ex_miss  = m_maker.m_cfg.undigitizeTwosComplementEt(exBits);
        outTob.ey_miss  = m_maker.m_cfg.undigitizeTwosComplementEt(eyBits);
        outTob.sum_et   = m_maker.m_cfg.undigitizeEt(sumEtBits);

        outTob.flag_sum_et_overflow   = sumEtOverflow;
        outTob.flag_met_overflow      = total.metOverflow;
        outTob.flag_upstream_overflow = total.inputSaturated;

        ATH_MSG_DEBUG("Built total MET: et_miss=" << total.met
                      << " phi_miss=" << total.phi
                      << " ex=" << total.metX << " ey=" << total.metY
                      << " sumEt=" << total.sumEt
                      << (total.metOverflow    ? " [MET overflow]"      : "")
                      << (exOverflow || eyOverflow ? " [Ex/Ey saturated, not in TOB]" : "")
                      << (total.inputSaturated ? " [upstream overflow]" : ""));

        return StatusCode::SUCCESS;
    }

}
