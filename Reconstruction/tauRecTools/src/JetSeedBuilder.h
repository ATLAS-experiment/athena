/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_JETSEEDBUILDER_H
#define TAURECTOOLS_JETSEEDBUILDER_H

#include "tauRecTools/TauRecToolBase.h"


/**
 * @brief Class to build tau jet seed.
 * @author N.Meyer <nicom@cern.ch>
 * @author Felix Friedrich
*/

class JetSeedBuilder : public TauRecToolBase {

  public:
    
    ASG_TOOL_CLASS2( JetSeedBuilder, TauRecToolBase, ITauToolBase )

    /** @brief Constructor */ 
    JetSeedBuilder(const std::string& name);
    
    /** @brief Destructor */
    virtual ~JetSeedBuilder();

    /** @brief Execution of this tool */ 
    using TauRecToolBase::executeTool;
    virtual StatusCode executeTool(xAOD::TauJet& pTau,
				   const EventContext& ctx) const override;
};

#endif // TAURECTOOLS_JETSEEDBUILDER_H
