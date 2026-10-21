
/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MlMuonDVtxHypoTool.h"


#include "TrigCompositeUtils/TrigCompositeUtils.h"

using namespace TrigCompositeUtils;

MlMuonDVtxHypoTool::MlMuonDVtxHypoTool(const std::string& type,
                                       const std::string& name,
                                       const IInterface* parent )
    : AthAlgTool( type, name, parent ),
    m_decisionId( HLT::Identifier::fromToolName( name ) ) {}

StatusCode MlMuonDVtxHypoTool::decide(Info& info)  const {

    if (info.activeChainIDs.count(m_decisionId.numeric()) == 0) {
        ATH_MSG_DEBUG("Chain ID " << m_decisionId.numeric() << " not active for this event, skipping");
        return StatusCode::SUCCESS;    
    }

    if (info.score < m_scoreCut) {
        ATH_MSG_DEBUG("Failing score cut: " << info.score << " < " << m_scoreCut);
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG("Passing score cut: " << info.score << " >= " << m_scoreCut);
	addDecisionID(m_decisionId.numeric(), info.outDecision);

	return StatusCode::SUCCESS;
}
