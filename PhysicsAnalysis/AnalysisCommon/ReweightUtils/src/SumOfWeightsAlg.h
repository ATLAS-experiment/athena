///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Calls IWeightTool objects for each event and stores SumOfWeights
// for each of these computations
//
// Author: Danilo Ferreira de Lima <dferreir@cern.ch>
///////////////////////////////////////////////////////////////////

#ifndef REWEIGHTUTILS_SUMOFWEIGHTSALG_H
#define REWEIGHTUTILS_SUMOFWEIGHTSALG_H

// Include the base class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaKernel/ICutFlowSvc.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ToolHandle.h"

#include <string>
#include <vector>

class IWeightTool;

class SumOfWeightsAlg : public AthReentrantAlgorithm {

  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    /// Athena algorithm's Hooks
    virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE () override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:

    /// Athena configured components
    ServiceHandle<ICutFlowSvc> m_cutFlowSvc{ this, "CutFlowSvc", "CutFlowSvc/CutFlowSvc", "Pointer to the CutFlowSvc"};
    PublicToolHandleArray<IWeightTool> m_weightTools{ this, "WeightTools", {}, "List of WeightTools to be called for each event"};

    /// cut IDs
    std::vector<CutIdentifier> m_cutIDs;

};

#endif //> !REWEIGHTUTILS_SUMOFWEIGHTSALG_H
