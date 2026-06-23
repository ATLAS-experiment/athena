///////////////////////// -*- C++ -*- /////////////////////////////
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// ViewTestAlg.h 
// Header file for class ViewTestAlg
// Author: B. Wynne <bwynne@cern.ch>
///////////////////////////////////////////////////////////////////

#ifndef ATHVIEWS_VIEWTESTALG_H
#define ATHVIEWS_VIEWTESTALG_H 1

#include <string>
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

namespace AthViews {

class ViewTestAlg : public AthReentrantAlgorithm
{
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    // Athena algorithm hooks
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual StatusCode finalize() override;

  private:

    SG::WriteHandleKey< int > m_output{ this, "Output", "", "Optional output object" };
};

} //> end namespace AthViews

#endif //> !ATHVIEWS_VIEWTESTALG_H
