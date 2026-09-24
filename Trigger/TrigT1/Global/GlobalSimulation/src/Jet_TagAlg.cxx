/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "Jet_TagAlg.h"
#include "GlobalSimulation/JET1Jet.h"
#include "GlobalSimulation/jet_tag_output.h"
#include "xAODCore/AuxContainerBase.h"

#include <algorithm>

namespace GlobalSim {

    StatusCode Jet_TagAlg::initialize() {

        CHECK( m_inputJetsKey.initialize() );
        CHECK( m_outputKey.initialize() );

        // v2 (basic) algorithm configuration
        // Mirroring the BasicV2 preset in TrigGepPerf's GepJetAlgConfig.py
        Gep::JetTaggerLRJConfig cfg;

        cfg.algoVersion          = 2;
        cfg.nSeedsInput          = m_nSeedsInput;
        cfg.nSeedsOutput         = m_nSeedsOutput;
        cfg.maxObjectsConsidered = m_maxObjectsConsidered;
        cfg.r2Cut                = static_cast<double>(m_jetR) * static_cast<double>(m_jetR);

        // Midpoint seeding and overlap removal are only in v3 algorithm --> disabled
        cfg.midpointSearchDistance   = 0.001;
        cfg.enableOverlapRemoval     = false;
        cfg.minEtSeedPosOptimization = false;
        cfg.enableEtWeightedMidpoint = false;

        // Field widths: the standard TOB format, as written by jet_tag_output_type
        cfg.et_bit_length  = 13;
        cfg.eta_bit_length = 10;
        cfg.phi_bit_length = 9;

        // v2 computes no jet substructure, so every substructure field is zero-width
        cfg.num_subjets_length       = 0;
        cfg.N_subjetiness_bit_length = 0;
        cfg.mass_approx_bit_length   = 0;
        cfg.psi_R_bit_length         = 0;
        cfg.deltaR_lut_length        = 8;

        // Digitization ranges. et_max 2048 GeV over 13 bits gives the 0.25 GeV
        // LSB the TOBs are in. The eta/phi ranges are the GEP tower grid, first
        // tower centre to one granularity past the last.
        // FIXME in the future these should either 1. not need to be configured or 
        // 2. take values from some default configuration (so they can be shared across algorithms)
        cfg.et_min  = 0.0;
        cfg.et_max  = 2048.0;
        cfg.eta_min = -4.85;
        cfg.eta_max = 4.95;
        cfg.phi_min = -3.15;
        cfg.phi_max = 3.25;

        cfg.computeDerived();
        m_maker.m_cfg = cfg;
        // Jet inputs: the seed's own E_T is primed into the output sum, and a
        // merged jet above threshold is itself a subjet candidate.
        m_maker.SetConstSource(Gep::JetTaggerConstSource::WTACone);

        ATH_MSG_DEBUG("Jet_Tag configured: v2, with R=" << cfg.rCut
                      << ", nSeedsInput=" << cfg.nSeedsInput
                      << ", nSeedsOutput=" << cfg.nSeedsOutput
                      << ", maxObjects=" << cfg.maxObjectsConsidered
                      << ", digitized_delta_R2Cut=" << cfg.digitized_delta_R2Cut
                      << ", pi_digitized_in_phi=" << cfg.pi_digitized_in_phi);

        return StatusCode::SUCCESS;
    }

    StatusCode Jet_TagAlg::execute(const EventContext& ctx) const {

        ATH_MSG_DEBUG("Building large-R tagged jets");

        // Read the JET1 (WTA cone) small-R jets
        auto inJets = SG::makeHandle(m_inputJetsKey, ctx);
        CHECK(inJets.isValid());
        const auto nJets = inJets->size();
        ATH_MSG_DEBUG("Reading " << nJets << " JET1 jets from xAOD::BaseContainer");

        // Reading in Jet1 input (in same manner that Jet1 reads in tower input)
        std::vector<Gep::JetTaggerLRJMaker::DigiObj> objects;
        objects.reserve(nJets);
        for (size_t i = 0; i < nJets; i++) {
            Object<JET1Jet> inJet( *inJets->at(i) );
            Gep::JetTaggerLRJMaker::DigiObj obj;
            obj.et  = inJet.ptt.bits().to_ulong();
            obj.eta = inJet.eta.bits().to_ulong();
            obj.phi = inJet.phi.bits().to_ulong();
            objects.push_back(obj);
        }

        // Seeds and constituents are read as two disjoint slices of the same
        // jet collection: the leading nSeedsOutput jets become the large-R jet
        // seeds, and the next maxObjectsConsidered are the constituents merged
        // into them.
        const size_t nSeeds = std::min<size_t>(m_nSeedsOutput.value(), objects.size());
        const size_t nConst = std::min<size_t>(m_maxObjectsConsidered.value(),
                                               objects.size() - nSeeds);

        const std::vector<Gep::JetTaggerLRJMaker::DigiObj>
            seeds(objects.begin(), objects.begin() + nSeeds);
        const std::vector<Gep::JetTaggerLRJMaker::DigiObj>
            constituents(objects.begin() + nSeeds, objects.begin() + nSeeds + nConst);

        const auto lrjs = m_maker.makeLargeRJetsDigitized(seeds, constituents);

        // Write out as an xAOD::BaseContainer of SG::AuxElements
        auto outJets = SG::makeHandle(m_outputKey, ctx);
        CHECK( outJets.record( std::make_unique<xAOD::BaseContainer>(),
                               std::make_unique<xAOD::AuxContainerBase>()) );

        for (const auto& lrj : lrjs) {
            outJets->push_back( std::make_unique<SG::AuxElement>() );
            auto outJet = Object<jet_tag_output_type>( *outJets->back() );

            outJet.ptt = lrj.et;
            outJet.eta = lrj.eta;
            outJet.phi = lrj.phi;
        }

        ATH_MSG_DEBUG("Built " << outJets->size() << " large-R tagged jets");

        return StatusCode::SUCCESS;
    }

}
