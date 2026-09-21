/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_METALG_H
#define GLOBALSIM_METALG_H

/*
  MET GEP Algorithm Simulation

  Total MET: the scale-factor-weighted combination of a tower term, built from the
  pileup-suppressed cell towers, and a jet term, built from the JET1 (WTA cone) small-R
  jets.

  Input and output are both already digitized - the TOBs carry integer tower indices and
  E_T counts - so this runs the bitwise core (Gep::TotalMETMaker::makeMETDigitized)
  directly and never constructs a floating point quantity. Going through the float
  adapter (TotalMETAlg in TrigGepPerf) would re-quantize values that are already on the
  grid and could cost an LSB.

  Unlike every other GlobalSim algorithm so far, this one takes TWO inputs: it needs the
  towers and the jets built from those same towers.

  Only total MET is emitted, one TOB per event, matching the single word MET_Engine.v
  produces. The jet and tower terms are still computed internally, since total MET is
  their weighted sum; they are simply not written out.
*/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "TrigGepPerf/TotalMETMaker.h"

#include "xAODCore/BaseContainer.h"

namespace GlobalSim {

    class METAlg : public AthReentrantAlgorithm {
    public:

        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        /** @brief initialize function running before first event */
        virtual StatusCode  initialize() override;
        /** @brief execute function running for every event */
        virtual StatusCode  execute(const EventContext& ) const override;

    private:

        /** @brief Key for the input cell towers. Name of property is taken from TeamGate */
        SG::ReadHandleKey<xAOD::BaseContainer> m_inputTowersKey{this, "topoc_pu_type", "GlobalSim_CellTowers", "type=topoc_pu_type; Key for the topoc_pu_type input"};

        /** @brief Key for the input JET1 jets. Name of property is taken from TeamGate */
        SG::ReadHandleKey<xAOD::BaseContainer> m_inputJetsKey{this, "jet1_jets", "GlobalSim_JET1Jets", "type=JET1Jet; Key for the JET1 jets input"};

        /** @brief Key for the output MET TOB. Name of property is taken from TeamGate */
        SG::WriteHandleKey<xAOD::BaseContainer> m_outputKey{this, "main_output", "GlobalSim_MET", "type=met_output_type; Key for the output container"};

        // Algorithm parameters.
        // Defaults are those of TrigGepPerf's GepTotalMETAlgCfg, which is the same
        // algorithm driven from floating point input.
        //
        // The jet cone radius and the jet multiplicity are deliberately NOT properties
        // here. Both belong to the upstream WTACone/JET1 algorithm that produced the jets
        // -- the radius is its Jet_dR and the multiplicity is how many jets it emits --
        // so MET takes what it is given rather than declaring its own and risking
        // disagreement.
        Gaudi::Property<unsigned int> m_maxTowersConsidered{this, "MaxTowersConsidered", 4096,
            "Towers considered per event."};
        Gaudi::Property<float> m_jetEtThresholdGeV{this, "JetEtThresholdGeV", 0.0,
            "Minimum jet E_T (GeV) entering the jet MET sum."};
        Gaudi::Property<float> m_towerEtThresholdGeV{this, "TowerEtThresholdGeV", 0.0,
            "Minimum tower E_T (GeV) entering the tower MET sum."};
        Gaudi::Property<bool> m_doJetTowerOverlapRemoval{this, "DoJetTowerOverlapRemoval", false,
            "Drop towers within the upstream jet cone radius of a jet that passed JetEtThresholdGeV."};
        Gaudi::Property<float> m_towerScaleFactor{this, "TowerScaleFactor", 1.0,
            "Scalar weight on the tower term in the total."};
        Gaudi::Property<float> m_jetScaleFactor{this, "JetScaleFactor", 1.0,
            "Scalar weight on the jet term in the total."};

        // Bitwise MET core, configured once in initialize()
        Gep::TotalMETMaker m_maker;
    };

}
#endif
