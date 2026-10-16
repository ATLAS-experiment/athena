/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "JET1Alg.h"
#include "GlobalSimulation/Tower.h"
#include "GlobalSimulation/JET1Jet.h"
#include "xAODCore/AuxContainerBase.h"

namespace GlobalSim {
  
    StatusCode JET1Alg::initialize() {

        CHECK( m_inputTowersKey.initialize() );
        CHECK( m_outputKey.initialize() );

        return StatusCode::SUCCESS;
    }

    StatusCode JET1Alg::execute(const EventContext& ctx) const {

        ATH_MSG_DEBUG("Building WTAConeJets");
        // Read the GlobalCellTowers
        auto towers = SG::makeHandle(m_inputTowersKey, ctx);
        CHECK(towers.isValid());
        auto nTowers = towers->size();
        ATH_MSG_DEBUG("Reading " << nTowers << " cell towers from xAOD::BaseContainer");

        std::vector<WTATrigObj> input_towers;
        for(size_t i = 0; i < nTowers; i++){
            Object<Tower> tower( *towers->at(i) );
            WTATrigObj this_tower(tower.ptt.bits().to_ulong(), tower.eta.bits().to_ulong(), tower.phi.bits().to_ulong(), 0, i);
            input_towers.push_back(this_tower);
        }

        // Do the usual
        std::unique_ptr<WTAConeMaker> MyWTAConeMaker = std::make_unique<WTACone2PassMaker>();
        WTAParameters MyWTAParameters = WTAParameters(
                                  8, // const_et_cut = 2 GeV
                                  20, // seed_et_cut = 5 GeV
                                  4 // jet_dR = 4
                                  );

        MyWTAConeMaker->m_WTAConeMakerParameter = MyWTAParameters; // Pass the WTAConeParameters
        WTAConeParallelHelper wta_parallel_helper;
        wta_parallel_helper.SetBlockN(4);
        wta_parallel_helper.CreateBlocks(input_towers);
        wta_parallel_helper.RunParallelWTA(MyWTAConeMaker);
        wta_parallel_helper.CheckJetInCore();
        std::vector<WTAJet> WTAJetList = wta_parallel_helper.GetAllJets(); // Bitwise Jets

        //Write out as an xAOD::BaseContainer of SG::AuxElements
        auto jets = SG::makeHandle(m_outputKey, ctx);
        CHECK( jets.record( std::make_unique<xAOD::BaseContainer>(), std::make_unique<xAOD::AuxContainerBase>()) );

        for(const auto& WTAJet: WTAJetList){
          jets->push_back( std::make_unique<SG::AuxElement>() );
          auto jet = Object<JET1Jet>( *jets->back());

          // Assigned through the bits overload, not the value one: ptt carries a 0.25 encoder,
          // so handing it the raw count would store a count where a physical value is expected
          // and multiply it by four on the way back out. The reference implementation writes
          // the count straight into the field (GoldenGate: obj.pt() & 0x1FFF).
          jet.ptt = std::bitset<jet.ptt.width>(WTAJet.pt());
          jet.eta = std::bitset<jet.eta.width>(WTAJet.eta());
          jet.phi = std::bitset<jet.phi.width>(WTAJet.phi());
          WTA4JetERingInfo ering_info = WTAJet.GetERingInfo();
          jet.ring_zero_ptt = ering_info.ring0_Et;
          jet.ring_one_ptt = ering_info.ring1_Et;
          jet.ring_two_ptt = ering_info.ring2_Et;
          jet.ring_three_ptt = ering_info.ring3_Et;
          jet.ring_four_ptt = ering_info.ring4_Et;
          jet.total_tobs = ering_info.total_TobN;
          jet.ring_one_tobs = ering_info.ring1_TobN;
          jet.ring_two_tobs = ering_info.ring2_TobN;
          jet.ring_three_tobs = ering_info.ring3_TobN;
          jet.ring_four_tobs = ering_info.ring4_TobN;
          // The WTA engine does not provide the flags yet, so they are written as
          // false. The fields still have to be filled: without them the aux variable
          // does not exist and packing the word throws. Note et_overflow is genuinely
          // set in the reference data, so this is a placeholder, not a final value.
          jet.flag_et_overflow      = false;
          jet.flag_error            = false;
          jet.flag_next_tob_same_et = false;
          jet.flag_truncate         = false;
        }

        return StatusCode::SUCCESS;
    }


}
