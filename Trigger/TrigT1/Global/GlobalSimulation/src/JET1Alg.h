/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_JET1ALG_H
#define GLOBALSIM_JET1ALG_H

/*
  JET1 GEP Algorithm Simulation
*/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

//AM: Just including to get the bit widths at the moment
#include "IO/CommonTOBContainer.h"

#include "TrigGepPerf/WTAConeParallelHelper.h"
#include "TrigGepPerf/WTACone2PassMaker.h"

#include "xAODCore/BaseContainer.h"

namespace GlobalSim {

    class JET1Alg : public AthReentrantAlgorithm {
    public:

        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        /** @brief initialize function running before first event */
        virtual StatusCode  initialize() override;
        /** @brief execute function running for every event */
        virtual StatusCode  execute(const EventContext& ) const override;

    private:

        /** @brief Key for the Input cell towers. Name of property is taken from TeamGate */
        SG::ReadHandleKey<xAOD::BaseContainer> m_inputTowersKey{this, "topoc_pu_type", "GlobalSim_CellTowers", "type=topoc_pu_type; Key for the topoc_pu_type input"};

        /** @brief Key for the output jets. Name of property is taken from TeamGate */
        SG::WriteHandleKey<xAOD::BaseContainer> m_outputKey{this, "main_output", "GlobalSim_JET1Jets", "type=JET1Jet; Key for the output container"};

    };

}
#endif
