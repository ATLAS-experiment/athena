/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_JET_TAGALG_H
#define GLOBALSIM_JET_TAGALG_H

/*
  Jet_Tag GEP Algorithm Simulation

  Large-R jet tagger: reclusters the JET1 (WTA cone) small-R jets into
  large-R jets. Basic algorithm (v2) only, which is the version integrated
  into the firmware chain.

  Input and output are both already digitized - the TOBs carry integer tower
  codes - so this runs the bitwise core of the tagger (Gep::JetTaggerLRJMaker)
  directly and never constructs a floating point quantity. Going through the
  float adapter (Gep::JetTaggerLRJJetMaker, used by GepJetAlg) would
  re-quantize values that are already on the grid and could cost an LSB.
*/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "TrigGepPerf/JetTaggerLRJMaker.h"

#include "xAODCore/BaseContainer.h"

namespace GlobalSim {

    class Jet_TagAlg : public AthReentrantAlgorithm {
    public:

        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        /** @brief initialize function running before first event */
        virtual StatusCode  initialize() override;
        /** @brief execute function running for every event */
        virtual StatusCode  execute(const EventContext& ) const override;

    private:

        /** @brief Key for the input JET1 jets. Name of property is taken from TeamGate */
        SG::ReadHandleKey<xAOD::BaseContainer> m_inputJetsKey{this, "jet1_jets", "GlobalSim_JET1Jets", "type=JET1Jet; Key for the JET1 jets input"};

        /** @brief Key for the output large-R jets. Name of property is taken from TeamGate */
        SG::WriteHandleKey<xAOD::BaseContainer> m_outputKey{this, "main_output", "GlobalSim_Jet_TagJets", "type=jet_tag_output_type; Key for the output container"};

        // Algorithm parameters:
        // Defaults are the BasicV2 preset from TrigGepPerf's GepJetAlgConfig.py
        Gaudi::Property<unsigned int> m_nSeedsInput{this, "NSeedsInput", 10,
            "Leading small-R jets considered as seeds (LRJNSeedsInput)."};
        Gaudi::Property<unsigned int> m_nSeedsOutput{this, "NSeedsOutput", 2,
            "Large-R jets emitted per event - Multiplicity is fixed to 2, "
            "with events with <= 1 WTA-cone jets being zero-padded."};
        Gaudi::Property<unsigned int> m_maxObjectsConsidered{this, "MaxObjectsConsidered", 8,
            "Small-R jets considered as constituents (LRJMaxObjectsConsidered)."};
        Gaudi::Property<float> m_jetR{this, "JetR", 1.1,
            "Large-R jet radius, nominally 1.1"};

        // Bit-wise large-R jet tagger
        Gep::JetTaggerLRJMaker m_maker;
    };

}
#endif
