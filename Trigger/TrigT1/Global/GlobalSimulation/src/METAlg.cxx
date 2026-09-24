/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "METAlg.h"
#include "GlobalSimulation/JET1Jet.h"
#include "GlobalSimulation/topoc_pu_type.h"
#include "GlobalSimulation/met_output.h"
#include "xAODCore/AuxContainerBase.h"

#include <algorithm>

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

        // Only total MET is emitted here. The jet and tower terms are computed either
        // way, since total MET is their weighted sum.
        cfg.doTotalMET   = true;
        cfg.doJetMET     = false;
        cfg.doTowerMET   = false;
        cfg.doGEPJwoJMET = false;

        // Field widths. The tower grid indices arrive in topoc_pu_type's 10-bit eta and
        // 9-bit phi fields, but the GRID itself is 98 x 64 -- eta_range and phi_range are
        // what size it, never the field widths, which merely have to be wide enough.
        cfg.et_bit_length        = 13;
        cfg.signed_et_bit_length = 13;
        cfg.eta_bit_length       = 10;
        cfg.phi_bit_length       = 9;
        cfg.sin_bit_length       = 13;

        cfg.eta_range       = 98;
        cfg.phi_range       = 64;
        cfg.eta_min         = -4.85;
        cfg.eta_granularity = 0.1;

        // et_max 2048 GeV over 13 bits gives the 0.25 GeV LSB the TOBs are in.
        cfg.et_min = 0.0;
        cfg.et_max = 2048.0;

        // Output azimuth, sized by its own field width rather than by the tower grid:
        // it is arctan(Ey, Ex), so nothing physical caps its resolution
        cfg.met_phi_bit_length           = 6;
        cfg.met_phi_tan_scale_bit_length = 10;

        cfg.computeDerived();
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
            Object<topoc_pu_type> inTower( *inTowers->at(i) );
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
        const unsigned int exBits    = m_maker.m_cfg.metTobSignedEt(total.metX, exOverflow);
        const unsigned int eyBits    = m_maker.m_cfg.metTobSignedEt(total.metY, eyOverflow);
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
        outTob.phi_miss = static_cast<uint8_t>(total.phi);
        outTob.ex_miss  = m_maker.m_cfg.undigitizeSignedEt(exBits);
        outTob.ey_miss  = m_maker.m_cfg.undigitizeSignedEt(eyBits);
        outTob.sum_et   = m_maker.m_cfg.undigitizeEt(sumEtBits);

        outTob.flag_ex_overflow       = exOverflow;
        outTob.flag_ey_overflow       = eyOverflow;
        outTob.flag_sum_et_overflow   = sumEtOverflow;
        outTob.flag_met_overflow      = total.metOverflow;
        outTob.flag_upstream_overflow = total.inputSaturated;

        ATH_MSG_DEBUG("Built total MET: et_miss=" << total.met
                      << " phi_miss=" << total.phi
                      << " ex=" << total.metX << " ey=" << total.metY
                      << " sumEt=" << total.sumEt
                      << (total.metOverflow    ? " [MET overflow]"      : "")
                      << (total.inputSaturated ? " [upstream overflow]" : ""));

        return StatusCode::SUCCESS;
    }

}
