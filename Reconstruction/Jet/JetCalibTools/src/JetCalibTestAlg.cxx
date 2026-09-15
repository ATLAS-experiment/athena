/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetCalibTools/JetCalibTestAlg.h"
#include "AsgDataHandles/ReadHandle.h"
#include "xAODCore/ShallowCopy.h"

#include <vector>

StatusCode JetCalibTestAlg::initialize() {
    ATH_CHECK(m_jetCalibTool.retrieve());
    ATH_CHECK(m_jetKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode JetCalibTestAlg::execute() {
    // Retrieve jet container
    SG::ReadHandle<xAOD::JetContainer> jets(m_jetKey);
    ATH_CHECK(jets.isValid());

    auto shallowCopy = xAOD::shallowCopy(*jets);
    auto& calibJets = *shallowCopy.first;

    // Record the pre-calibration pT so it can be compared afterwards
    std::vector<double> ptBefore;
    if (msgLvl(MSG::DEBUG)) {
        ptBefore.reserve(calibJets.size());
        for (const xAOD::Jet* jet : calibJets) {
            ptBefore.push_back(jet->pt());
        }
    }

    ATH_CHECK(m_jetCalibTool->calibrate(calibJets));

    if (msgLvl(MSG::DEBUG)) {
        ATH_MSG_DEBUG("Calibrated " << calibJets.size() << " jets from " << m_jetKey.key());
        for (size_t i = 0; i < calibJets.size(); ++i) {
            const double ptAfter = calibJets[i]->pt();
            ATH_MSG_DEBUG("  jet " << i
                          << ": pT before = " << ptBefore[i] / 1000. << " GeV"
                          << ", pT after = " << ptAfter / 1000. << " GeV"
                          << ", ratio = " << ptAfter / ptBefore[i]);
        }
    }

    return StatusCode::SUCCESS;
}
